/*
============================================================
 File Name    : ClientHandler.cpp
 Module       : Client Session Handling Module (Server-Side)
 Description  : Implements secure authentication, command
                execution, and per-client session logging
                within a multi-client TCP server.

 Architectural Role:
  - Each connected client is handled by one instance.
  - Runs inside a forked child process.
  - Maintains isolated session context.
  - Ensures secure authentication before command execution.

 Security Features:
  - Uses OpenSSL SHA-256 hashing.
  - Generates unique salt per user.
  - Stores credentials securely in users.txt.
  - No plain-text password storage.
  - Per-session log file generation.

 Concurrency Model:
  - Process-based concurrency (fork()).
  - Each client operates independently.
============================================================
*/

#include <Server/ClientHandler.h>
#include <DirectoryTraverser/DirectoryTraverser.h>
#include <ContentScanner/ContentScanner.h>
#include <FileInspector/FileInspector.h>

#include <unistd.h>
#include <cstring>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <sys/socket.h>
#include <openssl/sha.h>

/*
============================================================
 Communication Configuration
============================================================
 BUFFER_SIZE : Maximum socket data chunk size.
 END_MARK    : Protocol marker indicating end of response.
============================================================
*/
#define BUFFER_SIZE 4096
#define END_MARK "<<END>>\n"

using namespace std; 

/*
------------------------------------------------------------
 Constructor Implementation
------------------------------------------------------------
 Description:
  Initializes ClientHandler object with the
  connected client's socket descriptor.

 Parameter:
  fd -> Active client socket file descriptor.

 Note:
  No authentication is performed here.
  Authentication occurs inside handle().
------------------------------------------------------------
*/
ClientHandler::ClientHandler(int fd) : client_fd(fd) {}


/*
------------------------------------------------------------
 Function Name : log()
------------------------------------------------------------
 Purpose:
  Write session activity messages into the
  dedicated per-client log file.

 Behavior:
  - Ensures log file is open.
  - Appends log entry with newline.
  - Used throughout session lifecycle.

 Importance:
  Provides full audit trail of:
    - Authentication events
    - Commands executed
    - Session termination
------------------------------------------------------------
*/
void ClientHandler::log(const string& message) {
    if (logFile.is_open())
        logFile << message << endl;
}


/*
------------------------------------------------------------
 Function Name : hashPassword()
------------------------------------------------------------
 Purpose:
  Generate SHA-256 hash of password + salt.

 Implementation Details:
  - Uses OpenSSL SHA256().
  - Converts binary hash to hexadecimal string.
  - Ensures fixed-length secure representation.

 Parameter:
  password -> Plain password concatenated with salt.

 Return:
  Hexadecimal SHA-256 hash string.

 Security Note:
  Hashing ensures passwords are never stored
  or compared in plain text.
------------------------------------------------------------
*/
string ClientHandler::hashPassword(const string& password) {

    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()),
           password.size(), hash);

    stringstream ss;

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << hex << setw(2)
           << setfill('0') << (int)hash[i];

    return ss.str();
}


/*
------------------------------------------------------------
 Function Name : generateSalt()
------------------------------------------------------------
 Purpose:
  Create random 16-character alphanumeric salt.

 Why Salt?
  - Prevents identical passwords from generating
    identical hashes.
  - Protects against rainbow table attacks.
  - Strengthens credential storage security.

 Return:
  Randomly generated salt string.
------------------------------------------------------------
*/
string ClientHandler::generateSalt() {

    static const char charset[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    string salt;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, sizeof(charset) - 2);

    for (int i = 0; i < 16; ++i)
        salt += charset[dist(gen)];

    return salt;
}


/*
------------------------------------------------------------
 Function Name : authenticate()
------------------------------------------------------------
 Purpose:
  Perform secure login and automatic registration.

 Workflow:
   1. Prompt user for username.
   2. Prompt user for password.
   3. Read users.txt file.
   4. If user exists:
        - Retrieve salt.
        - Hash entered password.
        - Compare hashes.
   5. If user does not exist:
        - Generate new salt.
        - Hash password.
        - Append new entry to users.txt.
   6. Create per-client log file.
   7. Return authentication result.

 Security Model:
  - Uses salted SHA-256 hashing.
  - No plain-text password storage.
  - Persistent credential storage.

 Return:
  true  -> Login success or new account created.
  false -> Incorrect password.
------------------------------------------------------------
*/
bool ClientHandler::authenticate() {

    char buffer[BUFFER_SIZE];

    /*
     Prompt for Username
    */
    if (send(client_fd, "Username: ", 10, 0) < 0) {
       perror("Send failed");
       return false;
    }
    memset(buffer, 0, BUFFER_SIZE);
    if (recv(client_fd, buffer, BUFFER_SIZE, 0) < 0) {
    perror("Recv failed");
    return false;
    }

    username = string(buffer);
    username.erase(username.find_last_not_of("\n\r") + 1);

    /*
     Prompt for Password
    */
    send(client_fd, "Password: ", 10, 0);
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_fd, buffer, BUFFER_SIZE, 0);

    string password(buffer);
    password.erase(password.find_last_not_of("\n\r") + 1);

    /*
     Open credential storage file.
    */
    ifstream usersFile("users.txt");
    string line;

    /*
     Check if user already exists.
    */
    while (getline(usersFile, line)) {

        stringstream ss(line);
        string storedUser, storedSalt, storedHash;

        getline(ss, storedUser, ':');
        getline(ss, storedSalt, ':');
        getline(ss, storedHash);

        if (storedUser == username) {

            string computed =
                hashPassword(password + storedSalt);

            if (computed == storedHash) {

                string logPath =
                    "logs/" + username + "_" +
                    to_string(getpid()) + ".log";

                logFile.open(logPath, ios::app);
                log("User authenticated");

                send(client_fd, "Login successful\n", 17, 0);
                return true;
            }

            send(client_fd, "Incorrect password\n", 19, 0);
            return false;
        }
    }

    usersFile.close();

    /*
     User not found → Register automatically
    */
    string salt = generateSalt();
    string hashed = hashPassword(password + salt);

    ofstream out("users.txt", ios::app);
    out << username << ":" << salt << ":" << hashed << "\n";
    out.close();

    string logPath =
        "logs/" + username + "_" +
        to_string(getpid()) + ".log";

    logFile.open(logPath, ios::app);
    log("New user registered securely");

    send(client_fd, "Account created\n", 16, 0);
    return true;
}


/*
------------------------------------------------------------
 Function Name : handle()
------------------------------------------------------------
 Purpose:
  Manage full authenticated client session.

 Session Lifecycle:
   1. Authenticate client.
   2. Enter command processing loop.
   3. Execute requested operations:
        - TRAVERSE
        - SEARCH
        - INSPECT
        - EXIT
   4. Log all activity.
   5. Clean up resources.

 Concurrency:
   Runs inside dedicated child process
   created by fork() in Server.cpp.
------------------------------------------------------------
*/
void ClientHandler::handle() {

    /*
     Step 1: Authenticate client.
    */
    if (!authenticate()) {
        close(client_fd);
        return;
    }

    char buffer[BUFFER_SIZE];

    /*
     Step 2: Enter Command Processing Loop
    */
    while (true) {

        memset(buffer, 0, BUFFER_SIZE);
        ssize_t received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (received < 0) {
         perror("Recv failed");
         break;
        }

        if (received == 0) {
         std::cout << "Client disconnected\n";
         break;
        }


        string command(buffer);
        log("Command: " + command);

        /*
        =======================================================
         TRAVERSE COMMAND
        =======================================================
         Performs recursive directory traversal and
         returns directory structure + total file count.
        =======================================================
        */
        if (command.compare(0, 8, "TRAVERSE") == 0) {

            string path = command.substr(9);

            string fileList = "files.txt";
            ofstream(fileList, ios::trunc).close();

            int fileCount = 0;

            DirectoryTraverser::traverse(
                path,
                client_fd,
                fileCount,
                fileList
            );

            string summary =
                "\nTotal Files: " +
                to_string(fileCount) + "\n";

            send(client_fd,
                 summary.c_str(),
                 summary.size(),
                 0);

            send(client_fd,
                 END_MARK,
                 strlen(END_MARK),
                 0);

            log("Traverse completed");
        }

        /*
        =======================================================
         SEARCH COMMAND
        =======================================================
         Performs directory traversal followed by
         pattern matching on discovered files.
        =======================================================
        */
        else if (command.compare(0, 6, "SEARCH") == 0) {

            size_t spacePos = command.find(' ', 7);
            if (spacePos == string::npos) {
                continue;
            }

            string path =
                command.substr(7, spacePos - 7);
            string pattern =
                command.substr(spacePos + 1);

            string fileList = "files.txt";
            ofstream(fileList, ios::trunc).close();

            int fileCount = 0;

            DirectoryTraverser::traverse(
                path,
                client_fd,
                fileCount,
                fileList
            );

            auto matches =
                ContentScanner::scan(fileList, pattern);

            if (matches.empty()) {

                string msg =
                    "\nNo matches found\n";

                send(client_fd,
                     msg.c_str(),
                     msg.size(),
                     0);
            }
            else {

                string header =
                    "\nMatched Files:\n";

                send(client_fd,
                     header.c_str(),
                     header.size(),
                     0);

                for (const auto& file : matches) {

                    string msg =
                        file + "\n";

                    send(client_fd,
                         msg.c_str(),
                         msg.size(),
                         0);
                }
            }

            send(client_fd,
                 END_MARK,
                 strlen(END_MARK),
                 0);

            log("Search completed");
        }

        /*
        =======================================================
         INSPECT COMMAND
        =======================================================
         Displays contents of a specific file.
        =======================================================
        */
        else if (command.compare(0, 7, "INSPECT") == 0) {

            string path = command.substr(8);

            FileInspector::inspect(path,
                                   client_fd);

            log("Inspect executed");
        }

        /*
        =======================================================
         EXIT COMMAND
        =======================================================
         Terminates client session gracefully.
        =======================================================
        */
        else if (command.compare(0, 4, "EXIT") == 0) {

            log("Session ended");
            break;
        }

        /*
        =======================================================
         UNKNOWN COMMAND
        =======================================================
         Handles unsupported operations.
        =======================================================
        */
        else {

            string err =
                "ERROR: Unknown command\n";

            send(client_fd,
                 err.c_str(),
                 err.size(),
                 0);

            send(client_fd,
                 END_MARK,
                 strlen(END_MARK),
                 0);
        }
    }

    /*
     Final Cleanup
    */
    logFile.close();
    close(client_fd);
}

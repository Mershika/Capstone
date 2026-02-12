#include <Server/ClientHandler.h>
#include <DirectoryTraverser/DirectoryTraverser.h>
#include <ContentScanner/ContentScanner.h>
#include <FileInspector/FileInspector.h>
#include <Logger/Logger.h>

#include <unistd.h>
#include <cstring>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <sys/socket.h>
#include <openssl/sha.h>
#include <cerrno>
#include <iostream>

#define BUFFER_SIZE 4096
#define END_MARK "<<END>>\n"

using namespace std;


/* ----------------------------------------------------------
   Safe Send Utility
   Ensures full buffer transmission
----------------------------------------------------------- */
static bool safe_send(int fd, const string& data) {

    size_t totalSent = 0;
    while (totalSent < data.size()) {

        ssize_t sent = send(fd,
                            data.c_str() + totalSent,
                            data.size() - totalSent,
                            0);

        if (sent <= 0) {
            perror("send failed");
            return false;
        }

        totalSent += sent;
    }
    return true;
}
/*
============================================================
 File Name    : ClientHandler.cpp
 Module       : Client Session Management Module  
 Description  : Implements per-client session handling for
                the DCDIU server with authentication,
                command processing, and session logging.
============================================================
*/

/* ----------------------------------------------------------
   Constructor Implementation
   
   Description:
   Stores the connected client's socket descriptor.
   
   Parameter:
   fd -> File descriptor of connected client socket
   
   Note: Does not perform socket operations yet.
   Initialization happens in handle() method.
----------------------------------------------------------- */
ClientHandler::ClientHandler(int fd) : client_fd(fd) {}
/* ----------------------------------------------------------
   Logging Helper Function
   
   Description:
   Writes a message to the client's session log file.
   
   Log File Format:
   logs/{username}_{process_id}.log
   
   Example: logs/alice_4521.log
   
   This ensures each client session has its own log,
   with unique identification by user and PID.
----------------------------------------------------------- */
void ClientHandler::log(const string& message) {
    if (logFile.is_open())
        logFile << message << endl;
}
/* ----------------------------------------------------------
   Password Hashing Function
   
   Description:
   Computes SHA-256 hash of the given password string
   using OpenSSL library.
   
   Algorithm:
   - Converts string to bytes
   - Applies SHA-256 (NIST approved cryptographic hash)
   - Produces 32-byte (256-bit) hash
   - Converts bytes to hexadecimal (64 characters)
   
   Parameter: password -> String to hash
   Return: Hex string of hash (or empty on error)
   
   Security Note:
   This function computes the hash. Salting is done
   by calling: hashPassword(password + salt)
   in the authenticate() function.
----------------------------------------------------------- */
string ClientHandler::hashPassword(const string& password) {

    unsigned char hash[SHA256_DIGEST_LENGTH];

    if (!SHA256(reinterpret_cast<const unsigned char*>(password.c_str()),
                password.size(), hash)) {

        cerr << "SHA256 hashing failed\n";
        return "";
    }

    stringstream ss;

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << hex << setw(2)
           << setfill('0') << (int)hash[i];

    return ss.str();
}
/* ----------------------------------------------------------
   Salt Generation Function
   
   Description:
   Generates a random 16-character alphanumeric salt
   using C++11 random facilities.
   
   Character Set: [0-9A-Za-z] (62 possibilities)
   
   Implementation:
   - random_device for seed
   - mt19937 Mersenne Twister generator
   - uniform_int_distribution for balanced selection
   
   Return: Random 16-char string
   
   Purpose:
   Prevent rainbow table attacks. Each user's password
   is salted differently, making precomputed tables
   ineffective.
   
   Example:
   Password: "secret123"
   Salt: "aB7xK3mN2pQw1vYz"
   Hash Input: "secret123aB7xK3mN2pQw1vYz"
   This ensures identical passwords produce different hashes.
----------------------------------------------------------- */
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
================================================================
   Authentication Function
   
   This is the CORE SECURITY function of the DCDIU system.
   
   Workflow:
   ---------
   1. RECEIVE USERNAME from client socket
   2. RECEIVE PASSWORD from client socket
   3. OPEN credentials file (data/users.txt)
   4. SEARCH for matching username
      
      IF USER FOUND (LOGIN):
      ├─ RETRIEVE stored salt and hash
      ├─ COMPUTE hash of (provided_password + stored_salt)
      ├─ COMPARE with stored hash
      ├─ If MATCH: Login successful, create log file
      └─ If NO MATCH: Reject with "Incorrect password"
      
      IF USER NOT FOUND (REGISTRATION):
      ├─ GENERATE random 16-character salt
      ├─ COMPUTE hash of (provided_password + generated_salt)
      ├─ APPEND username:salt:hash to users.txt
      ├─ Create new session log file
      └─ Confirm account creation
   
   5. CREATE per-session log file
      Format: logs/{username}_{process_id}.log
      Content: All commands and results for this session
   
   Credentials File Format (data/users.txt):
   ────────────────────────────────────────
   username:16char_salt:SHA256_hash_of_(password+salt)
   
   Example:
   alice:aBcDeF1234567890:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
   
   Return:
   true  -> Authentication successful (user logged in)
   false -> Authentication failed (wrong password)
   
   Side Effects:
   - Creates or updates data/users.txt
   - Creates logs/{username}_{pid}.log
   - Sends messages to client via socket
   
   Security Considerations:
   - Passwords are salted and hashed (no plaintext storage)
   - Each user gets unique salt (prevents rainbow tables)
   - SHA-256 is cryptographically secure
   - Credentials file should be read-only (644 permissions)
================================================================================================
*/
bool ClientHandler::authenticate() {

    char buffer[BUFFER_SIZE];

    // Username prompt
    if (!safe_send(client_fd, "Username: "))
        return false;

    memset(buffer, 0, BUFFER_SIZE);
    ssize_t received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

    if (received <= 0) {
        perror("recv username failed");
        return false;
    }

    /* Store username and remove trailing newlines */
    username = string(buffer);

    size_t pos = username.find_last_not_of("\n\r");
    if (pos != string::npos)
        username.erase(pos + 1);

    // Password prompt
    if (!safe_send(client_fd, "Password: "))
        return false;

    memset(buffer, 0, BUFFER_SIZE);
    received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

    if (received <= 0) {
        perror("recv password failed");
        return false;
    }

    string password(buffer);

    pos = password.find_last_not_of("\n\r");
    if (pos != string::npos)
        password.erase(pos + 1);

    ifstream usersFile("data/users.txt");
    if (!usersFile.is_open()) {
        cerr << "Could not open users.txt\n";
    }

    string line;

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

                if (!logFile.is_open())
                    cerr << "Failed to open log file\n";

                log("User authenticated");

                safe_send(client_fd, "Login successful\n");
                return true;
            }

            safe_send(client_fd, "Incorrect password\n");
            return false;
        }
    }

    usersFile.close();

    string salt = generateSalt();
    string hashed = hashPassword(password + salt);

    ofstream out("data/users.txt", ios::app);


    if (!out.is_open()) {
        cerr << "Could not write users.txt\n";
        return false;
    }

    out << username << ":" << salt << ":" << hashed << "\n";
    out.close();

    string logPath =
        "logs/" + username + "_" +
        to_string(getpid()) + ".log";

    logFile.open(logPath, ios::app);

    if (!logFile.is_open())
        cerr << "Failed to open log file\n";

    log("New user registered securely");

    safe_send(client_fd, "Account created\n");
    return true;
}
/*
================================================================
   Main Session Handler Function
   
   Overview:
   ---------
   This function manages the complete lifecycle of a client
   session after successful authentication.
   
   Workflow:
   1. Call authenticate() to login/register user
   2. Enter command reception loop
   3. For each received command:
      - Parse command type and parameters
      - Execute corresponding operation
      - Send results back to client
   4. Exit on EXIT command or connection close
   
   Supported Commands:
   
   1. TRAVERSE {path}
      └─ Recursively list all files in directory tree
      └─ Store results in data/files.txt
      └─ Send summary with total file count
      
   2. SEARCH {path} {pattern}
      └─ Traverse directory to get file list
      └─ Scan files for pattern/string occurrences
      └─ Return list of matching files
      
   3. INSPECT {path}
      └─ Read file from disk
      └─ Stream entire file contents to client
      
   4. EXIT
      └─ Close session gracefully
      └─ Close client socket
      └─ Close session log file
   
   Protocol Details:
   - Commands sent as plain text strings (newline terminated)
   - Responses sent as text sequences
   - Response ends marked with "<<END>>"  (END_MARK)
   - Each command processed synchronously
   
   Error Handling:
   - Unknown commands return ERROR message
   - Missing parameters are silently ignored
   - File access errors are caught and logged
   
   Logging:
   - All received commands logged to session file
   - All operations logged to global system logger
   - Session starts/ends are logged
   
================================================================
*/
void ClientHandler::handle() {

    if (!authenticate()) {
        close(client_fd);
        return;
    }

    char buffer[BUFFER_SIZE];

    while (true) {

        memset(buffer, 0, BUFFER_SIZE);

        ssize_t received =
            recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (received <= 0) {
            if (errno != EINTR)
                perror("recv failed");
            break;
        }

        string command(buffer);
        log("Command: " + command);

        // ================= TRAVERSE =================
        if (command.compare(0, 8, "TRAVERSE") == 0) {

            string path = command.substr(9);

            string fileList = "data/files.txt";
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

            safe_send(client_fd, summary);
            safe_send(client_fd, END_MARK);

            LOG(INFO,
                           "TRAVERSE",
                           "Traversal completed");
        }

        // ================= SEARCH =================
        else if (command.compare(0, 6, "SEARCH") == 0) {

            size_t spacePos = command.find(' ', 7);
            if (spacePos == string::npos)
                continue;

            string path =
                command.substr(7, spacePos - 7);

            string pattern =
                command.substr(spacePos + 1);

            string fileList = "data/files.txt";

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

                safe_send(client_fd,
                          "\nNo matches found\n");
            }
            else {

                safe_send(client_fd,
                          "\nMatched Files:\n");

                for (const auto& file : matches)
                    safe_send(client_fd,
                              file + "\n");
            }

            safe_send(client_fd, END_MARK);

            LOG(INFO, "SEARCH", "Search completed");
        }

        // ================= INSPECT =================
        else if (command.compare(0, 7, "INSPECT") == 0) {

            string path = command.substr(8);

            FileInspector::inspect(path,
                                   client_fd);

            LOG(INFO, "INSPECT", "Inspect executed");
        }

        // ================= EXIT =================
        else if (command.compare(0, 4, "EXIT") == 0) {

            log("Session ended");
            break;
        }

        // ================= UNKNOWN =================
        else {

            safe_send(client_fd,
                      "ERROR: Unknown command\n");
            safe_send(client_fd,
                      END_MARK);
        }
    }

    logFile.close();
    close(client_fd);
}

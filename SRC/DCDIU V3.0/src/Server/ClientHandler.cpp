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


/* ----------------------------------------------------------
   Constructor
----------------------------------------------------------- */
ClientHandler::ClientHandler(int fd) : client_fd(fd) {}


/* ----------------------------------------------------------
   Logging
----------------------------------------------------------- */
void ClientHandler::log(const string& message) {
    if (logFile.is_open())
        logFile << message << endl;
}


/* ----------------------------------------------------------
   Password Hashing
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
   Salt Generator
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


/* ----------------------------------------------------------
   Authentication
----------------------------------------------------------- */
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


/* ----------------------------------------------------------
   Session Handler
----------------------------------------------------------- */
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

            Logger::getInstance()->log(INFO,
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

            Logger::getInstance()->log(INFO,
                           "SEARCH",
                           "Search completed");
        }

        // ================= INSPECT =================
        else if (command.compare(0, 7, "INSPECT") == 0) {

            string path = command.substr(8);

            FileInspector::inspect(path,
                                   client_fd);

            Logger::getInstance()->log(INFO,
                           "INSPECT",
                           "Inspect executed");
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

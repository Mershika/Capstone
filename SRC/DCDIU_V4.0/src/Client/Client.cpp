/*
============================================================
 File Name    : Client.cpp
 Module       : Client Application Module
 Description  : Implements the client-side TCP communication
                logic for interacting with the DCDIU server.

 Responsibilities:
  - Establish TCP connection with server
  - Perform secure authentication handshake
  - Provide interactive CLI menu
  - Send formatted protocol commands
  - Receive segmented server responses
  - Handle session termination gracefully

 Communication Protocol:
  - Client initiates connection
  - Performs authentication (username/password)
  - Sends command strings
  - Receives responses terminated by END_MARK
  - Closes connection on exit

============================================================
*/

#include <Client/Client.h>
#include <Logger/Logger.h>

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <termios.h>
#include <cerrno>
#include <limits>

#define BUFFER_SIZE 4096
#define END_MARK "<<END>>"

using namespace std;


/* ----------------------------------------------------------
   Safe send to avoid partial transmission
---------------------------------------------------------- */
static bool safe_send(int sock, const string& data) {

    size_t totalSent = 0;

    while (totalSent < data.size()) {

        ssize_t sent = send(sock,
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


Client::Client(int port) : port(port) {}


void Client::start() {

    Logger::getInstance()->setLogFile("logs/client.log");
    Logger::getInstance()->setLogLevel(DEBUG);
    LOG(INFO, "CLIENT", "Client started");

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        perror("socket creation failed");
        return;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sock);
        return;
    }

    if (connect(sock,
                (sockaddr*)&server,
                sizeof(server)) < 0) {
        perror("connect failed");
        close(sock);
        return;
    }

    char buffer[BUFFER_SIZE];
    string input;

    /* ---------------- Authentication ---------------- */

    memset(buffer, 0, BUFFER_SIZE);
    ssize_t bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);

    if (bytes <= 0) {
        perror("recv username prompt failed");
        close(sock);
        return;
    }

    cout << buffer;

    getline(cin, input);

    if (!safe_send(sock, input)) {
        close(sock);
        return;
    }

    memset(buffer, 0, BUFFER_SIZE);
    bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);

    if (bytes <= 0) {
        perror("recv password prompt failed");
        close(sock);
        return;
    }

    cout << buffer;

    termios oldt{}, newt{};

    if (tcgetattr(STDIN_FILENO, &oldt) == -1) {
        perror("tcgetattr failed");
        close(sock);
        return;
    }

    newt = oldt;
    newt.c_lflag &= ~ECHO;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) == -1) {
        perror("tcsetattr failed");
        close(sock);
        return;
    }

    getline(cin, input);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &oldt) == -1) {
        perror("tcsetattr restore failed");
    }

    cout << endl;

    if (!safe_send(sock, input)) {
        close(sock);
        return;
    }

    memset(buffer, 0, BUFFER_SIZE);
    bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);

    if (bytes <= 0) {
        cout << "Authentication failed. Connection closed.\n";
        close(sock);
        return;
    }

    cout << buffer << endl;

    if (string(buffer).find("Incorrect") != string::npos) {
        close(sock);
        return;
    }

    LOG(
    INFO,
    "AUTH",
    "Authentication successful"
    );

    /* ---------------- Main Loop ---------------- */

    while (true) {

        int choice;
        string path, cmd;

        cout << "\nDisplay Menu:";
        cout << "\n1. Traverse\n2. Search\n3. Inspect\n4. Exit\nChoice: ";

        if (!(cin >> choice)) {
            cerr << "Invalid input\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        cin.ignore();

        if (choice == 1) {

            cout << "Enter directory path: ";
            getline(cin, path);
            cmd = "TRAVERSE " + path;
        }
        else if (choice == 2) {

            string pattern;

            cout << "Enter directory path: ";
            getline(cin, path);

            cout << "Enter search pattern: ";
            getline(cin, pattern);

            cmd = "SEARCH " + path + " " + pattern;
        }
        else if (choice == 3) {

            cout << "Enter file path: ";
            getline(cin, path);
            cmd = "INSPECT " + path;
        }
        else {

            cmd = "EXIT";

            LOG(
            DEBUG,
            "COMMAND",
            cmd
        );

            safe_send(sock, cmd);
            break;
        }

        LOG(
        DEBUG,
        "COMMAND",
        cmd
    );

        if (!safe_send(sock, cmd)) {
            close(sock);
            return;
        }

        string response;

        while (true) {

            memset(buffer, 0, BUFFER_SIZE);

            ssize_t recvBytes =
                recv(sock, buffer, BUFFER_SIZE - 1, 0);

            if (recvBytes < 0) {

                if (errno == EINTR)
                    continue;

                perror("recv failed");
                close(sock);
                return;
            }

            if (recvBytes == 0)
                break;

            response.append(buffer, recvBytes);

            if (response.find(END_MARK) != string::npos)
                break;
        }

        size_t pos = response.find(END_MARK);

        if (pos != string::npos)
            response.erase(pos);

        cout << response << endl;
    }

    LOG(
    INFO,
    "CLIENT",
    "Client exited"
);


    close(sock);
}

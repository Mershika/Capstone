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

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <termios.h>   // Used for disabling terminal echo

/*
============================================================
 Communication Configuration
============================================================
 Defines:
  - BUFFER_SIZE : Maximum data chunk size for socket transfer
  - END_MARK    : Protocol marker indicating end of response
============================================================
*/
#define BUFFER_SIZE 4096
#define END_MARK "<<END>>"

using namespace std;


/*
============================================================
 Constructor : Client::Client
============================================================
 Description:
  Initializes client object with server port number.

 Notes:
  - Socket creation is deferred to start()
  - No connection established at construction time
============================================================
*/
Client::Client(int port) : port(port) {}


/*
============================================================
 Function Name : start()
============================================================
 Description:
  Controls the complete lifecycle of client execution.

 Responsibilities:
   1. Create TCP socket
   2. Connect to server
   3. Perform authentication handshake
   4. Display interactive CLI menu
   5. Send commands to server
   6. Receive segmented responses
   7. Perform graceful shutdown

 This function represents the main execution controller
 for the client-side application.
============================================================
*/
void Client::start() {

    /*
    --------------------------------------------------------
     Socket Initialization
    --------------------------------------------------------
     - Create TCP socket
     - Configure server address
     - Establish connection
    --------------------------------------------------------
    */
    sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    connect(sock, (sockaddr*)&server, sizeof(server));


    /*
    ========================================================
     AUTHENTICATION HANDSHAKE
    ========================================================
     Protocol Flow:
       1. Receive "Username:" prompt
       2. Send username
       3. Receive "Password:" prompt
       4. Disable terminal echo
       5. Send password securely
       6. Receive authentication result

     Security:
       - Password input is hidden using POSIX termios
       - Echo flag temporarily disabled
       - Terminal restored after input
    ========================================================
    */

    char buffer[BUFFER_SIZE];
    string input;

    /*
     Receive Username Prompt
    */
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);
    cout << buffer;

    getline(cin, input);
    send(sock, input.c_str(), input.size(), 0);


    /*
     Receive Password Prompt
    */
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);
    cout << buffer;

    /*
     Disable Terminal Echo for Secure Password Entry
    */
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    getline(cin, input);

    /*
     Restore Terminal Settings
    */
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    cout << endl;

    send(sock, input.c_str(), input.size(), 0);


    /*
     Receive Authentication Result
    */
    memset(buffer, 0, BUFFER_SIZE);
    ssize_t authBytes = recv(sock, buffer, BUFFER_SIZE, 0);

    if (authBytes <= 0) {
        cout << "Authentication failed. Connection closed.\n";
        close(sock);
        return;
    }

    cout << buffer << endl;

    /*
     If authentication fails,
     server closes connection.
    */
    if (string(buffer).find("Incorrect") != string::npos) {
        close(sock);
        return;
    }


    /*
    ========================================================
     CLIENT INTERACTION LOOP
    ========================================================
     Continuously:
       - Display menu
       - Accept user selection
       - Format command string
       - Send request to server
       - Receive full response
       - Strip END_MARK
       - Display result
    ========================================================
    */
    while (true) {

        int choice;
        string path, cmd;

        /*
         Display CLI Menu
        */
        cout << "\nDisplay Menu:";
        cout << "\n1. Traverse\n2. Search\n3. Inspect\n4. Exit\nChoice: ";

        cin >> choice;
        cin.ignore();

        /*
        ----------------------------------------------------
         Command Preparation
        ----------------------------------------------------
         Build protocol-compliant request strings
         based on selected operation.
        ----------------------------------------------------
        */
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

            /*
             Graceful Termination
            */
            cmd = "EXIT";
            send(sock, cmd.c_str(), cmd.size(), 0);
            break;
        }

        /*
         Send Command to Server
        */
        send(sock, cmd.c_str(), cmd.size(), 0);


        /*
        ----------------------------------------------------
         Response Reception Loop
        ----------------------------------------------------
         Server may send data in multiple segments.
         Continue reading until END_MARK is detected.
        ----------------------------------------------------
        */
        string response;

        while (true) {

            memset(buffer, 0, BUFFER_SIZE);
            ssize_t bytes = recv(sock, buffer, BUFFER_SIZE, 0);

            if (bytes <= 0)
                break;

            response.append(buffer, bytes);

            if (response.find(END_MARK) != string::npos)
                break;
        }

        /*
         Remove Protocol End Marker
        */
        size_t pos = response.find(END_MARK);
        if (pos != string::npos)
            response.erase(pos);

        /*
         Display Final Response
        */
        cout << response << endl;
    }


    /*
    --------------------------------------------------------
     Connection Cleanup
    --------------------------------------------------------
     Release socket descriptor before program exit.
    --------------------------------------------------------
    */
    close(sock);
}

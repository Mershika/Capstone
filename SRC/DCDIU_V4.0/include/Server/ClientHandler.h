/*
============================================================
 File Name    : ClientHandler.h
 Module       : Client Handling Module (Server-Side)
 Description  : Declares the ClientHandler class responsible
                for managing authenticated client sessions
                in a multi-client TCP server architecture.

 Responsibilities:
  - Manage client socket descriptor
  - Perform secure authentication (salted SHA-256)
  - Handle per-client logging
  - Process client commands
  - Maintain isolated session lifecycle
  - Ensure proper resource cleanup

 Security Features:
  - Salted SHA-256 password hashing
  - Persistent credential storage
  - Per-session log file generation

============================================================
*/

#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <string>
#include <fstream>

using namespace std;

/*
------------------------------------------------------------
 Class Name    : ClientHandler
 Description   : Represents a server-side session controller
                responsible for managing communication and
                request processing for a single connected
                client.

                Each client connection is handled inside
                a separate process (via fork()) and managed
                through an independent instance of this class.

                This ensures:
                  - Concurrency
                  - Session isolation
                  - Secure authentication
                  - Individual logging
------------------------------------------------------------
*/
class ClientHandler {

private:

    /*
     * File descriptor representing
     * the connected client socket.
     */
    int client_fd;

    /*
     * Authenticated username associated
     * with this client session.
     */
    string username;

    /*
     * Output file stream used to log
     * all client session activities.
     */
    ofstream logFile;

    /*
     * --------------------------------------------------------
     * Function Name : authenticate()
     * --------------------------------------------------------
     * Description   : Performs secure authentication
     *                using salted SHA-256 hashing.
     *
     *                Workflow:
     *                  - Receive username
     *                  - Receive password
     *                  - Validate against users.txt
     *                  - Register new user if not found
     *                  - Create per-client log file
     *
     * Return        : true  -> authentication success
     *                 false -> authentication failure
     * --------------------------------------------------------
     */
    bool authenticate();

    /*
     * --------------------------------------------------------
     * Function Name : log()
     * --------------------------------------------------------
     * Description   : Writes session activity messages
     *                into the client's dedicated log file.
     *
     * @param message Log message string.
     * --------------------------------------------------------
     */
    void log(const string& message);

    /*
     * --------------------------------------------------------
     * Function Name : hashPassword()
     * --------------------------------------------------------
     * Description   : Generates SHA-256 hash of a given
     *                password string.
     *
     * @param password Input password string.
     * Return         : Hexadecimal hashed string.
     * --------------------------------------------------------
     */
    string hashPassword(const string& password);

    /*
     * --------------------------------------------------------
     * Function Name : generateSalt()
     * --------------------------------------------------------
     * Description   : Generates a random alphanumeric
     *                salt string for password hashing.
     *
     * Return         : Random salt string.
     * --------------------------------------------------------
     */
    string generateSalt();

public:

    /*
     * --------------------------------------------------------
     * Constructor
     * --------------------------------------------------------
     * Initializes ClientHandler with client socket descriptor.
     *
     * @param fd  File descriptor of connected client.
     * --------------------------------------------------------
     */
    ClientHandler(int fd);

    /*
     * --------------------------------------------------------
     * Function Name : handle()
     * --------------------------------------------------------
     * Description   : Controls full lifecycle of a client
     *                session including:
     *
     *                  - Authentication
     *                  - Command reception
     *                  - Operation execution
     *                  - Logging
     *                  - Graceful termination
     *
     * Return        : void
     * --------------------------------------------------------
     */
    void handle();
};

#endif  // CLIENT_HANDLER_H

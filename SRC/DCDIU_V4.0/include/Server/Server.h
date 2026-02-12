/*
============================================================
 File Name    : Server.h
 Module       : Server Core Module
 Description  : Declares the Server class responsible for
                initializing and managing the TCP server
                of the DCDIU system.

 Responsibilities:
  - Create and configure server socket
  - Bind to specified port
  - Listen for incoming client connections
  - Accept client connections
  - Delegate client handling to ClientHandler

============================================================
*/

#ifndef SERVER_H
#define SERVER_H

/*
------------------------------------------------------------
 Class Name    : Server
 Description   : Represents the core TCP server component
                of the DCDIU system.

                Responsible for establishing the listening
                socket and managing incoming client
                connections in a structured manner.
------------------------------------------------------------
*/
class Server {

private:

    /*
     * Server socket file descriptor used to
     * accept incoming client connections.
     */
    int server_fd;

    /*
     * Port number on which the server listens
     * for client connections.
     */
    int port;

public:

    /*
     * --------------------------------------------------------
     * Constructor
     * --------------------------------------------------------
     * Initializes the Server with the specified
     * port number.
     *
     * @param port :  Port number on which the server
     *              will listen.
     * --------------------------------------------------------
     */
    Server(int port);

    /*
     * --------------------------------------------------------
     * Function Name : start()
     * --------------------------------------------------------
     * Description   : Starts the server execution.
     *
     *                Responsible for:
     *                  - Creating server socket
     *                  - Binding to the port
     *                  - Listening for connections
     *                  - Accepting client requests
     *                  - Delegating clients to handler
     *
     * Return        : void
     * --------------------------------------------------------
     */
    void start();
};

#endif  // SERVER_H
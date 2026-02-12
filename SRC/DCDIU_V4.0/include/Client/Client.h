/*
============================================================
 File Name    : Client.h
 Module       : Client Module
 Description  : Declares the Client class which manages
                client-side communication with the DCDIU
                server, including connection setup and
                request handling.

 Responsibilities:
  - Maintain socket connection
  - Store server port information
  - Provide interface for client execution
  - Support client-server interaction

============================================================
*/

#ifndef CLIENT_H
#define CLIENT_H

/*
------------------------------------------------------------
 Class Name    : Client
 Description   : Represents the client-side component of
                the DCDIU system. Handles connection
                initialization and communication with
                the server.
------------------------------------------------------------
*/
class Client {

private:

    /*
     * Socket descriptor for server communication.
     */
    int sock;

    /*
     * Port number of the DCDIU server.
     */
    int port;

public:

    /*
     * --------------------------------------------------------
     * Constructor
     * --------------------------------------------------------
     * Initializes Client with server port number.
     *
     * @param port  Port on which the server is listening.
     * --------------------------------------------------------
     */
    Client(int port);

    /*
     * --------------------------------------------------------
     * Function Name : start()
     * --------------------------------------------------------
     * Description   : Starts the client execution process.
     *                Establishes connection and begins
     *                communication with the server.
     *
     * Return        : void
     * --------------------------------------------------------
     */
    void start();
};

#endif  // CLIENT_H

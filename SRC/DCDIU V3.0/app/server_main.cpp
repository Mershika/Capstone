/*
============================================================
 Project      : DCDIU (Directory Content Detection & Inspection Utility)
 File Name    : server_main.cpp
 Module       : Server Application
 Description  : Acts as the entry point for the DCDIU server.
                Initializes the server, listens for client
                connections, and manages request processing.

 Responsibilities:
  - Initialize server configuration
  - Create Server instance
  - Start listening for client connections
  - Manage server lifecycle
  - Perform graceful shutdown

============================================================
*/

#include <Server/Server.h>

/*
------------------------------------------------------------
 Function Name : main()
 Description   : Entry point of the server application.
                Creates a Server object, binds to the given
                port, and starts server execution.
 Input         : None
 Output        : Returns program execution status.
 Return Value  : 0 on successful execution.
------------------------------------------------------------
*/
int main() {

    /*
     * Create Server instance with listening port.
     */
    Server server(9090);

    /*
     * Start server execution.
     */
    server.start();

    /*
     * Return success status.
     */
    return 0;
}

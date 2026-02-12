/*
===========================================
 Project      : DCDIU (Directory Content Detection & Inspection Utility)
 File Name    : client_main.cpp
 Module       : Client Application
 Description  : Acts as the entry point for the DCDIU client.
                Initializes the client object and starts
                client-side execution.
Responsibilities:
  - Initialize client configuration
  - Create Client instance
  - Invoke client startup sequence
  - Manage program termination

=============================================
*/
#include <Client/Client.h>

/*
----------------------------------------------------
Function Name : main()
 Description   : Entry point of the client application.
                Creates a Client object with the required
                configuration and starts execution.
 Input         : None
 Output        : Returns program execution status.
 Return Value  : 0 on successful execution.
 -----------------------------------------------------
*/

int main() {
    /*
      Create Client instance with server port number.
    */
    Client client(9090);

    /*
      Start client execution.
    */
    client.start();

    /*
     * Return success status.
     */
    return 0;
}

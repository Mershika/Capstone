/*
============================================================
 File Name    : Server.cpp
 Module       : Server Core Module
 Description  : Implements the primary TCP server responsible
                for:

                 - Creating listening socket
                 - Accepting multiple client connections
                 - Forking child processes per client
                 - Delegating session control to ClientHandler
                 - Managing zombie process cleanup
                 - Handling graceful shutdown using signals

 Architecture Overview:
  - Process-based concurrency using fork()
  - Each client runs in isolated child process
  - Parent process continues accepting connections
  - Clean shutdown via SIGINT (Ctrl + C)

 Networking Model:
  - IPv4 (AF_INET)
  - TCP protocol (SOCK_STREAM)
  - Blocking accept() loop
  - Port reuse enabled (SO_REUSEADDR)

 Signal Handling:
  - SIGINT captured
  - Listening socket closed
  - Accept loop terminated safely
  - Child processes reaped

============================================================
*/

#include <Server/Server.h>
#include <Server/ClientHandler.h>
#include <Logger/Logger.h>

#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <iostream>
#include <signal.h>

/*
============================================================
 Global Server Control Variables
============================================================

 g_server_fd:
   - Stores the listening socket descriptor.
   - Required so signal handler can close it safely.

 g_running:
   - Volatile flag used to control server accept loop.
   - Modified inside signal handler.
   - sig_atomic_t ensures safe access across signal context.

============================================================
*/
static int g_server_fd = -1;
static volatile sig_atomic_t g_running = 1;


/*
============================================================
 Function Name : handle_sigint
============================================================

 Purpose:
   Handle SIGINT signal (triggered by Ctrl + C).

 Behavior:
   - Print shutdown message
   - Stop main accept loop
   - Close listening socket
   - Allow server to exit gracefully

 Why Needed:
   Without this, server would terminate abruptly.
   With this, we ensure:
     - Controlled shutdown
     - Resource cleanup
     - No corrupted state

 Parameter:
   signum -> Signal number (unused)

============================================================
*/
void handle_sigint(int signum) {

    (void)signum;

    Logger::getInstance()->log(INFO,
                           "SERVER",
                           "Server shutting down");

    std::cout << "\nServer shutting down..." << std::endl;

    g_running = 0;

    if (g_server_fd != -1)
        close(g_server_fd);
}


/*
------------------------------------------------------------
 Constructor : Server::Server
------------------------------------------------------------

 Description:
   Initializes Server object with the specified port.

 Parameter:
   port -> TCP port number to bind and listen on.

 Note:
   No socket operations performed here.
   Actual initialization happens in start().

------------------------------------------------------------
*/
Server::Server(int port) : port(port) {}


/*
------------------------------------------------------------
 Function Name : start()
------------------------------------------------------------

 Description:
   Controls entire lifecycle of server execution.

 Responsibilities:
     1. Register signal handler
     2. Create TCP socket
     3. Configure socket options
     4. Bind to network interface and port
     5. Listen for incoming connections
     6. Accept clients continuously
     7. Fork child process per client
     8. Clean zombie processes
     9. Shutdown gracefully when signaled

 Concurrency Model:
   - fork() based multi-processing
   - Memory isolation per client
   - Parent process remains lightweight

------------------------------------------------------------
*/
void Server::start() {
    Logger::getInstance()->setLogFile("logs/server.log");
    Logger::getInstance()->setLogLevel(DEBUG);
    Logger::getInstance()->log(INFO, "SERVER", "Server started");
    /*
    ========================================================
     STEP 1: Register SIGINT Handler
    ========================================================

     This ensures that when Ctrl+C is pressed:
       - handle_sigint() is executed
       - Server stops safely
    ========================================================
    */
    signal(SIGINT, handle_sigint);


    /*
    ========================================================
     STEP 2: Create TCP Listening Socket
    ========================================================

     AF_INET     -> IPv4 address family
     SOCK_STREAM -> TCP (connection-oriented)
     0           -> Default protocol
    ========================================================
    */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
    perror("Socket failed");
    exit(1);
    }   

    g_server_fd = server_fd;


    /*
    ========================================================
     STEP 3: Enable Port Reuse
    ========================================================

     SO_REUSEADDR prevents:
       "Address already in use" error
     when server restarts quickly.

    ========================================================
    */
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET,
               SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt failed");
    close(server_fd);
    exit(1);
}



    /*
    ========================================================
     STEP 4: Configure Server Address Structure
    ========================================================

     sin_family      -> IPv4
     sin_addr        -> INADDR_ANY (bind all interfaces)
     sin_port        -> Convert port to network byte order

    ========================================================
    */
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);


    /*
    ========================================================
     STEP 5: Bind Socket to Address and Port
    ========================================================

     Associates socket descriptor with:
       - IP address
       - TCP port

    ========================================================
    */
    if (bind(server_fd,
         (sockaddr*)&server_addr,
         sizeof(server_addr)) < 0) {
    perror("Bind failed");
    close(server_fd);
    exit(1);
}


    /*
    ========================================================
     STEP 6: Start Listening for Clients
    ========================================================

     Backlog value 10:
       Maximum number of pending connections
       allowed in queue before accept().

    ========================================================
    */
    if (listen(server_fd, 10) < 0) {
    perror("Listen failed");
    close(server_fd);
    exit(1);
}


    std::cout << "Server running on port "
              << port << std::endl;


    /*
    ========================================================
     STEP 7: Main Accept Loop
    ========================================================

     Continues while g_running is true.
     Controlled shutdown via SIGINT modifies flag.

    ========================================================
    */
    while (g_running) {

        int client_fd = accept(server_fd, nullptr, nullptr);

        /*
         If shutdown occurred while waiting in accept(),
         break the loop.
        */
        if (!g_running)
            break;

        /*
         If accept fails, skip iteration.
        */
        if (client_fd < 0) {
           perror("Accept failed");
           continue;   // don't kill server
        }

        Logger::getInstance()->log(INFO,
                           "CONNECTION",
                           "Client connected. FD = " + std::to_string(client_fd));



        /*
        ====================================================
         STEP 8: Fork Child Process for Client
        ====================================================

         fork() creates duplicate process.

           pid == 0  -> Child process
           pid > 0   -> Parent process

        ====================================================
        */
    
        pid_t pid = fork();

        if (pid < 0) {
          perror("Fork failed");
          close(client_fd);
          continue;
        }


        /*
        ----------------------------------------------------
         CHILD PROCESS
        ----------------------------------------------------
         - Closes listening socket
         - Handles client session
         - Terminates after session ends
        ----------------------------------------------------
        */
        if (pid == 0) {

            close(server_fd);

            ClientHandler handler(client_fd);
            handler.handle();

            Logger::getInstance()->log(INFO,
                           "CONNECTION",
                           "Client session ended");

            close(client_fd);
            exit(0);
        }


        /*
        ----------------------------------------------------
         PARENT PROCESS
        ----------------------------------------------------
         - Closes client socket copy
         - Continues accepting new clients
         - Cleans up finished child processes
        ----------------------------------------------------
        */
        else {

            close(client_fd);

            /*
             Remove zombie processes using non-blocking wait.
            */
            while (waitpid(-1, nullptr,
                           WNOHANG) > 0);
        }
    }


    /*
    ========================================================
     STEP 9: Final Cleanup
    ========================================================

     Wait for any remaining child processes.
     Ensure no zombie processes remain.
    ========================================================
    */
    while (wait(nullptr) > 0);


    std::cout << "Server terminated cleanly."
              << std::endl;
}
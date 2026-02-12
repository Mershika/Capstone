/*
============================================================
 File Name    : ExceptionHandler.cpp
 Module       : Centralized Exception Handling System
 Description  : Implements error handling and reporting
                mechanism for the DCDIU system.

 Purpose:
  Provides standardized error reporting across all modules
  by combining contextual information with system error
  codes and messages.

 Functionality:
  - Retrieve system error information (errno)
  - Convert error codes to human-readable messages
  - Log errors with context information
  - Maintain consistent error reporting format

 Usage Pattern:
  When an operation fails (e.g., file open):
  1. Call ExceptionHandler::handle("description")
  2. Handler retrieves errno value
  3. Handler converts to readable error message
  4. Handler logs message with context
  5. Flow can continue or throw exception

 Integration:
  Called from multiple modules:
  - DirectoryTraverser (directory access errors)
  - ContentScanner (file access errors)
  - FileInspector (file read errors)
  - ClientHandler (general errors)

============================================================
*/

#include <ExceptionHandler/ExceptionHandler.h>
#include <Logger/Logger.h>
#include <cerrno>
#include <cstring>

using namespace std;

/*
================================================================
   Function Name : handle()
   
   Description:
   Centralized exception handler that reports errors
   with system-level context information.
   
   Implementation Strategy:
   - Retrieves current value of errno (system error code)
   - Converts errno to human-readable message using strerror()
   - Combines context message with system error info
   - Logs combined message at WARNING level
   
   Parameter:
   - context : Descriptive string indicating where error occurred
              Examples:
              "Cannot open users.txt"
              "Directory traversal failed"
              "Socket send operation"
   
   Return:
   void (error is logged, execution continues)
   
   Typical Use Cases:
   
   1. File Operation Failure:
      ─────────────────────
      if (open(filename) < 0)
          ExceptionHandler::handle("Cannot open file: " + filename);
      
      Output:
      [TIMESTAMP] [EXCEPTION] [WARNING] [file:line] [func]
      Cannot open file: /path/file | Error: Permission denied
   
   2. Socket Error:
      ──────────────
      if (send(sock, data, size, 0) < 0)
          ExceptionHandler::handle("Socket send failed");
      
      Output:
      [TIMESTAMP] [EXCEPTION] [WARNING] [file:line] [func]
      Socket send failed | Error: Broken pipe
   
   3. Directory Access:
      ──────────────────
      if (opendir(path) == NULL)
          ExceptionHandler::handle("Cannot open directory");
      
      Output:
      [TIMESTAMP] [EXCEPTION] [WARNING] [file:line] [func]
      Cannot open directory | Error: No such file or directory
   
   Error Code Meanings (Common):
   - EACCES (13) : Permission denied
   - ENOENT (2)  : No such file or directory
   - EISDIR (21) : Is a directory (expected regular file)
   - EBADF (9)   : Bad file descriptor
   - EPIPE (32)  : Broken pipe
   - ENOMEM (12) : Out of memory
   - EINVAL (22) : Invalid argument
   
   Error Logging Level:
   All errors logged at WARNING level (level 2)
   This allows filtering by log level configuration.
   
   Thread Safety:
   Current implementation reads errno which is usually
   thread-local in modern systems.
   
================================================================
*/
void ExceptionHandler::handle(const string& context) {

    /*
     * Retrieve human-readable error message
     * associated with current errno value.
     * 
     * strerror() is C library function that returns
     * pointer to static string (valid until next call).
     */
    const char* errorMsg = strerror(errno);

    /*
     * Construct complete error description
     * combining context with system error message.
     */
    string fullMessage = context + " | Error: " + errorMsg;

    /*
     * Log the error at WARNING severity.
     * This allows it to be filtered by log level.
     */
    LOG(
    WARNING,
    "EXCEPTION",
    fullMessage
);
}

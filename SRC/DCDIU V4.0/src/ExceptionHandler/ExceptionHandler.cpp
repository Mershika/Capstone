#include <ExceptionHandler/ExceptionHandler.h>
#include <Logger/Logger.h>
#include <cerrno>
#include <cstring>

using namespace std;


/*
 * ============================================================
 *  Exception Handling Module
 * ============================================================
 *  Provides centralized error handling and reporting.
 *
 *  This module is responsible for:
 *   - Capturing system-level error information
 *   - Formatting readable error messages
 *   - Logging warnings using Logger utility
 * ============================================================
 */


/*
 * ============================================================
 *  Function Name : handle()
 * ============================================================
 *  Description   : Handles system errors and logs them
 *                  with contextual information.
 *
 *  Input:
 *   - context : Description of where/why the error occurred
 *
 *  Output:
 *   - Writes formatted warning message to log file/system
 *
 *  Workflow:
 *   1. Retrieve last system error using errno
 *   2. Convert error code to readable message
 *   3. Append error info to context
 *   4. Send message to Logger
 *
 * ============================================================
 */
void ExceptionHandler::handle(const string& context) {

    /*
     * Retrieve human-readable error message
     * associated with the current errno value.
     */
    const char* errorMsg = strerror(errno);


    /*
     * Construct complete error description.
     */
    string fullMessage = context + " | Error: " + errorMsg;


    /*
     * Log the error as a warning message.
     */
    LOG(
    WARNING,
    "EXCEPTION",
    fullMessage
);
}

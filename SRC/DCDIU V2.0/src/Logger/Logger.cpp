#include <Logger/Logger.h>

#include <iostream>
#include <ctime>
#include <cstdlib>


using namespace std;


/*
 * ============================================================
 *  Logger Implementation
 * ============================================================
 *  Provides centralized logging functionality for the
 *  DCDIU system.
 *
 *  Features:
 *   - Severity-based logging
 *   - Timestamped log entries
 *   - Immediate termination on FATAL errors
 *
 *  This module ensures consistent diagnostic output
 *  across all system components.
 * ============================================================
 */


/*
 * ============================================================
 *  Log Function
 * ============================================================
 *  Logs a message with the specified severity level.
 *
 *  Responsibilities:
 *   - Map LogLevel enum to readable string
 *   - Attach current timestamp
 *   - Print structured log entry
 *   - Terminate program if severity is FATAL
 *
 *  Format:
 *   [LEVEL] TIMESTAMP  message
 *
 * ============================================================
 */
void Logger::log(LogLevel level, const string& message) {

    /*
     * --------------------------------------------------------
     *  Severity Mapping
     * --------------------------------------------------------
     */
    const char* levelStr[] = {
        "FATAL",
        "INFO",
        "WARNING",
        "DEBUG"
    };


    /*
     * --------------------------------------------------------
     *  Timestamp Retrieval
     * --------------------------------------------------------
     */
    time_t now = time(nullptr);


    /*
     * --------------------------------------------------------
     *  Log Output
     * --------------------------------------------------------
     */
    cout << "["
         << levelStr[(int)level]
         << "] "
         << ctime(&now)
         << "  "
         << message
         << endl;


    /*
     * --------------------------------------------------------
     *  Fatal Error Handling
     * --------------------------------------------------------
     *  Immediately terminate application if
     *  severity level is FATAL.
     * --------------------------------------------------------
     */
    if (level == LogLevel::FATAL) {
        exit(EXIT_FAILURE);
    }
}
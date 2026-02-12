/*
============================================================
 File Name    : Logger.h
 Module       : Logging Module
 Description  : Declares the Logger class and LogLevel
                enumeration used for standardized logging
                across the DCDIU system.

 Responsibilities:
  - Provide centralized logging mechanism
  - Support multiple log severity levels
  - Improve debugging and monitoring capability
  - Ensure consistent log formatting across modules

============================================================
*/

#ifndef LOGGER_H
#define LOGGER_H

#include <string>

using namespace std;

/*
------------------------------------------------------------
 Enum Name     : LogLevel
 Description   : Defines severity levels for log messages
                within the DCDIU system.

 Levels:
  - FATAL    : Critical system errors requiring immediate action
  - WARNING  : Potential issues that may affect functionality
  - INFO     : Informational runtime messages
  - DEBUG    : Detailed diagnostic information for development
------------------------------------------------------------
*/
enum class LogLevel {
    FATAL,
    INFO,
    WARNING,
    DEBUG
};

/*
------------------------------------------------------------
 Class Name    : Logger
 Description   : Provides static utility functionality for
                logging system events, errors, warnings,
                and debug information.

                Ensures consistent logging structure
                throughout the DCDIU architecture.
------------------------------------------------------------
*/
class Logger {

public:

    /*
     * --------------------------------------------------------
     * Function Name : log()
     * --------------------------------------------------------
     * Description   : Logs a message with the specified
     *                severity level.
     *
     *                Can be used by any module to record
     *                runtime events or error conditions.
     *
     * @param level    Severity level of the log entry.
     * @param message  Log message content.
     *
     * Return          : void
     *
     * Note:
     * This method is static and does not require
     * instantiation of Logger.
     * --------------------------------------------------------
     */
    static void log(LogLevel level,
                    const string& message);
};

#endif  // LOGGER_H
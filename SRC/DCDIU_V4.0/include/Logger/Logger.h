/*
============================================================
 File Name    : Logger.h
 Module       : Logging and Tracing Module
 Description  : Declares the Logger singleton class for
                centralized event logging, debugging, and
                performance tracing across the DCDIU system.

 Responsibilities:
  - Provide singleton logger instance
  - Support multiple log levels (FATAL, INFO, WARNING, DEBUG)
  - Write formatted logs to file and console
  - Include file location, line number, and function context
  - Capture timestamps for each log entry
  - Allow dynamic log level configuration
  - Allow runtime log file specification

 Design Pattern:
  - Singleton (only one Logger instance)
  - Deleted copy constructor and assignment operator
  - Resource management via RAII pattern

 Supported Log Levels:
  - FATAL   (0): Critical errors requiring immediate action
  - INFO    (1): General informational messages
  - WARNING (2): Potential issues or unexpected behavior
  - DEBUG   (3): Detailed debugging information

 Usage Example:
  LOG(INFO, "MODULE_NAME", "Operation completed successfully");

============================================================
*/

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <ctime>
#include <sstream>
#include <iostream>

using namespace std;

/*
 * ============================================================
 * Log Level Enumeration
 * ============================================================
 * Defines the severity levels for log messages.
 * Lower numeric value = Higher severity
 * ============================================================
 */
enum LogLevel {
    FATAL = 0,      /* Critical errors - system may fail */
    INFO = 1,       /* Informational messages */
    WARNING = 2,    /* Warning messages - potential issues */
    DEBUG = 3       /* Detailed debugging information */
};

/*
------------------------------------------------------------
 Class Name    : Logger
 Description   : Singleton utility class that provides
                thread-safe (non-concurrent) logging
                functionality across the DCDIU system.

                Each log entry includes:
                  - Timestamp (YYYY-MM-DD HH:MM:SS)
                  - Severity level
                  - Operation category
                  - Source file and line number
                  - Function name
                  - Log message

                Logs are output to both:
                  - Console (stdout)
                  - Log file (configurable path)
------------------------------------------------------------
*/
class Logger {

private:

    /*
     * Singleton instance pointer.
     * Holds the unique Logger object.
     */
    static Logger* instance;

    /*
     * File output stream for writing logs to file.
     * Opened in append mode.
     */
    ofstream logFile;

    /*
     * Current log level threshold.
     * Only messages at this level or higher severity
     * (lower numeric value) are actually logged.
     */
    LogLevel currentLevel;

    /*
     * Path to the current log file.
     * Defaults to "logs/default.log".
     */
    string logFilePath;

    /*
     * ========================================================
     * Private Constructor
     * ========================================================
     * Prevents instantiation from outside.
     * Initializes log level to INFO and opens default log file.
     * ========================================================
     */
    Logger();

    /*
     * ========================================================
     * Private Destructor
     * ========================================================
     * Closes the log file stream properly.
     * Called only when the singleton is destroyed.
     * ========================================================
     */
    ~Logger();

    /*
     * ========================================================
     * Function Name : getCurrentTimestamp()
     * ========================================================
     * Description   : Generates current date and time string
     *                in "YYYY-MM-DD HH:MM:SS" format.
     *
     * Return        : String containing formatted timestamp.
     * ========================================================
     */
    string getCurrentTimestamp();

    /*
     * ========================================================
     * Function Name : levelToString()
     * ========================================================
     * Description   : Converts LogLevel enum to readable
     *                string representation.
     *
     * @param level : LogLevel enum value to convert.
     *
     * Return         : String representation of log level.
     * ========================================================
     */
    string levelToString(LogLevel level);

public:

    /*
     * ========================================================
     * Function Name : getInstance()
     * ========================================================
     * Description   : Returns reference to the singleton
     *                Logger instance.
     *
     *                Creates the instance on first call.
     *                Subsequent calls return existing instance.
     *
     * Return        : Pointer to singleton Logger object.
     * ========================================================
     */
    static Logger* getInstance();

    /*
     * ========================================================
     * Function Name : setLogLevel()
     * ========================================================
     * Description   : Sets the minimum log level threshold.
     *
     *                Only messages with severity equal to
     *                or greater than this level will be logged.
     *
     * @param level : LogLevel threshold (FATAL, INFO, WARNING, DEBUG).
     * ========================================================
     */
    void setLogLevel(LogLevel level);

    /*
     * ========================================================
     * Function Name : setLogFile()
     * ========================================================
     * Description   : Changes the output log file path.
     *
     *                Closes current log file and opens new one.
     *                Useful for switching between component-specific
     *                log files.
     *
     * @param filePath : Path to new log file.
     * ========================================================
     */
    void setLogFile(const string& filePath);

    /*
     * ========================================================
     * Function Name : log()
     * ========================================================
     * Description   : Formats and writes a log entry with
     *                full contextual information.
     *
     *                Generally called via LOG() macro which
     *                automatically provides file, line, and
     *                function information.
     *
     * @param level     : Severity level of message.
     * @param operation : Category/module name (e.g., "AUTH", "TRAVERSE").
     * @param message   : Descriptive log message.
     * @param file      : Source code filename (__FILE__).
     * @param line      : Source code line number (__LINE__).
     * @param function  : Function name (__FUNCTION__).
     * ========================================================
     */
    void log(LogLevel level,
         const std::string& operation,
         const std::string& message,
         const char* file,
         int line,
         const char* function);

    /* Delete copy constructor to enforce singleton pattern */
    Logger(const Logger&) = delete;

    /* Delete assignment operator to enforce singleton pattern */
    Logger& operator=(const Logger&) = delete;
};

/*
 * ============================================================
 * LOG Macro
 * ============================================================
 * Convenient wrapper macro for logging messages.
 * Automatically captures file, line, and function information.
 *
 * Usage: LOG(level, operation, message)
 *
 * Example:
 *   LOG(INFO, "STARTUP", "Application initialized");
 * ============================================================
 */
#define LOG(level, operation, message) \
    Logger::getInstance()->log(level, operation, message, __FILE__, __LINE__, __FUNCTION__)

#endif  // LOGGER_H

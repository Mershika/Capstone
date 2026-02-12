/*
============================================================
 File Name    : Logger.cpp
 Module       : Logging and Tracing Module
 Description  : Implements the singleton Logger class for
                centralized system-wide event logging.

 Implementation Details:
  - Singleton pattern with static instance pointer
  - File I/O in append mode (non-destructive)
  - Timestamp generation for each log entry
  - Formatted output to both console and file
  - Configurable log levels for filtering

 Thread Safety Note:
  Current implementation is NOT thread-safe for
  multi-threaded environments. For concurrent access,
  consider adding mutex locks.

============================================================
*/

#include <Logger/Logger.h>

using namespace std;

/*
 * ============================================================
 * Static Member Initialization
 * ============================================================
 * Initialize the singleton instance pointer to nullptr.
 * The actual instance is created on first call to
 * getInstance().
 * ============================================================
 */
Logger* Logger::instance = nullptr;

/*
------------------------------------------------------------
 Constructor Implementation
------------------------------------------------------------

 Description:
   Initializes the Logger singleton instance with
   default configuration.

 Actions:
   - Set log level to INFO (medium verbosity)
   - Set default log file path
   - Open log file in append mode

 Notes:
   - Private to prevent direct instantiation
   - Only accessed via getInstance()

------------------------------------------------------------
*/
Logger::Logger()
    : currentLevel(INFO),
      logFilePath("logs/default.log") {

    /* Open log file in append mode to preserve history */
    logFile.open(logFilePath, ios::app);
}

/*
------------------------------------------------------------
 Destructor Implementation
------------------------------------------------------------

 Description:
   Properly closes the log file stream.

 Actions:
   - Check if file stream is open
   - Close the stream if open

 Notes:
   - Called only when the Logger singleton is destroyed
   - Usually at program termination

------------------------------------------------------------
*/
Logger::~Logger() {
    if (logFile.is_open())
        logFile.close();
}

/*
------------------------------------------------------------
 Function Name : getInstance()
------------------------------------------------------------

 Description:
   Returns the singleton Logger instance.

 Implementation:
   - First call: Allocates new Logger object
   - Subsequent calls: Returns existing instance
   - Thread-safe in single-threaded context
   - NOT thread-safe in multi-threaded environment

 Return:
   Pointer to the static Logger instance.

------------------------------------------------------------
*/
Logger* Logger::getInstance() {

    /* Lazy initialization: create instance on first use */
    if (!instance)
        instance = new Logger();

    return instance;
}

/*
------------------------------------------------------------
 Function Name : setLogLevel()
------------------------------------------------------------

 Description:
   Modifies the current log level threshold.

 Behavior:
   Only log entries with severity <= currentLevel
   will be actually logged.

 Parameter:
   level -> New LogLevel threshold

 Example:
   setLogLevel(DEBUG)
    -> Both WARNING and DEBUG messages logged
   setLogLevel(INFO)
    -> Only FATAL and INFO messages logged

------------------------------------------------------------
*/
void Logger::setLogLevel(LogLevel level) {
    currentLevel = level;
}

/*
------------------------------------------------------------
 Function Name : setLogFile()
------------------------------------------------------------

 Description:
   Changes the output log file path and reopens file.

 Behavior:
   - Closes current log file (if open)
   - Updates internal file path
   - Opens new log file in append mode

 Parameter:
   filePath -> Path to new log file

 Use Case:
   Different components may want separate log files
   (e.g., client.log, server.log, auth.log)

------------------------------------------------------------
*/
void Logger::setLogFile(const string& filePath) {

    /* Close existing log file */
    if (logFile.is_open())
        logFile.close();

    /* Update path and open new file */
    logFilePath = filePath;
    logFile.open(logFilePath, ios::app);
}

/*
------------------------------------------------------------
 Function Name : getCurrentTimestamp()
------------------------------------------------------------

 Description:
   Generates a formatted timestamp string of current
   date and time.

 Return:
   String in format: "YYYY-MM-DD HH:MM:SS"

 Implementation:
   - Uses C time library (time.h)
   - Calls time() to get current time
   - Uses localtime() to convert to struct tm
   - Formats using strftime()

------------------------------------------------------------
*/
string Logger::getCurrentTimestamp() {

    /* Get current time as seconds since epoch */
    time_t now = time(nullptr);

    /* Buffer for formatted time string */
    char buffer[80];

    /* Convert to local time structure */
    struct tm* timeinfo = localtime(&now);

    /* Format time string according to pattern */
    strftime(buffer,
             sizeof(buffer),
             "%Y-%m-%d %H:%M:%S",
             timeinfo);

    return string(buffer);
}

/*
------------------------------------------------------------
 Function Name : levelToString()
------------------------------------------------------------

 Description:
   Converts LogLevel enum to human-readable string.

 Parameter:
   level -> LogLevel enum value

 Return:
   String representation ("FATAL", "INFO", etc.)

------------------------------------------------------------
*/
string Logger::levelToString(LogLevel level) {

    switch (level) {
        case FATAL:   return "FATAL";
        case INFO:    return "INFO";
        case WARNING: return "WARNING";
        case DEBUG:   return "DEBUG";
        default:      return "UNKNOWN";
    }
}

/*
------------------------------------------------------------
 Function Name : log()
------------------------------------------------------------

 Description:
   Core logging function that formats and outputs
   log entries.

 Behavior:
   - Checks if message severity warrants logging
   - Formats message with timestamp and context
   - Writes to log file (if open)
   - Outputs to console (stdout)

 Parameters:
   level      -> Severity level of message
   operation  -> Module/operation name for categorization
   message    -> The actual log message
   file       -> Source code filename (__FILE__)
   line       -> Source code line number (__LINE__)
   function   -> Function name (__FUNCTION__)

 Log Format:
   [TIMESTAMP] [OPERATION] [LEVEL] [FILE:LINE] [FUNCTION] MESSAGE

 Example Output:
   [2026-02-12 14:30:45] [AUTH] [INFO] [Server.cpp:145] [start] Connection accepted

------------------------------------------------------------
*/
void Logger::log(LogLevel level,
                 const string& operation,
                 const string& message,
                 const char* file,
                 int line,
                 const char* function) {

    /* Only log if message severity meets threshold */
    if (level <= currentLevel) {

        /* Construct formatted log entry */
        string entry =
            "[" + getCurrentTimestamp() + "] "
            "[" + operation + "] "
            "[" + levelToString(level) + "] "
            "[" + file + ":" + to_string(line) + "] "
            "[" + function + "] "
            + message;

        /* Write to log file if open */
        if (logFile.is_open())
            logFile << entry << endl;

        /* Always output to console */
        cout << entry << endl;
    }
}
#include <Logger/Logger.h>

using namespace std;

Logger* Logger::instance = nullptr;

Logger::Logger()
    : currentLevel(INFO),
      logFilePath("logs/default.log") {

    logFile.open(logFilePath, ios::app);
}

Logger::~Logger() {
    if (logFile.is_open())
        logFile.close();
}

Logger* Logger::getInstance() {

    if (!instance)
        instance = new Logger();

    return instance;
}

void Logger::setLogLevel(LogLevel level) {
    currentLevel = level;
}

void Logger::setLogFile(const string& filePath) {

    if (logFile.is_open())
        logFile.close();

    logFilePath = filePath;
    logFile.open(logFilePath, ios::app);
}

string Logger::getCurrentTimestamp() {

    time_t now = time(nullptr);
    char buffer[80];

    struct tm* timeinfo = localtime(&now);

    strftime(buffer,
             sizeof(buffer),
             "%Y-%m-%d %H:%M:%S",
             timeinfo);

    return string(buffer);
}

string Logger::levelToString(LogLevel level) {

    switch (level) {
        case FATAL:   return "FATAL";
        case INFO:    return "INFO";
        case WARNING: return "WARNING";
        case DEBUG:   return "DEBUG";
        default:      return "UNKNOWN";
    }
}

void Logger::log(LogLevel level,
                 const string& operation,
                 const string& message,
                 const char* file,
                 int line,
                 const char* function) {

    if (level <= currentLevel) {

        string entry =
            "[" + getCurrentTimestamp() + "] "
            "[" + operation + "] "
            "[" + levelToString(level) + "] "
            "[" + file + ":" + to_string(line) + "] "
            "[" + function + "] "
            + message;

        if (logFile.is_open())
            logFile << entry << endl;

        cout << entry << endl;
    }
}
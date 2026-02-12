#include <Logger/Logger.h>

Logger* Logger::instance = nullptr;

Logger::Logger()
    : currentLevel(INFO),
      logFilePath("logs/default.log") {

    logFile.open(logFilePath, std::ios::app);
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

void Logger::setLogFile(const std::string& filePath) {

    if (logFile.is_open())
        logFile.close();

    logFilePath = filePath;
    logFile.open(logFilePath, std::ios::app);
}

std::string Logger::getCurrentTimestamp() {

    time_t now = time(nullptr);
    char buffer[80];

    struct tm* timeinfo = localtime(&now);

    strftime(buffer,
             sizeof(buffer),
             "%Y-%m-%d %H:%M:%S",
             timeinfo);

    return std::string(buffer);
}

std::string Logger::levelToString(LogLevel level) {

    switch (level) {
        case FATAL:   return "FATAL";
        case INFO:    return "INFO";
        case WARNING: return "WARNING";
        case DEBUG:   return "DEBUG";
        default:      return "UNKNOWN";
    }
}

void Logger::log(LogLevel level,
                 const std::string& operation,
                 const std::string& message) {

    if (level <= currentLevel) {

        std::string entry =
            "[" + getCurrentTimestamp() + "] "
            "[" + operation + "] "
            "[" + levelToString(level) + "] "
            + message;

        if (logFile.is_open())
            logFile << entry << std::endl;

        std::cout << entry << std::endl;
    }
}

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <ctime>
#include <sstream>
#include <iostream>

enum LogLevel {
    FATAL = 0,
    INFO = 1,
    WARNING = 2,
    DEBUG = 3
};

class Logger {

private:
    static Logger* instance;

    std::ofstream logFile;
    LogLevel currentLevel;
    std::string logFilePath;

    Logger();
    ~Logger();

    std::string getCurrentTimestamp();
    std::string levelToString(LogLevel level);

public:
    static Logger* getInstance();

    void setLogLevel(LogLevel level);
    void setLogFile(const std::string& filePath);

    void log(LogLevel level,
             const std::string& operation,
             const std::string& message);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

#endif

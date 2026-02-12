#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <ctime>
#include <sstream>
#include <iostream>

using namespace std;

enum LogLevel {
    FATAL = 0,
    INFO = 1,
    WARNING = 2,
    DEBUG = 3
};

class Logger {

private:
    static Logger* instance;

    ofstream logFile;
    LogLevel currentLevel;
    string logFilePath;

    Logger();
    ~Logger();

    string getCurrentTimestamp();
    string levelToString(LogLevel level);

public:
    static Logger* getInstance();

    void setLogLevel(LogLevel level);
    void setLogFile(const string& filePath);

    void log(LogLevel level,
         const std::string& operation,
         const std::string& message,
         const char* file,
         int line,
         const char* function);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

#define LOG(level, operation, message) \
    Logger::getInstance()->log(level, operation, message, __FILE__, __LINE__, __FUNCTION__)

#endif

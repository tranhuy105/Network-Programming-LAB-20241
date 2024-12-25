#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

class Logger {
public:
    enum LogLevel {
        INFO,
        WARN,
        ERROR,
        DEBUG
    };

    explicit Logger(const std::string &logFile);
    ~Logger();

    void logEvent(const std::string &deviceId, const std::string &message, LogLevel level = INFO);
    void logError(const std::string &deviceId, const std::string &message);
    void logInfo(const std::string &deviceId, const std::string &message);
    void logWarn(const std::string &deviceId, const std::string &message);
    void logDebug(const std::string &deviceId, const std::string &message);

private:
    std::ofstream logStream;
    std::mutex logMutex;

    std::string formatLog(const std::string &deviceId, const std::string &message, LogLevel level);
    std::string logLevelToString(LogLevel level);
};

#endif

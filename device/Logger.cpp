#include "Logger.h"
#include <ctime>
#include <iostream>
#include <iomanip>

Logger::Logger(const std::string &logFile) {
    logStream.open(logFile, std::ios::app);
    if (!logStream.is_open()) {
        throw std::runtime_error("Failed to open log file: " + logFile);
    }
}

Logger::~Logger() {
    if (logStream.is_open()) {
        logStream.close();
    }
}

void Logger::logEvent(const std::string &deviceId, const std::string &message, LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    logStream << formatLog(deviceId, message, level) << std::endl;
}

void Logger::logError(const std::string &deviceId, const std::string &message) {
    logEvent(deviceId, message, ERROR);
}

void Logger::logInfo(const std::string &deviceId, const std::string &message) {
    logEvent(deviceId, message, INFO);
}

void Logger::logWarn(const std::string &deviceId, const std::string &message) {
    logEvent(deviceId, message, WARN);
}

void Logger::logDebug(const std::string &deviceId, const std::string &message) {
    logEvent(deviceId, message, DEBUG);
}

std::string Logger::formatLog(const std::string &deviceId, const std::string &message, LogLevel level) {
    std::ostringstream logLine;
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);

    logLine << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] ";
    logLine << "[" << logLevelToString(level) << "] ";
    logLine << "[" << deviceId << "] " << message;

    return logLine.str();
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case INFO: return "INFO";
        case WARN: return "WARN";
        case ERROR: return "ERROR";
        case DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

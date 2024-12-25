#ifndef DEVICE_H
#define DEVICE_H

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "TimerManager.h"
#include "RuntimeTracker.h"
#include "AuthenticationManager.h"
#include "Logger.h"
#include "../lib/json.hpp"

class Device {
protected:
    std::string id;
    int powerConsumption = 0;
    bool state = false;

    // Components
    TimerManager timerManager;
    RuntimeTracker runtimeTracker;
    Logger logger;

public:
    explicit Device(const std::string& id, const std::string& password);
    ~Device();

    virtual void turnOn();
    virtual void turnOff();
    virtual std::string getType() const = 0;
    
    void setTimer(int duration, const std::string& action);
    void cancelAllTimers();

    std::string getId() const { return id; }
    nlohmann::json getInfo() const;
    virtual nlohmann::json getDetailedInfo() const;
    Logger& getLogger() { return logger; }
};

#endif

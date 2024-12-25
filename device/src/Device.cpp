#include "../include/Device.h"
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

Device::Device(const std::string &id, const std::string &password)
    : id(id), logger("log/device_" + id + ".log") {
    // Register callback to TimerManager
    timerManager.registerCallback([this](const std::string& action) {
        try {
            if (action == "turn_on") {
                this->turnOn();
            } else if (action == "turn_off") {
                this->turnOff();
            } else {
                logger.logError(this->id, "Unsupported timer action: " + action);
            }
        } catch (const std::exception& e) {
            logger.logError(this->id, "Timer action failed: " + std::string(e.what()));
        }
    });

}

Device::~Device() = default;

void Device::turnOn() {
    if (state) throw std::runtime_error("Device is already on.");
    state = true;
    powerConsumption = 10;
    runtimeTracker.startTimer(powerConsumption);
    logger.logEvent(id, "Device turned on.");
}

void Device::turnOff() {
    if (!state) throw std::runtime_error("Device is already off.");
    state = false;
    runtimeTracker.stopTimer();
    powerConsumption = 0;
    logger.logEvent(id, "Device turned off.");
}

void Device::setTimer(int duration, const std::string& action) {
    timerManager.setTimer(duration, action);
    logger.logEvent(id, "Timer set for " + std::to_string(duration) + " seconds to execute: " + action);
}

void Device::cancelAllTimers() {
    timerManager.cancelAllTimers();
    logger.logEvent(id, "All timers canceled.");
}

json Device::getInfo() const {
    return {
        {"id", id},
        {"state", state ? "on" : "off"},
        {"power", powerConsumption}
    };
}

json Device::getDetailedInfo() const {
    return {
        {"id", id},
        {"state", state ? "on" : "off"},
        {"power", powerConsumption},
        {"cumulative_power", runtimeTracker.getCumulativePowerConsumption()},
        {"runtime", {
            {"daily", runtimeTracker.getDailyRuntime()},
            {"monthly", runtimeTracker.getMonthlyRuntime()},
            {"yearly", runtimeTracker.getYearlyRuntime()}
        }}
    };
}

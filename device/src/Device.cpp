#include "../include/Device.h"
#include <stdexcept>
#include <iostream>

// Device.cpp: Implements core functionalities of a smart device, including power management,
// runtime tracking, and timers for scheduled actions.

using json = nlohmann::json;

// Constructor
// Initializes the device with a unique ID and a password.
// Registers timer callbacks to handle scheduled "turn on" or "turn off" actions.
Device::Device(const std::string &id, const std::string &password)
    : id(id), logger("log/device_" + id + ".log") {
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

// Destructor
// Ensures proper cleanup of device resources.
Device::~Device() = default;

// turnOn
// Changes the device state to "on" and starts runtime tracking.
// Throws an exception if the device is already on.
void Device::turnOn() {
    if (state) throw std::runtime_error("Device is already on.");
    state = true;
    powerConsumption = 10; // Example power consumption in watts.
    runtimeTracker.startTimer(powerConsumption);
    logger.logEvent(id, "Device turned on.");
}

// turnOff
// Changes the device state to "off" and stops runtime tracking.
// Throws an exception if the device is already off.
void Device::turnOff() {
    if (!state) throw std::runtime_error("Device is already off.");
    state = false;
    runtimeTracker.stopTimer();
    powerConsumption = 0;
    logger.logEvent(id, "Device turned off.");
}

// setTimer
// Schedules a "turn on" or "turn off" action after the specified duration.
// Logs the timer configuration.
void Device::setTimer(int duration, const std::string& action) {
    timerManager.setTimer(duration, action);
    logger.logEvent(id, "Timer set for " + std::to_string(duration) + " seconds to execute: " + action);
}

// cancelAllTimers
// Cancels all active timers for the device.
// Logs the cancellation event.
void Device::cancelAllTimers() {
    timerManager.cancelAllTimers();
    logger.logEvent(id, "All timers canceled.");
}

// getInfo
// Returns basic information about the device (ID, state, power consumption) as a JSON object.
json Device::getInfo() const {
    return {
        {"id", id},
        {"state", state ? "on" : "off"},
        {"power", powerConsumption}
    };
}

// getDetailedInfo
// Returns detailed information about the device, including runtime statistics and cumulative power consumption.
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

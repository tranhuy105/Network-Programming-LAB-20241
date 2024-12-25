#include "../include/AC.h"
#include <stdexcept>

AC::AC(const std::string& id, const std::string& password)
    : Device(id, password) {}

void AC::turnOn() {
    if (state) throw std::runtime_error("AC is already on.");
    state = true;
    powerConsumption = 100;
    runtimeTracker.startTimer(powerConsumption);
}

void AC::turnOff() {
    if (!state) throw std::runtime_error("AC is already off.");
    state = false;
    powerConsumption = 0;
    runtimeTracker.stopTimer();
}

void AC::setMode(ACMode newMode) {
    mode = newMode;
}

void AC::setTemperature(int newTemperature) {
    if (newTemperature < 18 || newTemperature > 30) throw std::runtime_error("Temperature out of range.");
    temperature = newTemperature;
}

ACMode AC::getMode() const {
    return mode;
}

int AC::getTemperature() const {
    return temperature;
}

#include "../include/Fan.h"
#include <stdexcept>

Fan::Fan(const std::string& id, const std::string& password)
    : Device(id, password) {}

void Fan::turnOn() {
    if (state) throw std::runtime_error("Fan is already on.");
    state = true;
    if (speed == 0) speed = 1;
    powerConsumption = 20 * speed;
    runtimeTracker.startTimer(powerConsumption);
}

void Fan::turnOff() {
    if (!state) throw std::runtime_error("Fan is already off.");
    state = false;
    powerConsumption = 0;
    runtimeTracker.stopTimer();
}

void Fan::setSpeed(int newSpeed) {
    if (newSpeed < 0 || newSpeed > 3) throw std::runtime_error("Invalid speed level.");
    speed = newSpeed;
    if (state) {
        runtimeTracker.stopTimer();
        powerConsumption = 20 * speed;
        runtimeTracker.startTimer(powerConsumption);
    }
}

int Fan::getSpeed() const {
    return speed;
}

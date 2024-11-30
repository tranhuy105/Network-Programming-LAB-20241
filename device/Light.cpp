#include "Light.h"
#include <stdexcept>

Light::Light(const std::string& id, const std::string& password)
    : Device(id, password) {}

void Light::turnOn() {
    if (state) throw std::runtime_error("Light is already on.");
    state = true;
    powerConsumption = 10; // Example power usage
}

void Light::turnOff() {
    if (!state) throw std::runtime_error("Light is already off.");
    state = false;
    powerConsumption = 0;
}

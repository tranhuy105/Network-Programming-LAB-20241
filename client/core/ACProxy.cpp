#include "../include/ACProxy.h"
#include <stdexcept>

void ACProxy::setMode(ACMode mode) {
    std::string modeStr;
    switch (mode) {
        case ACMode::COOL: modeStr = "cool"; break;
        case ACMode::HEAT: modeStr = "heat"; break;
        case ACMode::DRY: modeStr = "dry"; break;
        default: throw std::invalid_argument("Invalid AC mode.");
    }

    nlohmann::json request = {
        {"action", "set_mode"},
        {"token", token},
        {"clientId", getClientId()},
        {"mode", modeStr}
    };

    nlohmann::json response = sendRequest(request);
    if (response["status"] != 200) {
        throw std::runtime_error(response["message"]);
    }
}

void ACProxy::setTemperature(int temperature) {
    if (temperature < 18 || temperature > 30) {
        throw std::invalid_argument("Invalid temperature. Must be between 18 and 30.");
    }

    nlohmann::json request = {
        {"action", "set_temperature"},
        {"token", token},
        {"clientId", getClientId()},
        {"temperature", temperature}
    };

    nlohmann::json response = sendRequest(request);
    if (response["status"] != 200) {
        throw std::runtime_error(response["message"]);
    }
}

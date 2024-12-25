#include "../include/FanProxy.h"
#include <stdexcept>

void FanProxy::setSpeed(int speed) {
    if (speed < 1 || speed > 3) {
        throw std::invalid_argument("Invalid fan speed. Must be between 1 and 3.");
    }

    nlohmann::json request = {
        {"action", "set_speed"},
        {"token", token},
        {"clientId", getClientId()},
        {"speed", speed}
    };

    nlohmann::json response = sendRequest(request);
    if (response["status"] != 200) {
        throw std::runtime_error(response["message"]);
    }
}

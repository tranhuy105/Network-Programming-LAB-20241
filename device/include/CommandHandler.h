#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "Device.h"
#include "Light.h"
#include "Fan.h"
#include "AC.h"
#include "AuthenticationManager.h"
#include "../lib/json.hpp"

class CommandHandler {
private:
    std::shared_ptr<Device> device;
    AuthenticationManager authManager;
public:
    CommandHandler(std::shared_ptr<Device> device, const std::string& password);
    nlohmann::json handleCommand(const nlohmann::json& command);
    std::shared_ptr<Device> getDevice() const {
        return device;
    }
};

#endif

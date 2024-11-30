#include "CommandHandler.h"
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

CommandHandler::CommandHandler(std::shared_ptr<Device> device, const std::string& password)
    : device(std::move(device)), authManager(password) {}

json CommandHandler::handleCommand(const json& commandJson) {
    try {
        std::string action = commandJson.at("action");

        // Handle authentication-related actions
        if (action == "authenticate") {
            return authManager.authenticate(commandJson);
        } else if (action == "validate_token") {
            return authManager.validateToken(commandJson);
        } else if (action == "change_password") {
            return authManager.changePassword(commandJson);
        }

        // Validate token before executing device-related commands
        auto validationResponse = authManager.validateToken(commandJson);
        if (validationResponse["status"] != 200) {
            return validationResponse;
        }

        // Generic device-related actions
        if (action == "status") {
            return {
                {"status", 200},
                {"message", "Status retrieved successfully"},
                {"data", device->getInfo()}
            };
        } else if (action == "details") {
            return {
                {"status", 200},
                {"message", "Detailed info retrieved successfully"},
                {"data", device->getDetailedInfo()}
            };
        } else if (action == "turn_on") {
            device->turnOn();
            return {
                {"status", 200},
                {"message", "Device turned on"}
            };
        } else if (action == "turn_off") {
            device->turnOff();
            return {
                {"status", 200},
                {"message", "Device turned off"}
            };
        } else if (action == "set_timer") {
            int duration = commandJson.at("duration");
            std::string timerAction = commandJson.at("timer_action");
            if (timerAction != "turn_on" && timerAction != "turn_off") {
                throw std::invalid_argument("Unsupported timer action: " + timerAction);
            }
            device->setTimer(duration, timerAction);
            return {
                {"status", 200},
                {"message", "Timer set successfully"}
            };
        } else if (action == "cancel_timers") {
            device->cancelAllTimers();
            return {
                {"status", 200},
                {"message", "All timers canceled"}
            };
        }

        // Device-specific actions
        if (auto fan = std::dynamic_pointer_cast<Fan>(device)) {
            if (action == "set_speed") {
                int speed = commandJson.at("speed");
                fan->setSpeed(speed);
                return {
                    {"status", 200},
                    {"message", "Fan speed set successfully"}
                };
            }
            throw std::invalid_argument("Unsupported action for Fan: " + action);
        } else if (auto ac = std::dynamic_pointer_cast<AC>(device)) {
            if (action == "set_mode") {
                std::string mode = commandJson.at("mode");
                if (mode == "cool") ac->setMode(ACMode::COOL);
                else if (mode == "heat") ac->setMode(ACMode::HEAT);
                else if (mode == "dry") ac->setMode(ACMode::DRY);
                else throw std::invalid_argument("Invalid AC mode: " + mode);

                return {
                    {"status", 200},
                    {"message", "AC mode set successfully"}
                };
            } else if (action == "set_temperature") {
                int temperature = commandJson.at("temperature");
                ac->setTemperature(temperature);
                return {
                    {"status", 200},
                    {"message", "AC temperature set successfully"}
                };
            }
            throw std::invalid_argument("Unsupported action for AC: " + action);
        } else if (auto light = std::dynamic_pointer_cast<Light>(device)) {
            // No additional Light-specific actions for now
            throw std::invalid_argument("Unsupported action for Light: " + action);
        }

        throw std::invalid_argument("Unsupported action: " + action);
    } catch (const std::exception& e) {
        return {
            {"status", 400},
            {"message", std::string("Error: ") + e.what()}
        };
    }
}

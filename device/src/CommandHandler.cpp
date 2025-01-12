#include "../include/CommandHandler.h"
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

CommandHandler::CommandHandler(std::shared_ptr<Device> device, const std::string& password)
    : device(std::move(device)), authManager(password) {}

json CommandHandler::handleCommand(const json& commandJson) {
    Logger& logger = device->getLogger();
    try {
        std::string action = commandJson.at("action");

        // Handle authentication-related actions
        if (action == "authenticate") {
            logger.logInfo(device->getId(), "Authenticate client: " + commandJson["clientId"].get<std::string>());
            json response = authManager.authenticate(commandJson);
            if (response["status"] == 200) {
                logger.logInfo(device->getId(), "Authentication successful for client: " + commandJson["clientId"].get<std::string>());
            } else {
                logger.logError(device->getId(), "Authentication failed for client: " + commandJson["clientId"].get<std::string>());
            }
            return response;
        } else if (action == "validate_token") {
            logger.logInfo(device->getId(), "Validating token for client: " + commandJson["clientId"].get<std::string>());
            return authManager.validateToken(commandJson);
        } else if (action == "change_password") {
            logger.logInfo(device->getId(), "Changing password for client: " + commandJson["clientId"].get<std::string>());
            return authManager.changePassword(commandJson);
        }

        // Validate token before executing device-related commands
        auto validationResponse = authManager.validateToken(commandJson);
        if (validationResponse["status"] != 200) {
            logger.logError(device->getId(), "Invalid or expired token for client: " + commandJson["clientId"].get<std::string>());
            return validationResponse;
        }

        // Generic device-related actions
        if (action == "status") {
            logger.logDebug(device->getId(), "Fetching status for device: " + device->getId());
            return {
                {"status", 200},
                {"message", "Status retrieved successfully"},
                {"data", device->getInfo()}
            };
        } else if (action == "details") {
            logger.logDebug(device->getId(), "Fetching detailed info for device: " + device->getId());
            return {
                {"status", 200},
                {"message", "Detailed info retrieved successfully"},
                {"data", device->getDetailedInfo()}
            };
        } else if (action == "turn_on") {
            logger.logInfo(device->getId(), "Turning device ON");
            device->turnOn();
            return {
                {"status", 200},
                {"message", "Device turned on"}
            };
        } else if (action == "turn_off") {
            logger.logInfo(device->getId(), "Turning device OFF");
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
            logger.logInfo(device->getId(), "Setting timer for " + std::to_string(duration) + " seconds with action: " + timerAction);
            device->setTimer(duration, timerAction);
            return {
                {"status", 200},
                {"message", "Timer set successfully"}
            };
        } else if (action == "cancel_timers") {
            logger.logInfo(device->getId(), "Canceling all timers for device: " + device->getId());
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
                logger.logInfo(device->getId(), "Setting fan speed to: " + std::to_string(speed));
                fan->setSpeed(speed);
                return {
                    {"status", 200},
                    {"message", "Fan speed set successfully"}
                };
            }
            logger.logError(device->getId(), "Unsupported action for Fan: " + action);
            throw std::invalid_argument("Unsupported action for Fan: " + action);
        } else if (auto ac = std::dynamic_pointer_cast<AC>(device)) {
            if (action == "set_mode") {
                std::string mode = commandJson.at("mode");
                logger.logInfo(device->getId(), "Setting AC mode to: " + mode);
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
                logger.logInfo(device->getId(), "Setting AC temperature to: " + std::to_string(temperature));
                ac->setTemperature(temperature);
                return {
                    {"status", 200},
                    {"message", "AC temperature set successfully"}
                };
            }
            logger.logError(device->getId(), "Unsupported action for AC: " + action);
            throw std::invalid_argument("Unsupported action for AC: " + action);
        } else if (auto light = std::dynamic_pointer_cast<Light>(device)) {
            logger.logError(device->getId(), "Unsupported action for Light: " + action);
            // No additional Light-specific actions for now
            throw std::invalid_argument("Unsupported action for Light: " + action);
        }

        // Handle unsupported actions
        logger.logError(device->getId(), "Unsupported action: " + action);
        throw std::invalid_argument("Unsupported action: " + action);
    } catch (const std::exception& e) {
        logger.logError(device->getId(), std::string("Error: ") + e.what());
        return {
            {"status", 400},
            {"message", std::string("Error: ") + e.what()}
        };
    }
}

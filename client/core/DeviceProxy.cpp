#include "../include/DeviceProxy.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <thread>

using json = nlohmann::json;

DeviceProxy::DeviceProxy(const std::string& id, const std::string& type, const std::string& ipAddress, const std::string& clientId, int port)
    : id(id), type(type), ipAddress(ipAddress), port(port), socket(-1), clientId(clientId) {}

DeviceProxy::~DeviceProxy() {
    if (socket != -1) {
        closeSocket();
    }
}

int DeviceProxy::createSocket() {
    if (socket == -1) {
        socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (socket < 0) {
            std::cerr << "Failed to create socket." << std::endl;
            throw std::runtime_error("Failed to create socket");
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr);

        // Retry mechanism to handle transient connection failures
        int retries = 3;
        while (connect(socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0 && retries > 0) {
            std::cerr << "Failed to connect to device, retrying..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2)); // wait before retry
            retries--;
        }

        if (retries == 0) {
            std::cerr << "Unable to connect to device after multiple attempts." << std::endl;
            close(socket);
            socket = -1;
            throw std::runtime_error("Failed to connect to device after multiple attempts. Closing connection");
        }
    }

    if (socket != -1) deviceReachable = true;
    return socket;
}

void DeviceProxy::closeSocket() {
    if (socket != -1) {
        close(socket);
        socket = -1;
    }
}

// Handle request sending and response
nlohmann::json DeviceProxy::sendRequest(const nlohmann::json& request) {
    std::cout << "Sending request: " << request.dump(4) << std::endl;
    int sock = createSocket();
    try {
        std::string requestStr = request.dump();
        if (send(sock, requestStr.c_str(), requestStr.size(), 0) < 0) {
            throw std::runtime_error("Failed to send request");
        }

        char buffer[1024] = {0};
        ssize_t bytesRead = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            throw std::runtime_error("No response received from device");
        }

        // Try parsing response into JSON
        try {
            json response = json::parse(buffer);

            if (response["status"] == 403) { // Token invalid
                std::cerr << "Token expired or invalid, resetting token." << std::endl;
                token.clear();
                throw std::runtime_error("Token expired or invalid.");
            }

            std::cout << response.dump(4) << std::endl;
            return response;
        } catch (const json::exception& e) {
            throw std::runtime_error("Failed to parse response: " + std::string(e.what()));
        }

    } catch (const std::exception& e) {
        std::cerr << "Error during request/response: " << e.what() << std::endl;
        markUnreachable();
        closeSocket();
        throw;
    }
}

bool DeviceProxy::authenticate(const std::string& clientId, const std::string& password) {
    json request = {
        {"action", "authenticate"},
        {"clientId", clientId},
        {"password", password}
    };

    try {
        json response = sendRequest(request);
        if (response["status"] == 200) {
            token = response["token"];
            std::cout << "Token: " << token << std::endl;
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Authentication failed: " << e.what() << std::endl;
    }
    return false;
}

bool DeviceProxy::isAuthenticated() const {
    return !token.empty();
}

void DeviceProxy::turnOn() {
    json request = {
        {"action", "turn_on"},
        {"token", token},
        {"clientId", clientId}
    };

    try {
        json response = sendRequest(request);
        if (response["status"] != 200) {
            throw std::runtime_error(response["message"]);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error turning on device: " << e.what() << std::endl;
    }
}

void DeviceProxy::turnOff() {
    json request = {
        {"action", "turn_off"},
        {"token", token},
        {"clientId", clientId}
    };

    try {
        json response = sendRequest(request);
        if (response["status"] != 200) {
            throw std::runtime_error(response["message"]);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error turning off device: " << e.what() << std::endl;
    }
}

void DeviceProxy::setTimer(int duration, const std::string& action) {
    json request = {
        {"action", "set_timer"},
        {"token", token},
        {"clientId", clientId},
        {"duration", duration},
        {"timer_action", action}
    };

    try {
        json response = sendRequest(request);
        if (response["status"] != 200) {
            throw std::runtime_error(response["message"]);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error setting timer: " << e.what() << std::endl;
    }
}

nlohmann::json DeviceProxy::getInfo() {
    json request = {
        {"action", "status"},
        {"token", token},
        {"clientId", clientId}
    };

    try {
        return sendRequest(request);
    } catch (const std::exception& e) {
        std::cerr << "Error getting info: " << e.what() << std::endl;
        return {};
    }
}

nlohmann::json DeviceProxy::getDetailedInfo() {
    json request = {
        {"action", "details"},
        {"token", token},
        {"clientId", clientId}
    };

    try {
        return sendRequest(request);
    } catch (const std::exception& e) {
        std::cerr << "Error getting detailed info: " << e.what() << std::endl;
        return {};
    }
}

bool DeviceProxy::changePassword(const std::string& currentPassword, const std::string& newPassword) {
    json request = {
        {"action", "change_password"},
        {"token", token},
        {"clientId", clientId},
        {"currentPassword", currentPassword},
        {"newPassword", newPassword}
    };

    try {
        json response = sendRequest(request);
        if (response["status"] == 200) {
            std::cout << "Password changed successfully!" << std::endl;
            return true;
        } else {
            std::cerr << "Error changing password: " << response["message"] << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception during password change: " << e.what() << std::endl;
    }

    return false;
}

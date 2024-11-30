#include "AuthenticationManager.h"
#include <random>
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

// Constructor
AuthenticationManager::AuthenticationManager(const std::string &password)
    : password(password) {}

// Destructor
AuthenticationManager::~AuthenticationManager() {}

// Generate a random token
std::string AuthenticationManager::generateTokenInternal() {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string token;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2); // -2 for null terminator

    for (int i = 0; i < 16; ++i) {
        token += alphanum[dis(gen)];
    }
    return token;
}

// Authenticate client
json AuthenticationManager::authenticate(const json &request) {
    std::string clientId = request["clientId"];
    std::string clientPassword = request["password"];

    if (clientPassword == password) {
        std::string token = generateTokenInternal();
        activeTokens[clientId] = token;
        tokenExpiry[clientId] = std::chrono::system_clock::now() + std::chrono::hours(1);

        return {
            {"status", 200},
            {"message", "Authentication successful"},
            {"token", token}
        };
    }
    return {
        {"status", 401},
        {"message", "Authentication failed"}
    };
}

// Validate token
json AuthenticationManager::validateToken(const json &request) {
    std::string clientId = request["clientId"];
    std::string token = request["token"];

    auto it = activeTokens.find(clientId);
    if (it != activeTokens.end() && it->second == token) {
        auto now = std::chrono::system_clock::now();
        if (now < tokenExpiry[clientId]) {
            return {
                {"status", 200},
                {"message", "Token is valid"}
            };
        }
    }
    return {
        {"status", 403},
        {"message", "Invalid or expired token"}
    };
}

// Change password
json AuthenticationManager::changePassword(const json &request) {
    std::string clientPassword = request["currentPassword"];
    std::string newPassword = request["newPassword"];

    if (clientPassword == password) {
        password = newPassword;
        return {
            {"status", 200},
            {"message", "Password changed successfully"}
        };
    }
    return {
        {"status", 401},
        {"message", "Current password is incorrect"}
    };
}

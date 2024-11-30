#ifndef AUTHENTICATION_MANAGER_H
#define AUTHENTICATION_MANAGER_H

#include <string>
#include <unordered_map>
#include <chrono>
#include "lib/json.hpp"

class AuthenticationManager {
private:
    std::string password;
    std::unordered_map<std::string, std::string> activeTokens;
    std::unordered_map<std::string, std::chrono::system_clock::time_point> tokenExpiry;

    std::string generateTokenInternal();

public:
    AuthenticationManager(const std::string &password);
    ~AuthenticationManager();

    nlohmann::json authenticate(const nlohmann::json &request);
    nlohmann::json validateToken(const nlohmann::json &request);
    nlohmann::json changePassword(const nlohmann::json &request);
};

#endif // AUTHENTICATION_MANAGER_H

#ifndef UTILITY_H
#define UTILITY_H

#include "lib/json.hpp"
#include <string>

void sendJsonResponse(int clientSock, int statusCode, const std::string& message, const nlohmann::json& additionalFields = {});
nlohmann::json parseJsonRequest(int clientSock);
std::string trim(const std::string& str);

#endif // UTILITY_H

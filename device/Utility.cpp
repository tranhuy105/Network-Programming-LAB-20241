#include "Utility.h"
#include <unistd.h>
#include <stdexcept>
#include <cstring>

using json = nlohmann::json;

void sendJsonResponse(int clientSock, int statusCode, const std::string& message, const json& additionalFields) {
    json response = {
        {"status", statusCode},
        {"message", message}
    };

    if (!additionalFields.is_null()) {
        for (auto& [key, value] : additionalFields.items()) {
            response[key] = value;
        }
    }

    std::string responseStr = response.dump();
    ssize_t bytesSent = write(clientSock, responseStr.c_str(), responseStr.size());

    if (bytesSent < 0) {
        perror("Failed to send JSON response");
    }
}


// Parse a JSON request from a socket
json parseJsonRequest(int clientSock) {
    char buffer[1024] = {0}; // Adjust buffer size as needed
    ssize_t bytesRead = read(clientSock, buffer, sizeof(buffer) - 1);

    if (bytesRead <= 0) {
        throw std::runtime_error("Failed to read from socket or client disconnected");
    }

    buffer[bytesRead] = '\0'; // Null-terminate the string
    return json::parse(buffer); // Parse the string into a JSON object
}

// Trim leading and trailing whitespace
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

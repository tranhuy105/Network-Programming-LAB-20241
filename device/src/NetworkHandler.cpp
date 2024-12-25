#include "../include/NetworkHandler.h"
#include "../include/Utility.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using json = nlohmann::json;

NetworkHandler::NetworkHandler(CommandHandler& commandHandler, int tcpPort)
    : commandHandler(commandHandler), tcpPort(tcpPort) {}

NetworkHandler::~NetworkHandler() {
    stop();
}


void NetworkHandler::start() {
    stopFlag = false;

    // Start discovery thread
    std::thread(&NetworkHandler::udpDiscovery, this).detach();

    // Start TCP server thread
    std::thread(&NetworkHandler::tcpServer, this).detach();
}

void NetworkHandler::stop() {
    stopFlag = true;
}

void NetworkHandler::udpDiscovery() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("UDP socket creation failed");
        return;
    }

    sockaddr_in multicastAddr{};
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_port = htons(multicastPort);
    inet_pton(AF_INET, multicastIP.c_str(), &multicastAddr.sin_addr);

    json messageJson = {
        {"type", commandHandler.getDevice()->getType()},
        {"id", commandHandler.getDevice()->getId()},
        {"ipAddress", "127.0.0.1"},
        {"port", tcpPort} // Port the device is listening on
    };
    std::string message = messageJson.dump();

    while (!stopFlag) {
        sendto(sock, message.c_str(), message.size(), 0,
               (sockaddr *)&multicastAddr, sizeof(multicastAddr));
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    close(sock);
}

void NetworkHandler::tcpServer() {
    int serverSock = createServerSocket();
    if (serverSock < 0) return;

    fd_set masterSet, readSet;
    FD_ZERO(&masterSet);
    FD_SET(serverSock, &masterSet);
    int maxFd = serverSock;

    while (!stopFlag) {
        readSet = masterSet;
        int activity = select(maxFd + 1, &readSet, nullptr, nullptr, nullptr);
        if (activity < 0 && !stopFlag) {
            perror("Select failed");
            break;
        }

        if (FD_ISSET(serverSock, &readSet)) {
            handleNewConnection(serverSock, masterSet, maxFd);
        }

        for (int sock = 0; sock <= maxFd; ++sock) {
            if (sock != serverSock && FD_ISSET(sock, &readSet)) {
                if (!handleClientRequest(sock)) {
                    closeClientConnection(sock, masterSet);
                }
            }
        }
    }

    close(serverSock);
}

int NetworkHandler::createServerSocket() {
    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) {
        perror("TCP socket creation failed");
        return -1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(tcpPort);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(serverSock);
        return -1;
    }

    if (listen(serverSock, 5) < 0) {
        perror("Listen failed");
        close(serverSock);
        return -1;
    }

    std::cout << "Listening for commands on TCP port " << tcpPort << "\n";
    return serverSock;
}

void NetworkHandler::handleNewConnection(int serverSock, fd_set &masterSet, int &maxFd) {
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    int clientSock = accept(serverSock, (sockaddr *)&clientAddr, &clientLen);
    if (clientSock < 0) {
        perror("Accept failed");
        return;
    }

    std::cout << "New client connected: " << clientSock << "\n";
    FD_SET(clientSock, &masterSet);
    if (clientSock > maxFd) maxFd = clientSock;
}

bool NetworkHandler::handleClientRequest(int clientSock) {
    try {
        char buffer[1024] = {0};
        ssize_t bytesRead = read(clientSock, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            // Client disconnected
            std::cout << "Client disconnected: " << clientSock << "\n";
            return false;
        }

        buffer[bytesRead] = '\0';

        // Attempt to parse JSON request
        try {
            json requestJson = json::parse(trim(buffer)); // This might throw json::exception
            processJsonRequest(clientSock, requestJson);
        } catch (const json::exception& e) {
            // Invalid JSON input
            sendJsonResponse(clientSock, 400, "Invalid JSON format: " + std::string(e.what()), {});
        }

        return true; // Keep the connection open
    } catch (const std::exception &e) {
        // Catch unexpected errors
        sendJsonResponse(clientSock, 500, "Internal server error: " + std::string(e.what()), {});
        return false; // Close connection for critical errors
    }
}


void NetworkHandler::processJsonRequest(int clientSock, const json &requestJson) {
    try {
        json response = commandHandler.handleCommand(requestJson);

        // Remove "status" and "message" from the response before sending
        json additionalFields = response;
        additionalFields.erase("status");
        additionalFields.erase("message");

        sendJsonResponse(clientSock, response["status"], response["message"], additionalFields);
    } catch (const std::exception &e) {
        sendJsonResponse(clientSock, 400, std::string("Error: ") + e.what(), {});
    }
}


void NetworkHandler::closeClientConnection(int clientSock, fd_set &masterSet) {
    std::cout << "Client disconnected: " << clientSock << "\n";
    FD_CLR(clientSock, &masterSet);
    close(clientSock);
}

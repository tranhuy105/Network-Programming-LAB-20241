#include "../include/DeviceScanner.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

DeviceScanner::DeviceScanner(const std::string& multicastIP, int multicastPort)
    : multicastIP(multicastIP), multicastPort(multicastPort), socket(-1), stopFlag(false) {}

DeviceScanner::~DeviceScanner() {
    stopScan();
}

DeviceScanner& DeviceScanner::getInstance() {
    static DeviceScanner instance("239.255.255.250", 1900);
    return instance;
}

void DeviceScanner::startScan() {
    if (socket != -1) {
        throw std::runtime_error("Socket is already in use. Stop the current scan before starting a new one.");
    }

    clearLastError();  
    stopFlag = false;
    scannedDevices.clear();

    std::thread(&DeviceScanner::listenForDevices, this).detach();
}

void DeviceScanner::stopScan() {
    stopFlag = true;
    if (socket != -1) {
        closeSocket();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void DeviceScanner::listenForDevices() {
    try {
        socket = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (socket < 0) {
            throw std::runtime_error("Failed to create socket");
        }

        sockaddr_in localAddr{};
        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons(multicastPort);
        localAddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(socket, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
            throw std::runtime_error("Failed to bind socket");
        }

        ip_mreq mreq{};
        inet_pton(AF_INET, multicastIP.c_str(), &mreq.imr_multiaddr);
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);

        if (setsockopt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
            throw std::runtime_error("Failed to join multicast group");
        }

        char buffer[1024];
        while (!stopFlag) {
            ssize_t bytesRead = recv(socket, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                try {
                    json deviceInfo = json::parse(buffer);
                    if (validateDeviceInfo(deviceInfo) && !isDeviceAlreadyScanned(deviceInfo)) {
                        std::lock_guard<std::mutex> lock(deviceMutex);
                        scannedDevices.push_back(deviceInfo);
                    }
                } catch (const json::exception& e) {
                    std::cerr << "Failed to parse device info: " << e.what() << std::endl;
                }
            }
        }

        close(socket);
        socket = -1;

    } catch (const std::exception& e) {
        {
            std::lock_guard<std::mutex> lock(errorMutex);
            lastError = e.what();
        }
        stopFlag = true;
        closeSocket();
    }
}


bool DeviceScanner::isDeviceAlreadyScanned(const json& deviceInfo) const {
    std::lock_guard<std::mutex> lock(deviceMutex); // Đồng bộ hóa khi đọc danh sách thiết bị
    auto it = std::find_if(scannedDevices.begin(), scannedDevices.end(), [&](const json& device) {
        return device["id"] == deviceInfo["id"] && device["ipAddress"] == deviceInfo["ipAddress"];
    });
    return it != scannedDevices.end();
}

std::vector<json> DeviceScanner::getScannedDevices() const {
    std::lock_guard<std::mutex> lock(deviceMutex); 
    return scannedDevices; // Trả về bản sao
}


bool DeviceScanner::validateDeviceInfo(const json& deviceInfo) const {
    if (!deviceInfo.contains("type") || !deviceInfo["type"].is_string()) {
        return false;
    }
    if (!deviceInfo.contains("id") || !deviceInfo["id"].is_string()) {
        return false;
    }
    if (!deviceInfo.contains("ipAddress") || !deviceInfo["ipAddress"].is_string()) {
        return false;
    }
    if (!deviceInfo.contains("port") || !deviceInfo["port"].is_number_integer()) {
        return false;
    }
    return true;
}

void DeviceScanner::closeSocket() {
    if (socket != -1) {
        close(socket);
        socket = -1;
    }
}

std::string DeviceScanner::getLastError() const {
    std::lock_guard<std::mutex> lock(errorMutex);
    return lastError;
}

void DeviceScanner::clearLastError() {
    std::lock_guard<std::mutex> lock(errorMutex);
    lastError.clear();
}

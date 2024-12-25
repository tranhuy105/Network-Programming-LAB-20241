#include "../include/HomeManager.h"
#include <fstream>
#include <iostream>
#include <algorithm>

// Tải dữ liệu từ file JSON
void HomeManager::loadFromFile() {
    std::ifstream inFile(jsonFile);
    if (!inFile.is_open()) {
        std::cerr << "Could not open " << jsonFile << " for reading.\n";
        return;
    }

    try {
        nlohmann::json jsonData;
        inFile >> jsonData;

        homes.clear();
        for (const auto& homeJson : jsonData["homes"]) {
            Home home;
            home.name = homeJson["name"].get<std::string>();

            for (const auto& roomJson : homeJson["rooms"]) {
                Room room;
                room.name = roomJson["name"].get<std::string>();
                for (const auto& deviceJson : roomJson["devices"]) {
                    DeviceStatus device;
                    device.id = deviceJson["id"].get<std::string>();
                    device.type = deviceJson["type"].get<std::string>();
                    device.available = deviceJson["available"].get<bool>();
                    room.devices.push_back(device);
                }
                home.rooms.push_back(room);
            }

            homes.push_back(home);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading JSON: " << e.what() << "\n";
    }
}

// Lưu dữ liệu vào file JSON
void HomeManager::saveToFile() {
    std::ofstream outFile(jsonFile);
    if (!outFile.is_open()) {
        std::cerr << "Could not open " << jsonFile << " for writing.\n";
        return;
    }

    try {
        nlohmann::json jsonData;
        for (const auto& home : homes) {
            nlohmann::json homeJson;
            homeJson["name"] = home.name;

            for (const auto& room : home.rooms) {
                nlohmann::json roomJson;
                roomJson["name"] = room.name;

                for (const auto& device : room.devices) {
                    nlohmann::json deviceJson;
                    deviceJson["id"] = device.id;
                    deviceJson["type"] = device.type;
                    deviceJson["available"] = device.available;
                    roomJson["devices"].push_back(deviceJson);
                }

                homeJson["rooms"].push_back(roomJson);
            }

            jsonData["homes"].push_back(homeJson);
        }

        outFile << jsonData.dump(4);
    } catch (const std::exception& e) {
        std::cerr << "Error writing JSON: " << e.what() << "\n";
    }
}

// Thêm Home mới
void HomeManager::addHome(const std::string& homeName) {
    homes.push_back({homeName, {}});
}

// Sửa tên Home
void HomeManager::editHomeName(int index, const std::string& newName) {
    if (index >= 0 && index < homes.size()) {
        homes[index].name = newName;
    }
}

// Xóa Home
void HomeManager::deleteHome(int index) {
    if (index >= 0 && index < homes.size()) {
        homes.erase(homes.begin() + index);
    }
}

// Thêm Room vào Home
void HomeManager::addRoomToHome(int homeIndex, const std::string& roomName) {
    if (homeIndex >= 0 && homeIndex < homes.size()) {
        homes[homeIndex].rooms.push_back({roomName, {}});
    }
}

// Sửa tên Room
void HomeManager::editRoomName(int homeIndex, int roomIndex, const std::string& newName) {
    if (homeIndex >= 0 && homeIndex < homes.size() &&
        roomIndex >= 0 && roomIndex < homes[homeIndex].rooms.size()) {
        homes[homeIndex].rooms[roomIndex].name = newName;
    }
}

// Xóa Room khỏi Home
void HomeManager::deleteRoomFromHome(int homeIndex, int roomIndex) {
    if (homeIndex >= 0 && homeIndex < homes.size() &&
        roomIndex >= 0 && roomIndex < homes[homeIndex].rooms.size()) {
        homes[homeIndex].rooms.erase(homes[homeIndex].rooms.begin() + roomIndex);
    }
}

// Thêm thiết bị vào Room
void HomeManager::addDeviceToRoom(int homeIndex, int roomIndex, const std::string& deviceId) {
    if (homeIndex >= 0 && homeIndex < homes.size() &&
        roomIndex >= 0 && roomIndex < homes[homeIndex].rooms.size()) {
        auto& devices = homes[homeIndex].rooms[roomIndex].devices;
        if (std::none_of(devices.begin(), devices.end(), [&deviceId](const DeviceStatus& d) { return d.id == deviceId; })) {
            devices.push_back({deviceId, "Unknown", false}); // Unknown type, unavailable by default
        }
    }
}

// Xóa thiết bị khỏi Room
void HomeManager::removeDeviceFromRoom(int homeIndex, int roomIndex, const std::string& deviceId) {
    if (homeIndex >= 0 && homeIndex < homes.size() &&
        roomIndex >= 0 && roomIndex < homes[homeIndex].rooms.size()) {
        auto& devices = homes[homeIndex].rooms[roomIndex].devices;
        devices.erase(std::remove_if(devices.begin(), devices.end(),
                                     [&deviceId](const DeviceStatus& d) { return d.id == deviceId; }),
                      devices.end());
    }
}


void HomeManager::syncDeviceAvailability(const std::vector<std::shared_ptr<DeviceProxy>>& scannedDevices) {
    // Tạo map từ id -> thiết bị quét được
    std::unordered_map<std::string, std::shared_ptr<DeviceProxy>> scannedDeviceMap;
    for (const auto& device : scannedDevices) {
        scannedDeviceMap[device->getId()] = device;
    }

    // Cập nhật trạng thái available cho các thiết bị trong Home
    for (auto& home : homes) {
        for (auto& room : home.rooms) {
            for (auto& device : room.devices) {
                if (scannedDeviceMap.count(device.id)) {
                    device.available = true;
                    device.type = scannedDeviceMap[device.id]->getType(); // Cập nhật loại thiết bị
                } else {
                    device.available = false;
                }
            }
        }
    }
}

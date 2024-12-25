#ifndef HOME_MANAGER_H
#define HOME_MANAGER_H

#include "../lib/json.hpp"
#include "../include/DeviceProxy.h"
#include <vector>
#include <string>

struct DeviceStatus {
    std::string id;
    std::string type;
    bool available; // True nếu quét thấy, False nếu không khả dụng
};

// Cấu trúc dữ liệu Room và Home
struct Room {
    std::string name;                  // Tên của Room
    std::vector<DeviceStatus> devices; // ID của các thiết bị trong Room
};

struct Home {
    std::string name;         // Tên của Home
    std::vector<Room> rooms; // Danh sách các Room trong Home
};

class HomeManager {
private:
    std::vector<Home> homes;         // Danh sách Home
    std::string jsonFile = "homes.json"; // File JSON để lưu trữ

public:
    // Tải và lưu dữ liệu từ JSON
    void loadFromFile();            // Tải dữ liệu từ file JSON
    void saveToFile();              // Lưu dữ liệu vào file JSON

    // Quản lý Home
    void addHome(const std::string& homeName);                  // Thêm Home mới
    void editHomeName(int index, const std::string& newName);   // Sửa tên Home
    void deleteHome(int index);                                // Xóa Home

    // Quản lý Room
    void addRoomToHome(int homeIndex, const std::string& roomName);         // Thêm Room vào Home
    void editRoomName(int homeIndex, int roomIndex, const std::string& newName); // Sửa tên Room
    void deleteRoomFromHome(int homeIndex, int roomIndex);                 // Xóa Room khỏi Home

    // Quản lý thiết bị trong Room
    void addDeviceToRoom(int homeIndex, int roomIndex, const std::string& deviceId); // Thêm thiết bị vào Room
    void removeDeviceFromRoom(int homeIndex, int roomIndex, const std::string& deviceId); // Xóa thiết bị khỏi Room

    // Truy cập danh sách Home
    std::vector<Home>& getHomes() {
        return homes; // Provide mutable access to the homes vector
    }

    void syncDeviceAvailability(const std::vector<std::shared_ptr<DeviceProxy>>& scannedDevices);
};

#endif

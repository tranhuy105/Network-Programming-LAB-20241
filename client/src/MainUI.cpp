#include "../include/DeviceScanner.h"
#include "DeviceDetailUI.h"
#include "../include/DeviceProxy.h"
#include "../include/ACProxy.h"
#include "../include/LightProxy.h"
#include "../include/HomeManager.h"
#include "../include/FanProxy.h"
#include "../lib/imgui/imgui.h"
#include <vector>
#include <string>
#include <iostream>

// Hàm xử lý quét thiết bị
void handleDeviceScanning(bool& scanningInProgress, std::vector<std::shared_ptr<DeviceProxy>>& devices, std::string& clientId, std::string& errorMsg) {
    if (!scanningInProgress && ImGui::Button("Scan Devices")) {
        try {
            scanningInProgress = true;
            errorMsg.clear();
            devices.clear();
            DeviceScanner::getInstance().startScan();
        } catch (const std::exception& e) {
            errorMsg = e.what();
            scanningInProgress = false;
        }
    }

    if (scanningInProgress && ImGui::Button("Stop Scanning")) {
        scanningInProgress = false;
        DeviceScanner::getInstance().stopScan();
    }

    if (!scanningInProgress && !DeviceScanner::getInstance().getLastError().empty()) {
        errorMsg = DeviceScanner::getInstance().getLastError();
        DeviceScanner::getInstance().clearLastError();
    }

    if (scanningInProgress) {
        try {
            auto scannedDevices = DeviceScanner::getInstance().getScannedDevices();
            for (const auto& device : scannedDevices) {
                bool deviceAlreadyAdded = false;

                for (const auto& existingDevice : devices) {
                    if (existingDevice->getId() == device["id"].get<std::string>()) {
                        deviceAlreadyAdded = true;
                        break;
                    }
                }

                if (!deviceAlreadyAdded) {
                    std::string type = device["type"].get<std::string>();
                    if (type == "AC") {
                        devices.push_back(std::make_shared<ACProxy>(device["id"], device["ipAddress"], clientId, device["port"]));
                    } else if (type == "Light") {
                        devices.push_back(std::make_shared<LightProxy>(device["id"], device["ipAddress"], clientId, device["port"]));
                    } else if (type == "Fan") {
                        devices.push_back(std::make_shared<FanProxy>(device["id"], device["ipAddress"], clientId, device["port"]));
                    } else {
                        std::cerr << "Unknown device type: " << type << std::endl;
                    }
                }
            }
        } catch (const std::exception& e) {
            errorMsg = e.what();
            scanningInProgress = false;
        }
    }
}

// Hàm hiển thị thông báo lỗi
void displayError(const std::string& errorMsg) {
    if (!errorMsg.empty()) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", errorMsg.c_str());
    }
}

// Hàm hiển thị trạng thái quét
void displayScanningStatus(bool scanningInProgress, const std::vector<std::shared_ptr<DeviceProxy>>& devices) {
    if (scanningInProgress) {
        ImGui::Text("Scanning for devices...");
    } else if (devices.empty()) {
        ImGui::Text("No devices found.");
    }
}

void renderAssignDeviceUI(HomeManager& homeManager, 
                          std::vector<std::shared_ptr<DeviceProxy>>& unassignedDevices, 
                          std::string& selectedDeviceId, 
                          int& targetHomeIndex, 
                          int& targetRoomIndex) {
    if (ImGui::CollapsingHeader("Assign Devices to Room")) {
        // Hiển thị danh sách thiết bị chưa được gán
        ImGui::Text("Unassigned Devices:");
        ImGui::Separator();

        for (const auto& device : unassignedDevices) {
            if (ImGui::Selectable(device->getId().c_str(), selectedDeviceId == device->getId())) {
                selectedDeviceId = device->getId();
            }
        }

        ImGui::Separator();

        // Chọn Home và Room
        if (ImGui::TreeNode("Select Target Home & Room")) {
            for (int homeIndex = 0; homeIndex < homeManager.getHomes().size(); ++homeIndex) {
                const Home& home = homeManager.getHomes()[homeIndex];
                if (ImGui::TreeNode(home.name.c_str())) {
                    for (int roomIndex = 0; roomIndex < home.rooms.size(); ++roomIndex) {
                        const Room& room = home.rooms[roomIndex];
                        if (ImGui::Selectable(room.name.c_str(), targetHomeIndex == homeIndex && targetRoomIndex == roomIndex)) {
                            targetHomeIndex = homeIndex;
                            targetRoomIndex = roomIndex;
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        ImGui::Separator();

        // Nút Assign
        if (ImGui::Button("Assign Device") && targetHomeIndex >= 0 && targetRoomIndex >= 0 && !selectedDeviceId.empty()) {
            Home& targetHome = homeManager.getHomes()[targetHomeIndex];
            Room& targetRoom = targetHome.rooms[targetRoomIndex];

            // Tìm thiết bị đã chọn trong danh sách unassignedDevices
            auto it = std::find_if(unassignedDevices.begin(), unassignedDevices.end(), 
                                   [&selectedDeviceId](const std::shared_ptr<DeviceProxy>& device) {
                                       return device->getId() == selectedDeviceId;
                                   });

            if (it != unassignedDevices.end()) {
                DeviceStatus newDeviceStatus = { (*it)->getId(), (*it)->getType(), true }; // Thêm thiết bị vào Room
                targetRoom.devices.push_back(newDeviceStatus);
                unassignedDevices.erase(it); // Xóa thiết bị khỏi danh sách chưa gán
                selectedDeviceId.clear(); // Reset lựa chọn
            }
        }
    }
}


std::vector<DeviceStatus> filterDevicesByRoom(HomeManager& homeManager, 
                                              const std::vector<std::shared_ptr<DeviceProxy>>& devices, 
                                              int selectedHomeIndex, int selectedRoomIndex) {
    std::vector<DeviceStatus> filteredDevices;

    // Nếu không chọn Home hoặc Room, trả về danh sách trống
    if (selectedHomeIndex < 0 || selectedRoomIndex < 0) {
        return filteredDevices;
    }

    // Lấy Room được chọn
    Home& selectedHome = homeManager.getHomes()[selectedHomeIndex]; // Không còn lỗi
    Room& selectedRoom = selectedHome.rooms[selectedRoomIndex];

    // Lọc thiết bị từ Room
    for (auto roomDevice : selectedRoom.devices) { // Bản sao của roomDevice
        bool isAvailable = false;

        // Kiểm tra trạng thái availability từ danh sách thiết bị được quét
        for (const auto& scannedDevice : devices) {
            if (scannedDevice->getId() == roomDevice.id) {
                isAvailable = true;
                break;
            }
        }

        // Cập nhật trạng thái available cho thiết bị
        roomDevice.available = isAvailable;
        filteredDevices.push_back(roomDevice);
    }

    return filteredDevices;
}


void renderHomeAndRoomManagement(HomeManager& homeManager, int& selectedHomeIndex, int& selectedRoomIndex) {
    if (ImGui::CollapsingHeader("Home & Room Management", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Quản lý Home
        for (int homeIndex = 0; homeIndex < homeManager.getHomes().size(); ++homeIndex) {
            Home& home = homeManager.getHomes()[homeIndex];

            if (ImGui::TreeNode(home.name.c_str())) {
                // Quản lý Room trong từng Home
                for (int roomIndex = 0; roomIndex < home.rooms.size(); ++roomIndex) {
                    Room& room = home.rooms[roomIndex];

                    // Hiển thị Room
                    if (ImGui::Selectable(room.name.c_str(), 
                        selectedHomeIndex == homeIndex && selectedRoomIndex == roomIndex)) {
                        selectedHomeIndex = homeIndex;
                        selectedRoomIndex = roomIndex;
                    }

                    // Xóa Room
                    ImGui::SameLine();
                    if (ImGui::Button(("X##Room" + std::to_string(roomIndex)).c_str())) {
                        home.rooms.erase(home.rooms.begin() + roomIndex);
                        selectedRoomIndex = -1;
                        break; // Cần thoát khỏi vòng lặp vì danh sách đã thay đổi
                    }
                }

                // Thêm Room
                static char newRoomName[64] = "";
                ImGui::InputText(("New Room Name##" + std::to_string(homeIndex)).c_str(), 
                                 newRoomName, IM_ARRAYSIZE(newRoomName));
                if (ImGui::Button(("Add Room##" + std::to_string(homeIndex)).c_str())) {
                    Room newRoom;
                    newRoom.name = newRoomName;
                    home.rooms.push_back(newRoom);
                    std::fill(std::begin(newRoomName), std::end(newRoomName), '\0');
                }

                ImGui::TreePop();
            }

            // Xóa Home
            ImGui::SameLine();
            if (ImGui::Button(("X##Home" + std::to_string(homeIndex)).c_str())) {
                homeManager.getHomes().erase(homeManager.getHomes().begin() + homeIndex);
                selectedHomeIndex = -1;
                selectedRoomIndex = -1;
                break; // Cần thoát khỏi vòng lặp vì danh sách đã thay đổi
            }
        }

        ImGui::Separator();

        // Thêm Home mới
        static char newHomeName[64] = "";
        ImGui::InputText("New Home Name", newHomeName, IM_ARRAYSIZE(newHomeName));
        if (ImGui::Button("Add Home")) {
            Home newHome;
            newHome.name = newHomeName;
            homeManager.getHomes().push_back(newHome);
            std::fill(std::begin(newHomeName), std::end(newHomeName), '\0');
        }
    }
}


void renderRenameHomeRoomUI(HomeManager& homeManager, int& selectedHomeIndex, int& selectedRoomIndex) {
    if (ImGui::CollapsingHeader("Rename Home & Room")) {
        // Đổi tên Home
        if (selectedHomeIndex >= 0) {
            static char newHomeName[64] = "";
            ImGui::InputText("New Home Name", newHomeName, IM_ARRAYSIZE(newHomeName));
            if (ImGui::Button("Rename Home")) {
                homeManager.getHomes()[selectedHomeIndex].name = newHomeName;
                std::fill(std::begin(newHomeName), std::end(newHomeName), '\0');
            }
        }

        // Đổi tên Room
        if (selectedHomeIndex >= 0 && selectedRoomIndex >= 0) {
            static char newRoomName[64] = "";
            ImGui::InputText("New Room Name", newRoomName, IM_ARRAYSIZE(newRoomName));
            if (ImGui::Button("Rename Room")) {
                homeManager.getHomes()[selectedHomeIndex].rooms[selectedRoomIndex].name = newRoomName;
                std::fill(std::begin(newRoomName), std::end(newRoomName), '\0');
            }
        }
    }
}

void renderDeviceList(const std::vector<DeviceStatus>& devices, int& selectedDeviceIndex) {
    ImGui::BeginChild("Device List", ImVec2(250, 0), true, ImGuiWindowFlags_NoScrollbar);

    ImGui::Text("Devices Found:");
    ImGui::Separator();

    if (devices.empty()) {
        ImGui::Text("No devices in this room.");
    } else {
        for (size_t i = 0; i < devices.size(); ++i) {
            const auto& device = devices[i];
            std::string icon = (device.type == "Light") ? "[Light]" :
                               (device.type == "Fan") ? "[Fan]" : "[AC]";
            std::string deviceLabel = icon + " " + device.type + " - " + device.id +
                                      (device.available ? " [Available]" : " [Unavailable]");
            
            if (ImGui::Selectable(deviceLabel.c_str(), selectedDeviceIndex == static_cast<int>(i))) {
                selectedDeviceIndex = i;
            }
        }
    }

    ImGui::EndChild();
}


void renderMainUI(HomeManager& homeManager, 
                  std::vector<std::shared_ptr<DeviceProxy>>& devices, 
                  std::string& clientId) {
    static bool scanningInProgress = false;
    static std::string errorMsg;
    static int selectedDeviceIndex = -1;
    static int selectedHomeIndex = -1;
    static int selectedRoomIndex = -1;

    static std::vector<std::shared_ptr<DeviceProxy>> unassignedDevices;
    static std::string selectedDeviceId;
    static int targetHomeIndex = -1;
    static int targetRoomIndex = -1;

    // Cấu hình cửa sổ
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Device Management System");

    // Quét thiết bị
    handleDeviceScanning(scanningInProgress, devices, clientId, errorMsg);

    // Quản lý Home và Room
    renderHomeAndRoomManagement(homeManager, selectedHomeIndex, selectedRoomIndex);

    // Lọc thiết bị chưa gán
    unassignedDevices.clear();
    for (const auto& device : devices) {
        bool isAssigned = false;
        for (const auto& home : homeManager.getHomes()) {
            for (const auto& room : home.rooms) {
                for (const auto& roomDevice : room.devices) {
                    if (roomDevice.id == device->getId()) {
                        isAssigned = true;
                        break;
                    }
                }
                if (isAssigned) break;
            }
            if (isAssigned) break;
        }
        if (!isAssigned) {
            unassignedDevices.push_back(device);
        }
    }

    // Hiển thị giao diện Assign và Rename
    renderAssignDeviceUI(homeManager, unassignedDevices, selectedDeviceId, targetHomeIndex, targetRoomIndex);
    renderRenameHomeRoomUI(homeManager, selectedHomeIndex, selectedRoomIndex);

    // Layout danh sách thiết bị và chi tiết
    ImGui::Columns(2, nullptr, false);
    std::vector<DeviceStatus> filteredDevices = filterDevicesByRoom(homeManager, devices, selectedHomeIndex, selectedRoomIndex);
    renderDeviceList(filteredDevices, selectedDeviceIndex);
    ImGui::NextColumn();

    if (selectedDeviceIndex >= 0 && selectedDeviceIndex < static_cast<int>(filteredDevices.size())) {
        for (const auto& device : devices) {
            if (device->getId() == filteredDevices[selectedDeviceIndex].id) {
                renderDeviceDetail(*device, clientId);
                break;
            }
        }
    } else {
        ImGui::Text("No device selected.");
    }

    ImGui::End();
}

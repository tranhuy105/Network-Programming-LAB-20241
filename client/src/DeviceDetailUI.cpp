#include "../lib/imgui/imgui.h"
#include "../include/DeviceProxy.h"
#include "../include/FanProxy.h"
#include "../include/ACProxy.h"
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

// Hàm hiển thị thông tin cơ bản của thiết bị
void renderBasicInfo(DeviceProxy& device) {
    ImGui::Text("Device Detail:");
    ImGui::Separator();

    ImGui::Text("ID: %s", device.getId().c_str());
    ImGui::Text("Type: %s", device.getType().c_str());
    ImGui::Separator();
}

// Hàm xử lý xác thực
void renderAuthentication(DeviceProxy& device, std::string& clientId) {
    static char password[64] = "";

    if (!device.isAuthenticated()) {
        ImGui::Text("Authentication Required");
        ImGui::InputText("Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);
        if (ImGui::Button("Authenticate", ImVec2(150, 30))) {
            if (device.authenticate(clientId, password)) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "Authentication Successful");
            } else {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Authentication Failed. Please Try Again!");
            }
            std::fill(std::begin(password), std::end(password), '\0');
        }
    } else {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Authenticated");
    }
    ImGui::Separator();
}

// Hàm hiển thị điều khiển đặc thù cho thiết bị Fan
void renderFanControls(FanProxy& fan, const nlohmann::json& cachedDetails) {
    if (cachedDetails.contains("speed")) {
        int speed = cachedDetails["speed"].is_number() ? cachedDetails["speed"].get<int>() : 0;
        ImGui::Text("Fan Speed: %d", speed);
        static int newSpeed = speed;
        ImGui::SliderInt("Fan Speed", &newSpeed, 1, 3);
        if (ImGui::Button("Set Speed", ImVec2(150, 30))) {
            fan.setSpeed(newSpeed);
        }
    } else {
        ImGui::Text("Fan speed data not available.");
    }
    ImGui::Separator();
}

void renderChangePassword(DeviceProxy& device) {
    ImGui::Text("Change Password:");
    static char currentPassword[64] = "";
    static char newPassword[64] = "";
    static char confirmPassword[64] = "";
    static std::string feedbackMessage;

    ImGui::InputText("Current Password", currentPassword, IM_ARRAYSIZE(currentPassword), ImGuiInputTextFlags_Password);
    ImGui::InputText("New Password", newPassword, IM_ARRAYSIZE(newPassword), ImGuiInputTextFlags_Password);
    ImGui::InputText("Confirm Password", confirmPassword, IM_ARRAYSIZE(confirmPassword), ImGuiInputTextFlags_Password);

    if (ImGui::Button("Change Password", ImVec2(150, 30))) {
        if (strcmp(newPassword, confirmPassword) != 0) {
            feedbackMessage = "New passwords do not match!";
        } else {
            if (device.changePassword(currentPassword, newPassword)) {
                feedbackMessage = "Password changed successfully!";
            } else {
                feedbackMessage = "Password change failed. Please check the current password.";
            }
        }
        // Clear inputs
        std::fill(std::begin(currentPassword), std::end(currentPassword), '\0');
        std::fill(std::begin(newPassword), std::end(newPassword), '\0');
        std::fill(std::begin(confirmPassword), std::end(confirmPassword), '\0');
    }

    if (!feedbackMessage.empty()) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", feedbackMessage.c_str());
    }

    ImGui::Separator();
}


// Hàm hiển thị điều khiển đặc thù cho thiết bị AC
void renderACControls(ACProxy& ac, const nlohmann::json& cachedDetails) {
    int temperature = cachedDetails.contains("temperature") && cachedDetails["temperature"].is_number() 
                      ? cachedDetails["temperature"].get<int>() 
                      : 18; // Giá trị mặc định
    std::string mode = cachedDetails.contains("mode") && cachedDetails["mode"].is_string() 
                       ? cachedDetails["mode"].get<std::string>() 
                       : "unknown";

    ImGui::Text("Temperature: %d °C", temperature);
    ImGui::Text("AC Mode: %s", mode.c_str());

    static int newTemperature = temperature;
    static int newMode = (mode == "cool" ? 0 : (mode == "heat" ? 1 : 2));
    const char* modes[] = {"Cool", "Heat", "Dry"};

    ImGui::Combo("Mode", &newMode, modes, IM_ARRAYSIZE(modes));
    ImGui::SliderInt("Temperature", &newTemperature, 18, 30);
    if (ImGui::Button("Set Temperature and Mode", ImVec2(200, 30))) {
        ac.setMode(static_cast<ACMode>(newMode));
        ac.setTemperature(newTemperature);
    }
    ImGui::Separator();
}

// Hàm hiển thị các thông tin chi tiết được cache
void renderCachedDetails(DeviceProxy& device, nlohmann::json& cachedDetails, bool& refreshRequested, std::chrono::steady_clock::time_point& lastUpdate) {
    if (ImGui::Button("Refresh Details", ImVec2(150, 30))) {
        refreshRequested = true;
    }
    ImGui::SameLine();
    ImGui::Text("Last Updated: %.1f seconds ago",
                static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::steady_clock::now() - lastUpdate)
                                        .count()) /
                    1000.0);

    if (refreshRequested || std::chrono::steady_clock::now() - lastUpdate > std::chrono::seconds(30)) {
        try {
            auto details = device.getDetailedInfo();
            if (details.contains("status") && details["status"] == 200 && details.contains("data")) {
                cachedDetails = details["data"];
                lastUpdate = std::chrono::steady_clock::now();
                refreshRequested = false;
            } else {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to fetch details: Invalid response.");
            }
        } catch (const std::exception& e) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error fetching details: %s", e.what());
        }
    }

    if (!cachedDetails.is_null()) {
        if (cachedDetails.contains("state") && cachedDetails["state"].is_string()) {
            ImGui::Text("State: %s", cachedDetails["state"].get<std::string>().c_str());
        } else {
            ImGui::Text("State data not available.");
        }

        if (cachedDetails.contains("power") && cachedDetails["power"].is_number()) {
            ImGui::Text("Power Consumption: %d W", cachedDetails["power"].get<int>());
        } else {
            ImGui::Text("Power consumption data not available.");
        }

        if (cachedDetails.contains("cumulative_power") && cachedDetails["cumulative_power"].is_number()) {
            double cumulativePower = cachedDetails["cumulative_power"].get<double>();
            double cumulativePowerKWh = cumulativePower / 3600.0 / 1000.0; // Chuyển đổi từ W × s sang kWh
            ImGui::Text("Cumulative Power: %.2f kWh", cumulativePowerKWh);
        } else {
            ImGui::Text("Cumulative power data not available.");
        }

        if (cachedDetails.contains("runtime")) {
            const auto& runtime = cachedDetails["runtime"];
            if (runtime.contains("daily") && runtime["daily"].is_number()) {
                int daily = runtime["daily"];
                ImGui::Text("Daily Runtime: %02d:%02d:%02d",
                            daily / 3600, (daily % 3600) / 60, daily % 60);
            }
            if (runtime.contains("monthly") && runtime["monthly"].is_number()) {
                int monthly = runtime["monthly"];
                ImGui::Text("Monthly Runtime: %02d:%02d:%02d",
                            monthly / 3600, (monthly % 3600) / 60, monthly % 60);
            }
        }
    }
    ImGui::Separator();
}

// Hàm hiển thị các hành động chung (Turn On, Turn Off, Set Timer)
void renderCommonActions(DeviceProxy& device) {
    ImGui::Text("Actions:");
    if (ImGui::Button("Turn On", ImVec2(150, 30))) {
        device.turnOn();
    }
    ImGui::SameLine();
    if (ImGui::Button("Turn Off", ImVec2(150, 30))) {
        device.turnOff();
    }

    ImGui::Separator();

    // Set Timer Section
    ImGui::Text("Set Timer:");
    static int timerDuration = 0;
    static int timerAction = 0; // 0 = Turn On, 1 = Turn Off
    const char* actions[] = {"Turn On", "Turn Off"};

    ImGui::SliderInt("Duration (s)", &timerDuration, 1, 20);
    ImGui::Combo("Action", &timerAction, actions, IM_ARRAYSIZE(actions));

    if (ImGui::Button("Set Timer", ImVec2(150, 30))) {
        device.setTimer(timerDuration, (timerAction == 0 ? "turn_on" : "turn_off"));
    }

    renderChangePassword(device);
}

// Hàm chính hiển thị chi tiết thiết bị
void renderDeviceDetail(DeviceProxy& device, std::string& clientId) {
    ImGui::BeginChild("Device Detail", ImVec2(0, 0), true);

    try {
        renderBasicInfo(device);
        renderAuthentication(device, clientId);

        static nlohmann::json cachedDetails;
        static auto lastUpdate = std::chrono::steady_clock::now();
        static bool refreshRequested = true;

        if (device.isAuthenticated()) {
            renderCachedDetails(device, cachedDetails, refreshRequested, lastUpdate);

            if (auto* fan = dynamic_cast<FanProxy*>(&device)) {
                renderFanControls(*fan, cachedDetails);
            } else if (auto* ac = dynamic_cast<ACProxy*>(&device)) {
                renderACControls(*ac, cachedDetails);
            }

            renderCommonActions(device);
        }

    } catch (const std::exception& e) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error rendering device details: %s", e.what());
    }

    ImGui::EndChild();
}

#ifndef DEVICE_SCANNER_H
#define DEVICE_SCANNER_H

#include <vector>
#include <string>
#include <mutex>
#include "../lib/json.hpp"

class DeviceScanner {
public:
    static DeviceScanner& getInstance();
    void startScan();
    void stopScan();
    std::vector<nlohmann::json> getScannedDevices() const; // Trả về danh sách thiết bị quét được
    std::string getLastError() const; // Getter for the last error
    void clearLastError();           // Clear the last error


private:
    DeviceScanner(const std::string& multicastIP, int multicastPort);
    ~DeviceScanner();
    void listenForDevices(); // Luồng xử lý quét thiết bị
    bool validateDeviceInfo(const nlohmann::json& deviceInfo) const; // Kiểm tra thông tin thiết bị hợp lệ
    bool isDeviceAlreadyScanned(const nlohmann::json& deviceInfo) const; // Kiểm tra trùng lặp thiết bị
    void closeSocket();
    
    int socket;
    bool stopFlag;
    std::vector<nlohmann::json> scannedDevices;
    mutable std::mutex deviceMutex; // Mutex để đồng bộ hóa truy cập vào scannedDevices
    mutable std::mutex errorMutex;   // Mutex to protect error messages
    std::string lastError;           // To store the last error message
    std::string multicastIP;
    int multicastPort;
};

#endif

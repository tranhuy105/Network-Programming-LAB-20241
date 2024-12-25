#include "../../include/DeviceScanner.h"
#include "../../include/ACProxy.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>

using json = nlohmann::json;
std::string CLIENTID = "JACK97";

// Function to print the menu
void printMenu() {
    std::cout << "\nCommands:\n"
              << "1. Scan for devices\n"
              << "2. Authenticate with device\n"
              << "3. Turn AC on\n"
              << "4. Turn AC off\n"
              << "5. Set AC temperature\n"
              << "6. Set AC mode\n"
              << "7. View AC status\n"
              << "8. Exit\n"
              << "Enter your choice: ";
}

// Function to print AC modes
void printModes() {
    std::cout << "\nAC Modes:\n"
              << "1. Cool\n"
              << "2. Heat\n"
              << "3. Dry\n"
              << "Enter mode choice: ";
}

int main() {
    auto& scanner = DeviceScanner::getInstance();
    std::vector<json> devices;
    std::shared_ptr<ACProxy> acDevice = nullptr; // Shared pointer to maintain device state

    std::cout << "Welcome to the CLI Smart Home Controller\n";

    while (true) {
        printMenu();
        int choice;
        std::cin >> choice;

        switch (choice) {
            case 1: { // Scan for devices
                std::cout << "Scanning for devices...\n";
                scanner.startScan();
                std::this_thread::sleep_for(std::chrono::seconds(5));
                scanner.stopScan();
                devices = scanner.getScannedDevices();

                std::cout << "Devices found:\n";
                for (size_t i = 0; i < devices.size(); ++i) {
                    std::cout << i + 1 << ". " << devices[i].dump(4) << std::endl;
                }
                break;
            }
            case 2: { // Authenticate with a specific device
                if (devices.empty()) {
                    std::cout << "No devices found. Please scan first.\n";
                    break;
                }

                std::cout << "Enter the device index to authenticate: ";
                size_t index;
                std::cin >> index;
                if (index < 1 || index > devices.size()) {
                    std::cout << "Invalid device index.\n";
                    break;
                }

                auto deviceInfo = devices[index - 1];
                if (deviceInfo["type"] != "AC") {
                    std::cout << "Only AC devices are supported for now.\n";
                    break;
                }

                std::cout << "Enter password for device: ";
                std::string password;
                std::cin >> password;

                if (!acDevice) {
                    acDevice = std::make_shared<ACProxy>(deviceInfo["id"], deviceInfo["ipAddress"], CLIENTID, deviceInfo["port"]);
                }
                if (acDevice->authenticate(CLIENTID, password)) {
                    std::cout << "Authentication successful!\n";
                } else {
                    std::cout << "Authentication failed.\n";
                    acDevice = nullptr; // Reset the device if authentication fails
                }
                break;
            }
            case 3: { // Turn AC on
                if (!acDevice) {
                    std::cout << "No device authenticated. Please authenticate first.\n";
                    break;
                }
                try {
                    acDevice->turnOn();
                    std::cout << "AC turned on.\n";
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << "\n";
                }
                break;
            }
            case 4: { // Turn AC off
                if (!acDevice) {
                    std::cout << "No device authenticated. Please authenticate first.\n";
                    break;
                }
                try {
                    acDevice->turnOff();
                    std::cout << "AC turned off.\n";
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << "\n";
                }
                break;
            }
            case 5: { // Set AC temperature
                if (!acDevice) {
                    std::cout << "No device authenticated. Please authenticate first.\n";
                    break;
                }
                std::cout << "Enter temperature (18-30): ";
                int temperature;
                std::cin >> temperature;
                try {
                    acDevice->setTemperature(temperature);
                    std::cout << "AC temperature set to " << temperature << "°C.\n";
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << "\n";
                }
                break;
            }
            case 6: { // Set AC mode
                if (!acDevice) {
                    std::cout << "No device authenticated. Please authenticate first.\n";
                    break;
                }
                printModes();
                int modeChoice;
                std::cin >> modeChoice;

                try {
                    if (modeChoice == 1) acDevice->setMode(ACMode::COOL);
                    else if (modeChoice == 2) acDevice->setMode(ACMode::HEAT);
                    else if (modeChoice == 3) acDevice->setMode(ACMode::DRY);
                    else {
                        std::cout << "Invalid mode choice.\n";
                        break;
                    }
                    std::cout << "AC mode set successfully.\n";
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << "\n";
                }
                break;
            }
            case 7: { // View AC status
                if (!acDevice) {
                    std::cout << "No device authenticated. Please authenticate first.\n";
                    break;
                }
                try {
                    auto info = acDevice->getDetailedInfo();
                    std::cout << "AC Details:\n" << info.dump(4) << "\n";
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << "\n";
                }
                break;
            }
            case 8: { // Exit
                std::cout << "Exiting...\n";
                return 0;
            }
            default:
                std::cout << "Invalid choice. Try again.\n";
        }
    }
}

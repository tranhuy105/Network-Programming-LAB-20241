#include <iostream>
#include <memory>
#include <string>
#include "Light.h"
#include "Fan.h"
#include "AC.h"
#include "CommandHandler.h"
#include "NetworkHandler.h"

// Helper function to display usage
void printUsage() {
    std::cout << "Usage: ./device --type <device_type> --id <device_id> --password <password> --port <port>\n";
    std::cout << "Supported device types: light, fan, ac\n";
}

int main(int argc, char* argv[]) {
    std::string deviceType, deviceId, password;
    int port = 0;

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--type" && i + 1 < argc) {
            deviceType = argv[++i];
        } else if (arg == "--id" && i + 1 < argc) {
            deviceId = argv[++i];
        } else if (arg == "--password" && i + 1 < argc) {
            password = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else {
            printUsage();
            return 1;
        }
    }

    // Validate required arguments
    if (deviceType.empty() || deviceId.empty() || password.empty() || port == 0) {
        std::cerr << "Error: Missing required arguments.\n";
        printUsage();
        return 1;
    }

    // Instantiate the appropriate device
    std::shared_ptr<Device> device;

    if (deviceType == "light") {
        device = std::make_shared<Light>(deviceId, password);
    } else if (deviceType == "fan") {
        device = std::make_shared<Fan>(deviceId, password);
    } else if (deviceType == "ac") {
        device = std::make_shared<AC>(deviceId, password);
    } else {
        std::cerr << "Error: Unsupported device type \"" << deviceType << "\".\n";
        printUsage();
        return 1;
    }

    // Create CommandHandler and NetworkHandler
    CommandHandler commandHandler(device, password);
    NetworkHandler networkHandler(commandHandler, port);

    // Start the network handler
    networkHandler.start();

    std::cout << "Device is running.\n";
    std::cout << "Type: " << deviceType << ", ID: " << deviceId << ", Port: " << port << "\n";
    std::cout << "Use netcat or other tools to send commands.\n";

    // Wait for exit command
    std::string exitCommand;
    while (exitCommand != "exit") {
        std::getline(std::cin, exitCommand);
    }

    // Stop the network handler
    networkHandler.stop();

    std::cout << "Device stopped.\n";
    return 0;
}

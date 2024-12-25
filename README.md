# Smart Device Management System

## System Overview
The system is a smart device management system designed to manage and interact with various IoT devices. It provides both a client-side UI and a device-side backend, enabling device discovery, state monitoring, and control functionalities. Below is a breakdown of the key components and their technical roles:

## 1. Device-Side Backend

### Core Components

#### Device Class (`Device.cpp`)
Each IoT device (e.g., AC, Fan, Light) is represented by the `Device` class, which handles:
- **State Management**: Whether the device is "on" or "off."
- **Runtime Tracking**: Tracks daily, monthly, and yearly runtime and cumulative power consumption in watt-seconds (`runtimeTracker`).
- **Timers**: Supports actions like "turn on" or "turn off" using a `TimerManager` to schedule actions.
- **Detailed Information**: Provides structured data (via `getDetailedInfo`) such as state, power consumption, and runtime statistics.

#### Authentication Manager (`AuthenticationManager.cpp`)
- Handles client authentication with password verification.
- Supports commands such as changing the password and verifying client identity for secure access to devices.

#### Command Handler (`CommandHandler.cpp`)
- Processes JSON-formatted commands received from the client (e.g., "turn on", "set timer").
- Routes commands to the appropriate `Device` methods or authentication logic.
- Ensures proper error handling and JSON responses to the client.

#### Network Handler (`NetworkHandler.cpp`)
- Manages communication protocols for the device:
  - **UDP for Discovery**:
    - Devices broadcast their presence on the network every 5 seconds using multicast.
    - Includes device type, ID, IP address, and TCP port for further interactions.
  - **TCP for Commands**:
    - Devices listen on a specified TCP port for client commands.
    - Processes client requests, verifies their structure (JSON), and delegates them to the `CommandHandler`.
    - Provides feedback to the client in JSON format (e.g., command success or error messages).

## 2. Client-Side Management System

### Core Features

#### Device Discovery
- The client listens for UDP broadcast messages sent by devices on the network.
- Extracts details such as device ID, type, IP address, and TCP port, displaying them in the client UI.

#### Device Control
- Interacts with the device over TCP using JSON-formatted requests (e.g., "turn on", "change password").
- Parses JSON responses from devices for real-time status updates.

#### Room and Home Management
- Provides a hierarchical structure for organizing devices into rooms and homes.
- CRUD (Create, Read, Update, Delete) operations for managing rooms and homes.
- Assigns discovered devices to specific rooms.

#### Runtime Monitoring
- Displays runtime statistics (daily, monthly, yearly) and cumulative power consumption for each device.

#### UI with Modular Components
- Uses ImGui for rendering an interactive user interface.
- Includes separate UI elements for:
  - Device authentication
  - Detailed monitoring (state, power, runtime, etc.)
  - Assigning devices to rooms
  - CRUD operations for room and home management

## 3. Key Technical Workflows

### Device Discovery Workflow
- Devices periodically broadcast their details over UDP multicast.
- The client listens for these broadcasts and updates its list of available devices.
- Discovered devices can be assigned to rooms for easier management.

### Command Execution Workflow
- The client sends a TCP command (e.g., "turn on", "set timer") to a specific device's IP and port.
- The device receives and parses the command via `NetworkHandler`.
- The command is passed to the `CommandHandler`, which validates and executes it.
- The device responds with a JSON status message, which the client processes and displays.

### Authentication Workflow
- Devices require clients to authenticate using a password before accessing secure features.
- The client sends an authentication request, which is validated by the `AuthenticationManager`.
- Upon successful authentication, the device issues a token for subsequent interactions.

## 4. Technologies Used

### Backend (Device-Side)
- **C++**: Core logic, device state management, and JSON processing.
- **Libraries**: nlohmann::json for structured communication.

### Networking
- **UDP**: For discovery.
- **TCP**: For reliable command processing.

### Multithreading
- Ensures asynchronous handling of device discovery and command processing.

### Frontend (Client-Side)
- **ImGui**: A lightweight, immediate-mode GUI framework for rendering UI components.
- **Modular Design**: For easy extension (e.g., adding cumulative power display).

This system combines efficient IoT device management with a clean modular design, ensuring scalability and ease of integration for future features. The discovery mechanism ensures dynamic device addition, while the use of JSON ensures interoperability between the client and device.

## Setup

### Prerequisites

1. **Ubuntu Version**: 20.04 or later recommended.
2. **Dependencies**:
  - **C++17** compiler (GCC or Clang).
  - **CMake** for build configuration.
  - `nlohmann/json` library for JSON parsing (included in the project).
  - **ImGui** for client UI (included in the project).

3. **Install Tools**:
  ```bash
  sudo apt update
  sudo apt install build-essential cmake net-tools
  ```

### Backend (Device-Side)

#### Build Backend:
```bash
cd device
make
```

#### Run Device Backend:
```bash
./device --type <device_type> --id <device_id> --password <password> --port <port>
```
Example:
```bash
./device --type light --id light01 --password secret --port 8080
```

Supported Device Types:
- light
- fan
- ac

### Frontend (Client-Side)

#### Build Client:
```bash
cd client
make
```

#### Run Client:
```bash
./out/main
```




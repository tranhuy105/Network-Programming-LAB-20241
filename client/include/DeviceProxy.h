#ifndef DEVICE_PROXY_H
#define DEVICE_PROXY_H

#include <string>
#include "../lib/json.hpp"

class DeviceProxy {
private:
    std::string id;
    std::string type;
    std::string ipAddress;
    int port;
    int socket;
    
    std::string clientId;
    
    int createSocket();
    void closeSocket();
    
protected:
    std::string token;
    nlohmann::json sendRequest(const nlohmann::json& request);
    
public:
    DeviceProxy(const std::string& id, const std::string& type, const std::string& ipAddress, const std::string& clientId, int port);
    virtual ~DeviceProxy();

    bool authenticate(const std::string& clientId, const std::string& password);
    bool isAuthenticated() const;
    void turnOn();
    void turnOff();
    void setTimer(int duration, const std::string& action);
    nlohmann::json getInfo();
    nlohmann::json getDetailedInfo();

    std::string getId() const { return id; }
    std::string getType() const { return type; }
    std::string getClientId() const { return clientId; }
    bool changePassword(const std::string& currentPassword, const std::string& newPassword);
};

#endif

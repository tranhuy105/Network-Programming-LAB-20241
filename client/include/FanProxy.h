#ifndef FAN_PROXY_H
#define FAN_PROXY_H

#include "DeviceProxy.h"

class FanProxy : public DeviceProxy {
public:
    FanProxy(const std::string& id, const std::string& ipAddress, const std::string& clientId,  int port)
        : DeviceProxy(id, "fan", ipAddress, clientId, port) {}

    void setSpeed(int speed);
};

#endif

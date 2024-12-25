#ifndef LIGHT_PROXY_H
#define LIGHT_PROXY_H

#include "DeviceProxy.h"

class LightProxy : public DeviceProxy {
public:
    LightProxy(const std::string& id, const std::string& ipAddress, const std::string& clientId,  int port)
        : DeviceProxy(id, "light", ipAddress, clientId, port) {}
};

#endif

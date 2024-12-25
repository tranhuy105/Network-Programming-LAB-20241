#ifndef AC_PROXY_H
#define AC_PROXY_H

#include "DeviceProxy.h"

enum class ACMode {
    COOL,
    HEAT,
    DRY
};

class ACProxy : public DeviceProxy {
public:
    ACProxy(const std::string& id, const std::string& ipAddress, const std::string& clientId, int port)
        : DeviceProxy(id, "ac", ipAddress, clientId, port) {}

    void setMode(ACMode mode);
    void setTemperature(int temperature);
};

#endif

#ifndef AC_H
#define AC_H

#include "Device.h"

enum class ACMode { COOL, HEAT, DRY };

class AC : public Device {
private:
    ACMode mode = ACMode::COOL;
    int temperature = 24; // Default temperature
public:
    AC(const std::string& id, const std::string& password);

    void turnOn() override;
    void turnOff() override;
    void setMode(ACMode mode);
    void setTemperature(int temperature);
    ACMode getMode() const;
    int getTemperature() const;

    std::string getType() const override {
        return "AC";
    }

    nlohmann::json getDetailedInfo() const override {
        nlohmann::json details = Device::getDetailedInfo();
        details["mode"] = (mode == ACMode::COOL ? "cool" : (mode == ACMode::HEAT ? "heat" : "dry"));
        details["temperature"] = temperature;
        return details;
    }
};

#endif

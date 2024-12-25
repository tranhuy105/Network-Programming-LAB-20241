#ifndef FAN_H
#define FAN_H

#include "Device.h"

class Fan : public Device {
private:
    int speed = 0; // Speed level (0: off, 1-3: speed levels)
public:
    Fan(const std::string& id, const std::string& password);

    void turnOn() override;
    void turnOff() override;
    void setSpeed(int speed);
    int getSpeed() const;

    std::string getType() const override {
        return "Fan";
    }

    nlohmann::json getDetailedInfo() const override {
        nlohmann::json details = Device::getDetailedInfo();
        details["speed"] = speed;
        return details;
    }
};

#endif

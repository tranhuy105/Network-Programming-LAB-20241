#ifndef LIGHT_H
#define LIGHT_H

#include "Device.h"

class Light : public Device {
public:
    Light(const std::string& id, const std::string& password);

    void turnOn() override;
    void turnOff() override;

    std::string getType() const override {
        return "Light";
    }
};

#endif
#ifndef MAIN_UI_H
#define MAIN_UI_H

#include <vector>
#include <memory>
#include <string>
#include "../include/DeviceProxy.h"
#include "../include/HomeManager.h"

// Hàm render chính
void renderMainUI(HomeManager& homeManager, std::vector<std::shared_ptr<DeviceProxy>>& devices, std::string& clientId);

#endif

// src/domain/model/DeviceInfo.hpp
#pragma once
#include <string>
#include <vector>
#include <Windows.h>

struct DeviceInfo {
    std::wstring              instanceId;
    std::vector<std::wstring> hardwareIds;
    std::vector<std::wstring> compatibleIds;
    GUID                      classGuid{};
    DWORD                     deviceStatus = 0;
    DWORD                     problemCode = 0;
};

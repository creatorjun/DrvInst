// src/application/port/in/IDeviceEnumerateUseCase.hpp
#pragma once
#include <vector>
#include "src/domain/model/DeviceInfo.hpp"

class IDeviceEnumerateUseCase {
public:
    virtual ~IDeviceEnumerateUseCase() = default;
    virtual std::vector<DeviceInfo> execute() = 0;
};

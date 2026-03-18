// src/application/port/out/IDeviceRepository.hpp
#pragma once
#include <vector>
#include "src/domain/model/DeviceInfo.hpp"

class IDeviceRepository {
public:
    virtual ~IDeviceRepository() = default;
    virtual std::vector<DeviceInfo> enumerateAllPresentDevices() = 0;
};

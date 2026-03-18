// tests/mock/MockDeviceRepository.hpp
#pragma once
#include <vector>
#include <functional>
#include "src/application/port/out/IDeviceRepository.hpp"

class MockDeviceRepository final : public IDeviceRepository {
public:
    std::function<std::vector<DeviceInfo>()> onEnumerate;

    std::vector<DeviceInfo> enumerateAllPresentDevices() override {
        if (onEnumerate)
            return onEnumerate();
        return {};
    }
};

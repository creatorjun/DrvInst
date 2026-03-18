// src/application/port/out/IDriverInstallPort.hpp
#pragma once
#include "src/domain/model/DeviceInfo.hpp"
#include "src/domain/model/DriverPackage.hpp"
#include "src/domain/model/InstallResult.hpp"

class IDriverInstallPort {
public:
    virtual ~IDriverInstallPort() = default;
    virtual InstallResult installDriver(
        const DeviceInfo& device,
        const DriverPackage& package) = 0;
};

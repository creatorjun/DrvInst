// src/application/port/in/IDriverInstallUseCase.hpp
#pragma once
#include <vector>
#include "src/domain/model/DriverPackage.hpp"
#include "src/domain/model/DeviceInfo.hpp"
#include "src/domain/model/InstallResult.hpp"

struct DriverInstallOutput {
    std::vector<InstallResult> results;
    bool                       needReboot = false;
};

class IDriverInstallUseCase {
public:
    virtual ~IDriverInstallUseCase() = default;
    virtual DriverInstallOutput execute(
        const std::vector<DriverPackage>& packages,
        const std::vector<DeviceInfo>& devices) = 0;
};

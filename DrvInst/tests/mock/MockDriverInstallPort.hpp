// tests/mock/MockDriverInstallPort.hpp
#pragma once
#include <functional>
#include "src/application/port/out/IDriverInstallPort.hpp"

class MockDriverInstallPort final : public IDriverInstallPort {
public:
    std::function<InstallResult(const DeviceInfo&, const DriverPackage&)> onInstall;

    InstallResult installDriver(
        const DeviceInfo&    device,
        const DriverPackage& package) override
    {
        if (onInstall)
            return onInstall(device, package);
        InstallResult r;
        r.status           = InstallStatus::Success;
        r.infPath          = package.infFullPath;
        r.deviceInstanceId = device.instanceId;
        r.win32ErrorCode   = 0;
        return r;
    }
};

// src/adapter/secondary/win32/Win32DriverInstallAdapter.hpp
#pragma once
#include <memory>
#include "src/application/port/out/IDriverInstallPort.hpp"
#include "src/application/port/out/ILoggerPort.hpp"

class Win32DriverInstallAdapter final : public IDriverInstallPort {
public:
    explicit Win32DriverInstallAdapter(std::shared_ptr<ILoggerPort> logger);
    InstallResult installDriver(
        const DeviceInfo& device,
        const DriverPackage& package) override;

private:
    std::shared_ptr<ILoggerPort> logger_;

    static bool infFileNameMatch(
        std::wstring_view drvInfoPath,
        std::wstring_view targetFileName);
};

// src/application/usecase/DriverInstallInteractor.cpp
#include <Windows.h>
#include "src/application/usecase/DriverInstallInteractor.hpp"
#include <format>

DriverInstallInteractor::DriverInstallInteractor(
    std::shared_ptr<IDriverInstallPort> installPort,
    std::shared_ptr<ILoggerPort>        logger)
    : installPort_(std::move(installPort))
    , logger_(std::move(logger))
{
}

DriverInstallOutput DriverInstallInteractor::execute(
    const std::vector<DriverPackage>& packages,
    const std::vector<DeviceInfo>& devices)
{
    logger_->log(LogLevel::Info,
        std::format(L"[Install] start - devices={} packages={}",
            devices.size(), packages.size()));

    DriverInstallOutput output;
    output.results.reserve(devices.size());

    for (const auto& device : devices) {
        auto match = DriverMatchService::findBestMatch(device, packages);

        if (!match.package) {
            logger_->log(LogLevel::Warn,
                std::format(L"[Install] no match: {}", device.instanceId));

            output.results.push_back(InstallResult{
                .status = InstallStatus::NoMatchingDevice,
                .deviceInstanceId = device.instanceId
                });
            continue;
        }

        logger_->log(LogLevel::Info,
            std::format(L"[Install] match: {} -> {} ({})",
                device.instanceId,
                match.package->infFileName,
                match.isExactMatch ? L"HWID" : L"CompatID"));

        auto result = installPort_->installDriver(device, *match.package);

        if (result.status == InstallStatus::NeedReboot)
            output.needReboot = true;

        logger_->log(
            result.status == InstallStatus::Failed ? LogLevel::Error : LogLevel::Info,
            std::format(L"[Install] result: {} - {}",
                device.instanceId, statusToString(result.status)));

        output.results.push_back(std::move(result));
    }

    const auto successCount = std::ranges::count_if(output.results,
        [](const InstallResult& r) {
            return r.status == InstallStatus::Success ||
                r.status == InstallStatus::NeedReboot;
        });

    logger_->log(LogLevel::Info,
        std::format(L"[Install] done - success={} total={} reboot={}",
            successCount, output.results.size(),
            output.needReboot ? L"yes" : L"no"));

    return output;
}

std::wstring DriverInstallInteractor::statusToString(InstallStatus status)
{
    switch (status) {
    case InstallStatus::Success:          return L"Success";
    case InstallStatus::AlreadyInstalled: return L"AlreadyInstalled";
    case InstallStatus::NeedReboot:       return L"NeedReboot";
    case InstallStatus::NoMatchingDevice: return L"NoMatchingDevice";
    case InstallStatus::Failed:           return L"Failed";
    default:                              return L"Unknown";
    }
}

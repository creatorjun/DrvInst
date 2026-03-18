// src/application/usecase/DeviceEnumerateInteractor.cpp
#include <Windows.h>
#include <cfgmgr32.h>
#include "src/application/usecase/DeviceEnumerateInteractor.hpp"
#include <format>

DeviceEnumerateInteractor::DeviceEnumerateInteractor(
    std::shared_ptr<IDeviceRepository> deviceRepo,
    std::shared_ptr<ILoggerPort>       logger)
    : deviceRepo_(std::move(deviceRepo))
    , logger_(std::move(logger))
{
}

std::vector<DeviceInfo> DeviceEnumerateInteractor::execute()
{
    logger_->log(LogLevel::Info, L"[Device Enum] start");

    auto all = deviceRepo_->enumerateAllPresentDevices();

    std::vector<DeviceInfo> filtered;
    filtered.reserve(all.size());

    for (auto& dev : all) {
        if (needsInstall(dev.problemCode)) {
            logger_->log(LogLevel::Debug,
                std::format(L"[Device Enum] need install: {} (problem={:#x})",
                    dev.instanceId, dev.problemCode));
            filtered.push_back(std::move(dev));
        }
    }

    logger_->log(LogLevel::Info,
        std::format(L"[Device Enum] total={} / need-install={}",
            all.size(), filtered.size()));

    return filtered;
}

bool DeviceEnumerateInteractor::needsInstall(DWORD problemCode)
{
    switch (problemCode) {
    case CM_PROB_NOT_CONFIGURED:
    case CM_PROB_REINSTALL:
    case CM_PROB_FAILED_INSTALL:
    case CM_PROB_NEED_CLASS_CONFIG:
        return true;
    default:
        return false;
    }
}

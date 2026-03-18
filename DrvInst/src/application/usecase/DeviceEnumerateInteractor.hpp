// src/application/usecase/DeviceEnumerateInteractor.hpp
#pragma once
#include <Windows.h>
#include <memory>
#include <vector>
#include "src/application/port/in/IDeviceEnumerateUseCase.hpp"
#include "src/application/port/out/IDeviceRepository.hpp"
#include "src/application/port/out/ILoggerPort.hpp"

class DeviceEnumerateInteractor final : public IDeviceEnumerateUseCase {
public:
    explicit DeviceEnumerateInteractor(
        std::shared_ptr<IDeviceRepository> deviceRepo,
        std::shared_ptr<ILoggerPort>       logger);

    std::vector<DeviceInfo> execute() override;

private:
    std::shared_ptr<IDeviceRepository> deviceRepo_;
    std::shared_ptr<ILoggerPort>       logger_;

    static bool needsInstall(DWORD problemCode);
};

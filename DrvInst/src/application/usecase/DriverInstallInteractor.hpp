// src/application/usecase/DriverInstallInteractor.hpp
#pragma once
#include <memory>
#include <vector>
#include "src/application/port/in/IDriverInstallUseCase.hpp"
#include "src/application/port/out/IDriverInstallPort.hpp"
#include "src/application/port/out/ILoggerPort.hpp"
#include "src/domain/service/DriverMatchService.hpp"

class DriverInstallInteractor final : public IDriverInstallUseCase {
public:
    explicit DriverInstallInteractor(
        std::shared_ptr<IDriverInstallPort> installPort,
        std::shared_ptr<ILoggerPort>        logger);

    DriverInstallOutput execute(
        const std::vector<DriverPackage>& packages,
        const std::vector<DeviceInfo>& devices) override;

private:
    std::shared_ptr<IDriverInstallPort> installPort_;
    std::shared_ptr<ILoggerPort>        logger_;

    static std::wstring statusToString(InstallStatus status);
};

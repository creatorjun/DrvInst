// src/application/usecase/InfImportInteractor.hpp
#pragma once
#include <memory>
#include <vector>
#include "src/application/port/in/IInfImportUseCase.hpp"
#include "src/application/port/out/IDriverStorePort.hpp"
#include "src/application/port/out/ILoggerPort.hpp"

class InfImportInteractor final : public IInfImportUseCase {
public:
    explicit InfImportInteractor(
        std::shared_ptr<IDriverStorePort> storePort,
        std::shared_ptr<ILoggerPort>      logger);

    std::vector<DriverPackage> execute(
        std::vector<DriverPackage> packages) override;

private:
    std::shared_ptr<IDriverStorePort> storePort_;
    std::shared_ptr<ILoggerPort>      logger_;
};

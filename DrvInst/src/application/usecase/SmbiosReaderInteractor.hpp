// src/application/usecase/SmbiosReaderInteractor.hpp
#pragma once
#include <memory>
#include "src/application/port/in/ISmbiosReaderUseCase.hpp"
#include "src/application/port/out/ISmbiosPort.hpp"
#include "src/application/port/out/ILoggerPort.hpp"

class SmbiosReaderInteractor final : public ISmbiosReaderUseCase {
public:
    explicit SmbiosReaderInteractor(
        std::shared_ptr<ISmbiosPort> smbiosPort,
        std::shared_ptr<ILoggerPort> logger);

    MotherboardInfo execute() override;

private:
    std::shared_ptr<ISmbiosPort> smbiosPort_;
    std::shared_ptr<ILoggerPort> logger_;
};

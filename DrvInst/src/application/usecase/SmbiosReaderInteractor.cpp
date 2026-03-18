// src/application/usecase/SmbiosReaderInteractor.cpp
#include <Windows.h>
#include "src/application/usecase/SmbiosReaderInteractor.hpp"
#include <format>

SmbiosReaderInteractor::SmbiosReaderInteractor(
    std::shared_ptr<ISmbiosPort> smbiosPort,
    std::shared_ptr<ILoggerPort> logger)
    : smbiosPort_(std::move(smbiosPort))
    , logger_(std::move(logger))
{
}

MotherboardInfo SmbiosReaderInteractor::execute()
{
    logger_->log(LogLevel::Info, L"[SMBIOS] read start");

    auto result = smbiosPort_->readMotherboardInfo();
    if (!result) {
        logger_->log(LogLevel::Error,
            std::format(L"[SMBIOS] read failed - error={}", result.error()));
        return MotherboardInfo{ .isValid = false };
    }

    auto& info = result.value();
    if (info.productName.empty()) {
        logger_->log(LogLevel::Error, L"[SMBIOS] productName is empty");
        info.isValid = false;
        return info;
    }

    logger_->log(LogLevel::Info,
        std::format(L"[SMBIOS] Manufacturer: {}", info.manufacturer));
    logger_->log(LogLevel::Info,
        std::format(L"[SMBIOS] ProductName : {}", info.productName));

    info.isValid = true;
    return info;
}

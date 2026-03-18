// src/application/usecase/InfImportInteractor.cpp
#include <Windows.h>
#include "src/application/usecase/InfImportInteractor.hpp"
#include <format>

InfImportInteractor::InfImportInteractor(
    std::shared_ptr<IDriverStorePort> storePort,
    std::shared_ptr<ILoggerPort>      logger)
    : storePort_(std::move(storePort))
    , logger_(std::move(logger))
{
}

std::vector<DriverPackage> InfImportInteractor::execute(
    std::vector<DriverPackage> packages)
{
    for (auto& pkg : packages) {
        if (storePort_->isAlreadyImported(pkg.infFileName)) {
            pkg.alreadyImported = true;
            logger_->log(LogLevel::Info,
                std::format(L"[INF Import] skip (already imported): {}", pkg.infFileName));
            continue;
        }

        auto result = storePort_->importInf(pkg.infFullPath);
        if (!result) {
            logger_->log(LogLevel::Error,
                std::format(L"[INF Import] failed: {} (error={})",
                    pkg.infFileName, result.error()));
            continue;
        }

        logger_->log(LogLevel::Info,
            std::format(L"[INF Import] ok: {} -> {}", pkg.infFileName, result.value()));
    }

    return packages;
}

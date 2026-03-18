// src/application/usecase/InfSearchInteractor.cpp
#include <Windows.h>
#include "src/application/usecase/InfSearchInteractor.hpp"
#include <format>

InfSearchInteractor::InfSearchInteractor(
    std::shared_ptr<IInfSearchPort> searchPort,
    std::shared_ptr<ILoggerPort>    logger)
    : searchPort_(std::move(searchPort))
    , logger_(std::move(logger))
{
}

std::vector<DriverPackage> InfSearchInteractor::execute(
    std::wstring_view basePath,
    std::wstring_view boardName)
{
    const std::wstring searchPath =
        std::wstring(basePath) + L"\\" + std::wstring(boardName);

    logger_->log(LogLevel::Info,
        std::format(L"[INF Search] path: {}", searchPath));

    if (!searchPort_->directoryExists(searchPath)) {
        logger_->log(LogLevel::Error,
            std::format(L"[INF Search] directory not found: {}", searchPath));
        return {};
    }

    auto infPaths = searchPort_->findInfFiles(searchPath);
    if (infPaths.empty()) {
        logger_->log(LogLevel::Warn, L"[INF Search] no INF files found");
        return {};
    }

    std::vector<DriverPackage> packages;
    packages.reserve(infPaths.size());

    for (auto& path : infPaths) {
        DriverPackage pkg;
        pkg.infFullPath = std::move(path);
        pkg.infFileName = extractFileName(pkg.infFullPath);
        packages.push_back(std::move(pkg));

        logger_->log(LogLevel::Debug,
            std::format(L"[INF Search] found: {}", packages.back().infFileName));
    }

    logger_->log(LogLevel::Info,
        std::format(L"[INF Search] total={}", packages.size()));

    return packages;
}

std::wstring InfSearchInteractor::extractFileName(std::wstring_view fullPath)
{
    const auto pos = fullPath.find_last_of(L"\\/");
    return (pos == std::wstring_view::npos)
        ? std::wstring(fullPath)
        : std::wstring(fullPath.substr(pos + 1));
}

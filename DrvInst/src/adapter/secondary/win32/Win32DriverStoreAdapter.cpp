// src/adapter/secondary/win32/Win32DriverStoreAdapter.cpp
#include <Windows.h>
#include <setupapi.h>
#include "src/adapter/secondary/win32/Win32DriverStoreAdapter.hpp"
#include <format>

#pragma comment(lib, "setupapi.lib")

Win32DriverStoreAdapter::Win32DriverStoreAdapter(std::shared_ptr<ILoggerPort> logger)
    : logger_(std::move(logger))
{
}

std::expected<std::wstring, DWORD> Win32DriverStoreAdapter::importInf(
    std::wstring_view srcInfPath)
{
    WCHAR destInfName[MAX_PATH]{};

    const BOOL ok = SetupCopyOEMInfW(
        srcInfPath.data(),
        nullptr,
        SPOST_PATH,
        0,
        destInfName,
        MAX_PATH,
        nullptr,
        nullptr);

    if (!ok) {
        const DWORD err = GetLastError();
        if (err == ERROR_FILE_EXISTS) {
            logger_->log(LogLevel::Info,
                std::format(L"[DriverStore] already exists: {}", srcInfPath));
            return std::wstring(srcInfPath);
        }
        return std::unexpected(err);
    }

    return std::wstring(destInfName);
}

bool Win32DriverStoreAdapter::isAlreadyImported(std::wstring_view infFileName)
{
    WCHAR winDir[MAX_PATH]{};
    if (!GetWindowsDirectoryW(winDir, MAX_PATH))
        return false;

    const std::wstring infPath =
        std::wstring(winDir) + L"\\INF\\" + std::wstring(infFileName);

    const DWORD attr = GetFileAttributesW(infPath.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES) &&
        !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

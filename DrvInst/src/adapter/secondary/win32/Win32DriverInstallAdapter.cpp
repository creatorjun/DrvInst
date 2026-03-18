// src/adapter/secondary/win32/Win32DriverInstallAdapter.cpp
#include <Windows.h>
#include <setupapi.h>
#include <newdev.h>
#include "src/adapter/secondary/win32/Win32DriverInstallAdapter.hpp"
#include <format>
#include <algorithm>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "newdev.lib")

Win32DriverInstallAdapter::Win32DriverInstallAdapter(
    std::shared_ptr<ILoggerPort> logger)
    : logger_(std::move(logger))
{
}

InstallResult Win32DriverInstallAdapter::installDriver(
    const DeviceInfo& device,
    const DriverPackage& package)
{
    InstallResult result;
    result.deviceInstanceId = device.instanceId;
    result.infPath = package.infFullPath;

    HDEVINFO hDevInfo = SetupDiCreateDeviceInfoList(nullptr, nullptr);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        result.win32ErrorCode = GetLastError();
        result.errorMessage = L"SetupDiCreateDeviceInfoList failed";
        result.status = InstallStatus::Failed;
        return result;
    }

    SP_DEVINFO_DATA devInfoData{};
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    if (!SetupDiOpenDeviceInfoW(
        hDevInfo,
        device.instanceId.c_str(),
        nullptr, 0,
        &devInfoData))
    {
        result.win32ErrorCode = GetLastError();
        result.errorMessage = L"SetupDiOpenDeviceInfoW failed";
        result.status = InstallStatus::Failed;
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return result;
    }

    if (!SetupDiBuildDriverInfoList(hDevInfo, &devInfoData, SPDIT_COMPATDRIVER)) {
        result.win32ErrorCode = GetLastError();
        result.errorMessage = L"SetupDiBuildDriverInfoList failed";
        result.status = InstallStatus::Failed;
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return result;
    }

    SP_DRVINFO_DATA_W drvInfoData{};
    drvInfoData.cbSize = sizeof(SP_DRVINFO_DATA_W);
    bool found = false;

    for (DWORD i = 0;
        SetupDiEnumDriverInfoW(
            hDevInfo, &devInfoData, SPDIT_COMPATDRIVER, i, &drvInfoData);
        ++i)
    {
        SP_DRVINFO_DETAIL_DATA_W detail{};
        detail.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA_W);

        SetupDiGetDriverInfoDetailW(
            hDevInfo, &devInfoData, &drvInfoData,
            &detail, sizeof(detail), nullptr);

        if (infFileNameMatch(detail.InfFileName, package.infFileName)) {
            found = true;
            break;
        }
    }

    if (!found) {
        result.errorMessage = L"INF not found in driver list";
        result.status = InstallStatus::NoMatchingDevice;
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return result;
    }

    BOOL needReboot = FALSE;
    const BOOL ok = DiInstallDevice(
        nullptr,
        hDevInfo,
        &devInfoData,
        &drvInfoData,
        0,
        &needReboot);

    SetupDiDestroyDeviceInfoList(hDevInfo);

    if (!ok) {
        result.win32ErrorCode = GetLastError();
        result.errorMessage =
            std::format(L"DiInstallDevice failed (error={})", result.win32ErrorCode);
        result.status = InstallStatus::Failed;
        return result;
    }

    result.status = needReboot ? InstallStatus::NeedReboot : InstallStatus::Success;
    return result;
}

bool Win32DriverInstallAdapter::infFileNameMatch(
    std::wstring_view drvInfoPath,
    std::wstring_view targetFileName)
{
    auto normalize = [](std::wstring_view s) -> std::wstring {
        const auto pos = s.find_last_of(L"\\/");
        std::wstring name = (pos == std::wstring_view::npos)
            ? std::wstring(s)
            : std::wstring(s.substr(pos + 1));
        std::transform(name.begin(), name.end(), name.begin(), ::towlower);
        return name;
        };

    return normalize(drvInfoPath) == normalize(targetFileName);
}

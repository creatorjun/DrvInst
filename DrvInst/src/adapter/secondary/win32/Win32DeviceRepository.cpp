// src/adapter/secondary/win32/Win32DeviceRepository.cpp
#include <Windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include "src/adapter/secondary/win32/Win32DeviceRepository.hpp"
#include <format>
#include <vector>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "cfgmgr32.lib")

Win32DeviceRepository::Win32DeviceRepository(std::shared_ptr<ILoggerPort> logger)
    : logger_(std::move(logger))
{
}

std::vector<DeviceInfo> Win32DeviceRepository::enumerateAllPresentDevices()
{
    std::vector<DeviceInfo> devices;

    HDEVINFO hDevInfo = SetupDiGetClassDevsW(
        nullptr, nullptr, nullptr,
        DIGCF_ALLCLASSES | DIGCF_PRESENT);

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        logger_->log(LogLevel::Error,
            std::format(L"[DeviceRepo] SetupDiGetClassDevs failed: {}", GetLastError()));
        return devices;
    }

    SP_DEVINFO_DATA devInfoData{};
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD index = 0;
        SetupDiEnumDeviceInfo(hDevInfo, index, &devInfoData);
        ++index)
    {
        DeviceInfo dev;

        WCHAR instanceId[MAX_DEVICE_ID_LEN]{};
        if (SetupDiGetDeviceInstanceIdW(
            hDevInfo, &devInfoData,
            instanceId, MAX_DEVICE_ID_LEN, nullptr))
        {
            dev.instanceId = instanceId;
        }

        auto getMultiSz = [&](DWORD prop) -> std::vector<std::wstring> {
            DWORD dataType = 0, reqSize = 0;
            SetupDiGetDeviceRegistryPropertyW(
                hDevInfo, &devInfoData, prop,
                &dataType, nullptr, 0, &reqSize);

            if (reqSize == 0) return {};

            std::vector<BYTE> buf(reqSize);
            if (!SetupDiGetDeviceRegistryPropertyW(
                hDevInfo, &devInfoData, prop,
                &dataType, buf.data(), reqSize, nullptr))
                return {};

            return parseMultiSz(buf.data(), reqSize);
            };

        dev.hardwareIds = getMultiSz(SPDRP_HARDWAREID);
        dev.compatibleIds = getMultiSz(SPDRP_COMPATIBLEIDS);
        dev.classGuid = devInfoData.ClassGuid;

        DWORD status = 0, problemCode = 0;
        if (CM_Get_DevNode_Status(
            &status, &problemCode,
            devInfoData.DevInst, 0) == CR_SUCCESS)
        {
            dev.deviceStatus = status;
            dev.problemCode = problemCode;
        }

        devices.push_back(std::move(dev));
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

    logger_->log(LogLevel::Debug,
        std::format(L"[DeviceRepo] enumerated: {}", devices.size()));

    return devices;
}

std::vector<std::wstring> Win32DeviceRepository::parseMultiSz(
    const BYTE* buf, DWORD size)
{
    std::vector<std::wstring> result;
    const wchar_t* ptr = reinterpret_cast<const wchar_t*>(buf);
    const wchar_t* end = ptr + size / sizeof(wchar_t);

    while (ptr < end && *ptr != L'\0') {
        std::wstring s{ ptr };
        ptr += s.size() + 1;
        result.push_back(std::move(s));
    }
    return result;
}

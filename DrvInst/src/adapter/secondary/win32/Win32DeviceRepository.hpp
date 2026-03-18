// src/adapter/secondary/win32/Win32DeviceRepository.hpp
#pragma once
#include <Windows.h>
#include <memory>
#include <vector>
#include "src/application/port/out/IDeviceRepository.hpp"
#include "src/application/port/out/ILoggerPort.hpp"

class Win32DeviceRepository final : public IDeviceRepository {
public:
    explicit Win32DeviceRepository(std::shared_ptr<ILoggerPort> logger);
    std::vector<DeviceInfo> enumerateAllPresentDevices() override;

private:
    std::shared_ptr<ILoggerPort> logger_;

    static std::vector<std::wstring> parseMultiSz(const BYTE* buf, DWORD size);
};

// src/adapter/secondary/win32/Win32SmbiosAdapter.hpp
#pragma once
#include <memory>
#include "src/application/port/out/ISmbiosPort.hpp"
#include "src/application/port/out/ILoggerPort.hpp"

class Win32SmbiosAdapter final : public ISmbiosPort {
public:
    explicit Win32SmbiosAdapter(std::shared_ptr<ILoggerPort> logger);
    std::expected<MotherboardInfo, DWORD> readMotherboardInfo() override;

private:
    std::shared_ptr<ILoggerPort> logger_;

    static std::wstring ansiToWide(const char* str);
    static std::wstring getSmBiosString(const BYTE* stringSection, BYTE index);
};

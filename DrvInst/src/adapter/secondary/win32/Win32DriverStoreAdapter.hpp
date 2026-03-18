// src/adapter/secondary/win32/Win32DriverStoreAdapter.hpp
#pragma once
#include <memory>
#include "src/application/port/out/IDriverStorePort.hpp"
#include "src/application/port/out/ILoggerPort.hpp"

class Win32DriverStoreAdapter final : public IDriverStorePort {
public:
    explicit Win32DriverStoreAdapter(std::shared_ptr<ILoggerPort> logger);
    std::expected<std::wstring, DWORD> importInf(std::wstring_view srcInfPath) override;
    bool isAlreadyImported(std::wstring_view infFileName) override;

private:
    std::shared_ptr<ILoggerPort> logger_;
};

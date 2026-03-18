// src/adapter/secondary/win32/Win32InfSearchAdapter.hpp
#pragma once
#include <memory>
#include "src/application/port/out/IInfSearchPort.hpp"
#include "src/application/port/out/ILoggerPort.hpp"

class Win32InfSearchAdapter final : public IInfSearchPort {
public:
    explicit Win32InfSearchAdapter(std::shared_ptr<ILoggerPort> logger);
    std::vector<std::wstring> findInfFiles(std::wstring_view rootPath) override;
    bool directoryExists(std::wstring_view path) override;

private:
    std::shared_ptr<ILoggerPort> logger_;

    static bool endsWithInf(std::wstring_view name);
};

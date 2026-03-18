// src/adapter/secondary/logger/ConsoleLoggerAdapter.hpp
#pragma once
#include <Windows.h>
#include <memory>
#include "src/application/port/out/ILoggerPort.hpp"

class ConsoleLoggerAdapter final : public ILoggerPort {
public:
    ConsoleLoggerAdapter();
    void log(LogLevel level, std::wstring_view message) override;

private:
    HANDLE hConsole_;

    static std::wstring_view levelToString(LogLevel level);
    static WORD              levelToColor(LogLevel level);
};

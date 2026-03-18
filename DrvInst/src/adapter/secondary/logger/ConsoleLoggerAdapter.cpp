// src/adapter/secondary/logger/ConsoleLoggerAdapter.cpp
#include <Windows.h>
#include "src/adapter/secondary/logger/ConsoleLoggerAdapter.hpp"
#include <format>

ConsoleLoggerAdapter::ConsoleLoggerAdapter()
    : hConsole_(GetStdHandle(STD_OUTPUT_HANDLE))
{
}

void ConsoleLoggerAdapter::log(LogLevel level, std::wstring_view message)
{
    SYSTEMTIME st{};
    GetLocalTime(&st);

    const std::wstring line = std::format(
        L"{:02}:{:02}:{:02}.{:03} [{:5}] {}\n",
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        levelToString(level),
        message);

    SetConsoleTextAttribute(hConsole_, levelToColor(level));
    WriteConsoleW(hConsole_, line.c_str(),
        static_cast<DWORD>(line.size()), nullptr, nullptr);
    SetConsoleTextAttribute(hConsole_,
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

std::wstring_view ConsoleLoggerAdapter::levelToString(LogLevel level)
{
    switch (level) {
    case LogLevel::Debug: return L"DEBUG";
    case LogLevel::Info:  return L"INFO ";
    case LogLevel::Warn:  return L"WARN ";
    case LogLevel::Error: return L"ERROR";
    default:              return L"?????";
    }
}

WORD ConsoleLoggerAdapter::levelToColor(LogLevel level)
{
    switch (level) {
    case LogLevel::Debug:
        return FOREGROUND_INTENSITY;
    case LogLevel::Info:
        return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    case LogLevel::Warn:
        return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    case LogLevel::Error:
        return FOREGROUND_RED | FOREGROUND_INTENSITY;
    default:
        return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }
}

// src/application/port/out/ILoggerPort.hpp
#pragma once
#include <string>

enum class LogLevel { Debug, Info, Warn, Error };

class ILoggerPort {
public:
    virtual ~ILoggerPort() = default;
    virtual void log(LogLevel level, std::wstring_view message) = 0;
};

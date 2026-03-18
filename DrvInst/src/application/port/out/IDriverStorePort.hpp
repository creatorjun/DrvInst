// src/application/port/out/IDriverStorePort.hpp
#pragma once
#include <Windows.h>
#include <expected>
#include <string>

class IDriverStorePort {
public:
    virtual ~IDriverStorePort() = default;
    virtual std::expected<std::wstring, DWORD> importInf(std::wstring_view srcInfPath) = 0;
    virtual bool isAlreadyImported(std::wstring_view infFileName) = 0;
};

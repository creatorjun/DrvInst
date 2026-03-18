// src/application/port/out/IInfSearchPort.hpp
#pragma once
#include <string>
#include <vector>

class IInfSearchPort {
public:
    virtual ~IInfSearchPort() = default;
    virtual std::vector<std::wstring> findInfFiles(std::wstring_view rootPath) = 0;
    virtual bool directoryExists(std::wstring_view path) = 0;
};

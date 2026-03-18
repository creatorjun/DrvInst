// tests/mock/MockInfSearchPort.hpp
#pragma once
#include <string>
#include <vector>
#include <functional>
#include "src/application/port/out/IInfSearchPort.hpp"

class MockInfSearchPort final : public IInfSearchPort {
public:
    std::function<std::vector<std::wstring>(std::wstring_view)> onFind;
    std::function<bool(std::wstring_view)>                      onExists;

    std::vector<std::wstring> findInfFiles(std::wstring_view rootPath) override {
        if (onFind)
            return onFind(rootPath);
        return {};
    }

    bool directoryExists(std::wstring_view path) override {
        if (onExists)
            return onExists(path);
        return false;
    }
};

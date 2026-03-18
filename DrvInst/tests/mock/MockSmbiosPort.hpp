// tests/mock/MockSmbiosPort.hpp
#pragma once
#include <Windows.h>
#include <expected>
#include <functional>
#include "src/application/port/out/ISmbiosPort.hpp"

class MockSmbiosPort final : public ISmbiosPort {
public:
    std::function<std::expected<MotherboardInfo, DWORD>()> onRead;

    std::expected<MotherboardInfo, DWORD> readMotherboardInfo() override {
        if (onRead)
            return onRead();
        MotherboardInfo info;
        info.manufacturer = L"MockVendor";
        info.productName  = L"MockBoard";
        info.version      = L"1.0";
        info.isValid      = true;
        return info;
    }
};

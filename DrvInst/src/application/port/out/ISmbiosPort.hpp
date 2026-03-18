// src/application/port/out/ISmbiosPort.hpp
#pragma once
#include <Windows.h>
#include <expected>
#include "src/domain/model/MotherboardInfo.hpp"

class ISmbiosPort {
public:
    virtual ~ISmbiosPort() = default;
    virtual std::expected<MotherboardInfo, DWORD> readMotherboardInfo() = 0;
};

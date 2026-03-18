// src/domain/model/MotherboardInfo.hpp
#pragma once
#include <string>

struct MotherboardInfo {
    std::wstring manufacturer;
    std::wstring productName;
    std::wstring version;
    bool         isValid = false;
};

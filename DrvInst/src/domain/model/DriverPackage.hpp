// src/domain/model/DriverPackage.hpp
#pragma once
#include <string>
#include <vector>

struct DriverPackage {
    std::wstring              infFullPath;
    std::wstring              infFileName;
    std::vector<std::wstring> hwIds;
    bool                      alreadyImported = false;
};

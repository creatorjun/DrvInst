// src/domain/model/InstallResult.hpp
#pragma once
#include <string>
#include <Windows.h>

enum class InstallStatus {
    Success,
    AlreadyInstalled,
    NeedReboot,
    NoMatchingDevice,
    Failed
};

struct InstallResult {
    InstallStatus status = InstallStatus::Failed;
    std::wstring  infPath;
    std::wstring  deviceInstanceId;
    DWORD         win32ErrorCode = 0;
    std::wstring  errorMessage;
};

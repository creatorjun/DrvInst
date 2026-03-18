// src/domain/service/DriverMatchService.hpp
#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include "src/domain/model/DeviceInfo.hpp"
#include "src/domain/model/DriverPackage.hpp"

class DriverMatchService {
public:
    struct MatchResult {
        const DriverPackage* package = nullptr;
        bool                 isExactMatch = false;
    };

    static MatchResult findBestMatch(
        const DeviceInfo& device,
        const std::vector<DriverPackage>& packages)
    {
        for (const auto& pkg : packages) {
            for (const auto& devId : device.hardwareIds) {
                for (const auto& infId : pkg.hwIds) {
                    if (hwIdMatch(devId, infId))
                        return { &pkg, true };
                }
            }
        }
        for (const auto& pkg : packages) {
            for (const auto& devId : device.compatibleIds) {
                for (const auto& infId : pkg.hwIds) {
                    if (compatMatch(devId, infId))
                        return { &pkg, false };
                }
            }
        }
        return { nullptr, false };
    }

private:
    static std::wstring normalize(std::wstring_view s)
    {
        std::wstring result{ s };
        std::transform(result.begin(), result.end(), result.begin(), ::towlower);
        return result;
    }

    static bool hwIdMatch(std::wstring_view deviceId, std::wstring_view infId)
    {
        return normalize(deviceId) == normalize(infId);
    }

    static bool compatMatch(std::wstring_view deviceId, std::wstring_view infId)
    {
        const auto d = normalize(deviceId);
        const auto i = normalize(infId);
        return d.starts_with(i) || i.starts_with(d);
    }
};

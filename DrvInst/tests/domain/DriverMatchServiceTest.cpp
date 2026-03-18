// tests/domain/DriverMatchServiceTest.cpp
#include <Windows.h>
#include <cassert>
#include <string>
#include <vector>
#include "src/domain/model/DeviceInfo.hpp"
#include "src/domain/model/DriverPackage.hpp"
#include "src/domain/service/DriverMatchService.hpp"

static void test_exact_hwid_match_case_insensitive() {
    DeviceInfo dev;
    dev.hardwareIds = { L"USB\\VID_1234&PID_ABCD" };

    DriverPackage pkg;
    pkg.infFullPath  = L"Y:\\KdicSetup\\Drivers\\Board\\test.inf";
    pkg.infFileName  = L"test.inf";
    pkg.hwIds        = { L"usb\\vid_1234&pid_abcd" };

    std::vector<DriverPackage> packages = { pkg };
    const auto result = DriverMatchService::findBestMatch(dev, packages);

    assert(result.package != nullptr);
    assert(result.isExactMatch == true);
}

static void test_no_match_returns_null() {
    DeviceInfo dev;
    dev.hardwareIds  = { L"PCI\\VEN_8086&DEV_0001" };
    dev.compatibleIds = { L"PCI\\VEN_8086" };

    DriverPackage pkg;
    pkg.hwIds = { L"USB\\VID_9999&PID_0001" };

    std::vector<DriverPackage> packages = { pkg };
    const auto result = DriverMatchService::findBestMatch(dev, packages);

    assert(result.package == nullptr);
    assert(result.isExactMatch == false);
}

static void test_compat_prefix_match() {
    DeviceInfo dev;
    dev.hardwareIds   = { L"USB\\VID_DEAD&PID_BEEF&REV_0100" };
    dev.compatibleIds = { L"USB\\CLASS_FF&SUBCLASS_00" };

    DriverPackage pkg;
    pkg.hwIds = { L"USB\\CLASS_FF" };

    std::vector<DriverPackage> packages = { pkg };
    const auto result = DriverMatchService::findBestMatch(dev, packages);

    assert(result.package != nullptr);
    assert(result.isExactMatch == false);
}

static void test_hwid_preferred_over_compat() {
    DeviceInfo dev;
    dev.hardwareIds   = { L"USB\\VID_1111&PID_2222" };
    dev.compatibleIds = { L"USB\\CLASS_FF" };

    DriverPackage exactPkg;
    exactPkg.hwIds = { L"USB\\VID_1111&PID_2222" };

    DriverPackage compatPkg;
    compatPkg.hwIds = { L"USB\\CLASS_FF" };

    std::vector<DriverPackage> packages = { exactPkg, compatPkg };
    const auto result = DriverMatchService::findBestMatch(dev, packages);

    assert(result.package == &packages[0]);
    assert(result.isExactMatch == true);
}

static void test_multiple_packages_first_exact_wins() {
    DeviceInfo dev;
    dev.hardwareIds = { L"PCI\\VEN_10DE&DEV_2204" };

    DriverPackage pkgA;
    pkgA.hwIds = { L"PCI\\VEN_10DE&DEV_0000" };

    DriverPackage pkgB;
    pkgB.hwIds = { L"PCI\\VEN_10DE&DEV_2204" };

    std::vector<DriverPackage> packages = { pkgA, pkgB };
    const auto result = DriverMatchService::findBestMatch(dev, packages);

    assert(result.package == &packages[1]);
    assert(result.isExactMatch == true);
}

static void test_empty_packages_returns_null() {
    DeviceInfo dev;
    dev.hardwareIds = { L"USB\\VID_1234&PID_5678" };

    std::vector<DriverPackage> packages;
    const auto result = DriverMatchService::findBestMatch(dev, packages);

    assert(result.package == nullptr);
}

static void test_empty_device_ids_returns_null() {
    DeviceInfo dev;

    DriverPackage pkg;
    pkg.hwIds = { L"USB\\VID_1234&PID_5678" };

    std::vector<DriverPackage> packages = { pkg };
    const auto result = DriverMatchService::findBestMatch(dev, packages);

    assert(result.package == nullptr);
}

int wmain() {
    test_exact_hwid_match_case_insensitive();
    test_no_match_returns_null();
    test_compat_prefix_match();
    test_hwid_preferred_over_compat();
    test_multiple_packages_first_exact_wins();
    test_empty_packages_returns_null();
    test_empty_device_ids_returns_null();
    return 0;
}

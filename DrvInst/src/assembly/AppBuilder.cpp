// src/assembly/AppBuilder.cpp
#include <Windows.h>
#include "src/assembly/AppBuilder.hpp"
#include "src/adapter/secondary/logger/ConsoleLoggerAdapter.hpp"
#include "src/adapter/secondary/win32/Win32SmbiosAdapter.hpp"
#include "src/adapter/secondary/win32/Win32InfSearchAdapter.hpp"
#include "src/adapter/secondary/win32/Win32DriverStoreAdapter.hpp"
#include "src/adapter/secondary/win32/Win32DeviceRepository.hpp"
#include "src/adapter/secondary/win32/Win32DriverInstallAdapter.hpp"
#include "src/application/usecase/SmbiosReaderInteractor.hpp"
#include "src/application/usecase/InfSearchInteractor.hpp"
#include "src/application/usecase/InfImportInteractor.hpp"
#include "src/application/usecase/DeviceEnumerateInteractor.hpp"
#include "src/application/usecase/DriverInstallInteractor.hpp"

std::unique_ptr<InstallViewModel> AppBuilder::build()
{
    auto logger = std::make_shared<ConsoleLoggerAdapter>();
    auto smbiosPort = std::make_shared<Win32SmbiosAdapter>(logger);
    auto searchPort = std::make_shared<Win32InfSearchAdapter>(logger);
    auto storePort = std::make_shared<Win32DriverStoreAdapter>(logger);
    auto deviceRepo = std::make_shared<Win32DeviceRepository>(logger);
    auto installPort = std::make_shared<Win32DriverInstallAdapter>(logger);

    auto smbiosUC = std::make_shared<SmbiosReaderInteractor>(smbiosPort, logger);
    auto searchUC = std::make_shared<InfSearchInteractor>(searchPort, logger);
    auto importUC = std::make_shared<InfImportInteractor>(storePort, logger);
    auto enumUC = std::make_shared<DeviceEnumerateInteractor>(deviceRepo, logger);
    auto installUC = std::make_shared<DriverInstallInteractor>(installPort, logger);

    return std::make_unique<InstallViewModel>(
        smbiosUC, searchUC, importUC, enumUC, installUC);
}

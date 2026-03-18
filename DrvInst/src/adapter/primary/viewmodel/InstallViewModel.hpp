// src/adapter/primary/viewmodel/InstallViewModel.hpp
#pragma once
#include <Windows.h>
#include <memory>
#include <atomic>
#include <vector>
#include <string>
#include <functional>
#include "src/application/port/in/ISmbiosReaderUseCase.hpp"
#include "src/application/port/in/IInfSearchUseCase.hpp"
#include "src/application/port/in/IInfImportUseCase.hpp"
#include "src/application/port/in/IDeviceEnumerateUseCase.hpp"
#include "src/application/port/in/IDriverInstallUseCase.hpp"

class InstallViewModel {
public:
    explicit InstallViewModel(
        std::shared_ptr<ISmbiosReaderUseCase>    smbiosUC,
        std::shared_ptr<IInfSearchUseCase>       searchUC,
        std::shared_ptr<IInfImportUseCase>       importUC,
        std::shared_ptr<IDeviceEnumerateUseCase> enumUC,
        std::shared_ptr<IDriverInstallUseCase>   installUC);

    void execute();

    std::wstring                boardName;
    std::wstring                driverPath;
    std::atomic<int>            totalInf{ 0 };
    std::atomic<int>            doneCount{ 0 };
    std::atomic<bool>           needReboot{ false };
    std::vector<InstallResult>  results;
    std::wstring                lastError;

    std::function<void()>                      onPhaseChanged;
    std::function<void()>                      onProgressChanged;
    std::function<void()>                      onCompleted;
    std::function<void(std::wstring_view)>     onError;

private:
    static constexpr std::wstring_view kDriverBasePath = L"Y:\\KdicSetup\\Drivers";

    std::shared_ptr<ISmbiosReaderUseCase>    smbiosUC_;
    std::shared_ptr<IInfSearchUseCase>       searchUC_;
    std::shared_ptr<IInfImportUseCase>       importUC_;
    std::shared_ptr<IDeviceEnumerateUseCase> enumUC_;
    std::shared_ptr<IDriverInstallUseCase>   installUC_;
};

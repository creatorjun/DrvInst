// src/adapter/primary/viewmodel/InstallViewModel.cpp
#include <Windows.h>
#include "src/adapter/primary/viewmodel/InstallViewModel.hpp"

InstallViewModel::InstallViewModel(
    std::shared_ptr<ISmbiosReaderUseCase>    smbiosUC,
    std::shared_ptr<IInfSearchUseCase>       searchUC,
    std::shared_ptr<IInfImportUseCase>       importUC,
    std::shared_ptr<IDeviceEnumerateUseCase> enumUC,
    std::shared_ptr<IDriverInstallUseCase>   installUC)
    : smbiosUC_(std::move(smbiosUC))
    , searchUC_(std::move(searchUC))
    , importUC_(std::move(importUC))
    , enumUC_(std::move(enumUC))
    , installUC_(std::move(installUC))
{
}

void InstallViewModel::execute()
{
    if (onPhaseChanged) onPhaseChanged();

    auto board = smbiosUC_->execute();
    if (!board.isValid) {
        lastError = L"SMBIOS read failed";
        if (onError) onError(lastError);
        return;
    }
    boardName = board.productName;
    driverPath = std::wstring(kDriverBasePath) + L"\\" + boardName;
    if (onPhaseChanged) onPhaseChanged();

    auto packages = searchUC_->execute(kDriverBasePath, boardName);
    if (packages.empty()) {
        lastError = L"No INF files found";
        if (onError) onError(lastError);
        return;
    }
    totalInf.store(static_cast<int>(packages.size()));
    if (onPhaseChanged) onPhaseChanged();

    packages = importUC_->execute(std::move(packages));
    if (onPhaseChanged) onPhaseChanged();

    auto devices = enumUC_->execute();
    if (onPhaseChanged) onPhaseChanged();

    auto output = installUC_->execute(packages, devices);

    results = std::move(output.results);
    doneCount.store(static_cast<int>(results.size()));
    needReboot.store(output.needReboot);

    if (onCompleted) onCompleted();
}

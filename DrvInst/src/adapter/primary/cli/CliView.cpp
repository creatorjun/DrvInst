// src/adapter/primary/cli/CliView.cpp
#include <Windows.h>
#include "src/adapter/primary/cli/CliView.hpp"
#include <format>

CliView::CliView(InstallViewModel* vm)
    : vm_(vm)
{
}

void CliView::bindCallbacks()
{
    vm_->onPhaseChanged = [this]() {
        printLine(std::format(L"[Phase] board={} path={}",
            vm_->boardName.empty() ? L"(reading...)" : vm_->boardName,
            vm_->driverPath.empty() ? L"(pending)" : vm_->driverPath));
        };

    vm_->onProgressChanged = [this]() {
        printLine(std::format(L"[Progress] {}/{}",
            vm_->doneCount.load(), vm_->totalInf.load()));
        };

    vm_->onCompleted = [this]() {
        int success = 0, failed = 0, noMatch = 0;
        for (const auto& r : vm_->results) {
            if (r.status == InstallStatus::Success ||
                r.status == InstallStatus::NeedReboot)   ++success;
            else if (r.status == InstallStatus::Failed)  ++failed;
            else if (r.status == InstallStatus::NoMatchingDevice) ++noMatch;
        }
        printLine(std::format(
            L"[Done] total={} success={} failed={} no-match={} reboot={}",
            vm_->results.size(), success, failed, noMatch,
            vm_->needReboot.load() ? L"yes" : L"no"));
        };

    vm_->onError = [](std::wstring_view msg) {
        printLine(std::format(L"[Error] {}", msg));
        };
}

void CliView::promptReboot()
{
    printLine(L"[Reboot] Reboot required. Press any key to reboot...");
    _getwch();
    ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_OTHER);
}

void CliView::printLine(std::wstring_view msg)
{
    const HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    std::wstring line = std::wstring(msg) + L"\n";
    WriteConsoleW(h, line.c_str(), static_cast<DWORD>(line.size()), nullptr, nullptr);
}

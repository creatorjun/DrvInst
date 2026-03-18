// src/adapter/primary/cli/CliView.hpp
#pragma once
#include <Windows.h>
#include "src/adapter/primary/viewmodel/InstallViewModel.hpp"

class CliView {
public:
    explicit CliView(InstallViewModel* vm);
    void bindCallbacks();
    void promptReboot();

private:
    InstallViewModel* vm_;

    static void printLine(std::wstring_view msg);
};

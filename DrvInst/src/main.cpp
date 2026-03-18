// src/main.cpp
#include <Windows.h>
#include "src/assembly/AppBuilder.hpp"
#include "src/adapter/primary/cli/CliView.hpp"

int wmain()
{
    auto vm = AppBuilder::build();
    auto view = CliView(vm.get());

    view.bindCallbacks();
    vm->execute();

    if (vm->needReboot.load()) {
        view.promptReboot();
    }

    return 0;
}

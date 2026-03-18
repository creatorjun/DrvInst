// src/assembly/AppBuilder.hpp
#pragma once
#include <memory>
#include "src/adapter/primary/viewmodel/InstallViewModel.hpp"

class AppBuilder {
public:
    static std::unique_ptr<InstallViewModel> build();
};

// src/application/port/in/IInfImportUseCase.hpp
#pragma once
#include <vector>
#include "src/domain/model/DriverPackage.hpp"

class IInfImportUseCase {
public:
    virtual ~IInfImportUseCase() = default;
    virtual std::vector<DriverPackage> execute(
        std::vector<DriverPackage> packages) = 0;
};

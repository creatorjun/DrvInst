// src/application/port/in/IInfSearchUseCase.hpp
#pragma once
#include <string>
#include <vector>
#include "src/domain/model/DriverPackage.hpp"

class IInfSearchUseCase {
public:
    virtual ~IInfSearchUseCase() = default;
    virtual std::vector<DriverPackage> execute(
        std::wstring_view basePath,
        std::wstring_view boardName) = 0;
};

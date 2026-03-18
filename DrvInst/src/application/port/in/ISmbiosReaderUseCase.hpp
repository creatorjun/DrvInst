// src/application/port/in/ISmbiosReaderUseCase.hpp
#pragma once
#include "src/domain/model/MotherboardInfo.hpp"

class ISmbiosReaderUseCase {
public:
    virtual ~ISmbiosReaderUseCase() = default;
    virtual MotherboardInfo execute() = 0;
};

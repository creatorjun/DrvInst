// src/application/usecase/InfSearchInteractor.hpp
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "src/application/port/in/IInfSearchUseCase.hpp"
#include "src/application/port/out/IInfSearchPort.hpp"
#include "src/application/port/out/ILoggerPort.hpp"

class InfSearchInteractor final : public IInfSearchUseCase {
public:
    explicit InfSearchInteractor(
        std::shared_ptr<IInfSearchPort> searchPort,
        std::shared_ptr<ILoggerPort>    logger);

    std::vector<DriverPackage> execute(
        std::wstring_view basePath,
        std::wstring_view boardName) override;

private:
    std::shared_ptr<IInfSearchPort> searchPort_;
    std::shared_ptr<ILoggerPort>    logger_;

    static std::wstring extractFileName(std::wstring_view fullPath);
};

// src/adapter/secondary/win32/Win32InfSearchAdapter.cpp
#include "src/adapter/secondary/win32/Win32InfSearchAdapter.hpp"
#include <Windows.h>
#include <queue>
#include <format>

Win32InfSearchAdapter::Win32InfSearchAdapter(std::shared_ptr<ILoggerPort> logger)
    : logger_(std::move(logger))
{
}

bool Win32InfSearchAdapter::directoryExists(std::wstring_view path)
{
    const DWORD attr = GetFileAttributesW(path.data());
    return (attr != INVALID_FILE_ATTRIBUTES) &&
        (attr & FILE_ATTRIBUTE_DIRECTORY);
}

std::vector<std::wstring> Win32InfSearchAdapter::findInfFiles(std::wstring_view rootPath)
{
    std::vector<std::wstring> result;
    std::queue<std::wstring>  dirs;
    dirs.push(std::wstring(rootPath));

    while (!dirs.empty()) {
        std::wstring cur = std::move(dirs.front());
        dirs.pop();

        const std::wstring pattern = cur + L"\\*";
        WIN32_FIND_DATAW   wfd{};
        HANDLE hFind = FindFirstFileW(pattern.c_str(), &wfd);

        if (hFind == INVALID_HANDLE_VALUE)
            continue;

        do {
            std::wstring_view name{ wfd.cFileName };

            if (name == L"." || name == L"..")
                continue;

            std::wstring fullPath = cur + L"\\" + std::wstring(name);

            if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                dirs.push(std::move(fullPath));
            }
            else if (endsWithInf(name)) {
                result.push_back(std::move(fullPath));
            }
        } while (FindNextFileW(hFind, &wfd));

        FindClose(hFind);
    }

    return result;
}

bool Win32InfSearchAdapter::endsWithInf(std::wstring_view name)
{
    if (name.size() < 4)
        return false;

    std::wstring ext{ name.substr(name.size() - 4) };
    for (auto& c : ext) c = static_cast<wchar_t>(towlower(c));
    return ext == L".inf";
}

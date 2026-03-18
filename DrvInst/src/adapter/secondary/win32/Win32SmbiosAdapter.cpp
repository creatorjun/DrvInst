// src/adapter/secondary/win32/Win32SmbiosAdapter.cpp
#include <Windows.h>
#include <sysinfoapi.h>
#include "src/adapter/secondary/win32/Win32SmbiosAdapter.hpp"
#include <vector>
#include <format>

#pragma pack(push, 1)
struct RawSMBIOSData {
    BYTE  Used20CallingMethod;
    BYTE  SMBIOSMajorVersion;
    BYTE  SMBIOSMinorVersion;
    BYTE  DmiRevision;
    DWORD Length;
    BYTE  SMBIOSTableData[1];
};

struct SMBIOSHeader {
    BYTE type;
    BYTE length;
    WORD handle;
};
#pragma pack(pop)

Win32SmbiosAdapter::Win32SmbiosAdapter(std::shared_ptr<ILoggerPort> logger)
    : logger_(std::move(logger))
{
}

std::expected<MotherboardInfo, DWORD> Win32SmbiosAdapter::readMotherboardInfo()
{
    const DWORD bufSize = GetSystemFirmwareTable('RSMB', 0, nullptr, 0);
    if (bufSize == 0)
        return std::unexpected(GetLastError());

    std::vector<BYTE> buf(bufSize);
    if (GetSystemFirmwareTable('RSMB', 0, buf.data(), bufSize) == 0)
        return std::unexpected(GetLastError());

    auto* raw = reinterpret_cast<RawSMBIOSData*>(buf.data());
    const BYTE* tableStart = raw->SMBIOSTableData;
    const BYTE* tableEnd = tableStart + raw->Length;
    const BYTE* ptr = tableStart;

    while (ptr + sizeof(SMBIOSHeader) <= tableEnd) {
        auto* hdr = reinterpret_cast<const SMBIOSHeader*>(ptr);

        if (hdr->type == 127)
            break;

        if (hdr->type == 2 && hdr->length >= 0x08) {
            const BYTE* stringSection = ptr + hdr->length;
            const BYTE  mfrIdx = ptr[0x04];
            const BYTE  prodIdx = ptr[0x05];
            const BYTE  verIdx = ptr[0x06];

            MotherboardInfo info;
            info.manufacturer = getSmBiosString(stringSection, mfrIdx);
            info.productName = getSmBiosString(stringSection, prodIdx);
            info.version = getSmBiosString(stringSection, verIdx);
            info.isValid = true;
            return info;
        }

        const BYTE* strPtr = ptr + hdr->length;
        while (strPtr + 1 <= tableEnd) {
            if (strPtr[0] == 0 && strPtr[1] == 0) {
                ptr = strPtr + 2;
                break;
            }
            ++strPtr;
        }
        if (strPtr + 1 > tableEnd)
            break;
    }

    return std::unexpected(static_cast<DWORD>(ERROR_NOT_FOUND));
}

std::wstring Win32SmbiosAdapter::getSmBiosString(const BYTE* stringSection, BYTE index)
{
    if (index == 0)
        return L"";

    const char* cur = reinterpret_cast<const char*>(stringSection);
    for (BYTE i = 1; i < index; ++i) {
        while (*cur != '\0') ++cur;
        ++cur;
    }
    return ansiToWide(cur);
}

std::wstring Win32SmbiosAdapter::ansiToWide(const char* str)
{
    if (!str || *str == '\0')
        return L"";

    const int len = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
    if (len <= 0)
        return L"";

    std::wstring result(len - 1, L'\0');
    MultiByteToWideChar(CP_ACP, 0, str, -1, result.data(), len);
    return result;
}

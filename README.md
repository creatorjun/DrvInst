# drvinst — 구현 계획서 v1.0

## 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 프로젝트명 | drvinst |
| 언어 표준 | C++23 / C17 |
| 빌드 환경 | Visual Studio Community 2022+, MSVC v143 toolset |
| 대상 OS | Windows 11 / WinPE (amd64) |
| 아키텍처 | Hexagonal Architecture + MVVM |
| API 수준 | Win32 저수준 API 전용 |
| SubSystem | Console (`/SUBSYSTEM:CONSOLE`) |
| 링크 라이브러리 | `setupapi.lib` `newdev.lib` `cfgmgr32.lib` `kernel32.lib` `user32.lib` |
| 실행 흐름 | SMBIOS → 메인보드명 추출 → INF 탐색 → 드라이버 설치 |

---

## 1. 전체 실행 흐름

```
wmain()
  │
  ▼
[Phase 1] SmbiosReaderInteractor
  GetSystemFirmwareTable('RSMB') → SMBIOS Type 2 파싱
  → MotherboardInfo { manufacturer, productName, version, isValid }
  isValid == false → onError 콜백 → 종료
  │
  ▼
[Phase 2] InfSearchInteractor
  Y:\KdicSetup\Drivers\{productName}\ 경로 존재 확인
  FindFirstFileW / FindNextFileW → *.inf 재귀 탐색 (BFS)
  → vector<DriverPackage>
  empty → onError 콜백 → 종료
  │
  ▼
[Phase 3] InfImportInteractor
  각 INF → SetupCopyOEMInfW → DriverStore import
  이미 import됐으면 alreadyImported = true 로 스킵 (idempotent)
  단일 INF 실패 시 로그 후 continue (전체 중단 없음)
  │
  ▼
[Phase 4] DeviceEnumerateInteractor
  SetupDiGetClassDevs(DIGCF_ALLCLASSES | DIGCF_PRESENT)
  CM_Get_DevNode_Status → problemCode 필터링
  → vector<DeviceInfo>
  │
  ▼
[Phase 5] DriverInstallInteractor
  DriverMatchService: HW ID 매칭 (Hardware ID 우선 → Compatible ID 폴백)
  DiInstallDevice → vector<InstallResult>
  NeedReboot 플래그 집계
  │
  ▼
[Phase 6] CliView / InstallViewModel
  onPhaseChanged / onCompleted 콜백으로 결과 출력
  needReboot == true → promptReboot() → ExitWindowsEx
```

---

## 2. 아키텍처 레이어 구조

```
┌──────────────────────────────────────────────────────────────┐
│                      Adapter Layer                           │
│                                                              │
│  Primary Adapter              Secondary Adapter              │
│  ┌───────────────────┐        ┌──────────────────────────┐  │
│  │ CliView           │        │ Win32SmbiosAdapter       │  │
│  │ InstallViewModel  │        │ Win32InfSearchAdapter    │  │
│  └─────────┬─────────┘        │ Win32DriverStoreAdapter  │  │
│            │ Input Port       │ Win32DeviceRepository    │  │
│  ──────────┼──────────────────│ Win32DriverInstallAdapter│  │
│            ▼  Application     │ ConsoleLoggerAdapter     │  │
│  ┌──────────────────────────────────────────────────────┐│  │
│  │                Application Core                      ││  │
│  │  ┌──────────────────────────────────────────────┐   ││  │
│  │  │  Use Cases (Interactors)                     │   ││  │
│  │  │  SmbiosReaderInteractor                      │   ││  │
│  │  │  InfSearchInteractor                         │   ││  │
│  │  │  InfImportInteractor                         │   ││  │
│  │  │  DeviceEnumerateInteractor                   │   ││  │
│  │  │  DriverInstallInteractor                     │   ││  │
│  │  └──────────────────────────────────────────────┘   ││  │
│  │  ┌──────────────────────────────────────────────┐   ││  │
│  │  │  Domain Models                               │   ││  │
│  │  │  MotherboardInfo  DriverPackage              │   ││  │
│  │  │  DeviceInfo       InstallResult              │   ││  │
│  │  └──────────────────────────────────────────────┘   ││  │
│  │  ┌──────────────────────────────────────────────┐   ││  │
│  │  │  Domain Service                              │   ││  │
│  │  │  DriverMatchService (순수 HW ID 매칭 로직)   │   ││  │
│  │  └──────────────────────────────────────────────┘   ││  │
│  └──────────────────────────────────────────────────────┘│  │
└──────────────────────────────────────────────────────────────┘
```

---

## 3. 디렉터리 구조

```
drvinst/
├── drvinst.sln
├── drvinst.vcxproj
├── drvinst.vcxproj.filters
│
├── src/
│   ├── main.cpp
│   │
│   ├── domain/
│   │   ├── model/
│   │   │   ├── MotherboardInfo.hpp
│   │   │   ├── DriverPackage.hpp
│   │   │   ├── DeviceInfo.hpp              ← #include <Windows.h> 필수
│   │   │   └── InstallResult.hpp           ← #include <Windows.h> 필수
│   │   └── service/
│   │       └── DriverMatchService.hpp      ← 외부 의존 없는 순수 로직
│   │
│   ├── application/
│   │   ├── port/
│   │   │   ├── in/
│   │   │   │   ├── ISmbiosReaderUseCase.hpp
│   │   │   │   ├── IInfSearchUseCase.hpp
│   │   │   │   ├── IInfImportUseCase.hpp
│   │   │   │   ├── IDeviceEnumerateUseCase.hpp
│   │   │   │   └── IDriverInstallUseCase.hpp
│   │   │   └── out/
│   │   │       ├── ISmbiosPort.hpp         ← #include <Windows.h> 필수
│   │   │       ├── IInfSearchPort.hpp
│   │   │       ├── IDriverStorePort.hpp    ← #include <Windows.h> 필수
│   │   │       ├── IDeviceRepository.hpp
│   │   │       ├── IDriverInstallPort.hpp
│   │   │       └── ILoggerPort.hpp
│   │   └── usecase/
│   │       ├── SmbiosReaderInteractor.hpp / .cpp   ← #include <Windows.h> 최상단
│   │       ├── InfSearchInteractor.hpp / .cpp       ← #include <Windows.h> 최상단
│   │       ├── InfImportInteractor.hpp / .cpp       ← #include <Windows.h> 최상단
│   │       ├── DeviceEnumerateInteractor.hpp / .cpp ← #include <Windows.h> + <cfgmgr32.h>
│   │       └── DriverInstallInteractor.hpp / .cpp   ← #include <Windows.h> 최상단
│   │
│   ├── adapter/
│   │   ├── primary/
│   │   │   ├── cli/
│   │   │   │   ├── CliView.hpp / .cpp
│   │   │   └── viewmodel/
│   │   │       └── InstallViewModel.hpp / .cpp
│   │   └── secondary/
│   │       ├── win32/
│   │       │   ├── Win32SmbiosAdapter.hpp / .cpp
│   │       │   ├── Win32InfSearchAdapter.hpp / .cpp
│   │       │   ├── Win32DriverStoreAdapter.hpp / .cpp
│   │       │   ├── Win32DeviceRepository.hpp / .cpp
│   │       │   └── Win32DriverInstallAdapter.hpp / .cpp
│   │       └── logger/
│   │           └── ConsoleLoggerAdapter.hpp / .cpp  ← HANDLE hConsole_ 선언 필수
│   │
│   └── assembly/
│       ├── AppBuilder.hpp
│       └── AppBuilder.cpp                 ← 유일한 DI 조립 지점
│
└── tests/
    ├── domain/
    │   └── DriverMatchServiceTest.cpp
    └── mock/
        ├── MockSmbiosPort.hpp
        ├── MockInfSearchPort.hpp
        ├── MockDriverInstallPort.hpp
        └── MockDeviceRepository.hpp
```

---

## 4. 도메인 모델

### MotherboardInfo
```cpp
#pragma once
#include <string>

struct MotherboardInfo {
    std::wstring manufacturer;
    std::wstring productName;   // SMBIOS Type2 — 드라이버 폴더명으로 사용
    std::wstring version;
    bool         isValid = false;
};
```

### DriverPackage
```cpp
#pragma once
#include <string>
#include <vector>

struct DriverPackage {
    std::wstring              infFullPath;
    std::wstring              infFileName;
    std::vector<std::wstring> hwIds;
    bool                      alreadyImported = false;
};
```

### DeviceInfo
```cpp
#pragma once
#include <string>
#include <vector>
#include <Windows.h>     // GUID, DWORD

struct DeviceInfo {
    std::wstring              instanceId;
    std::vector<std::wstring> hardwareIds;
    std::vector<std::wstring> compatibleIds;
    GUID                      classGuid{};
    DWORD                     deviceStatus = 0;
    DWORD                     problemCode  = 0;
};
```

### InstallResult
```cpp
#pragma once
#include <string>
#include <Windows.h>     // DWORD

enum class InstallStatus {
    Success,
    AlreadyInstalled,
    NeedReboot,
    NoMatchingDevice,
    Failed
};

struct InstallResult {
    InstallStatus status           = InstallStatus::Failed;
    std::wstring  infPath;
    std::wstring  deviceInstanceId;
    DWORD         win32ErrorCode   = 0;
    std::wstring  errorMessage;
};
```

---

## 5. Port 인터페이스

### include 규칙 (오류 방지)

| Port 파일 | 필수 선행 include |
|-----------|-----------------|
| `ISmbiosPort.hpp` | `<Windows.h>` → `<expected>` 순서 |
| `IDriverStorePort.hpp` | `<Windows.h>` → `<expected>` 순서 |
| `IDeviceRepository.hpp` | `DeviceInfo.hpp` (내부적으로 `<Windows.h>` 포함) |
| `IDriverInstallPort.hpp` | `DeviceInfo.hpp`, `DriverPackage.hpp`, `InstallResult.hpp` |
| 모든 usecase `.cpp` | **`<Windows.h>`를 첫 번째 include로 배치** |
| `cfgmgr32.h` 사용 파일 | `<Windows.h>` 이후 include |

### Output Ports 요약

```cpp
class ISmbiosPort {
    virtual std::expected<MotherboardInfo, DWORD> readMotherboardInfo() = 0;
};

class IInfSearchPort {
    virtual std::vector<std::wstring> findInfFiles(std::wstring_view rootPath) = 0;
    virtual bool directoryExists(std::wstring_view path) = 0;
};

class IDriverStorePort {
    virtual std::expected<std::wstring, DWORD> importInf(std::wstring_view srcInfPath) = 0;
    virtual bool isAlreadyImported(std::wstring_view infFileName) = 0;
};

class IDeviceRepository {
    virtual std::vector<DeviceInfo> enumerateAllPresentDevices() = 0;
};

class IDriverInstallPort {
    virtual InstallResult installDriver(
        const DeviceInfo& device, const DriverPackage& package) = 0;
};

enum class LogLevel { Debug, Info, Warn, Error };
class ILoggerPort {
    virtual void log(LogLevel level, std::wstring_view message) = 0;
};
```

---

## 6. Use Case 상세 명세

### Phase 1 — SmbiosReaderInteractor
```
Input  : 없음
Output : MotherboardInfo

1. ISmbiosPort::readMotherboardInfo()
2. productName 비어있거나 isValid == false → onError → 종료
3. 성공 시 MotherboardInfo 반환
```

### Phase 2 — InfSearchInteractor
```
Input  : basePath = L"Y:\KdicSetup\Drivers", boardName = productName
Output : vector<DriverPackage>

1. searchPath = basePath + L"\" + boardName
2. IInfSearchPort::directoryExists(searchPath) → false: 로그 후 빈 목록 반환
3. IInfSearchPort::findInfFiles(searchPath) → infPaths (BFS 재귀)
4. 각 path → DriverPackage { infFullPath, infFileName } 변환
```

### Phase 3 — InfImportInteractor
```
Input  : vector<DriverPackage>
Output : vector<DriverPackage> (alreadyImported 플래그 갱신)

for each package:
  1. isAlreadyImported(infFileName) → true: skip, alreadyImported = true
  2. importInf(infFullPath) 실패 시 로그 후 continue (전체 중단 없음)
```

### Phase 4 — DeviceEnumerateInteractor
```
Input  : 없음
Output : vector<DeviceInfo> (설치 필요 디바이스만)

필터 대상 problemCode:
  CM_PROB_NOT_CONFIGURED    (0x01)
  CM_PROB_REINSTALL         (0x12)
  CM_PROB_FAILED_INSTALL    (0x1C)
  CM_PROB_NEED_CLASS_CONFIG (0x38)
```

### Phase 5 — DriverInstallInteractor
```
Input  : vector<DriverPackage>, vector<DeviceInfo>
Output : DriverInstallOutput { vector<InstallResult>, bool needReboot }

for each device:
  1. DriverMatchService::findBestMatch()
     - Step 1: hardwareIds exact match (대소문자 무시)
     - Step 2: compatibleIds prefix match
     - 미매칭: NoMatchingDevice 기록 후 continue
  2. IDriverInstallPort::installDriver()
  3. NeedReboot → globalNeedReboot = true
```

---

## 7. Win32 Secondary Adapter 구현 상세

### Win32SmbiosAdapter
```
GetSystemFirmwareTable('RSMB', 0, nullptr, 0) → 버퍼 크기
vector<BYTE> buf → GetSystemFirmwareTable

SMBIOS 순회:
  type == 127 → end-of-table, break
  type == 2 (Baseboard):
    ptr[0x04] = Manufacturer index
    ptr[0x05] = Product Name index
    ptr[0x06] = Version index
    getString(stringSection, index) → ANSI → MultiByteToWideChar → wstring
  다음 구조체: header.length 이후 double-null(00 00) 스킵
```

### Win32InfSearchAdapter
```
findInfFiles(rootPath):
  queue<wstring> dirs = { rootPath }
  while (!dirs.empty()):
    FindFirstFileW(dir + L"\*") → WIN32_FIND_DATAW
    FILE_ATTRIBUTE_DIRECTORY → dirs.push()
    .inf (대소문자 무시) → result.push_back()
    FindClose()

directoryExists(path):
  GetFileAttributesW → INVALID_FILE_ATTRIBUTES 또는 DIRECTORY 플래그 확인
```

### Win32DriverStoreAdapter
```
importInf(srcPath):
  SetupCopyOEMInfW(srcPath, nullptr, SPOST_PATH, 0, destBuf, MAX_PATH, ...)
  실패:
    ERROR_FILE_EXISTS → 성공으로 간주, srcPath 반환
    그 외 → std::unexpected(GetLastError())

isAlreadyImported(infFileName):
  GetWindowsDirectoryW → winDir
  winDir + "\INF\" + infFileName → GetFileAttributesW 존재 확인
```

### Win32DeviceRepository
```
SetupDiGetClassDevs(nullptr, nullptr, nullptr, DIGCF_ALLCLASSES | DIGCF_PRESENT)
SetupDiEnumDeviceInfo 루프:
  SetupDiGetDeviceInstanceIdW          → instanceId
  SPDRP_HARDWAREID  (REG_MULTI_SZ)     → parseMultiSz → hardwareIds
  SPDRP_COMPATIBLEIDS (REG_MULTI_SZ)   → parseMultiSz → compatibleIds
  CM_Get_DevNode_Status                → deviceStatus, problemCode
SetupDiDestroyDeviceInfoList
```

### Win32DriverInstallAdapter
```
SetupDiCreateDeviceInfoList
SetupDiOpenDeviceInfoW(instanceId)
SetupDiBuildDriverInfoList(SPDIT_COMPATDRIVER)
SetupDiEnumDriverInfoW 루프:
  SetupDiGetDriverInfoDetailW → detail.InfFileName
  infFileNameMatch(detail.InfFileName, package.infFileName) → break

DiInstallDevice(nullptr, hDevInfo, &devInfoData, &drvInfoData, 0, &needReboot)
  실패 → Failed + GetLastError()
  needReboot == TRUE → NeedReboot
  성공 → Success
SetupDiDestroyDeviceInfoList
```

---

## 8. Domain Service — DriverMatchService

```cpp
class DriverMatchService {
public:
    struct MatchResult {
        const DriverPackage* package      = nullptr;
        bool                 isExactMatch = false;
    };

    static MatchResult findBestMatch(
        const DeviceInfo&                 device,
        const std::vector<DriverPackage>& packages);

private:
    static std::wstring normalize(std::wstring_view s);
    // 대소문자 무시 exact match
    static bool hwIdMatch(std::wstring_view deviceId, std::wstring_view infId);
    // prefix match (usb\class_xx 계열)
    static bool compatMatch(std::wstring_view deviceId, std::wstring_view infId);
};
```

---

## 9. ViewModel

```cpp
class InstallViewModel {
public:
    explicit InstallViewModel(
        std::shared_ptr<ISmbiosReaderUseCase>    smbiosUC,
        std::shared_ptr<IInfSearchUseCase>       searchUC,
        std::shared_ptr<IInfImportUseCase>       importUC,
        std::shared_ptr<IDeviceEnumerateUseCase> enumUC,
        std::shared_ptr<IDriverInstallUseCase>   installUC);

    void execute();  // Phase 1~5 순차 실행

    // 관찰 가능 상태
    std::wstring               boardName;
    std::wstring               driverPath;
    std::atomic<int>           totalInf{ 0 };
    std::atomic<int>           doneCount{ 0 };
    std::atomic<bool>          needReboot{ false };
    std::vector<InstallResult> results;
    std::wstring               lastError;

    // View 콜백
    std::function<void()>                  onPhaseChanged;
    std::function<void()>                  onProgressChanged;
    std::function<void()>                  onCompleted;
    std::function<void(std::wstring_view)> onError;

private:
    static constexpr std::wstring_view kDriverBasePath = L"Y:\\KdicSetup\\Drivers";
    // ... use case shared_ptr 멤버
};
```

---

## 10. CliView

```cpp
class CliView {
public:
    explicit CliView(InstallViewModel* vm);
    void bindCallbacks();   // onPhaseChanged, onCompleted, onError 등록
    void promptReboot();    // 키 입력 대기 후 ExitWindowsEx(EWX_REBOOT)

private:
    InstallViewModel* vm_;
    static void printLine(std::wstring_view msg);  // WriteConsoleW 래핑
};
```

---

## 11. 조립 (AppBuilder — DI 루트)

```cpp
// 모든 구체 타입은 AppBuilder::build() 에서만 생성
// 나머지 파일은 인터페이스 포인터만 보유

std::unique_ptr<InstallViewModel> AppBuilder::build() {
    auto logger      = std::make_shared<ConsoleLoggerAdapter>();
    auto smbiosPort  = std::make_shared<Win32SmbiosAdapter>(logger);
    auto searchPort  = std::make_shared<Win32InfSearchAdapter>(logger);
    auto storePort   = std::make_shared<Win32DriverStoreAdapter>(logger);
    auto deviceRepo  = std::make_shared<Win32DeviceRepository>(logger);
    auto installPort = std::make_shared<Win32DriverInstallAdapter>(logger);

    auto smbiosUC  = std::make_shared<SmbiosReaderInteractor>(smbiosPort, logger);
    auto searchUC  = std::make_shared<InfSearchInteractor>(searchPort, logger);
    auto importUC  = std::make_shared<InfImportInteractor>(storePort, logger);
    auto enumUC    = std::make_shared<DeviceEnumerateInteractor>(deviceRepo, logger);
    auto installUC = std::make_shared<DriverInstallInteractor>(installPort, logger);

    return std::make_unique<InstallViewModel>(
        smbiosUC, searchUC, importUC, enumUC, installUC);
}
```

---

## 12. 진입점

```cpp
// src/main.cpp
int wmain() {
    auto vm   = AppBuilder::build();
    auto view = CliView(vm.get());

    view.bindCallbacks();
    vm->execute();

    if (vm->needReboot.load())
        view.promptReboot();

    return 0;
}
```

---

## 13. 빌드 설정

| 항목 | 값 |
|------|----|
| `PlatformToolset` | `v143` |
| `LanguageStandard` (C++) | `stdcpp23` |
| `LanguageStandard_C` | `stdc17` |
| `CharacterSet` | `Unicode` |
| `SubSystem` | `Console (/SUBSYSTEM:CONSOLE)` |
| `AdditionalDependencies` | `setupapi.lib; newdev.lib; cfgmgr32.lib; kernel32.lib; user32.lib` |
| `RuntimeLibrary` (Release) | `MultiThreaded (/MT)` — WinPE 정적 링크 |
| `RuntimeLibrary` (Debug) | `MultiThreadedDebug (/MTd)` |
| `Optimization` (Release) | `O2` |
| `AdditionalIncludeDirectories` | `$(ProjectDir)` — `src/` 경로 기준 include 해소 |

---

## 14. include 규칙 요약 (C4430 / C4819 방지)

| 규칙 | 내용 |
|------|------|
| Win32 타입 사용 파일 | `<Windows.h>` 를 **첫 번째** include로 배치 |
| `cfgmgr32.h` | 반드시 `<Windows.h>` **이후** include |
| `<expected>` | `<Windows.h>` 이후 include |
| 한글 문자열 리터럴 | `std::format` 인자에 한글 사용 금지 → **영문 로그 메시지** 사용 |
| 파일 인코딩 | 모든 소스파일 **UTF-8 with BOM** 저장 |

---

## 15. WinPE 특이사항 처리

| 상황 | 처리 방법 |
|------|-----------|
| HardLink 실패 (`flq` 오류) | `FILE_QUEUE_COMMIT` exit `0x00000000` → 성공 처리 |
| `CM_PROB_NEED_CLASS_CONFIG (0x38)` | pending 상태, 오류 아님 — 정상 설치로 간주 |
| `0x00000bc3` (`ERROR_RESTART_DEVICE_ONLY`) | NeedReboot 플래그만 set, 실패 아님 |
| `Y:\` 드라이브 미존재 | `directoryExists()` 조기 실패 → `onError` 콜백 |
| WinPE RAM 드라이브 (X:) | NTFS 비지원 → 파일 복사 실패 무시 로직 내장 |
| `wpeinit.exe` 컨텍스트 | SYSTEM 권한 실행 → 별도 권한 상승 불필요 |

---

## 16. 구현 완료 현황

| Phase | 내용 | 상태 |
|-------|------|------|
| Phase 1 | 도메인 모델 + Port 인터페이스 정의 | ✅ 완료 |
| Phase 2 | Use Case Interactor 5종 구현 | ✅ 완료 |
| Phase 3 | Win32 Secondary Adapter 5종 구현 | ✅ 완료 |
| Phase 4 | ConsoleLoggerAdapter 구현 | ✅ 완료 |
| Phase 5 | InstallViewModel + CliView 구현 | ✅ 완료 |
| Phase 6 | AppBuilder DI 조립 + main.cpp 완성 | ✅ 완료 |
| Phase 7 | WinPE 실환경 통합 테스트 | ⬜ 미완료 |
| Phase 8 | Mock 테스트 구현 | ✅ 완료 |

---

## 17. 잔여 작업

```
1. WinPE 실환경 통합 테스트
   └─ setupapi.dev.log 출력과 비교 검증
   └─ Y:\KdicSetup\Drivers\{boardName}\ 경로 구조 확인

2. vcxproj.filters 파일 정리
   └─ 소스 필터를 디렉터리 구조와 일치시킴
```

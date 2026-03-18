# drvinst — 구현 계획서 v2

## 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 프로젝트명 | drvinst |
| 언어 표준 | C++23 / C17 |
| 빌드 환경 | Visual Studio Community 2022+, MSVC v143 toolset |
| 대상 OS | Windows 11 / WinPE (amd64) |
| 아키텍처 | Hexagonal Architecture + MVVM |
| API 수준 | Win32 저수준 API 전용 |
| 링크 라이브러리 | `setupapi.lib` `newdev.lib` `cfgmgr32.lib` |
| 실행 흐름 | SMBIOS → 메인보드명 추출 → INF 탐색 → 드라이버 설치 |

---

## 1. 전체 실행 흐름

```
wmain()
  │
  ▼
[Phase 1] SmbiosReaderInteractor
  GetSystemFirmwareTable('RSMB') → SMBIOS Type 2 파싱
  → MotherboardInfo { manufacturer, productName }
  │
  ▼
[Phase 2] InfSearchInteractor
  Y:\KdicSetup\Drivers\{productName}\ 경로 존재 확인
  FindFirstFileW / FindNextFileW → *.inf 재귀 탐색
  → vector<DriverPackage>
  │
  ▼
[Phase 3] InfImportInteractor
  각 INF → SetupCopyOEMInf → DriverStore import
  (이미 import됐으면 스킵, idempotent)
  │
  ▼
[Phase 4] DeviceEnumerateInteractor
  SetupDiGetClassDevs(DIGCF_ALLCLASSES | DIGCF_PRESENT)
  → CM_Get_DevNode_Status로 설치 필요 디바이스 필터링
  → vector<DeviceInfo>
  │
  ▼
[Phase 5] DriverInstallInteractor
  DriverMatchService: HW ID 매칭 (Hardware ID 우선 → Compatible ID 폴백)
  DiInstallDevice → vector<InstallResult>
  │
  ▼
[Phase 6] CliView / InstallViewModel
  결과 출력, needReboot 플래그 처리
```

---

## 2. 아키텍처 레이어 구조

```
┌──────────────────────────────────────────────────────────────┐
│                     Adapter Layer                            │
│                                                              │
│  Primary Adapter            Secondary Adapter                │
│  ┌─────────────────┐        ┌────────────────────────────┐  │
│  │ CliView         │        │ Win32SmbiosAdapter         │  │
│  │ InstallViewModel│        │ Win32InfSearchAdapter      │  │
│  └────────┬────────┘        │ Win32DriverStoreAdapter    │  │
│           │ Input Port      │ Win32DeviceRepository      │  │
│  ─────────┼─────────────────│ Win32DriverInstallAdapter  │  │
│           ▼  Application    │ ConsoleLoggerAdapter       │  │
│  ┌────────────────────────────────────────────────────┐  │  │
│  │               Application Core                     │  │  │
│  │  ┌─────────────────────────────────────────────┐   │  │  │
│  │  │  Use Cases (Interactors)                    │   │  │  │
│  │  │  SmbiosReaderInteractor                     │   │  │  │
│  │  │  InfSearchInteractor                        │   │  │  │
│  │  │  InfImportInteractor                        │   │  │  │
│  │  │  DeviceEnumerateInteractor                  │   │  │  │
│  │  │  DriverInstallInteractor                    │   │  │  │
│  │  └─────────────────────────────────────────────┘   │  │  │
│  │  ┌─────────────────────────────────────────────┐   │  │  │
│  │  │  Domain Models                              │   │  │  │
│  │  │  MotherboardInfo  DriverPackage             │   │  │  │
│  │  │  DeviceInfo       InstallResult             │   │  │  │
│  │  └─────────────────────────────────────────────┘   │  │  │
│  │  ┌─────────────────────────────────────────────┐   │  │  │
│  │  │  Domain Service                             │   │  │  │
│  │  │  DriverMatchService (순수 HW ID 매칭 로직)  │   │  │  │
│  │  └─────────────────────────────────────────────┘   │  │  │
│  └────────────────────────────────────────────────────┘  │  │
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
│   │   │   ├── DeviceInfo.hpp
│   │   │   └── InstallResult.hpp
│   │   └── service/
│   │       └── DriverMatchService.hpp        ← 외부 의존 없는 순수 로직
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
│   │   │       ├── ISmbiosPort.hpp
│   │   │       ├── IInfSearchPort.hpp
│   │   │       ├── IDriverStorePort.hpp
│   │   │       ├── IDeviceRepository.hpp
│   │   │       ├── IDriverInstallPort.hpp
│   │   │       └── ILoggerPort.hpp
│   │   └── usecase/
│   │       ├── SmbiosReaderInteractor.hpp / .cpp
│   │       ├── InfSearchInteractor.hpp / .cpp
│   │       ├── InfImportInteractor.hpp / .cpp
│   │       ├── DeviceEnumerateInteractor.hpp / .cpp
│   │       └── DriverInstallInteractor.hpp / .cpp
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
│   │           └── ConsoleLoggerAdapter.hpp / .cpp
│   │
│   └── assembly/
│       ├── AppBuilder.hpp
│       └── AppBuilder.cpp
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
// src/domain/model/MotherboardInfo.hpp
struct MotherboardInfo {
    std::wstring manufacturer;   // SMBIOS Type2 Manufacturer
    std::wstring productName;    // SMBIOS Type2 Product Name  ← 폴더명으로 사용
    std::wstring version;
    bool         isValid = false;
};
```

### DriverPackage
```cpp
// src/domain/model/DriverPackage.hpp
struct DriverPackage {
    std::wstring infFullPath;    // Y:\KdicSetup\Drivers\보드명\xxx.inf
    std::wstring infFileName;    // xxx.inf
    std::vector<std::wstring> hwIds;
    bool alreadyImported = false;
};
```

### DeviceInfo
```cpp
// src/domain/model/DeviceInfo.hpp
struct DeviceInfo {
    std::wstring instanceId;
    std::vector<std::wstring> hardwareIds;
    std::vector<std::wstring> compatibleIds;
    GUID  classGuid{};
    DWORD deviceStatus  = 0;
    DWORD problemCode   = 0;
};
```

### InstallResult
```cpp
// src/domain/model/InstallResult.hpp
enum class InstallStatus {
    Success,
    AlreadyInstalled,
    NeedReboot,
    NoMatchingDevice,
    Failed
};

struct InstallResult {
    InstallStatus status;
    std::wstring  infPath;
    std::wstring  deviceInstanceId;
    DWORD         win32ErrorCode = 0;
    std::wstring  errorMessage;
};
```

---

## 5. Output Port 인터페이스

### ISmbiosPort
```cpp
// src/application/port/out/ISmbiosPort.hpp
class ISmbiosPort {
public:
    virtual ~ISmbiosPort() = default;
    // GetSystemFirmwareTable('RSMB') 호출 후 Type2 파싱
    virtual std::expected<MotherboardInfo, DWORD> readMotherboardInfo() = 0;
};
```

### IInfSearchPort
```cpp
// src/application/port/out/IInfSearchPort.hpp
class IInfSearchPort {
public:
    virtual ~IInfSearchPort() = default;
    // 지정 폴더 하위 *.inf 파일 재귀 탐색
    virtual std::vector<std::wstring> findInfFiles(std::wstring_view rootPath) = 0;
    // 폴더 존재 여부 확인
    virtual bool directoryExists(std::wstring_view path) = 0;
};
```

### IDriverStorePort
```cpp
// src/application/port/out/IDriverStorePort.hpp
class IDriverStorePort {
public:
    virtual ~IDriverStorePort() = default;
    virtual std::expected<std::wstring, DWORD> importInf(std::wstring_view srcInfPath) = 0;
    virtual bool isAlreadyImported(std::wstring_view infFileName) = 0;
};
```

### IDeviceRepository
```cpp
// src/application/port/out/IDeviceRepository.hpp
class IDeviceRepository {
public:
    virtual ~IDeviceRepository() = default;
    virtual std::vector<DeviceInfo> enumerateAllPresentDevices() = 0;
};
```

### IDriverInstallPort
```cpp
// src/application/port/out/IDriverInstallPort.hpp
class IDriverInstallPort {
public:
    virtual ~IDriverInstallPort() = default;
    virtual InstallResult installDriver(
        const DeviceInfo&    device,
        const DriverPackage& package) = 0;
};
```

### ILoggerPort
```cpp
// src/application/port/out/ILoggerPort.hpp
enum class LogLevel { Debug, Info, Warn, Error };

class ILoggerPort {
public:
    virtual ~ILoggerPort() = default;
    virtual void log(LogLevel level, std::wstring_view message) = 0;
};
```

---

## 6. Use Case 상세 명세

### Phase 1 — SmbiosReaderInteractor

```
Input  : 없음
Output : MotherboardInfo

Logic:
  1. ISmbiosPort::readMotherboardInfo() 호출
  2. productName 공백/특수문자 정규화 (폴더명 사용 가능 형태로)
  3. isValid == false 이면 예외 throw → 전체 중단
```

### Phase 2 — InfSearchInteractor

```
Input  : basePath (L"Y:\\KdicSetup\\Drivers"), productName
Output : vector<DriverPackage>

Logic:
  1. searchPath = basePath + L"\\" + productName
  2. IInfSearchPort::directoryExists(searchPath) 확인
     → false : ILoggerPort::log(Error, "드라이버 폴더 없음") → 빈 목록 반환
  3. IInfSearchPort::findInfFiles(searchPath) → infPaths
  4. infPaths 각각을 DriverPackage 로 변환하여 반환
```

### Phase 3 — InfImportInteractor

```
Input  : vector<DriverPackage>
Output : vector<DriverPackage> (alreadyImported 플래그 갱신)

Logic:
  for each package:
    1. IDriverStorePort::isAlreadyImported(infFileName) → true: skip, flag set
    2. false: IDriverStorePort::importInf(infFullPath) 호출
    3. 실패해도 계속 진행 (로그 기록 후 continue) — 단일 INF 실패가 전체 중단되지 않도록
```

### Phase 4 — DeviceEnumerateInteractor

```
Input  : 없음
Output : vector<DeviceInfo>

Logic:
  1. IDeviceRepository::enumerateAllPresentDevices()
  2. 필터링: problemCode 가 아래 중 하나인 디바이스만 선택
     - CM_PROB_NEED_CLASS_CONFIG (0x38)
     - CM_PROB_FAILED_INSTALL    (0x1C)
     - CM_PROB_NOT_CONFIGURED    (0x01)
     - CM_PROB_REINSTALL         (0x12)
  3. hardwareIds / compatibleIds 멀티스트링 → vector<wstring> 정규화
```

### Phase 5 — DriverInstallInteractor

```
Input  : vector<DriverPackage>, vector<DeviceInfo>
Output : vector<InstallResult>, bool needReboot

Logic:
  for each device in devices:
    1. DriverMatchService::findBestMatch(device, packages)
       - Step 1: hardwareIds × package.hwIds exact match
       - Step 2: 미매칭 시 compatibleIds prefix match
       - Step 3: 매칭 없으면 InstallStatus::NoMatchingDevice 기록 후 continue
    2. IDriverInstallPort::installDriver(device, matchedPackage)
    3. result.status == NeedReboot → globalNeedReboot = true
    4. 결과 누적 + ILoggerPort 로 상태 출력
```

---

## 7. Win32 Secondary Adapter 구현 상세

### Win32SmbiosAdapter — SMBIOS Type 2 파싱

```
GetSystemFirmwareTable('RSMB', 0, NULL, 0) → 버퍼 크기 획득
HeapAlloc → GetSystemFirmwareTable → RawSMBIOSData 버퍼

SMBIOS Table 순회:
  while (offset < totalLength):
    header = SMBIOSHeader { type, length, handle }

    if (header.type == 2):  // Baseboard Information
      stringSection = ptr + header.length
      manufacturerIdx = ptr[0x04]   // offset 4: Manufacturer string index
      productNameIdx  = ptr[0x05]   // offset 5: Product Name string index
      → getString(stringSection, manufacturerIdx) → manufacturer
      → getString(stringSection, productNameIdx)  → productName
      break

    // 다음 구조체로 이동: length 이후 double-null 종단 문자열 섹션 스킵
    offset += header.length
    while (!(data[offset]==0 && data[offset+1]==0)): offset++
    offset += 2

// getString(): null-delimited 문자열 섹션에서 N번째 문자열 반환
// 반환값은 ANSI → MultiByteToWideChar → wstring 변환
```

### Win32InfSearchAdapter — INF 재귀 탐색

```
findInfFiles(rootPath):
  queue<wstring> dirs = { rootPath }
  while (!dirs.empty()):
    dir = dirs.front(); dirs.pop()
    hFind = FindFirstFileW((dir + L"\\*"), &wfd)
    while FindNextFileW:
      if (FILE_ATTRIBUTE_DIRECTORY): dirs.push(subDir)   // 재귀
      else if (ends_with_case_insensitive(name, L".inf")): result.push_back(fullPath)
    FindClose(hFind)
  return result
```

### Win32DriverStoreAdapter — INF Import

```
importInf(srcPath):
  WCHAR destInfName[MAX_PATH]
  BOOL result = SetupCopyOEMInfW(
      srcPath.data(), NULL, SPOST_PATH, 0,
      destInfName, MAX_PATH, NULL, NULL)

  if (!result):
    err = GetLastError()
    if (err == ERROR_FILE_EXISTS): return srcPath (alreadyImported)
    else: return std::unexpected(err)
  return wstring(destInfName)
```

### Win32DeviceRepository — 디바이스 열거

```
enumerateAllPresentDevices():
  hDevInfo = SetupDiGetClassDevs(NULL, NULL, NULL,
                 DIGCF_ALLCLASSES | DIGCF_PRESENT)
  index = 0
  while SetupDiEnumDeviceInfo(hDevInfo, index++, &devInfoData):
    SetupDiGetDeviceInstanceIdW   → instanceId
    GetDeviceRegistryProperty(SPDRP_HARDWAREID)    → hwIds (멀티스트링 파싱)
    GetDeviceRegistryProperty(SPDRP_COMPATIBLEIDS) → compatIds
    CM_Get_DevNode_Status → status, problemCode
    push DeviceInfo
  SetupDiDestroyDeviceInfoList(hDevInfo)
```

### Win32DriverInstallAdapter — 드라이버 설치

```
installDriver(device, package):
  hDevInfo = SetupDiCreateDeviceInfoList(NULL, NULL)
  SetupDiOpenDeviceInfoW(hDevInfo, device.instanceId, NULL, 0, &devInfoData)

  SetupDiBuildDriverInfoList(hDevInfo, &devInfoData, SPDIT_COMPATDRIVER)

  // package의 INF 파일명과 일치하는 드라이버 항목 탐색
  index = 0
  while SetupDiEnumDriverInfoW(hDevInfo, &devInfoData, SPDIT_COMPATDRIVER,
                               index++, &drvInfoData):
    if (drvInfoData.InfFileName matches package.infFileName): break

  BOOL needReboot = FALSE
  BOOL ok = DiInstallDevice(NULL, hDevInfo, &devInfoData,
                            &drvInfoData, 0, &needReboot)

  SetupDiDestroyDeviceInfoList(hDevInfo)

  if (!ok): return { Failed, GetLastError() }
  if (needReboot): return { NeedReboot }
  return { Success }
```

---

## 8. Domain Service — DriverMatchService

```cpp
// src/domain/service/DriverMatchService.hpp
// 순수 로직, Win32 의존 없음 — 단위 테스트 최적화

class DriverMatchService {
public:
    struct MatchResult {
        const DriverPackage* package = nullptr;  // nullptr = 미매칭
        bool isExactMatch            = false;     // true=HW ID, false=Compatible ID
    };

    static MatchResult findBestMatch(
        const DeviceInfo&              device,
        const std::vector<DriverPackage>& packages);

private:
    // 대소문자 무시 정규화 비교
    static bool hwIdMatch(std::wstring_view deviceId, std::wstring_view infId);
    // Compatible ID prefix 매칭 (usb\class_xx)
    static bool compatMatch(std::wstring_view deviceId, std::wstring_view infId);
};
```

---

## 9. ViewModel

```cpp
// src/adapter/primary/viewmodel/InstallViewModel.hpp
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
    std::wstring                boardName;     // Phase1 완료 후 set
    std::wstring                driverPath;    // Phase2 완료 후 set
    std::atomic<int>            totalInf{0};
    std::atomic<int>            doneCount{0};
    std::atomic<bool>           needReboot{false};
    std::vector<InstallResult>  results;
    std::wstring                lastError;

    // View 콜백
    std::function<void()>  onPhaseChanged;
    std::function<void()>  onProgressChanged;
    std::function<void()>  onCompleted;
    std::function<void(std::wstring_view)> onError;
};
```

---

## 10. 조립 (AppBuilder — DI 루트)

```cpp
// src/assembly/AppBuilder.cpp
// 모든 구체 타입은 여기서만 생성 — 다른 파일은 인터페이스만 참조

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

## 11. 진입점

```cpp
// src/main.cpp
int wmain() {
    auto vm   = AppBuilder::build();
    auto view = CliView(vm.get());

    view.bindCallbacks();   // onPhaseChanged, onProgressChanged, onCompleted 등록
    vm->execute();          // Phase 1~5 순차 실행

    if (vm->needReboot) {
        view.promptReboot();
    }
    return 0;
}
```

---

## 12. 빌드 설정

| 항목 | 값 |
|------|----|
| `PlatformToolset` | `v143` |
| `LanguageStandard` | `stdcpp23` |
| `LanguageStandard_C` | `stdc17` |
| `CharacterSet` | `Unicode` |
| `SubSystem` | `Console` |
| `AdditionalDependencies` | `setupapi.lib; newdev.lib; cfgmgr32.lib` |
| `RuntimeLibrary` | `MultiThreaded` (WinPE 정적 링크) |
| `Optimization (Release)` | `O2` |

---

## 13. WinPE 특이사항 처리

| 상황 | 처리 방법 |
|------|-----------|
| HardLink 실패 | `FILE_QUEUE_COMMIT` exit `0x00000000` → 성공으로 처리 |
| `CM_PROB_NEED_CLASS_CONFIG (0x38)` | pending 상태, 오류 아님 |
| `0x00000bc3` (`ERROR_RESTART_DEVICE_ONLY`) | NeedReboot 플래그만 set |
| SMBIOS `productName` 공백 포함 | 폴더명 정규화 불필요 — `FindFirstFileW`는 공백 경로 허용 |
| `Y:\` 드라이브 미존재 | `Win32InfSearchAdapter::directoryExists()` 에서 조기 실패 처리 |

---

## 14. 구현 순서

```
Phase 1  도메인 모델 + 모든 Port 인터페이스 정의
          └─ domain/model/*.hpp
          └─ application/port/**/*.hpp

Phase 2  Use Case Interactor 구현 (Mock Port 연결, 단위 테스트)
          └─ application/usecase/*.cpp
          └─ tests/mock/*.hpp

Phase 3  Win32SmbiosAdapter 구현 + SMBIOS 파싱 검증
          (실기기 또는 VM에서 productName 정확성 확인)

Phase 4  Win32InfSearchAdapter + Win32DriverStoreAdapter 구현

Phase 5  Win32DeviceRepository + Win32DriverInstallAdapter 구현

Phase 6  ViewModel + CliView + AppBuilder 조립

Phase 7  WinPE 실환경 통합 테스트
          setupapi.dev.log 출력 비교 검증
```

#include "Memory.h"

using namespace winternl_h;

void InitMemoryLogger() {
    try {
        auto logger = spdlog::basic_logger_mt("memory", "memory.log", true);
        logger->set_pattern("[%d-%m-%Y %H:%M:%S] [%l] %v");
        logger->flush_on(spdlog::level::info);

        logger->info("===============================================");
        logger->info("             Volcano ESP by flosur");
        logger->info("===============================================");

        spdlog::register_logger(logger);
    }
    catch (const spdlog::spdlog_ex& e) {
        std::cout << "Memory log init failed: " << e.what() << std::endl;
    }
}

OBJECT_ATTRIBUTES Memory::InitObjectAttributes(PUNICODE_STRING name, ULONG attributes, HANDLE hRoot, PSECURITY_DESCRIPTOR security) {
    OBJECT_ATTRIBUTES object;
    object.Length = sizeof(OBJECT_ATTRIBUTES);
    object.ObjectName = name;
    object.Attributes = attributes;
    object.RootDirectory = hRoot;
    object.SecurityDescriptor = security;
    return object;
}

DWORD Memory::GetPID(const wchar_t* processName) {
    std::string processNameStr(processName, processName + wcslen(processName));

    HANDLE handleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, false);

    if (handleSnap != INVALID_HANDLE_VALUE) {

        PROCESSENTRY32 processEntry = {};
        processEntry.dwSize = sizeof(PROCESSENTRY32);

        while (Process32Next(handleSnap, &processEntry)) {
            if (_wcsicmp(processEntry.szExeFile, processName) == 0) {
                CloseHandle(handleSnap);

                if (processNameStr == "cs2.exe") {
                    spdlog::get("memory")->info("Process {} found with PID: {}", processNameStr, processEntry.th32ProcessID);
                }

                return processEntry.th32ProcessID;
            }
        }

        CloseHandle(handleSnap);
    }

    spdlog::get("memory")->error("Process {} not found in system", processNameStr);
    return 0;
}

const std::uintptr_t Memory::GetModuleAddress(const wchar_t* moduleName) const noexcept {
    std::string moduleNameStr(moduleName, moduleName + wcslen(moduleName));

    HANDLE handleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, targetpID);

    if (handleSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 moduleEntry = {};
        moduleEntry.dwSize = sizeof(MODULEENTRY32);

        std::uintptr_t result = 0;
        while (::Module32Next(handleSnap, &moduleEntry)) {
            if (_wcsicmp(moduleName, moduleEntry.szModule) == 0) {
                result = reinterpret_cast<std::uintptr_t>(moduleEntry.modBaseAddr);
                CloseHandle(handleSnap);
                spdlog::get("memory")->info("Module {} loaded at base address: 0x{:X}", moduleNameStr, result);
                return result;
            }
        }

        CloseHandle(handleSnap);
    }

    spdlog::get("memory")->error("Module {} not found in target process", moduleNameStr);
    return 0;
}

bool Memory::IsHandleValid(HANDLE handle) {
    return handle && handle != INVALID_HANDLE_VALUE;
}

HANDLE Memory::GetHijackedHandle(DWORD targetPID) {

    spdlog::get("memory")->info("Starting handle hijacking for cs2.exe");

    HANDLE hDuplicated = nullptr;
    HANDLE hOwner = nullptr;

    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");

    auto RtlAdjustPrivilege = reinterpret_cast<_RtlAdjustPrivilege>(GetProcAddress(ntdll, "RtlAdjustPrivilege"));
    auto NtQuerySystemInformation = reinterpret_cast<_NtQuerySystemInformation>(GetProcAddress(ntdll, "NtQuerySystemInformation"));
    auto NtDuplicateObject = reinterpret_cast<_NtDuplicateObject>(GetProcAddress(ntdll, "NtDuplicateObject"));
    auto NtOpenProcess = reinterpret_cast<_NtOpenProcess>(GetProcAddress(ntdll, "NtOpenProcess"));

    BOOLEAN oldPrivilege;
    NTSTATUS privStatus = RtlAdjustPrivilege(SeDebugPriv, TRUE, FALSE, &oldPrivilege);

    DWORD size = sizeof(SYSTEM_HANDLE_INFORMATION);
    PSYSTEM_HANDLE_INFORMATION allHandlesInfo = nullptr;
    NTSTATUS status = NULL;

    while (true) {
        if (allHandlesInfo) {
            delete[] reinterpret_cast<unsigned char*>(allHandlesInfo);
        }

        allHandlesInfo = reinterpret_cast<PSYSTEM_HANDLE_INFORMATION>(new unsigned char[size]);
        status = NtQuerySystemInformation(SystemHandleInformation, allHandlesInfo, size, nullptr);

        if (status == STATUS_SUCCESS) {
            break;
        }
        size = static_cast<DWORD>(size * 1.5);
    }

    spdlog::get("memory")->info("Retrieved {} total system handles", allHandlesInfo->HandleCount);

    auto steamPID = GetPID(L"steam.exe");

    spdlog::get("memory")->info("Scanning steam.exe handles for CS2 access...");

    int totalScanned = 0;
    int steamHandles = 0;
    int processTypeHandles = 0;
    int candidateHandles = 0;

    for (ULONG i = 0; i < allHandlesInfo->HandleCount; ++i) {
        const auto& currentHandle = allHandlesInfo->Handles[i];
        totalScanned++;

        if (!IsHandleValid(reinterpret_cast<HANDLE>(currentHandle.Handle))) {
            continue;
        }

        // skip handles not opened by steam.exe
        if (currentHandle.ProcessId != steamPID) {
            continue;
        }
        steamHandles++;

        if (currentHandle.ObjectTypeNumber != ProcessHandleType) {
            continue;
        }
        processTypeHandles++;

        //0x101400
        DWORD requiredAccess =
            SYNCHRONIZE |
            PROCESS_QUERY_LIMITED_INFORMATION |
            PROCESS_QUERY_INFORMATION;

        if (currentHandle.GrantedAccess != requiredAccess) {
            continue;
        }
        
        candidateHandles++;

        // at this point, we found a handle opened by steam.exe with the required permissions to read memory
        // we still don't know which process it targets
        // to find out, we need to duplicate the handle into our own process
        spdlog::get("memory")->debug("Found candidate handle #{}: 0x{:X} with access rights: 0x{:X}", candidateHandles, currentHandle.Handle, currentHandle.GrantedAccess);

        spdlog::get("memory")->debug("Process that opened the handle - PID: {}", currentHandle.ProcessId);

        OBJECT_ATTRIBUTES objAttributes = InitObjectAttributes(NULL, NULL, NULL, NULL);
        CLIENT_ID clientId = { };
        clientId.UniqueProcess = (reinterpret_cast<DWORD*>(currentHandle.ProcessId));

        // open a handle (hOwner) to steam.exe with PROCESS_DUP_HANDLE rights
        // hOwner gives us access to steam.exe with PROCESS_DUP_HANDLE permission, so we can duplicate its open handles (currentHandle)
        status = NtOpenProcess(&hOwner, PROCESS_DUP_HANDLE, &objAttributes, &clientId);

        if (!IsHandleValid(hOwner) || !NT_SUCCESS(status)) {
            spdlog::get("memory")->debug("Failed to open a handle to steam.exe with PROCESS_DUP_HANDLE - status: 0x{:X}", status);
            continue;
        }

        status = NtDuplicateObject(
            hOwner,        // hOwner is the handle that points steam.exe
            reinterpret_cast<HANDLE>(currentHandle.Handle), // this is the handle inside steam.exe that passed our filters
            NtCurrentProcess, // the duplicated handle will be created inside our own process (ESP.exe)
            &hDuplicated, // the duplicated handle will be stored in hDuplicated
            PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
            0,
            0);

        if (!IsHandleValid(hDuplicated) || !NT_SUCCESS(status)) {
            spdlog::get("memory")->debug("Failed to duplicate steam.exe's handle 0x{:X} - status: 0x{:X}",
                currentHandle.Handle, status);
            CloseHandle(hOwner);
            continue;
        }

        CloseHandle(hOwner); // closes hOwner, because we already duplicated it

        DWORD handleTargetPID = GetProcessId(hDuplicated);

        if (handleTargetPID == targetPID) {
            spdlog::get("memory")->info("SUCCESS! Hijacked handle from steam.exe that points to CS2");
            spdlog::get("memory")->info("Duplicated handle: 0x{:X}, granted access: 0x{:X}",
                currentHandle.Handle, currentHandle.GrantedAccess);
            spdlog::get("memory")->info("Scan statistics: {} total handles, {} from steam.exe, {} process handles, {} with correct permissions",
                totalScanned, steamHandles, processTypeHandles, candidateHandles);
            delete[] allHandlesInfo;
            return hDuplicated;
        }

        spdlog::get("memory")->warn("Handle: 0x{:X} points to wrong process - PID: {}", currentHandle.Handle, handleTargetPID);
        CloseHandle(hDuplicated);
    }

    spdlog::get("memory")->error("Handle hijacking FAILED! No suitable handle found in steam.exe");
    spdlog::get("memory")->error("Final statistics: {} total handles, {} from steam.exe, {} process handles, {} with correct permissions",
        totalScanned, steamHandles, processTypeHandles, candidateHandles);
    delete[] allHandlesInfo;
    return nullptr;
}

Memory::Memory(const wchar_t* processName) noexcept {
    if (!spdlog::get("memory")) {
        InitMemoryLogger();
    }

    targetpID = GetPID(processName);
    if (targetpID != 0) {
        hHijacked = GetHijackedHandle(static_cast<DWORD>(targetpID));

        if (hHijacked) {
            spdlog::get("memory")->info("Memory initialization successful - CS2 memory read access ready!");
        }
        else {
            spdlog::get("memory")->error("Memory initialization failed - handle hijacking unsuccessful");
        }
     }
}

Memory::~Memory() {
    if (hHijacked) {
        CloseHandle(hHijacked);
        spdlog::get("memory")->info("Memory system destroyed - hijacked handle closed and resources cleaned up");
    }
}
#pragma once
#include <iostream>
#include <vector>

#include <Windows.h>

inline auto SetSystemEnviromentPrivilege()
{
    TOKEN_PRIVILEGES tp{};
    LUID luid{};
    HANDLE hToken = nullptr;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
        std::cout << "Can't open process token" << std::endl;
        return false;
    }

    if (!LookupPrivilegeValue(nullptr, SE_SYSTEM_ENVIRONMENT_NAME, &luid)) {
        std::cout << "Can't lookup SE_SYSTEM_ENVIRONMENT_NAME privilege" << std::endl;
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, false, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr)) {
        std::cout << "Can't adjust token privileges" << std::endl;
        return false;
    }

    if (!CloseHandle(hToken)) {
        std::cout << "Can't close token handle" << std::endl;
        return false;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
        std::cout << "Can't not all privileges assigned" << std::endl;
        return false;
    }

    return true;
}

inline auto UpdatePrevBootVar(const std::string lpName, const std::string lpGuid, std::vector<uint8_t> buffer)
{
    if (!SetSystemEnviromentPrivilege())
        return false;

    if (!SetFirmwareEnvironmentVariableA(lpName.c_str(), lpGuid.c_str(), buffer.data(), static_cast<DWORD>(buffer.size()))) {
        std::cout << "Can't set new value to '" << lpName << "'" << std::endl;
        return false;
    }

    return true;
}

inline auto Reboot()
{
    if (!ExitWindowsEx(EWX_REBOOT,
        SHTDN_REASON_MAJOR_OTHER |
        SHTDN_REASON_MINOR_OTHER |
        SHTDN_REASON_FLAG_PLANNED))
        return false;
				
    return true;
}
#include <iostream>
#include <vector>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>

constexpr auto bootEntry = "Boot Ubuntu from 107 GiB Btrfs volume"; // To run Linux

#else

#include <unistd.h>
extern "C" {
    #include <efivar/efivar.h>
}

constexpr auto bootEntry = "Boot Windows 10 from EFI system partition"; // To run Windows
#endif

constexpr auto lpName = "PreviousBoot";
constexpr auto lpGuid = "{36d08fa7-cf0b-42f5-8f14-68df73ed3740}";

auto GetNewVarDataForPreviousBoot() -> std::vector<uint8_t>
{
    std::vector<uint8_t> buffer;
    buffer.reserve(std::string(bootEntry).length()* 2 + 2);
    for (auto c : std::string(bootEntry)) {
        buffer.push_back(c);
        buffer.push_back(0);
    }
    buffer.push_back(0);
    buffer.push_back(0);

    return buffer;
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

auto SetSystemEnviromentPrivilege()
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

auto SetPreviousBootVar()
{
    if (!SetSystemEnviromentPrivilege())
        return false;

    auto buffer = GetNewVarDataForPreviousBoot();

    if (!SetFirmwareEnvironmentVariableA(lpName, lpGuid, buffer.data(), static_cast<uint32_t>(buffer.size()))) {
        std::cout << "Can't set new value to '" << lpName << "'" << std::endl;
        return false;
    }

    return true;
}

#else

auto SetPreviousBootVar()
{
    if (getuid()) {
        std::cout << "Run as root" << std::endl;
        return false;
    }

    if (!efi_variables_supported()) {
        std::cout << "Your system don't support EFI variables" << std::endl;
        return false;
    }
        
    efi_guid_t guid{};
        
    if (efi_str_to_guid(lpGuid, &guid)) {
        std::cout << "Can't parse GUID from string" << std::endl;
        return false;
    }

	uint32_t attrs = 0;
        
    if (efi_get_variable_attributes(guid, lpName, &attrs)) {
        std::cout << "Can't get attributes from '" << lpName << "'" << std::endl;
        return false;
    }
        
    auto buffer = GetNewVarDataForPreviousBoot();
        
    if (efi_set_variable(guid, lpName, buffer.data(), buffer.size(), attrs, 0644)) {
        std::cout << "Can't set new value to '" << lpName << "'" << std::endl;
        return false;
    }

    return true;
}

#endif

auto main() -> int
{
    if (!SetPreviousBootVar()) {
        std::cout << "Press any key to continue..." << std::endl;
        std::cin.get();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

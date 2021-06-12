#pragma once

#include <iostream>
#include <vector>

#include <unistd.h>
extern "C" {
	#include <efivar/efivar.h>
}

inline auto UpdatePrevBootVar(const std::string lpName, const std::string lpGuid, std::vector<uint8_t> buffer)
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

    if (efi_str_to_guid(lpGuid.c_str(), &guid)) {
        std::cout << "Can't parse GUID from string" << std::endl;
        return false;
    }

    uint32_t attrs = 0;

    if (efi_get_variable_attributes(guid, lpName.c_str(), &attrs)) {
        std::cout << "Can't get attributes from '" << lpName << "'" << std::endl;
        return false;
    }

    if (efi_set_variable(guid, lpName.c_str(), buffer.data(), buffer.size(), attrs, 0644)) {
        std::cout << "Can't set new value to '" << lpName << "'" << std::endl;
        return false;
    }

    return true;
}
#include <iostream>
#include <vector>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include "WinAPI.hpp"
constexpr auto bootEntry = "linux";
#else
#include "LinuxAPI.hpp"
constexpr auto bootEntry = "windows";
#endif

constexpr auto lpName = "PreviousBoot";
constexpr auto lpGuid = "{36d08fa7-cf0b-42f5-8f14-68df73ed3740}";

auto GetNewVarDataForPreviousBoot(const std::string& entry) -> std::vector<uint8_t>
{
    std::vector<uint8_t> buffer;
    buffer.reserve(entry.length() * 2 + 2);
    for (auto c : entry) {
        buffer.push_back(c);
        buffer.push_back(0);
    }
    buffer.push_back(0);
    buffer.push_back(0);

    return buffer;
}

int main(const int argc, char** argv)
{
    if (const auto buffer = GetNewVarDataForPreviousBoot(bootEntry);
        !UpdatePrevBootVar(lpName, lpGuid, buffer)) {
        std::cout << "Press any key to continue..." << std::endl;
        std::cin.get();
        return EXIT_FAILURE;
    }
}
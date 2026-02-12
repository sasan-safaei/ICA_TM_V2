#include "ICA2506_Func.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {
void PrintUsage(const char* prog) {
    std::cerr << "Usage: " << prog << " <firmware.bin> [interface.cfg] [target.cfg] [address_hex] [adapter_khz]\n";
    std::cerr << "Example: " << prog << " firmware.bin interface/stlink.cfg target/stm32g0x.cfg 0x08000000 1000\n";
}

bool ParseHex(const std::string& text, uint32_t& value) {
    char* end = nullptr;
    const unsigned long parsed = std::strtoul(text.c_str(), &end, 0);
    if (end == text.c_str() || *end != '\0') {
        return false;
    }
    value = static_cast<uint32_t>(parsed);
    return true;
}
} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 2;
    }

    const std::string binPath = argv[1];
    if (argc == 2) {
        const bool ok = Flash_STM32G030F6(binPath);
        return ok ? 0 : 1;
    }

    if (argc < 6) {
        PrintUsage(argv[0]);
        return 2;
    }

    const std::string interfaceCfg = argv[2];
    const std::string targetCfg = argv[3];

    uint32_t address = 0;
    if (!ParseHex(argv[4], address)) {
        std::cerr << "Invalid address: " << argv[4] << "\n";
        return 2;
    }

    const int adapterKhz = std::atoi(argv[5]);
    if (adapterKhz <= 0) {
        std::cerr << "Invalid adapter_khz: " << argv[5] << "\n";
        return 2;
    }

    const bool ok = Flash_STM32G030F6(binPath, interfaceCfg, targetCfg, address, adapterKhz);
    return ok ? 0 : 1;
}

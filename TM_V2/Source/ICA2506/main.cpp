#include "ICA2506_Flash_Micro.h"
#include "ICA2506_UART.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {
void PrintUsage(const char* prog) {
    std::cerr << "Usage: " << prog << " <firmware.bin> [interface.cfg] [target.cfg] [address_hex] [adapter_khz]\n";
    std::cerr << "       " << prog << " --ob [interface.cfg] [target.cfg] [adapter_khz]\n";
    std::cerr << "       " << prog << " --cmp <firmware.bin> [interface.cfg] [target.cfg] [address_hex] [adapter_khz]\n";
    std::cerr << "       " << prog << " --erase [interface.cfg] [target.cfg] [adapter_khz]\n";
    std::cerr << "       " << prog << " --reset [interface.cfg] [target.cfg] [adapter_khz]\n";
    std::cerr << "       " << prog << " --uart \"HEX BYTES\" [port] [baud] [idle_ms]\n";
    std::cerr << "Example: " << prog << " firmware.bin interface/stlink.cfg target/stm32g0x.cfg 0x08000000 1000\n";
    std::cerr << "Example: " << prog << " --ob interface/raspberrypi2-native.cfg target/stm32g0x.cfg 200\n";
    std::cerr << "Example: " << prog << " --cmp firmware.bin interface/raspberrypi2-native.cfg target/stm32g0x.cfg 0x08000000 200\n";
    std::cerr << "Example: " << prog << " --erase interface/raspberrypi2-native.cfg target/stm32g0x.cfg 200\n";
    std::cerr << "Example: " << prog << " --reset interface/raspberrypi2-native.cfg target/stm32g0x.cfg 200\n";
    std::cerr << "Example: " << prog << " --uart \"49 50 A8\" /dev/ttyAMA1 9600 200\n";
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

std::string HexToAscii(const std::string& hexStr) {
    std::string ascii;
    std::string hex;
    
    for (char ch : hexStr) {
        if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r') {
            hex += ch;
        }
        
        if (hex.length() == 2) {
            unsigned int byte;
            if (sscanf(hex.c_str(), "%2x", &byte) == 1) {
                if (byte >= 32 && byte <= 126) {
                    ascii += static_cast<char>(byte);
                } else {
                    ascii += '.';
                }
            }
            hex.clear();
        }
    }
    
    return ascii;
}
} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 2;
    }

    if (std::string(argv[1]) == "--ob") {
        if (argc == 2) {
            const bool ok = Read_STM32G030F6_OptionBytes();
            return ok ? 0 : 1;
        }
        if (argc < 5) {
            PrintUsage(argv[0]);
            return 2;
        }
        const std::string interfaceCfg = argv[2];
        const std::string targetCfg = argv[3];
        const int adapterKhz = std::atoi(argv[4]);
        if (adapterKhz <= 0) {
            std::cerr << "Invalid adapter_khz: " << argv[4] << "\n";
            return 2;
        }
        const bool ok = Read_STM32G030F6_OptionBytes(interfaceCfg, targetCfg, adapterKhz);
        return ok ? 0 : 1;
    }

    if (std::string(argv[1]) == "--cmp") {
        if (argc == 3) {
            const FlashCompareResult res = Compare_STM32G030F6_Flash(argv[2]);
            if (res == FlashCompareResult::Empty) {
                std::cout << "Flash is empty" << std::endl;
                return 0;
            }
            if (res == FlashCompareResult::Same) {
                std::cout << "Flash matches firmware" << std::endl;
                return 0;
            }
            if (res == FlashCompareResult::Different) {
                std::cout << "Flash programmed with different firmware" << std::endl;
                return 0;
            }
            std::cerr << "Flash compare failed" << std::endl;
            return 1;
        }
        if (argc < 7) {
            PrintUsage(argv[0]);
            return 2;
        }

        const std::string binPath = argv[2];
        const std::string interfaceCfg = argv[3];
        const std::string targetCfg = argv[4];

        uint32_t address = 0;
        if (!ParseHex(argv[5], address)) {
            std::cerr << "Invalid address: " << argv[5] << "\n";
            return 2;
        }

        const int adapterKhz = std::atoi(argv[6]);
        if (adapterKhz <= 0) {
            std::cerr << "Invalid adapter_khz: " << argv[6] << "\n";
            return 2;
        }

        const FlashCompareResult res = Compare_STM32G030F6_Flash(binPath, interfaceCfg, targetCfg, address, adapterKhz);
        if (res == FlashCompareResult::Empty) {
            std::cout << "Flash is empty" << std::endl;
            return 0;
        }
        if (res == FlashCompareResult::Same) {
            std::cout << "Flash matches firmware" << std::endl;
            return 0;
        }
        if (res == FlashCompareResult::Different) {
            std::cout << "Flash programmed with different firmware" << std::endl;
            return 0;
        }
        std::cerr << "Flash compare failed" << std::endl;
        return 1;
    }

    if (std::string(argv[1]) == "--erase") {
        if (argc == 2) {
            const bool ok = Erase_STM32G030F6_Flash();
            return ok ? 0 : 1;
        }
        if (argc < 5) {
            PrintUsage(argv[0]);
            return 2;
        }

        const std::string interfaceCfg = argv[2];
        const std::string targetCfg = argv[3];
        const int adapterKhz = std::atoi(argv[4]);
        if (adapterKhz <= 0) {
            std::cerr << "Invalid adapter_khz: " << argv[4] << "\n";
            return 2;
        }

        const bool ok = Erase_STM32G030F6_Flash(interfaceCfg, targetCfg, adapterKhz);
        return ok ? 0 : 1;
    }

    if (std::string(argv[1]) == "--reset") {
        if (argc == 2) {
            const bool ok = Reset_STM32G030F6();
            return ok ? 0 : 1;
        }
        if (argc < 5) {
            PrintUsage(argv[0]);
            return 2;
        }

        const std::string interfaceCfg = argv[2];
        const std::string targetCfg = argv[3];
        const int adapterKhz = std::atoi(argv[4]);
        if (adapterKhz <= 0) {
            std::cerr << "Invalid adapter_khz: " << argv[4] << "\n";
            return 2;
        }

        const bool ok = Reset_STM32G030F6(interfaceCfg, targetCfg, adapterKhz);
        return ok ? 0 : 1;
    }

    if (std::string(argv[1]) == "--uart") {
        if (argc < 3) {
            PrintUsage(argv[0]);
            return 2;
        }
        const std::string hexStream = argv[2];
        std::string port = "/dev/ttyAMA1";
        int baud = 9600;
        int idleMs = 200;

        if (argc >= 4) {
            port = argv[3];
        }
        if (argc >= 5) {
            baud = std::atoi(argv[4]);
            if (baud <= 0) {
                std::cerr << "Invalid baud: " << argv[4] << "\n";
                return 2;
            }
        }
        if (argc >= 6) {
            idleMs = std::atoi(argv[5]);
            if (idleMs <= 0) {
                std::cerr << "Invalid idle_ms: " << argv[5] << "\n";
                return 2;
            }
        }

        std::string responseHex;
        const bool ok = UART_SendHexStream(hexStream, responseHex, port, baud, idleMs);
        if (!ok) {
            return 1;
        }
        std::cout << responseHex << std::endl;
        std::cout << HexToAscii(responseHex) << std::endl;
        return 0;
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

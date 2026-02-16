#include <iostream>
#include <iomanip>
#include <cstdint>
#include <string>

void PrintHelp() {
    std::cout << "STM32G030 OPTR Calculator\n\n";
    std::cout << "This tool helps calculate the OPTR (Option Bytes) value.\n";
    std::cout << "Enter values for each field (or press Enter for default):\n\n";
}

void PrintCurrentConfig(uint32_t optr) {
    std::cout << "\n=== Current Configuration ===\n";
    std::cout << "OPTR Value: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << optr << "\n\n";
    
    const uint8_t rdp = static_cast<uint8_t>(optr & 0xFF);
    const uint8_t bor = static_cast<uint8_t>((optr >> 8) & 0x7);
    
    std::cout << "RDP:          0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(rdp);
    if (rdp == 0xAA) std::cout << " (Level 0 - No protection)";
    else if (rdp == 0xCC) std::cout << " (Level 2 - PERMANENT LOCK!)";
    else std::cout << " (Level 1 - Debug disabled)";
    std::cout << "\n";
    
    std::cout << "BOR_LEV:      " << std::dec << static_cast<int>(bor) << "\n";
    std::cout << "nRST_STOP:    " << ((optr >> 12) & 0x1) << "\n";
    std::cout << "nRST_STDBY:   " << ((optr >> 13) & 0x1) << "\n";
    std::cout << "nRST_SHDW:    " << ((optr >> 14) & 0x1) << "\n";
    std::cout << "IWDG_SW:      " << ((optr >> 16) & 0x1) << "\n";
    std::cout << "IWDG_STOP:    " << ((optr >> 17) & 0x1) << "\n";
    std::cout << "IWDG_STDBY:   " << ((optr >> 18) & 0x1) << "\n";
    std::cout << "WWDG_SW:      " << ((optr >> 19) & 0x1) << "\n";
    std::cout << "SRAM_PARITY:  " << ((optr >> 21) & 0x1) << "\n";
    std::cout << "nBOOT0:       " << ((optr >> 24) & 0x1) << "\n";
    std::cout << "nBOOT1:       " << ((optr >> 25) & 0x1) << "\n";
    std::cout << "nBOOT_SEL:    " << ((optr >> 26) & 0x1) << "\n";
    std::cout << "BOOT_LOCK:    " << ((optr >> 27) & 0x1) << "\n";
    std::cout << "\n";
}

int GetBit(const std::string& prompt, int defaultVal) {
    std::cout << prompt << " [" << defaultVal << "]: ";
    std::string input;
    std::getline(std::cin, input);
    if (input.empty()) return defaultVal;
    return (input[0] == '1' || input[0] == 'y' || input[0] == 'Y') ? 1 : 0;
}

int GetValue(const std::string& prompt, int defaultVal, int maxVal = 255) {
    std::cout << prompt << " [" << defaultVal << "]: ";
    std::string input;
    std::getline(std::cin, input);
    if (input.empty()) return defaultVal;
    try {
        int val = std::stoi(input, nullptr, 0);
        if (val < 0 || val > maxVal) return defaultVal;
        return val;
    } catch (...) {
        return defaultVal;
    }
}

int main() {
    PrintHelp();
    
    std::cout << "Start with current OPTR value? (y/n) [n]: ";
    std::string useCurrentStr;
    std::getline(std::cin, useCurrentStr);
    
    uint32_t optr = 0x1FBEF8AA; // Default typical value
    
    if (!useCurrentStr.empty() && (useCurrentStr[0] == 'y' || useCurrentStr[0] == 'Y')) {
        std::cout << "Enter current OPTR (hex, e.g., 0x1FBEF8AA): ";
        std::string optrStr;
        std::getline(std::cin, optrStr);
        try {
            optr = std::stoul(optrStr, nullptr, 16);
        } catch (...) {
            std::cout << "Invalid value, using default\n";
        }
    }
    
    PrintCurrentConfig(optr);
    
    std::cout << "\n=== Modify Configuration ===\n";
    std::cout << "WARNING: Never set RDP to 0xCC - it PERMANENTLY locks the chip!\n\n";
    
    int rdp = GetValue("RDP (hex, 0xAA=no protection, 0xCC=PERMANENT LOCK)", 
                       static_cast<int>(optr & 0xFF), 255);
    
    int bor = GetValue("BOR_LEV (0-7)", (optr >> 8) & 0x7, 7);
    
    int nrst_stop = GetBit("nRST_STOP (1=allow Stop mode, 0=reset on Stop)", (optr >> 12) & 0x1);
    int nrst_stdby = GetBit("nRST_STDBY (1=allow Standby)", (optr >> 13) & 0x1);
    int nrst_shdw = GetBit("nRST_SHDW (1=allow Shutdown)", (optr >> 14) & 0x1);
    
    int iwdg_sw = GetBit("IWDG_SW (1=software control)", (optr >> 16) & 0x1);
    int iwdg_stop = GetBit("IWDG_STOP (1=frozen in Stop)", (optr >> 17) & 0x1);
    int iwdg_stdby = GetBit("IWDG_STDBY (1=frozen in Standby)", (optr >> 18) & 0x1);
    int wwdg_sw = GetBit("WWDG_SW (1=software control)", (optr >> 19) & 0x1);
    
    int sram_parity = GetBit("SRAM_PARITY (1=enable parity check)", (optr >> 21) & 0x1);
    
    std::cout << "\nBoot Configuration:\n";
    int nboot_sel = GetBit("nBOOT_SEL (0=use BOOT0 pin, 1=use nBOOT0/1 bits)", (optr >> 26) & 0x1);
    int nboot0 = GetBit("nBOOT0 (boot selection bit 0)", (optr >> 24) & 0x1);
    int nboot1 = GetBit("nBOOT1 (boot selection bit 1)", (optr >> 25) & 0x1);
    int boot_lock = GetBit("BOOT_LOCK (1=lock until reset)", (optr >> 27) & 0x1);
    
    // Build new OPTR
    uint32_t newOptr = 0;
    newOptr |= static_cast<uint32_t>(rdp & 0xFF);
    newOptr |= static_cast<uint32_t>(bor & 0x7) << 8;
    if (nrst_stop)   newOptr |= (1U << 12);
    if (nrst_stdby)  newOptr |= (1U << 13);
    if (nrst_shdw)   newOptr |= (1U << 14);
    if (iwdg_sw)     newOptr |= (1U << 16);
    if (iwdg_stop)   newOptr |= (1U << 17);
    if (iwdg_stdby)  newOptr |= (1U << 18);
    if (wwdg_sw)     newOptr |= (1U << 19);
    if (sram_parity) newOptr |= (1U << 21);
    if (nboot0)      newOptr |= (1U << 24);
    if (nboot1)      newOptr |= (1U << 25);
    if (nboot_sel)   newOptr |= (1U << 26);
    if (boot_lock)   newOptr |= (1U << 27);
    
    PrintCurrentConfig(newOptr);
    
    std::cout << "To program this value, run:\n";
    std::cout << "  ./ica2506_flash --write-ob 0x" << std::hex << std::uppercase 
              << std::setw(8) << std::setfill('0') << newOptr << "\n\n";
    
    if ((rdp & 0xFF) == 0xCC) {
        std::cout << "!!! WARNING: This configuration will PERMANENTLY LOCK the chip !!!\n";
        std::cout << "!!! The tool will refuse to program RDP=0xCC for your safety !!!\n";
    }
    
    return 0;
}

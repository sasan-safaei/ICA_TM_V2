# STM32G030 Option Bytes Programming Guide

## Quick Start

### 1. Read Current Option Bytes
```bash
./ica2506_flash --ob
```

### 2. Calculate New OPTR Value
```bash
./optr_calc
# Follow the interactive prompts
# It will display the command to program
```

### 3. Program Option Bytes
```bash
./ica2506_flash --write-ob 0x1FFEF8AA
```

## Common Configurations

### Enable Stop Mode (Fix nRST_STOP=0)
Current: `0x1FBEF8AA` (nRST_STOP=0, can't use Stop mode)
Fixed:   `0x1FFEF8AA` (nRST_STOP=1, can use Stop mode)

```bash
./ica2506_flash --write-ob 0x1FFEF8AA
```

### Boot from System Memory (STM32 Bootloader)
To boot from system memory for UART programming:
- Set nBOOT_SEL=1, nBOOT1=0, nBOOT0=1

```bash
# Use calculator to compute the value
./optr_calc
```

### Default Safe Configuration
```bash
./ica2506_flash --write-ob 0x1FFEF8AA
```

This sets:
- RDP: 0xAA (no protection)
- BOR_LEV: 1 (~2.0V)
- nRST_STOP: 1 (allow Stop mode)
- nRST_STDBY: 1 (allow Standby)
- nRST_SHDW: 1 (allow Shutdown)
- IWDG_SW: 1 (software watchdog)
- IWDG_STOP: 1 (frozen in Stop)
- IWDG_STDBY: 1 (frozen in Standby)
- WWDG_SW: 1 (software window watchdog)
- SRAM_PARITY: 1 (enabled)
- nBOOT0: 0 (boot from flash when BOOT0 pin low)
- nBOOT1: 1
- nBOOT_SEL: 0 (use BOOT0 pin)
- BOOT_LOCK: 1 (locked)

## Using in C++ Code

```cpp
#include "Stm32G030Class.h"

int main() {
    STM32G030F6_Class stm32;
    
    // Read current option bytes
    uint32_t currentOptr;
    if (stm32.ReadOptionBytes(currentOptr)) {
        printf("Current OPTR: 0x%08X\n", currentOptr);
    }
    
    // Build new OPTR value
    uint32_t newOptr = STM32G030F6_Class::BuildOPTR(
        0xAA,  // RDP (0xAA = no protection)
        1,     // BOR_LEV
        true,  // nRST_STOP (enable Stop mode)
        true,  // nRST_STDBY
        true,  // nRST_SHDW
        true,  // IWDG_SW
        true,  // IWDG_STOP
        true,  // IWDG_STDBY
        true,  // WWDG_SW
        true,  // SRAM_PARITY
        false, // nBOOT0
        true,  // nBOOT1
        false, // nBOOT_SEL
        true   // BOOT_LOCK
    );
    
    // Program option bytes
    if (stm32.WriteOptionBytes(newOptr)) {
        printf("Option bytes programmed successfully!\n");
    }
    
    return 0;
}
```

## Option Byte Bit Definitions

### RDP (Bits 7:0) - Read Protection
- **0xAA**: Level 0 - No protection (debug/programming allowed)
- **0xCC**: Level 2 - **PERMANENT PROTECTION** (irreversible!)
- **Other**: Level 1 - Debug disabled, can be changed

### BOR_LEV (Bits 10:8) - Brown-Out Reset Level
- 0: BOR OFF
- 1: ~2.0V
- 2: ~2.2V
- 3: ~2.5V
- 4: ~2.8V

### nRST_STOP (Bit 12)
- **0**: Reset generated when entering Stop mode
- **1**: No reset, can use Stop mode

### nRST_STDBY (Bit 13)
- **0**: Reset generated when entering Standby mode
- **1**: No reset, can use Standby mode

### nRST_SHDW (Bit 14)
- **0**: Reset generated when entering Shutdown mode
- **1**: No reset, can use Shutdown mode

### IWDG_SW (Bit 16) - Independent Watchdog
- **0**: Hardware (auto-start)
- **1**: Software controlled

### IWDG_STOP (Bit 17)
- **0**: IWDG runs in Stop mode
- **1**: IWDG frozen in Stop mode

### IWDG_STDBY (Bit 18)
- **0**: IWDG runs in Standby
- **1**: IWDG frozen in Standby

### WWDG_SW (Bit 19) - Window Watchdog
- **0**: Hardware (auto-start)
- **1**: Software controlled

### SRAM_PARITY (Bit 21)
- **0**: Disabled
- **1**: Enabled (recommended)

### Boot Configuration

#### nBOOT_SEL (Bit 26)
- **0**: Use BOOT0 pin to select boot mode
- **1**: Use nBOOT0/nBOOT1 bits (ignore pin)

#### When nBOOT_SEL=0 (Pin mode):
- **BOOT0 pin LOW**: Boot from flash (0x08000000)
- **BOOT0 pin HIGH**: Boot from system memory (bootloader)

#### When nBOOT_SEL=1 (Bit mode):
- nBOOT1=0, nBOOT0=0: Boot from system memory
- nBOOT1=0, nBOOT0=1: Boot from system memory
- nBOOT1=1, nBOOT0=0: Boot from flash
- nBOOT1=1, nBOOT0=1: Boot from SRAM

### BOOT_LOCK (Bit 27)
- **0**: Boot config can be changed by software
- **1**: Boot config locked until reset

## Safety Features

The `WriteOptionBytes()` method includes safety checks:

1. ✅ Reads current value first for verification
2. ✅ **Refuses to set RDP=0xCC** (permanent lock protection)
3. ✅ Warns when changing RDP level
4. ✅ Verifies write operation after programming
5. ✅ Displays before/after values

## Troubleshooting

### Option bytes didn't change
- Power cycle the chip completely
- Some changes require a reset to take effect
- Check OpenOCD output for errors

### Chip won't boot after changing option bytes
1. Check boot configuration (nBOOT_SEL, nBOOT0, nBOOT1)
2. If nBOOT_SEL=0, check BOOT0 pin state
3. Try mass erase: `./ica2506_flash --erase`

### Can't connect after changing option bytes
- If RDP was changed to Level 1, you'll need to mass erase first
- This will erase all flash but restore debug access
- **If RDP=0xCC was set, chip is permanently locked!**

## Important Warnings

⚠️ **NEVER SET RDP TO 0xCC** - This permanently locks the chip!

⚠️ Be careful with boot configuration - wrong settings can make chip difficult to reprogram

⚠️ Always backup current option bytes before changing

⚠️ Test configuration on one chip before programming production devices

## Reference

STM32G030 Option Bytes Address: `0x1FFF7800`

For detailed information, see:
- STM32G030 Reference Manual (RM0454)
- Section on Flash and Option Bytes

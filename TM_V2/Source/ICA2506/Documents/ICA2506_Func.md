# STM32G030F6_Class - ICA2506 Flash & UART Control

This module provides a self-contained C++ class for controlling the STM32G030F6 microcontroller, including flash programming, verification, erasing, boot mode configuration, reset, and UART communication via OpenOCD from a Raspberry Pi.

## Requirements

- OpenOCD installed and in PATH
- SWD wiring from RPi to STM32G030F6 (3.3V logic)
- Common GND between RPi and target
- UART connection if using serial communication features

### Default GPIO mapping (RPi 40-pin header)

- SWCLK: Pin 23 (GPIO11)
- SWDIO: Pin 22 (GPIO25)
- NRST: Pin 18 (GPIO24) (not used by current OpenOCD build)

## Class Overview

The `STM32G030F6_Class` provides a unified interface for all STM32G030F6 operations. All functionality is self-contained within the class - no external dependencies on ICA2506_Flash_Micro or ICA2506_UART modules.

### Class Files

- `Stm32G030Class.h` - Header with class definition
- `STM32G030Class.cpp` - Implementation with all helper functions

## Using the Class

### Basic Usage

```cpp
#include "Stm32G030Class.h"

int main() {
    // Create instance with default configuration
    STM32G030F6_Class stm32;
    
    // Flash firmware
    if (stm32.Flash("firmware.bin")) {
        printf("Flash programming successful\n");
    }
    
    // Verify flash content
    FlashCompareResult result = stm32.Verify("firmware.bin");
    if (result == FlashCompareResult::Same) {
        printf("Flash verification passed\n");
    }
    
    // Send UART command and get response
    std::string response;
    if (stm32.FrameProcess("49 50 A8", response)) {
        printf("UART Response: %s\n", response.c_str());
    }
    
    return 0;
}
```

### Custom Configuration

```cpp
// Create instance with custom settings
STM32G030F6_Class stm32(
    "interface/stlink.cfg",      // Interface config
    "target/stm32g0x.cfg",       // Target config
    0x08000000,                   // Flash base address
    1000,                         // Adapter speed (kHz)
    "/dev/ttyAMA1",              // UART port
    9600                          // UART baud rate
);
```

## Class Methods

### Flash Operations

#### `bool Flash(const std::string& binPath)`
Programs firmware binary to flash memory.
- **Parameter**: Path to .bin file
- **Returns**: true on success, false on failure

#### `bool Clear()`
Performs mass erase of flash memory.
- **Returns**: true on success, false on failure

#### `FlashCompareResult Verify(const std::string& binPath)`
Compares flash content with binary file.
- **Parameter**: Path to .bin file
- **Returns**: 
  - `FlashCompareResult::Same` - Flash matches binary
  - `FlashCompareResult::Empty` - Flash is erased (all 0xFF)
  - `FlashCompareResult::Different` - Flash contains different data
  - `FlashCompareResult::Error` - Verification error

#### `bool BootMode()`
Reads and displays option bytes (boot configuration).
- **Returns**: true on success, false on failure

#### `bool Reset()`
Resets the microcontroller.
- **Returns**: true on success, false on failure

### UART Operations

#### `bool UartTx(const std::string& hexStream)`
Transmits hex stream via UART and receives response into internal buffer.
- **Parameter**: Hex string (e.g., "49 50 A8")
- **Returns**: true on success, false on failure

#### `std::string UartRx() const`
Returns the last UART response received.
- **Returns**: Hex string of last response

#### `bool FrameProcess(const std::string& hexStream, std::string& responseHex)`
Sends UART frame and receives response.
- **Parameters**: 
  - `hexStream` - Hex string to send
  - `responseHex` - Output parameter for response
- **Returns**: true on success, false on failure

#### `bool FrameProcess(const std::string& hexStream, std::string& responseHex, int idleTimeoutMs)`
Sends UART frame with custom timeout.
- **Parameters**: 
  - `hexStream` - Hex string to send
  - `responseHex` - Output parameter for response
  - `idleTimeoutMs` - Idle timeout in milliseconds
- **Returns**: true on success, false on failure

## Build Instructions

### Build as Library

From TM_V2/Source/ICA2506:

```bash
g++ -std=c++17 -c STM32G030Class.cpp -o STM32G030Class.o
ar rcs libstm32g030.a STM32G030Class.o
```

### Build Command-Line Tool

```bash
g++ -std=c++17 main.cpp STM32G030Class.cpp -o ica2506_flash
```

### Link to Your Project

```bash
g++ -std=c++17 your_app.cpp STM32G030Class.cpp -o your_app
```

## Main Program Implementation

The `main.cpp` command-line tool is structured to automatically perform initialization tasks before processing user commands:

```cpp
int main(int argc, char* argv[]) {
    // Initialize with default configuration
    STM32G030F6_Class stm32("interface/raspberrypi2-native.cfg", "target/stm32g0x.cfg", 
                                0x08000000, 200, "/dev/ttyAMA1", 9600);
    
    // Always read option bytes (displays boot configuration)
    stm32.Flash_Func({"--ob"});
    
    // Always reset the MCU
    stm32.Flash_Func({"--reset"});
    
    // Process all command-line arguments (including program name)
    return stm32.Flash_Func(argc, argv);
}
```

This ensures:
- Boot configuration is displayed on every run
- MCU is reset after option byte read
- User commands are then processed

**Note**: The Flash_Func method internally skips argv[0] when processing arguments, so the command-line tool correctly handles user arguments starting from position 1.

To skip automatic initialization, modify main.cpp to call Flash_Func directly with your desired arguments.

## Command-Line Tool Usage

The included `main.cpp` provides a command-line interface to the class functionality.

**Note**: The command-line tool automatically reads option bytes (`--ob`) and resets the MCU (`--reset`) before processing any command-line arguments.

### Flash firmware

```bash
./ica2506_flash ./Firmware_Folder/STM32_ICA2410R1.1.bin
./ica2506_flash --write-ob 0xDEFFE1AA
./ica2506_flash --reset
./ica2506_flash --uart "55 56 52 50"
```

Custom interface/target/address/speed:

```bash
./ica2506_flash STM32G030F6.bin interface/raspberrypi2-native.cfg target/stm32g0x.cfg 0x08000000 200
```

### Compare flash with .bin

```bash
./ica2506_flash --cmp STM32G030F6.bin
```

Possible results:
- Flash is empty
- Flash matches firmware
- Flash programmed with different firmware

### Read option bytes

Option bytes are automatically read on every invocation. To only read option bytes:

```bash
./ica2506_flash
```

This prints raw option-byte words and a decoded OPTR summary, then resets the MCU.

### Mass erase

```bash
./ica2506_flash --erase
```

### Reset MCU (exit boot mode)

The MCU is automatically reset after reading option bytes. To explicitly request reset:

```bash
./ica2506_flash --reset
```

### UART hex stream

Send a hex byte stream over UART and print the response as hex bytes.

```bash
./ica2506_flash --uart "49 50 A8" /dev/ttyAMA1 9600 200
```

Arguments:
- port: UART device (default /dev/ttyAMA1)
- baud: baud rate (default 9600)
- idle_ms: stop reading when no data for this time (default 200)

## Complete Example

```cpp
#include "Stm32G030Class.h"
#include <iostream>

int main() {
    // Initialize with default configuration
    STM32G030F6_Class stm32;
    
    // 1. Erase flash
    std::cout << "Erasing flash..." << std::endl;
    if (!stm32.Clear()) {
        std::cerr << "Erase failed!" << std::endl;
        return 1;
    }
    
    // 2. Flash new firmware
    std::cout << "Programming flash..." << std::endl;
    if (!stm32.Flash("firmware.bin")) {
        std::cerr << "Flash programming failed!" << std::endl;
        return 1;
    }
    
    // 3. Verify flash
    std::cout << "Verifying flash..." << std::endl;
    FlashCompareResult result = stm32.Verify("firmware.bin");
    if (result != FlashCompareResult::Same) {
        std::cerr << "Verification failed!" << std::endl;
        return 1;
    }
    std::cout << "Flash verified successfully!" << std::endl;
    
    // 4. Read boot configuration
    std::cout << "Reading option bytes..." << std::endl;
    stm32.BootMode();
    
    // 5. Reset MCU
    std::cout << "Resetting MCU..." << std::endl;
    stm32.Reset();
    
    // 6. UART communication
    std::cout << "Testing UART..." << std::endl;
    std::string response;
    if (stm32.FrameProcess("49 50 A8", response)) {
        std::cout << "UART Response: " << response << std::endl;
        
        // Convert hex to ASCII if needed
        std::cout << "ASCII: " << stm32.UartRx() << std::endl;
    }
    
    return 0;
}
```

## Default Configuration

When using the default constructor, the following settings are used:

- **Interface**: `interface/raspberrypi2-native.cfg`
- **Target**: `target/stm32g0x.cfg`
- **Flash Address**: `0x08000000`
- **Adapter Speed**: `200 kHz`
- **UART Port**: `/dev/ttyAMA1`
- **UART Baud**: `9600`
- **UART Timeout**: `200 ms`

## Notes

- The OpenOCD command uses SWD via bcm2835 GPIO bit-bang driver
- If flashing fails, reduce the adapter speed (example: 50 or 25 kHz) by using the custom constructor
- Some OpenOCD builds do not support NRST control with bcm2835gpio
- All helper functions are private to the class - implementation is fully self-contained
- The class automatically handles temporary files during verification
- UART idle timeout determines when to stop waiting for response data

## Troubleshooting

### Flash Programming Fails
- Check SWD connections (SWCLK, SWDIO, GND)
- Reduce adapter speed to 50 kHz or 25 kHz
- Verify target has power (3.3V)
- Check OpenOCD is installed: `openocd --version`

### UART Communication Fails
- Verify UART connections (TX, RX, GND)
- Check baud rate matches target device
- Ensure serial port permissions: `sudo usermod -a -G dialout $USER`
- Test port with: `ls -l /dev/ttyAMA1`

### Verification Returns Different
- Flash was partially programmed
- Wrong binary file used
- Try erasing first, then flash again

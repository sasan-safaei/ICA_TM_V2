# ICA2506_Flash_Micro usage

This module provides simple helpers to flash, erase, reset, read option bytes, and compare flash contents on STM32G030F6 using OpenOCD from a Raspberry Pi.

## Requirements

- OpenOCD installed and in PATH.
- SWD wiring from RPi to STM32G030F6 (3.3V logic).
- Common GND between RPi and target.

### Default GPIO mapping (RPi 40-pin header)

- SWCLK: Pin 23 (GPIO11)
- SWDIO: Pin 22 (GPIO25)
- NRST: Pin 18 (GPIO24) (not used by current OpenOCD build)

## Build test tool

From TM_V2/Source/ICA2506:

```bash
g++ -std=c++17 main.cpp ICA2506_Flash_Micro.cpp -o ica2506_flash
```

## Flash firmware

```bash
./ica2506_flash STM32G030F6.bin
```

Custom interface/target/address/speed:

```bash
./ica2506_flash STM32G030F6.bin interface/raspberrypi2-native.cfg target/stm32g0x.cfg 0x08000000 200
```

## Compare flash with .bin

```bash
./ica2506_flash --cmp STM32G030F6.bin
```

Possible results:

- Flash is empty
- Flash matches firmware
- Flash programmed with different firmware

## Read option bytes

```bash
./ica2506_flash --ob
```

This prints raw option-byte words and a decoded OPTR summary.

## Mass erase

```bash
./ica2506_flash --erase
```

## Reset MCU (exit boot mode)

```bash
./ica2506_flash --reset
```

## UART hex stream

Send a hex byte stream over UART and print the response as hex bytes.

```bash
./ica2506_flash --uart "49 50 A8" /dev/ttyAMA1 9600 200
```

Arguments:

- port: UART device (default /dev/ttyAMA1)
- baud: baud rate (default 9600)
- idle_ms: stop reading when no data for this time (default 200)

## Notes

- The OpenOCD command uses SWD via bcm2835 GPIO bit-bang driver.
- If flashing fails, reduce the adapter speed (example: 50 or 25 kHz).
- Some OpenOCD builds do not support NRST control with bcm2835gpio.

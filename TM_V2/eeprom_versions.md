# EEPROM Save Versions

This document summarizes the EEPROM data layout for the three save versions  and CRC-calculation function.

## Version 1

| Offset | Field | Description | Format / Notes |
| ---: | --- | --- | --- |
| 0 | Identifier | Version marker (checked == 0x20) | 0x20 indicates ver1 |
| 1 | Year (BCD) | Year stored as BCD (two digits) | BCD; used as `1900 + 100 + bcd` in code (i.e. 2000+year) |
| 2 | Month (BCD) | Month (1-12) | BCD |
| 3 | Day (BCD) | Day of month | BCD |
| 4 | Hour (BCD) | Hour of day | BCD |
| 5 | Minute (BCD) | Minute | BCD |
| 6-7 | (unused) | Not used  | - |
| 8 -9 | Board type | (ICA2405)-> 0x24,0x05  (2315)-> 0x23,0x15 | Hex
| 10 | Board version | Hardware version | raw byte; also checked in detection ((&0xF0)==0xA0) |
| 11 | RSNSI1 | RSNSI1 value | raw byte |
| 12 | RSNSI2 | RSNSI2 value | raw byte |
| 13 | RSNSC | RSNSC value | raw byte |
| 14 | RTST | RTST value | raw byte |
| 15 | RT | RT value | raw byte |
| 16 | VshutDown | Vcap switch off threshold | Stored as value*10 (divide by 10 to get float) |
| 17 | supperCapVoltage | Supercap voltage | Stored as value*10 (divide by 10) |
| 18 | capacitorValue | Single cap value | raw byte |
| 19 | CapNo | Number of supercap | raw byte |

Notes: Version 1 does not use the "ICA" header and does not have an explicit CRC handling in the code paths shown.

---

## Version 2

| Offset | Field | Description | Format / Notes |
| ---: | --- | --- | --- |
| 0-2 | Name | ASCII header, expected "ICA" | bytes: 'I','C','A' |
| 3 | Length | Buffer length  |  |
| 4 | Data version | Version number (2) | raw byte |
| 5 -6 | Board type | (ICA2405)-> 0x24,0x05 | Hex
| 7 | Board version | Hardware version | raw byte |
| 8-11 | Timestamp area / reserved | Part of reserved/testTime area (depending on arch) | see note below |
| 12-15 | Test time (32-bit) | `time_t` low-to-high for 32-bit builds | bytes [12]=LSB ... [15]=MSB (assembled as 32-bit) |
| 16 | RSNSI1 | RSNSI1 | raw byte |
| 17 | RSNSI2 | RSNSI2 | raw byte |
| 18 | RSNSC | RSNSC | raw byte |
| 19 | RTST | RTST | raw byte |
| 20 | RT | RT | raw byte |
| 21 | VshutDown | Vcap switch off threshold | stored as value*10 (divide by 10) |
| 22 | supperCapVoltage | Supercap voltage | stored as value*10 (divide by 10) |
| 23 | capacitorValue | Single cap value | raw byte |
| 24 | CapNo | Number of supercap | raw byte |
| 25-26 | CRC16 | 2-byte CRC (big-endian) | CRC stored as high-byte then low-byte; writer computes CRC over first 25 bytes and stores at [25],[26] |


---

## Version 3 (Last)

| Offset | Field | Description | Format / Notes |
| ---: | --- | --- | --- |
| 0-2 | Name | ASCII header "ICA" | 'I','C','A' |
| 3 | Length | Buffer length (set to 27) | writer sets this to 27 |
| 4 | Data version | Version number (3) | raw byte |
| 5 | Board type high | High byte of board type | combined with [6] |
| 6 | Board type low | Low byte of board type | combined to uint16_t |
| 7 | Board version | Hardware version | raw byte |
| 8-11 | Timestamp area / reserved | writer reserves space for 64-bit or 32-bit time | depends on `time_sizeX64` macro |
| 12-15 | Test time (32-bit) | `time_t` assembled as 32-bit when not `time_sizeX64` | same layout as ver2 for 32-bit builds |
| 16 | RSNSI1 | RSNSI1 | raw byte |
| 17 | RSNSI2 | RSNSI2 | raw byte |
| 18 | RSNSC | RSNSC | raw byte |
| 19 | RTST | RTST | raw byte |
| 20 | RT | RT | raw byte |
| 21 | VshutDown | Vcap switch off threshold | stored as value*10 (divide by 10) |
| 22 | supperCapVoltage | Supercap voltage | stored as value*10 (divide by 10) |
| 23 | capacitorValue | Single cap value | raw byte |
| 24 | CapNo | Number of supercap | raw byte |
| 25 | CapType | Supercap type | raw byte (new in v3) |
| 26 | Temp85MaxV | Max voltage at 85°C | raw byte (new in v3) |
| 27-28 | CRC16 | 2-byte CRC (big-endian) | The writer computes CRC over bytes 0..26 and stores hi/lo at [27],[28]. The reader validates by computing CRC over full 29 bytes and expecting 0. |

Notes: Version 3 is considered the "last" version in `eeprom.cpp`. 

---

## Additional Notes

- **Version 1 status:** Version 1 is legacy and has not been used at all.

- **Differences between Ver2 and Ver3 (excluding the version byte and header/length):**
	- Version 3 adds two data fields not present in Version 2: `CapType` and `Temp85MaxV`, allowing storage of supercap type and the maximum voltage at 85°C.
	- To accommodate these new fields, Version 3 expands the payload and places the CRC after the larger payload. 
	



## CRC Function

- **Algorithm:** Computes a 16-bit CRC using the initial `CRC init value: 0xFFFF` and `CRC_POLYNOMIAL:0x1021`. For each input byte the implementation XORs the byte into the high byte of the running CRC, then performs 8 rounds of: if MSB set, left-shift and XOR the polynomial; otherwise just left-shift.

```cpp
#define CRC_POLYNOMIAL 0x1021  // Polynomial used for CRC-16-CCITT
#define CRC_INITIAL    0xFFFF  // Initial value
uint16_t eeprom::calculate_crc16(const uint8_t *data, size_t length) {
	uint16_t crc = CRC_INITIAL;
	for (size_t i = 0; i < length; i++) {
		crc ^= (data[i] << 8); // XOR the byte into the high byte of the CRC

		for (uint8_t bit = 0; bit < 8; bit++) {
			if (crc & 0x8000) {
				crc = (crc << 1) ^ CRC_POLYNOMIAL;
			} else {
				crc = (crc << 1);
			}
		}
	}
	return crc;
}
```

## Author / Location
Generated from `TM_V2/Source/TestFunction/eeprom.cpp` on analysis.


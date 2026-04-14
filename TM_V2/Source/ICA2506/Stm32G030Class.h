#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <termios.h>

/**
 * Example Usage:
 * 
 * // Create instance with default configuration
 * STM32G030F6_Class stm32;
 * 
 * // Flash a firmware binary
 * if (stm32.Flash("firmware.bin")) {
 *     printf("Flash successful\n");
 * }
 * 
 * // Verify flash content
 * FlashCompareResult result = stm32.Verify("firmware.bin");
 * if (result == FlashCompareResult::Same) {
 *     printf("Flash content matches\n");
 * }
 * 
 * // Erase flash
 * stm32.Clear();
 * 
 * // Reset microcontroller
 * stm32.Reset();
 * 
 * // Send UART frame and get response
 * std::string response;
 * if (stm32.FrameProcess("49 50 A8", response)) {
 *     printf("Response: %s\n", response.c_str());
 * }
 * 
 * // Read option bytes (boot configuration)
 * stm32.BootMode();
 */

enum class FlashCompareResult {
	Error = -1,
	Empty = 0,
	Same = 1,
	Different = 2
};

class STM32G030F6_Class {
public:
	int Flash_Func(int argc, char* argv[]);
	int Flash_Func(std::initializer_list<const char*> args);
	// Constructor with default configuration
	STM32G030F6_Class();
	
	// Constructor with custom configuration
	STM32G030F6_Class(const std::string& interfaceCfg,
					  const std::string& targetCfg,
					  uint32_t flashAddress,
					  int adapterKhz,
					  const std::string& uartPort,
					  int uartBaud);
private:
	// Flash programming
	bool Flash(const std::string& binPath);
	
	// Erase flash memory
	bool Clear();
	
	// Verify/compare flash content
	FlashCompareResult Verify(const std::string& binPath);
	
	// Read and display option bytes (boot mode configuration)
	bool BootMode();
	
	// Read option bytes and return OPTR value
	bool ReadOptionBytes(uint32_t& optrValue);
	
	// Program option bytes with new OPTR value
	bool WriteOptionBytes(uint32_t optrValue);
	
	// Build OPTR value from individual bit parameters
	static uint32_t BuildOPTR(uint8_t rdp, uint8_t bor_lev, bool nrst_stop, bool nrst_stdby,
							  bool nrst_shdw, bool iwdg_sw, bool iwdg_stop, bool iwdg_stdby,
							  bool wwdg_sw, bool sram_parity, bool nboot0, bool nboot1,
							  bool nboot_sel, bool boot_lock);
	
	// Reset the microcontroller
	bool Reset();
	
	// Test SWD interface connection
	bool TestInterface();
	
	// UART transmit
	bool UartTx(const std::string& hexStream);
	
	// UART receive (get last response)
	std::string UartRx() const;
	
	// UART transmit and receive
	bool FrameProcess(const std::string& hexStream, std::string& responseHex);
	
	// UART transmit and receive with custom timeout
	bool FrameProcess(const std::string& hexStream, std::string& responseHex, int idleTimeoutMs);

private:
	// Constants
	static constexpr int kRpiSwdioGpio = 25;
	static constexpr int kRpiSwclkGpio = 11;
	static constexpr int kRpiNresetGpio = 24;
	static constexpr int kRpiWorkAreaSize = 0x400;
	static constexpr int kRpiWorkAreaBase = 0x20000000;
	static constexpr uint32_t kOptionBytesAddr = 0x1FFF7800;
	static constexpr int kOptionBytesWords = 16;

	// Flash/SWD configuration
	std::string interfaceCfg_;
	std::string targetCfg_;
	uint32_t flashAddress_;
	int adapterKhz_;
	
	// UART configuration
	std::string uartPort_;
	int uartBaud_;
	int uartTimeoutMs_;
	
	// Last UART response
	std::string lastUartResponse_;

	// Private helper methods - Flash/SWD
	bool FileExists(const std::string& path);
	bool FileSize(const std::string& path, size_t& size);
	std::string ToAbsolutePath(const std::string& path);
	bool CommandAvailable(const std::string& command);
	bool IsRaspberryPiInterface(const std::string& interfaceCfg);
	void AppendInterfaceConfig(std::ostringstream& cmd, const std::string& interfaceCfg);
	int RunCommandCapture(const std::string& cmd, std::string& output);
	bool ParseMdwLine(const std::string& line, uint32_t& addr, std::vector<uint32_t>& values);
	void PrintOptionBytesDecode(const std::vector<std::pair<uint32_t, uint32_t>>& words);
	bool DumpFlashImage(const std::string& outPath, uint32_t address, size_t size, std::string& output);
	FlashCompareResult CompareBinToDump(const std::string& binPath, const std::string& dumpPath);

	// Private helper methods - UART
	int HexValue(char ch);
	bool ParseHexStream(const std::string& hexStream, std::vector<uint8_t>& bytes);
	bool GetBaudConstant(int baud, speed_t& out);
	bool ConfigurePort(int fd, int baud);
	std::string BytesToHex(const std::vector<uint8_t>& bytes);
};

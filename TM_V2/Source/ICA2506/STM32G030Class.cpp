#include "Stm32G030Class.h"

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <limits.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>





// Constructor with default configuration
STM32G030F6_Class::STM32G030F6_Class()
	: interfaceCfg_("interface/raspberrypi2-native.cfg"),
	  targetCfg_("target/stm32g0x.cfg"),
	  //interfaceCfg_("./OpenOCD_CFG/raspberrypi2-native.cfg"),//("interface/raspberrypi2-native.cfg"),
	  //targetCfg_("./OpenOCD_CFG/stm32g0x.cfg"),//("target/stm32g0x.cfg"),	   
	  flashAddress_(0x08000000),
	  adapterKhz_(200),
	  uartPort_("/dev/ttyAMA1"),
	  uartBaud_(9600),
	  uartTimeoutMs_(200),
	  lastUartResponse_("") {
}

// Constructor with custom configuration
STM32G030F6_Class::STM32G030F6_Class(const std::string& interfaceCfg,
									 const std::string& targetCfg,
									 uint32_t flashAddress,
									 int adapterKhz,
									 const std::string& uartPort,
									 int uartBaud)
	: interfaceCfg_(interfaceCfg),
	  targetCfg_(targetCfg),
	  flashAddress_(flashAddress),
	  adapterKhz_(adapterKhz),
	  uartPort_(uartPort),
	  uartBaud_(uartBaud),
	  uartTimeoutMs_(200),
	  lastUartResponse_("") {
}

// Flash programming
bool STM32G030F6_Class::Flash(const std::string& binPath) {
	if (!FileExists(binPath)) {
		fprintf(stderr, "Flash: file not found: %s\n", binPath.c_str());
		return false;
	}
	if (!CommandAvailable("openocd")) {
		fprintf(stderr, "Flash: openocd not found in PATH\n");
		return false;
	}

	const std::string absBinPath = ToAbsolutePath(binPath);

	// Because the board uses a MOSFET that inverts the reset signal (gate on GPIO 26,
	// source to target reset on GPIO 24), assert the gate here and keep it asserted
	// for the whole OpenOCD operation so the target is held in reset while we connect
	// (connect-under-reset). Release the gate after OpenOCD finishes. This requires
	// running as root so sysfs GPIO is writable.
	std::string gpioPath = std::string("/sys/class/gpio/gpio") + std::to_string(kRpiNresetGpio);
	std::system((std::string("echo ") + std::to_string(kRpiNresetGpio) + " > /sys/class/gpio/export 2>/dev/null || true").c_str());
	std::system((std::string("echo out > ") + gpioPath + "/direction 2>/dev/null || true").c_str());
	// Assert gate (drive high) to hold reset (MOSFET inverts this)
	std::system((std::string("echo 0 > ") + gpioPath + "/value 2>/dev/null || true").c_str());

	std::ostringstream cmd;
	cmd << "openocd";
	AppendInterfaceConfig(cmd, interfaceCfg_);
	cmd << " -c \"set WORKAREASIZE 0x" << std::hex << kRpiWorkAreaSize << "\""
		<< " -f " << targetCfg_
		<< " -c \"adapter speed " << std::dec << adapterKhz_ << "\""
		<< " -c \"stm32g0x.cpu configure -work-area-phys 0x" << std::hex << kRpiWorkAreaBase
		<< " -work-area-size 0x" << std::hex << kRpiWorkAreaSize << " -work-area-backup 0\""
        
		<< " -c \"init\""
		<< " -c \"reset halt\""
		<< " -c \"flash write_image erase " << absBinPath << " 0x" << std::hex << flashAddress_ << " bin\""
		<< " -c \"verify_image " << absBinPath << " 0x" << std::hex << flashAddress_ << " bin\""
		<< " -c \"reset run\""
		<< " -c \"shutdown\"";

	const int result = std::system(cmd.str().c_str());

	// Release reset gate after OpenOCD finishes
	std::system((std::string("echo 0 > ") + gpioPath + "/value 2>/dev/null || true").c_str());

	if (result != 0) {
		fprintf(stderr, "Flash: openocd failed (%d)\n", result);
		return false;
	}

	return true;
}

// Erase flash memory
bool STM32G030F6_Class::Clear() {
	if (!CommandAvailable("openocd")) {
		fprintf(stderr, "Clear: openocd not found in PATH\n");
		return false;
	}

	std::ostringstream cmd;
	cmd << "openocd";
	AppendInterfaceConfig(cmd, interfaceCfg_);
	cmd << " -c \"set WORKAREASIZE 0x" << std::hex << kRpiWorkAreaSize << "\""
		<< " -f " << targetCfg_
		<< " -c \"adapter speed " << std::dec << adapterKhz_ << "\""
		<< " -c \"init\""
		<< " -c \"reset halt\""
		<< " -c \"stm32g0x mass_erase 0\""
		<< " -c \"reset run\""
		<< " -c \"shutdown\"";

	const int result = std::system(cmd.str().c_str());
	if (result != 0) {
		fprintf(stderr, "Clear: openocd failed (%d)\n", result);
		return false;
	}

	return true;
}

// Verify/compare flash content
FlashCompareResult STM32G030F6_Class::Verify(const std::string& binPath) {
	if (!FileExists(binPath)) {
		fprintf(stderr, "Verify: file not found: %s\n", binPath.c_str());
		return FlashCompareResult::Error;
	}
	if (!CommandAvailable("openocd")) {
		fprintf(stderr, "Verify: openocd not found in PATH\n");
		return FlashCompareResult::Error;
	}

	size_t binSize = 0;
	if (!FileSize(binPath, binSize) || binSize == 0) {
		fprintf(stderr, "Verify: invalid file size\n");
		return FlashCompareResult::Error;
	}

	char tmpTemplate[] = "/tmp/ica2506_dump_XXXXXX";
	const int fd = mkstemp(tmpTemplate);
	if (fd < 0) {
		fprintf(stderr, "Verify: failed to create temp file\n");
		return FlashCompareResult::Error;
	}
	close(fd);
	const std::string dumpPath = tmpTemplate;

	std::string output;
	const bool ok = DumpFlashImage(dumpPath, flashAddress_, binSize, output);
	if (!ok) {
		fprintf(stderr, "Verify: openocd dump failed\n");
		fprintf(stderr, "%s\n", output.c_str());
		std::remove(dumpPath.c_str());
		return FlashCompareResult::Error;
	}

	FlashCompareResult result = CompareBinToDump(binPath, dumpPath);
	std::remove(dumpPath.c_str());
	return result;
}

// Read and display option bytes (boot mode configuration)
bool STM32G030F6_Class::BootMode() {
	if (!CommandAvailable("openocd")) {
		fprintf(stderr, "BootMode: openocd not found in PATH\n");
		return false;
	}

	std::ostringstream cmd;
	cmd << "openocd";
	AppendInterfaceConfig(cmd, interfaceCfg_);
	cmd << " -c \"set WORKAREASIZE 0x" << std::hex << kRpiWorkAreaSize << "\""
		<< " -f " << targetCfg_
		<< " -c \"adapter speed " << std::dec << adapterKhz_ << "\""
		<< " -c \"init\""
		<< " -c \"reset halt\""
		<< " -c \"mdw 0x" << std::hex << kOptionBytesAddr << " " << std::dec << kOptionBytesWords << "\""
		<< " -c \"shutdown\"";

	std::string output;
	const int status = RunCommandCapture(cmd.str() + " 2>&1", output);

	std::vector<std::pair<uint32_t, uint32_t>> words;
	std::istringstream iss(output);
	std::string line;
	while (std::getline(iss, line)) {
		uint32_t addr = 0;
		std::vector<uint32_t> values;
		if (!ParseMdwLine(line, addr, values)) {
			continue;
		}
		if (addr < kOptionBytesAddr ||
			addr >= kOptionBytesAddr + static_cast<uint32_t>(kOptionBytesWords * 4)) {
			continue;
		}
		for (size_t i = 0; i < values.size(); ++i) {
			const uint32_t word_addr = addr + static_cast<uint32_t>(i * 4);
			words.emplace_back(word_addr, values[i]);
		}
	}

	if (words.empty()) {
		printf("No option bytes found in OpenOCD output.\n");
		printf("Raw OpenOCD output:\n%s\n", output.c_str());
		return false;
	}

	if (status != 0) {
		fprintf(stderr, "BootMode: openocd exited with status %d\n", status);
	}

	PrintOptionBytesDecode(words);

	return true;
}

// Read option bytes and return OPTR value
bool STM32G030F6_Class::ReadOptionBytes(uint32_t& optrValue) {
	if (!CommandAvailable("openocd")) {
		fprintf(stderr, "ReadOptionBytes: openocd not found in PATH\n");
		return false;
	}

	std::ostringstream cmd;
	cmd << "openocd";
	AppendInterfaceConfig(cmd, interfaceCfg_);
	cmd << " -c \"set WORKAREASIZE 0x" << std::hex << kRpiWorkAreaSize << "\""
		<< " -f " << targetCfg_
		<< " -c \"adapter speed " << std::dec << adapterKhz_ << "\""
		<< " -c \"init\""
		<< " -c \"reset halt\""
		<< " -c \"mdw 0x" << std::hex << kOptionBytesAddr << " 1\""
		<< " -c \"shutdown\"";

	std::string output;
	const int status = RunCommandCapture(cmd.str() + " 2>&1", output);

	// Parse the output to extract OPTR value
	std::istringstream iss(output);
	std::string line;
	while (std::getline(iss, line)) {
		uint32_t addr = 0;
		std::vector<uint32_t> values;
		if (ParseMdwLine(line, addr, values) && !values.empty()) {
			if (addr == kOptionBytesAddr) {
				optrValue = values[0];
				return true;
			}
		}
	}

	fprintf(stderr, "ReadOptionBytes: failed to read OPTR\n");
	return false;
}

// Program option bytes with new OPTR value
bool STM32G030F6_Class::WriteOptionBytes(uint32_t optrValue) {
	if (!CommandAvailable("openocd")) {
		fprintf(stderr, "WriteOptionBytes: openocd not found in PATH\n");
		return false;
	}

	// Read current value first for safety
	uint32_t currentOptr = 0;
	if (!ReadOptionBytes(currentOptr)) {
		fprintf(stderr, "WriteOptionBytes: failed to read current option bytes\n");
		return false;
	}

	printf("Current OPTR: 0x%08X\n", currentOptr);
	printf("New OPTR:     0x%08X\n", optrValue);

	// Safety check: Warn if RDP is being changed
	const uint8_t currentRdp = static_cast<uint8_t>(currentOptr & 0xFF);
	const uint8_t newRdp = static_cast<uint8_t>(optrValue & 0xFF);
	if (currentRdp != newRdp) {
		if (newRdp == 0xCC) {
			fprintf(stderr, "WriteOptionBytes: DANGEROUS! RDP Level 2 (0xCC) is PERMANENT!\n");
			fprintf(stderr, "WriteOptionBytes: This will PERMANENTLY LOCK the chip!\n");
			fprintf(stderr, "WriteOptionBytes: Refusing to proceed.\n");
			return false;
		}
		printf("WARNING: RDP level changing from 0x%02X to 0x%02X\n", currentRdp, newRdp);
	}

	// Program option bytes using OpenOCD stm32g0x option_write command
	std::ostringstream cmd;
	cmd << "openocd";
	AppendInterfaceConfig(cmd, interfaceCfg_);
	cmd << " -c \"set WORKAREASIZE 0x" << std::hex << kRpiWorkAreaSize << "\""
		<< " -f " << targetCfg_
		<< " -c \"adapter speed " << std::dec << adapterKhz_ << "\""
		<< " -c \"init\""
		<< " -c \"reset halt\""
		<< " -c \"sleep 50\""
		// Use OpenOCD's built-in command to write option bytes
		// Format: stm32g0x option_write bank_id offset value
		<< " -c \"stm32g0x option_write 0 0x20 0x" << std::hex << optrValue << "\""
		<< " -c \"sleep 200\""
		// Reload option bytes
		<< " -c \"stm32g0x option_load 0\""
		<< " -c \"sleep 200\""
		// Reset and verify
		<< " -c \"reset halt\""
		<< " -c \"sleep 50\""
		<< " -c \"mdw 0x40022020\""
		<< " -c \"reset run\""
		<< " -c \"shutdown\"";

	printf("Programming option bytes...\n");
	const int result = std::system(cmd.str().c_str());
	if (result != 0) {
		fprintf(stderr, "WriteOptionBytes: openocd failed (%d)\n", result);
		return false;
	}

	// Give the chip time to reset and reload option bytes
	sleep(1);

	// Verify the write
	uint32_t verifyOptr = 0;
	if (ReadOptionBytes(verifyOptr)) {
		if (verifyOptr == optrValue) {
			printf("Option bytes programmed successfully!\n");
			return true;
		} else {
			fprintf(stderr, "WriteOptionBytes: verification failed (read 0x%08X, expected 0x%08X)\n",
					verifyOptr, optrValue);
			return false;
		}
	}

	return true;
}

// Helper: Build OPTR value from individual bits
uint32_t STM32G030F6_Class::BuildOPTR(uint8_t rdp, uint8_t bor_lev, bool nrst_stop, bool nrst_stdby,
                                       bool nrst_shdw, bool iwdg_sw, bool iwdg_stop, bool iwdg_stdby,
                                       bool wwdg_sw, bool sram_parity, bool nboot0, bool nboot1,
                                       bool nboot_sel, bool boot_lock) {
	uint32_t optr = 0;
	optr |= static_cast<uint32_t>(rdp);
	optr |= static_cast<uint32_t>(bor_lev & 0x7) << 8;
	if (nrst_stop)   optr |= (1U << 12);
	if (nrst_stdby)  optr |= (1U << 13);
	if (nrst_shdw)   optr |= (1U << 14);
	if (iwdg_sw)     optr |= (1U << 16);
	if (iwdg_stop)   optr |= (1U << 17);
	if (iwdg_stdby)  optr |= (1U << 18);
	if (wwdg_sw)     optr |= (1U << 19);
	if (sram_parity) optr |= (1U << 21);
	if (nboot0)      optr |= (1U << 24);
	if (nboot1)      optr |= (1U << 25);
	if (nboot_sel)   optr |= (1U << 26);
	if (boot_lock)   optr |= (1U << 27);
	return optr;
}

// Reset the microcontroller
bool STM32G030F6_Class::Reset() {
	if (!CommandAvailable("openocd")) {
		fprintf(stderr, "Reset: openocd not found in PATH\n");
		return false;
	}

	// If using the Raspberry Pi bcm2835gpio interface and the NRST line
	// is driven through an inverting MOSFET, pulse the gate GPIO here
	// so the target leaves an unexpected held-reset state before OpenOCD
	// runs. This uses sysfs and requires the program to run with sudo.
	if (IsRaspberryPiInterface(interfaceCfg_)) {
		std::system((std::string("echo ") + std::to_string(kRpiNresetGpio) + " > /sys/class/gpio/export 2>/dev/null || true").c_str());
		std::system((std::string("echo out > /sys/class/gpio/gpio") + std::to_string(kRpiNresetGpio) + "/direction 2>/dev/null || true").c_str());
		std::system((std::string("echo 1 > /sys/class/gpio/gpio") + std::to_string(kRpiNresetGpio) + "/value 2>/dev/null || true").c_str());
		usleep(100000);
		std::system((std::string("echo 0 > /sys/class/gpio/gpio") + std::to_string(kRpiNresetGpio) + "/value 2>/dev/null || true").c_str());
		usleep(50000);
	}

	std::ostringstream cmd;
	cmd << "openocd";
	AppendInterfaceConfig(cmd, interfaceCfg_);
	cmd << " -c \"set WORKAREASIZE 0x" << std::hex << kRpiWorkAreaSize << "\""
		<< " -f " << targetCfg_
		<< " -c \"adapter speed " << std::dec << adapterKhz_ << "\""
		<< " -c \"init\""
		<< " -c \"reset run\""
		<< " -c \"shutdown\"";

	const int result = std::system(cmd.str().c_str());
	if (result != 0) {
		fprintf(stderr, "Reset: openocd failed (%d)\n", result);
		return false;
	}

	return true;
}

// Test SWD interface connection
bool STM32G030F6_Class::TestInterface() {
	if (!CommandAvailable("openocd")) {
		fprintf(stderr, "TestInterface: openocd not found in PATH\n");
		return false;
	}

	printf("==========================================\n");
	printf("SWD Interface Connection Test\n");
	printf("==========================================\n");
	printf("Configuration:\n");
	printf("  Interface: %s\n", interfaceCfg_.c_str());
	printf("  Target:    %s\n", targetCfg_.c_str());
	printf("  Speed:     %d kHz\n", adapterKhz_);
	printf("  GPIO pins: SWCLK=%d, SWDIO=%d\n", kRpiSwclkGpio, kRpiSwdioGpio);
	printf("==========================================\n\n");

	// Build OpenOCD command to test connection
	std::ostringstream cmd;
	cmd << "openocd";
	AppendInterfaceConfig(cmd, interfaceCfg_);
	cmd << " -c \"set WORKAREASIZE 0x" << std::hex << kRpiWorkAreaSize << "\""
		<< " -f " << targetCfg_
		<< " -c \"adapter speed " << std::dec << adapterKhz_ << "\""
		<< " -c \"init\""
		<< " -c \"shutdown\" 2>&1";

	// Run command and capture output
	std::string output;
	printf("Testing SWD connection...\n\n");
	const int status = RunCommandCapture(cmd.str(), output);

	// Display raw output
	printf("OpenOCD Output:\n");
	printf("----------------------------------------\n");
	printf("%s\n", output.c_str());
	printf("----------------------------------------\n\n");

	// Parse output for key indicators
	bool hasDeadbeef = (output.find("0xdeadbeef") != std::string::npos);
	bool hasValidDpidr = (output.find("SWD DPIDR") != std::string::npos) && !hasDeadbeef;
	bool hasError = (output.find("Error:") != std::string::npos);
	
	// Extract DPIDR value if present
	std::string dpidrValue = "Not found";
	size_t dpidrPos = output.find("SWD DPIDR");
	if (dpidrPos != std::string::npos) {
		size_t valueStart = output.find("0x", dpidrPos);
		if (valueStart != std::string::npos) {
			size_t valueEnd = output.find_first_of(" \t\n\r", valueStart);
			if (valueEnd != std::string::npos) {
				dpidrValue = output.substr(valueStart, valueEnd - valueStart);
			}
		}
	}

	// Print diagnostic results
	printf("Diagnostic Results:\n");
	printf("----------------------------------------\n");
	printf("DPIDR Value:       %s\n", dpidrValue.c_str());
	
	if (hasDeadbeef) {
		printf("Connection Status: FAILED (0xdeadbeef)\n");
		printf("\nProbable Causes:\n");
		printf("  1. Loose or disconnected wires (SWDIO/GPIO %d or SWCLK/GPIO %d)\n", 
		       kRpiSwdioGpio, kRpiSwclkGpio);
		printf("  2. Target chip not powered (check VDD = 3.3V)\n");
		printf("  3. Ground not connected between debugger and target\n");
		printf("  4. Wrong GPIO pin configuration\n");
		printf("  5. Faulty target chip\n");
		printf("\nTroubleshooting Steps:\n");
		printf("  1. Check physical connections (SWDIO, SWCLK, GND, VDD)\n");
		printf("  2. Verify target power (measure 3.3V on VDD pin)\n");
		printf("  3. Try lower speed: use --testInterface interface target 10\n");
		printf("  4. Check for shorts or damaged wires\n");
		printf("  5. Verify GPIO pins are not used by other processes\n");
	} else if (hasValidDpidr) {
		printf("Connection Status: SUCCESS\n");
		printf("\nThe SWD interface is working correctly.\n");
		printf("You can proceed with flash operations.\n");
	} else if (hasError) {
		printf("Connection Status: ERROR\n");
		printf("\nOpenOCD reported errors. Check the output above.\n");
	} else {
		printf("Connection Status: UNKNOWN\n");
		printf("\nCould not determine connection status from output.\n");
	}
	printf("==========================================\n");

	return hasValidDpidr && !hasDeadbeef;
}

// UART transmit
bool STM32G030F6_Class::UartTx(const std::string& hexStream) {
	std::vector<uint8_t> tx;
	if (!ParseHexStream(hexStream, tx)) {
		fprintf(stderr, "UartTx: invalid hex stream\n");
		return false;
	}

	const int fd = open(uartPort_.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "UartTx: open failed: %s\n", strerror(errno));
		return false;
	}

	if (!ConfigurePort(fd, uartBaud_)) {
		fprintf(stderr, "UartTx: configure failed\n");
		close(fd);
		return false;
	}

	if (!tx.empty()) {
		const ssize_t written = write(fd, tx.data(), tx.size());
		if (written < 0 || static_cast<size_t>(written) != tx.size()) {
			fprintf(stderr, "UartTx: write failed\n");
			close(fd);
			return false;
		}
	}

	std::vector<uint8_t> rx;
	rx.reserve(1024);

	const auto start = std::chrono::steady_clock::now();
	auto last_data = start;

	while (true) {
		const auto now = std::chrono::steady_clock::now();
		const auto idle_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_data).count();
		if (!rx.empty() && idle_elapsed >= uartTimeoutMs_) {
			break;
		}

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 20000;
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);

		const int ready = select(fd + 1, &rfds, nullptr, nullptr, &tv);
		if (ready > 0 && FD_ISSET(fd, &rfds)) {
			uint8_t buf[256];
			const ssize_t n = read(fd, buf, sizeof(buf));
			if (n > 0) {
				rx.insert(rx.end(), buf, buf + n);
				last_data = std::chrono::steady_clock::now();
			}
		}
	}

	close(fd);
	lastUartResponse_ = BytesToHex(rx);
	return true;
}

// UART receive (get last response)
std::string STM32G030F6_Class::UartRx() const {
	return lastUartResponse_;
}

// UART transmit and receive
bool STM32G030F6_Class::FrameProcess(const std::string& hexStream, std::string& responseHex) {
	responseHex.clear();
	std::vector<uint8_t> tx;
	if (!ParseHexStream(hexStream, tx)) {
		fprintf(stderr, "FrameProcess: invalid hex stream\n");
		return false;
	}

	const int fd = open(uartPort_.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "FrameProcess: open failed: %s\n", strerror(errno));
		return false;
	}

	if (!ConfigurePort(fd, uartBaud_)) {
		fprintf(stderr, "FrameProcess: configure failed\n");
		close(fd);
		return false;
	}

	if (!tx.empty()) {
		const ssize_t written = write(fd, tx.data(), tx.size());
		if (written < 0 || static_cast<size_t>(written) != tx.size()) {
			fprintf(stderr, "FrameProcess: write failed\n");
			close(fd);
			return false;
		}
	}

	std::vector<uint8_t> rx;
	rx.reserve(1024);

	const auto start = std::chrono::steady_clock::now();
	auto last_data = start;

	while (true) {
		const auto now = std::chrono::steady_clock::now();
		const auto idle_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_data).count();
		if (!rx.empty() && idle_elapsed >= uartTimeoutMs_) {
			break;
		}

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 20000;
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);

		const int ready = select(fd + 1, &rfds, nullptr, nullptr, &tv);
		if (ready > 0 && FD_ISSET(fd, &rfds)) {
			uint8_t buf[256];
			const ssize_t n = read(fd, buf, sizeof(buf));
			if (n > 0) {
				rx.insert(rx.end(), buf, buf + n);
				last_data = std::chrono::steady_clock::now();
			}
		}
	}

	close(fd);
	responseHex = BytesToHex(rx);
	lastUartResponse_ = responseHex;
	return true;
}

// UART transmit and receive with custom timeout
bool STM32G030F6_Class::FrameProcess(const std::string& hexStream, std::string& responseHex, int idleTimeoutMs) {
	responseHex.clear();
	std::vector<uint8_t> tx;
	if (!ParseHexStream(hexStream, tx)) {
		fprintf(stderr, "FrameProcess: invalid hex stream\n");
		return false;
	}

	const int fd = open(uartPort_.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "FrameProcess: open failed: %s\n", strerror(errno));
		return false;
	}

	if (!ConfigurePort(fd, uartBaud_)) {
		fprintf(stderr, "FrameProcess: configure failed\n");
		close(fd);
		return false;
	}

	if (!tx.empty()) {
		const ssize_t written = write(fd, tx.data(), tx.size());
		if (written < 0 || static_cast<size_t>(written) != tx.size()) {
			fprintf(stderr, "FrameProcess: write failed\n");
			close(fd);
			return false;
		}
	}

	std::vector<uint8_t> rx;
	rx.reserve(1024);

	const auto start = std::chrono::steady_clock::now();
	auto last_data = start;

	while (true) {
		const auto now = std::chrono::steady_clock::now();
		const auto idle_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_data).count();
		if (!rx.empty() && idle_elapsed >= idleTimeoutMs) {
			break;
		}

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 20000;
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);

		const int ready = select(fd + 1, &rfds, nullptr, nullptr, &tv);
		if (ready > 0 && FD_ISSET(fd, &rfds)) {
			uint8_t buf[256];
			const ssize_t n = read(fd, buf, sizeof(buf));
			if (n > 0) {
				rx.insert(rx.end(), buf, buf + n);
				last_data = std::chrono::steady_clock::now();
			}
		}
	}

	close(fd);
	responseHex = BytesToHex(rx);
	lastUartResponse_ = responseHex;
	return true;
}

// ============================================================================
// Private Helper Methods - Flash/SWD
// ============================================================================

bool STM32G030F6_Class::FileExists(const std::string& path) {
struct stat st;
return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

bool STM32G030F6_Class::FileSize(const std::string& path, size_t& size) {
struct stat st;
if (stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) {
return false;
}
if (st.st_size < 0) {
return false;
}
size = static_cast<size_t>(st.st_size);
return true;
}

std::string STM32G030F6_Class::ToAbsolutePath(const std::string& path) {
char resolved[PATH_MAX];
if (realpath(path.c_str(), resolved)) {
return std::string(resolved);
}
return path;
}

bool STM32G030F6_Class::CommandAvailable(const std::string& command) {
std::ostringstream oss;
oss << "command -v " << command << " >/dev/null 2>&1";
return std::system(oss.str().c_str()) == 0;
}

bool STM32G030F6_Class::IsRaspberryPiInterface(const std::string& interfaceCfg) {
return interfaceCfg.find("raspberrypi") != std::string::npos;
}

void STM32G030F6_Class::AppendInterfaceConfig(std::ostringstream& cmd, const std::string& interfaceCfg) {
if (IsRaspberryPiInterface(interfaceCfg)) {
	cmd << " -c \"adapter driver bcm2835gpio\""
		<< " -c \"transport select swd\""
		<< " -c \"bcm2835gpio_swd_nums " << kRpiSwclkGpio << " " << kRpiSwdioGpio << "\"";
} else {
cmd << " -f " << interfaceCfg;
}
}

int STM32G030F6_Class::RunCommandCapture(const std::string& cmd, std::string& output) {
FILE* pipe = popen(cmd.c_str(), "r");
if (!pipe) {
return -1;
}
char buffer[512];
output.clear();
while (fgets(buffer, sizeof(buffer), pipe)) {
output.append(buffer);
}
return pclose(pipe);
}

bool STM32G030F6_Class::ParseMdwLine(const std::string& line, uint32_t& addr, std::vector<uint32_t>& values) {
unsigned int a = 0;
if (std::sscanf(line.c_str(), "0x%x:", &a) != 1) {
return false;
}
addr = static_cast<uint32_t>(a);
values.clear();

const char* p = std::strchr(line.c_str(), ':');
if (!p) {
return false;
}
++p;
while (*p != '\0') {
while (*p == ' ' || *p == '\t') {
++p;
}
if (*p == '\0') {
break;
}
unsigned int v = 0;
int consumed = 0;
if (std::sscanf(p, "0x%x%n", &v, &consumed) == 1) {
values.push_back(static_cast<uint32_t>(v));
p += consumed;
continue;
}
if (std::sscanf(p, "%x%n", &v, &consumed) == 1) {
values.push_back(static_cast<uint32_t>(v));
p += consumed;
continue;
}
break;
}

return !values.empty();
}

void STM32G030F6_Class::PrintOptionBytesDecode(const std::vector<std::pair<uint32_t, uint32_t>>& words) {
if (words.empty()) {
printf("Option bytes: no data\n");
return;
}

printf("Option bytes raw (0x%08X..):\n", kOptionBytesAddr);
for (const auto& entry : words) {
printf("  0x%08X: 0x%08X\n", entry.first, entry.second);
}

const uint32_t optr = words.front().second;
const uint8_t rdp = static_cast<uint8_t>(optr & 0xFFu);
const uint8_t bor = static_cast<uint8_t>((optr >> 8) & 0x7u);
const bool n_rst_stop = (optr >> 12) & 0x1u;
const bool n_rst_stdby = (optr >> 13) & 0x1u;
const bool n_rst_shdw = (optr >> 14) & 0x1u;
const bool iwdg_sw = (optr >> 16) & 0x1u;
const bool iwdg_stop = (optr >> 17) & 0x1u;
const bool iwdg_stdby = (optr >> 18) & 0x1u;
const bool wwdg_sw = (optr >> 19) & 0x1u;
const bool sram_parity = (optr >> 21) & 0x1u;
const bool n_boot0 = (optr >> 24) & 0x1u;
const bool n_boot1 = (optr >> 25) & 0x1u;
const bool n_boot_sel = (optr >> 26) & 0x1u;
const bool boot_lock = (optr >> 27) & 0x1u;

printf("Decoded OPTR (0x%08X):\n", optr);
printf("  RDP: 0x%02X\n", rdp);
printf("  BOR_LEV: %u\n", bor);
printf("  nRST_STOP: %u\n", n_rst_stop ? 1 : 0);
printf("  nRST_STDBY: %u\n", n_rst_stdby ? 1 : 0);
printf("  nRST_SHDW: %u\n", n_rst_shdw ? 1 : 0);
printf("  IWDG_SW: %u\n", iwdg_sw ? 1 : 0);
printf("  IWDG_STOP: %u\n", iwdg_stop ? 1 : 0);
printf("  IWDG_STDBY: %u\n", iwdg_stdby ? 1 : 0);
printf("  WWDG_SW: %u\n", wwdg_sw ? 1 : 0);
printf("  SRAM_PARITY: %u\n", sram_parity ? 1 : 0);
printf("  nBOOT0: %u\n", n_boot0 ? 1 : 0);
printf("  nBOOT1: %u\n", n_boot1 ? 1 : 0);
printf("  nBOOT_SEL: %u\n", n_boot_sel ? 1 : 0);
printf("  BOOT_LOCK: %u\n", boot_lock ? 1 : 0);
}

bool STM32G030F6_Class::DumpFlashImage(const std::string& outPath, uint32_t address, size_t size, std::string& output) {
std::ostringstream cmd;
cmd << "openocd";
AppendInterfaceConfig(cmd, interfaceCfg_);
cmd << " -c \"set WORKAREASIZE 0x" << std::hex << kRpiWorkAreaSize << "\""
<< " -f " << targetCfg_
<< " -c \"adapter speed " << std::dec << adapterKhz_ << "\""
<< " -c \"init\""
<< " -c \"reset halt\""
<< " -c \"dump_image " << outPath << " 0x" << std::hex << address
<< " 0x" << std::hex << size << "\""
<< " -c \"shutdown\"";

const int status = RunCommandCapture(cmd.str() + " 2>&1", output);
return status == 0;
}

FlashCompareResult STM32G030F6_Class::CompareBinToDump(const std::string& binPath, const std::string& dumpPath) {
std::ifstream binFile(binPath, std::ios::binary);
std::ifstream dumpFile(dumpPath, std::ios::binary);
if (!binFile.is_open() || !dumpFile.is_open()) {
return FlashCompareResult::Error;
}

const size_t kBufferSize = 4096;
std::vector<unsigned char> binBuf(kBufferSize);
std::vector<unsigned char> dumpBuf(kBufferSize);
bool all_ff = true;
bool all_same = true;

while (binFile && dumpFile) {
binFile.read(reinterpret_cast<char*>(binBuf.data()), kBufferSize);
dumpFile.read(reinterpret_cast<char*>(dumpBuf.data()), kBufferSize);
const std::streamsize binRead = binFile.gcount();
const std::streamsize dumpRead = dumpFile.gcount();
if (binRead != dumpRead) {
return FlashCompareResult::Error;
}
if (binRead <= 0) {
break;
}
for (std::streamsize i = 0; i < binRead; ++i) {
if (dumpBuf[static_cast<size_t>(i)] != 0xFFu) {
all_ff = false;
}
if (dumpBuf[static_cast<size_t>(i)] != binBuf[static_cast<size_t>(i)]) {
all_same = false;
}
}
if (!all_ff && !all_same) {
break;
}
}

if (all_ff) {
return FlashCompareResult::Empty;
}
if (all_same) {
return FlashCompareResult::Same;
}
return FlashCompareResult::Different;
}

// ============================================================================
// Private Helper Methods - UART
// ============================================================================

int STM32G030F6_Class::HexValue(char ch) {
if (ch >= '0' && ch <= '9') {
return ch - '0';
}
if (ch >= 'a' && ch <= 'f') {
return 10 + (ch - 'a');
}
if (ch >= 'A' && ch <= 'F') {
return 10 + (ch - 'A');
}
return -1;
}

bool STM32G030F6_Class::ParseHexStream(const std::string& hexStream, std::vector<uint8_t>& bytes) {
bytes.clear();
int high = -1;
for (size_t i = 0; i < hexStream.size(); ++i) {
const char ch = hexStream[i];
if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == ',') {
continue;
}
if (ch == '0' && (i + 1 < hexStream.size()) && (hexStream[i + 1] == 'x' || hexStream[i + 1] == 'X')) {
++i;
continue;
}
const int v = HexValue(ch);
if (v < 0) {
return false;
}
if (high < 0) {
high = v;
} else {
bytes.push_back(static_cast<uint8_t>((high << 4) | v));
high = -1;
}
}
return high < 0;
}

bool STM32G030F6_Class::GetBaudConstant(int baud, speed_t& out) {
switch (baud) {
case 9600: out = B9600; return true;
case 19200: out = B19200; return true;
case 38400: out = B38400; return true;
case 57600: out = B57600; return true;
case 115200: out = B115200; return true;
case 230400: out = B230400; return true;
case 460800: out = B460800; return true;
case 921600: out = B921600; return true;
default: return false;
}
}

bool STM32G030F6_Class::ConfigurePort(int fd, int baud) {
struct termios tty;
if (tcgetattr(fd, &tty) != 0) {
return false;
}

speed_t speed;
if (!GetBaudConstant(baud, speed)) {
return false;
}

cfsetispeed(&tty, speed);
cfsetospeed(&tty, speed);

tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
tty.c_cflag |= (CLOCAL | CREAD);
tty.c_cflag &= ~(PARENB | PARODD);
tty.c_cflag &= ~CSTOPB;
tty.c_cflag &= ~CRTSCTS;

tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
tty.c_oflag &= ~OPOST;
tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

tty.c_cc[VMIN] = 0;
tty.c_cc[VTIME] = 0;

return tcsetattr(fd, TCSANOW, &tty) == 0;
}

std::string STM32G030F6_Class::BytesToHex(const std::vector<uint8_t>& bytes) {
std::ostringstream oss;
for (size_t i = 0; i < bytes.size(); ++i) {
if (i > 0) {
oss << ' ';
}
oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
<< static_cast<int>(bytes[i]);
}
return oss.str();
}




namespace {
void PrintUsage(const char* prog) {
    std::cerr << "Usage: " << prog << " <firmware.bin> [interface.cfg] [target.cfg] [address_hex] [adapter_khz]\n";
    std::cerr << "       " << prog << " --ob [interface.cfg] [target.cfg] [adapter_khz]\n";
    std::cerr << "       " << prog << " --write-ob <OPTR_hex> [interface.cfg] [target.cfg] [adapter_khz]\n";
    std::cerr << "       " << prog << " --cmp <firmware.bin> [interface.cfg] [target.cfg] [address_hex] [adapter_khz]\n";
    std::cerr << "       " << prog << " --erase [interface.cfg] [target.cfg] [adapter_khz]\n";
    std::cerr << "       " << prog << " --reset [interface.cfg] [target.cfg] [adapter_khz]\n";
    std::cerr << "       " << prog << " --testInterface [interface.cfg] [target.cfg] [adapter_khz]\n";
    std::cerr << "       " << prog << " --uart \"HEX BYTES\" [port] [baud] [idle_ms]\n";
    std::cerr << "Example: " << prog << " firmware.bin interface/stlink.cfg target/stm32g0x.cfg 0x08000000 1000\n";
    std::cerr << "Example: " << prog << " --ob interface/raspberrypi2-native.cfg target/stm32g0x.cfg 200\n";
    std::cerr << "Example: " << prog << " --write-ob 0x1FFEF8AA\n";
    std::cerr << "Example: " << prog << " --cmp firmware.bin interface/raspberrypi2-native.cfg target/stm32g0x.cfg 0x08000000 200\n";
    std::cerr << "Example: " << prog << " --erase interface/raspberrypi2-native.cfg target/stm32g0x.cfg 200\n";
    std::cerr << "Example: " << prog << " --reset interface/raspberrypi2-native.cfg target/stm32g0x.cfg 200\n";
    std::cerr << "Example: " << prog << " --testInterface\n";
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

int STM32G030F6_Class::Flash_Func(int argc, char* argv[]){
	if (argc < 2) {
        PrintUsage(argv[0]);
        return 2;
    }

    if (std::string(argv[1]) == "--ob") {
        if (argc == 2) {
            //STM32G030F6_Class stm32;
            const bool ok = BootMode();
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
        interfaceCfg_ = interfaceCfg;
        targetCfg_ = targetCfg;
        adapterKhz_ = adapterKhz;
        const bool ok = BootMode();
        return ok ? 0 : 1;
    }

    if (std::string(argv[1]) == "--write-ob") {
        if (argc < 3) {
            PrintUsage(argv[0]);
            return 2;
        }
        
        uint32_t optrValue = 0;
        if (!ParseHex(argv[2], optrValue)) {
            std::cerr << "Invalid OPTR value: " << argv[2] << "\n";
            return 2;
        }

        if (argc == 3) {
            //STM32G030F6_Class stm32;
            const bool ok = WriteOptionBytes(optrValue);
            return ok ? 0 : 1;
        }
        
        if (argc < 6) {
            PrintUsage(argv[0]);
            return 2;
        }
        
        const std::string interfaceCfg = argv[3];
        const std::string targetCfg = argv[4];
        const int adapterKhz = std::atoi(argv[5]);
        if (adapterKhz <= 0) {
            std::cerr << "Invalid adapter_khz: " << argv[5] << "\n";
            return 2;
        }
        
        interfaceCfg_ = interfaceCfg;
        targetCfg_ = targetCfg;
        adapterKhz_ = adapterKhz;
        const bool ok = WriteOptionBytes(optrValue);
        return ok ? 0 : 1;
    }

    if (std::string(argv[1]) == "--cmp") {
        if (argc == 3) {
            //STM32G030F6_Class stm32;
            const FlashCompareResult res = Verify(argv[2]);
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
        interfaceCfg_ = interfaceCfg;
        targetCfg_ = targetCfg;
        flashAddress_ = address;
        adapterKhz_ = adapterKhz;
        const FlashCompareResult res = Verify(binPath);
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
            //STM32G030F6_Class stm32;
            const bool ok = Clear();
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
        interfaceCfg_ = interfaceCfg;
        targetCfg_ = targetCfg;
        adapterKhz_ = adapterKhz;
        const bool ok = Clear();
        return ok ? 0 : 1;
    }

    if (std::string(argv[1]) == "--reset") {
        if (argc == 2) {
            //STM32G030F6_Class stm32;
            const bool ok = Reset();
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

        interfaceCfg_ = interfaceCfg;
        targetCfg_ = targetCfg;
        adapterKhz_ = adapterKhz;
        const bool ok = Reset();
        return ok ? 0 : 1;
    }

    if (std::string(argv[1]) == "--testInterface") {
        if (argc == 2) {
            const bool ok = TestInterface();
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

        interfaceCfg_ = interfaceCfg;
        targetCfg_ = targetCfg;
        adapterKhz_ = adapterKhz;
        const bool ok = TestInterface();
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

        //STM32G030F6_Class stm32("interface/raspberrypi2-native.cfg", "target/stm32g0x.cfg", 0x08000000, 200, port, baud);
		uartPort_ = port;
		uartBaud_ = baud;
        std::string responseHex;
        const bool ok = FrameProcess(hexStream, responseHex, idleMs);
        if (!ok) {
            return 1;
        }
        std::cout << responseHex << std::endl;
        std::cout << HexToAscii(responseHex) << std::endl;
        return 0;
    }

    const std::string binPath = argv[1];
    if (argc == 2) {
        //STM32G030F6_Class stm32;
        const bool ok = Flash(binPath);
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
    interfaceCfg_ = interfaceCfg;
    targetCfg_ = targetCfg;
    flashAddress_ = address;
    adapterKhz_ = adapterKhz;
    const bool ok = Flash(binPath);
    return ok ? 0 : 1;
}
int STM32G030F6_Class::Flash_Func(std::initializer_list<const char*> args) {
    std::vector<const char*> argv_vec;
    argv_vec.push_back("ica2506_flash");  // Program name as argv[0]
    argv_vec.insert(argv_vec.end(), args.begin(), args.end());
    return Flash_Func(static_cast<int>(argv_vec.size()), 
                      const_cast<char**>(argv_vec.data()));
}
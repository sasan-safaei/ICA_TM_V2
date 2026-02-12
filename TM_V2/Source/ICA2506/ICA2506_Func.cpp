#include "ICA2506_Func.h"

#include <cstdio>
#include <cstdlib>
#include <limits.h>
#include <sstream>
#include <sys/stat.h>

namespace {
constexpr int kRpiSwdioGpio = 25; // Pin 22
constexpr int kRpiSwclkGpio = 11; // Pin 23
constexpr int kRpiNresetGpio = 24; // Pin 18
constexpr int kRpiWorkAreaSize = 0x400;
constexpr int kRpiWorkAreaBase = 0x20000000;

bool FileExists(const std::string& path) {
	struct stat st;
	return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

std::string ToAbsolutePath(const std::string& path) {
	char resolved[PATH_MAX];
	if (realpath(path.c_str(), resolved)) {
		return std::string(resolved);
	}
	return path;
}

bool CommandAvailable(const std::string& command) {
	std::ostringstream oss;
	oss << "command -v " << command << " >/dev/null 2>&1";
	return std::system(oss.str().c_str()) == 0;
}

bool IsRaspberryPiInterface(const std::string& interfaceCfg) {
	return interfaceCfg.find("raspberrypi") != std::string::npos;
}
} // namespace

bool Flash_STM32G030F6(const std::string& binPath) {
	return Flash_STM32G030F6(binPath,
						 "interface/raspberrypi2-native.cfg",
							 "target/stm32g0x.cfg",
							 0x08000000,
						 200);
}

bool Flash_STM32G030F6(const std::string& binPath,
					   const std::string& interfaceCfg,
					   const std::string& targetCfg,
					   uint32_t address,
					   int adapterKhz) {
	if (!FileExists(binPath)) {
		fprintf(stderr, "Flash_STM32G030F6: file not found: %s\n", binPath.c_str());
		return false;
	}
	if (!CommandAvailable("openocd")) {
		fprintf(stderr, "Flash_STM32G030F6: openocd not found in PATH\n");
		return false;
	}

	const std::string absBinPath = ToAbsolutePath(binPath);

	std::ostringstream cmd;
	cmd << "openocd";

	if (IsRaspberryPiInterface(interfaceCfg)) {
		cmd << " -c \"adapter driver bcm2835gpio\""
			<< " -c \"transport select swd\""
			<< " -c \"bcm2835gpio_swd_nums " << kRpiSwclkGpio << " " << kRpiSwdioGpio << "\"";
	} else {
		cmd << " -f " << interfaceCfg;
	}

	cmd << " -c \"set WORKAREASIZE 0x" << std::hex << kRpiWorkAreaSize << "\""
		<< " -f " << targetCfg
		<< " -c \"adapter speed " << std::dec << adapterKhz << "\""
		<< " -c \"stm32g0x.cpu configure -work-area-phys 0x" << std::hex << kRpiWorkAreaBase
		<< " -work-area-size 0x" << std::hex << kRpiWorkAreaSize << " -work-area-backup 0\""
		<< " -c \"cortex_m reset_config sysresetreq\""
		<< " -c \"init\""
		<< " -c \"reset halt\""
		<< " -c \"flash write_image erase " << absBinPath << " 0x" << std::hex << address << " bin\""
		<< " -c \"verify_image " << absBinPath << " 0x" << std::hex << address << " bin\""
		<< " -c \"reset run\""
		<< " -c \"shutdown\"";

	const int result = std::system(cmd.str().c_str());
	if (result != 0) {
		fprintf(stderr, "Flash_STM32G030F6: openocd failed (%d)\n", result);
		return false;
	}

	return true;
}

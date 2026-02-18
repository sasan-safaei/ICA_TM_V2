#include "ICA2506_Flash_Micro.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits.h>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>

namespace {
constexpr int kRpiSwdioGpio = 25; // Pin 22
constexpr int kRpiSwclkGpio = 11; // Pin 23
constexpr int kRpiNresetGpio = 24; // Pin 18
constexpr int kRpiWorkAreaSize = 0x400;
constexpr int kRpiWorkAreaBase = 0x20000000;
constexpr uint32_t kOptionBytesAddr = 0x1FFF7800;
constexpr int kOptionBytesWords = 16;

bool FileExists(const std::string& path) {
	struct stat st;
	return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

bool FileSize(const std::string& path, size_t& size) {
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

void AppendInterfaceConfig(std::ostringstream& cmd, const std::string& interfaceCfg) {
	if (IsRaspberryPiInterface(interfaceCfg)) {
		cmd << " -c \"adapter driver bcm2835gpio\""
			<< " -c \"transport select swd\""
			<< " -c \"bcm2835gpio_swd_nums " << kRpiSwclkGpio << " " << kRpiSwdioGpio << "\"";
	} else {
		cmd << " -f " << interfaceCfg;
	}
}

int RunCommandCapture(const std::string& cmd, std::string& output) {
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

bool ParseMdwLine(const std::string& line, uint32_t& addr, std::vector<uint32_t>& values) {
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

void PrintOptionBytesDecode(const std::vector<std::pair<uint32_t, uint32_t>>& words) {
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

bool DumpFlashImage(const std::string& outPath,
					const std::string& interfaceCfg,
					const std::string& targetCfg,
					uint32_t address,
					size_t size,
					int adapterKhz,
					std::string& output) {
	std::ostringstream cmd;
	cmd << "openocd";
	AppendInterfaceConfig(cmd, interfaceCfg);
	cmd << " -c \"set WORKAREASIZE 0x" << std::hex << kRpiWorkAreaSize << "\""
		<< " -f " << targetCfg
		<< " -c \"adapter speed " << std::dec << adapterKhz << "\""
		<< " -c \"init\""
		<< " -c \"reset halt\""
		<< " -c \"dump_image " << outPath << " 0x" << std::hex << address
		<< " 0x" << std::hex << size << "\""
		<< " -c \"shutdown\"";

	const int status = RunCommandCapture(cmd.str() + " 2>&1", output);
	return status == 0;
}

FlashCompareResult CompareBinToDump(const std::string& binPath, const std::string& dumpPath) {
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
	AppendInterfaceConfig(cmd, interfaceCfg);
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

bool Read_STM32G030F6_OptionBytes() {
	return Read_STM32G030F6_OptionBytes("interface/raspberrypi2-native.cfg",
									"target/stm32g0x.cfg",
									200);
}

bool Read_STM32G030F6_OptionBytes(const std::string& interfaceCfg,
                                 const std::string& targetCfg,
                                 int adapterKhz) {
	if (!CommandAvailable("openocd")) {
		fprintf(stderr, "Read_STM32G030F6_OptionBytes: openocd not found in PATH\n");
		return false;
	}

	std::ostringstream cmd;
	cmd << "openocd";
	AppendInterfaceConfig(cmd, interfaceCfg);
	cmd << " -c \"set WORKAREASIZE 0x" << std::hex << kRpiWorkAreaSize << "\""
		<< " -f " << targetCfg
		<< " -c \"adapter speed " << std::dec << adapterKhz << "\""
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
		fprintf(stderr, "Read_STM32G030F6_OptionBytes: openocd exited with status %d\n", status);
	}

	PrintOptionBytesDecode(words);

	return true;
}

FlashCompareResult Compare_STM32G030F6_Flash(const std::string& binPath) {
	return Compare_STM32G030F6_Flash(binPath,
								"interface/raspberrypi2-native.cfg",
								"target/stm32g0x.cfg",
								0x08000000,
								200);
}

FlashCompareResult Compare_STM32G030F6_Flash(const std::string& binPath,
									  const std::string& interfaceCfg,
									  const std::string& targetCfg,
									  uint32_t address,
									  int adapterKhz) {
	if (!FileExists(binPath)) {
		fprintf(stderr, "Compare_STM32G030F6_Flash: file not found: %s\n", binPath.c_str());
		return FlashCompareResult::Error;
	}
	if (!CommandAvailable("openocd")) {
		fprintf(stderr, "Compare_STM32G030F6_Flash: openocd not found in PATH\n");
		return FlashCompareResult::Error;
	}

	size_t binSize = 0;
	if (!FileSize(binPath, binSize) || binSize == 0) {
		fprintf(stderr, "Compare_STM32G030F6_Flash: invalid file size\n");
		return FlashCompareResult::Error;
	}

	char tmpTemplate[] = "/tmp/ica2506_dump_XXXXXX";
	const int fd = mkstemp(tmpTemplate);
	if (fd < 0) {
		fprintf(stderr, "Compare_STM32G030F6_Flash: failed to create temp file\n");
		return FlashCompareResult::Error;
	}
	close(fd);
	const std::string dumpPath = tmpTemplate;

	std::string output;
	const bool ok = DumpFlashImage(dumpPath, interfaceCfg, targetCfg, address, binSize, adapterKhz, output);
	if (!ok) {
		fprintf(stderr, "Compare_STM32G030F6_Flash: openocd dump failed\n");
		fprintf(stderr, "%s\n", output.c_str());
		std::remove(dumpPath.c_str());
		return FlashCompareResult::Error;
	}

	FlashCompareResult result = CompareBinToDump(binPath, dumpPath);
	std::remove(dumpPath.c_str());
	return result;
}

bool Erase_STM32G030F6_Flash() {
	return Erase_STM32G030F6_Flash("interface/raspberrypi2-native.cfg",
								"target/stm32g0x.cfg",
								200);
}

bool Erase_STM32G030F6_Flash(const std::string& interfaceCfg,
							 const std::string& targetCfg,
							 int adapterKhz) {
	if (!CommandAvailable("openocd")) {
		fprintf(stderr, "Erase_STM32G030F6_Flash: openocd not found in PATH\n");
		return false;
	}

	std::ostringstream cmd;
	cmd << "openocd";
	AppendInterfaceConfig(cmd, interfaceCfg);
	cmd << " -c \"set WORKAREASIZE 0x" << std::hex << kRpiWorkAreaSize << "\""
		<< " -f " << targetCfg
		<< " -c \"adapter speed " << std::dec << adapterKhz << "\""
		<< " -c \"init\""
		<< " -c \"reset halt\""
		<< " -c \"stm32g0x mass_erase 0\""
		<< " -c \"reset run\""
		<< " -c \"shutdown\"";

	const int result = std::system(cmd.str().c_str());
	if (result != 0) {
		fprintf(stderr, "Erase_STM32G030F6_Flash: openocd failed (%d)\n", result);
		return false;
	}

	return true;
}

bool Reset_STM32G030F6() {
	return Reset_STM32G030F6("interface/raspberrypi2-native.cfg",
							"target/stm32g0x.cfg",
							200);
}

bool Reset_STM32G030F6(const std::string& interfaceCfg,
						 const std::string& targetCfg,
						 int adapterKhz) {
	if (!CommandAvailable("openocd")) {
		fprintf(stderr, "Reset_STM32G030F6: openocd not found in PATH\n");
		return false;
	}

	std::ostringstream cmd;
	cmd << "openocd";
	AppendInterfaceConfig(cmd, interfaceCfg);
	cmd << " -c \"set WORKAREASIZE 0x" << std::hex << kRpiWorkAreaSize << "\""
		<< " -f " << targetCfg
		<< " -c \"adapter speed " << std::dec << adapterKhz << "\""
		<< " -c \"init\""
		<< " -c \"reset run\""
		<< " -c \"shutdown\"";

	const int result = std::system(cmd.str().c_str());
	if (result != 0) {
		fprintf(stderr, "Reset_STM32G030F6: openocd failed (%d)\n", result);
		return false;
	}

	return true;
}

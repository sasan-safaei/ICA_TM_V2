#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <chrono>
#include <vector>
#include <cmath>

/**
 * STM32G030 Bootloader Option Bytes Programmer
 * 
 * Requires:
 * - BOOT0 pin held HIGH (GPIO pin 16 or manual connection)
 * - Device in bootloader mode (power cycle with BOOT0 high)
 * - UART connection at /dev/ttyAMA1 (115200 baud for bootloader)
 * 
 * Usage: ./bootloader_optr <new_optr_value> [port] [baud]
 * Example: ./bootloader_optr 0xDAFF44BB /dev/ttyAMA1 115200
 */

static const int BOOTLOADER_TIMEOUT_MS = 100;
static const int RETRY_COUNT = 3;

// STM32 Bootloader Commands
static const uint8_t CMD_GET = 0x00;
static const uint8_t CMD_READ = 0x11;
static const uint8_t CMD_WRITE = 0x31;
static const uint8_t CMD_ERASE = 0x44;
static const uint8_t CMD_EXTENDED_ERASE = 0x45;
static const uint8_t ACK = 0x79;
static const uint8_t NACK = 0x1F;

int SetupUART(const char* port, int baudrate) {
	int fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		perror("Failed to open UART port");
		return -1;
	}

	struct termios tty;
	if (tcgetattr(fd, &tty) != 0) {
		perror("Failed to get terminal attributes");
		close(fd);
		return -1;
	}

	cfsetospeed(&tty, baudrate);
	cfsetispeed(&tty, baudrate);

	tty.c_cflag |= CS8;
	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	tty.c_oflag = 0;
	tty.c_lflag = 0;
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 5; // 500ms timeout

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		perror("Failed to set terminal attributes");
		close(fd);
		return -1;
	}

	// Send sync byte to trigger bootloader response
	uint8_t sync = 0x7F;
	write(fd, &sync, 1);
	usleep(10000);

	return fd;
}

bool ReadResponse(int fd, uint8_t& response, int timeout_ms) {
	fd_set rfds;
	struct timeval tv;
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;

	int ready = select(fd + 1, &rfds, nullptr, nullptr, &tv);
	if (ready <= 0) return false;

	uint8_t buf[1];
	ssize_t n = read(fd, buf, 1);
	if (n <= 0) return false;

	response = buf[0];
	return true;
}

bool BootloaderAck(int fd) {
	uint8_t response;
	return ReadResponse(fd, response, BOOTLOADER_TIMEOUT_MS) && response == ACK;
}

bool ProgramOptionBytes(int fd, uint32_t optr_value) {
	printf("Attempting to program option bytes via bootloader...\n");
	printf("Note: This bootloader method is experimental for STM32G030\n");

	// The actual option bytes programming via bootloader is complex
	// and may require platform-specific sequences. For now, this is a placeholder.
	// The recommended method is to use mass erase + reflash with new config.

	fprintf(stderr, "ERROR: STM32G030 bootloader option byte programming not yet implemented\n");
	fprintf(stderr, "Alternative: Use mass erase and reflash firmware\n");
	return false;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <optr_value_hex> [uart_port] [baudrate]\n", argv[0]);
		fprintf(stderr, "Example: %s 0xDAFF44BB /dev/ttyAMA1 115200\n", argv[0]);
		fprintf(stderr, "\nWARNING: This requires BOOT0 pin held HIGH and device in bootloader mode!\n");
		return 1;
	}

	uint32_t optr_value = 0;
	if (sscanf(argv[1], "0x%x", &optr_value) != 1 &&
	    sscanf(argv[1], "%u", &optr_value) != 1) {
		fprintf(stderr, "Invalid OPTR value: %s\n", argv[1]);
		return 1;
	}

	const char* port = argc > 2 ? argv[2] : "/dev/ttyAMA1";
	int baudrate = (argc > 3) ? atoi(argv[3]) : 115200;

	printf("OPTR Value: 0x%08X\n", optr_value);
	printf("UART Port: %s\n", port);
	printf("Baudrate: %d\n", baudrate);
	printf("\nIMPORTANT: Device must be in bootloader mode (BOOT0=HIGH)\n");
	printf("Press ENTER when ready: ");
	getchar();

	int fd = SetupUART(port, B115200);
	if (fd < 0) {
		fprintf(stderr, "Failed to setup UART\n");
		return 1;
	}

	if (!ProgramOptionBytes(fd, optr_value)) {
		fprintf(stderr, "Failed to program option bytes\n");
		close(fd);
		return 1;
	}

	close(fd);
	printf("Option bytes programmed successfully!\n");
	printf("Device will reset. BOOT0 must be LOW after programming.\n");
	return 0;
}

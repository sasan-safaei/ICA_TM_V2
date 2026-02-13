#include "ICA2506_UART.h"

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

namespace {
int HexValue(char ch) {
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

bool ParseHexStream(const std::string& hexStream, std::vector<uint8_t>& bytes) {
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

bool GetBaudConstant(int baud, speed_t& out) {
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

bool ConfigurePort(int fd, int baud) {
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

std::string BytesToHex(const std::vector<uint8_t>& bytes) {
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
} // namespace

bool UART_SendHexStream(const std::string& hexStream, std::string& responseHex) {
	return UART_SendHexStream(hexStream, responseHex, "/dev/ttyAMA1", 9600, 200);
}

bool UART_SendHexStream(const std::string& hexStream,
						std::string& responseHex,
						const std::string& port,
						int baud,
						int idleTimeoutMs) {
	responseHex.clear();
	std::vector<uint8_t> tx;
	if (!ParseHexStream(hexStream, tx)) {
		fprintf(stderr, "UART_SendHexStream: invalid hex stream\n");
		return false;
	}

	const int fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "UART_SendHexStream: open failed: %s\n", strerror(errno));
		return false;
	}

	if (!ConfigurePort(fd, baud)) {
		fprintf(stderr, "UART_SendHexStream: configure failed\n");
		close(fd);
		return false;
	}

	if (!tx.empty()) {
		const ssize_t written = write(fd, tx.data(), tx.size());
		if (written < 0 || static_cast<size_t>(written) != tx.size()) {
			fprintf(stderr, "UART_SendHexStream: write failed\n");
			close(fd);
			return false;
		}
	}

	std::vector<uint8_t> rx;
	rx.reserve(1024);

	const auto start = std::chrono::steady_clock::now();
	auto last_data = start;
	const int timeout_ms = idleTimeoutMs > 0 ? idleTimeoutMs : 200;

	while (true) {
		const auto now = std::chrono::steady_clock::now();
		const auto idle_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_data).count();
		if (!rx.empty() && idle_elapsed >= timeout_ms) {
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
	return true;
}

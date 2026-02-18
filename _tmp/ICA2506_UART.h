
#pragma once

#include <string>

bool UART_SendHexStream(const std::string& hexStream,
						std::string& responseHex);
bool UART_SendHexStream(const std::string& hexStream,
						std::string& responseHex,
						const std::string& port,
						int baud,
						int idleTimeoutMs);

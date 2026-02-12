
#pragma once

#include <cstdint>
#include <string>

bool Flash_STM32G030F6(const std::string& binPath);
bool Flash_STM32G030F6(const std::string& binPath,
					   const std::string& interfaceCfg,
					   const std::string& targetCfg,
					   uint32_t address,
					   int adapterKhz);

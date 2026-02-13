
#pragma once

#include <cstdint>
#include <string>

bool Flash_STM32G030F6(const std::string& binPath);
bool Flash_STM32G030F6(const std::string& binPath,
					   const std::string& interfaceCfg,
					   const std::string& targetCfg,
					   uint32_t address,
					   int adapterKhz);

bool Read_STM32G030F6_OptionBytes();
bool Read_STM32G030F6_OptionBytes(const std::string& interfaceCfg,
								 const std::string& targetCfg,
								 int adapterKhz);

enum class FlashCompareResult {
	Error = -1,
	Empty = 0,
	Same = 1,
	Different = 2
};

FlashCompareResult Compare_STM32G030F6_Flash(const std::string& binPath);
FlashCompareResult Compare_STM32G030F6_Flash(const std::string& binPath,
									  const std::string& interfaceCfg,
									  const std::string& targetCfg,
									  uint32_t address,
									  int adapterKhz);

bool Erase_STM32G030F6_Flash();
bool Erase_STM32G030F6_Flash(const std::string& interfaceCfg,
							 const std::string& targetCfg,
							 int adapterKhz);

bool Reset_STM32G030F6();
bool Reset_STM32G030F6(const std::string& interfaceCfg,
						 const std::string& targetCfg,
						 int adapterKhz);

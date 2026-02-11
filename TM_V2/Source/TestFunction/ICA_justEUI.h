#ifndef __ICA2308_H__
#define __ICA2308_H__
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>

#include <sstream>
#include <iomanip>
#include <string>
#include "eeprom.h"
class EEPROM24xx256{
public:
    explicit EEPROM24xx256(uint8_t i2c_addr = 0x50, const char* i2c_dev = "/dev/i2c-3")
        : address(i2c_addr), dev_path(i2c_dev) {}

    bool readBlock(uint8_t startReg, uint8_t length, std::vector<uint8_t>& buffer);
    //bool writeBlock(uint8_t startReg, const std::vector<uint8_t>& data);
    
private:
    uint8_t address;
    const char* dev_path;

    bool openBus(int& file){
    file = open(dev_path, O_RDWR);
    if (file < 0) return false;
    if (ioctl(file, I2C_SLAVE, address) < 0) {
        close(file);
        return false;
    }
    return true;
}
    void closeBus(int file){if (file >= 0) close(file);}
};
class MCP79412 {
public:
    explicit MCP79412(uint8_t i2c_addr = 0x57, const char* i2c_dev = "/dev/i2c-3")
        : address(i2c_addr), dev_path(i2c_dev) {}

    bool readBlock(uint8_t startReg, uint8_t length, std::vector<uint8_t>& buffer);
    bool writeBlock(uint8_t startReg, const std::vector<uint8_t>& data);
    
private:
    uint8_t address;
    const char* dev_path;

    bool openBus(int& file);
    void closeBus(int file);

};

class ICA_justEUI{
    
    MCP79412 rtc;
    EEPROM24xx256 myEEPROM;    
public:
    std::vector<uint8_t> EUI_Buffer;
    bool ReadEUI();
    bool ReadEUI_EEPROM();
    std::string showEUI();
};

#endif
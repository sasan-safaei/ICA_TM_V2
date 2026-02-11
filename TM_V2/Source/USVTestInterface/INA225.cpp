#include "INA226.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <iostream>

#define INA226_REG_BUS_VOLTAGE  0x02
#define INA226_REG_CURRENT      0x04
#define INA226_REG_CALIBRATION 0x05

INA226CLASS::INA226CLASS(const std::string& i2cBus, uint8_t address)
    : i2cBusPath(i2cBus), i2cAddress(address), i2cFile(-1) {}
INA226CLASS::~INA226CLASS() {
    if (i2cFile >= 0) close(i2cFile);
}
void INA226CLASS::setBus(const std::string& i2cBus, uint8_t address) {
        i2cBusPath = i2cBus;
        i2cAddress = address;
    }
bool INA226CLASS::begin() {
    i2cFile = open(i2cBusPath.c_str(), O_RDWR);
    if (i2cFile < 0) {
        std::cerr << "Failed to open I2C bus\n";
        return false;
    }
    if (ioctl(i2cFile, I2C_SLAVE, i2cAddress) < 0) {
        std::cerr << "Failed to set I2C address\n";
        return false;
    }
    setCalibration(8738);    
    return true;
}

void INA226CLASS::setAddress(uint8_t address) {
    i2cAddress = address;
    ioctl(i2cFile, I2C_SLAVE, i2cAddress);
}

int16_t INA226CLASS::readRegister(uint8_t reg) {
    uint8_t buffer[2];
    uint8_t __try_cnt=10;
    while(--__try_cnt>0){
    
        if (write(i2cFile, &reg, 1) == 1) {
            if (read(i2cFile, buffer, 2) == 2) {
                return (buffer[0] << 8) | buffer[1];
            }
        }
        usleep(1000);
    }
    return -1;
    //if (write(i2cFile, &reg, 1) != 1) {
    //    std::cerr << "Failed to write register address\n";
    //    return 0;
    //}
    //if (read(i2cFile, buffer, 2) != 2) {
    //    std::cerr << "Failed to read register\n";
    //    return 0;
    //}
    //return (buffer[0] << 8) | buffer[1];
}
bool INA226CLASS::writeRegister(uint8_t reg, uint16_t value) {
    uint8_t buffer[3];
    buffer[0] = reg;
    buffer[1] = value >> 8;
    buffer[2] = value & 0xFF;

    if (write(i2cFile, buffer, 3) != 3) {
        std::cerr << "Failed to write to register 0x" << std::hex << (int)reg << "\n";
        return false;
    }
    return true;
}
float INA226CLASS::readBusVoltage() {
    int16_t raw = readRegister(INA226_REG_BUS_VOLTAGE);
    if(raw==-1) return -1;
    return raw * 1.25e-3f;  // 1.25 mV per bit
}

float INA226CLASS::readCurrent() {
    float currentLSB = 0.0000977f;
    int16_t raw = static_cast<int16_t>(readRegister(INA226_REG_CURRENT));
    if(raw==-1) return -1;
    return (float)raw *currentLSB;//* 0.001f;  // You must adjust this based on your calibration
}
void INA226CLASS::setCalibration(uint16_t cal) {
    writeRegister(INA226_REG_CALIBRATION, cal);
}

#ifndef INA226CLASS_H
#define INA226CLASS_H

#include <string>



class INA226CLASS {
public:
    INA226CLASS(){}    
    INA226CLASS(const std::string& i2cBus, uint8_t address);
    ~INA226CLASS();

    bool begin();
    void setAddress(uint8_t address);
    float readBusVoltage();  // in volts
    float readCurrent();     // in amps (assumes calibration done externally)
    void setBus(const std::string& i2cBus, uint8_t address);
private:
    int i2cFile;
    std::string i2cBusPath;
    uint8_t i2cAddress;

    bool writeRegister(uint8_t reg, uint16_t value);
    int16_t readRegister(uint8_t reg);
    void setCalibration(uint16_t cal);
};

#endif

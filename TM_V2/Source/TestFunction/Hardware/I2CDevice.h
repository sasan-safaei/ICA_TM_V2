// ========================================================================
//	ICA
//	Walter-Welp Str. 25, 44149 Dortmund
// +49-231-917044-0
// ========================================================================
//	I2C
// ========================================================================
// Project		I2C
// Filename  	I2C.h
// Author		Sasan Safaei
// Date      	03.15.2023
// ========================================================================

#ifndef _ICA_I2C_H
#define _ICA_I2C_H

#include <stdint.h>
//#include <string>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
//extern "C" {
//#include <i2c/smbus.h>
//}
#include <iostream>


// === CLASS: I2C ===========================================================================================

class I2CDevice
{
private:
	char 		mDevice[128];
	int			mI2C_FD;
	uint8_t	mDevAddr;
	uint8_t write_i2c_byte(int fd, uint8_t reg, uint8_t value) {
		uint8_t buffer[2] = {reg, value};
	
		if (write(fd, buffer, 2) != 2) {
			//perror("Failed to write to the i2c bus");
			return -1;
		}	
		return 0;
	}
	uint8_t i2c_read_byte(int fd, uint8_t reg) {
		// Write register address
		if (write(fd, &reg, 1) != 1) {
			//perror("Failed to write register address");
			return -1;
		}	
		// Read response byte
		uint8_t value;
		if (read(fd, &value, 1) != 1) {
			//perror("Failed to read data");
			return -1;
		}
		return value; // Successfully read the byte
	}
	
	
	
	
	
public:
		I2CDevice (){
			sprintf(mDevice,"/dev/i2c-0");
			mI2C_FD= -1;
			mDevAddr= 0;
		}
		~I2CDevice (){
			if (mI2C_FD >= 0)
				close(mI2C_FD);
		}
	bool Init (uint8_t __busAdd, uint8_t __devAdd){
		sprintf(mDevice,"/dev/i2c-%d",__busAdd);
		mDevAddr= __devAdd;
		if (mI2C_FD >= 0)
			close(mI2C_FD);
		mI2C_FD = open(mDevice, O_RDWR);
		if (mI2C_FD < 0) {
			std::cout << "Unable to open device node: " << mDevice << '\n';
			return false;
		}
		if (ioctl(mI2C_FD, I2C_SLAVE, mDevAddr) < 0) {
			std::cout << "Could not set I2C_SLAVE\n";
			close(mI2C_FD);
			mI2C_FD = -1;
			return false;
		}
		return true;
	}
	
	uint8_t ReadByte(){
		uint8_t value;
		if (read(mI2C_FD, &value, 1) != 1) {
			if (read(mI2C_FD, &value, 1) != 1) {
				if (read(mI2C_FD, &value, 1) != 1) {
					//perror("Failed to read data");
					return -1;
				}
			}
		}
		return value; 
	}
	bool WriteByte(const uint8_t iValue){
		uint8_t buffer[1] = {iValue};
		if (mI2C_FD < 0) return false;
		if (write(mI2C_FD, buffer, 1) != 1) return -1;
		return true;
	}
	bool WriteReg (const uint8_t iReg,const uint8_t iValue)
	{
		if (mI2C_FD < 0) 
			return false;
		//if (i2c_smbus_write_byte_data(mI2C_FD, iReg, iValue) < 0) { return false; }
		write_i2c_byte(mI2C_FD, iReg, iValue);
		return true;
	}
	uint8_t ReadReg (const uint8_t iReg){
		if (mI2C_FD < 0)
			return false;
		//return i2c_smbus_read_byte_data(mI2C_FD, iReg);
		return i2c_read_byte(mI2C_FD, iReg);
		return false;
	}	
	bool WriteRegWord(uint8_t reg, uint16_t value) {
		uint8_t buffer[3] = {reg, (uint8_t)((value>>8)&0xFF),(uint8_t)(value&0xFF)};		
		if (write(mI2C_FD, buffer, 3) != 3) return -1;
		return 0;
	}
	uint16_t ReadRegWord(uint8_t reg) {
		if (write(mI2C_FD, &reg, 1) != 1) return -1;
		uint8_t value[2];
		uint16_t result=0;
		if (read(mI2C_FD, &value, 2) != 2) return -1;
		result=((uint16_t)value[0]<<8)& 0xFF00;
		result+=(uint16_t)value[1]& 0xFF;
		return result;
	}

};

#endif // _ICA_I2C_H


// ========================================================================
//	ICA
//	Walter-Welp Str. 25, 44149 Dortmund
// +49-231-917044-0
// ========================================================================
//	LCD on ICA-USV-TestDevice
// ========================================================================
// Project		ICA-USV-TestDevice
// Filename  	i2C-ADC (ADS111X)
// Author		Sasan Safaei
// Date      	2025-04-04
// ========================================================================

#ifndef _MY_ADS111X_H_
#define _MY_ADS111X_H_

#include <stdint.h>
#include <string>
#include <stdio.h>
#include "../TestFunction/Hardware/I2CDevice.h"


class myADS111XClass
{
private:
    struct I2C_Reg {
		enum N {
             Conversion = 	0x00
			,Config  	= 	0x01
			,LoThresh	= 	0x02
			,HiThresh	= 	0x03			
		};
	};
	struct I2C_CMD {
		enum N {
              X1    = 	0x01
			, X2    = 	0x02
			, X3    = 	0x03
			, X4	= 	0x04
            , X5    =   0x05
		};
	};
    float gainAmp[8]={6.144,4.096,2.048,1.024,.512,.256,.256,.256};
    bool busStatus=false;
    uint8_t defaultGAIN = 0;
	I2CDevice myI2C;
    
public:	
// Initialize
    bool Init (uint8_t _busAdd, uint8_t _devAdd, uint8_t _defaultGain) {
    busStatus=myI2C.Init(_busAdd,_devAdd);	
    myI2C.WriteRegWord(myADS111XClass::I2C_Reg::HiThresh,0xFFFF);
    myI2C.WriteRegWord(myADS111XClass::I2C_Reg::LoThresh,0x0000);
    defaultGAIN=_defaultGain &0x07;
    return busStatus;	
    
    }
    uint16_t readRegister(uint8_t _regNo){
        return myI2C.ReadRegWord(_regNo);
    }
	bool read_Start(uint8_t _portNo){
        uint16_t ConfigValue=0x8180;//0x8583;
        uint16_t tmpValue= (_portNo) &0x03;
        tmpValue|=0x4;
        ConfigValue|= tmpValue<<12;
        ConfigValue|= defaultGAIN<<9;
        
        if(_portNo>=0 && _portNo<4){
            myI2C.WriteRegWord(myADS111XClass::I2C_Reg::Config,ConfigValue);
            return true;
        }
        return false;
    }
    float read_Data(float _correction){        
        float result=0;
        int value=  myI2C.ReadRegWord(myADS111XClass::I2C_Reg::Conversion);
        if (value<0x7FFF)
            result=(float)value*gainAmp[defaultGAIN]/0x7FFF;
        else
            result=-1;
        return result * _correction;
    }
};
    
// =====================================================================================================================
#endif


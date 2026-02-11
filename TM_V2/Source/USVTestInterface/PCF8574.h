// ========================================================================
//	ICA
//	Walter-Welp Str. 25, 44149 Dortmund
// +49-231-917044-0
// ========================================================================
//	LCD on ICA-USV-TestDevice
// ========================================================================
// Project		ICA-USV-TestDevice
// Filename  	i2C-8xGPIO
// Author		Sasan Safaei
// Date      	2025-04-02
// ========================================================================

#ifndef _MY_EXTENDGPIO_H_
#define _MY_EXTENDGPIO_H_

#include <stdint.h>
#include <string>
#include <stdio.h>
#include "../TestFunction/Hardware/I2CDevice.h"
// === CLASS: MyLCD ===========================================================================================


class myExtendGPIO
{
private:
    //uint8_t GPIO_Value=0;	
    bool busStatus=false;
	I2CDevice myI2C;
public:
    // Initialize LCD
    bool Init (uint8_t _busAdd, uint8_t _devAdd) {
    busStatus=myI2C.Init(_busAdd,_devAdd);	
    return busStatus;	
    }	
    void SetBit(uint8_t bitNo1_8){
        uint8_t _value;
        uint8_t _repetCnt = 10;
        if(bitNo1_8 > 0 && bitNo1_8<=8){
            _value= myI2C.ReadByte();
            _value=_value | 1<< (bitNo1_8-1);
            while(_value!=myI2C.ReadByte() && _repetCnt-->0){
                myI2C.WriteByte(_value);
            }
        }        
    }
    void ClearBit(uint8_t bitNo1_8){
        uint8_t _value;
        uint8_t _repetCnt = 10;
        if(bitNo1_8 > 0 && bitNo1_8<=8){
            _value= myI2C.ReadByte();
            _value=_value & ~(1<< (bitNo1_8-1));
            while(_value!=myI2C.ReadByte() && _repetCnt-->0){
                myI2C.WriteByte(_value);
            }
        }
    }
    bool GetBit(uint8_t bitNo1_8){
        uint8_t _value;
        if(bitNo1_8 > 0 && bitNo1_8<=8){
            _value= myI2C.ReadByte();
            if(_value && 1<<(bitNo1_8-1))
                return true;
            else
                return false;
        }
    }
    void WriteGPIO(uint8_t GPIOValue){        
        while( GPIOValue!= myI2C.ReadByte()){
            myI2C.WriteByte(GPIOValue);
        }
    }
    uint8_t ReadGPIO(){
        uint8_t _value;
        _value= myI2C.ReadByte();
        return _value;
    }
};

// =====================================================================================================================
#endif


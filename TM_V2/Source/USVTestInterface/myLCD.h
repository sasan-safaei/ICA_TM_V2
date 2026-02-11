// ========================================================================
//	ICA
//	Walter-Welp Str. 25, 44149 Dortmund
// +49-231-917044-0
// ========================================================================
//	LCD on ICA-USV-TestDevice
// ========================================================================
// Project		ICA-USV-TestDevice
// Filename  	i2C-LCD 2x16.h
// Author		Sasan Safaei
// Date      	2025-04-01
// ========================================================================

#ifndef _MY_LCD_H
#define _MY_LCD_H

#include <stdint.h>
#include <string>
#include <stdio.h>
#include "../TestFunction/Hardware/I2CDevice.h"
// === CLASS: MyLCD ===========================================================================================


class myLCDClass
{
// === 2x16 LCD Interface ==================================================================================================
public:
	
private:
    uint8_t LCD_BACKLIGHT;
	struct I2C_Reg {
		enum N {
             LCD_CMD   	= 	0x00
			,LCD_CHR  	= 	0x01
			,POLARITY	= 	0x02
			,CONFIG	= 	0x03			
		};
	};
	struct I2C_CMD {
		enum N {
              LCD_LINE_1   	= 	0x80
			, LCD_LINE_2    = 	0xC0
            , LCD_LINE_3    = 	0x94
            , LCD_LINE_4    = 	0xD4
            
			, LCD_BL_ON = 	0x08//BackLight on
			, ENABLE	    = 	0x04
            , CLEAR_LCD     =   0x01
		};
	};
    bool busStatus=false;
	//uint8_t busAdd=0,devAdd=0;
	I2CDevice myI2C;
// Function to send byte to LCD
    void lcd_byte(int mode, int bits) {
        int bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
        int bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;
        myI2C.WriteByte(bits_high);//usleep(1);
        myI2C.WriteByte(bits_high | I2C_CMD::ENABLE );//usleep(1);
        myI2C.WriteByte(bits_high & ~I2C_CMD::ENABLE);//usleep(1);
        myI2C.WriteByte(bits_low);//usleep(1);
        myI2C.WriteByte(bits_low | I2C_CMD::ENABLE );//usleep(1);
        myI2C.WriteByte(bits_low & ~I2C_CMD::ENABLE);//usleep(1);

    }
// === internal typedefs ===============================================================================================
// === internal structs ================================================================================================
// === ctor/dtor/copy op= ==============================================================================================
public:
	void lcd_string(const char *message) {
        while (*message) {
            lcd_byte (myLCDClass::I2C_Reg::LCD_CHR, *message++);
            usleep(5000);
        }
    }
    void select_Line(uint8_t LineNo){
        switch (LineNo){
            case 1: lcd_byte (myLCDClass::I2C_Reg::LCD_CMD, myLCDClass::I2C_CMD::LCD_LINE_1); break;
            case 2: lcd_byte (myLCDClass::I2C_Reg::LCD_CMD, myLCDClass::I2C_CMD::LCD_LINE_2); break;
            case 3: lcd_byte (myLCDClass::I2C_Reg::LCD_CMD, myLCDClass::I2C_CMD::LCD_LINE_3); break;
            case 4: lcd_byte (myLCDClass::I2C_Reg::LCD_CMD, myLCDClass::I2C_CMD::LCD_LINE_4); break;
            default:lcd_byte (myLCDClass::I2C_Reg::LCD_CMD, myLCDClass::I2C_CMD::LCD_LINE_4); break;
        }
    }
    void cleanLCD(){
        lcd_byte (myLCDClass::I2C_Reg::LCD_CMD, myLCDClass::I2C_CMD::CLEAR_LCD);
        usleep(2000);
    }
    void setBackLight(bool _setBL){
        if(_setBL)
            LCD_BACKLIGHT=myLCDClass::I2C_CMD::LCD_BL_ON;
        else    
            LCD_BACKLIGHT=0;
    }
// Initialize LCD
    bool Init (uint8_t _busAdd, uint8_t _devAdd) {
    busStatus=myI2C.Init(_busAdd,_devAdd);	
    LCD_BACKLIGHT=I2C_CMD::LCD_BL_ON;
	lcd_byte (myLCDClass::I2C_Reg::LCD_CMD, 0x33);	
    lcd_byte (myLCDClass::I2C_Reg::LCD_CMD, 0x32);	
    lcd_byte (myLCDClass::I2C_Reg::LCD_CMD, 0x06);	
    lcd_byte (myLCDClass::I2C_Reg::LCD_CMD, 0x0C);	
    lcd_byte (myLCDClass::I2C_Reg::LCD_CMD, 0x28);	
    lcd_byte (myLCDClass::I2C_Reg::LCD_CMD, 0x01);	
    return busStatus;	
    
    }
	
};
// =====================================================================================================================
#endif


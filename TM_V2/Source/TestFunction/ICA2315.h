#ifndef ICA2315_H
#define ICA2315_H


#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "mySerialPort.h"
#include <stdint.h>
#include <stddef.h>
#include "eeprom.h"
#include "GlobalFunc.h"

#include "./LTC3350/LTC3350_reg_defs.h"
#include "./LTC3350/LTC3350_formats.h"

#define GPI_SMBALERT 0
#define GPO_turnOff_MPower 4
#define VCellMax 0x463F //3.3V
#define VCellMin 0x2A93 //2.0V
#define VCellStep 0x220 //0.1V

//extern float Board_SupperCapVoltage;
//extern int Board_SupperCapSingleCap;
//extern int Board_SupperCapNum;
//extern boardInfo LTC3350value;

class ICA2315 {
public:
    eeprom myEEPROM;
    boardInfo myBoardInfo;
    
    int boardName=-1;
    int boardType=-1;
    int boardVer=-1;
    char boardName_str[25];
    char boardKind_str[25];
    ~ICA2315(){serialPort.sp_close();}
    void updateBoardNameStr(){ sprintf(boardName_str,"%dV%X",boardName,boardVer); }
    void serialRx_delay(useconds_t _usdelay = 20000){usleep(_usdelay);}//50000
    void GPIOResetAll(){ SP_Write({'O',0xE0,'P'});}
    void OutPUT_Off(){ SP_Write({'O',0x60,'P'});}
    void FlyBack_Off(){ SP_Write({'O',0xC0,'P'});}
    void FlyBackEn(){ SP_Write({'O',0xA0,'P'});}

    bool init(char* ttyName);
    bool showValue();
    bool showValue_Rest();
    void GetVINOUT(float* Vin,float* Vout);
    float GetLoadCurrent();
    float GetVCap(uint8_t _VCapNo);
    bool GetICTemp(float* tempVal,bool ShowVal);
    bool GetBatBankTemp(float* tempVal,bool ShowVal);
    bool Get_SMPALERT(){return GPIO_get(GPI_SMBALERT);}
    bool TurnOff_MPower(){return GPIO_set(GPO_turnOff_MPower);}
    
    bool GPIO_set(uint16_t GPO_No);
    bool GPIO_clr(uint16_t GPO_No);
    bool GPIO_get(uint16_t GPO_No);
    bool CapRefUp(void);    
    bool CapRefDown(void);    
    bool VShuntUp(void);
    bool VShuntDown(void);
    bool RemessCapESR(void);
    
    unsigned char GPIO_read();
    void LineClear(const char *message);    
    bool readEUI64(uint8_t *eui64);
    bool readDataBuff(uint8_t *buff);//new
    bool writeDataBuff(uint8_t *buff);//new
    bool SC18IM_I2C_Write_EEPROMBlock(uint8_t ic_i2c_add, uint8_t reg_add, uint8_t *data);
    bool SC18IM_I2C_Read_EEPROMBlock(uint8_t ic_i2c_add, uint8_t reg_add, uint8_t *data);
    
    void ReadEEPROMBlock(uint8_t block_add, uint8_t *data);
    bool WriteEEPROMBlock(uint8_t block_add, uint8_t *data);

    float getInputVoltage();//new

    bool CheckCapsVoltage(float* VCap);
private:
    static inline uint8_t get_size(uint16_t registerinfo) { return ((registerinfo >> 8) & 0x0F) + 1; }
    static inline uint8_t get_subaddr(uint16_t registerinfo) { return (registerinfo) & 0xFF; }
    static inline uint8_t get_offset(uint16_t registerinfo) { return (registerinfo >> 12) & 0x0F; }
    static inline uint16_t get_mask(uint16_t registerinfo)
    {
        uint16_t mask = 1 << get_offset(registerinfo);
        uint8_t size = get_size(registerinfo);
        uint8_t i;
        for (i=0; i<size-1; i++)
        {
            mask |= mask << 1;
        }
        return mask;
    }
    bool IC_LTC3350_read_register( uint16_t registerinfo, uint16_t *data);
    bool IC_LTC3350_write_register( uint16_t registerinfo, uint16_t data);
    uint16_t IC_LTC3350_GetValue(uint16_t registerinfo, uint16_t data);
    
    bool IC_18IM704_getVersion();
    void IC_18IM704_setBR();
    
    
    
    void IC_18IM704_Write_GPIO();
    bool SP_Write(std::initializer_list<unsigned char> arr);
    //bool SC18IM_I2C_Read(uint8_t ic_i2c_add,uint8_t reg_add,uint8_t len, uint8_t *data);
    
    
private:
    SerialPort serialPort;  
    uint16_t reg_read_LTC3350;
    uint16_t data;
    unsigned char I2C_LT3350_ADD = 0x09;
    //#define EEPROM_I2C_ADDRESS 0x50  // I2C address of 24AA025E64
    //#define EUI64_ADDRESS      0xF8  // Starting address of EUI-64 in EEPROM
    //#define EEPROMDateBufferAdd   0x00
    //#define EEPROMDateBufferLen   4
    
    uint16_t capRafValue= 15;
    uint16_t VShuntValue= 0;

};
extern ICA2315 myBoard;
#endif
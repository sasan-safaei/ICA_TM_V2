// ========================================================================
//	ICA
//	Walter-Welp Str. 25, 44149 Dortmund
// +49-231-917044-0
// ========================================================================
//	USV Test Interface
// ========================================================================
// Project		USV Test
// Filename  	USVTestInterface.h
// Author		Sasan Safaei
// Date      	2025-04-07
// ========================================================================

#ifndef _ICA_USVTESTINTERFACE_H_
#define _ICA_USVTESTINTERFACE_H_

#include "printFunc.h"
#include "myLCD.h"
#include "PCF8574.h"
#include "ADS111X.h"
#include "INA226.h"
#include <string>
#include <cstring> // for memcpy
//changes for RPi4
//testRoutine.cpp Line 34: testKeyLed.Init(1,0x27);
#define FronLED 2
class USV_Test_Interface{
private:
    float adcValue[4]={0.0,0.0,0.0,0.0};
    const float adcCorrector[4]={1.0,8.58,1.0,1.0};
    const float ACS712_Factor_LTable[11]={15.0,9.1,7.7,7.3,6.8,6.5,6.5,6.3,6.2,6.1,6};
    myLCDClass myLCD;
    myExtendGPIO myRELAY,myDONGLE,myLEDKEY;
    myADS111XClass myADC1;
    INA226CLASS myVIN,myVOUT;
    
public:
    float InAmp0Value=0;
    float LoadAmp0Value=0;
    float InAmpFactor=80;
    float LoadAmpFactor=80;
    struct Relays {
        enum N {
            Load   	= 	1
            ,VCCLoad = 	2
            ,MPower = 	3
            ,LabPowerSel = 	4	
            ,AR = 5
            ,All    =   6	
        };
    };
    struct AnalogInputPort{
        enum N{
            VCC         =   0
            //,VOUT       =   1
            //,Load_Amp   =   2
            //,In_Amp     =   3
        };
    };
    struct _KEY{
        enum N{
            K1  =   1
            ,K2 =   2
            ,K3 =   4
            ,K4 =   8
            
        };
    };
    
    void Init(){
        myLCD.Init(1,0x27);
        myRELAY.Init(1,0x38);//RELAY GPO //0x20
        myDONGLE.Init(1,0x39);//DunGLE //0x21
        myLEDKEY.Init(1,0x3A);//KEY & LED
        
        myADC1.Init(1,0x48,0);
        myVIN.setBus("/dev/i2c-1",0x40);
        myVOUT.setBus("/dev/i2c-1",0x41);
        myVIN.begin();
        myVOUT.begin();

        usleep(5000);
        myLCD.setBackLight(true);
        showOnLCD(1,"ICA-EUI-Test!");
        showOnLCD(2,"ver 1.2.0");
        usleep(1000000);
    }
    void cleanLCD(){
        myLCD.cleanLCD();
    }
    void showOnLCD(uint8_t _lineNo,std::string _str){
        char _buff[21];
        if(_lineNo<=4){
            size_t len = _str.length();
            if(len>20){
                std::memcpy(_buff, _str.data(), 20);
            }
            else{
                std::memcpy(_buff, _str.data(), len);
                std::memset(_buff + len, ' ', 20 - len);
            }
            _buff[20]=0;
            myLCD.select_Line(_lineNo);
            myLCD.lcd_string(_buff);
        }
    }
    float getAnalog(uint8_t _analogPort){
        if(_analogPort>=0 && _analogPort<4){
            myADC1.read_Start(_analogPort);
            usleep(50000);
            adcValue[_analogPort]=myADC1.read_Data(adcCorrector[_analogPort]);
            return adcValue[_analogPort];
        }
        return -1;
    }

    void __getAllAnalog(){
        for(int __i=0;__i<4;__i++){
            getAnalog(__i);
        }
    }    
    bool __set0Current(){
        //InAmp0Value=getAnalog(AnalogInputPort::In_Amp);
        //LoadAmp0Value=getAnalog(AnalogInputPort::Load_Amp);
        return true;
    }
    float __getPort_InAmp(){ 
        /*float _tmpValue=getAnalog(AnalogInputPort::In_Amp)-InAmp0Value;
        if(_tmpValue>=.005){
            InAmpFactor=11.8;//10.8;
            return .2+(_tmpValue-.005)*InAmpFactor;
        }
        if(_tmpValue>=0){
            InAmpFactor=40;
            return _tmpValue*40;
        }
        return _tmpValue;//-1; //just for test
        */
       return -1;
    }
    float __getPort_LoadAmp(){ 
        /*float _tmpValue=getAnalog(AnalogInputPort::Load_Amp)-LoadAmp0Value;
        if(_tmpValue>=.005){
            LoadAmpFactor=11.8;//10.8;
            return .2+(_tmpValue-.005)*LoadAmpFactor;
        }
        if(_tmpValue>=0){
            LoadAmpFactor=40;
            return _tmpValue*40;
        }
        return 0.0;
        */
        return -1;
        
    }
    void __ShowAnalog(){
        char _str[32];
        sprintf(_str,"0:%.2fV 1:%.2fV ",adcValue[0],adcValue[1]);
        showOnLCD(1,_str);
        sprintf(_str,"2:%.3f 3:%.3f ",adcValue[2],adcValue[3]);
        showOnLCD(2,_str);
    }
    bool setRelay(uint8_t _relayNo, bool _status){
        uint8_t _cnt=10;

        while (_cnt-->0)
        {
            if(_relayNo>0 && _relayNo <=5){
                if(_status){
                    myRELAY.ClearBit(_relayNo);
                    if(!(readRelay() & 1<<(_relayNo-1))) return true;
                }
                else{
                    myRELAY.SetBit(_relayNo);
                    if((readRelay() & 1<<(_relayNo-1))) return true;
                }
            }
            if(_relayNo==Relays::All){
                uint8_t _tmp = myRELAY.ReadGPIO();                
                if(_status)
                    myRELAY.WriteGPIO(_tmp & 0xE0);
                else
                    myRELAY.WriteGPIO(_tmp | 0x1F);
                if((readRelay() & 0x1F) == 0) return true;
            }    
            usleep(10000);
        }
        return false;    
    }
    uint8_t readRelay(){
        uint8_t _tmp = myRELAY.ReadGPIO();  
        return (~_tmp)&0x1F;
    }
    uint8_t readDongle(){
        uint8_t __dan;//= myDONGLE.ReadGPIO();
        //__dan|=0xF8;
        myDONGLE.WriteGPIO(0xFF);//(__dan);
        __dan= ~(myDONGLE.ReadGPIO());        
        return __dan; //(__dan>>3)&0x1F;
    }
    uint8_t readKeys(){
        uint8_t __key1= myLEDKEY.ReadGPIO();
        uint8_t __resualt=0;
        __key1|=0x0F;//0x70;

        myLEDKEY.WriteGPIO(__key1);
        __key1= ~(myLEDKEY.ReadGPIO());
        __key1&=0x0F;
        __resualt=__key1;
        if(__key1>0){
            cleanLCD();
            while(true){
                __key1= myLEDKEY.ReadGPIO();
                __key1|=0x0F;
                myLEDKEY.WriteGPIO(__key1);
                __key1= ~(myLEDKEY.ReadGPIO());
                __key1&=0x0F;
                if(__key1!=0){
                    if(__key1>__resualt)
                        __resualt=__key1;
                }
                else    
                    break;
                usleep(50000);
            } 
        }
        return __resualt;
    }
    void setLED(uint8_t _ledNo,bool _status){
        uint8_t __led= myLEDKEY.ReadGPIO();
        uint8_t __ledTmp=0xF0;
        __led|=0xF0;
        if(_ledNo>0 && _ledNo<=4) __ledTmp=1<<(_ledNo+3);
        if(_status)
            __led&=~(__ledTmp);
        else
            __led|=__ledTmp;
        myLEDKEY.WriteGPIO(__led);
    }
    void ShowAnalogInfo(){
        char _str[32];
        sprintf(_str,"Alanlog Value:");showOnLCD(1,_str);
        sprintf(_str,"Input :%.1fV %dmA",getDUT_VIN(),(int32_t)(getDUT_VINAmp()*1000));
        showOnLCD(2,_str);
        sprintf(_str,"OUTPUT:%.1fV %dmA",getDUT_VOUT(),(int32_t)(getDUT_VOUTAmp()*1000));
        showOnLCD(3,_str);            
        sprintf(_str,"DUT-VCC: %.1fV  ",getDUT_VCC());
        showOnLCD(4,_str);         
    }
    void ShowDischargeInfo(){
        char _str[32];
        sprintf(_str,"Discharge:");showOnLCD(1,_str);
        sprintf(_str,"Input :%.1fV %dmA",getDUT_VIN(),(int32_t)(getDUT_VINAmp()*1000));
        showOnLCD(2,_str);
        sprintf(_str,"OUTPUT:%.1fV %dmA",getDUT_VOUT(),(int32_t)(getDUT_VOUTAmp()*1000));
        showOnLCD(3,_str);            
        sprintf(_str,"DUT-VCC: %.1fV  ",getDUT_VCC());
        showOnLCD(4,_str);         
    }
    float getDUT_VCC(){
        float __data=getAnalog(AnalogInputPort::VCC);
        return __data;
    }
    float getDUT_VIN(){return myVIN.readBusVoltage();}
    float getDUT_VOUT(){return myVOUT.readBusVoltage();}
    float getDUT_VINAmp(){return myVIN.readCurrent();}
    float getDUT_VOUTAmp(){return myVOUT.readCurrent();}
    
};
#endif

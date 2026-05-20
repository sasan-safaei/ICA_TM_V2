#ifndef _GLOBALFUNC_H_
#define _GLOBALFUNC_H_

#include <iostream>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <termios.h>
#include <fcntl.h>

#include <iostream>
#include <ctime>


#define SET_TO_NS(sec) ((sec)*1000000000)

#define LED_Ok			1
#define LED_Failed		2
#define LED_TurnOnMainPower     3
#define LED_Sts2		4
#define LED_Sts3		5
#define LED_Sts4		6

#define Key_No          'N'
#define Key_Yes         'Y'
#define Key_Exit        'Q'

#define __const_PSVoltage 24.0
#define __const_PSCurrent 2.5
#define __const_PSCurrent_NoCap_NoLoad 0.5
#define __const_PSCurrent_NoCap_Load 1.0

#define __const_LoadCurrent24V 0.480//24V 50R
#define __const_LoadCurrent12V 0.240//12V 50R

#define __const_LoadCurrent_NoCap 0.300

#define __Limit_MAX_IC_Temp 65.0
#define __Limit_MAX_BatBank_Temp 50.0

#define __Limit_MAX_Charge_time_2405 60
#define __Limit_MAX_Charge_time_2315 180
//#define __Limit_MAX_ExtendChargeTime 180
#define __Limit_MIN_ChargeCurrent_2405 0.350
#define __Limit_MIN_ChargeCurrent_2315 0.500

#define __Limit_MIN_FullChargeCurrent_2405 0.160
#define __Limit_MIN_FullChargeCurrent_2315 0.300 //0.220

		
#define __Limit_MIN_WaitToOutSwOff_uC 8
#define __Limit_MAX_WaitToOutSwOff_uC 12
#define __Limit_MIN_OutSwOff_uC 3
#define __Limit_MAX_OutSwOff_uC 7

#define __Limit_MIN_WaitToOutSwOff_2405 0
#define __Limit_MAX_WaitToOutSwOff_2405 5
#define __Limit_MIN_OutSwOff_2405 10
#define __Limit_MAX_OutSwOff_2405 30

#define __Limit_MIN_VCap_ShutdownVoltage_2405 4.2
#define __Limit_MAX_VCap_ShutdownVoltage_2405 4.7

#define __Limit_MIN_WaitToOutSwOff_2315 5
#define __Limit_MAX_WaitToOutSwOff_2315 20
#define __Limit_MIN_OutSwOff_2315 10
#define __Limit_MAX_OutSwOff_2315 30
#define __Limit_MIN_VCap_ShutdownVoltage_2315 5.5
#define __Limit_MAX_VCap_ShutdownVoltage_2315 6.5

#define __Limit_MAX_Charge_time_2510 70
#define __Limit_MIN_FullChargeCurrent_2510 0.160
#define __Limit_MIN_ChargeCurrent_2510 0.500
//#define __Limit_MIN_WaitToOutSwOff_2510 8
//#define __Limit_MAX_WaitToOutSwOff_2510 12
//#define __Limit_MIN_OutSwOff_2510 3
//#define __Limit_MAX_OutSwOff_2510 7
#define __Limit_MIN_VCap_ShutdownVoltage_2510 4.2
#define __Limit_MAX_VCap_ShutdownVoltage_2510 4.7


void printCurrentTime();
void ica_delay_us (int _value);
int ica_kbhit2(char* currentVal);

int time_test();
void convert_timeToBytes(uint8_t* buff);
time_t convert_BytesToTime(uint8_t* buff);


// Duration Timer
class durationTimerClass{
        private:
            struct timespec StartTime;
        public:
        void testTimeStartSec(){
            clock_gettime(CLOCK_MONOTONIC, &StartTime);
        }
        uint16_t TestTimeSec(){
            struct timespec __now;
            clock_gettime(CLOCK_MONOTONIC, &__now);
            return (uint16_t)(__now.tv_sec - StartTime.tv_sec);
        }
        
    };
    
    
    


struct testResult{
	uint8_t ErrorNo=0;
	uint8_t EEPROM_Status=0;
	char Logstr='0';
	uint8_t repaired_NoCap=0,repaired_Cap=0;
	uint8_t time_charge=0,time_DisCharge=0,time_OutSwOff=0,time_WaitToOutSwOff=0;
	float Vcap_Max=0;
	float VCap_SWOff=0;
	float Load_Current=0;
	float VOut1=0,VOut2=0;
	float Vout_SaveResult=0;
	float Vin_SaveResult=0;
	float tempIC=0,tempBatBank=0;
	//float Limit_MIN_ChargeCurrent=0;
	//float Limit_MIN_FullChargeCurrent=0;
	//int Limit_MAX_Charge_time=0;
	//int Limit_MIN_WaitToOutSwOff=0;
	//int Limit_MAX_WaitToOutSwOff=0;
	//int Limit_MIN_OutSwOff=0;
	//int Limit_MAX_OutSwOff=0;
	//float Limit_MAX_VCap_ShutdownVoltage=0;
	//float Limit_MIN_VCap_ShutdownVoltage=0;
	int Board_MaxTemp85V=0;
    void clear(int _boardName){
		Vout_SaveResult=0;Vin_SaveResult=0;
		ErrorNo=0;
		EEPROM_Status=0;
		Logstr='0';
		repaired_NoCap=0;repaired_Cap=0;
		time_charge=0;time_DisCharge=0;time_OutSwOff=0;
		time_WaitToOutSwOff=0;//was Not in testrReset()
		Vcap_Max=0;
		VCap_SWOff=0;//was Not in testrReset()
		Load_Current=0;
		VOut1=0;VOut2=0;
		tempIC=0;
		tempBatBank=0;//was Not in testrReset()
		//Limit_MIN_ChargeCurrent=0;
		//Limit_MIN_FullChargeCurrent=0;
		//Limit_MAX_Charge_time=0;
		//Limit_MIN_WaitToOutSwOff=0;
		//Limit_MAX_WaitToOutSwOff=0;		
		//Limit_MIN_OutSwOff=0;
		//Limit_MAX_OutSwOff=0;
		//Limit_MAX_VCap_ShutdownVoltage=0;
		//Limit_MIN_VCap_ShutdownVoltage=0;
		Board_MaxTemp85V=0;//was Not in testrReset()
		/*Load_Current=__const_LoadCurrent24V;
		switch (_boardName){
			case 2315:
				Limit_MAX_Charge_time=__Limit_MAX_Charge_time_2315;
				Limit_MIN_ChargeCurrent=__Limit_MIN_ChargeCurrent_2315;
				Limit_MIN_FullChargeCurrent=__Limit_MIN_FullChargeCurrent_2315;
				Limit_MAX_WaitToOutSwOff=__Limit_MAX_WaitToOutSwOff_2315;
				Limit_MIN_WaitToOutSwOff=__Limit_MIN_WaitToOutSwOff_2315;
				Limit_MIN_OutSwOff=__Limit_MIN_OutSwOff_2315;
				Limit_MAX_OutSwOff=__Limit_MAX_OutSwOff_2315;
				Limit_MIN_VCap_ShutdownVoltage=__Limit_MIN_VCap_ShutdownVoltage_2315;
				Limit_MAX_VCap_ShutdownVoltage=__Limit_MAX_VCap_ShutdownVoltage_2315;
				break;
			case 2405:
			case 2506:
				Limit_MAX_WaitToOutSwOff=__Limit_MAX_WaitToOutSwOff_2405;
				Limit_MIN_WaitToOutSwOff=__Limit_MIN_WaitToOutSwOff_2405;
				Limit_MIN_OutSwOff=__Limit_MIN_OutSwOff_2405;
				Limit_MAX_OutSwOff=__Limit_MAX_OutSwOff_2405;
				if(_boardName==2506){
					Limit_MAX_WaitToOutSwOff=__Limit_MAX_WaitToOutSwOff_uC;
					Limit_MIN_WaitToOutSwOff=__Limit_MIN_WaitToOutSwOff_uC;
					Limit_MIN_OutSwOff=__Limit_MIN_OutSwOff_uC;
					Limit_MAX_OutSwOff=__Limit_MAX_OutSwOff_uC;
				}
				Limit_MAX_Charge_time=__Limit_MAX_Charge_time_2405;
				Limit_MIN_ChargeCurrent=__Limit_MIN_ChargeCurrent_2405;
				Limit_MIN_FullChargeCurrent=__Limit_MIN_FullChargeCurrent_2405;
				
				Limit_MIN_VCap_ShutdownVoltage=__Limit_MIN_VCap_ShutdownVoltage_2405;
				Limit_MAX_VCap_ShutdownVoltage=__Limit_MAX_VCap_ShutdownVoltage_2405;
				break;
			case 2510:
				Load_Current=__const_LoadCurrent12V;
				Limit_MAX_Charge_time= __Limit_MAX_Charge_time_2405;
				Limit_MIN_ChargeCurrent=__Limit_MIN_ChargeCurrent_2510;
				Limit_MIN_FullChargeCurrent=__Limit_MIN_FullChargeCurrent_2510;
				Limit_MAX_WaitToOutSwOff=__Limit_MAX_WaitToOutSwOff_uC;
				Limit_MIN_WaitToOutSwOff=__Limit_MIN_WaitToOutSwOff_uC;
				Limit_MIN_OutSwOff=__Limit_MIN_OutSwOff_uC;
				Limit_MAX_OutSwOff=__Limit_MAX_OutSwOff_uC;
				Limit_MIN_VCap_ShutdownVoltage=__Limit_MIN_VCap_ShutdownVoltage_2510;
				Limit_MAX_VCap_ShutdownVoltage=__Limit_MAX_VCap_ShutdownVoltage_2510;
				break;
		}*/
    }
};
extern testResult myTestResult;

//RSL:runStepsList
/*
struct DUT_ID{
    enum ID{
        NoDUT=0
        ,ICA2405 //1
        ,ICA2315 //2
        ,ICA2308 //3 
        ,ICA2407 //4 
        ,ICA2506 //5 
        ,ICA2510 //6
        ,ICA1234 //7
        ,Unknown=9
    };
    std::string getName(ID id){
        switch(id){
            case ID::NoDUT: return "NoDUT";
            case ID::ICA2405: return "ICA2405";
            case ID::ICA2315: return "ICA2315";
            case ID::ICA2308: return "ICA2308";
            case ID::ICA2407: return "ICA2407";
            case ID::ICA2506: return "ICA2506";
            case ID::ICA2510: return "ICA2510";
            case ID::ICA1234: return "ICA1234";
            default:
                return "";
        }
    }
    std::string getCompleteName(uint8_t id){
        switch(id){
            case ID::NoDUT: return "NoDUT";
            case ID::ICA2405: return "ICA2405-NT-USV";
            case ID::ICA2315: return "ICA2315-NT-USV";
            case ID::ICA2308: return "ICA2308-LinuxBase";
            case ID::ICA2407: return "ICA2407-IBISSlave";
            case ID::ICA2506: return "ICA2506-NT-USV";
            case ID::ICA2510: return "ICA2510-NT-USV";
            case ID::ICA1234: return "ICA1234-TEST";
            default:
                return "";
        }
    }
    std::string getBoardKind(uint8_t id){
        switch(id){
            case ID::NoDUT: return "NoDUT";
            case ID::ICA2405: return "NT-CLX USV";
            case ID::ICA2315: return "NT-CLS USV";
            case ID::ICA2308: return "Linux Base";
            case ID::ICA2407: return "IBIS Slave";
            case ID::ICA2506: return "NT-CLX USV Pro";
            case ID::ICA2510: return "MB-PSU-MCU";
            case ID::ICA1234: return "TEST Board";
            default:
                return "";
        }
    }
    uint32_t getNameInt(uint8_t id){
        switch(id){
            case ID::NoDUT: return 0;
            case ID::ICA2405: return 2405;
            case ID::ICA2315: return 2315;
            case ID::ICA2308: return 2308;
            case ID::ICA2407: return 2407;
            case ID::ICA2506: return 2506;
            case ID::ICA2510: return 2510;
            case ID::ICA1234: return 1234;
            default:
                return 0;
        }
    }
    std::string getNameIDStr(uint8_t id){
        switch(id){
            case ID::ICA2405: return "2405";
            case ID::ICA2315: return "2315";
            case ID::ICA2308: return "2308";
            case ID::ICA2407: return "2407";
            case ID::ICA2506: return "2506";
            case ID::ICA2510: return "2510";
            case ID::ICA1234: return "1234";
            default:
                return "";
        }
    }
    int getBoardVersion(uint8_t id, float version){
        switch(id){
            case ID::ICA2405: {
                int __tmpVer = static_cast<int>(version * 10);
                return (((__tmpVer / 10) * 10) * 16) + (__tmpVer % 10);
            }
            break;
            case ID::ICA1234:
            case ID::ICA2506:
            case ID::ICA2510:
            case ID::ICA2315: {
                int __tmpVer2 = static_cast<int>(version * 10);
                return (__tmpVer2 / 10) * 16 + (__tmpVer2 % 10);
            }
            break;
            default:
                return 0;
        }
     }
     
};
*/
    

#endif

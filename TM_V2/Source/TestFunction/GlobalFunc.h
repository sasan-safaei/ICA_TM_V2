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

#define __const_LoadCurrent 0.850
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
    
    
    

struct boardInfo{
        float LTC3350_RSNSI1=0;
        float LTC3350_RSNSI2=0;
        float LTC3350_RSNSC=0;
        float LTC3350_RTST=0;
        float LTC3350_RT=0;
        float Board_SupperCapVoltage=0;
        int Board_SupperCapSingleCap=0;
        int Board_SupperCapNum=0;
        int Board_SupperCapType=0;
        int Board_MaxTemp85V=0;
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
	float tempIC=0,tempBatBank=0;
	float Limit_MIN_ChargeCurrent=0;
	float Limit_MIN_FullChargeCurrent=0;
	int Limit_MAX_Charge_time=0;
	int Limit_MIN_WaitToOutSwOff=0;
	int Limit_MAX_WaitToOutSwOff=0;
	int Limit_MIN_OutSwOff=0;
	int Limit_MAX_OutSwOff=0;
	float Limit_MAX_VCap_ShutdownVoltage=0;
	float Limit_MIN_VCap_ShutdownVoltage=0;
	int Board_MaxTemp85V=0;
    void clear(){
        ErrorNo=0;
	    EEPROM_Status=0;
	    Logstr='0';
	    repaired_NoCap=0;repaired_Cap=0;
	    time_charge=0;time_DisCharge=0;time_OutSwOff=0;time_WaitToOutSwOff=0;
	    Vcap_Max=0;
	    VCap_SWOff=0;
	    Load_Current=0;
	    VOut1=0;VOut2=0;
	    tempIC=0;tempBatBank=0;
	    Limit_MIN_ChargeCurrent=0;
	    Limit_MIN_FullChargeCurrent=0;
	    Limit_MAX_Charge_time=0;
		Limit_MIN_WaitToOutSwOff=0;
		Limit_MAX_WaitToOutSwOff=0;		
		Limit_MIN_OutSwOff=0;
		Limit_MAX_OutSwOff=0;
		Limit_MAX_VCap_ShutdownVoltage=0;
		Limit_MIN_VCap_ShutdownVoltage=0;
	    Board_MaxTemp85V=0;
    }
};
extern testResult testr;


    

#endif

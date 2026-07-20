// ======================================================================== 
// ICA Group
//
// Walter-Welp Str. 25-27
// 44149 Dortmund
//
// +49-231-22248-0
//
// ========================================================================
// Test Util V2
// ========================================================================
// Project   ICA2315
// Filename  usv_test_util_v2.h
// Author Sasan Safaei                                                                   
// Date      2025-04-08
// ======================================================================== 
#pragma once
#ifndef _USV_TEST_UTIL_V2_H_
#define _USV_TEST_UTIL_V2_H_


#include <iostream>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <vector>


#include "./TestFunction/ICA2315.h"
#include "./USVTestInterface/USVTestInterface.h"
#include "./TestFunction/ErrorList.h"
#include "./TestFunction/GlobalFunc.h"
#include "./TestFunction/log_resualt.h"
#include "./TestFunction/Hardware/LabDevice.h"
#include "./TestFunction/ConsoleKeyBoard.h"
#include "./TestFunction/eeprom.h"
#include "./TestFunction/ErrorList.h"
#include "main_pi.h"
// === ICA ===========================================================================================
void testrReset();


struct __temp__register{
    std::string current_RSL_Num_Str;
    uint8_t RSL_state=0;
    uint8_t RSL_Cnt=0;    
    uint8_t mState = 0;
    uint8_t m2State = 0;
    uint8_t m2ErrorCnt=0;
    uint8_t m2ErrorCntLimit=0;
    uint8_t m2ErrorNo=0;
    uint8_t key=0;
    uint16_t dcnt100ms=0;
    std::ostringstream oss;
    bool testingNoCap=false;    
    uint8_t __rKey=0;
    std::ofstream file;
    uint8_t LMState=0xFF;
    
    uint16_t __lastTimeValue=0;
    float __floatTmpVale=0;
    uint8_t __tempBatBack__error__cnt=0;
    float __tempICtempVal=0.0;
    uint8_t __tempIC__error__cnt=0;
    uint8_t __diffVcap__error__cnt=0;
    uint8_t __error_cnt=0;
    bool __uartConnection=false;
    bool __isSupperCapsOnBoard=false;
 };

enum __TestResult{
    T_AR_Off=1
    ,T_AR_On
    ,T_VCC_Test
    ,T_uC_Program
    ,T_Uart
    ,T_EEPROM
    ,T_EEPROM_I2C
    ,T_ChargeTest
    ,T_FlyBackTest
    ,T_WaitToOutSWOffTest
    ,T_DisChargeTest
    ,T_EEPROM_Uart_Save
    ,T_Just_On
};

enum FuncStatus {
    running=0
    ,success
    ,failed
};

class USV_TEST_UTIL_V2{
    public:
        std::string current_RSL_Name;
        std::vector<uint8_t> toDoList={};
        bool xrunning;
        struct MYAruments{
            //uint8_t manual_Dongle=false;               
            bool showErrorList=false;
            bool normalTest=false;
            bool manualTest=false; 
            bool showAll=false;       
            bool LabDevice_PS=false;
            bool LabDevice_Load=false;
            bool EEPROMCFG=false;
            char ttyName[25];
            //std::string ttyName;
            std::string StoreFolderName;
            std::string StoreFolderPath;
            std::string FileName_EUI;
            std::string FileName_Test;
            std::string workSpace;            
            void showHelp(){
                printf("\033[2H");
                printf("\033[2J");
                printf("Input Arguments:\n");
                printf("  -Normal/-MONITOR/-MonitorAll:  select test codition.\n");
                //printf("  -LabDevice:  enable Lab Device.\n");    
                printf("  -EEPROMCFG: just write EEPROM & read and print UID\n");
                printf("  -PORT port addr: UARt default port (/dev/ttyAMA4)\n");
                printf("  -2405/-2315: select board manualy(V1)\n");
                printf("  -ErroList: Show Error List\n");
                printf("  -WORKSPACE: path of working files\n");
                printf("\nDongle is 5-bit-GPInput [B1,B0,V2,V1,V0]\n");        
                printf("   B[2]    V[3]\n");
                printf("   B[2]=1:          ICA2405\n");
                printf("   B[2]=2:          ICA2315\n");        
                printf ("          V[3]= 0: %s",myBoard.myEEPROM.myData.getCapsKind(0).c_str());
                printf ("          V[3]= 1: %s",myBoard.myEEPROM.myData.getCapsKind(1).c_str());
                printf ("          V[3]= 2: %s",myBoard.myEEPROM.myData.getCapsKind(2).c_str());
                printf ("          V[3]= 3: %s",myBoard.myEEPROM.myData.getCapsKind(3).c_str());        
            }
        } myArg;        
        USV_Test_Interface myTestDevice;
    private:
        //ICA2315 myBoard;
        //testResult testr;
        //LabDevice MyLabDevice;
        //durationTimerClass myDurationTimer;
        struct MYTempValue{
            uint8_t result=0;
            float InCurrent=0.0;
            float LoadCurrent=0.0;
            float VOut=0.0;
            float VIn=0.0;
            float VIn_LTC3350=0.0;
            float VCap=0.0;
            float VCC=0.0;
            uint16_t time_mess=0;
            uint16_t ltime_mess=0;
            uint16_t chargeTime,DisChargeTime,WaitToOutSWOffTime,OutSWOffTime;
            bool chargingTestProgress=false;
            void clear(){
                result=0;
                InCurrent=0.0;
                LoadCurrent=0.0;
                VOut=0.0;
                VIn=0.0;
                VIn_LTC3350=0.0;
                VCap=0.0;
                VCC=0.0;
                time_mess=0;
                ltime_mess=0;
                chargeTime=0;
                DisChargeTime=0;
                OutSWOffTime=0;
                WaitToOutSWOffTime=0;
                chargingTestProgress=false;
            }
        } myTempVal;
        uint8_t Dongle=0;
        //std::string lastModifiedTime="2026.05.12";
        bool SelectBoard(uint8_t _dongle, float _version);
        /*void ShowMyName(){
            char _str[32];
            sprintf(_str,"ICA USV Test.");myTestDevice.showOnLCD(1,_str);
            myTestDevice.showOnLCD(2,lastModifiedTime);
        }*/
        std::string DongleNameStr(){
            std::stringstream ss;
                        
            if(myBoard.boardName>0){
                int __tmpVersion=myBoard.boardVerDec*100;
                if((__tmpVersion % 10)==0)
                    ss << "ICA" << myBoard.boardName << "V" << std::fixed << std::setprecision(1) << myBoard.boardVerDec;
                else
                    ss << "ICA" << myBoard.boardName << "V" << std::fixed << std::setprecision(2) << myBoard.boardVerDec;
                //ss << "ICA" << myBoard.boardName << "V" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << myBoard.boardVer;    
            }else{
                if(Dongle==0)
                    ss<<"No Dongle!";
                else
                    ss<<"unKnown Dongle!";
            }
            return ss.str();
            
        }
        
        void showOnRun(uint8_t _menuNo);
        
        //void InformationMenu(void);
        uint8_t run_GPIO_Test(void);
        void waitToPressKey(){ while(myTestDevice.readKeys()==0) usleep(100000);}
    public:
        bool Init();
        void showSelectedBoardInfo(void);        
        void showLog(std::string _str);
        void getArg(int argc, char* argv[]);
        void DongleCheck(void);
        //void run(void);
        //void run_TestMachine(void);
        //void run_TestMachine_ucProgram(uint8_t _DUT);
        //void runICA2308_simple_test(float _version=1.0);
        //void runICA2407_simple_test(void);
        //void runICA2506(void);
        void forceStop(void);
        //void run_ManualTest(void);
        uint8_t showError(uint8_t _errorNo,__temp__register & _M2);
        void checkLabDevice(void);       
        /*struct __constValue{
            float InCurrent_NoAR_MaxLimit=0.020;
            float InCurrent_AR_MinLimit=0.100;
            float VCC_minLimit=3.1;
            float VCC_maxLimit=3.6;
            
            void setDefault( uint8_t __DUT_ID)
            {
                switch(__DUT_ID){     
                    case DUT_ID::ICA2405:                        
                    case DUT_ID::ICA2506:
                        VCC_minLimit=2.85;//3.1 TEST TEST TEST
                        VCC_maxLimit=3.6;           
                    break;               
                    case DUT_ID::ICA2510:
                        InCurrent_NoAR_MaxLimit=0.030;
                        InCurrent_AR_MinLimit=0.100;
                    break;
                }
                
            }              
        } constValue;
        */
        void run_Test_Func();
        void preLoopFunc(__temp__register & _M2);
        void postLoopFunc(void);
        void preLoopGetCaps(__temp__register & _M2);        
        bool CheckCapsVoltageDiff(void);
        bool LabelPrint(void);
        int runSTM32ProgFunc(std::string _cmd, std::string &result);
        bool convertHexStrToByteArray(std::string hexStr, uint8_t* byteArray,uint16_t maxSize);
        bool convertByteArrayToHexStr(uint8_t* byteArray, size_t byteArraySize, std::string &hexStr);
        uint8_t RSL_Init(__temp__register & _M2);
        uint8_t RSL_AR_Test(__temp__register & _M2);
        uint8_t RSL_VCC_Test(__temp__register & _M2);
        uint8_t RSL_uC_Program(__temp__register & _M2);
        uint8_t RSL_UART_EEPROM(__temp__register & _M2);
        uint8_t RSL_EEPROM_RTC_I2C(__temp__register & _M2);
        uint8_t RSL_ChargeTest(__temp__register & _M2);
        uint8_t RSL_InChargeWait(__temp__register & _M2);
        uint8_t RSL_FlyBackTest(__temp__register & _M2);
        uint8_t RSL_WaitToOutSWOffTest(__temp__register & _M2);
        uint8_t RSL_DisChargeTest(__temp__register & _M2);
        uint8_t RSL_UART_Save_EEPROM(__temp__register & _M2);
        uint8_t RSL_JUST_ON(__temp__register & _M2);
        uint8_t RSL_JUST_OFF(__temp__register & _M2);
        uint8_t RSL_POWER_ON(__temp__register & _M2);
        uint8_t RSL_IBIS_LoopBackCheck(__temp__register & _M2);
        

};



#endif


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


#include "./TestFunction/ICA2315.h"
#include "./USVTestInterface/USVTestInterface.h"
#include "./TestFunction/ErrorList.h"
#include "./TestFunction/GlobalFunc.h"
#include "./TestFunction/log_resualt.h"
#include "./TestFunction/Hardware/LabDevice.h"
#include "./TestFunction/ConsoleKeyBoard.h"
#include "./TestFunction/eeprom.h"
#include "main_pi.h"
// === ICA ===========================================================================================
void testrReset();

struct runStepList {
    enum N {
        testP1
        ,testP2_InputCurrent
        ,testP3_InputCurrent
        ,testP4_EUI
        ,testP5_ChargeTime
        ,testP6_DisChargeTime
        ,testP0_END
    };
};

class USV_TEST_UTIL_V2{
    public:
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
            float VCap=0.0;
            float VCC=0.0;
            uint16_t time_mess=0;
            uint16_t ltime_mess=0;
            uint16_t chargeTime,DisChargeTime,WaitToOutSWOffTime,OutSWOffTime;

            void clear(){
                result=0;
                InCurrent=0.0;
                LoadCurrent=0.0;
                VOut=0.0;
                VIn=0.0;
                VCap=0.0;
                VCC=0.0;
                time_mess=0;
                ltime_mess=0;
                chargeTime=0;
                DisChargeTime=0;
                OutSWOffTime=0;
                WaitToOutSWOffTime=0;
            }
        } myTempVal;
        uint8_t Dongle=0;
        std::string lastModifiedTime="2025.05.22";
        
        bool SelectBoard(uint8_t _dongle, float _version);
        void ShowMyName(){
            char _str[32];
            sprintf(_str,"ICA USV Test.");myTestDevice.showOnLCD(1,_str);
            myTestDevice.showOnLCD(2,lastModifiedTime);
        }
        std::string DongleNameStr(){
            std::stringstream ss;
    
            if(myBoard.boardName>0)
            ss << "ICA" << myBoard.boardName << " V " << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << myBoard.boardVer;    
            else
                if(Dongle==0)
                    ss<<"No Dongle!";
                else
                    ss<<"unKnown Dongle!";
            return ss.str();
            
        }
        void ShowMyDongle(){
            char _str[32];
            sprintf(_str,"ICA%d V %02X ",myBoard.boardName,myBoard.boardVer);            
            showLog(_str);
            if(myBoard.myBoardInfo.Board_SupperCapType>0){
                sprintf(_str,"Cap:%d Max %.1fV ",myBoard.myBoardInfo.Board_SupperCapType,((float)myBoard.myBoardInfo.Board_MaxTemp85V)/10);            
                showLog(_str);
            }
    
        }
        void showOnRun(uint8_t _menuNo);
        
        void InformationMenu(void);
        uint8_t run_GPIO_Test(void);
        void waitToPressKey(){ while(myTestDevice.readKeys()==0) usleep(100000);}
    public:
        bool Init();
        void showLog(std::string _str);
        void getArg(int argc, char* argv[]);
        void DongleCheck(void);
        void run(void);
        void run_TestMachine(void);
        void runICA2308_simple_test(float _version=1.0);
        void runICA2407_simple_test(void);
        void runICA2506(void);
        void forceStop(void);
        void run_ManualTest(void);
        uint8_t showError(uint8_t _errorNo);
        void checkLabDevice(void);
        
};



#endif


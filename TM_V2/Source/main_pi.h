// ======================================================================== 
// ICA Group
//
// Walter-Welp Str. 25-27
// 44149 Dortmund
//
// +49-231-22248-0
//
// ========================================================================
// global function
// ========================================================================
// Project   ICA2315
// Filename  main.h
// Author Sasan Safaei                                                                   
// Date      2024-06-10
// ======================================================================== 

#ifndef _ICA_MAIN_PI_H
#define _ICA_MAIN_PI_H
#include <stdio.h>

#include <algorithm> // for std::remove
#include <string>
#include <iostream>

#include <thread>
#include <atomic>
#include <chrono>
#include <queue>

#include <sstream>
#include "unistd.h"
struct _testResualtValue{
    uint8_t currentTestNo;
    int8_t AR_On,AR_Off,UART_Con,eeprom_status,FlyBackDis,FlayBackEn,OutSwOff;
    uint16_t CapChargeTime,CapDisChargeTime,OutSWOffTime,WaitToOutSWOffTime;
    float Vin,VOut,VcapBank,Vcap1,Vcap2,Vcap3,Vcap4,Vvcc;
    float InCurrent,LoadCurrent;
    std::string EEPROMBuff_str;
    std::string UID_str;
    std::string errorStr1="";
    std::string errorStr2="";
    
    void DataClear(){        
        currentTestNo=0;
        AR_On=-1; AR_Off=-1;
        UART_Con=-1; eeprom_status=-1;
        FlyBackDis=-1; FlayBackEn=-1; OutSwOff=-1;
        CapChargeTime=0; CapDisChargeTime=0; OutSWOffTime=0; WaitToOutSWOffTime=0;
        Vin=0; VOut=0; VcapBank=0;
        Vcap1=0; Vcap2=0; Vcap3=0; Vcap4=0;
        Vvcc=0;
        InCurrent=0; LoadCurrent=0;
        UID_str="";
        EEPROMBuff_str="";
        errorStr1="";
        errorStr2="";
    }
    std::string show_str(){
        std::ostringstream oss;
        oss <<"TestNo:" << static_cast<int>(currentTestNo);
        return oss.str();
    }
    std::string __show_str(){
        std::ostringstream oss;
        oss <<"TestNo:" << static_cast<int>(currentTestNo) <<" ,"
        <<"AR_On:"<<static_cast<int>(AR_On) <<" ,"
        <<"AR_Off:"<<static_cast<int>(AR_Off) <<" ,"
        <<"UART_Con:"<<static_cast<int>(UART_Con) <<" ,"
        <<"eeprom_status:"<<static_cast<int>(eeprom_status) <<" ,"
        <<"FlyBackDis:"<<static_cast<int>(FlyBackDis) <<" ,"
        <<"FlayBackEn:"<<static_cast<int>(FlayBackEn) <<" ,"
        <<"OutSwOff:"<<static_cast<int>(OutSwOff) <<" ,"
        <<"CapChargeTime:"<<static_cast<int>(CapChargeTime) <<" ,"
        <<"CapDisChargeTime:"<<static_cast<int>(CapDisChargeTime) <<" ,"
        <<"OutSWOffTime:"<<static_cast<int>(OutSWOffTime )<<" ,"
        <<"Vin:"<<static_cast<float>(Vin) <<" ,"
        <<"VOut:"<<static_cast<float>(VOut) <<" ,"
        <<"VcapBank:"<<static_cast<float>(VcapBank) <<" ("
        <<static_cast<float>(Vcap1) <<" ,"<<static_cast<float>(Vcap2) <<" ,"<<static_cast<float>(Vcap3) <<" ,"<<static_cast<float>(Vcap4) <<" )"
        <<"Vvcc:"<<static_cast<float>(Vvcc) <<" ,"
        <<"InCurrent:"<<static_cast<float>(InCurrent) <<" ,"
        <<"LoadCurrent:"<< static_cast<float>(LoadCurrent) <<" ,"
        <<"UID_str:"<<UID_str <<" ,"
        <<"EEPROMBuff_str:"<<EEPROMBuff_str <<" ,"
        <<"errorStr:"<<errorStr1 << " "<<errorStr2 <<" .";
        return oss.str();
    }
};
const size_t MAX_SIZE_fifo_csLog = 20;
struct _msg_box{
    int32_t counter=0;
    uint16_t timeOutCnt=0;        
    int32_t resualt=0;
    std::string msg;
    std::string btnYesTxt;
    std::string btnNoTxt;
    void clear(){
        msg="";
        btnYesTxt="";
        btnNoTxt="";
    }
    void setResualt(int32_t _resualt){
        resualt=_resualt;
    }
    bool waitForUser(std::string _msg,std::string _yesTxt,std::string _noTxt, uint16_t _timeOutSec){
        uint16_t ms100=0;
        timeOutCnt=60*60;        
        msg=_msg;
        btnYesTxt=_yesTxt;
        btnNoTxt=_noTxt;
        resualt=0;
        if(_timeOutSec>0)
            timeOutCnt=_timeOutSec;
        counter++;        
        while(resualt==0 && timeOutCnt>0){
            usleep(100000);
            if(++ms100>=10){
                ms100=0;
                timeOutCnt--;
            }
        }
        if(resualt==1)
            return true;
        return false;
    }
    
};

struct _interact_registers{
    bool running_status = false;
    uint32_t gui_CMD=0;

    uint32_t tm_state =0;
    std::queue<std::string> fifo_csLog;
    
    std::string Dongle="";
    uint32_t DongleID=0;
    float board_version=0.0;
    _msg_box msgBox;
    _testResualtValue TR;
    
    uint8_t getGuiCMD(){
        uint8_t tmp= (uint8_t)gui_CMD;
        gui_CMD=0;
        return tmp;
    }
    // Push function: adds a new message to the queue
    void csLogWrite(const std::string& message) {
        if (fifo_csLog.size() >= MAX_SIZE_fifo_csLog) {
            fifo_csLog.pop(); // discard oldest entry if full
        }
        fifo_csLog.push(message);
    }
    // Pull function: reads and removes the oldest message
    std::string csLogRead() {
        if (!fifo_csLog.empty()) {
            std::string front = fifo_csLog.front();
            fifo_csLog.pop();
            return front;
        } else {
            return "";
        }
    }

};


int usv_test_util(int argc, char* argv[]);

extern _interact_registers myInterActReg;  
class App_TM_V2 {
    public:
        bool initialize(int argc, char* argv[]);
        void stop();
        void run();
        void pre_Check();
        std::string send(const std::string& msg);
        std::string receive();
        App_TM_V2(): running(false) {}
        void task_run();
        void task_stop();
        // Destructor ensures clean exit
        ~App_TM_V2() {
            stop();
            //task_stop();
        }

        private:
        std::atomic<bool> running;
        std::thread worker;
        // The background task
        void taskLoop();
};
extern App_TM_V2 myApp;

#endif // _ICA_MAIN_PI_H

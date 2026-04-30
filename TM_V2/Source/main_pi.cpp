#include "main_pi.h"
_interact_registers myInterActReg;
App_TM_V2 myApp;
#include "./TestFunction/Hardware/LabDevice.h"
#include "./TestFunction/ICA2315.h"
#include "./TestFunction/myNet.h"    
//#include "./TestFunction/ConsoleKeyBoard.h"
        
char ttyName[25];

ICA2315 myBoard;
LabDevice MyLabDevice;
durationTimerClass myDurationTimer;
testResult myTestResult;
myNet myTcpUdpNet;
std::string lastModifiedTime="2025.03.18";
//ConsoleKeyClass myCKey;
#include "usv_test_util.h"
//#include "./TestFunction/usv_test_util.h"
USV_TEST_UTIL_V2 myUSVTestV2;

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include <algorithm> // for std::remove
#include <string>
#include <iostream>
#include <thread>
#include <system_error>
    
int create_folder(const char *path) {
    struct stat st = {0};

    // Check if directory exists
    if (stat(path, &st) == -1) {
        // Create directory with read/write/search permissions for owner
        if (mkdir(path, 0755) != 0) {
            perror("mkdir failed");
            return -1;
        }
    }
    return 0;
}
bool getConfig(){

    std::string configFile = myUSVTestV2.myArg.workSpace+"config.cfg";
    std::ifstream  file(configFile);
    std::string line;

    sprintf(myUSVTestV2.myArg.ttyName,"/dev/ttyAMA3");
    myUSVTestV2.myArg.StoreFolderName="Default";
    myUSVTestV2.myArg.FileName_EUI="ICA-NT-EUI.csv";
    myUSVTestV2.myArg.FileName_Test="ICA-NT-TEST.csv";

    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << configFile << std::endl;
        return 1;
    }
    while (std::getline(file, line)) {
        // Remove any leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
        //std::cout << "LineXXX: " << line << std::endl;
    
        // Check if line starts with "StoreFolder="
        //const std::string key = "StoreFolder=";
        if (line.rfind("StoreFolder=", 0) == 0) myUSVTestV2.myArg.StoreFolderName= line.substr(12)+"/";
        if (line.rfind("Port=", 0) == 0) sprintf(myUSVTestV2.myArg.ttyName,"%s",line.substr(5).c_str());
    }
    file.close();
    printf("\nConfig Value:\n");
    std::cout << "  ttyName = " << myUSVTestV2.myArg.ttyName << std::endl;
    std::cout << "  StoreFolder = " << myUSVTestV2.myArg.StoreFolderName << std::endl;
    printf("\n");
    
    return true;
}


bool App_TM_V2::initialize(int argc, char* argv[]){
    
    if (create_folder("./tmp") != 0) {
        return 1; // Handle error
    }
    myUSVTestV2.getArg(argc, argv);
    getConfig();
    myUSVTestV2.myArg.StoreFolderPath=myUSVTestV2.myArg.workSpace+myUSVTestV2.myArg.StoreFolderName;
    if (create_folder( myUSVTestV2.myArg.StoreFolderPath.c_str()) != 0) {
        return 1; // Handle error
    }

    if(myUSVTestV2.myArg.showErrorList){
        std::cout<<"SHOW ERROR LIST:\n";
        for (int i = 1; i < ERROR::END; ++i){
            myUSVTestV2.showError(i);            
        }
        return 0;
    }
    if(argc>1)
        return true;
    else   
        myUSVTestV2.myArg.showHelp();
    
        myUSVTestV2.Init();
    return false;
}
void App_TM_V2::run() {
    if (!running){
        worker = std::thread(&App_TM_V2::taskLoop, this);
        running = true;
    }
    else {
        std::cerr << "Task is already running.\n";
    }
}
void App_TM_V2::stop() {
    myUSVTestV2.xrunning = false;
}
void App_TM_V2::pre_Check() {           
    myUSVTestV2.DongleCheck();

}

std::string App_TM_V2::send(const std::string& msg){
    std::ostringstream oss;
    oss << "TM3) status:" << myInterActReg.running_status<< ", M_State:" << myInterActReg.tm_state<<"\n" ;
    return oss.str();
}
std::string App_TM_V2::receive(){
    return " No Data .....";
}

void App_TM_V2::taskLoop() {
        pthread_setname_np(pthread_self(), "Test-Loop-Task");
        myInterActReg.TR.DataClear();
        myUSVTestV2.Init();
        myUSVTestV2.xrunning = true;
        myBoard.init(myUSVTestV2.myArg.ttyName);
        int i=0;
        //myUSVTestV2.myTestDevice.cleanLCD();    
        //while (running.load()) {
        
        myInterActReg.running_status=true;//myUSVTestV2.xrunning;        
        std::string __str= myInterActReg.Dongle+".csv";
        __str.erase(std::remove(__str.begin(), __str.end(), ' '), __str.end());
        myUSVTestV2.myArg.FileName_EUI = __str;
        switch(myInterActReg.gui_CMD){
            case ICA_NT_USV_2315://1//ICA2315 
            case ICA_NT_USV_2405://2//ICA2405
                if (myInterActReg.gui_CMD==ICA_NT_USV_2315) myUSVTestV2.constValue.setDefault();
                if (myInterActReg.gui_CMD==ICA_NT_USV_2405) myUSVTestV2.constValue.setDefault();
                
                myUSVTestV2.showLog((std::ostringstream{} << "run_TestMachin!!! (" << myInterActReg.gui_CMD << ")").str());
                myUSVTestV2.checkLabDevice();sleep(.5);                    
                myUSVTestV2.run_TestMachine();
                break;
            case ICA_2506://5//ICA2506
            case ICA_2510://6 //ICA2510
                myUSVTestV2.showLog((std::ostringstream{} << "run_TestMachin + uC!!! (" << myInterActReg.gui_CMD << ")").str());
                myUSVTestV2.checkLabDevice();sleep(.5); 
                if (myInterActReg.gui_CMD==ICA_2506) {
                    myUSVTestV2.constValue.setDefault();
                    myUSVTestV2.toDoList = myUSVTestV2.toDO_ICA2506;
                }
                if (myInterActReg.gui_CMD==ICA_2510) {
                    myUSVTestV2.constValue.setFor2510();
                    myUSVTestV2.toDoList = myUSVTestV2.toDO_ICA2510;
                }
                //myUSVTestV2.run_TestMachine_ucProgram(myInterActReg.gui_CMD);
                myUSVTestV2.run_Test_Func();
            break;            
            case 3:            
                myUSVTestV2.showLog("ICA2308 Test Function");                   
                myUSVTestV2.runICA2308_simple_test(myInterActReg.board_version);       
                break;
            case 4:
                myUSVTestV2.showLog("ICA2407 Test Function");                     
                myUSVTestV2.runICA2407_simple_test();
                break;
            default:
                myUSVTestV2.showLog((std::ostringstream{} << "unKnown Dongle Num (guiCMD)!!! (" << myInterActReg.gui_CMD << ")").str());
                break;

        }
            //myUSVTestV2.run();
            myInterActReg.tm_state+=1;
            //std::cout << "Task running...\n";
            //std::this_thread::sleep_for(std::chrono::seconds(1));
        //}
        myUSVTestV2.xrunning = false;
        std::cout << "Task stopped.\n";
        // If this function is running on the worker thread, detach and clear
        // the `worker` member so the std::thread object is not left joinable
        // after the thread function returns.
        //if (worker.joinable() && std::this_thread::get_id() == worker.get_id()) {
        try {
            worker.detach();
            running = false;
        } catch (const std::system_error& e) {
            std::cerr << "Failed to detach worker: " << e.what() << std::endl;
        }
        worker = std::thread();
        //}
        //myInterActReg.TR.DataClear();
        myInterActReg.running_status=false;
        myUSVTestV2.showLog("End Of taskLoop!!!");     

}

/*int main(int argc, char* argv[]){
    
    if (myApp.initialize(argc,argv))
        myApp.run();
    return 0;
}*/

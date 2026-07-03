//#include "ICA2506_Flash_Micro.h"
//#include "ICA2506_UART.h"
#include "Stm32G030Class.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <limits.h>


int main(int argc, char* argv[]) {
    // Resolve paths relative to the executable location so the program
    // can be started from any working directory.
    char exePath[PATH_MAX] = {0};
    std::string exeDir = ".";
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len != -1) {
        exePath[len] = '\0';
        std::string fullPath(exePath);
        auto pos = fullPath.find_last_of('/');
        if (pos != std::string::npos)
            exeDir = fullPath.substr(0, pos);
    }

    std::string cfg_interface = exeDir + "/OpenOCD_CFG/raspberrypi2-native.cfg";
    std::string cfg_target = exeDir + "/OpenOCD_CFG/stm32g0x.cfg";

    STM32G030F6_Class stm32(cfg_interface.c_str(), cfg_target.c_str(), 
                                0x08000000, 100, "/dev/ttyAMA1", 9600);
    
    //STM32G030F6_Class stm32("interface/raspberrypi2-native.cfg", "target/stm32g0x.cfg", 
    //                            0x08000000, 50, "/dev/ttyAMA1", 9600);
    //stm32.Flash_Func({"--ob"});
    //stm32.Flash_Func({"--reset"});    
    //return stm32.Flash_Func(argc, argv);
    return stm32.Flash_Func(argc, argv);
}

//#include "ICA2506_Flash_Micro.h"
//#include "ICA2506_UART.h"
#include "Stm32G030Class.h"
#include <cstdlib>
#include <iostream>
#include <string>


int main(int argc, char* argv[]) {
    //STM32G030F6_Class stm32("./OpenOCD_CFG/raspberrypi2-native.cfg", "./OpenOCD_CFG/stm32g0x.cfg", 
    //                            0x08000000, 50, "/dev/ttyAMA1", 9600);
    
    STM32G030F6_Class stm32("interface/raspberrypi2-native.cfg", "target/stm32g0x.cfg", 
                                0x08000000, 50, "/dev/ttyAMA1", 9600);
    //stm32.Flash_Func({"--ob"});
    //stm32.Flash_Func({"--reset"});    
    //return stm32.Flash_Func(argc, argv);
    return stm32.Flash_Func(argc, argv);
}

//#include "ICA2506_Flash_Micro.h"
//#include "ICA2506_UART.h"
#include "Stm32G030Class.h"
#include <cstdlib>
#include <iostream>
#include <string>


int main(int argc, char* argv[]) {
    STM32G030F6_Class stm32;
    stm32.Flash_Func(argc, argv);    
}

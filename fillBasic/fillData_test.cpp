#include "fillBasic.h"

int main(int argc, char* argv[])
{
    fillBasic fb;
    std::cout << "\n\nfill arm Basic -----------------------------------" << std::endl;
    std::vector<arm_Basic> myARM;
    fb.getBasicData("./config.cfg","ARM",myARM);
    for(auto& item : myARM){ item.show(); }
    std::cout << "\n\nfill P26 Basic -----------------------------------" << std::endl;
    std::vector<p26_Basic> myP26;
    fb.getBasicData("./config.cfg","P26",myP26);
    for(auto& item : myP26){ item.show(); }
    std::cout << "\n\nfill S42 Basic -----------------------------------" << std::endl;
    std::vector<s42_Basic> myS42;
    fb.getBasicData("./config.cfg","S42",myS42);
    for(auto& item : myS42){ item.show(); }

}

#ifndef LABDEVICE_H
#define LABDEVICE_H

#include "../mySerialPort.h"

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>

class LabDevice
{
    private:
        SerialPort spPSupply;
        SerialPort spLoad;
        char DCMD[50];
        std::vector<std::string> listSerialPorts();
        bool configurePort(int fd);
        std::string findCorrectSerialPort(const std::string& testFrame, const std::string& expectedResponse);
    public:
        bool PSupply_INI(char* ttyName,int bps);
        bool Load_INI(char* ttyName, int bps);
        void setAufruesten(bool _en);
        void SetPSEnable(bool _en);
        void SetPSVoltage(float _value);
        void SetPSCurrent(float _value);
        bool SetPSOCP(bool _en);
        void SetLoadCurrent(float _value);
        float ReadPSCurrent();
        
        float ReadLoadVoltage();
        float ReadLoadCurrent();
        void PSupply_Close() {spPSupply.sp_close();}
        void Load_Close(){spLoad.sp_close();}
};

#endif



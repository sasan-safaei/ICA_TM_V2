#ifndef __EEPROM_H
#define __EEPROM_H

#include <iostream>
#include <ctime>
#include <string.h>
#include "GlobalFunc.h"

#include <sstream>
#include <iomanip>



#define ICA_CapType_2405_1 0
#define ICA_CapType_2405_2 1
#define ICA_CapType_2315_1 2
#define ICA_CapType_2315_2 3

#define EEPROM_LastVersion 3
#define EEPROM_I2C_ADDRESS  0x50  // I2C address of 24AA025E64
#define EUI64_ADDRESS       0xF8  // Starting address of EUI-64 in EEPROM
#define EEPROMDateBufferAdd 0x00
#define EEPROMDateBufferLen 4
// Define CRC-16 parameters (e.g., CRC-16-CCITT)
#define CRC_POLYNOMIAL 0x1021  // Polynomial used for CRC-16-CCITT
#define CRC_INITIAL    0xFFFF  // Initial value

enum EEPROMProcesSts{ Ok=1, Failed, ConvertedToLastVer, EmptyEEPROM, WrongIC};
// Static check for the size of time_t
constexpr size_t time_t_size = sizeof(time_t);
#if time_t_size == 8
    #define time_sizeX64
#endif

struct struct_eepromData{
    uint8_t name[3]={'-','-','-'};
    uint8_t EUI[8]={0,0,0,0,0,0,0,0};
    uint16_t boardType=0;
    uint16_t boardVer=0 ;
    uint8_t dataVersion=0;
    time_t testTime;
    uint8_t RSNSI1=0,RSNSI2=0,RSNSC=0;
    uint8_t RTST=0,RT=0;
    uint8_t CapNo=0;
    uint8_t CapType=0xff;
    uint8_t Temp85MaxV=0;
    uint8_t capacitorValue=0;
    float VshutDown=0,supperCapVoltage=0;
    void clear(){
        name[0] = '-'; name[1] = '-'; name[2] = '-';
        for (int i = 0; i < 8; ++i) EUI[i] = 0;
        boardType=0;
        boardVer=0 ;
        dataVersion=0;
        testTime;
        RSNSI1=0;RSNSI2=0;RSNSC=0;
        RTST=0;RT=0;
        CapNo=0;
        CapType=0xff;
        Temp85MaxV=0;
        capacitorValue=0;
        VshutDown=0;supperCapVoltage=0;
    }
    std::string getCapsKind(uint8_t _CapType ){
        char _result[128];
        switch(_CapType){
            case ICA_CapType_2405_1: sprintf(_result," (0) 2405-(Vishay- MAL223551008E3  50F 3V 18x35mm)\n");break;
            case ICA_CapType_2405_2: sprintf(_result," (1) 2405-(MaxWell BCAP0050P300X11 50F 3V 18x41mm)\n");break;
            case ICA_CapType_2315_1: sprintf(_result," (2) 2315-(MaxWell BCAP0150P300S17 150F 3V 25x50mm)\n");break;
            case ICA_CapType_2315_2: sprintf(_result," (3) 2315-(CDA     CXP-3R0157T-TW  150F 3V 23x58mm)\n");break;
            default: sprintf(_result, " (\"<< %d <<\")unKnown\n",_CapType);    
        }
        return _result;
    }
    std::string getEUI5Byte_Str(){
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') 
            << std::setw(2) << static_cast<int>(EUI[3])
            << std::setw(2) << static_cast<int>(EUI[4])
            << std::setw(2) << static_cast<int>(EUI[5])
            << std::setw(2) << static_cast<int>(EUI[6])
            << std::setw(2) << static_cast<int>(EUI[7]);
        return oss.str();
    }
    std::string getEUI_Str(){
        std::ostringstream oss;
        oss << "\nEUI : [" 
            << std::hex << std::uppercase <<std::setw(2) << std::setfill('0') << static_cast<int>(EUI[0]) << "."
            << std::setw(2) << static_cast<int>(EUI[1]) << "."
            << std::setw(2) << static_cast<int>(EUI[2]) << "]"
            << std::setw(2) << static_cast<int>(EUI[3]) << "."
            << std::setw(2) << static_cast<int>(EUI[4]) << "."
            << std::setw(2) << static_cast<int>(EUI[5]) << "."
            << std::setw(2) << static_cast<int>(EUI[6]) << "."
            << std::setw(2) << static_cast<int>(EUI[7]) << "\n";
        return oss.str();
    }
    std::string getAll_Str(){
        std::ostringstream oss;
        oss << ".----------------------------------------\n";
        oss << "| Data Struct (ver." << static_cast<int>(dataVersion) << "):\n";
        oss << "|  name: " << name[0] << name[1] << name[2] << "\n";
        oss << "|  Board: " << boardType << " V " << std::setw(2) << std::setfill('0') << std::hex << boardVer << std::dec << "\n";
        oss << "|  EUI : [" 
            << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(EUI[0]) << "."
            << std::setw(2) << static_cast<int>(EUI[1]) << "."
            << std::setw(2) << static_cast<int>(EUI[2]) << "]"
            << std::setw(2) << static_cast<int>(EUI[3]) << "."
            << std::setw(2) << static_cast<int>(EUI[4]) << "."
            << std::setw(2) << static_cast<int>(EUI[5]) << "."
            << std::setw(2) << static_cast<int>(EUI[6]) << "."
            << std::setw(2) << static_cast<int>(EUI[7]) << ".\n";
        oss << std::nouppercase;
        std::tm *ttTime = std::localtime(&testTime); // Convert to local time
        oss << "|  SaveTime: " 
            << std::dec
            << (ttTime->tm_year + 1900) << "-" 
            << (ttTime->tm_mon + 1) << "-" 
            << ttTime->tm_mday << " " 
            << ttTime->tm_hour << ":" 
            << ttTime->tm_min << "\n";
        
        oss << "|  RSNSI1/2: " << static_cast<int>(RSNSI1) << " mOhm  / " << static_cast<int>(RSNSI2) << " mOhm \n";
        oss << "|  RSNSC: " << static_cast<int>(RSNSC) << " mOhm  \n";
        oss << "|  RTST: " << static_cast<int>(RTST) << " Ohm \n";
        oss << "|  RT: " << static_cast<int>(RT) << " KOhm \n";
        oss << "|  Caps: " << static_cast<int>(CapNo) << "x " << static_cast<int>(capacitorValue) << "F " << std::fixed << std::setprecision(1) << supperCapVoltage << "V \n";
        oss << "|  Caps-Type: ";
        oss << getCapsKind(CapType);
        
        
        oss << "|  MaxTemp85V: ";
        if(Temp85MaxV>0) 
            oss << static_cast<float>((float)Temp85MaxV/10) << "V\n";
        else
            oss <<"("<< static_cast<float>((float)Temp85MaxV/10) << ") unKnown\n";
        oss << "|  V-ShutDown: " << VshutDown << "V \n";
        oss << ".----------------------------------------\n";

        return oss.str();
    }
    std::string getAll_StrExpress(){
        std::ostringstream oss;
        oss << "Express Buffer Show\n";
        oss << ".----------------------------------------\n";
        std::tm *ttTime = std::localtime(&testTime); // Convert to local time
        oss << "| Data Struct (ver." << static_cast<int>(dataVersion) << ") ";
        oss << "|  Saved: " 
            << std::dec
            << (ttTime->tm_year + 1900) << "-" 
            << (ttTime->tm_mon + 1) << "-" 
            << ttTime->tm_mday << " " 
            << ttTime->tm_hour << ":" 
            << ttTime->tm_min << ":";
        oss << "|  name: " << name[0] << name[1] << name[2] << " "<< boardType << " V " << std::setw(2) << std::setfill('0') << std::hex << boardVer << std::dec << "\n";
        oss << "|  EUI : [" 
            << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(EUI[0]) << "."
            << std::setw(2) << static_cast<int>(EUI[1]) << "."
            << std::setw(2) << static_cast<int>(EUI[2]) << "]"
            << std::setw(2) << static_cast<int>(EUI[3]) << "."
            << std::setw(2) << static_cast<int>(EUI[4]) << "."
            << std::setw(2) << static_cast<int>(EUI[5]) << "."
            << std::setw(2) << static_cast<int>(EUI[6]) << "."
            << std::setw(2) << static_cast<int>(EUI[7]) << ".\n";
        oss << std::nouppercase;
        
        
        oss << "|  RSNSI1/2,RSNSC: " << static_cast<int>(RSNSI1) << " mOhm  / " << static_cast<int>(RSNSI2) << " mOhm ";
        oss << ", " << static_cast<int>(RSNSC) << " mOhm  \n";
        oss << "|  RTST/RT: " << static_cast<int>(RTST) << " Ohm ";
        oss << "/ " << static_cast<int>(RT) << " KOhm \n";
        oss << "|  Caps: " << static_cast<int>(CapNo) << "x " << static_cast<int>(capacitorValue) << "F " << std::fixed << std::setprecision(1) << supperCapVoltage << "V ";
        oss << " ";
        oss << getCapsKind(CapType);
        
        
        oss << "|  MaxTemp85V/: V-ShutDown";
        if(Temp85MaxV>0) 
            oss << static_cast<float>((float)Temp85MaxV/10) << "V";
        else
            oss <<"("<< static_cast<float>((float)Temp85MaxV/10) << ") unKnown";
        oss << " / " << VshutDown << "V \n";
        oss << ".----------------------------------------";

        return oss.str();
    }
    
    void show(){
        std::cout<<getAll_Str();
    }
};
class eeprom {
private:
    const unsigned char microChip_ID[3]={0x00,0x04,0xA3};        
public:
    struct ConvertV2toV3{
        uint8_t CapType=0xFF;
        uint8_t Temp85MaxV=0;
    }convertV2toV3;
    struct_eepromData myData;
    uint8_t EEPROMDataBuffer[EEPROMDateBufferLen*8];
private:
    uint16_t calculate_crc16(const uint8_t *data, size_t length);
    uint8_t bcd_to_decimal(uint8_t bcd);    
    bool isEmpty();
    // read version of saving data from EEPROMDataBuffer
    uint8_t getVersion();
    //Copy ver1.Data from EEPROMDataBuffer to myData struct
    void getVer1Data();
    //Copy ver2.Data from EEPROMDataBuffer to myData struct
    bool getVer2Data();
    public:	
    bool ismicroChipIC();
    void ShowEUI();
    void EEPROMDataBuffShow();
    void clear_EEPROM_Buffer();
    //Copy Last-ver.Data from EEPROMDataBuffer to myData struct
    bool getLastVerData();
    //Updata EEPROMDataBuffer from LastVersion DataStruct
    void BuffUpdate_LVer();
    // Update myData struct from Current Time
    void getCurrentTime();
    // Update myData struct from Arquments
    void updateBoardInfo(int _boardName, int _boardVer, boardInfo _dataBI, testResult _tr);
    uint8_t CheckDataVersionProcess(bool debugShow);  
};

#endif

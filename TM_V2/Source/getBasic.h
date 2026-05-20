// ========================================================================
// ICA Group
// ========================================================================
// Project   ICA_TM_V2
// Filename  getBasic.h
// Author    Sasan Safaei
// Date      2026-05-19
// ========================================================================
// Reads config.cfg and exposes typed settings to the rest of the app.
// ========================================================================

#ifndef ICA_GET_BASIC_H
#define ICA_GET_BASIC_H

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
struct RSL_struct{
    enum RSL {
    Init=0
    ,AR_Test
    ,VCC_Test
    ,uC_Program
    ,Uart_EEPROM
    ,uart_EEPROM_RTC_I2C
    ,uart_EEPROM_Save
    ,ChargeTest
    ,FlyBackTest
    ,WaitToOutSWOffTest
    ,DisChargeTest                
    ,EndSuccess
    ,EndFailed
    ,justOn
    ,Stop
    };
    std::string getRSLStr(uint8_t _step){
        switch(_step){
            case Init: return "Init";
            case AR_Test: return "AR_Test";
            case VCC_Test: return "VCC_Test";
            case uC_Program: return "uC_Program";
            case Uart_EEPROM: return "Uart_EEPROM";
            case uart_EEPROM_RTC_I2C: return "uart_EEPROM_RTC_I2C";
            case uart_EEPROM_Save: return "uart_EEPROM_Save";
            case ChargeTest: return "ChargeTest";
            case FlyBackTest: return "FlyBackTest";
            case WaitToOutSWOffTest: return "WaitToOutSWOffTest";
            case DisChargeTest: return "DisChargeTest";                
            case EndSuccess: return "EndSuccess";                
            case EndFailed: return "EndFailed";                
            case justOn: return "justOn";
            case Stop: return "Stop";                
            default:
                return "XX";
        }
     }
};

// ---- per-DUT entry read from [DUT_LIST] --------------------------------
struct boardInfo_struct{
        float LTC3350_RSNSI1=0;
        float LTC3350_RSNSI2=0;
        float LTC3350_RSNSC=0;
        float LTC3350_RTST=0;
        float LTC3350_RT=0;
        float Board_SupperCapVoltage=0;
        int Board_SupperCapSingleCap=0;
        int Board_SupperCapNum=0;
        int Board_SupperCapType=0;
        int Board_MaxTemp85V=0;
		float Board_VShutdownVoltage=0;
        std::string toString() const {
        std::ostringstream oss;
        oss << "BardInfo:\n"
            << "   LTC3350_RSNSI1: " << LTC3350_RSNSI1 << " mOhm, \n"
            << "   LTC3350_RSNSI2: " << LTC3350_RSNSI2 << " mOhm, \n"
            << "   LTC3350_RSNSC: "  << LTC3350_RSNSC  << " mOhm, \n"
            << "   LTC3350_RTST: "   << LTC3350_RTST   << " Ohm, \n"
            << "   LTC3350_RT: "     << LTC3350_RT     << " KOhm, \n"
            << "   B_SCapVoltage: " << Board_SupperCapVoltage << " V, \n"
            << "   B_SCapSingleCap: " << Board_SupperCapSingleCap << " F, \n"
            << "   B_SCapNum: "      << Board_SupperCapNum      << ", \n"
            << "   B_SCapType: "     << Board_SupperCapType     << ", \n"
            << "   B_MaxTemp85V: "       << Board_MaxTemp85V       << ", \n"
            << "   B_VShutdown: " << Board_VShutdownVoltage << " V";
            return oss.str();
        }
};
struct DutEntry {
    int id = 0;         // e.g. 1
    std::string name;     // e.g. "ICA2405"
    std::string version;  // e.g. "1.61"
    std::string FullName; // e.g. "ICA2405A6-vishay"
    boardInfo_struct boardInfo; 
    std::vector<RSL_struct::RSL> toDoList;
};

// ---- all settings loaded from one config.cfg ---------------------------
struct BasicConfig {
    // [TM_DEVICE]
    std::string devicePort;        // e.g. /dev/ttyAMA1
    std::string storeFolder;       // e.g. ./TestMachine001
    // [DUT_LIST]
    std::vector<DutEntry> dutList;
};

// ---- reader class ------------------------------------------------------
class CfgReader {
public:
    CfgReader();

    // Load (or reload) a config file.  Returns true on success.
    bool load(const std::string& filePath);

    // Parsed result – valid after a successful load().
    const BasicConfig& config() const { return m_cfg; }

    // Convenience helpers
    std::string getDevicePort()   const { return m_cfg.devicePort; }
    std::string getStoreFolder()  const { return m_cfg.storeFolder; }
    const std::vector<DutEntry>& getDutList() const { return m_cfg.dutList; }
    void showAllConfig();
private:
    BasicConfig m_cfg;

    // Internal helpers
    static std::string trim(const std::string& s);
    static std::vector<std::string> splitCsv(const std::string& s, char delim = ',');
    static bool parseRslStep(const std::string& token, RSL_struct::RSL& outStep);
    void parseDeviceLine(const std::string& line);
    void parseDutListLine(const std::string& line, int& nextDutId);
    void parseBoardInfoLine(const std::string& line, boardInfo_struct& info);
    void parseToDoListLine(const std::string& line, std::vector<RSL_struct::RSL>& toDoList);
};




#endif // ICA_GET_BASIC_H

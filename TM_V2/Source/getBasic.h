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
    ,EEPROM_RTC_I2C
    ,uart_EEPROM_Save
    ,ChargeTest
    ,FlyBackTest
    ,WaitToOutSWOffTest
    ,DisChargeTest  
    ,EndSuccess
    ,EndFailed
    ,justOn
    ,powerOn
    ,IBIS_LoopBackCheck
    ,Stop
    };
    std::string getRSLStr(uint8_t _step){
        switch(_step){
            case Init: return "Init";
            case AR_Test: return "AR_Test";
            case VCC_Test: return "VCC_Test";
            case uC_Program: return "uC_Program";
            case Uart_EEPROM: return "Uart_EEPROM";
            case EEPROM_RTC_I2C: return "EEPROM_RTC_I2C";
            case uart_EEPROM_Save: return "uart_EEPROM_Save";
            case ChargeTest: return "ChargeTest";
            case FlyBackTest: return "FlyBackTest";
            case WaitToOutSWOffTest: return "WaitToOutSWOffTest";
            case DisChargeTest: return "DisChargeTest";                
            case EndSuccess: return "End-Success";                
            case EndFailed: return "End-Failed";                
            case justOn: return "justOn";
            case powerOn: return "powerOn";
            case IBIS_LoopBackCheck: return "IBIS_LoopBackCheck";
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
            std::string Board_SupperCapName;
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
                << "   B_SCapType: "     << Board_SupperCapType
                << (Board_SupperCapName.empty() ? "" : (" (" + Board_SupperCapName + ")"))
                << ", \n"
            << "   B_MaxTemp85V: "       << (float)(Board_MaxTemp85V)/10       << ", \n"
            << "   B_VShutdown: " << Board_VShutdownVoltage << " V\n";
            return oss.str();
        }
};
struct measurementPoint_struct{
        float InCurrent_NoAR_MaxLimit=0.020;
        float InCurrent_AR_MinLimit=0.100;
        float VCC_minLimit=3.1;
        float VCC_maxLimit=3.6;
        float Load_Current=0;
        float Limit_MIN_ChargeCurrent=0;
        float Limit_MIN_FullChargeCurrent=0;
        int Limit_MAX_Charge_time=0;
        int Limit_MIN_WaitToOutSwOff=0;
        int Limit_MAX_WaitToOutSwOff=0;
        int Limit_MIN_OutSwOff=0;
        int Limit_MAX_OutSwOff=0;
        float Limit_MIN_VCap_ShutdownVoltage=0;
        float Limit_MAX_VCap_ShutdownVoltage=0;
        std::string toString() const {
        std::ostringstream oss;
        oss << "MeasurementPoint:\n"
            << "   InCurrent_NoAR_MaxLimit: " << InCurrent_NoAR_MaxLimit << " A, \n"
            << "   InCurrent_AR_MinLimit: " << InCurrent_AR_MinLimit << " A, \n"
            << "   VCC_minLimit: " << VCC_minLimit << " V, \n"
            << "   VCC_maxLimit: " << VCC_maxLimit << " V, \n"
            << "   Load_Current: " << Load_Current << " A, \n"
            << "   Limit_MIN_ChargeCurrent: " << Limit_MIN_ChargeCurrent << " A, \n"
            << "   Limit_MIN_FullChargeCurrent: " << Limit_MIN_FullChargeCurrent << " A, \n"
            << "   Limit_MAX_Charge_time: " << Limit_MAX_Charge_time << " sec, \n"
            << "   Limit_MIN_WaitToOutSwOff: " << Limit_MIN_WaitToOutSwOff << " sec, \n"
            << "   Limit_MAX_WaitToOutSwOff: " << Limit_MAX_WaitToOutSwOff << " sec, \n"
            << "   Limit_MIN_OutSwOff: " << Limit_MIN_OutSwOff << " sec, \n"
            << "   Limit_MAX_OutSwOff: " << Limit_MAX_OutSwOff << " sec, \n"
            << "   Limit_MIN_VCap_ShutdownVoltage: " << Limit_MIN_VCap_ShutdownVoltage << " V, \n"
            << "   Limit_MAX_VCap_ShutdownVoltage: " << Limit_MAX_VCap_ShutdownVoltage << " V";
            return oss.str();
        }
};
struct DutEntry {
    int id = 0;         // e.g. 1
    std::string name;     // e.g. "ICA2405"
    std::string version;  // e.g. "1.61"
    int EEPROM_BVer = 0;
    bool hasEEPROM_BVer = false;
    std::string FullName; // e.g. "ICA2405A6-vishay"
    std::string BoardKind; // e.g. "NT-CLX USV"
    boardInfo_struct boardInfo; 
    measurementPoint_struct measurementPoint;
    std::vector<RSL_struct::RSL> toDoList;
    int labelPrintNumber = 0; // per-DUT label print count (overrides global if non-zero)
};

// ---- all settings loaded from one config.cfg ---------------------------
struct BasicConfig {
    // [TM_DEVICE]
    std::string devicePort;        // e.g. /dev/ttyAMA1
    std::string storeFolder;       // e.g. ./TestMachine001
    std::string labelPrintCmd;     // full lp command/options read from LPrintCMD
    //int labelPrintNumber = 0;      // optional global label print count (can be overridden per-DUT)
    // [CAPS_LIST]
    std::map<int, std::string> capTypeNames; // e.g. 1 -> "2405-(MaxWell...)"
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
    std::string getLabelPrintCmd() const { return m_cfg.labelPrintCmd; }
    //int getLabelPrintNumber() const { return m_cfg.labelPrintNumber; }
    std::string getCapTypeName(int capTypeId) const;
    void showAllConfig();
private:
    BasicConfig m_cfg;

    // Internal helpers
    static std::string trim(const std::string& s);
    static std::vector<std::string> splitCsv(const std::string& s, char delim = ',');
    static bool parseRslStep(const std::string& token, RSL_struct::RSL& outStep);
    void parseDeviceLine(const std::string& line);
    void parseCapsListLine(const std::string& line);
    void parseDutListLine(const std::string& line, int& nextDutId);
    void parseBoardInfoLine(const std::string& line, boardInfo_struct& info);
    void parseMeasurementPointLine(const std::string& line, measurementPoint_struct& info);
    void parseToDoListLine(const std::string& line, std::vector<RSL_struct::RSL>& toDoList);
};




#endif // ICA_GET_BASIC_H

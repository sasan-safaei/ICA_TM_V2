// ========================================================================
// ICA Group
// ========================================================================
// Project   ICA_TM_V2
// Filename  getBasic.cpp
// Author    Sasan Safaei
// Date      2026-05-19
// ========================================================================

#include "getBasic.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// ---- CfgReader ---------------------------------------------------------

CfgReader::CfgReader() {}

bool CfgReader::load(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[CfgReader] Cannot open: " << filePath << "\n";
        return false;
    }

    // Reset previous values
    m_cfg = BasicConfig{};

    enum class Section { NONE, TM_DEVICE, EMAIL, DUT_LIST, DUT_SECTION, DUT_VERSION, BOARD_INFO, OTHER };
    Section currentSection = Section::NONE;
    std::string currentDutName;
    std::string currentDutVersion;
    std::string currentDutDisplayName;
    boardInfo_struct currentBoardInfo;
    bool boardInfoHasData = false;
    std::vector<RSL_struct::RSL> currentToDoList;
    bool toDoListHasData = false;

    auto findDutEntry = [&](const std::string& name, const std::string& version) -> DutEntry* {
        for (auto& dut : m_cfg.dutList) {
            if (dut.name == name && dut.version == version) {
                return &dut;
            }
        }
        return nullptr;
    };

    auto applyBoardInfoToBaseDut = [&](const std::string& name, const boardInfo_struct& info) {
        for (auto& dut : m_cfg.dutList) {
            if (dut.name == name) {
                dut.boardInfo = info;
            }
        }
    };

    auto applyToDoListToBaseDut = [&](const std::string& name, const std::vector<RSL_struct::RSL>& toDoList) {
        for (auto& dut : m_cfg.dutList) {
            if (dut.name == name) {
                dut.toDoList = toDoList;
            }
        }
    };

    std::string line;
    while (std::getline(file, line)) {
        std::string s = trim(line);

        // Skip empty lines and comments
        if (s.empty() || s[0] == '#')
            continue;

        // Section header
        if (s.front() == '[' && s.back() == ']') {
            std::string sec = s.substr(1, s.size() - 2);
            if      (sec == "TM_DEVICE") currentSection = Section::TM_DEVICE;
            else if (sec == "DUT_LIST")  currentSection = Section::DUT_LIST;
            else if (sec == "DUT_VERSION" || sec == "DUT_version" || sec == "DUT-version") {
                currentSection = Section::DUT_VERSION;
                currentDutVersion.clear();
                currentDutDisplayName.clear();
            }
            else {
                currentDutName = sec;
                currentDutVersion.clear();
                currentDutDisplayName.clear();
                currentSection = Section::DUT_SECTION;
            }
            continue;
        }

        if (currentSection == Section::DUT_VERSION && s.rfind("BOARD_INFO={", 0) == 0) {
            if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                currentBoardInfo = dut->boardInfo;
            } else {
                currentBoardInfo = boardInfo_struct{};
            }
            boardInfoHasData = true;
            currentSection = Section::BOARD_INFO;
            std::string rest = trim(s.substr(std::string("BOARD_INFO={").size()));
            if (!rest.empty() && rest != "}") {
                if (rest.back() == '}')
                    rest.pop_back();
                parseBoardInfoLine(trim(rest), currentBoardInfo);
            }
            continue;
        }

        if (currentSection == Section::DUT_SECTION && s.rfind("toDO_LIST=", 0) == 0) {
            currentToDoList.clear();
            toDoListHasData = true;
            std::string val = trim(s.substr(std::string("toDO_LIST=").size()));
            parseToDoListLine(val, currentToDoList);
            applyToDoListToBaseDut(currentDutName, currentToDoList);
            continue;
        }

        if (currentSection == Section::DUT_VERSION && s.rfind("toDO_LIST=", 0) == 0) {
            currentToDoList.clear();
            toDoListHasData = true;
            std::string val = trim(s.substr(std::string("toDO_LIST=").size()));
            parseToDoListLine(val, currentToDoList);
            if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                dut->toDoList = currentToDoList;
            }
            continue;
        }

        if (currentSection == Section::DUT_SECTION && s.rfind("BOARD_INFO={", 0) == 0) {
            currentBoardInfo = boardInfo_struct{};
            boardInfoHasData = true;
            currentSection = Section::BOARD_INFO;
            std::string rest = trim(s.substr(std::string("BOARD_INFO={").size()));
            if (!rest.empty() && rest != "}") {
                if (rest.back() == '}')
                    rest.pop_back();
                parseBoardInfoLine(trim(rest), currentBoardInfo);
            }
            continue;
        }

        if (currentSection == Section::DUT_VERSION && s.rfind("Name=", 0) == 0) {
            currentDutDisplayName = trim(s.substr(5));
            if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                dut->displayName = currentDutDisplayName;
            }
            continue;
        }

        if (currentSection == Section::BOARD_INFO) {
            if (s == "}") {
                if (boardInfoHasData) {
                    if (currentSection == Section::BOARD_INFO && currentDutVersion.empty()) {
                        applyBoardInfoToBaseDut(currentDutName, currentBoardInfo);
                    } else if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                        dut->boardInfo = currentBoardInfo;
                        if (!currentDutDisplayName.empty()) {
                            dut->displayName = currentDutDisplayName;
                        }
                    }
                }
                currentSection = Section::DUT_SECTION;
                continue;
            }
            std::string boardLine = s;
            if (!boardLine.empty() && boardLine.back() == ',')
                boardLine.pop_back();
            parseBoardInfoLine(boardLine, currentBoardInfo);
            continue;
        }

        switch (currentSection) {

        case Section::TM_DEVICE: parseDeviceLine(s); break;       
        case Section::DUT_LIST: parseDutListLine(s); break;

        case Section::DUT_VERSION: {
            auto eq = s.find('=');
            if (eq != std::string::npos) {
                std::string key = trim(s.substr(0, eq));
                std::string val = trim(s.substr(eq + 1));
                if (key == "version") {
                    currentDutVersion = val;
                } else if (key == "Name") {
                    currentDutDisplayName = val;
                    if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                        dut->displayName = currentDutDisplayName;
                    }
                }
            }
            break;
        }

        default:
            break;
        }
    }

    std::cout << "[CfgReader] Loaded " << filePath
              << "  DUTs: " << m_cfg.dutList.size() << "\n";
    return true;
}

// Parse  Key = Value  lines inside [TM_DEVICE]
void CfgReader::parseDeviceLine(const std::string& line)
{
    auto eq = line.find('=');
    if (eq == std::string::npos) return;

    std::string key = trim(line.substr(0, eq));
    std::string val = trim(line.substr(eq + 1));

    if      (key == "Port")        m_cfg.devicePort   = val;
    else if (key == "StoreFolder") m_cfg.storeFolder  = val;
}
// Parse  {NAME,ver1,ver2,...}  lines inside [DUT_LIST]
void CfgReader::parseDutListLine(const std::string& line)
{
    if (line.empty())
        return;

    if (line.front() != '{' || line.back() != '}')
        return;

    std::string inner = line.substr(1, line.size() - 2);
    std::vector<std::string> parts = splitCsv(inner);
    if (parts.size() < 2) return;

    std::string dutName = trim(parts[0]);
    for (size_t i = 1; i < parts.size(); ++i) {
        std::string v = trim(parts[i]);
        if (!v.empty()) {
            DutEntry entry;
            entry.name = dutName;
            entry.version = v;
            m_cfg.dutList.push_back(entry);
        }
    }
}

void CfgReader::parseBoardInfoLine(const std::string& line, boardInfo_struct& info)
{
    auto eq = line.find('=');
    if (eq == std::string::npos)
        return;

    std::string key = trim(line.substr(0, eq));
    std::string val = trim(line.substr(eq + 1));

    if (!val.empty() && val.back() == ',')
        val.pop_back();

    if (!val.empty() && val.front() == '{')
        val.erase(val.begin());
    if (!val.empty() && val.back() == '}')
        val.pop_back();

    try {
        if      (key == "LTC3350_RSNSI1") info.LTC3350_RSNSI1 = std::stof(val);
        else if (key == "LTC3350_RSNSI2") info.LTC3350_RSNSI2 = std::stof(val);
        else if (key == "LTC3350_RSNSC")  info.LTC3350_RSNSC  = std::stof(val);
        else if (key == "LTC3350_RTST")   info.LTC3350_RTST   = std::stof(val);
        else if (key == "LTC3350_RT")     info.LTC3350_RT     = std::stof(val);
        else if (key == "B_SCapVoltage")  info.Board_SupperCapVoltage = std::stof(val);
        else if (key == "B_SCapSingleCap") info.Board_SupperCapSingleCap = std::stoi(val);
        else if (key == "B_SCapNum")      info.Board_SupperCapNum = std::stoi(val);
        else if (key == "B_SCapType")     info.Board_SupperCapType = std::stoi(val);
        else if (key == "B_MaxTemp85V")   info.Board_MaxTemp85V = std::stoi(val);
        else if (key == "B_VShutdown")    info.Board_VShutdownVoltage = std::stof(val);
    } catch (...) {
        // Ignore malformed values and keep defaults
    }
}

void CfgReader::parseToDoListLine(const std::string& line, std::vector<RSL_struct::RSL>& toDoList)
{
    std::string s = trim(line);
    if (s.empty())
        return;

    if (s.front() == '{')
        s.erase(s.begin());
    if (!s.empty() && s.back() == '}')
        s.pop_back();

    for (const auto& token : splitCsv(s)) {
        RSL_struct::RSL step;
        if (parseRslStep(token, step)) {
            toDoList.push_back(step);
        }
    }
}

bool CfgReader::parseRslStep(const std::string& token, RSL_struct::RSL& outStep)
{
    const std::string s = trim(token);
    if (s == "Init") { outStep = RSL_struct::RSL::Init; return true; }
    if (s == "AR_Test") { outStep = RSL_struct::RSL::AR_Test; return true; }
    if (s == "VCC_Test") { outStep = RSL_struct::RSL::VCC_Test; return true; }
    if (s == "uC_Program") { outStep = RSL_struct::RSL::uC_Program; return true; }
    if (s == "Uart_EEPROM") { outStep = RSL_struct::RSL::Uart_EEPROM; return true; }
    if (s == "uart_EEPROM_RTC_I2C") { outStep = RSL_struct::RSL::uart_EEPROM_RTC_I2C; return true; }
    if (s == "uart_EEPROM_Save") { outStep = RSL_struct::RSL::uart_EEPROM_Save; return true; }
    if (s == "ChargeTest") { outStep = RSL_struct::RSL::ChargeTest; return true; }
    if (s == "FlyBackTest") { outStep = RSL_struct::RSL::FlyBackTest; return true; }
    if (s == "WaitToOutSWOffTest") { outStep = RSL_struct::RSL::WaitToOutSWOffTest; return true; }
    if (s == "DisChargeTest") { outStep = RSL_struct::RSL::DisChargeTest; return true; }
    if (s == "EndSuccess") { outStep = RSL_struct::RSL::EndSuccess; return true; }
    if (s == "EndFailed") { outStep = RSL_struct::RSL::EndFailed; return true; }
    if (s == "justOn") { outStep = RSL_struct::RSL::justOn; return true; }
    if (s == "Stop") { outStep = RSL_struct::RSL::Stop; return true; }
    return false;
}


// ---- static helpers ----------------------------------------------------

std::string CfgReader::trim(const std::string& s)
{
    const char* ws = " \t\r\n";
    size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) return {};
    size_t end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

std::vector<std::string> CfgReader::splitCsv(const std::string& s, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delim))
        result.push_back(trim(token));
    return result;
}

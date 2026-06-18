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

    enum class Section { NONE, TM_DEVICE, EMAIL, CAPS_LIST, DUT_LIST, DUT_DEFAULT, DUT_SECTION, DUT_VERSION, BOARD_INFO, MEASUREMENT_POINT, OTHER };
    Section currentSection = Section::NONE;
    Section boardInfoOwner = Section::NONE;
    Section measurementPointOwner = Section::NONE;
    std::string currentDutName;
    std::string currentDutVersion;
    std::string currentDutFullName;
    std::string currentDutBoardKind;
    std::string defaultDutFullName;
    std::string defaultDutBoardKind;
    int defaultDutEEPROM_BVer = 0;
    boardInfo_struct currentBoardInfo;
    boardInfo_struct defaultBoardInfo;
    measurementPoint_struct currentMeasurementPoint;
    measurementPoint_struct defaultMeasurementPoint;
    bool hasDefaultBoardInfo = false;
    bool hasDefaultMeasurementPoint = false;
    bool hasDefaultFullName = false;
    bool hasDefaultBoardKind = false;
    bool hasDefaultEEPROM_BVer = false;
    bool boardInfoHasData = false;
    std::vector<RSL_struct::RSL> currentToDoList;
    std::vector<RSL_struct::RSL> defaultToDoList;
    bool hasDefaultToDoList = false;
    int nextDutId = 1;

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

    auto applyMeasurementPointToBaseDut = [&](const std::string& name, const measurementPoint_struct& info) {
        for (auto& dut : m_cfg.dutList) {
            if (dut.name == name) {
                dut.measurementPoint = info;
            }
        }
    };

    auto applyFullNameToBaseDut = [&](const std::string& name, const std::string& FullName) {
        for (auto& dut : m_cfg.dutList) {
            if (dut.name == name) {
                dut.FullName = FullName;
            }
        }
    };

    auto applyBoardKindToBaseDut = [&](const std::string& name, const std::string& boardKind) {
        for (auto& dut : m_cfg.dutList) {
            if (dut.name == name) {
                dut.BoardKind = boardKind;
            }
        }
    };

    auto applyIdToBaseDut = [&](const std::string& name, int id) {
        for (auto& dut : m_cfg.dutList) {
            if (dut.name == name) {
                dut.id = id;
            }
        }
    };

    auto applyEEPROMBVerToBaseDut = [&](const std::string& name, int eepromBoardVersion) {
        for (auto& dut : m_cfg.dutList) {
            if (dut.name == name) {
                dut.EEPROM_BVer = eepromBoardVersion;
                dut.hasEEPROM_BVer = true;
            }
        }
    };

    auto applyDefaultsToEntry = [&](DutEntry& dut) {
        if (hasDefaultFullName && dut.FullName.empty()) {
            dut.FullName = defaultDutFullName;
        }
        if (hasDefaultBoardKind && dut.BoardKind.empty()) {
            dut.BoardKind = defaultDutBoardKind;
        }
        if (hasDefaultEEPROM_BVer && !dut.hasEEPROM_BVer) {
            dut.EEPROM_BVer = defaultDutEEPROM_BVer;
            dut.hasEEPROM_BVer = true;
        }
        if (hasDefaultBoardInfo) {
            dut.boardInfo = defaultBoardInfo;
        }
        if (hasDefaultMeasurementPoint) {
            dut.measurementPoint = defaultMeasurementPoint;
        }
        if (hasDefaultToDoList) {
            dut.toDoList = defaultToDoList;
        }
    };

    auto applyDefaultsToAllDuts = [&]() {
        for (auto& dut : m_cfg.dutList) {
            applyDefaultsToEntry(dut);
        }
    };

    auto beginDutSection = [&](const std::string& dutName) {
        currentDutName = trim(dutName);
        currentDutVersion.clear();
        currentDutFullName.clear();
        currentDutBoardKind.clear();
        currentSection = Section::DUT_SECTION;
    };

    std::string line;
    while (std::getline(file, line)) {
        std::string s = trim(line);

        // Skip empty lines and comments
        if (s.empty() || s[0] == '#')
            continue;

        // Section header. Supports both legacy "[ICA2405]" and new "[DUT]#ICA2405" forms.
        if (s.front() == '[') {
            std::size_t closePos = s.find(']');
            if (closePos != std::string::npos) {
                std::string sec = trim(s.substr(1, closePos - 1));
                std::string tail = trim(s.substr(closePos + 1));

                if      (sec == "TM_DEVICE") currentSection = Section::TM_DEVICE;
                else if (sec == "EMAIL")     currentSection = Section::EMAIL;
                else if (sec == "CAPS_LIST") currentSection = Section::CAPS_LIST;
                else if (sec == "DUT_LIST")  currentSection = Section::DUT_LIST;
                 else if (sec == "DUT_DEFAULT") currentSection = Section::DUT_DEFAULT;
                else if (sec == "DUT_VERSION" || sec == "DUT_version" || sec == "DUT-version" ||
                         sec == "DUT_ver" || sec == "DUT-Ver") {
                    currentSection = Section::DUT_VERSION;
                    currentDutVersion.clear();
                    currentDutFullName.clear();
                    currentDutBoardKind.clear();
                }
                else if (sec == "DUT") {
                    std::string dutName = tail;
                    if (!dutName.empty() && dutName.front() == '#') {
                        dutName.erase(dutName.begin());
                    }
                    beginDutSection(dutName);
                }
                else if (sec == "ALL") {
                    currentSection = Section::OTHER;
                }
                else {
                    beginDutSection(sec);
                }
                continue;
            }
        }

        if (currentSection == Section::DUT_VERSION && s.rfind("BOARD_INFO={", 0) == 0) {
            if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                currentBoardInfo = dut->boardInfo;
            } else {
                currentBoardInfo = boardInfo_struct{};
            }
            boardInfoHasData = true;
            boardInfoOwner = currentSection;
            currentSection = Section::BOARD_INFO;
            std::string rest = trim(s.substr(std::string("BOARD_INFO={").size()));
            if (!rest.empty() && rest != "}") {
                if (rest.back() == '}')
                    rest.pop_back();
                parseBoardInfoLine(trim(rest), currentBoardInfo);
            }
            continue;
        }

        if (currentSection == Section::DUT_VERSION && s.rfind("MEASUREMENT_POINT={", 0) == 0) {
            if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                currentMeasurementPoint = dut->measurementPoint;
            } else {
                currentMeasurementPoint = measurementPoint_struct{};
            }
            measurementPointOwner = currentSection;
            currentSection = Section::MEASUREMENT_POINT;
            std::string rest = trim(s.substr(std::string("MEASUREMENT_POINT={").size()));
            if (!rest.empty() && rest != "}") {
                if (rest.back() == '}')
                    rest.pop_back();
                parseMeasurementPointLine(trim(rest), currentMeasurementPoint);
            }
            continue;
        }

        if ((currentSection == Section::DUT_SECTION || currentSection == Section::DUT_VERSION || currentSection == Section::DUT_DEFAULT) &&
            s.rfind("toDO_LIST=", 0) == 0) {
            currentToDoList.clear();
            std::string val = trim(s.substr(std::string("toDO_LIST=").size()));
            parseToDoListLine(val, currentToDoList);
            if (currentSection == Section::DUT_DEFAULT) {
                defaultToDoList = currentToDoList;
                hasDefaultToDoList = true;
                applyDefaultsToAllDuts();
            } else if (currentSection == Section::DUT_SECTION) {
                applyToDoListToBaseDut(currentDutName, currentToDoList);
            } else if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                dut->toDoList = currentToDoList;
            }
            continue;
        }

        if ((currentSection == Section::DUT_SECTION || currentSection == Section::DUT_VERSION || currentSection == Section::DUT_DEFAULT) &&
            s.rfind("BOARD_INFO={", 0) == 0) {
            currentBoardInfo = boardInfo_struct{};
            boardInfoHasData = true;
            boardInfoOwner = currentSection;
            currentSection = Section::BOARD_INFO;
            std::string rest = trim(s.substr(std::string("BOARD_INFO={").size()));
            if (!rest.empty() && rest != "}") {
                if (rest.back() == '}')
                    rest.pop_back();
                parseBoardInfoLine(trim(rest), currentBoardInfo);
            }
            continue;
        }

        if ((currentSection == Section::DUT_SECTION || currentSection == Section::DUT_VERSION || currentSection == Section::DUT_DEFAULT) &&
            s.rfind("MEASUREMENT_POINT={", 0) == 0) {
            currentMeasurementPoint = measurementPoint_struct{};
            measurementPointOwner = currentSection;
            currentSection = Section::MEASUREMENT_POINT;
            std::string rest = trim(s.substr(std::string("MEASUREMENT_POINT={").size()));
            if (!rest.empty() && rest != "}") {
                if (rest.back() == '}')
                    rest.pop_back();
                parseMeasurementPointLine(trim(rest), currentMeasurementPoint);
            }
            continue;
        }

        if (currentSection == Section::DUT_SECTION &&
            (s.rfind("Name=", 0) == 0 || s.rfind("DUT-ID=", 0) == 0)) {
            std::string val = trim(s.substr(s.find('=') + 1));
            if (currentDutName.empty()) {
                currentDutName = val;
            }
            continue;
        }

        if (currentSection == Section::DUT_SECTION && s.rfind("FullName=", 0) == 0) {
            const std::string val = trim(s.substr(std::string("FullName=").size()));
            applyFullNameToBaseDut(currentDutName, val);
            continue;
        }

        if (currentSection == Section::DUT_SECTION && s.rfind("BoardKind=", 0) == 0) {
            const std::string val = trim(s.substr(std::string("BoardKind=").size()));
            applyBoardKindToBaseDut(currentDutName, val);
            continue;
        }

        if (currentSection == Section::DUT_DEFAULT &&
            (s.rfind("FullName=", 0) == 0 || s.rfind("Name=", 0) == 0)) {
            defaultDutFullName = trim(s.substr(s.find('=') + 1));
            hasDefaultFullName = !defaultDutFullName.empty();
            applyDefaultsToAllDuts();
            continue;
        }

        if (currentSection == Section::DUT_DEFAULT && s.rfind("BoardKind=", 0) == 0) {
            defaultDutBoardKind = trim(s.substr(std::string("BoardKind=").size()));
            hasDefaultBoardKind = !defaultDutBoardKind.empty();
            applyDefaultsToAllDuts();
            continue;
        }

        if (currentSection == Section::DUT_DEFAULT && s.rfind("EEPROM_BVer=", 0) == 0) {
            try {
                defaultDutEEPROM_BVer = std::stoi(trim(s.substr(std::string("EEPROM_BVer=").size())), nullptr, 0);
                hasDefaultEEPROM_BVer = true;
                applyDefaultsToAllDuts();
            } catch (...) {
                // Ignore malformed EEPROM_BVer and keep existing values.
            }
            continue;
        }

        if (currentSection == Section::DUT_SECTION && s.rfind("ID=", 0) == 0) {
            try {
                int id = std::stoi(trim(s.substr(3)));
                applyIdToBaseDut(currentDutName, id);
            } catch (...) {
                // Ignore malformed IDs and keep existing values.
            }
            continue;
        }

        if (currentSection == Section::DUT_SECTION && s.rfind("EEPROM_BVer=", 0) == 0) {
            try {
                int eepromBoardVersion = std::stoi(trim(s.substr(std::string("EEPROM_BVer=").size())), nullptr, 0);
                applyEEPROMBVerToBaseDut(currentDutName, eepromBoardVersion);
            } catch (...) {
                // Ignore malformed EEPROM_BVer and keep existing values.
            }
            continue;
        }

        if (currentSection == Section::DUT_VERSION &&
            (s.rfind("Name=", 0) == 0 || s.rfind("FullName=", 0) == 0)) {
            std::string val = trim(s.substr(s.find('=') + 1));
            currentDutFullName = val;
            if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                dut->FullName = currentDutFullName;
            }
            continue;
        }

        if (currentSection == Section::DUT_VERSION && s.rfind("BoardKind=", 0) == 0) {
            std::string val = trim(s.substr(std::string("BoardKind=").size()));
            currentDutBoardKind = val;
            if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                dut->BoardKind = currentDutBoardKind;
            }
            continue;
        }

        if (currentSection == Section::BOARD_INFO) {
            if (s == "}") {
                if (boardInfoHasData) {
                    if (boardInfoOwner == Section::DUT_DEFAULT) {
                        defaultBoardInfo = currentBoardInfo;
                        hasDefaultBoardInfo = true;
                        applyDefaultsToAllDuts();
                    } else if (boardInfoOwner == Section::DUT_SECTION && currentDutVersion.empty()) {
                        applyBoardInfoToBaseDut(currentDutName, currentBoardInfo);
                    } else if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                        dut->boardInfo = currentBoardInfo;
                        if (!currentDutFullName.empty()) {
                            dut->FullName = currentDutFullName;
                        }
                        if (!currentDutBoardKind.empty()) {
                            dut->BoardKind = currentDutBoardKind;
                        }
                    }
                }
                currentSection = boardInfoOwner;
                boardInfoOwner = Section::NONE;
                continue;
            }
            std::string boardLine = s;
            if (!boardLine.empty() && boardLine.back() == ',')
                boardLine.pop_back();
            parseBoardInfoLine(boardLine, currentBoardInfo);
            continue;
        }

        if (currentSection == Section::MEASUREMENT_POINT) {
            if (s == "}") {
                if (measurementPointOwner == Section::DUT_DEFAULT) {
                    defaultMeasurementPoint = currentMeasurementPoint;
                    hasDefaultMeasurementPoint = true;
                    applyDefaultsToAllDuts();
                } else if (measurementPointOwner == Section::DUT_SECTION && currentDutVersion.empty()) {
                    applyMeasurementPointToBaseDut(currentDutName, currentMeasurementPoint);
                } else if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                    dut->measurementPoint = currentMeasurementPoint;
                }
                currentSection = measurementPointOwner;
                measurementPointOwner = Section::NONE;
                continue;
            }

            std::string measurementLine = s;
            if (!measurementLine.empty() && measurementLine.back() == ',')
                measurementLine.pop_back();
            parseMeasurementPointLine(measurementLine, currentMeasurementPoint);
            continue;
        }

        switch (currentSection) {

        case Section::TM_DEVICE: parseDeviceLine(s); break;       
        case Section::CAPS_LIST: parseCapsListLine(s); break;
        case Section::DUT_LIST: {
            const std::size_t beforeCount = m_cfg.dutList.size();
            parseDutListLine(s, nextDutId);
            for (std::size_t i = beforeCount; i < m_cfg.dutList.size(); ++i) {
                applyDefaultsToEntry(m_cfg.dutList[i]);
            }
            break;
        }

        case Section::DUT_VERSION: {
            auto eq = s.find('=');
            if (eq != std::string::npos) {
                std::string key = trim(s.substr(0, eq));
                std::string val = trim(s.substr(eq + 1));
                if (key == "version") {
                    currentDutVersion = val;
                } else if (key == "ID") {
                    try {
                        if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                            dut->id = std::stoi(val);
                        }
                    } catch (...) {
                        // Ignore malformed IDs and keep existing values.
                    }
                } else if (key == "EEPROM_BVer") {
                    try {
                        if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                            dut->EEPROM_BVer = std::stoi(val, nullptr, 0);
                            dut->hasEEPROM_BVer = true;
                        }
                    } catch (...) {
                        // Ignore malformed EEPROM_BVer and keep existing values.
                    }
                } else if (key == "Name") {
                    currentDutFullName = val;
                    if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                        dut->FullName = currentDutFullName;
                    }
                } else if (key == "BoardKind") {
                    currentDutBoardKind = val;
                    if (auto* dut = findDutEntry(currentDutName, currentDutVersion)) {
                        dut->BoardKind = currentDutBoardKind;
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

void CfgReader::parseCapsListLine(const std::string& line)
{
    if (line.empty())
        return;

    if (line.front() != '{' || line.back() != '}')
        return;

    std::string inner = line.substr(1, line.size() - 2);
    std::vector<std::string> parts = splitCsv(inner);
    if (parts.size() < 2)
        return;

    int capId = 0;
    try {
        capId = std::stoi(trim(parts[0]));
    } catch (...) {
        return;
    }

    std::string capName;
    if (parts.size() >= 3) {
        capName = trim(parts[2]);
    } else {
        capName = trim(parts[1]);
    }

    if (!capName.empty() && capName.front() == '"') {
        capName.erase(capName.begin());
    }
    if (!capName.empty() && capName.back() == '"') {
        capName.pop_back();
    }

    m_cfg.capTypeNames[capId] = capName;
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
    else if (key == "LPrintCMD") {
        // remove surrounding quotes if present
        if (!val.empty() && val.front() == '"') val.erase(val.begin());
        if (!val.empty() && val.back() == '"') val.pop_back();
        m_cfg.labelPrintCmd = val;
    }
}
// Parse  {NAME,ver1,ver2,...}  lines inside [DUT_LIST]
void CfgReader::parseDutListLine(const std::string& line, int& nextDutId)
{
    if (line.empty())
        return;

    if (line.front() != '{' || line.back() != '}')
        return;

    std::string inner = line.substr(1, line.size() - 2);
    std::vector<std::string> parts = splitCsv(inner);
    if (parts.size() < 2) return;

    std::string dutName = trim(parts[0]);
    const int dutId = nextDutId;
    bool addedAtLeastOneVersion = false;
    for (size_t i = 1; i < parts.size(); ++i) {
        std::string v = trim(parts[i]);
        if (!v.empty()) {
            DutEntry entry;
            entry.id = dutId;
            entry.name = dutName;
            entry.version = v;
            m_cfg.dutList.push_back(entry);
            addedAtLeastOneVersion = true;
        }
    }

    if (addedAtLeastOneVersion) {
        ++nextDutId;
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
        else if (key == "B_MaxTemp85V")   info.Board_MaxTemp85V = (int)(std::stof(val)*10);
        else if (key == "B_VShutdown")    info.Board_VShutdownVoltage = std::stof(val);
    } catch (...) {
        // Ignore malformed values and keep defaults
    }
}

void CfgReader::parseMeasurementPointLine(const std::string& line, measurementPoint_struct& info)
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
        if      (key == "InCurrent_NoAR_MaxLimit") info.InCurrent_NoAR_MaxLimit = std::stof(val);
        else if (key == "InCurrent_AR_MinLimit")   info.InCurrent_AR_MinLimit   = std::stof(val);
        else if (key == "VCC_minLimit")            info.VCC_minLimit            = std::stof(val);
        else if (key == "VCC_maxLimit")            info.VCC_maxLimit            = std::stof(val);
        else if (key == "Load_Current")            info.Load_Current            = std::stof(val);
        else if (key == "Limit_MIN_ChargeCurrent") info.Limit_MIN_ChargeCurrent = std::stof(val);
        else if (key == "Limit_MIN_FullChargeCurrent") info.Limit_MIN_FullChargeCurrent = std::stof(val);
        else if (key == "Limit_MAX_Charge_time")   info.Limit_MAX_Charge_time   = std::stoi(val);
        else if (key == "Limit_MIN_WaitToOutSwOff") info.Limit_MIN_WaitToOutSwOff = std::stoi(val);
        else if (key == "Limit_MAX_WaitToOutSwOff") info.Limit_MAX_WaitToOutSwOff = std::stoi(val);
        else if (key == "Limit_MIN_OutSwOff")      info.Limit_MIN_OutSwOff      = std::stoi(val);
        else if (key == "Limit_MAX_OutSwOff")      info.Limit_MAX_OutSwOff      = std::stoi(val);
        else if (key == "Limit_MIN_VCap_ShutdownVoltage") info.Limit_MIN_VCap_ShutdownVoltage = std::stof(val);
        else if (key == "Limit_MAX_VCap_ShutdownVoltage") info.Limit_MAX_VCap_ShutdownVoltage = std::stof(val);
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
    if (s == "EEPROM_RTC_I2C") { outStep = RSL_struct::RSL::EEPROM_RTC_I2C; return true; }
    if (s == "uart_EEPROM_Save") { outStep = RSL_struct::RSL::uart_EEPROM_Save; return true; }
    if (s == "ChargeTest") { outStep = RSL_struct::RSL::ChargeTest; return true; }
    if (s == "FlyBackTest") { outStep = RSL_struct::RSL::FlyBackTest; return true; }
    if (s == "WaitToOutSWOffTest") { outStep = RSL_struct::RSL::WaitToOutSWOffTest; return true; }
    if (s == "DisChargeTest") { outStep = RSL_struct::RSL::DisChargeTest; return true; }
    if (s == "EndSuccess") { outStep = RSL_struct::RSL::EndSuccess; return true; }
    if (s == "EndFailed") { outStep = RSL_struct::RSL::EndFailed; return true; }
    if (s == "justOn") { outStep = RSL_struct::RSL::justOn; return true; }
    if (s== "powerOn") { outStep = RSL_struct::RSL::powerOn; return true; }
    if (s== "IBIS_LoopBackCheck") { outStep = RSL_struct::RSL::IBIS_LoopBackCheck; return true; }
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

void CfgReader::showAllConfig()
{
    std::cout << "Device Port: " << m_cfg.devicePort << "\n";
    std::cout << "Store Folder: " << m_cfg.storeFolder << "\n";
    if (!m_cfg.labelPrintCmd.empty())
        std::cout << "Label Print Cmd: " << m_cfg.labelPrintCmd << "\n";
    std::cout << "DUT List:\n";
    for (const auto& dut : m_cfg.dutList) {
        std::cout <<"ID: " << dut.id << ", Name: " << dut.name << ", Version: " << dut.version;
        if (dut.hasEEPROM_BVer) {
            std::cout << ", EEPROM_BVer: 0x" << std::hex << std::uppercase << dut.EEPROM_BVer << std::dec;
        }
        if (!dut.FullName.empty()) {
            std::cout << ", Full Name: " << dut.FullName;
        }
        if (!dut.BoardKind.empty()) {
            std::cout << ", Board Kind: " << dut.BoardKind;
        }
        std::cout << std::endl;
        std::cout << dut.boardInfo.toString() << std::endl;
        if (dut.boardInfo.Board_SupperCapType != 0) {
            const std::string capName = getCapTypeName(dut.boardInfo.Board_SupperCapType);
            std::cout << "Cap Type Name: ";
            if (!capName.empty()) {
                std::cout << capName;
            } else {
                std::cout << "Unknown Cap Type";
            }
            std::cout << " (ID: " << dut.boardInfo.Board_SupperCapType << ")\n";
        }
        std::cout << dut.measurementPoint.toString() << std::endl;
        std::cout << "ToDo List: [";
        
        for (size_t i = 0; i < dut.toDoList.size(); ++i) {
            std::cout << RSL_struct().getRSLStr(dut.toDoList[i]);
            if (i + 1 < dut.toDoList.size())
                std::cout << ", ";
        }
        std::cout << "]\n";
        std::cout << "-----------------------" << std::endl;
    }
}

std::string CfgReader::getCapTypeName(int capTypeId) const
{
    auto it = m_cfg.capTypeNames.find(capTypeId);
    if (it == m_cfg.capTypeNames.end()) {
        return "";
    }
    return it->second;
}


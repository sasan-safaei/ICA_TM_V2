#include "ICA_justEUI.h"
bool ICA_justEUI::ReadEUI(){
    if (rtc.readBlock(0xF0, 8, EUI_Buffer))
        return true;
    else
        return false;
}
bool ICA_justEUI::ReadEUI_EEPROM(){
    if (myEEPROM.readBlock(0xF8, 8, EUI_Buffer))
        return true;
    else
        return false;
}
std::string ICA_justEUI::showEUI(){
    std::ostringstream oss;
    if (EUI_Buffer.size() != 8) {
        return "Invalid EUI";
    }
    oss << "EUI:["
        << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)EUI_Buffer[0] << "."
        << std::setw(2) << (int)EUI_Buffer[1] << "."
        << std::setw(2) << (int)EUI_Buffer[2] << "]"
        << std::setw(2) << (int)EUI_Buffer[3] << "."
        << std::setw(2) << (int)EUI_Buffer[4] << "."
        << std::setw(2) << (int)EUI_Buffer[5] << "."
        << std::setw(2) << (int)EUI_Buffer[6] << "."
        << std::setw(2) << (int)EUI_Buffer[7];

    return oss.str();
}
bool MCP79412::openBus(int& file) {
    file = open(dev_path, O_RDWR);
    if (file < 0) return false;
    if (ioctl(file, I2C_SLAVE, address) < 0) {
        close(file);
        return false;
    }
    return true;
}
void MCP79412::closeBus(int file) {
    if (file >= 0) close(file);
}
bool MCP79412::readBlock(uint8_t startReg, uint8_t length, std::vector<uint8_t>& buffer) {
    buffer.clear();
    int file;
    if (!openBus(file)) return false;

    if (write(file, &startReg, 1) != 1) {
        closeBus(file);
        return false;
    }
    uint8_t temp[length];
    if (read(file, temp, length) != length) {
        closeBus(file);
        return false;
    }
    buffer.insert(buffer.end(), temp, temp + length);
    closeBus(file);
    return true;
}
bool MCP79412::writeBlock(uint8_t startReg, const std::vector<uint8_t>& data) {
    int file;
    if (!openBus(file)) return false;

    std::vector<uint8_t> payload;
    payload.push_back(startReg);
    payload.insert(payload.end(), data.begin(), data.end());

    if (write(file, payload.data(), payload.size()) != (ssize_t)payload.size()) {
        closeBus(file);
        return false;
    }

    closeBus(file);
    return true;
}


bool EEPROM24xx256::readBlock(uint8_t startReg, uint8_t length, std::vector<uint8_t>& buffer) {
    buffer.clear();
    int file;
    if (!openBus(file)) return false;

    if (write(file, &startReg, 1) != 1) {
        closeBus(file);
        return false;
    }
    uint8_t temp[length];
    if (read(file, temp, length) != length) {
        closeBus(file);
        return false;
    }
    buffer.insert(buffer.end(), temp, temp + length);
    closeBus(file);
    return true;
}

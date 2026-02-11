#include "mySerialPort.h"
#include <iostream>
#include <string>


SerialPort::~SerialPort() {
    sp_close();
}
void SerialPort::sp_init(char* ttyName, int _baudRate){
    hSerial = -1;
    tty_sp=ttyName;
    baudRate =_baudRate;
    memset(&tty, 0, sizeof(tty));
}
bool SerialPort::sp_open() {
   //std::cout<<"portname: " <<tty_sp <<"\n";
   
    hSerial = open(tty_sp, O_RDWR | O_NOCTTY | O_NDELAY);
    if (hSerial < 0) {
        std::cerr << "Error opening serial port." << std::endl;
        return false;
    }
    if (tcgetattr(hSerial, &tty) != 0) {
        perror("Error getting tty attributes");
        sp_close();
        return 1;
    }
    // Set baud rate
    cfsetispeed(&tty, baudRate);
    cfsetospeed(&tty, baudRate);
    // 8 bits, no parity, 1 stop bit
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo, no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls, enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= 0;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    if (tcsetattr(hSerial, TCSANOW, &tty) != 0) {
        perror("Error setting tty attributes");
        sp_close();
        return 1;
    }
    return true;
}
void SerialPort::sp_close() {
    if (hSerial >0) {
        close(hSerial);
        hSerial = -1;
    }
}

bool SerialPort::sp_write(unsigned char* data, int dataSize) {
    if (hSerial < 0 ) {
        std::cerr << "Serial port not open." << std::endl;
        return false;
    }
    tcflush(hSerial, TCIOFLUSH);
    int n_written = write(hSerial, data, dataSize);
    usleep(5000);
    tcflush(hSerial, TCIOFLUSH);    
    if (n_written < 0) {
        std::cerr << "Error writing to serial port." << std::endl;
        return false;
    }
    return true;
}

bool SerialPort::sp_read(unsigned char* buffer, int bufferSize) {
    if (hSerial < 0 ) {
        std::cerr << "Serial port not open." << std::endl;
        return false;
    }    
    int n_read = read(hSerial, buffer, bufferSize);
    if (n_read< 0 ) {
        //std::cerr << "Error reading from serial port."<< std::endl;
        return false;
    }
    return true;
}
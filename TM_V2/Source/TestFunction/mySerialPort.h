//#pragma once

#ifndef MYSERIALPORT_H
#define MYSERIALPORT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
class SerialPort {
public:
    ~SerialPort();

    bool sp_open();
    void sp_close();
    bool sp_write(unsigned char* data, int dataSize);
    bool sp_read(unsigned char* buffer, int bufferSize);
    void sp_init(char* ttyName, int baudRate);
    //int clearInputBuffer() { return tcflush(hSerial, TCIFLUSH); }
private:
    int hSerial;
    char* tty_sp;
    int baudRate;
    struct termios tty;
};

#endif

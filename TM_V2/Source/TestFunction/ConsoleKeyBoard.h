#ifndef __CONSOLEKEYBOARD_H__
#define __CONSOLEKEYBOARD_H__

#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <cctype>
#include <cstdio>
#include <atomic>

class ConsoleKeyClass {
public:
    static struct termios oldTermios, newTermios;

private:
    pthread_t Thread_KB;
    pthread_attr_t attr_kb;
    static std::atomic<char> ConsoleKeyPress;

    static void* thread_consolKB(void* arg);

public:
    ConsoleKeyClass();
    ~ConsoleKeyClass();
    char getConsolKey(bool waitToPress);
};

#endif

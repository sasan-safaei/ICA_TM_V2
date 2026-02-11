#include"ConsoleKeyBoard.h"

struct termios ConsoleKeyClass::oldTermios;
struct termios ConsoleKeyClass::newTermios;
std::atomic<char> ConsoleKeyClass::ConsoleKeyPress;

void* ConsoleKeyClass::thread_consolKB(void* arg) {
    tcgetattr(STDIN_FILENO, &oldTermios);
    newTermios = oldTermios;
    newTermios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);

    char ch;
    while (true) {
        ch = getchar();
        ConsoleKeyPress = toupper(ch);
        usleep(1000);
    }
}

ConsoleKeyClass::ConsoleKeyClass() {
    ConsoleKeyPress = 0;
    pthread_attr_init(&attr_kb);
    pthread_create(&Thread_KB, &attr_kb, &thread_consolKB, nullptr);
    pthread_detach(Thread_KB);
}

ConsoleKeyClass::~ConsoleKeyClass() {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
    pthread_cancel(Thread_KB);
}

char ConsoleKeyClass::getConsolKey(bool waitToPress) {
    if (waitToPress) {
        ConsoleKeyPress = 0;
        while (ConsoleKeyPress == 0) {
            usleep(1000);
        }
    }
    return ConsoleKeyPress.load();
}

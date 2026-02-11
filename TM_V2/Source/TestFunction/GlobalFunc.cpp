#include "GlobalFunc.h"

void printCurrentTime() {
    std::time_t now = std::time(nullptr); // Get current time as a time_t object
    std::tm *localTime = std::localtime(&now); // Convert to local time
    // Print formatted time
    printf("Time: %d-%d-%d %d:%d\n",1900+localTime->tm_year,localTime->tm_mon+1,localTime->tm_mday,localTime->tm_hour,localTime->tm_min);
    //std::cout << "Current time: " << std::asctime(localTime)<<"\n";
    //std::cout << "time: " << localTime->tm_year <<" "<<localTime->tm_sec;
	
}
void ica_delay_us (int _value)
{
	timespec time1,time2;
	clock_gettime(CLOCK_REALTIME,&time1);
	clock_gettime(CLOCK_REALTIME,&time2);	
	while( (SET_TO_NS((uint64_t)time2.tv_sec)+ time2.tv_nsec)< (SET_TO_NS((uint64_t)time1.tv_sec)+time1.tv_nsec+_value*1000))
	{	
		clock_gettime(CLOCK_REALTIME,&time2);
	}
}
int ica_kbhit(char* currentVal) {
    struct termios oldTerm, newTerm;
    int ch;
    int oldFlags;

    // Save the old terminal settings
    tcgetattr(STDIN_FILENO, &oldTerm);
    newTerm = oldTerm;

    // Disable canonical mode and input echo
    newTerm.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newTerm);

    // Set the file descriptor for the terminal in non-blocking mode
    oldFlags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldFlags | O_NONBLOCK);

    // Check if a key is pressed
    ch = getchar();

    // Restore the terminal settings and file descriptor flags
    tcsetattr(STDIN_FILENO, TCSANOW, &oldTerm);
    fcntl(STDIN_FILENO, F_SETFL, oldFlags);

    // Check if a key is pressed
	*currentVal=ch;
    return ch != EOF;
}
void convert_timeToBytes(uint8_t* buff) {
    uint8_t time_array[8] = {0};  // Fixed-size array for storage
    time_t current_time = time(NULL);

    //printCurrentTime();
    //printf("... Size of time_t: %zu bytes\n", sizeof(time_t));
    //printf("... Original timestamp: %ld\n", current_time);
    // Store the timestamp in the uint8_t array
    memcpy(time_array, &current_time, sizeof(current_time));
    memcpy(buff,time_array,8);

    // Print the raw bytes for debugging
    //printf("... Stored bytes: ");
    //for (size_t i = 0; i < sizeof(current_time); i++) {
    //    printf("%02X ", time_array[i]);
    //}
    //printf("\n");
}

time_t convert_BytesToTime(uint8_t* buff) {
    uint8_t time_array[8] = {0};  // Fixed-size array for storage
    time_t convertedTime=0;
    memcpy(&convertedTime, &buff, sizeof(time_t));
    return convertedTime;
    
}


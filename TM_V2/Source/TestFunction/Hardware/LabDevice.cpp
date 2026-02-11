#include "LabDevice.h"
std::vector<std::string> LabDevice::listSerialPorts() {
    std::vector<std::string> ports;
    for (const auto& entry : std::filesystem::directory_iterator("/dev")) {
        std::string path = entry.path().string();
        if (path.find("/dev/ttyUSB") == 0){// || path.find("/dev/ttyACM") == 0) {
            ports.push_back(path);
			//printf("Path: %s\n",path.c_str());
        }
    }
    return ports;
}
bool LabDevice::configurePort(int fd) {
    struct termios options;
    if (tcgetattr(fd, &options) != 0) return false;

    cfsetispeed(&options, B9600);  // Change baud rate as needed
    cfsetospeed(&options, B9600);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;        // 8 data bits
    options.c_cflag &= ~PARENB;    // No parity
    options.c_cflag &= ~CSTOPB;    // 1 stop bit
    options.c_cflag &= ~CRTSCTS;   // No flow control
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;

    return (tcsetattr(fd, TCSANOW, &options) == 0);
}
std::string LabDevice::findCorrectSerialPort(const std::string& testFrame, const std::string& expectedResponse) {
	SerialPort xsp;
	auto ports=listSerialPorts();
	for (const auto& port : ports) {
		int fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (fd == -1) continue;
		if (!configurePort(fd)) {
            close(fd);
            continue;
        }
		 // Send frame
		 write(fd, testFrame.c_str(), testFrame.size());
		//printf("CMD: %s\n",testFrame.c_str());
		// Read response
		char buffer[256] = {0};
		usleep(200000); // Wait for response (200ms)
		int n = read(fd, buffer, sizeof(buffer) - 1);
		close(fd);

		if (n > 0) {
			std::string response(buffer, n);
			if (response.find(expectedResponse) != std::string::npos) {
				printf("Device: %s\n",response.c_str());
				return port;
			}
		}    
	}
	return "";
}

bool LabDevice::PSupply_INI(char* _model,int bps){
    spPSupply.sp_close();
	std::string PortPath= findCorrectSerialPort("*IDN?\n",_model);//"RS-6005") ;	
	if(PortPath.length()==0)	return false;
	spPSupply.sp_init((char* )PortPath.c_str(),bps);
    if (spPSupply.sp_open()==false) 
    {
        printf("No Power Supply connected!!!\n\n");
        return false;
    }
	return true;	
	
}
bool LabDevice::Load_INI(char* _model, int bps){   
	spLoad.sp_close();
	std::string PortPath= findCorrectSerialPort("*IDN?\n",_model);//"RS-KEL102") ;
	if(PortPath.length()==0)	return false;
	spLoad.sp_init((char* )PortPath.c_str(),bps);
    if (spLoad.sp_open()==false) 
    {
        printf("No Load connected!!!\n\n");
        return false;
    }
    return true;
}

void LabDevice::SetPSEnable(bool _en){
	if (_en)
		sprintf(DCMD,"OUT1\n"); 
	else 
		sprintf(DCMD,"OUT0\n"); 	
	spPSupply.sp_write((unsigned char *)DCMD,strlen(DCMD));	
}
void LabDevice::SetPSVoltage(float _value){
	sprintf(DCMD,"VSET1:%.2f\n",_value); 
	spPSupply.sp_write((unsigned char *)DCMD,strlen(DCMD));
}
void LabDevice::SetPSCurrent(float _value){
	sprintf(DCMD,"ISET1:%.3f\n",_value);
	spPSupply.sp_write((unsigned char *)DCMD,strlen(DCMD));		
}
void LabDevice::SetLoadCurrent(float _value){	
	sprintf(DCMD,":CURR %.3fA\n",_value); 
	spLoad.sp_write((unsigned char *)DCMD,strlen(DCMD));		
	usleep(100000);
	if (_value>0)
		sprintf(DCMD,":INP 1\n");
	else
		sprintf(DCMD,":INP 0\n");
	spLoad.sp_write((unsigned char *)DCMD,strlen(DCMD));					
}
float LabDevice::ReadPSCurrent(){
	sprintf(DCMD,"IOUT1?\n"); 
	spPSupply.sp_write((unsigned char *)DCMD,strlen(DCMD));
	DCMD[0]=0;usleep(100000);
	spPSupply.sp_read((unsigned char *)DCMD,6);DCMD[6]=0;
	if(strlen(DCMD)>0)
		return strtof(DCMD, NULL);
	else
		return -1;
}
bool LabDevice::SetPSOCP(bool _en){
	if (_en)
		sprintf(DCMD,"OCP1\n"); 
	else
		sprintf(DCMD,"OCP0\n"); 	
	spPSupply.sp_write((unsigned char *)DCMD,strlen(DCMD));		
	return true;
}

float LabDevice::ReadLoadVoltage(){
	sprintf(DCMD,":MEAS:VOLT?\n");
	spLoad.sp_write((unsigned char *)DCMD,strlen(DCMD));
	DCMD[0]=0;usleep(100000);
	spLoad.sp_read((unsigned char *)DCMD,10);DCMD[6]=0;
	if(strlen(DCMD)>0)
		return strtof(DCMD, NULL);
	else
		return -1;
				
}
float LabDevice::ReadLoadCurrent(){
	sprintf(DCMD,":MEAS:CURR?\n"); 
	spLoad.sp_write((unsigned char *)DCMD,strlen(DCMD));
	DCMD[0]=0;usleep(100000); 
	spLoad.sp_read((unsigned char *)DCMD,10);DCMD[6]=0;
	if(strlen(DCMD)>0)
		return strtof(DCMD, NULL);
	else
		return -1;
}


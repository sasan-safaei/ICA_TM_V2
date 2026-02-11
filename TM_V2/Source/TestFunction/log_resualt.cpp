#include"log_resualt.h"
//File saving
int getColumnInt(char* sline, int cNum ){
	char *token;
	char *line_copy = strdup(sline);
	int field_count = 0;	
	token = strtok(line_copy, ",");	
	while (token != NULL) {
		field_count++;
		if (field_count == cNum) {			
			printf("%d/%d th value: %s\n",field_count,cNum,token);
			return std::stoi(token);
		}
		token = strtok(NULL, ",");
    }	
	free(line_copy);
	return -1;
}
std::string getColumnStr(char* sline, int cNum ){
	char *token;
	char *line_copy = strdup(sline);
	int field_count = 0;	
	token = strtok(line_copy, ",");	
	while (token != NULL) {
		field_count++;
		if (field_count == cNum) {			
			//printf("%d/%d th value: %s\n",field_count,cNum,token);
			return token;
		}
		token = strtok(NULL, ",");
    }	
	free(line_copy);
	return "NULL";
}
bool CheckEUI(char* sline){
	uint8_t read_EUI[8];
	char *start = strstr(sline, ",");			
	if (!start) { fprintf(stderr, "Invalid line format: %s", sline); return false; }			
	char hex_string[17]; // 16 hex digits + null terminator
	strncpy(hex_string, start + 1, 6);
	strncpy(&hex_string[6], start + 8, 10);	
	hex_string[16] = '\0';
	// Convert the hex string to uint8_t array
	for (int i = 0; i < 8; i++) {
		char byte_string[3] = {hex_string[i * 2], hex_string[i * 2 + 1], '\0'};
		read_EUI[i] = (uint8_t)strtoul(byte_string, NULL, 16);
	}	
	// Compare the two arrays
	if (memcmp(read_EUI, myBoard.myEEPROM.myData.EUI, 8) == 0) return true;
	return false;
}
int count_lines(FILE *file,bool checkEUI,std::string _fileName, int *currentLine) {
	//uint8_t read_EUI[8];
    char line[256];  // Buffer to store each line, adjust size as needed
	int line_cnt=0;
	rewind(file);  // Go back to the beginning to count lines
    // Read each line and print it until the end of the file is reached
    while (fgets(line, sizeof(line), file) != NULL) {
		//line[strcspn(line, "\n")] = '\0';
		if(line_cnt>0){
			if (checkEUI){
				if (CheckEUI(line)){
					printf("!!! EUI was already saved on %s !!!:\n   %s\n",_fileName.c_str(), line);
					*currentLine=line_cnt;
					return -1;
				}
			}
		}
		//printf("TTT %d :: %s",line_cnt,line);
        line_cnt++;
    }
    // Reset file pointer to the start of the file if needed for further operations
    //rewind(file);
	return line_cnt;
}
int check_repaired(std::string _fileName){
	int line_cnt=0;
	char line[256];
	FILE *file = fopen(_fileName.c_str(), "r+");    
    if (file == NULL) return -2;
	
	fseek(file, 0, SEEK_END);
    
	long file_size = ftell(file);
	if (file_size > 0) {
		rewind(file);
		while (fgets(line, sizeof(line), file) != NULL) {
			line[strcspn(line, "\n")] = '\0';			
			if (CheckEUI(line)){
				//printf("!!! Line%d: %s\n", line_cnt, line);
				std::string __str = getColumnStr(line,7);				
				 if(__str == "0") return __okID;
				 else if(__str == __okStr) return __okID;
				 else if(__str == __defectedStr) return __defectedID;
				 else if(__str == __repairedStr) return __repairedID;				 
				 else return -1;
			}
			line_cnt++;
		}
		
    } else {
        printf("The file is empty. Starting line count from 0.\n");		
    }
	return -1;
}
std::string statusToString(int status){
	if(status == __okID) return __okStr;
	else if(status == __defectedID) return __defectedStr;
	else if(status == __repairedID) return __repairedStr;
	else return "Unknown";
}	
int checkForOldTestingData(std::string _fileName){
	//char fileName[80];
	//sprintf(fileName,"%s/ICA-NT-TEST.csv",myUSVTestV2.myArg.StoreFolder);//\0
    FILE *file = fopen(_fileName.c_str(), "r+");
    if (file == NULL) return 0;
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
	int line_count = 0;
	int __currentLine=0;
	if (file_size > 0) {
        //rewind(file);  // Go back to the beginning to count lines
        line_count = count_lines(file,true,_fileName,&__currentLine);
	}
	return line_count;
}

bool update_eui_line_and_append_data(const char* filename, int line_number, int lastStatus, bool testResualt) {
	
	std::ifstream infile(filename);
	
	
	if (!infile.is_open()) { return false; }

	std::string line;
	std::string out_content;
	int current = 0;
	// Prepare new date/time
	std::time_t now = std::time(nullptr);
	std::tm *tmnow = std::localtime(&now);
	char datebuf[32];
	char timebuf[32];
	std::snprintf(datebuf, sizeof(datebuf), "20%02d-%02d-%02d", tmnow->tm_year%100, tmnow->tm_mon+1, tmnow->tm_mday);
	std::snprintf(timebuf, sizeof(timebuf), "%02d:%02d:%02d", tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec);

	
	while (std::getline(infile, line)) {
		if (current == line_number) {
			
			// split CSV
			std::vector<std::string> fields;
			std::istringstream ss(line);
			std::string f;
			while (std::getline(ss, f, ',')) fields.push_back(f);
			std::string repairedStr = statusToString(lastStatus)+"_was_on_";
			if (fields.size() > 5) {
				repairedStr += fields[4] + "_" + fields[5];
				fields[4] = datebuf;
				fields[5] = timebuf;
			}
			if (fields.size() > 6) {
				if(testResualt)
					fields[6] = statusToString(__repairedID);
				else
					fields[6] = statusToString(__defectedID);
			}

			// reconstruct
			std::string newl;
			for (size_t i = 0; i < fields.size(); ++i) {
				if (i) newl += ",";
				newl += fields[i];
			}
			newl += "," + repairedStr;
			out_content += newl + "\n";
		} else {
			out_content += line + "\n";
		}
		current++;
	}
	infile.close();

	std::ofstream outfile(filename, std::ios::trunc);
	if (!outfile.is_open()) return false;
	outfile << out_content;
	outfile.close();
	return true;
}
int SaveEUI(std::string _fileNameEUI,bool testResualt){
	char file_stream[256];
	FILE *file = fopen(_fileNameEUI.c_str(), "r+");

	if (file == NULL) {
        file = fopen(_fileNameEUI.c_str(), "w+");
        if (file == NULL) { perror("Error creating file"); return EXIT_FAILURE; }
		sprintf(file_stream, "Num,EUI-M,EUI,Device,Date,Time,Status,Description\n");
		fprintf(file, "%s", file_stream);
        printf("The file was not found, so a new file has been created.\n");
    }

	fseek(file, 0, SEEK_END);
    long file_size = ftell(file);

    int currentLine=0;
	int line_count = count_lines(file,true,_fileNameEUI,&currentLine);
	int lastStatus=check_repaired(_fileNameEUI);	
	// Check whether this EUI already exists; if so, count_lines returns -1 and sets currentLine
	//printf("************ SAVE EUI LineCount: %d , CurrentLine: %d , LastStatus: %d ************\n", line_count, currentLine, lastStatus);
	if(line_count == -1) {
		if((lastStatus == __defectedID && testResualt == true) || (lastStatus != __defectedID && testResualt == false)) {
			bool upd = update_eui_line_and_append_data(_fileNameEUI.c_str(), currentLine, lastStatus,testResualt);
			if (!upd) {
				std::cerr << "Failed to update existing EUI line in " << _fileNameEUI << std::endl;
			}
		}
	}else{
		std::time_t Savetime_now = std::time(nullptr); // Get current time as a time_t object
		std::tm *SaveTime = std::localtime(&Savetime_now); // Convert to local time    
		myBoard.updateBoardNameStr();
		uint8_t EUI[8];
		memcpy(EUI,myBoard.myEEPROM.myData.EUI,8);
		sprintf(file_stream, "%04d,%02X%02X%02X,%02X%02X%02X%02X%02X,%s,20%02d-%02d-%02d,%02d:%02d:%02d"
					,line_count
					,EUI[0],EUI[1],EUI[2],EUI[3],EUI[4],EUI[5],EUI[6],EUI[7]
					,myBoard.boardName_str
					,SaveTime->tm_year%100,SaveTime->tm_mon+1,SaveTime->tm_mday,SaveTime->tm_hour,SaveTime->tm_min,SaveTime->tm_sec);

		// Already saved, just update status
		sprintf(&file_stream[strlen(file_stream)], ",");			
		if (testResualt)
			sprintf(&file_stream[strlen(file_stream)], "Ok\n");
		else 
			sprintf(&file_stream[strlen(file_stream)], "Defected\n");
		
		bool __saveData=true;
		if(EUI[0]==0 && EUI[1]==0 && EUI[2]==0 && EUI[3]==0 && EUI[4]==0 && EUI[5]==0 && EUI[6]==0 && EUI[7]==0)
			__saveData=false;
		if(__saveData)
			fprintf(file, "%s", file_stream);
	}
    
	// Close the file	
	fclose(file);
	printf("%s", file_stream);//printf(file_stream);
   	 printf("Data has been appended to %s\n",_fileNameEUI.c_str());

    return EXIT_SUCCESS;

}







//int SaveResult(bool testingNoCap,std::string _fileNameEUI,std::string _fileNameTest)
int SaveResult(std::string _fileNameTest){
	//char fileName[80];
	char file_stream[256];	
	std::string cFileName;
	cFileName=_fileNameTest;
    FILE *file = fopen(cFileName.c_str(), "r+");
    // Check if file exists; if not, create it in write mode
    if (file == NULL) {
        file = fopen(cFileName.c_str(), "w+");
        if (file == NULL) { perror("Error creating file"); return EXIT_FAILURE; }
		sprintf(file_stream, "NUM,EUI-M,EUI,Device,Date,Time,VCapMaxs,VCap,T.charge,T.DisCharge,T.WaitToSWoff,T.SWOff,Load,VOut1,VOUT2,IC-Temp,Repaired_NoCap,Repaird_Cap,Desc\n");
		fprintf(file, "%s", file_stream);
        printf("The file was not found, so a new file has been created.\n");
    }
	// Check if the file is empty by moving the pointer to the end and checking its position
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);

    int line_count = 0;
    //if (file_size > 0) {
    //    line_count = count_lines(file,true,cFileName,NULL);
    //} else {
    //    printf("The file is empty. Starting line count from 0.\n");		
    //}
	int __tmp1=0;
	line_count = count_lines(file,false,cFileName,&__tmp1);
	std::time_t Savetime_now = std::time(nullptr); // Get current time as a time_t object
    std::tm *SaveTime = std::localtime(&Savetime_now); // Convert to local time    
	myBoard.updateBoardNameStr();
	uint8_t EUI[8];
	memcpy(EUI,myBoard.myEEPROM.myData.EUI,8);
	sprintf(file_stream, "%04d,%02X%02X%02X,%02X%02X%02X%02X%02X,%s,20%02d-%02d-%02d,%02d:%02d:%02d"
				,line_count
				,EUI[0],EUI[1],EUI[2],EUI[3],EUI[4],EUI[5],EUI[6],EUI[7]
				,myBoard.boardName_str
				,SaveTime->tm_year%100,SaveTime->tm_mon+1,SaveTime->tm_mday,SaveTime->tm_hour,SaveTime->tm_min,SaveTime->tm_sec);
	sprintf(&file_stream[strlen(file_stream)], ",%.1fV,%.1fV,%dsec,%dsec,%dsec,%dsec,%.3fA,%.1fV,%.1fV,%.1f°C,_,%d,%d\n", 
				testr.Vcap_Max, testr.VCap_SWOff, 
				testr.time_charge, testr.time_DisCharge,testr.time_WaitToOutSwOff, testr.time_OutSwOff, 
				testr.Load_Current, testr.VOut1, testr.VOut2, 
				testr.tempIC,
				testr.repaired_Cap,testr.ErrorNo);

	fprintf(file, "%s", file_stream);
	// Close the file	
	fclose(file);
	printf("%s", file_stream);//printf(file_stream);
   	 printf("Data has been appended to %s\n",cFileName.c_str());
    return EXIT_SUCCESS;
}


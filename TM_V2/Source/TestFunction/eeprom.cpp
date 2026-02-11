#include "eeprom.h"
uint16_t eeprom::calculate_crc16(const uint8_t *data, size_t length) {
    uint16_t crc = CRC_INITIAL;
    for (size_t i = 0; i < length; i++) {
        crc ^= (data[i] << 8); // XOR the byte into the high byte of the CRC

        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ CRC_POLYNOMIAL;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}
uint8_t eeprom::bcd_to_decimal(uint8_t bcd) { return ((bcd >> 4) * 10) + (bcd & 0x0F); }
bool eeprom::isEmpty(){
    for(int i=0;i<32;i++)
        if(EEPROMDataBuffer[i]!=0xFF)
            return false;
    return true;
}
uint8_t eeprom::getVersion(){
    //Version 1
    if(EEPROMDataBuffer[0]==0x20){
        uint16_t tt=EEPROMDataBuffer[8];
        tt*=0x100;
        tt+=EEPROMDataBuffer[9]&0xFF;
        if(tt==0x2405 || tt==0x2315){
            if((EEPROMDataBuffer[10]&0xF0)==0xA0){
                return 1;
            }
        }
    }
    //Other Versions 
    if(memcmp(EEPROMDataBuffer, "ICA", 3) == 0){
            return EEPROMDataBuffer[4];
    }
    return 0;
}
void eeprom::getVer1Data(){
    myData.dataVersion=1;
    struct tm timeinfo = {0};
    timeinfo.tm_year = 100 + bcd_to_decimal(EEPROMDataBuffer[1]);  // Years since 1900
    timeinfo.tm_mon = bcd_to_decimal(EEPROMDataBuffer[2]) - 1;     // Months are 0-based in struct tm
    timeinfo.tm_mday = bcd_to_decimal(EEPROMDataBuffer[3]);        // Day of the month
    timeinfo.tm_hour = bcd_to_decimal(EEPROMDataBuffer[4]);        // Hour of the day
    timeinfo.tm_min = bcd_to_decimal(EEPROMDataBuffer[5]);         // Minute of the hour
    timeinfo.tm_sec = 0; 
    myData.testTime=mktime(&timeinfo);
    if(EEPROMDataBuffer[8]==0x24 && EEPROMDataBuffer[9]==0x05) myData.boardType=2405;
    if(EEPROMDataBuffer[8]==0x23 && EEPROMDataBuffer[9]==0x15) myData.boardType=2315;
    myData.boardVer=EEPROMDataBuffer[10];
    myData.RSNSI1=EEPROMDataBuffer[11];
    myData.RSNSI2=EEPROMDataBuffer[12];
    myData.RSNSC=EEPROMDataBuffer[13];
    myData.RTST=EEPROMDataBuffer[14];
    myData.RT=EEPROMDataBuffer[15];
    myData.VshutDown=((float)EEPROMDataBuffer[16])/10;
    myData.supperCapVoltage=((float)EEPROMDataBuffer[17])/10;
    myData.capacitorValue=EEPROMDataBuffer[18];
    myData.CapNo=EEPROMDataBuffer[19];
}
bool eeprom::getVer2Data(){
    uint16_t crc= calculate_crc16(EEPROMDataBuffer,27);
    if(crc!=0){
        printf("CRC ERROR !\n");
        return false;
    }
        
    myData.name[0]=EEPROMDataBuffer[0];
    myData.name[1]=EEPROMDataBuffer[1];
    myData.name[2]=EEPROMDataBuffer[2];
    myData.dataVersion=EEPROMDataBuffer[4];
    myData.boardType=(uint16_t)(EEPROMDataBuffer[5])*0x100;;
    myData.boardType+=(uint16_t)(EEPROMDataBuffer[6])&0xFF;
    myData.boardVer=EEPROMDataBuffer[7];
    #ifdef time_sizeX64
        myData.testTime= (static_cast<time_t>(EEPROMDataBuffer[19]) << 56) |
                        (static_cast<time_t>(EEPROMDataBuffer[18]) << 48) |
                        (static_cast<time_t>(EEPROMDataBuffer[17]) << 40) |
                        (static_cast<time_t>(EEPROMDataBuffer[16]) << 32) |
                        (static_cast<time_t>(EEPROMDataBuffer[15]) << 24) |
                        (static_cast<time_t>(EEPROMDataBuffer[14]) << 16) |
                        (static_cast<time_t>(EEPROMDataBuffer[13]) << 8) |
                        static_cast<time_t>(EEPROMDataBuffer[12]);
    #else
        myData.testTime= (static_cast<time_t>(EEPROMDataBuffer[15]) << 24) |
                        (static_cast<time_t>(EEPROMDataBuffer[14]) << 16) |
                        (static_cast<time_t>(EEPROMDataBuffer[13]) << 8) |
                        static_cast<time_t>(EEPROMDataBuffer[12]);
    #endif
    
    myData.RSNSI1=EEPROMDataBuffer[16];
    myData.RSNSI2=EEPROMDataBuffer[17];
    myData.RSNSC=EEPROMDataBuffer[18];
    myData.RTST=EEPROMDataBuffer[19];
    myData.RT=EEPROMDataBuffer[20];
    myData.VshutDown=((float)EEPROMDataBuffer[21])/10;
    myData.supperCapVoltage=((float)EEPROMDataBuffer[22])/10;
    myData.capacitorValue=EEPROMDataBuffer[23];
    myData.CapNo=EEPROMDataBuffer[24];
    return true;
}
//public:	
bool eeprom::ismicroChipIC(){
    if(myData.EUI[0]==microChip_ID[0] && myData.EUI[1]==microChip_ID[1] && myData.EUI[2]==microChip_ID[2])
        return true;
    printf("unKnown EEPROM IC!!!!\n");
    return false;
}
void eeprom::ShowEUI(){ printf("\n %s \n\n", myData.getEUI_Str().c_str()); }        
void eeprom::EEPROMDataBuffShow(){            
    for(int i=0;i<EEPROMDateBufferLen;i++){
        printf(" Buff.%d: ",i+1);
        for(int j=0;j<8;j++){
            printf("%02X, ",EEPROMDataBuffer[i*8+j]);
        }
        printf("\n");
    }
}  
void eeprom::clear_EEPROM_Buffer(){
    for(int i=0;i<32;i++)
        EEPROMDataBuffer[i]=0xFF;
}
bool eeprom::getLastVerData(){
    uint8_t __len=27+2;
    uint16_t crc= calculate_crc16(EEPROMDataBuffer,__len);
    if(crc!=0){
        printf("CRC ERROR !\n");
        return false;
    }            
    myData.name[0]=EEPROMDataBuffer[0];
    myData.name[1]=EEPROMDataBuffer[1];
    myData.name[2]=EEPROMDataBuffer[2];
    myData.dataVersion=EEPROMDataBuffer[4];
    myData.boardType=(uint16_t)(EEPROMDataBuffer[5])*0x100;;
    myData.boardType+=(uint16_t)(EEPROMDataBuffer[6])&0xFF;
    myData.boardVer=EEPROMDataBuffer[7];
    #ifdef time_sizeX64
        myData.testTime= (static_cast<time_t>(EEPROMDataBuffer[19]) << 56) |
                        (static_cast<time_t>(EEPROMDataBuffer[18]) << 48) |
                        (static_cast<time_t>(EEPROMDataBuffer[17]) << 40) |
                        (static_cast<time_t>(EEPROMDataBuffer[16]) << 32) |
                        (static_cast<time_t>(EEPROMDataBuffer[15]) << 24) |
                        (static_cast<time_t>(EEPROMDataBuffer[14]) << 16) |
                        (static_cast<time_t>(EEPROMDataBuffer[13]) << 8) |
                        static_cast<time_t>(EEPROMDataBuffer[12]);
    #else
        myData.testTime= (static_cast<time_t>(EEPROMDataBuffer[15]) << 24) |
                        (static_cast<time_t>(EEPROMDataBuffer[14]) << 16) |
                        (static_cast<time_t>(EEPROMDataBuffer[13]) << 8) |
                        static_cast<time_t>(EEPROMDataBuffer[12]);
    #endif
    
    myData.RSNSI1=EEPROMDataBuffer[16];
    myData.RSNSI2=EEPROMDataBuffer[17];
    myData.RSNSC=EEPROMDataBuffer[18];
    myData.RTST=EEPROMDataBuffer[19];
    myData.RT=EEPROMDataBuffer[20];
    myData.VshutDown=((float)EEPROMDataBuffer[21])/10;
    myData.supperCapVoltage=((float)EEPROMDataBuffer[22])/10;
    myData.capacitorValue=EEPROMDataBuffer[23];
    myData.CapNo=EEPROMDataBuffer[24];
    myData.CapType=EEPROMDataBuffer[25];
    myData.Temp85MaxV=EEPROMDataBuffer[26];        
    return true;
}
void eeprom::BuffUpdate_LVer(){
    uint8_t __len;
    
    __len=27;
    myData.dataVersion=3;
    myData.name[0]='I';
    myData.name[1]='C';
    myData.name[2]='A';
    EEPROMDataBuffer[0]=myData.name[0];
    EEPROMDataBuffer[1]=myData.name[1];
    EEPROMDataBuffer[2]=myData.name[2];
    EEPROMDataBuffer[3]=__len;
    EEPROMDataBuffer[4]=myData.dataVersion;// Data Version
    EEPROMDataBuffer[5]=(uint8_t)(myData.boardType/0x100);//BoardType B1
    EEPROMDataBuffer[6]=(uint8_t)(myData.boardType&0xFF);//BoardType B2
    EEPROMDataBuffer[7]=myData.boardVer; //Hardware Version
    EEPROMDataBuffer[8]=0; EEPROMDataBuffer[9]=0; EEPROMDataBuffer[10]=0; EEPROMDataBuffer[11]=0;
    EEPROMDataBuffer[12]=0; EEPROMDataBuffer[13]=0; EEPROMDataBuffer[14]=0; EEPROMDataBuffer[15]=0;
    #ifdef time_sizeX64
        memcpy(&EEPROMDataBuffer[8], &myData.testTime, 8);
    #else
        memcpy(&EEPROMDataBuffer[12], &myData.testTime, 4);
    #endif
    EEPROMDataBuffer[16]= myData.RSNSI1;//RSNSI1
    EEPROMDataBuffer[17]= myData.RSNSI2;//RSNSI2
    EEPROMDataBuffer[18]= myData.RSNSC; //RSNSC
    EEPROMDataBuffer[19]= myData.RTST;//RTST
    EEPROMDataBuffer[20]= myData.RT;//RT
    EEPROMDataBuffer[21]= (uint8_t)(myData.VshutDown*10);//Vcap. Sw. Off (4.0V)
    EEPROMDataBuffer[22]= (uint8_t)(myData.supperCapVoltage*10);//SupperCapVoltage(3.0V)
    EEPROMDataBuffer[23]= myData.capacitorValue;//SuperCap Single Cap
    EEPROMDataBuffer[24]= myData.CapNo;//SupperCap Number
    EEPROMDataBuffer[25]= myData.CapType;//SupperCap Number
    EEPROMDataBuffer[26]= myData.Temp85MaxV;//SupperCap Number
    
    uint16_t crc = calculate_crc16(&EEPROMDataBuffer[0], __len);
    EEPROMDataBuffer[__len]=crc/0x100;
    EEPROMDataBuffer[__len+1]=crc&0x0FF;
}
void eeprom::getCurrentTime(){ myData.testTime = std::time(nullptr); }
void eeprom::updateBoardInfo(int _boardName, int _boardVer, boardInfo _dataBI, testResult _tr){
    myData.RSNSI1=_dataBI.LTC3350_RSNSI1*1000;
    myData.RSNSI2=_dataBI.LTC3350_RSNSI2*1000;
    myData.RSNSC=_dataBI.LTC3350_RSNSC*1000;
    myData.RTST=_dataBI.LTC3350_RTST;
    myData.RT=_dataBI.LTC3350_RT/1000;
    myData.CapNo=_dataBI.Board_SupperCapNum;
    myData.CapType=_dataBI.Board_SupperCapType;
    myData.capacitorValue=_dataBI.Board_SupperCapSingleCap;
    myData.supperCapVoltage=_dataBI.Board_SupperCapVoltage;
    myData.boardType=_boardName;
    myData.boardVer=_boardVer;
    myData.VshutDown = _tr.VCap_SWOff;
    myData.CapType=_dataBI.Board_SupperCapType;
    myData.Temp85MaxV=_dataBI.Board_MaxTemp85V;

}
uint8_t eeprom::CheckDataVersionProcess(bool debugShow){
    uint8_t __version=0;
    if(ismicroChipIC()==false){
        printf(" !!! EEPROM Check !!! Wrong IC (is not microChip IC).\n");
        return EEPROMProcesSts::WrongIC;
    } 
    if(isEmpty())
    {
        printf(" !!! EEPROM Check !!! Empty.\n");
        return EEPROMProcesSts::EmptyEEPROM;
    }
    __version=getVersion();
    if(__version!= EEPROM_LastVersion)// is not Last version
    {
        printf(" !!! EEPROM-DataStruct Version: %d !!!\n",__version);
        if(debugShow) EEPROMDataBuffShow();
        switch(__version){
            case 1: getVer1Data(); break;
            case 2: getVer2Data(); break;
            default: printf(" !!!!!!!!!!!!!!!!!!!!! %d is unKnown Version. !!!!!!!!!!!!!!!!!!!!!\n",__version); break;
        }
        if(debugShow) myData.show();
        
        if (__version==1) myData.VshutDown=4.4;// data is not right in this register
        if (__version==2) { 
            myData.CapType=convertV2toV3.CapType;
            myData.Temp85MaxV=convertV2toV3.Temp85MaxV;
        }                
        BuffUpdate_LVer();
        printf("\nData converted to Last Version\n");
        if(debugShow) EEPROMDataBuffShow();
        if(getLastVerData()){
            if(debugShow) myData.show();
            return EEPROMProcesSts::ConvertedToLastVer;
        }
        return EEPROMProcesSts::Failed;
        
    }
    else
    {
        printf(" !!! EEPROM-DataStruct Version: 3 !!!\n");                
        if(getLastVerData())
        {
            if(debugShow) EEPROMDataBuffShow();
            if(debugShow)myData.show();
            return EEPROMProcesSts::Ok;
        }
        return EEPROMProcesSts::Failed;
    }

}		

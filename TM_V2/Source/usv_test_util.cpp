#include "usv_test_util.h"
#include "./TestFunction/ICA_justEUI.h"
#include <filesystem>
extern LabDevice MyLabDevice;
extern durationTimerClass myDurationTimer;
extern testResult myTestResult;
extern _interact_registers myInterActReg;
ConsoleKeyClass myCKey;



void USV_TEST_UTIL_V2::showLog(std::string _str){
            //printf("%s\n",_str.c_str());
            myInterActReg.csLogWrite(_str);
        }
void USV_TEST_UTIL_V2::checkLabDevice(){
    myArg.LabDevice_PS=false;
    myArg.LabDevice_Load=false;
    //myTestDevice.cleanLCD();
    //myTestDevice.showOnLCD(1,"Lab Devices:");
   showLog("Finding LabDevice.PowerSupply... ");
    if (MyLabDevice.PSupply_INI((char*)"RS-6005",B9600)){
        //myTestDevice.showOnLCD(2,"P-Supply Ok");
        myArg.LabDevice_PS=true;
        
    }else{
       showLog(" !!! LabDevice.PowerSupply not found !!!");
        //myTestDevice.showOnLCD(2,"P-Supply Failed!");
    }
   showLog("Finding LabDevice.Load... ");
    if (MyLabDevice.Load_INI((char*)"RS-KEL102",B9600)){
        //myTestDevice.showOnLCD(3,"E-Load Ok");
        myArg.LabDevice_Load=true;        
    }else{
       showLog(" !!! LabDevice.E-Load not found !!!");
        //myTestDevice.showOnLCD(3,"E-Load Failed!");
    }
}
void USV_TEST_UTIL_V2::getArg(int argc, char* argv[]){
    char cArg[128];
    char* lArgv;
    bool __process;
    //sprintf(myArg.ttyName,"/dev/ttyAMA4");
    while (argc-- > 0)
    {
        strcpy(cArg, argv[argc]);
		for (int i = 0; i < (int)(strlen(cArg)); i++) {
			cArg[i] = toupper(cArg[i]);
		}
        __process=false;  
              
        //if (strcmp(cArg, "-2315") == 0) {myArg.manual_Dongle=0x12;__process=true;}
        //if (strcmp(cArg, "-2405") == 0) {myArg.manual_Dongle=0x08;__process=true;}

        if (strcmp(cArg, "-ERRORLIST") == 0) {myArg.showErrorList=true;__process=true;}
        if (strcmp(cArg, "-NORMAL") == 0) {myArg.normalTest=true;myArg.manualTest=false;__process=true;}
        if (strcmp(cArg, "-MONITOR") == 0) {myArg.normalTest=false;myArg.manualTest=true;__process=true;}
        if (strcmp(cArg, "-MONITORALL") == 0) {myArg.normalTest=false;myArg.manualTest=true;myArg.showAll=true;__process=true;}
        //if (strcmp(cArg, "-LABDEVICE") == 0) {myArg.useLabDevice=true;__process=true;}
        if (strcmp(cArg, "-EEPROMCFG") == 0) {myArg.EEPROMCFG=true;__process=true;}
        if (strcmp(cArg, "-WORKSPACE") == 0) {myArg.workSpace=std::string(lArgv);__process=true;}
        if (strcmp(cArg, "-PORT") == 0) {sprintf(myArg.ttyName,"%s",lArgv);__process=true;}
        printf("Argument (%d): %s\n",__process,cArg);
        lArgv = argv[argc];      
    }
}
bool USV_TEST_UTIL_V2::Init(){
    myTestDevice.Init();   
    return true;     
}
bool USV_TEST_UTIL_V2::SelectBoard(uint8_t _dongle, float _version){
    //uint8_t _Board_type= _dongle;//(_dongle & 0xF8)>>3;
    //uint8_t _Board_Ver = _version;//(_dongle & 0x07);
    switch (_dongle)
    {
            
    case 1://2405
    case 5://ICA2506
    case 6://ICA2510
        myBoard.boardVer=0;            
        myBoard.myBoardInfo.LTC3350_RSNSI1=0.016;
        myBoard.myBoardInfo.LTC3350_RSNSI2=0.016;
        myBoard.myBoardInfo.LTC3350_RSNSC=0.003;
        myBoard.myBoardInfo.LTC3350_RTST=121;
        myBoard.myBoardInfo.LTC3350_RT=107000;        
        myBoard.myBoardInfo.Board_SupperCapVoltage = 3.0;//3.0V
        myBoard.myBoardInfo.Board_SupperCapSingleCap = 50;//50.0F
        myBoard.myBoardInfo.Board_SupperCapNum = 4;//3.0V
        myBoard.myBoardInfo.Board_SupperCapType=255;//XXXX
        myBoard.myBoardInfo.Board_MaxTemp85V=22;//2.2V    
        myBoard.myBoardInfo.Board_VShutdownVoltage=0;
        if(_dongle==ICA_NT_USV_2405){    
            myBoard.boardName=2405;
            sprintf(myBoard.boardKind_str,"NT-CLX USV");
            myBoard.boardType=ICA_NT_USV_2405;
            switch ((uint16_t)_version*100)
            {
                case 161:
                myBoard.boardVer=0xA6;
                myBoard.myBoardInfo.Board_SupperCapType=ICA_CapType_2405_1;
                myBoard.myBoardInfo.Board_MaxTemp85V=26;    
                break;
                case 162:            
                myBoard.boardVer=0xA6;
                myBoard.myBoardInfo.Board_SupperCapType=ICA_CapType_2405_2;
                myBoard.myBoardInfo.Board_MaxTemp85V=26;
                break;            
            }            
        }
        if(_dongle==ICA_2506){
            myBoard.boardName=2506;            
            sprintf(myBoard.boardKind_str,"NT-CLX USV Pro");
            myBoard.boardType=ICA_2506;            
            switch ((uint16_t)_version*100)
            {
                case 100:
                myBoard.boardVer=0x10;
                myBoard.myBoardInfo.Board_SupperCapType=ICA_CapType_2405_1;
                myBoard.myBoardInfo.Board_MaxTemp85V=26;    
                break;
            }            
        }
        if(_dongle==ICA_2510){
            myBoard.boardName=2510;           
            myBoard.myBoardInfo.LTC3350_RT=105000; 

            sprintf(myBoard.boardKind_str,"MB-PSU-MCU");
            myBoard.boardType=ICA_2510;
            myBoard.boardVer=(uint16_t)(_version*100);
            switch (myBoard.boardVer)
            {
                case 100:
                case 110:
                case 120:
                default:
                myBoard.myBoardInfo.Board_SupperCapSingleCap = 33;//33F
                myBoard.myBoardInfo.Board_SupperCapType=ICA_CapType_2510_1;
                myBoard.myBoardInfo.Board_MaxTemp85V=26;   
                myBoard.myBoardInfo.Board_VShutdownVoltage=4.2;//4.2V 
                break;
            }
        }
    break;
    case 2://2315
        myBoard.boardName=2315;
        sprintf(myBoard.boardKind_str,"NT-CLS USV");
        myBoard.boardType=ICA_NT_USV_2315;
        myBoard.boardVer=0;
        myBoard.myBoardInfo.LTC3350_RSNSI1=0.006;
        myBoard.myBoardInfo.LTC3350_RSNSI2=0.012;
        myBoard.myBoardInfo.LTC3350_RSNSC=0.003;
        myBoard.myBoardInfo.LTC3350_RTST=5;//for 50 ohm External load +121R  
                                //10.7 for 100 ohm external Load
        myBoard.myBoardInfo.LTC3350_RT=107000;
        
        myBoard.myBoardInfo.Board_SupperCapVoltage = 3.0;//3.0V
        myBoard.myBoardInfo.Board_SupperCapSingleCap = 150;//50.0F
        myBoard.myBoardInfo.Board_SupperCapNum = 4;//3.0V
        myBoard.myBoardInfo.Board_SupperCapType=255;//XXXX
        myBoard.myBoardInfo.Board_MaxTemp85V=22;//2.2V
        myBoard.boardVer=(static_cast<int>(_version) << 4) | static_cast<int>((_version - static_cast<int>(_version)) * 10 + 0.5);
        
        switch ((uint16_t)(_version*100))
        {
            case 251:
                myBoard.boardVer=0x25;
                myBoard.myBoardInfo.Board_SupperCapType=ICA_CapType_2315_1;
                myBoard.myBoardInfo.Board_MaxTemp85V=27;   
            break;
            case 252:            
                myBoard.boardVer=0x25;
                myBoard.myBoardInfo.Board_SupperCapType=ICA_CapType_2315_2;
                myBoard.myBoardInfo.Board_MaxTemp85V=25;
            break;            
            case 261:
                myBoard.boardVer=0x26;
                myBoard.myBoardInfo.Board_SupperCapType=ICA_CapType_2315_1;
                myBoard.myBoardInfo.Board_MaxTemp85V=27;   
            break;
            
        }
        break;
    case 3: //ica2308R1.2
        myBoard.boardName=2308;
        sprintf(myBoard.boardKind_str,"Linux Base");
        sprintf(myBoard.boardName_str,"2308");        
        myBoard.boardType=ICA_2308;
        //myBoard.boardVer=(static_cast<int>(_version) << 4) | static_cast<int>((_version - static_cast<int>(_version)) * 10 + 0.5);
        myBoard.boardVer=(uint16_t)(_version*100);
    break;
    case 4: //ICA2407
        myBoard.boardName=2407;a
        sprintf(myBoard.boardKind_str,"IBIS Slave");
        sprintf(myBoard.boardName_str,"2407");        
        myBoard.boardType=ICA_2407;
        myBoard.boardVer=(uint16_t)(_version*100);        
    break;
    default:
        myBoard.boardName=0;
        myBoard.boardVer=0;
        myBoard.boardType=9;
        myBoard.myBoardInfo.Board_SupperCapType=0;
        myBoard.myBoardInfo.Board_MaxTemp85V=0;
        return false;
        break;
    }
    return true;
}
uint8_t USV_TEST_UTIL_V2::showError(uint8_t _errorNo){
    std::string _strError1="", _strError2="",_strError3="";
    std::string _strDesc="--";

    myTestResult.ErrorNo=_errorNo;
    _strError1="Error : "+std::to_string(_errorNo);
    switch (_errorNo)
    {
    case ERROR::Aufruesten:         _strError2="AR.Off Failed!"; _strDesc="there is voltage when AR is Disable\n\t*check optoCoupler";break;
    case ERROR::AufruestenOn:       _strError2="AR.On Failed"; _strDesc="No Voltage when AR is Enable\n\t*check AR-Fuse, eFuse, OptoCoupler, flyBack"; break;
    case ERROR::VCCIsLow:           _strError2="VCC is LOW";_strDesc="VCC is Lower then Min \n\t *check 24V and DC-DC convertor";break;
    case ERROR::VCCIsOver:          _strError2="VCC Over Voltage";_strDesc="VCC is Lower then Min \n\t *check DC-DC convertor";break;
    
    case ERROR::BoardConnection1:   _strError2="UART-No Connect";_strDesc="UART Connection failed\n\t* check UART Buffer and Bridge(SC18IM)"; break;
    case ERROR::BoardConnection2:   _strError2="EEPROM not Matched";_strDesc="Wrong EUI value (it is not Micro chip Fammily)"; break;
    case ERROR::BoardConnection3:   _strError2="EEPROM Read-Err";_strDesc="EEPROM read Error\n\t * check EEPROM IC and I2c Connection"; break;
    case ERROR::ChargeDuration:     _strError2="Charge Time Err";_strDesc="Long SCaps charge Time"; break;//EEPROM Read Failed!
    //case ERROR::GPIO:               sprintf(_str2,"GPIO failed!    ");break;
    case ERROR::FlyBackdis:         _strError2="FlyBack-Dis!";_strDesc="FlyBack not disable with UART Command";break;
    case ERROR::FlyBackEn:          _strError2="FlyBack En.!";_strDesc="flyBack not enable again with UART Command";break;
    case ERROR::waitToOutSwOff:     _strError2="Wait-Out-SW off!";_strDesc="output switch has not in time Disable";break;
    case ERROR::OutSwOff:           _strError2="Out-SW off!";_strDesc="output switch has not temporary Disable";break;
    case ERROR::EEPROMWrite:        _strError2="*EEPROMWrite";_strDesc="EEPROM ERROR";break;
    case ERROR::VCapsNotSame:        _strError2="VCaps not Same";_strDesc="Caps Vol Error";break;    
    case ERROR::VCapShutDownOutOfRange: _strError2="VCap Shutdown Err";_strDesc="VCap at Shutdown is out of range";break;
    case ERROR::TempSensor_IC:      _strError2=("*TempSensor "+std::to_string((int16_t)myTestResult.tempIC)+"°c");_strDesc="High IC-Temperature";break;
    case ERROR::TempSensor_CapBank: _strError2=("*TempSensor "+std::to_string((int16_t)myTestResult.tempBatBank)+"°c");_strDesc="High CapBank-Temperature";break;
    case ERROR::LabelPrintError:    _strError2="*LabelPrintError";_strDesc="Label Print not found! \n\t Check that the printer is installed correctly.";break;
    case ERROR::LabPSNoAnswer:      _strError2="*Lab.PS No Answer";_strDesc="Lab.PS No Answer \n\t*check USB Connection";break;
    case ERROR::LabLoadNoAnswer:    _strError2="*Lab.Load No Answer";_strDesc="Lab.Load No Answer \n\t*check USB Connection";break;
    case ERROR::I2CFailed:          _strError2="I2C Not Found";_strDesc="I2C Connection failed\n\t* check I2C-BUS with I2Cdetect command"; break;
    
    //case ERROR::FirstTry:           sprintf(_str2,"*FirstTry");_strDesc="...";break;
    //case ERROR::Dongle:             sprintf(_str2,"unKnown Dungle!"); _strDesc="put right DP9-Dongle in ... ";break;
    //case ERROR::TestDeviceConnection: sprintf(_str2,"TDev-CON Read-Err");_strDesc="..."; break;
    //case ERROR::DisChargeDuration:  sprintf(_str2,"*DisChargeDuration");_strDesc="...";break;
    //case ERROR::GPIO:               sprintf(_str2,"*GPIO");_strDesc="...";break;
    //case ERROR::VCapsNotSame:       sprintf(_str2,"*VCapsNotSame");_strDesc="...";break;
    //case ERROR::LoadConnection:     sprintf(_str2,"*LoadConnection");_strDesc="...";break;
    //case ERROR::PSConnection:       sprintf(_str2,"*PSConnection");_strDesc="...";break;
    //case ERROR::StopJustForTest:    sprintf(_str2,"*StopJustForTest");_strDesc="...";break;
    default: 
        if(_errorNo>=100 && _errorNo<130)
            _strError2="GPIO failed!    ";
        else
            _strError2="unKnown Error!"; 
        break;
    }    
    if(myArg.showErrorList==false){
        _strError3= "S-Nr:"+ myBoard.myEEPROM.myData.getEUI5Byte_Str();
        showLog(_strError1+" "+_strError2+" "+_strDesc);    
        printErrorLabel(_strError1,_strError2,_strError3);
        //myTestDevice.setLED(FronLED,true);
    }else{
        showLog(_strError1+" "+_strError2+" "+_strDesc);                    
        showLog("\n");            
    }
    myInterActReg.TR.errorStr1=_strError1;
    myInterActReg.TR.errorStr2=_strError2+" "+_strDesc;
    return 0xFF;
}
void USV_TEST_UTIL_V2::InformationMenu(){
    char _str[32];
    uint8_t _StepNo=0;
    uint8_t _key=0;
    myTestDevice.cleanLCD();
    while(1){
        switch (_StepNo)
        {
        case 0 : // First Page
            sprintf(_str,"Information");myTestDevice.showOnLCD(1,_str);
            sprintf(_str,"Menu.");myTestDevice.showOnLCD(2,_str); 
            myTestDevice.showOnLCD(4,"               Next>");                     
            sleep(.5);
        break;
        case 1:ShowMyName();break;
        case 2: //Show Input Current Information
            myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
                        
            if(myArg.LabDevice_PS || myArg.LabDevice_Load){
                if(myArg.LabDevice_PS) myTestDevice.setRelay(USV_Test_Interface::Relays::LabPowerSel,true);
                MyLabDevice.SetPSEnable(true);
                MyLabDevice.SetPSVoltage(__const_PSVoltage);
                MyLabDevice.SetPSCurrent(__const_PSCurrent);    
            }
            if(myArg.LabDevice_Load) MyLabDevice.SetLoadCurrent(myTestResult.Load_Current);	

            myTestDevice.setRelay(USV_Test_Interface::Relays::AR,true);            
            myTestDevice.setRelay(USV_Test_Interface::Relays::MPower,true);
            myTestDevice.setRelay(USV_Test_Interface::Relays::Load,true);

            _StepNo++;            
        break;
        case 3: myTestDevice.ShowAnalogInfo(); break;
        case 4: myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);_StepNo++; break;
        case 5: checkLabDevice();_StepNo++; break;
        case 6: break;
        case 7: 
            if(myArg.LabDevice_Load) MyLabDevice.SetLoadCurrent(myTestResult.Load_Current);	
            myTestDevice.setRelay(USV_Test_Interface::Relays::Load,true); 
            myTestDevice.setRelay(USV_Test_Interface::Relays::VCCLoad,true);
            _StepNo++; break;
        case 8: myTestDevice.ShowDischargeInfo(); break;
        default:    
            myTestDevice.cleanLCD();
            _StepNo=0;
        break;
        }
        _key=myTestDevice.readKeys();
        switch (_key)
        {
            case USV_Test_Interface::_KEY::K1 : _StepNo++; myTestDevice.cleanLCD(); break;
            case USV_Test_Interface::_KEY::K2 : _StepNo--; myTestDevice.cleanLCD(); break;
            case USV_Test_Interface::_KEY::K3 : 
                myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
                myTestDevice.cleanLCD(); 
                return; 
        }
        if(_key!=0)
            while(myTestDevice.readKeys()!=0);
    }
    
}
void USV_TEST_UTIL_V2::DongleCheck(){
    //if(myArg.manual_Dongle!=0)
    //    Dongle=myArg.manual_Dongle;
    //else
    //    Dongle=myTestDevice.readDongle();
    //SelectBoard(Dongle,myInterActReg.board_version);
    SelectBoard(myInterActReg.DongleID,myInterActReg.board_version);
    myInterActReg.Dongle= DongleNameStr();
    
}
void USV_TEST_UTIL_V2::forceStop(void){
    std::cout<<"Force Stop! (setRelayAllOff)"<<std::endl;
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
    //myTestDevice.cleanLCD();
}


#define _mState_EEPROMCFG 20
void USV_TEST_UTIL_V2::runICA2407_simple_test(){
    ICA_justEUI myICA2407;
    //std::ofstream file;
    uint8_t _mState = 0, LMState=0xFF;
    uint16_t _dcnt100ms=0;
    //float __floatTmpVale=0;
    showLog("******************************");
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);            
    DongleCheck();  
    ShowMyDongle();
    
    //myArg.FileName_EUI="ICA-2407-EUI.csv";
    
    while((_mState<0xF0) && (xrunning==true)){        
        usleep(100000);
        //myTempVal.VIn= myTestDevice.getDUT_VIN();
        //myTempVal.InCurrent = myTestDevice.getDUT_VINAmp();
        
        switch (_mState){ 
            case 0://clear registers & reset Relays *********************************************
                myInterActReg.TR.DataClear();                    
                removeJPG_PFiles_Jobs();
                myTempVal.clear();
                myBoard.myEEPROM.clear_EEPROM_Buffer();
                myBoard.myEEPROM.myData.clear();
                //myTestResult.clear();
                //testrReset(myBoard.boardName);
                showLog("ICA2407 ReadEUI");
                showLog("Start: Reset Relays and Test start.");
                myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);                
            _mState++;
            break;           
            case 1:                 
                myTestResult.clear(myBoard.boardName);
                showLog(" Ok.");
                _mState++;
            break;
            case 2://TEST2 : Check EEPROM & serial Connection *********************************************
                showLog("Check EEPROM & serial Connection");
                sleep(1);
                //system("i2cdetect -y 3");
                _dcnt100ms=5;
                _mState++;
            break;
            case 3:
                if(_dcnt100ms-->0){
                    if(myICA2407.ReadEUI_EEPROM()){
                        _mState++;
                    }
                    else{
                        showLog("ReadEUI Failed!!!!!!!!!");
                    }
                }
                else
                {
                        printf(" Failed to I2C Connection! \n"); 
                        _mState=showError(ERROR::I2CFailed);
                }
				break;
            case 4:
                for(int i=0;i<8;i++)
                    myBoard.myEEPROM.myData.EUI[i]=myICA2407.EUI_Buffer[i];
                myBoard.myEEPROM.ShowEUI(); //just for test  
                myInterActReg.TR.EUI_str=myBoard.myEEPROM.myData.getEUI_Str();
                showLog(myInterActReg.TR.EUI_str+"\n");
                _mState++;                  
            break;            
            case 5:
                _mState=25;
            break;
            case 25:// Label Print
                showLog("Label Print... ");
                _mState++;        
                break;
            case 26:
                char tmp1[50];
                char tmp2[50];
                myBoard.updateBoardNameStr();
                sprintf(myBoard.boardName_str,"2407A0");
                sprintf(myBoard.boardKind_str,"IBIS Slave");
                sprintf(tmp1,"S-Nr:%s",myBoard.myEEPROM.myData.getEUI5Byte_Str().c_str());
                sprintf(tmp2,"ICA%s",myBoard.boardName_str);
                qrcode_jpeg_output(myBoard.myEEPROM.myData.getEUI5Byte_Str().c_str(),myBoard.boardKind_str,tmp2,tmp1);
                
                if(QL700_Print()==0){                        
                    QL700_Print();
                    printf(" Ok.\n");
                    _mState++; 
                }
                else{
                    printf(" Error.\n");
                    _mState=showError(ERROR::LabelPrintError);
                }
            break;
            case 27:
		            myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
                    myTestDevice.cleanLCD();
                    _mState++;
            break;
            default: 
                showLog("END Wait to press Key.");
                _mState=0xFE;
                break;
        
        }
        
        myTempVal.result=_mState;        
        //myInterActReg.TR.InCurrent=myTempVal.InCurrent;
        //myInterActReg.TR.Vin=myTempVal.VIn;
        //myInterActReg.TR.Vvcc=myTempVal.VCC;             
    }
    
    showLog("SAVE DATA...");
    //just Save EUI for ICA2407
    //SaveResult(true,myArg.StoreFolderPath+myArg.FileName_EUI,myArg.StoreFolderPath+myArg.FileName_Test);    
    SaveEUI(myArg.StoreFolderPath+myArg.FileName_EUI,true);
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);    
}








void USV_TEST_UTIL_V2::preLoopFunc(){
    myTempVal.VIn= myTestDevice.getDUT_VIN();
    myTempVal.InCurrent = myTestDevice.getDUT_VINAmp();
    if(myArg.LabDevice_PS) myTempVal.InCurrent= MyLabDevice.ReadPSCurrent();
    myTempVal.VOut= myTestDevice.getDUT_VOUT();
    myTempVal.LoadCurrent = myTestDevice.getDUT_VOUTAmp();
    if(myArg.LabDevice_Load){
        myTempVal.VOut = MyLabDevice.ReadLoadVoltage();
        myTempVal.LoadCurrent  = MyLabDevice.ReadLoadCurrent(); 
    }
    myBoard.GetBatBankTemp(&myTestResult.tempBatBank,false);
    myBoard.GetICTemp(&myTestResult.tempIC,false);

     
    
}
void USV_TEST_UTIL_V2::preLoopGetCaps(){
    float __tmpFloat=0;
    __tmpFloat = myBoard.GetVCap(0);
    if (__tmpFloat!=-1) myTempVal.VCap=__tmpFloat;
    myInterActReg.TR.Vcap1=myBoard.GetVCap(1);
    myInterActReg.TR.Vcap2=myBoard.GetVCap(2);
    myInterActReg.TR.Vcap3=myBoard.GetVCap(3);
    myInterActReg.TR.Vcap4=myBoard.GetVCap(4);
}

void USV_TEST_UTIL_V2::postLoopFunc(){
    myInterActReg.TR.InCurrent=myTempVal.InCurrent;
    myInterActReg.TR.LoadCurrent=myTempVal.LoadCurrent;
    myInterActReg.TR.VOut=myTempVal.VOut;
    myInterActReg.TR.Vin=myTempVal.VIn;
    myInterActReg.TR.VcapBank=myTempVal.VCap;
    myInterActReg.TR.Vvcc=myTempVal.VCC;
    myInterActReg.TR.CapChargeTime=myTempVal.chargeTime;
    myInterActReg.TR.CapDisChargeTime=myTempVal.DisChargeTime;
    myInterActReg.TR.OutSWOffTime=myTempVal.OutSWOffTime;  
    myInterActReg.TR.TempScapsBank=myTestResult.tempBatBank;
    myInterActReg.TR.TempIC=myTestResult.tempIC;
}
bool USV_TEST_UTIL_V2::CheckCapsVoltageDiff(void){
  uint16_t data;
  float Vcaps[5]={0,0,0,0,0};
  Vcaps[0] = myTempVal.VCap;//GetVCap(0);
  Vcaps[1] = myInterActReg.TR.Vcap1;//GetVCap(1);
  Vcaps[2] = myInterActReg.TR.Vcap2;//GetVCap(2);
  Vcaps[3] = myInterActReg.TR.Vcap3;//GetVCap(3);
  Vcaps[4] = myInterActReg.TR.Vcap4;//GetVCap(4);
  //printf("VCaps: %.1fv(%.1fv %.1fv %.1fv %.1fv )   \n",Vcaps[0],Vcaps[1],Vcaps[2],Vcaps[3],Vcaps[4]);
  //*VCap=Vcaps[0];
  Vcaps[0]=Vcaps[1]+Vcaps[2]+Vcaps[3]+Vcaps[4];
  Vcaps[0]/=4;
  float maxVal=Vcaps[0]*1.2,minVal=Vcaps[0]*.8;
  for(int i=1;i<5;i++)
      if(Vcaps[i]>maxVal || Vcaps[i]<minVal || Vcaps[i]>3.0)
      {
          printf("\n failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!! Vcaps are not Equal !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
          return false;
      }
          
  return true;
}
int USV_TEST_UTIL_V2::runSTM32ProgFunc(std::string _cmd, std::string &result){
    
    FILE* p = popen(_cmd.c_str(), "r");
    if (p) {
        char buf[1024];
        std::string out;
        while (fgets(buf, sizeof(buf), p)) out += buf;
        pclose(p);
        // use `out`
        if (out.length() > 1 ) {
            //showLog(out);
            result = out;
            return 0;    
        }else{
            std::cout << "!!! runSTM32ProgFunc OK !!! \n";
        }
    }
    std::cout<< "!!! runSTM32ProgFunc Error!!!\n";
    return 1;
}
bool USV_TEST_UTIL_V2::convertHexStrToByteArray(std::string hexStr, uint8_t* _byteArray,uint16_t maxSize){
    //byteArray.clear();
    size_t pos = hexStr.find('\n');
    if (pos != std::string::npos) hexStr = hexStr.substr(0, pos);
    hexStr.erase(std::remove_if(hexStr.begin(), hexStr.end(), ::isspace), hexStr.end());
    if (hexStr.length() % 2 != 0) {
        std::cerr << "Invalid hex string length." << std::endl;
        return false;
    }
    for (size_t i = 0; i < hexStr.length(); i += 2) {
        if (i / 2 >= maxSize) {
            std::cerr << "Hex string exceeds maximum buffer size." << std::endl;
            return false;
        }
        std::string byteString = hexStr.substr(i, 2);
        try {
            uint8_t byte = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
            _byteArray[i / 2] = byte;
            //std::cout<< "Byte " << (i / 2) << ": " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << std::dec << std::endl;

        } catch (const std::exception &e) {
            std::cerr << "Error converting hex string to byte array: " << e.what() << std::endl;
            return false;
        }
    }
    return true;
}
bool USV_TEST_UTIL_V2::convertByteArrayToHexStr(uint8_t* byteArray, size_t byteArraySize, std::string &hexStr){
    std::ostringstream oss;
    for (size_t i = 0; i < byteArraySize; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byteArray[i])<<" ";
    }
    hexStr = oss.str();
    return true;
}
bool USV_TEST_UTIL_V2::LabelPrint(){
    char tmp1[50];
    char tmp2[50];
    showLog("Label Print... ");
    myBoard.updateBoardNameStr();
    sprintf(tmp1,"S-Nr:%s",myBoard.myEEPROM.myData.getEUI5Byte_Str().c_str());
    sprintf(tmp2,"ICA%s",myBoard.boardName_str);
    qrcode_jpeg_output(myBoard.myEEPROM.myData.getEUI5Byte_Str().c_str(),myBoard.boardKind_str,tmp2,tmp1);                
    int __ret = QL700_Print();
    if(__ret==0){  
        //QL700_Print();
        showLog(" Ok.\n");
        return true;
    }
    else{        
        showLog(" Error.\n");
        return false;
    }
}

uint8_t USV_TEST_UTIL_V2::RSL_Init(__temp__register & _M2){    
    switch (_M2.m2State)
    {
    case 0://clear registers & reset Relays & Check for Lab-Device
    {
        showLog("\nDo RSL_Init... ");
        myInterActReg.TR.DataClear();
        removeJPG_PFiles_Jobs();
        myTempVal.clear();
        myBoard.myEEPROM.clear_EEPROM_Buffer();
        myBoard.myEEPROM.myData.clear();
        //showLog("ICA2315/ICA2405/ICA2506/ICA2510 Testing Routine...");
        showLog("Start: Reset Relays and Test start.");
        myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
        if(myArg.LabDevice_PS || myArg.LabDevice_Load){
            if(myArg.LabDevice_PS) 
                if (MyLabDevice.ReadPSCurrent()==-1){ _M2.mState=showError(ERROR::LabPSNoAnswer);break; }
            if(myArg.LabDevice_Load) 
                if(MyLabDevice.ReadLoadVoltage()==-1){ _M2.mState=showError(ERROR::LabLoadNoAnswer);break; }                    
            if(myArg.LabDevice_PS) myTestDevice.setRelay(USV_Test_Interface::Relays::LabPowerSel,true);
            MyLabDevice.SetPSEnable(true);
            MyLabDevice.SetPSVoltage(__const_PSVoltage);
            MyLabDevice.SetPSCurrent(__const_PSCurrent_NoCap_NoLoad);
            MyLabDevice.SetLoadCurrent(0);	
        }
        _M2.m2State++;        
        _M2.__isSupperCapsOnBoard=false;
    }
    break;
    case 1://clear test result and show log
    {                 
        myTestResult.clear(myBoard.boardName);
        showLog(" Ok.");
        _M2.m2State++;
    }
    break;
    case 2: return FuncStatus::success;
    default: return FuncStatus::failed;
    }
    return FuncStatus::running;
    
}
uint8_t USV_TEST_UTIL_V2::RSL_AR_Test(__temp__register & _M2){
   
    switch(_M2.m2State){
            case 0: //set Power On *********************************************
            {   
                myInterActReg.TR.currentTestNo=TestResult::T_AR_Off;             
                showLog("\nDo RSL_AR_OFF TEST:"+ std::to_string(myInterActReg.TR.currentTestNo));
                myTestDevice.setRelay(USV_Test_Interface::Relays::MPower,true);
                _M2.dcnt100ms=5;//500mSec              
                _M2.m2State++;
            }     
            break;      
            case 1: //TEST1: wait for a 500mSec (MPOWER:ON AR:OFF -> No Current) *********************************************
            {
                //showLog("Test1: No Current when Aufruesten is Off...");
                _M2.m2State++;            
            }
            break;
            case 2: //Wait for 500mSec and check current *********************************************
            {
                if(_M2.dcnt100ms>0){
                    if (myArg.LabDevice_PS) myTempVal.InCurrent=MyLabDevice.ReadPSCurrent();
                    if(myTempVal.InCurrent>constValue.InCurrent_NoAR_MaxLimit){
                        showLog((std::ostringstream{} << "Failed (Current > " << std::fixed << std::setprecision(2) << constValue.InCurrent_NoAR_MaxLimit << "A)!").str());
                        myInterActReg.TR.AR_Off=false;
                        _M2.m2State=showError(ERROR::Aufruesten);
                        return FuncStatus::failed;
                    }
                }
                else{
                    myInterActReg.TR.AR_Off=true;
                    showLog(" Ok.");
                    _M2.m2State++;
                }                                
            }
            break;
            case 3:// TEST2: Mpower:ON & AR:ON -> current:300mA *********************************************
            {
                if(myArg.LabDevice_PS) MyLabDevice.SetPSCurrent(__const_PSCurrent);		            
                myTestDevice.setRelay(USV_Test_Interface::Relays::AR,true);
                //myDurationTimer.testTimeStartSec();//ChargeTime Start
                myInterActReg.TR.currentTestNo=TestResult::T_AR_On;
                showLog("\nDo RSL_AR_ON TEST:"+ std::to_string(myInterActReg.TR.currentTestNo));
                //showLog("Test2: Turn On AufRuesten... ");                
                _M2.dcnt100ms=5;//500mSec
                _M2.m2State++;    
            }
            break;
            case 4://Wait for 500mSec and check current ********************************************* 
            {
                if(_M2.dcnt100ms>0){
                    if (myArg.LabDevice_PS) myTempVal.InCurrent=MyLabDevice.ReadPSCurrent();
                    if(myTempVal.InCurrent>constValue.InCurrent_AR_MinLimit){ 
                        myInterActReg.TR.AR_On=true;
                        showLog(" Ok.");
                        _M2.m2State++;
                    }
                }
                else{                
                    myInterActReg.TR.AR_On=false;
                    showLog((std::ostringstream{} << "Failed! (Current<" << std::fixed << std::setprecision(2) << constValue.InCurrent_AR_MinLimit << "})").str());
                    _M2.m2State=showError(ERROR::AufruestenOn); 
                    return FuncStatus::failed;
                } 
            }      
            break;              
            case 5: 
                myDurationTimer.testTimeStartSec();//ChargeTime Start
                return FuncStatus::success;
            default: return FuncStatus::failed;
        }
    return FuncStatus::running;      
}
uint8_t USV_TEST_UTIL_V2::RSL_VCC_Test(__temp__register & _M2){
    myInterActReg.TR.currentTestNo=TestResult::T_VCC_Test;
    switch(_M2.m2State){
            case 0: //set Power On *********************************************
            {
                
                showLog("\nDo RSL_VCC_TEST:"+ std::to_string(myInterActReg.TR.currentTestNo));
                myTestDevice.setRelay(USV_Test_Interface::Relays::MPower,true);
                myTestDevice.setRelay(USV_Test_Interface::Relays::AR,true);     
                myDurationTimer.testTimeStartSec();//ChargeTime Start           
                _M2.dcnt100ms=2;//500ms
                _M2.m2State++;
            }     
            break; 
            case 1: //TEST3 : VCC test  3.1 < VCC < 3.6 *********************************************
            {                
                myTempVal.VCC = myTestDevice.getDUT_VCC();
                myInterActReg.TR.Vvcc=myTempVal.VCC;
                if(myTempVal.VCC> 3.1 && myTempVal.VCC< 3.6){                    
                    showLog((std::ostringstream{} << "Test3: VCC test (3.1V < DUT[" << std::fixed << std::setprecision(2) << myTempVal.VCC << "V] < 3.6V)").str());
                    showLog(" Ok.");                        
                    _M2.m2State++;
                }else{
                    if(!(_M2.dcnt100ms==0)){
                        showLog((std::ostringstream{} << "Test3: VCC test (3.1V < DUT[" << std::fixed << std::setprecision(2) << myTempVal.VCC << "V] < 3.6V)").str());                    
                        showLog("Failed!");
                        if(myTempVal.VCC< 3.1)
                            _M2.mState=showError(ERROR::VCCIsLow);
                        else
                            _M2.mState=showError(ERROR::VCCIsOver);
                        return FuncStatus::failed;
                    }                    
                }                            
            }
            break;
            case 2: return FuncStatus::success;
            default: return FuncStatus::failed;
    }
    return FuncStatus::running;
}
uint8_t USV_TEST_UTIL_V2::RSL_uC_Program(__temp__register & _M2){
    myInterActReg.TR.currentTestNo=TestResult::T_uC_Program;
    std::string STM32Path="../TM_V2/Source/STM32ProgFunc";     
    std::string binFileName = myInterActReg.Dongle+"_STM32.bin";    
    std::string eepromFileName = myInterActReg.Dongle+"_EEPROM.txt";

    switch (_M2.m2State) //TEST4 : uC Program ********************************************* 
    {
    case 0:// Compare uC-Flash with current firmware
    {
        //showLog("\nDo RSL_uC_Program... ");
        showLog("\nDo RSL_uC_Program TEST:"+ std::to_string(myInterActReg.TR.currentTestNo));
        std::filesystem::path firmwarePath = STM32Path + "/Firmware_Folder/" + binFileName;
        if (!std::filesystem::exists(firmwarePath)) {
            showLog(std::string(" !!! Firmware file not found: !!!") + firmwarePath.string());
            _M2.m2State = showError(ERROR::uCProgramFailed);
            return FuncStatus::failed;
        }
        int __ret = std::system(std::string(STM32Path + "/STM32ProgFunc --cmp " + firmwarePath.string()).c_str());
        std::cout<< "Flash Compare Result: "<< __ret << std::endl;
        if(__ret==0){ showLog("Flash matches firmware."); _M2.m2State+=3; }
        if(__ret==0x200){ showLog("Flash is empty."); _M2.m2State++; }
        if(__ret==0x300){ showLog("Flash programmed with different firmware."); _M2.m2State++; }
        if (__ret!=0 && __ret!=0x200 && __ret!=0x300){ showLog("Flash compare failed"); _M2.m2State=showError(ERROR::uCProgramFailed); return FuncStatus::failed; }
    }
    break;
    case 1:// program uC with current firmware
    {
        showLog("Programming uC with firmware... ");
        std::string __cmd = std::string(STM32Path+ "/STM32ProgFunc "+STM32Path+"/Firmware_Folder/"+binFileName);
        std::cout << __cmd<< std::endl;
        int __ret = std::system(__cmd.c_str());
        if(__ret==0){ showLog("uC Program OK."); _M2.m2State++; }
        else{ showLog("uC Program Failed!"); _M2.m2State=showError(ERROR::uCProgramFailed); return FuncStatus::failed; }
    }    
    break;
    case 2:// Compare uC-Flash with current firmware
    {
        int __ret = std::system(std::string(STM32Path+ "/STM32ProgFunc --cmp "+STM32Path+"/Firmware_Folder/"+binFileName).c_str());
        if(__ret==0){ showLog("uC cmp Program OK."); _M2.m2State++; }
        else{ showLog("uC cmp Program Failed!"); _M2.m2State=showError(ERROR::uCProgramFailed); return FuncStatus::failed; }
    }
    break;
    case 3:// Reset uC
    {
        int __ret = std::system(std::string(STM32Path+ "/STM32ProgFunc --reset").c_str());
        if(__ret==0){ 
            showLog("uC reset OK."); 
            _M2.m2State++; }
        else{ showLog("uC reset Failed!"); _M2.m2State=showError(ERROR::uCProgramFailed); return FuncStatus::failed; }
    }
    break;
    case 4:
    case 5:
        _M2.m2State++;
    break;
    case 6://Get Firmware Version with UART command
    {
        std::string __result="";
        int __ret = runSTM32ProgFunc(std::string(STM32Path+ "/STM32ProgFunc --uart STR \"55 56 52 50\"").c_str(),__result);
        if(__ret==0){ 
            showLog("FirmWare:"); 
            {
                size_t pos = __result.find('\n');
                if (pos != std::string::npos && pos + 1 < __result.size())
                    showLog(__result.substr(pos + 1));
                else{
                    showLog("Not Find!!!");
                    _M2.m2State=showError(ERROR::uCProgramFailed); return FuncStatus::failed; 
                }
            }
            _M2.m2State++; }
        else{ showLog("uC reset Failed!"); _M2.m2State=showError(ERROR::uCProgramFailed); return FuncStatus::failed; }
    }
    break;
    case 7://Read EUI with UART command
    {
        myInterActReg.TR.currentTestNo=TestResult::T_Uart;
        //showLog("Read EUI OK.");
        if (myBoard.readEUI64(myBoard.myEEPROM.myData.EUI)){
            if(myBoard.myEEPROM.isKnownIC()){    
                //showLog(" Known IC.");
                myInterActReg.TR.EUI_str=myBoard.myEEPROM.myData.getEUI_Str();
                showLog(myInterActReg.TR.EUI_str);
                _M2.m2State++;	                
            }
            else {
                    showLog("unKnown IC.");
                    showLog("-- "+myInterActReg.TR.EUI_str);
                    _M2.m2State=showError(ERROR::ucUartFailed); 
                    return FuncStatus::failed;
            }   
        }
        else {
            showLog("Read EUI Failed!"); 
            _M2.m2State=showError(ERROR::ucUartFailed); 
            return FuncStatus::failed; 
        }
    }
    break;
    case 8://Read UID with UART command
    {
        std::string __resualt="";
        int __ret = runSTM32ProgFunc(std::string(STM32Path+ "/STM32ProgFunc --uart STR \"55 55 50\"").c_str(),__resualt);
        if(__ret==0){ 
            std::vector<uint8_t> byteArray;;
            if (convertHexStrToByteArray(__resualt,myBoard.myEEPROM.myData.UID,12)){
                showLog("UID: "+myBoard.myEEPROM.myData.getUID_Str());
                _M2.m2State++; 
            }
            else{
                showLog("Failed to convert UID hex string to byte array.");
                _M2.m2State=showError(ERROR::ucUartFailed); 
                return FuncStatus::failed;
            }
        }
        else{ 
            showLog("Read UID Failed!"); 
            _M2.m2State=showError(ERROR::ucUartFailed); 
            return FuncStatus::failed; 
        }
    }
    break;
    case 9:
    case 10:
        _M2.m2State++;
    break;
    case 11://Read EEPROM Value with current EEPROM-Ver (Just For Test)
    {
        std::string __resualt="";
        //int __ret = std::system(std::string(STM32Path+ "/STM32ProgFunc --uart STR \"55 43 52 50\"").c_str());
        int __ret = runSTM32ProgFunc(std::string(STM32Path+ "/STM32ProgFunc --uart STR \"55 43 52 50\"").c_str(),__resualt);
        if(__ret==0){ 
            showLog("Read current uC EEPROM value (UART) OK.");
            //showLog("EEPROM raw data: "+__resualt); 
            //std::cout<<"EEPROM:( "<< __resualt << " )"<<std::endl;
            //std::vector<uint8_t> byteArray;;
            if (convertHexStrToByteArray(__resualt,myBoard.myEEPROM.EEPROMDataBuffer,EEPROMDateBufferLen*8)){
                myBoard.myEEPROM.EEPROMDataBuffShow();
                if (myTestResult.EEPROM_Status=myBoard.myEEPROM.CheckDataVersionProcess(false)==EEPROMProcesSts::Ok){
                    memcpy(&myBoard.myEEPROM.myData.testTime, &myBoard.myEEPROM.EEPROMDataBuffer[12], 4);
                    std::tm *ttTime = std::localtime(&myBoard.myEEPROM.myData.testTime);
                    if(ttTime->tm_year==70){
                        showLog("EEPROM data has No time!!!.");
                        _M2.m2State+=2;
                    }
                    else{
                        showLog("EEPROM data version is OK.");
                        //myBoard.myEEPROM.EEPROMDataBuffShow();                           
                        //myBoard.myEEPROM.myData.show(); 
                        showLog(myBoard.myEEPROM.myData.getAll_Str());
                        _M2.m2State++; 
                    }                    
                }
                else{
                    showLog("EEPROM data version is Not OK!!!.");
                    _M2.m2State+=2;
                }
            }
            else
            {
                showLog("Failed to convert EEPROM hex string to byte array.");
                _M2.m2State=showError(ERROR::ucUartFailed); 
                return FuncStatus::failed;
            }
        }
        else{ 
            showLog("Read current uC EEPROM value (UART) Failed!"); 
            _M2.m2State=showError(ERROR::ucUartFailed); 
            return FuncStatus::failed; 
        }
    }
    break;   
    case 12://Ask user if EEPROM overwrite or not
    {
        if(myInterActReg.msgBox.waitForUser("EEPROM Over write?","Yes","NO",10)){
                showLog("Operator: YES");
                 _M2.m2State++;
            }
            else{
                showLog("Operator/timeOut: NO");
                myInterActReg.msgBox.clear();
                //showLog("!!!!!!!!!!!!!!!!!!  Ovrwrite Disable  !!!!!!!!!!!!!!!!!!");
                _M2.m2State+=3;
            }
    }
    break;
    case 13://Update EEPROM value with current Board Info and write to uC EEPROM with UART
    {   
        showLog("Update EEPROM value with current Board Info... ");
        myBoard.myEEPROM.updateBoardInfo(myBoard.boardName,myBoard.boardVer,myBoard.myBoardInfo,myTestResult);
        myBoard.myEEPROM.myData.VshutDown=4.2;//myBoard.myBoardInfo.Board_VShutdownVoltage;
        myBoard.myEEPROM.getCurrentTime();						        
        myBoard.myEEPROM.BuffUpdate_LVer();
        std::string __resStr="";
        
        convertByteArrayToHexStr(myBoard.myEEPROM.EEPROMDataBuffer,29, __resStr);
        std::string __str2= "55 43 57 "+__resStr+"50";
        showLog("NewData:"+__str2);
        int __ret = runSTM32ProgFunc(std::string(STM32Path+ "/STM32ProgFunc --uart STR \""+__str2+"\"").c_str(),__resStr);
        if(__ret==0){ 
            showLog("Write uC EEPROM value (UART) OK."); 
            //showLog("EEPROM:( "+__resStr+" )");
        }
        else{ 
            showLog("Write uC EEPROM value (UART) Failed!"); _M2.m2State=showError(ERROR::ucUartFailed); 
            return FuncStatus::failed;
        }
        _M2.m2State++;
    }
    break;
    case 14:// Reset uC
    {
        int __ret = std::system(std::string(STM32Path+ "/STM32ProgFunc --reset").c_str());
        if(__ret==0){ showLog("uC reset OK."); _M2.m2State++; }
        else{ showLog("uC reset Failed!"); _M2.m2State=showError(ERROR::uCProgramFailed); return FuncStatus::failed; }
    }
    break;
    case 15:
    case 16:
    case 17:
        _M2.m2State++;
    break;
    case 18: return FuncStatus::success;
    default:return FuncStatus::failed;
    }
    return FuncStatus::running;
}
uint8_t USV_TEST_UTIL_V2::RSL_UART_EEPROM(__temp__register & _M2){
    myInterActReg.TR.currentTestNo=TestResult::T_Uart;
    std::string STM32Path="../TM_V2/Source/STM32ProgFunc";     
    std::string eepromFileName = myInterActReg.Dongle+"_EEPROM.txt";
    switch (_M2.m2State)
    {
    case 0:
    {
        showLog("\nDo RSL_uart TEST:"+ std::to_string(myInterActReg.TR.currentTestNo));
        //showLog("Test5: Board Connection... ");
        _M2.dcnt100ms=250;//25Sec
        _M2.m2State++;  
    }
    break;
    case 1://check if UART Worked
    {
        if(_M2.dcnt100ms>0){
            if(!myBoard.init(myArg.ttyName)){
                showLog(" Failed to Serial Port Connection (INI)! \n");                         
                myInterActReg.TR.UART_Con=false;
            }else{
                myBoard.GPIOResetAll();
                if(myBoard.GPIO_read()!=false){
                    showLog((std::ostringstream{} << "UART connection Ok (after " << std::fixed << std::setprecision(2) << myDurationTimer.TestTimeSec()  << "sec)").str());                        
                    myInterActReg.TR.UART_Con=true;
                    _M2.m2State++;
                } 				
            }         
        }
        else{
            showLog(" Failed to Serial Port Connection (Time Out)! \n");                         
            myInterActReg.TR.UART_Con=false;
            _M2.m2State=showError(ERROR::BoardConnection1);
            return FuncStatus::failed;
        }
    }
    break;
    case 2: 
    case 3:
    case 4:
    case 5:
        _M2.m2State++;
    break;
    case 6:
    {
        if (myBoard.readEUI64(myBoard.myEEPROM.myData.EUI)){
            if(myBoard.myEEPROM.isKnownIC()){    
                showLog(" Known IC.");
                myInterActReg.TR.EUI_str=myBoard.myEEPROM.myData.getEUI_Str();
                showLog(myInterActReg.TR.EUI_str);
                if(myBoard.readDataBuff(myBoard.myEEPROM.EEPROMDataBuffer))
                    myTestResult.EEPROM_Status=myBoard.myEEPROM.CheckDataVersionProcess(false);  
                
                myInterActReg.TR.EEPROMBuff_str=myBoard.myEEPROM.myData.getAll_Str(); 
                myBoard.myEEPROM.EEPROMDataBuffShow();                           
                myBoard.myEEPROM.myData.show(); 
                _M2.m2State++;	
                _M2.dcnt100ms=5;				
            }	
            else {
                    showLog("unKnown ");
                    showLog("-- "+myInterActReg.TR.EUI_str+"");
                    _M2.m2State=showError(ERROR::BoardConnection2);
                    return FuncStatus::failed;
            }   
        }
        else {
            _M2.mState=showError(ERROR::BoardConnection3); 
            return FuncStatus::failed;
        }
    }
    break;
    case 7: return FuncStatus::success;    
    default: return FuncStatus::failed;
    }
    return FuncStatus::running;
}
uint8_t USV_TEST_UTIL_V2::RSL_UART_EEPROM_RTC_I2C(__temp__register & _M2){
    static ICA_justEUI myICA2308;
    myInterActReg.TR.currentTestNo=TestResult::T_EEPROM_I2C;
    switch (_M2.m2State)
    {
        case 0:
        {
            showLog("\nDo RSL_uart TEST:"+ std::to_string(myInterActReg.TR.currentTestNo));
            _M2.dcnt100ms=5;
            _M2.m2State++;  
            myICA2308.clearEUIBuffer();
        }
        break;
        case 1:
            if(_M2.dcnt100ms-->0){
                if(myICA2308.ReadEUI()){
                    //printf("%s",myICA2308.showEUI().c_str());
                    _M2.m2State++;
                }
                else{
                    //printf("ReadEUI Failed!!!!!!!!!\n");
                    showLog("ReadEUI Failed!!!!!!!!!");
                    _M2.m2State=showError(ERROR::I2CFailed);
                     return FuncStatus::failed;
                }
            }
            else
            {
                    showLog(" Failed to I2C Connection!"); 
                    _M2.m2State=showError(ERROR::I2CFailed);
                     return FuncStatus::failed;
            }
        break;
        case 2:
        {
            for(int i=0;i<8;i++)
                myBoard.myEEPROM.myData.EUI[i]=myICA2308.EUI_Buffer[i];
            myBoard.myEEPROM.ShowEUI(); //just for test  
            myInterActReg.TR.EUI_str=myBoard.myEEPROM.myData.getEUI_Str();
            showLog(myInterActReg.TR.EUI_str+"\n");
            _M2.m2State++;  
        }
        case 3: return FuncStatus::success;    
        default: return FuncStatus::failed;
    }
    return FuncStatus::running;
}
uint8_t USV_TEST_UTIL_V2::RSL_ChargeTest(__temp__register & _M2){
    // It Should first run RSL_AR_Test to active chargeing-timer
    myInterActReg.TR.currentTestNo=TestResult::T_ChargeTest;
    switch (_M2.m2State)
    {
    case 0:{//Start Charge Test
        showLog("\nDo RSL_ChargeTest TEST:"+ std::to_string(myInterActReg.TR.currentTestNo));
        _M2.__isSupperCapsOnBoard=true;
        _M2.m2State++;
    }
    break;
    case 1:{//Check Charge Time and Current
        //preLoopFunc if(myArg.LabDevice_PS) myTempVal.InCurrent= MyLabDevice.ReadPSCurrent();
            myTempVal.chargeTime=myDurationTimer.TestTimeSec();                
            if(myTempVal.chargeTime!=myTempVal.ltime_mess){
                    //myTempVal.VCap=myBoard.GetVCap(0);                
                    _M2.file << myTempVal.chargeTime<<","<< std::fixed
						<<std::setprecision(1)<<myTempVal.VCap <<","
						<<std::setprecision(3)<< myTempVal.InCurrent<< std::endl;
                    myTempVal.ltime_mess=myTempVal.chargeTime;
                }
                if(myTempVal.InCurrent!=-1){    
                    myTestResult.Vcap_Max=myTempVal.VCap;
                    //PreLoopFunc myBoard.CheckCapsVoltage(&myTestResult.Vcap_Max);
                    _M2.__error_cnt=0;
                }
                else{
                    printf("\n failed !!!Current read Error!!! (Value:%.2f)\n",myTempVal.InCurrent);
                    showLog((std::ostringstream{} <<" failed !!!Current read Error!!! (Value:"<< std::fixed << std::setprecision(2)<< myTempVal.InCurrent<<")").str());
                    if (_M2.__error_cnt++>3) _M2.m2State=showError(ERROR::ChargeDuration);
                }                
                if ((myTempVal.chargeTime > myTestResult.Limit_MAX_Charge_time) & (myTempVal.InCurrent> myTestResult.Limit_MIN_ChargeCurrent)) {
                    showLog((std::ostringstream{} << "\nTEST5.Error!!!  Time (" 
                        << myTempVal.chargeTime << " > " << myTestResult.Limit_MAX_Charge_time 
                        << ") Current (" 
                        << myTempVal.InCurrent << " > " << myTestResult.Limit_MIN_ChargeCurrent 
                        << ")\n").str());
                    _M2.m2State=showError(ERROR::ChargeDuration);
                }
                if (myTempVal.chargeTime > myTestResult.Limit_MAX_Charge_time){//+__Limit_MAX_ExtendChargeTime){                    
                    showLog((std::ostringstream{}<< "\nTEST5.Error!!!  Time ("
                        << myTempVal.chargeTime << " > "
                        << (myTestResult.Limit_MAX_Charge_time)// + __Limit_MAX_ExtendChargeTime)
                        << ")\n" ).str());
                    _M2.m2State=showError(ERROR::ChargeDuration);
                }
                if(myTempVal.InCurrent < myTestResult.Limit_MIN_FullChargeCurrent && myTempVal.InCurrent > 0){
                    myTestResult.time_charge=myTempVal.chargeTime;
                    _M2.m2State++;
                }	
    }
    break;    
    case 2: return FuncStatus::success;    
    default: return FuncStatus::failed;
    }
    return FuncStatus::running;
    
}
uint8_t USV_TEST_UTIL_V2::RSL_FlyBackTest(__temp__register & _M2){
    myInterActReg.TR.currentTestNo=TestResult::T_FlyBackTest;
    switch (_M2.m2State){    
    case 0:
    {        
        showLog("\nDo RSL_FlyBackTest TEST:"+ std::to_string(myInterActReg.TR.currentTestNo));
        myBoard.GPIOResetAll();
        _M2.m2State++;
    }
    break;
    case 1:
    {
        myTestDevice.setRelay(USV_Test_Interface::Relays::LabPowerSel,false);
        myTestDevice.setRelay(USV_Test_Interface::Relays::VCCLoad,false);
        myTestDevice.setRelay(USV_Test_Interface::Relays::MPower,true);
        myTestDevice.setRelay(USV_Test_Interface::Relays::AR,true);
        myTestDevice.setRelay(USV_Test_Interface::Relays::Load,true);
        if(myArg.LabDevice_Load) MyLabDevice.SetLoadCurrent(myTestResult.Load_Current);
        if((myTestDevice.readRelay()&0x15)!=0x15)
            std::cout << "start... " << myTestDevice.readRelay() << std::endl;                
        else{
            _M2.m2State++;            
            _M2.dcnt100ms=5; 
        }            
    }
    break;
    case 2: //wait to Power Current More then 500mA
        printf("S2 INAmp:%.3fA\n",myTempVal.InCurrent); 
        myTestResult.VOut1=myTempVal.VOut;
        if(myTempVal.InCurrent> myTestResult.Load_Current ) _M2.m2State++;
        break;
    case 3: //Set FlayBack Off.
        myInterActReg.TR.currentTestNo=6;
        showLog("Test6: FlyBack-Dis... ");
        _M2.dcnt100ms=20;
        _M2.m2State++;
        break;
    case 4:  // check FlyBack Dis...
        //sprintf(_str,"S6  INAmp:%.3fA ",myTempVal.InCurrent);
        printf("\rTest6: FlyBack-Dis... %.3fA   ",myTempVal.InCurrent);
        myTestResult.VOut2=myTempVal.VOut;
        myBoard.FlyBack_Off();
        if(myTempVal.InCurrent!=-1 && myTempVal.InCurrent< myTestResult.Load_Current ) _M2.m2State++;
    break;
    case 5: //FlyBack diss is Ok        
        //sprintf(_str,"T6 Ok.");
        showLog("Ok.\n");
        myInterActReg.TR.FlyBackDis=true;
        _M2.dcnt100ms=20;_M2.m2State++;
        break;        
    case 6:  // check FlyBack reset
        //sprintf(_str,"S2 INAmp:%.3fA",myTempVal.InCurrent);
        if(myTempVal.InCurrent> myTestResult.Load_Current) _M2.m2State++;
        myBoard.GPIOResetAll();
        break;
    case 7:  //trun off Aufruesten
        _M2.dcnt100ms=20; 
        _M2.m2State++;
        break;
    case 8: //wait to Power Current less then 500mA
        //sprintf(_str,"S8 INAmp:%.3fA\n",myTempVal.InCurrent);myTestDevice.showOnLCD(2,_str);
        //printf("\r",_str);
        myTestDevice.setRelay(USV_Test_Interface::Relays::AR,false);            
        if(myTempVal.InCurrent!=-1 && myTempVal.InCurrent< myTestResult.Load_Current ) _M2.m2State++;        
        break;
    case 9: //Set FlyBack Enable
        myInterActReg.TR.currentTestNo=7;
        //sprintf(_str,"T7 FlyBack-En...");
        showLog("Test7: FlyBack-En... ");
        _M2.dcnt100ms=20; _M2.m2State++;
        break; 
    case 10: //wait to Power Current More then 500mA
        //sprintf(_str,"S10 INAmp:%.3fA\n",myTempVal.InCurrent);myTestDevice.showOnLCD(2,_str);
        //printf(_str);
        myBoard.FlyBackEn();
        if(myTempVal.InCurrent> myTestResult.Load_Current )  _M2.m2State++;
        break;
    case 11: 
        //sprintf(_str,"T7 OK");
        showLog("Ok.\n");
        myInterActReg.TR.FlayBackEn=true;            
        myBoard.GPIOResetAll();
        _M2.dcnt100ms=20; 
        _M2.m2State++;
        break;
    break;
    case 12: return FuncStatus::success;    
    default: return FuncStatus::failed;
    }
    return FuncStatus::running;
}
uint8_t USV_TEST_UTIL_V2::RSL_WaitToOutSWOffTest(__temp__register & _M2){
    myInterActReg.TR.currentTestNo=TestResult::T_WaitToOutSWOffTest;
    switch (_M2.m2State){        
    case 0:
        showLog("\nDo RSL_WaitToOutSWOffTest:"+ std::to_string(myInterActReg.TR.currentTestNo));
        myBoard.GPIOResetAll();
        _M2.m2State++;
    break;
    case 1:
        myTestDevice.setRelay(USV_Test_Interface::Relays::LabPowerSel,false);
        myTestDevice.setRelay(USV_Test_Interface::Relays::VCCLoad,false);
        myTestDevice.setRelay(USV_Test_Interface::Relays::MPower,true);
        myTestDevice.setRelay(USV_Test_Interface::Relays::AR,true);
        myTestDevice.setRelay(USV_Test_Interface::Relays::Load,true);
        if(myArg.LabDevice_Load) MyLabDevice.SetLoadCurrent(myTestResult.Load_Current);
        if((myTestDevice.readRelay()&0x15)!=0x15)
            std::cout << "start... " << myTestDevice.readRelay() << std::endl;                
        else{
            _M2.m2State++;            
        }
    break;
    case 2: 
        if( myTempVal.LoadCurrent> myTestResult.Load_Current*.70 ) _M2.m2State++;
    break;
    case 3: 
        showLog("Output-SW ... ");
        myBoard.OutPUT_Off();
        _M2.dcnt100ms=150;
        myDurationTimer.testTimeStartSec();            
        _M2.m2State++;
        break;
    case 4: 
        myTempVal.WaitToOutSWOffTime=myDurationTimer.TestTimeSec();                        
        //printf("\r   wait to Output off time : %d sec... ",myTempVal.WaitToOutSWOffTime);            
        //myBoard.OutPUT_Off();
        if(myTempVal.LoadCurrent!=-1 && myTempVal.LoadCurrent< myTestResult.Load_Current*.70 ){
            //showLog(_str);
            if(myTempVal.WaitToOutSWOffTime>=myTestResult.Limit_MIN_WaitToOutSwOff && myTempVal.WaitToOutSWOffTime<=myTestResult.Limit_MAX_WaitToOutSwOff){
                myDurationTimer.testTimeStartSec();
                _M2.m2State++; 
            }
            else{
                std::cout<< "Time:" << myTempVal.WaitToOutSWOffTime << "(" << myTestResult.Limit_MIN_WaitToOutSwOff << "-" << myTestResult.Limit_MAX_WaitToOutSwOff << ") \n";
                showLog("   wait to Output off time is not in range ");
                myTestResult.ErrorNo=ERROR::waitToOutSwOff;
                //sprintf(_str,"Err.%d",myTestResult.ErrorNo);myTestDevice.showOnLCD(2,_str);
                printf("\nGPIO Test Error (%d) on step %d\n",myTestResult.ErrorNo,_M2.m2State);
                showError(myTestResult.ErrorNo);
                return myTestResult.ErrorNo;                    
            }            
        } 
        break;
    case 5: 
        //sprintf(_str,"T8 Ok");
        showLog("Ok.\n"); 
        myInterActReg.TR.OutSwOff=true;            
        //myBoard.GPIOResetAll();  
        _M2.dcnt100ms=150; 
        _M2.m2State++;
    break;
    case 6:
        myTempVal.OutSWOffTime=myDurationTimer.TestTimeSec();
        //sprintf(_str,"T8 %d sec (%.1f)",myTempVal.OutSWOffTime,myTempVal.InCurrent);
        //printf("\r   Output Off time : %d sec... ",myTempVal.OutSWOffTime);
        //myBoard.GPIOResetAll();
        if(myTempVal.LoadCurrent> myTestResult.Load_Current*.70 ) {
            //showLog(_str);
            if(myTempVal.OutSWOffTime>myTestResult.Limit_MIN_OutSwOff && myTempVal.OutSWOffTime<myTestResult.Limit_MAX_OutSwOff){
                _M2.m2State++; 
            }else{
                showLog("   Output off time is not in range \n");
                myTestResult.ErrorNo=ERROR::OutSwOff;
                //sprintf(_str,"Err.%d",myTestResult.ErrorNo);myTestDevice.showOnLCD(2,_str);
                printf("\nGPIO Test Error (%d) on step %d\n",myTestResult.ErrorNo,_M2.m2State);
                showError(myTestResult.ErrorNo);
                return myTestResult.ErrorNo;                    
            }                            
        } 
        break;
    case 7:
        //sprintf(_str,"S17");
        myBoard.GPIOResetAll();
        myTestResult.time_WaitToOutSwOff=myTempVal.WaitToOutSWOffTime;
        myTestResult.time_OutSwOff=myTempVal.OutSWOffTime;
        //showLog("\n");
        _M2.m2State++;
        break;
    case 8: return FuncStatus::success;    
    default: return FuncStatus::failed;
    }
    /*if(_M2.dcnt100ms<=0)
        { 
            myTestResult.ErrorNo=100+_M2.m2State;// ERROR::GPIO;
            if(_M2.m2State==2) myTestResult.ErrorNo=ERROR::FlyBackdis;
            if(_M2.m2State==8) myTestResult.ErrorNo=ERROR::FlyBackEn;
            if(_M2.m2State==12) myTestResult.ErrorNo=ERROR::OutSwOff;
            //sprintf(_str,"Err.%d",myTestResult.ErrorNo);myTestDevice.showOnLCD(2,_str);
            //printf("\nGPIO Test Error (%d) on step %d\n",myTestResult.ErrorNo,_mState);
            showError(myTestResult.ErrorNo);
            return myTestResult.ErrorNo;
        }
    */
    return FuncStatus::running;
}
uint8_t USV_TEST_UTIL_V2::RSL_DisChargeTest(__temp__register & _M2){
    //static float __tempICtempVal=0.0;
    static uint8_t __tempIC__error__cnt=0;
    static uint8_t __diffVcap__error__cnt=0;
    static uint8_t __tempBatBack__error__cnt=0;
     myInterActReg.TR.currentTestNo=TestResult::T_DisChargeTest;
    switch (_M2.m2State){        
    case 0:
        showLog("\nDo RSL_DisChargeTest:"+ std::to_string(myInterActReg.TR.currentTestNo));
        //__tempICtempVal=0.0;
        __tempIC__error__cnt=0;
        __diffVcap__error__cnt=0;
        __tempBatBack__error__cnt=0;
        _M2.m2State++;
    break;
    case 1:
    {
        myTestDevice.setRelay(USV_Test_Interface::Relays::Load,false);
        if(myArg.LabDevice_PS) MyLabDevice.SetPSCurrent(__const_PSCurrent);
        if(myArg.LabDevice_Load) MyLabDevice.SetLoadCurrent(0);    
        _M2.m2State++;            
    }       
    break;
    case 2:
    {
        //preLoopFunc if(myArg.LabDevice_PS) myTempVal.InCurrent = MyLabDevice.ReadPSCurrent();					
        if (myTempVal.InCurrent!=0 && myTempVal.InCurrent<.2) _M2.m2State++;
    }
    break;
    case 3:// TEST9 : Discharge *********************************************
    {
        showLog("Test9: wait to disCharge SCap");
        myInterActReg.TR.currentTestNo=9;
        myTestDevice.setRelay(USV_Test_Interface::Relays::Load,true);
        if(myArg.LabDevice_PS) MyLabDevice.SetPSCurrent(__const_PSCurrent);
        if(myArg.LabDevice_Load) MyLabDevice.SetLoadCurrent(myTestResult.Load_Current);                                
        myDurationTimer.testTimeStartSec();
        myTestDevice.setRelay(USV_Test_Interface::Relays::AR,false);				            
        if (_M2.file.is_open()) { _M2.file.close(); }
        _M2.file.open("./tmp/disChargeCurve.csv", std::ios::out);
        _M2.file << "time,voltage"<< std::endl;
        _M2.m2State++;	
    }
    break;
    case 4:
    {            
        //preLoopFunc myTempVal.LoadCurrent = myTestDevice.getDUT_VOUTAmp();
        //preLoopFunc myTempVal.VOut= myTestDevice.getDUT_VOUT();
        //Pre LoopFunc if(myArg.LabDevice_Load){
        //    myTempVal.VOut = MyLabDevice.ReadLoadVoltage();
        //    myTempVal.LoadCurrent  = MyLabDevice.ReadLoadCurrent(); 
        //}
        
        //PostLoopFunc myTempVal.VCap=myBoard.GetVCap(0);
        
        myTempVal.DisChargeTime=myDurationTimer.TestTimeSec();
        //if(!myBoard.CheckCapsVoltage(&myTempVal.VCap)){
        if(!CheckCapsVoltageDiff()){
            myTempVal.VCap=0;
            if(__diffVcap__error__cnt++>5)
                _M2.m2State=showError(ERROR::VCapsNotSame);//testr.ErrorNo=ERROR::VCapsNotSame;
        }else {_M2.__diffVcap__error__cnt=0;}

        if(myTestResult.tempIC>__Limit_MAX_IC_Temp || myTestResult.tempIC==-273) {
            if(__tempIC__error__cnt++>5)
                _M2.m2State=showError(ERROR::TempSensor_IC);//testr.ErrorNo=ERROR::TempSensor;
        }else {
            //myTestResult.tempIC=__tempICtempVal;
            __tempIC__error__cnt=0;}

        //if(!myBoard.GetBatBankTemp(&myTestResult.tempBatBank,false) || (myTestResult.tempBatBank>__Limit_MAX_BatBank_Temp)) 
        if(myTestResult.tempBatBank>__Limit_MAX_BatBank_Temp) 
        {
            if(__tempBatBack__error__cnt++>5) _M2.m2State=showError(ERROR::TempSensor_CapBank);//testr.ErrorNo=ERROR::TempSensor;                
        }
                _M2.file << myTempVal.DisChargeTime<<","<< std::fixed
            <<std::setprecision(1)<<myTempVal.VCap << std::endl;
        //myTestResult.ErrorNo=0;
        usleep(100000);
        if(myTempVal.VCap>0) myTestResult.VCap_SWOff=myTempVal.VCap;

        if ( myTempVal.LoadCurrent>-1 && myTempVal.LoadCurrent<0.05 ) {
            showLog("\n");
            myTestResult.time_DisCharge=myTempVal.DisChargeTime;
            if(myTestResult.VCap_SWOff< myTestResult.Limit_MIN_VCap_ShutdownVoltage || myTestResult.VCap_SWOff> myTestResult.Limit_MAX_VCap_ShutdownVoltage){
                showLog((std::ostringstream{} << "Vcap on Shut Down is:" << std::fixed << std::setprecision(2) << myTestResult.VCap_SWOff << "V").str());                            
                _M2.m2State=showError(ERROR::VCapShutDownOutOfRange);//testr.ErrorNo=ERROR::VCapOutOfRange;
            }
            else{
            _M2.m2State++;
            _M2.dcnt100ms=50;
            }
            
        }	
    }                              
    break;
    case 5: return FuncStatus::success;    
    default: return FuncStatus::failed;
    }

    return FuncStatus::running;
}

void USV_TEST_UTIL_V2::run_Test_Func(){
    uint8_t _key=0;
    __temp__register __tr;
    std::string STM32Path="../TM_V2/Source/STM32ProgFunc";     
    std::string binFileName = myInterActReg.Dongle+"_STM32.bin";
    std::string uartFileName = myInterActReg.Dongle+"_EEPROM.txt";
    showLog("******************************");
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);            
    DongleCheck();  
    ShowMyDongle();
    uint8_t __funcResualt=0;
    __tr.RSL_Cnt=0;
    __tr.RSL_state=toDoList[__tr.RSL_Cnt];
    RSL_struct x1;
    uint8_t __cnt1=0;
    uint8_t lCurrentTestNo=0;
    uint8_t lState=0;
    while((__tr.RSL_state!=RSL_struct::RSL::Stop) && (xrunning==true) ){
        myInterActReg.TR.currentTestNoStr = x1.getRSLStr(__tr.RSL_state);
        if(myInterActReg.TR.currentTestNo!=lCurrentTestNo || __tr.m2State!=lState){
            std::cout << "StateMachin:  RSL(" << std::to_string(myInterActReg.TR.currentTestNo) <<")"
                        << myInterActReg.TR.currentTestNoStr
                        << " M2State: " << std::to_string(__tr.m2State) << std::endl;
            lCurrentTestNo=myInterActReg.TR.currentTestNo;
            lState=__tr.m2State;
        }   
        usleep(100000);
        
        

        if (__tr.dcnt100ms>0) __tr.dcnt100ms--;        
        preLoopFunc();
        preLoopGetCaps();
        __funcResualt=-1;
        switch(__tr.RSL_state){
            case RSL_struct::RSL::Init: __funcResualt= RSL_Init(__tr); break;
            case RSL_struct::RSL::AR_Test: __funcResualt = RSL_AR_Test(__tr); break;
            case RSL_struct::RSL::VCC_Test: __funcResualt = RSL_VCC_Test(__tr); break;
            case RSL_struct::RSL::uC_Program: __funcResualt = RSL_uC_Program(__tr); break;
            case RSL_struct::RSL::Uart_EEPROM: __funcResualt = RSL_UART_EEPROM(__tr); break;            
            case RSL_struct::RSL::uart_EEPROM_RTC_I2C: __funcResualt = RSL_UART_EEPROM_RTC_I2C(__tr); break;
            case RSL_struct::RSL::ChargeTest: __funcResualt = RSL_ChargeTest(__tr); break;
            case RSL_struct::RSL::FlyBackTest: __funcResualt = RSL_FlyBackTest(__tr); break;
            case RSL_struct::RSL::WaitToOutSWOffTest: __funcResualt = RSL_WaitToOutSWOffTest(__tr) ; break;
            case RSL_struct::RSL::DisChargeTest: __funcResualt = RSL_DisChargeTest(__tr); break;
            case RSL_struct::RSL::EndSuccess: //Label Print if Test Success
            {   
                LabelPrint();
                __tr.RSL_state=RSL_struct::RSL::Stop; 
            }
            break;
            case RSL_struct::RSL::EndFailed: __tr.RSL_state=RSL_struct::RSL::Stop; break;
            default:
                showLog("\nEND Wait to press Key.\n");        
                __tr.RSL_state=RSL_struct::RSL::Stop;
            break;
        }
        if(__tr.RSL_state!=RSL_struct::RSL::Stop){
            switch(__funcResualt){
                case 0: break;
                case 1: 
                    //showLog("Succes!"); 
                    if(++__tr.RSL_Cnt<toDoList.size())
                        __tr.RSL_state=toDoList[__tr.RSL_Cnt];
                    else
                        __tr.RSL_state=RSL_struct::RSL::EndSuccess;
                    __tr.m2State=0;//reset m2State for next function
                break;
                case 2: showLog("failed!\n"); __tr.RSL_state=RSL_struct::RSL::EndFailed; break;
                default: showLog("Unknown Error in RSL State Machine\n"); __tr.RSL_state=RSL_struct::RSL::Stop; break;
            }
        }
        myTempVal.result=__tr.RSL_state; 
        _key=myInterActReg.getGuiCMD();
        if(_key==USV_Test_Interface::_KEY::K3) __tr.RSL_state=RSL_struct::RSL::Stop;
        //if(__tr.RSL_state==RSL_struct::RSL::ChargeTest || __tr.RSL_state==RSL_struct::RSL::DisChargeTest) postLoopGetCaps();
        

        postLoopFunc();        
    }

    showLog("SAVE DATA...");
    SaveEUI(myArg.StoreFolderPath+myArg.FileName_EUI,(myTestResult.ErrorNo == 0) ? true : false);
    SaveResult(myArg.StoreFolderPath+myArg.FileName_Test);
    
    myTestDevice.setRelay(USV_Test_Interface::Relays::AR,false);
    myTestDevice.setRelay(USV_Test_Interface::Relays::MPower,false);
    myTestDevice.setRelay(USV_Test_Interface::Relays::Load,true);
    if(__tr.__isSupperCapsOnBoard) myInterActReg.msgBox.waitForUser("Wait 120 to Discharge","Ok","",120);    
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
}

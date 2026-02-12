#include "usv_test_util.h"
#include "./TestFunction/ICA_justEUI.h"
extern LabDevice MyLabDevice;
extern durationTimerClass myDurationTimer;
extern testResult testr;
extern _interact_registers myInterActReg;
ConsoleKeyClass myCKey;
void USV_TEST_UTIL_V2::showLog(std::string _str){
            printf("%s\n",_str.c_str());
            myInterActReg.csLogWrite(_str);
        }
void testrReset(){
	testr.ErrorNo=0;
    testr.Logstr='0';
    testr.Load_Current=__const_LoadCurrent;
    testr.VOut1=0;
    testr.VOut2=0;
    testr.time_charge=0;
    testr.time_DisCharge=0;
    testr.time_OutSwOff=0;
    
    testr.repaired_Cap=0;
    testr.repaired_NoCap=0;
    testr.EEPROM_Status=0;
    testr.Limit_MAX_Charge_time=__Limit_MAX_Charge_time_2405;
    testr.Limit_MIN_ChargeCurrent=__Limit_MIN_ChargeCurrent_2405;
    testr.Limit_MIN_FullChargeCurrent=__Limit_MIN_FullChargeCurrent_2405;
    testr.Limit_MAX_WaitToOutSwOff=__Limit_MAX_WaitToOutSwOff_2405;
    testr.Limit_MIN_WaitToOutSwOff=__Limit_MIN_WaitToOutSwOff_2405;
    testr.Limit_MIN_OutSwOff=__Limit_MIN_OutSwOff_2405;
    testr.Limit_MAX_OutSwOff=__Limit_MAX_OutSwOff_2405;
    testr.Limit_MIN_VCap_ShutdownVoltage=__Limit_MIN_VCap_ShutdownVoltage_2405;
    testr.Limit_MAX_VCap_ShutdownVoltage=__Limit_MAX_VCap_ShutdownVoltage_2405;
    testr.tempIC=0;
    testr.Vcap_Max=0;
    if(myBoard.boardName==2315){
        testr.Limit_MAX_Charge_time=__Limit_MAX_Charge_time_2315;
        testr.Limit_MIN_ChargeCurrent=__Limit_MIN_ChargeCurrent_2315;
        testr.Limit_MIN_FullChargeCurrent=__Limit_MIN_FullChargeCurrent_2315;
        testr.Limit_MAX_WaitToOutSwOff=__Limit_MAX_WaitToOutSwOff_2315;
        testr.Limit_MIN_WaitToOutSwOff=__Limit_MIN_WaitToOutSwOff_2315;
        testr.Limit_MIN_OutSwOff=__Limit_MIN_OutSwOff_2315;
        testr.Limit_MAX_OutSwOff=__Limit_MAX_OutSwOff_2315;
        testr.Limit_MIN_VCap_ShutdownVoltage=__Limit_MIN_VCap_ShutdownVoltage_2315;
        testr.Limit_MAX_VCap_ShutdownVoltage=__Limit_MAX_VCap_ShutdownVoltage_2315;
    }
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
        myBoard.boardName=2405;
        sprintf(myBoard.boardKind_str,"NT-CLX USV");
        myBoard.boardType=ICA_NT_USV_2405;
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
        myBoard.boardVer=(static_cast<int>(_version) << 4) | static_cast<int>((_version - static_cast<int>(_version)) * 10 + 0.5);
    break;
    case 4: //ICA2407
        myBoard.boardName=2407;
        sprintf(myBoard.boardKind_str,"IBIS Slave");
        sprintf(myBoard.boardName_str,"2407");        
        myBoard.boardType=ICA_2407;
        myBoard.boardVer=0;        
    break;
    case 5: //ICA2506
        myBoard.boardName=2506;
        sprintf(myBoard.boardKind_str,"NT-CLX USV Pro");
        sprintf(myBoard.boardName_str,"2506");        
        myBoard.boardType=ICA_2506;//ICA_NT_USV_2405
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
        switch ((uint16_t)_version*100)
        {
            case 100:
            myBoard.boardVer=0x10;
            myBoard.myBoardInfo.Board_SupperCapType=ICA_CapType_2405_1;
            myBoard.myBoardInfo.Board_MaxTemp85V=26;    
            break;
        }
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

    testr.ErrorNo=_errorNo;
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
    case ERROR::TempSensor_IC:      _strError2=("*TempSensor "+std::to_string((int16_t)testr.tempIC)+"°c");_strDesc="High IC-Temperature";break;
    case ERROR::TempSensor_CapBank: _strError2=("*TempSensor "+std::to_string((int16_t)testr.tempBatBank)+"°c");_strDesc="High CapBank-Temperature";break;
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
void USV_TEST_UTIL_V2::showOnRun(uint8_t _StepNo){
    char _str[32];
    switch (_StepNo)
    {
    
    case runStepList::testP2_InputCurrent:
    break;    
    case runStepList::testP3_InputCurrent:
        sprintf(_str,"M-Power + AR On");myTestDevice.showOnLCD(1,_str);
        sprintf(_str,"In-Amp: %.3f A",myTempVal.InCurrent);myTestDevice.showOnLCD(2,_str);
    break;
    case runStepList::testP4_EUI:
        sprintf(_str,"EUI: %02X-%02X-%02X",myBoard.myEEPROM.EEPROMDataBuffer[0]
            ,myBoard.myEEPROM.EEPROMDataBuffer[1],myBoard.myEEPROM.EEPROMDataBuffer[2]);
        myTestDevice.showOnLCD(1,_str);
        sprintf(_str,"%02X-%02X-%02X-%02X-%02X",myBoard.myEEPROM.EEPROMDataBuffer[3]
            ,myBoard.myEEPROM.EEPROMDataBuffer[4],myBoard.myEEPROM.EEPROMDataBuffer[5]
            ,myBoard.myEEPROM.EEPROMDataBuffer[6],myBoard.myEEPROM.EEPROMDataBuffer[7]);
            myTestDevice.showOnLCD(2,_str);
    break;
    case runStepList::testP5_ChargeTime:
        sprintf(_str,"T:%ds Cap:%.1fV",myTempVal.chargeTime,testr.Vcap_Max);myTestDevice.showOnLCD(1,_str);
        sprintf(_str,"In-Amp: %.3f A",myTempVal.InCurrent);myTestDevice.showOnLCD(2,_str);        
    break;
    case runStepList::testP6_DisChargeTime:
        sprintf(_str,"T:%ds Cap:%.1fV",myTempVal.DisChargeTime,myTempVal.VCap);myTestDevice.showOnLCD(1,_str);
        sprintf(_str,"L-Amp: %.3f A",myTempVal.LoadCurrent);myTestDevice.showOnLCD(2,_str);        
    break;
    case runStepList::testP0_END:
        sprintf(_str,"END.");myTestDevice.showOnLCD(1,_str);
        sprintf(_str,"Pass/Failed%d",myTempVal.result);myTestDevice.showOnLCD(2,_str);        
    break;
    
    default: 
        sprintf(_str,"empty");myTestDevice.showOnLCD(1,_str);
        myTestDevice.showOnLCD(2,_str); 
        break;
    }
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
            if(myArg.LabDevice_Load) MyLabDevice.SetLoadCurrent(__const_LoadCurrent);	

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
            if(myArg.LabDevice_Load) MyLabDevice.SetLoadCurrent(__const_LoadCurrent);	
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
void USV_TEST_UTIL_V2::run(){
    //static int32_t xxx=0;
    uint8_t _key=0;
    uint8_t _ledBlinkCnt=0;
    int i=0;
        myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
        //std::stringstream ss;
        //ss << "wait to Run ..." << xxx++;
        //myInterActReg.csLog = ss.str();
        DongleCheck();
        
        if(myBoard.boardName>0) 
        {
            ShowMyDongle();
            myTestDevice.showOnLCD(1,"<-          UpLoad->");
            myTestDevice.showOnLCD(4,"<-Option       RUN->");
            if(myArg.manualTest){
                run_ManualTest();
                myArg.manualTest=false;
            }
            if(_ledBlinkCnt++>5){ myTestDevice.setLED(FronLED,true);_ledBlinkCnt=0;} else {myTestDevice.setLED(FronLED,false);}
            //_key=myTestDevice.readKeys();
            //if (_key==0)
            _key=myInterActReg.getGuiCMD();
            switch (_key)
            {
                case USV_Test_Interface::_KEY::K2:
                    system("./DoPush.sh");
                    //system("git add --all");
                    //system("git commit -m \"ModifyDate $(date '+%Y-%m-%d %H:%M:%S')\"");
                    //git push origin main

                break;
                case (USV_Test_Interface::_KEY::K2 + USV_Test_Interface::_KEY::K4) :
                    myTestDevice.showOnLCD(1,"End of ");
                    myTestDevice.showOnLCD(2,"  Test Process!");
                    return;
                break;
                case USV_Test_Interface::_KEY::K1 :
                    myTestDevice.cleanLCD();
                    myTestDevice.setLED(FronLED,false);
                    //myBoard.init(myArg.ttyName); 
                    //myTestDevice.set0Current();
                    showLog("start to run_TestMachine...\n");
                    run_TestMachine();
                break;
                case USV_Test_Interface::_KEY::K3 :
                    myTestDevice.setLED(FronLED,false);
                    InformationMenu();
                break;                            
                /* code */
                break;        
                default:
                    usleep(10000);
                break;
            }        
        }
        /*else{
            myTestDevice.setLED(FronLED,true);
            if(Dongle==0)
                myTestDevice.showOnLCD(1,"No Dongle!");
            else
                myTestDevice.showOnLCD(1,"unKnown Dongle!");            
            myTestDevice.showOnLCD(2," ");
            myTestDevice.showOnLCD(3," ");
            myTestDevice.showOnLCD(4," ");
            usleep(100000);
        } */
         
    //}
}
void USV_TEST_UTIL_V2::forceStop(void){
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
    //myTestDevice.cleanLCD();
}


void USV_TEST_UTIL_V2::run_ManualTest(void){
    bool exit_status=false;
    static uint16_t time_mess=0;
    myDurationTimer.testTimeStartSec();
    uint8_t tt[32];   
    
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
    myTestDevice.setRelay(USV_Test_Interface::Relays::MPower,true);
    myTestDevice.setRelay(USV_Test_Interface::Relays::AR,true);
    if(myArg.LabDevice_PS || myArg.LabDevice_Load){
        if(myArg.LabDevice_PS) myTestDevice.setRelay(USV_Test_Interface::Relays::LabPowerSel,true);
        //if(myArg.LabDevice_Load) myTestDevice.setRelay(USV_Test_Interface::Relays::LabLoadSel,true);

        // Set Power Supply to 24V 2.5A & Turn Off Load
        MyLabDevice.SetPSEnable(true);
        MyLabDevice.SetPSVoltage(__const_PSVoltage);
        MyLabDevice.SetPSCurrent(__const_PSCurrent);
        MyLabDevice.SetLoadCurrent(0);	
    }
    myTestDevice.showOnLCD(1,"Manual Test");
    myTestDevice.showOnLCD(2,"Console...");                                    
    printf("\033[2H");
    printf("\033[2J");
    while(!exit_status){
        time_mess=myDurationTimer.TestTimeSec();
        printf("\n Duration : %dsec\n\n",time_mess);
        printf(" 1:Reset-GPO 2:Output-Off 3:FlyBack-Dis 4:FlyBack-En X:Next Step \n");
        printf(" 5/6:AufRuesten On/Off 8/9:Turn load On/Off E:Read-EEPROM C:CleanScreen\n");
        printf(" \n");
        printf("LTC3350 registers main info:\n");
        printf("____________________________\n");
        
        myBoard.showValue();
        if(myArg.showAll) myBoard.showValue_Rest();
        char chr=toupper(myCKey.getConsolKey(false));
        switch (chr){
            case '1':	myBoard.GPIOResetAll(); break;
            case '2':	myBoard.OutPUT_Off(); break;
            case '3':	myBoard.FlyBack_Off(); break;
            case '4':	myBoard.FlyBackEn(); break;
            case '5':	myTestDevice.setRelay(USV_Test_Interface::Relays::AR,true); break;
            case '6':	myTestDevice.setRelay(USV_Test_Interface::Relays::AR,false); break;
            case 'X':	exit_status=true; break;
            case 'C':	printf("\033[2J"); break;
            case 'U':	myBoard.CapRefUp(); break;
            case 'D':	myBoard.CapRefDown(); break;
            case 'P':	myBoard.VShuntUp(); break;
            case 'M':	myBoard.VShuntDown(); break;
            case 'R':	myBoard.RemessCapESR(); break;
            case '8':	
                myTestDevice.setRelay(USV_Test_Interface::Relays::Load,true);
                if(myArg.LabDevice_Load) MyLabDevice.SetLoadCurrent(__const_LoadCurrent);
            break;
            case '9':	
                myTestDevice.setRelay(USV_Test_Interface::Relays::Load,false);
                if(myArg.LabDevice_Load) MyLabDevice.SetLoadCurrent(0.0);
            break;
            case 'E':
                uint8_t __tmp[8];
               showLog("\nEEPROM:\n");
                myBoard.ReadEEPROMBlock(1,__tmp);
                myBoard.ReadEEPROMBlock(2,__tmp);
                myBoard.ReadEEPROMBlock(3,__tmp);
                myBoard.ReadEEPROMBlock(4,__tmp);
                myBoard.ReadEEPROMBlock(32,__tmp);
                break;
        }
        if(chr=='E')
            printf("\033[7A");
        if(chr!='X')
            printf("\033[27A");
        if(exit_status)
            time_mess=0;
        //fflush(stdout);
        usleep(1000);
        printf("\033[2H");
    }    
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
    myTestDevice.showOnLCD(1,".");
    myTestDevice.showOnLCD(2,".");
    if(myArg.LabDevice_PS || myArg.LabDevice_Load){
        MyLabDevice.SetPSEnable(false);
        MyLabDevice.SetPSVoltage(__const_PSVoltage);
        MyLabDevice.SetPSCurrent(__const_PSCurrent);
        MyLabDevice.SetLoadCurrent(0);	
    }
        
}
#define _mState_EEPROMCFG 20
void USV_TEST_UTIL_V2::runICA2308_simple_test(float __version){
    ICA_justEUI myICA2308;
    bool testingNoCap=false;    
    uint8_t __rKey=0;
    char _str1[32],_str2[32],_str3[32],_str4[32];
    std::ofstream file;
    uint8_t _mState = 0, LMState=0xFF;
    uint16_t _dcnt100ms=0;
    float __floatTmpVale=0;
    showLog("******************************");
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);            
    DongleCheck();  
    ShowMyDongle();
    
    //myArg.FileName_EUI="ICA-2308-EUI.csv";
    
    while((_mState<0xF0) && (xrunning==true)){        
        //showOnRun(_menuNo);
        //fflush(stdout);        
        usleep(100000);
        myTempVal.VIn= myTestDevice.getDUT_VIN();
        myTempVal.InCurrent = myTestDevice.getDUT_VINAmp();
        //myTempVal.VCC = myTestDevice.getDUT_VCC();
        
        switch (_mState){ 
            case 0://clear registers & reset Relays *********************************************
                myInterActReg.TR.DataClear();
                    
                //myTestDevice.cleanLCD();
                removeJPG_PFiles_Jobs();
                myTempVal.clear();
                myBoard.myEEPROM.clear_EEPROM_Buffer();
                myBoard.myEEPROM.myData.clear();
                testr.clear();
                testrReset();                
    //system("clear");
    //printf("ICA2308 READ EUI ...\n");            
    //sprintf(_str1,"ICA2308"); sprintf(_str2,"Read EUI ...");                
    //printf("Start: Reset Relays and Test start.");
                showLog("ICA2308 ReadEUI");
                showLog("Start: Reset Relays and Test start.");
                myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);                
            _mState++;
            break;           
            case 1:                 
                testrReset();
                showLog(" Ok.");
    //printf(" Ok.\n");
                _mState++;
            break;
            case 2: //set Power On *********************************************
                myTestDevice.setRelay(USV_Test_Interface::Relays::MPower,true);
                showLog("VCC test  3.1 < VCC < 3.6");                
                _dcnt100ms=5;//500mSec
            _mState++;
            break;         
            case 3://TEST1 : VCC test  3.1 < VCC < 3.6 ********************************************* 
                
    //myTestDevice.cleanLCD();
    //sprintf(_str1,"TEST1");
    //sprintf(_str2,"  VCC = %.1f",myTempVal.VCC);
    //printf("\rTest1: VCC test (3.1V < DUT[%.2fV] < 3.6V) ",myTempVal.VCC);
                myTempVal.VCC = myTestDevice.getDUT_VCC();
                if(myTempVal.VCC> 3.1 && myTempVal.VCC< 3.6){
                    showLog("Ok.");
    //printf(" Ok.\n");                        
                    _mState++;
                }else{
                    if(!(_dcnt100ms-->=0)){
                        printf("Failed!\n");
                        if(myTempVal.VCC< 3.1)
                            _mState=showError(ERROR::VCCIsLow);
                        else
                            _mState=showError(ERROR::VCCIsOver);
                    }                    
                }                
            break;
            case 4://TEST2 : Check EEPROM & serial Connection *********************************************
                showLog("Check EEPROM & serial Connection");
    //myTestDevice.cleanLCD();
    //sprintf(_str1,"Test2");
    //sprintf(_str2,"  I2c Connection");
    //printf("\nTest2: I2c Connection.\n");
                sleep(2);
                //system("i2cdetect -y 3");
                _dcnt100ms=5;
                _mState++;
            break;
            case 5:
                if(_dcnt100ms-->0){
                    if(myICA2308.ReadEUI()){
                        //printf("%s",myICA2308.showEUI().c_str());
                        _mState++;
                    }
                    else{
                        //printf("ReadEUI Failed!!!!!!!!!\n");
                        showLog("ReadEUI Failed!!!!!!!!!");
                    }
                }
                else
                {
                        printf(" Failed to I2C Connection! \n"); 
                        _mState=showError(ERROR::I2CFailed);
                }
				break;
            case 6:
                for(int i=0;i<8;i++)
                    myBoard.myEEPROM.myData.EUI[i]=myICA2308.EUI_Buffer[i];
                myBoard.myEEPROM.ShowEUI(); //just for test  
                myInterActReg.TR.UID_str=myBoard.myEEPROM.myData.getEUI_Str();
                showLog(myInterActReg.TR.UID_str+"\n");
                _mState++;                  
            break;            
            case 7:
                _mState=25;
            break;
            case 25:// Label Print
                showLog("\nLabel Print... ");
                _mState++;        
                break;
            case 26:
                char tmp1[50];
                char tmp2[50];
                myBoard.updateBoardNameStr();
                sprintf(myBoard.boardName_str,"2308R%.1f",__version );
                sprintf(myBoard.boardKind_str,"Linux Base");
                sprintf(tmp1,"S-Nr:%s",myBoard.myEEPROM.myData.getEUI5Byte_Str().c_str());
                sprintf(tmp2,"ICA%s",myBoard.boardName_str);
                qrcode_jpeg_output(myBoard.myEEPROM.myData.getEUI5Byte_Str().c_str(),myBoard.boardKind_str,tmp2,tmp1);
                
                if(QL700_Print()==0){                        
                    QL700_Print();
                    sprintf(_str2,"Ok.");
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
                showLog("\nEND Wait to press Key.\n");
                _mState=0xFE;
                break;
        
        }
        if(_mState!=0xFF){
            //if(LMState!= _mState){ LMState = _mState; myTestDevice.cleanLCD();}
            if(strlen(_str1)>0 ) {myTestDevice.showOnLCD(1,_str1);_str1[0]=0;}
            if(strlen(_str2)>0 ) {myTestDevice.showOnLCD(2,_str2);_str2[0]=0;}
            if(strlen(_str3)>0 ) {myTestDevice.showOnLCD(3,_str3);_str3[0]=0;}
            if(strlen(_str4)>0 ) {myTestDevice.showOnLCD(4,_str4);_str4[0]=0;}            
        }

        myTempVal.result=_mState;        
        myInterActReg.TR.InCurrent=myTempVal.InCurrent;
        myInterActReg.TR.Vin=myTempVal.VIn;
        myInterActReg.TR.Vvcc=myTempVal.VCC;        
        //if(myTestDevice.readKeys()==USV_Test_Interface::_KEY::K3) _mState=0xF1;
    }
    
    //myTestDevice.cleanLCD();
    //myTestDevice.showOnLCD(2,"SAVE DATA");
    showLog("SAVE DATA...");
    //just Save EUI for ICA2308
    
    //SaveResult(true,myArg.StoreFolderPath+myArg.FileName_EUI,myArg.StoreFolderPath+myArg.FileName_Test);    
    SaveEUI(myArg.StoreFolderPath+myArg.FileName_EUI,true);
    
    
    //SaveResult(false,myArg.StoreFolder+myArg.FileName_EUI,myArg.StoreFolder+myArg.FileName_Test);//testingNoCap);
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
    //myTestDevice.showOnLCD(2,"Press Key");
    //myTestDevice.showOnLCD(3,"to Continue...");    
    //myTestDevice.showOnLCD(4,"<-");    
    //waitToPressKey();
}
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
                testr.clear();
                testrReset();                
                showLog("ICA2407 ReadEUI");
                showLog("Start: Reset Relays and Test start.");
                myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);                
            _mState++;
            break;           
            case 1:                 
                testrReset();
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
                myInterActReg.TR.UID_str=myBoard.myEEPROM.myData.getEUI_Str();
                showLog(myInterActReg.TR.UID_str+"\n");
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


uint8_t USV_TEST_UTIL_V2::run_GPIO_Test(void){
    uint8_t _mState=0;
    uint8_t wait_dCnt=5;
    char _str[32];
    uint8_t _readCnt=10;
    //sprintf(_str,"GPIO TEST");myTestDevice.showOnLCD(1,_str);
    //sprintf(_str,"...");
    while(1){
        
        myTempVal.VIn= myTestDevice.getDUT_VIN();
        myTempVal.InCurrent = myTestDevice.getDUT_VINAmp();
        myTempVal.VOut= myTestDevice.getDUT_VOUT();
        myTempVal.LoadCurrent = myTestDevice.getDUT_VOUTAmp();    
        
        
        myTempVal.VCap=myBoard.GetVCap(0);
        

        //myTestDevice.showOnLCD(2,_str);
        //fflush(stdout);
        usleep(500000);
        switch (_mState)
        {
        case 0: // turn On Load and clear GPIO and set Aufruesten
            sprintf(_str,"Start...");
            /*if(myBoard.init(myArg.ttyName)){
                myBoard.GPIOResetAll();
                if(myBoard.GPIO_read()!=false){
                    _mState++;
                    wait_dCnt=5;
                } 
            }*/                    
			myBoard.GPIOResetAll();
            _mState++;
            wait_dCnt=5; 

            break;
        case 1:
            myTestDevice.setRelay(USV_Test_Interface::Relays::LabPowerSel,false);
            myTestDevice.setRelay(USV_Test_Interface::Relays::VCCLoad,false);

            myTestDevice.setRelay(USV_Test_Interface::Relays::MPower,true);
            myTestDevice.setRelay(USV_Test_Interface::Relays::AR,true);
            myTestDevice.setRelay(USV_Test_Interface::Relays::Load,true);

            if(myArg.LabDevice_Load) MyLabDevice.SetLoadCurrent(__const_LoadCurrent);
            if((myTestDevice.readRelay()&0x15)!=0x15)
                sprintf(_str,"start... %d",myTestDevice.readRelay());                
            else{
                _mState++;            
                wait_dCnt=5; 
            }            
            break;
        case 2: //wait to Power Current More then 500mA
            printf("S2 INAmp:%.3fA\n",myTempVal.InCurrent); 
            testr.VOut1=myTempVal.VOut;
            if(myTempVal.InCurrent> 0.5 ) _mState++;
            break;
        case 3: //Set FlayBack Off.
            myInterActReg.TR.currentTestNo=6;
            sprintf(_str,"T6 FlyBack-Dis...");
            printf("Test6: FlyBack-Dis... ");
            showLog("Test6: FlyBack-Dis... ");
            wait_dCnt=20;
            _mState++;
            break;
        case 4:  // check FlyBack Dis...
            sprintf(_str,"S6  INAmp:%.3fA ",myTempVal.InCurrent);
            printf("\rTest6: FlyBack-Dis... %.3fA   ",myTempVal.InCurrent);
            testr.VOut2=myTempVal.VOut;
            myBoard.FlyBack_Off();
            if(myTempVal.InCurrent!=-1 && myTempVal.InCurrent< 0.5) _mState++;
            break;
        case 5: //FlyBack diss is Ok        
            sprintf(_str,"T6 Ok.");
            showLog("Ok.\n");
            myInterActReg.TR.FlyBackDis=true;
            wait_dCnt=20;_mState++;
            break;        
        case 6:  // check FlyBack reset
            //sprintf(_str,"S2 INAmp:%.3fA",myTempVal.InCurrent);
            if(myTempVal.InCurrent> 0.5) _mState++;
            myBoard.GPIOResetAll();
            break;
        case 7:  //trun off Aufruesten
            wait_dCnt=20; 
            _mState++;
            break;
        case 8: //wait to Power Current less then 500mA
            //sprintf(_str,"S8 INAmp:%.3fA\n",myTempVal.InCurrent);myTestDevice.showOnLCD(2,_str);
            //printf("\r",_str);
            myTestDevice.setRelay(USV_Test_Interface::Relays::AR,false);            
            if(myTempVal.InCurrent!=-1 && myTempVal.InCurrent< 0.5 ) _mState++;
            break;
        case 9: //Set FlyBack Enable
            myInterActReg.TR.currentTestNo=7;
            sprintf(_str,"T7 FlyBack-En...");
            showLog("Test7: FlyBack-En... ");
            wait_dCnt=20; _mState++;
            break; 
        case 10: //wait to Power Current More then 500mA
            //sprintf(_str,"S10 INAmp:%.3fA\n",myTempVal.InCurrent);myTestDevice.showOnLCD(2,_str);
            //printf(_str);
            myBoard.FlyBackEn();
            if(myTempVal.InCurrent> 0.5 )  _mState++;
            break;
        case 11: 
            sprintf(_str,"T7 OK");
            showLog("Ok.\n");
            myInterActReg.TR.FlayBackEn=true;            
            myBoard.GPIOResetAll();
            wait_dCnt=20; 
            _mState++;
            break;
        case 12: 
            myTestDevice.setRelay(USV_Test_Interface::Relays::AR,true);
            myTestDevice.setRelay(USV_Test_Interface::Relays::Load,true);                        
            if( myTempVal.LoadCurrent> 0.2 ) _mState++; 
            break;
        case 13: 
            myInterActReg.TR.currentTestNo=8;
            sprintf(_str,"T8 Output-SW Off");
            showLog("Test8: Output-SW ... ");
            myBoard.OutPUT_Off();wait_dCnt=150;
            myDurationTimer.testTimeStartSec();            
            _mState++;
            break;
        case 14: 
            myTempVal.WaitToOutSWOffTime=myDurationTimer.TestTimeSec();                
            sprintf(_str,"T8(wait) %d sec (%.1f)",myTempVal.WaitToOutSWOffTime,myTempVal.InCurrent);
            printf("\r   wait to Output off time : %d sec... ",myTempVal.WaitToOutSWOffTime);            
            myBoard.OutPUT_Off();
            if(myTempVal.LoadCurrent!=-1 && myTempVal.LoadCurrent< 0.2 ){
                showLog(_str);
                if(myTempVal.WaitToOutSWOffTime>=testr.Limit_MIN_WaitToOutSwOff && myTempVal.WaitToOutSWOffTime<=testr.Limit_MAX_WaitToOutSwOff){
                    myDurationTimer.testTimeStartSec();
                    _mState++; 
                }
                else{
                    showLog("   wait to Output off time is not in range \n");
                    testr.ErrorNo=ERROR::waitToOutSwOff;
                    sprintf(_str,"Err.%d",testr.ErrorNo);myTestDevice.showOnLCD(2,_str);
                    printf("\nGPIO Test Error (%d) on step %d\n",testr.ErrorNo,_mState);
                    showError(testr.ErrorNo);
                    return testr.ErrorNo;                    
                }
                
            } 
            break;
        case 15: 
            sprintf(_str,"T8 Ok");
            showLog("Ok.\n"); 
            myInterActReg.TR.OutSwOff=true;            
            myBoard.GPIOResetAll();  wait_dCnt=150; 
            _mState++;
        break;
        case 16:
            myTempVal.OutSWOffTime=myDurationTimer.TestTimeSec();
            sprintf(_str,"T8 %d sec (%.1f)",myTempVal.OutSWOffTime,myTempVal.InCurrent);
            printf("\r   Output Off time : %d sec... ",myTempVal.OutSWOffTime);
            myBoard.GPIOResetAll();
            if(myTempVal.LoadCurrent> 0.2 ){
                showLog(_str);
                if(myTempVal.OutSWOffTime>testr.Limit_MIN_OutSwOff && myTempVal.OutSWOffTime<testr.Limit_MAX_OutSwOff){
                    _mState++; 
                }else{
                    showLog("   Output off time is not in range \n");
                    testr.ErrorNo=ERROR::OutSwOff;
                    sprintf(_str,"Err.%d",testr.ErrorNo);myTestDevice.showOnLCD(2,_str);
                    printf("\nGPIO Test Error (%d) on step %d\n",testr.ErrorNo,_mState);
                    showError(testr.ErrorNo);
                    return testr.ErrorNo;                    
                }
                                
            } 
            break;
        case 17:
            //sprintf(_str,"S17");
            testr.time_WaitToOutSwOff=myTempVal.WaitToOutSWOffTime;
            testr.time_OutSwOff=myTempVal.OutSWOffTime;
            //showLog("\n");
            _mState++;
            break;
        default:
            //myBoard.FlyBack_Off();
            sprintf(_str,"Ok.");myTestDevice.showOnLCD(2,_str);
            //showLog("GPIO Test Ok.");
            //waitToPressKey();
            return 0;
            break;
        }
        if(--wait_dCnt<=0)
        { 
            testr.ErrorNo=100+_mState;// ERROR::GPIO;
            if(_mState==2) testr.ErrorNo=ERROR::FlyBackdis;
            if(_mState==8) testr.ErrorNo=ERROR::FlyBackEn;
            if(_mState==12) testr.ErrorNo=ERROR::OutSwOff;
    sprintf(_str,"Err.%d",testr.ErrorNo);myTestDevice.showOnLCD(2,_str);
    printf("\nGPIO Test Error (%d) on step %d\n",testr.ErrorNo,_mState);
            showError(testr.ErrorNo);

            //waitToPressKey();
            return testr.ErrorNo;
        }	
        myInterActReg.TR.InCurrent=myTempVal.InCurrent;
        myInterActReg.TR.LoadCurrent=myTempVal.LoadCurrent;
        myInterActReg.TR.VOut=myTempVal.VOut;
        myInterActReg.TR.Vin=myTempVal.VIn;
        myInterActReg.TR.VcapBank=myTempVal.VCap;
        myInterActReg.TR.Vvcc=myTempVal.VCC;
        myInterActReg.TR.OutSWOffTime=myTempVal.OutSWOffTime;
        myInterActReg.TR.WaitToOutSWOffTime=myTempVal.WaitToOutSWOffTime;
        
        		
    }
    

}

void USV_TEST_UTIL_V2::run_TestMachine(void){
    uint8_t _key=0;
    std::ostringstream oss;
    bool testingNoCap=false;    
    uint8_t __rKey=0;
    std::ofstream file;
    uint8_t _mState = 0, LMState=0xFF;
    uint16_t _dcnt100ms=0;
    uint16_t __lastTimeValue=0;
    float __floatTmpVale=0;
    uint8_t __tempBatBack__error__cnt=0;
    float __tempICtempVal=0.0;
    uint8_t __tempIC__error__cnt=0;
    uint8_t __diffVcap__error__cnt=0;
    uint8_t __error_cnt=0;
    showLog("******************************");
    //myArg.FileName_EUI="ICA-NT-EUI.csv";
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);            
    DongleCheck();  
    ShowMyDongle();
    while((_mState<0xF0) && (xrunning==true)){        
        //fflush(stdout);        
        usleep(100000);
        myTempVal.VIn= myTestDevice.getDUT_VIN();
        myTempVal.InCurrent = myTestDevice.getDUT_VINAmp();
        myTempVal.VOut= myTestDevice.getDUT_VOUT();
        myTempVal.LoadCurrent = myTestDevice.getDUT_VOUTAmp();
        
        
        switch (_mState){ 
            case 0://clear registers & reset Relays *********************************************
                myInterActReg.TR.DataClear();
                //myTestDevice.cleanLCD();
                removeJPG_PFiles_Jobs();
                myTempVal.clear();
                myBoard.myEEPROM.clear_EEPROM_Buffer();
                myBoard.myEEPROM.myData.clear();
                testr.clear();
                testrReset();                
                //system("clear");
                showLog("ICA2315/ICA2405 Testing Routine...\n");
                showLog("Start: Reset Relays and Test start.");
                myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
                if(myArg.LabDevice_PS || myArg.LabDevice_Load){
                    if(myArg.LabDevice_PS) 
                        if (MyLabDevice.ReadPSCurrent()==-1){ _mState=showError(ERROR::LabPSNoAnswer);break; }
                    if(myArg.LabDevice_Load) 
                        if(MyLabDevice.ReadLoadVoltage()==-1){ _mState=showError(ERROR::LabLoadNoAnswer);break; }                    
                    if(myArg.LabDevice_PS) myTestDevice.setRelay(USV_Test_Interface::Relays::LabPowerSel,true);
                    //if(myArg.LabDevice_Load) myTestDevice.setRelay(USV_Test_Interface::Relays::LabLoadSel,true);        
                    // Set Power Supply to 24V 500mA & Turn Off Load
		            MyLabDevice.SetPSEnable(true);
		            MyLabDevice.SetPSVoltage(__const_PSVoltage);
		            MyLabDevice.SetPSCurrent(__const_PSCurrent_NoCap_NoLoad);
		            MyLabDevice.SetLoadCurrent(0);	
                }
                _mState++;
            break;           
            case 1:                 
                testrReset();
                showLog(" Ok.\n");
                _mState++;
            break;
            case 2: //set Power On *********************************************
                myTestDevice.setRelay(USV_Test_Interface::Relays::MPower,true);
                _dcnt100ms=5;//500mSec
                _mState++;
            break;     
            case 3: //TEST1: wait for a 500mSec (MPOWER:ON AR:OFF -> No Current) *********************************************
                myInterActReg.TR.currentTestNo=1;
                showLog("Test1: No Current when Aufruesten is Off...");
                _mState++;            
            break;
            case 4:
                if(_dcnt100ms-->0){
                    //myTempVal.InCurrent=myTestDevice.getDUT_VINAmp();
                    if (myArg.LabDevice_PS) myTempVal.InCurrent=MyLabDevice.ReadPSCurrent();
                    if(myTempVal.InCurrent>.020){
                        showLog("Failed (Current > 20mA)!\n");
                        myInterActReg.TR.AR_Off=false;
                        _mState=showError(ERROR::Aufruesten);
                    }
                }
                else{
                    if(myArg.LabDevice_PS) MyLabDevice.SetPSCurrent(__const_PSCurrent);		            
                    myTestDevice.setRelay(USV_Test_Interface::Relays::AR,true);
                    myDurationTimer.testTimeStartSec();//ChargeTime Start
                    myInterActReg.TR.AR_Off=true;
                    showLog(" Ok.\n");
                    _mState++;
                }                    
            break;
            case 5:// TEST2: Mpower:ON & AR:ON -> current:300mA *********************************************
                myInterActReg.TR.currentTestNo=2;
                showLog("Test2: Turn On AufRuesten... ");                
                _dcnt100ms=5;//500mSec
                _mState++;    
            break;
            case 6: 
                if(_dcnt100ms-->0){
                    //myTempVal.InCurrent=myTestDevice.getDUT_VINAmp();
                    if (myArg.LabDevice_PS) myTempVal.InCurrent=MyLabDevice.ReadPSCurrent();
                    if(myTempVal.InCurrent>.300){ 
                        myInterActReg.TR.AR_On=true;
                        showLog(" Ok.\n");
                        _mState++;
                    }
                    else{                
                        myInterActReg.TR.AR_On=false;
                        showLog("Failed! (Current<300mA)\n");
                        _mState=showError(ERROR::AufruestenOn);    
                        
                    }                    
                }
            break;
            case 7://TEST3 : VCC test  3.1 < VCC < 3.6 ********************************************* 
                myInterActReg.TR.currentTestNo=3;
                myTempVal.VCC = myTestDevice.getDUT_VCC();
                showLog((std::ostringstream{} << "Test3: VCC test (3.1V < DUT[" << std::fixed << std::setprecision(2) << myTempVal.VCC << "V] < 3.6V)").str());
                myInterActReg.TR.Vvcc=myTempVal.VCC;
                if(myTempVal.VCC> 3.1 && myTempVal.VCC< 3.6){                    
                    showLog(" Ok.\n");                        
                    _mState++;
                }else{
                    if(!(_dcnt100ms-->=0)){
                        showLog("Failed!\n");
                        if(myTempVal.VCC< 3.1)
                            _mState=showError(ERROR::VCCIsLow);
                        else
                            _mState=showError(ERROR::VCCIsOver);
                    }
                    
                }                
            break;
            case 8://TEST4 : Check EEPROM & serial Connection *********************************************
                myInterActReg.TR.currentTestNo=4;
                showLog("Test4: Board Connection.");
                _dcnt100ms=250;//25Sec
                _mState++;
            break;
            case 9:
                if(_dcnt100ms-->0){
                    if(!myBoard.init(myArg.ttyName)){
                        showLog(" Failed to Serial Port Connection (INI)! \n");                         
                        myInterActReg.TR.UART_Con=false;
                        //_mState=showError(ERROR::BoardConnection1);    
                    }else{
                        myBoard.GPIOResetAll();
				        if(myBoard.GPIO_read()!=false){
                            showLog((std::ostringstream{} << "UART Ok (in " << std::fixed << std::setprecision(2) << myDurationTimer.TestTimeSec()  << "sec)").str());
                            
                            myInterActReg.TR.UART_Con=true;
                            _mState++;  
                        } 				
                    }                    
				    
                }else{
                    showLog(" Failed to Serial Port Connection! \n"); 
                    myInterActReg.TR.UART_Con=false;
                    _mState=showError(ERROR::BoardConnection1);
                }
				break;
            case 10:
                if (myBoard.readEUI64(myBoard.myEEPROM.myData.EUI)){
                    if(myBoard.myEEPROM.ismicroChipIC()){                        
                        //myBoard.myEEPROM.ShowEUI(); 
                        myInterActReg.TR.UID_str=myBoard.myEEPROM.myData.getEUI_Str();
                        showLog(myInterActReg.TR.UID_str+"\n");
                        if(myBoard.readDataBuff(myBoard.myEEPROM.EEPROMDataBuffer))
                            testr.EEPROM_Status=myBoard.myEEPROM.CheckDataVersionProcess(false);                            
                            myInterActReg.TR.EEPROMBuff_str=myBoard.myEEPROM.myData.getAll_Str(); 
    myBoard.myEEPROM.EEPROMDataBuffShow();                           
    myBoard.myEEPROM.myData.show(); 
                        _mState++;	
                        _dcnt100ms=5;				
                    }	
                    else {
                        showLog("unKnown ");
                        //myBoard.myEEPROM.ShowEUI(); //just for test
                        myInterActReg.TR.UID_str=myBoard.myEEPROM.myData.getEUI_Str();
                        showLog("-- "+myInterActReg.TR.UID_str+"");
                        
                        _mState=showError(ERROR::BoardConnection2); 
                    }
                }
                else {_mState=showError(ERROR::BoardConnection3); }
            break;
            case 11:
                if(myArg.EEPROMCFG) {_mState=_mState_EEPROMCFG;}
					else{
						if (file.is_open()) { file.close(); }
						file.open("./tmp/chargeCurve.csv", std::ios::out);
						file << "time,voltage,current"<< std::endl;
						_mState++;
					}	
                break;
            case 12:// TEST5: wait to SCap full (PSCurrent<150mA) *********************************************
                myInterActReg.TR.currentTestNo=5;
                showLog("Test5: wait to charge SCap");
                _mState++;
            break;
            case 13: 
                //myTempVal.InCurrent=myTestDevice.getDUT_VINAmp();
                if(myArg.LabDevice_PS) myTempVal.InCurrent= MyLabDevice.ReadPSCurrent();
                myTempVal.chargeTime=myDurationTimer.TestTimeSec();                
                if(myTempVal.chargeTime!=myTempVal.ltime_mess){
                    myTempVal.VCap=myBoard.GetVCap(0);
                    //myInterActReg.TR.Vcap1=myBoard.GetVCap(1);
                    //myInterActReg.TR.Vcap2=myBoard.GetVCap(2);
                    //myInterActReg.TR.Vcap3=myBoard.GetVCap(3);
                    //myInterActReg.TR.Vcap4=myBoard.GetVCap(4);
                                
                    file << myTempVal.chargeTime<<","<< std::fixed
						<<std::setprecision(1)<<myTempVal.VCap <<","
						<<std::setprecision(3)<< myTempVal.InCurrent<< std::endl;
                    myTempVal.ltime_mess=myTempVal.chargeTime;
                }
                if(myTempVal.InCurrent!=-1){
                    //printf("\rTest5: wait to charge SCap  [Time: %dSec Current: %.3fA > (limit)%dmA]         ",myTempVal.time_mess,myTempVal.InCurrent,(int32_t)(testr.Limit_MIN_FullChargeCurrent*1000));
                    /*if(__lastTimeValue!=myTempVal.chargeTime){
                        __lastTimeValue=myTempVal.chargeTime;
                        showLog((std::ostringstream{} <<"Test5: wait to charge SCap  [Time:"<< myTempVal.chargeTime << "Sec Current: "<< std::fixed << std::setprecision(3)<< myTempVal.InCurrent << "A > (limit)"<< static_cast<int32_t>(testr.Limit_MIN_FullChargeCurrent * 1000)<< "mA]").str());
                    }*/
                    myBoard.CheckCapsVoltage(&testr.Vcap_Max);
                    __error_cnt=0;
                }
                else{

                    printf("\n failed !!!Current read Error!!! (Value:%.2f)\n",myTempVal.InCurrent);
                    showLog((std::ostringstream{} <<" failed !!!Current read Error!!! (Value:"<< std::fixed << std::setprecision(2)<< myTempVal.InCurrent<<")").str());
                    if (__error_cnt++>3) _mState=showError(ERROR::ChargeDuration);
                }                
                if ((myTempVal.chargeTime > testr.Limit_MAX_Charge_time) & (myTempVal.InCurrent> testr.Limit_MIN_ChargeCurrent)) {
                    showLog((std::ostringstream{} << "\nTEST5.Error!!!  Time (" 
                        << myTempVal.chargeTime << " > " << testr.Limit_MAX_Charge_time 
                        << ") Current (" 
                        << myTempVal.InCurrent << " > " << testr.Limit_MIN_ChargeCurrent 
                        << ")\n").str());
                }
                if (myTempVal.chargeTime > testr.Limit_MAX_Charge_time){//+__Limit_MAX_ExtendChargeTime){                    
                    showLog((std::ostringstream{}<< "\nTEST5.Error!!!  Time ("
                        << myTempVal.chargeTime << " > "
                        << (testr.Limit_MAX_Charge_time)// + __Limit_MAX_ExtendChargeTime)
                        << ")\n" ).str());
                    _mState=showError(ERROR::ChargeDuration);
                }
                if(_mState<0xF0 && myTempVal.InCurrent < testr.Limit_MIN_FullChargeCurrent && myTempVal.InCurrent > 0){
                    testr.time_charge=myTempVal.chargeTime;
                    _mState++;
                }				    
            break;
            case 14:// GPIO Test *********************************************
                myTestDevice.cleanLCD();
                showLog("\nGPIO Test... ");
                if(run_GPIO_Test()==0){
                    _mState++;
                    myBoard.init(myArg.ttyName);
                } 
                else _mState=0xFE;
            break;
            case 15:       
                myTestDevice.setRelay(USV_Test_Interface::Relays::Load,false);
                if(myArg.LabDevice_PS) MyLabDevice.SetPSCurrent(__const_PSCurrent);
				if(myArg.LabDevice_Load) MyLabDevice.SetLoadCurrent(0);                
                _mState++;
            break;
            case 16:
                //myTempVal.InCurrent=myTestDevice.getDUT_VINAmp();
                if(myArg.LabDevice_PS) myTempVal.InCurrent = MyLabDevice.ReadPSCurrent();					
                if (myTempVal.InCurrent!=0 && myTempVal.InCurrent<.2) _mState++;
            break;
            case 17:// TEST9 : Discharge *********************************************
                showLog("Test9: wait to disCharge SCap");
                myInterActReg.TR.currentTestNo=9;
                myTestDevice.setRelay(USV_Test_Interface::Relays::Load,true);
                if(myArg.LabDevice_PS) MyLabDevice.SetPSCurrent(__const_PSCurrent);
                if(myArg.LabDevice_Load) MyLabDevice.SetLoadCurrent(__const_LoadCurrent);                                
                myDurationTimer.testTimeStartSec();
                myTestDevice.setRelay(USV_Test_Interface::Relays::AR,false);				            
				if (file.is_open()) { file.close(); }
				file.open("./tmp/disChargeCurve.csv", std::ios::out);
				file << "time,voltage"<< std::endl;
				_mState++;	
            break;
            case 18:            
                myTempVal.LoadCurrent = myTestDevice.getDUT_VOUTAmp();
                myTempVal.VOut= myTestDevice.getDUT_VOUT();
                if(myArg.LabDevice_Load){
                    myTempVal.VOut = MyLabDevice.ReadLoadVoltage();
                    myTempVal.LoadCurrent  = MyLabDevice.ReadLoadCurrent(); 
                }
                
                myTempVal.VCap=myBoard.GetVCap(0);
                
                myTempVal.DisChargeTime=myDurationTimer.TestTimeSec();
                if(!myBoard.CheckCapsVoltage(&myTempVal.VCap)){
                    myTempVal.VCap=0;
                    if(__diffVcap__error__cnt++>5)
                        _mState=showError(ERROR::VCapsNotSame);//testr.ErrorNo=ERROR::VCapsNotSame;
                }else {__diffVcap__error__cnt=0;}

                if(!myBoard.GetICTemp(&__tempICtempVal,true) || (__tempICtempVal>__Limit_MAX_IC_Temp)) {
                    if(__tempIC__error__cnt++>5)
                        _mState=showError(ERROR::TempSensor_IC);//testr.ErrorNo=ERROR::TempSensor;
                }else {
                    testr.tempIC=__tempICtempVal;
                    __tempIC__error__cnt=0;}

                if(!myBoard.GetBatBankTemp(&testr.tempBatBank,true) || (testr.tempBatBank>__Limit_MAX_BatBank_Temp)) 
                {
                    if(__tempBatBack__error__cnt++>5) _mState=showError(ERROR::TempSensor_CapBank);//testr.ErrorNo=ERROR::TempSensor;                
                }
                        file << myTempVal.DisChargeTime<<","<< std::fixed
                    <<std::setprecision(1)<<myTempVal.VCap << std::endl;
                //testr.ErrorNo=0;
                usleep(100000);
                if(myTempVal.VCap>0) testr.VCap_SWOff=myTempVal.VCap;

                if ( myTempVal.LoadCurrent>-1 && myTempVal.LoadCurrent<0.05 ) {
                    showLog("\n");
                    testr.time_DisCharge=myTempVal.DisChargeTime;
                    if(testr.VCap_SWOff< testr.Limit_MIN_VCap_ShutdownVoltage || testr.VCap_SWOff> testr.Limit_MAX_VCap_ShutdownVoltage){
                        showLog((std::ostringstream{} << "Vcap on Shut Down is:" << std::fixed << std::setprecision(2) << testr.VCap_SWOff << "V").str());                            
                        _mState=showError(ERROR::VCapShutDownOutOfRange);//testr.ErrorNo=ERROR::VCapOutOfRange;
                    }
                    else{
                    _mState++;
                    _dcnt100ms=50;//25Sec
                    }
                    
                }	
                //myInterActReg.TR.Vcap1=myBoard.GetVCap(1);
                //myInterActReg.TR.Vcap2=myBoard.GetVCap(2);
                //myInterActReg.TR.Vcap3=myBoard.GetVCap(3);
                //myInterActReg.TR.Vcap4=myBoard.GetVCap(4);
                    			                
            break;
            case 19:
                myInterActReg.TR.currentTestNo=10;
                //Ask for Manual Test
                if(_dcnt100ms-->0){
                    if(_dcnt100ms>10){
                        myTestDevice.setRelay(USV_Test_Interface::Relays::AR,true);	
                        myTestDevice.setRelay(USV_Test_Interface::Relays::MPower,true);	
                        myTestDevice.setRelay(USV_Test_Interface::Relays::Load,true);
                    }
                    else{
                        myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
                    }
                }
                else{
                    _mState++;
                    _dcnt100ms=250;
                }
            break;
            case 20:            
                if(_dcnt100ms-->0){
                    if(!myBoard.init(myArg.ttyName)){
                        showLog(" Failed to Serial Port Connection (INI)! \n"); 
                        myInterActReg.TR.UART_Con=false;
                    }else{
                        myBoard.GPIOResetAll();
				        if(myBoard.GPIO_read()!=false){
                            showLog((std::ostringstream{} << "UART Ok (in " << std::fixed << std::setprecision(2) << (100-_dcnt100ms) << "th try)").str());
                            myInterActReg.TR.UART_Con=true;
                            _mState++;  
                        } 				
                    }                    
				    
                }else{
                    showLog(" Failed to Serial Port Connection! \n"); 
                    myInterActReg.TR.UART_Con=false;
                    _mState=showError(ERROR::BoardConnection1);
                }
				break;
            break;
            case 21:
                //check EEPROM to Save Data ....        
                //showLog("\n");
                showLog("Current EEPROM");
                myBoard.myEEPROM.EEPROMDataBuffShow();
                if (testr.EEPROM_Status==EEPROMProcesSts::Ok){
                    myBoard.myEEPROM.getLastVerData();
                    myBoard.myEEPROM.myData.show();
                    //showLog(myBoard.myEEPROM.myData.getAll_Str());
                    showLog(myBoard.myEEPROM.myData.getAll_StrExpress());
                    checkForOldTestingData(myArg.StoreFolderPath+myArg.FileName_EUI);                    
                }
                else
                {
                    _mState++;
                }
                _mState++;
            break;
            case 22:
                if(myInterActReg.msgBox.waitForUser("EEPROM Over write?","Yes","NOOO",20)){
                    showLog("YES******************************");
                    _mState=24;
                }
                else{
                    showLog("No******************************");
                    //showLog("!!!!!!!!!!!!!!!!!!  Ovrwrite Disable  !!!!!!!!!!!!!!!!!!");
                    _mState=26;
                }
                
                //_mState+=3;
            break;
            case 23:
                __rKey=0;
                /*while(__rKey==0){
                    __rKey=myTestDevice.readKeys();
                    usleep(50000);
                }
                switch(__rKey){
                    case USV_Test_Interface::_KEY::K2: _mState++;break;
                    case USV_Test_Interface::_KEY::K1: _mState+=2;break;
                    default: _mState--;
                }*/   
                _mState++;  
                
            break;
            case 24:
                //if(myArg.EEPROMCFG) testr.VCap_SWOff=0;
                myBoard.myEEPROM.updateBoardInfo(myBoard.boardName,myBoard.boardVer,myBoard.myBoardInfo,testr);
                myBoard.myEEPROM.getCurrentTime();						
                myBoard.myEEPROM.BuffUpdate_LVer();
                myBoard.myEEPROM.EEPROMDataBuffShow();//just for test
                myBoard.myEEPROM.myData.show();
                showLog(myBoard.myEEPROM.myData.getAll_Str());   
    myBoard.myEEPROM.EEPROMDataBuffShow();                           
    myBoard.myEEPROM.myData.show();                  
                _mState++;                
                _dcnt100ms=10;
            break;
            case 25:
                
                if(_dcnt100ms-->0){
                    if(!myBoard.init(myArg.ttyName)){
                        showLog(" Failed to Serial Port Connection (INI)! \n"); 
                        myInterActReg.TR.UART_Con=false;
                        //_mState=showError(ERROR::BoardConnection1);    
                    }else{
                        myBoard.GPIOResetAll();
				        if(myBoard.GPIO_read()!=false){
                            showLog((std::ostringstream{} << "UART Ok (in " << std::fixed << std::setprecision(2) << (100-_dcnt100ms) << "th try)").str());
                            myInterActReg.TR.UART_Con=true;
                            showLog("EEPROM write\n");
                            if(myBoard.writeDataBuff(myBoard.myEEPROM.EEPROMDataBuffer))
                                _mState++;
                            else
                                showLog("\nEEPROM-Write Error!\n");                                                                
                        } 				
                    }                    				    
                }                                        
            break;
            case 26:// Label Print
                showLog("Label Print... ");
                _mState++;        
            break;
            case 27:
                char tmp1[50];
                char tmp2[50];
                myBoard.updateBoardNameStr();
                sprintf(tmp1,"S-Nr:%s",myBoard.myEEPROM.myData.getEUI5Byte_Str().c_str());
                sprintf(tmp2,"ICA%s",myBoard.boardName_str);
                qrcode_jpeg_output(myBoard.myEEPROM.myData.getEUI5Byte_Str().c_str(),myBoard.boardKind_str,tmp2,tmp1);                
                if(QL700_Print()==0){  
                    QL700_Print();
                    showLog(" Ok.\n");
                    _mState++; 
                }
                else{
                    showLog(" Error.\n");
                    _mState=showError(ERROR::LabelPrintError);
                }
            break;

            case 28://wait to Discharge
                myBoard.GPIOResetAll();
                myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
                myTestDevice.setRelay(USV_Test_Interface::Relays::Load,true);
                myTestDevice.setRelay(USV_Test_Interface::Relays::VCCLoad,true);
                myTestDevice.setRelay(USV_Test_Interface::Relays::VCCLoad,true);            
                // use another GPIO & resistor on 3.3V to discharge the board
                showLog("Wait to DisCharge...");
                _mState++;
            break;
            case 29://Under Contrtuction
                //myTempVal.VCC = myTestDevice.getDUT_VCC();
                __floatTmpVale=myTestDevice.getDUT_VCC();
                myInterActReg.TR.Vvcc=__floatTmpVale;//myTempVal.VCC;
                
                //showLog((std::ostringstream{}<<"wait to Discharge... "
                //    <<"OUTPUT:" << std::fixed << std::setprecision(1) << myTestDevice.getDUT_VOUT()
                //    << "V " << static_cast<int32_t>(myTestDevice.getDUT_VOUTAmp() * 1000) << "mA"
                //    "DUT-VCC: " << std::fixed << std::setprecision(1) << __floatTmpVale << "V  ").str());
                
                if(__floatTmpVale<.5) // if there is no 3.3V can go to next steps...
                    _mState++;
                break;
            case 30:
                myTestDevice.setRelay(USV_Test_Interface::Relays::VCCLoad,true);
                myTestDevice.setRelay(USV_Test_Interface::Relays::VCCLoad,true);            
                myTestDevice.cleanLCD();
                _mState++;
            break;
            default: 
                showLog("\nEND Wait to press Key.\n");        
                _mState=0xFE;
                break;
        }
        
        
        
        myTempVal.result=_mState;        
        //if(myTestDevice.readKeys()==USV_Test_Interface::_KEY::K3) _mState=0xF1;
        _key=myInterActReg.getGuiCMD();
        if(_key==USV_Test_Interface::_KEY::K3) _mState=0xF1;
        
        if(_mState>11){
            myTempVal.VCap=myBoard.GetVCap(0);        
            myInterActReg.TR.Vcap1=myBoard.GetVCap(1);
            myInterActReg.TR.Vcap2=myBoard.GetVCap(2);
            myInterActReg.TR.Vcap3=myBoard.GetVCap(3);
            myInterActReg.TR.Vcap4=myBoard.GetVCap(4);
        }
        myInterActReg.TR.InCurrent=myTempVal.InCurrent;
        myInterActReg.TR.LoadCurrent=myTempVal.LoadCurrent;
        myInterActReg.TR.VOut=myTempVal.VOut;
        myInterActReg.TR.Vin=myTempVal.VIn;
        myInterActReg.TR.VcapBank=myTempVal.VCap;
        myInterActReg.TR.Vvcc=myTempVal.VCC;
        myInterActReg.TR.CapChargeTime=myTempVal.chargeTime;
        myInterActReg.TR.CapDisChargeTime=myTempVal.DisChargeTime;
        myInterActReg.TR.OutSWOffTime=myTempVal.OutSWOffTime;    
    }
        
    showLog("SAVE DATA...");
    //SaveResult(true,myArg.StoreFolderPath+myArg.FileName_EUI,myArg.StoreFolderPath+myArg.FileName_Test);    
    SaveEUI(myArg.StoreFolderPath+myArg.FileName_EUI,(testr.ErrorNo == 0) ? true : false);
    //SaveResult(false,myArg.StoreFolderPath+myArg.FileName_EUI,myArg.StoreFolderPath+myArg.FileName_Test);//testingNoCap);
    SaveResult(myArg.StoreFolderPath+myArg.FileName_Test);
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
        
}

void USV_TEST_UTIL_V2::runICA2506(void){
    uint8_t _mState = 0, LMState=0xFF;
    uint16_t _dcnt100ms=0;

    showLog("******************************");
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);            
    DongleCheck();  
    ShowMyDongle();
    while((_mState<0xF0) && (xrunning==true)){        
        usleep(100000);
        myTempVal.VIn= myTestDevice.getDUT_VIN();
        myTempVal.InCurrent = myTestDevice.getDUT_VINAmp();        
        switch (_mState){ 
            case 0://clear registers & reset Relays *********************************************
                myInterActReg.TR.DataClear();
                removeJPG_PFiles_Jobs();
                myTempVal.clear();
                myBoard.myEEPROM.clear_EEPROM_Buffer();
                myBoard.myEEPROM.myData.clear();
                testr.clear();
                testrReset();                
                showLog("ICA2506 ReadEUI");
                showLog("Start: Reset Relays and Test start.");
                myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);                
            _mState++;
            break;           
            default: 
                showLog("\n!!! END !!!\n");
                _mState=0xFE;
                break;
        
        }        
        myTempVal.result=_mState;        
        myInterActReg.TR.InCurrent=myTempVal.InCurrent;
        myInterActReg.TR.Vin=myTempVal.VIn;
        myInterActReg.TR.Vvcc=myTempVal.VCC;                
    }
    
    showLog("SAVE DATA...");
    SaveEUI(myArg.StoreFolderPath+myArg.FileName_EUI,true);
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);

}
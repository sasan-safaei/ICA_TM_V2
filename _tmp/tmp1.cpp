
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
                            myTestResult.EEPROM_Status=myBoard.myEEPROM.CheckDataVersionProcess(false);                            
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
                    myBoard.CheckCapsVoltage(&myTestResult.Vcap_Max);
                    __error_cnt=0;
                }
                else{

                    printf("\n failed !!!Current read Error!!! (Value:%.2f)\n",myTempVal.InCurrent);
                    showLog((std::ostringstream{} <<" failed !!!Current read Error!!! (Value:"<< std::fixed << std::setprecision(2)<< myTempVal.InCurrent<<")").str());
                    if (__error_cnt++>3) _mState=showError(ERROR::ChargeDuration);
                }                
                if ((myTempVal.chargeTime > myTestResult.Limit_MAX_Charge_time) & (myTempVal.InCurrent> myTestResult.Limit_MIN_ChargeCurrent)) {
                    showLog((std::ostringstream{} << "\nTEST5.Error!!!  Time (" 
                        << myTempVal.chargeTime << " > " << myTestResult.Limit_MAX_Charge_time 
                        << ") Current (" 
                        << myTempVal.InCurrent << " > " << myTestResult.Limit_MIN_ChargeCurrent 
                        << ")\n").str());
                }
                if (myTempVal.chargeTime > myTestResult.Limit_MAX_Charge_time){//+__Limit_MAX_ExtendChargeTime){                    
                    showLog((std::ostringstream{}<< "\nTEST5.Error!!!  Time ("
                        << myTempVal.chargeTime << " > "
                        << (myTestResult.Limit_MAX_Charge_time)// + __Limit_MAX_ExtendChargeTime)
                        << ")\n" ).str());
                    _mState=showError(ERROR::ChargeDuration);
                }
                if(_mState<0xF0 && myTempVal.InCurrent < myTestResult.Limit_MIN_FullChargeCurrent && myTempVal.InCurrent > 0){
                    myTestResult.time_charge=myTempVal.chargeTime;
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
                    myTestResult.tempIC=__tempICtempVal;
                    __tempIC__error__cnt=0;}

                if(!myBoard.GetBatBankTemp(&myTestResult.tempBatBank,true) || (myTestResult.tempBatBank>__Limit_MAX_BatBank_Temp)) 
                {
                    if(__tempBatBack__error__cnt++>5) _mState=showError(ERROR::TempSensor_CapBank);//testr.ErrorNo=ERROR::TempSensor;                
                }
                        file << myTempVal.DisChargeTime<<","<< std::fixed
                    <<std::setprecision(1)<<myTempVal.VCap << std::endl;
                //myTestResult.ErrorNo=0;
                usleep(100000);
                if(myTempVal.VCap>0) myTestResult.VCap_SWOff=myTempVal.VCap;

                if ( myTempVal.LoadCurrent>-1 && myTempVal.LoadCurrent<0.05 ) {
                    showLog("\n");
                    myTestResult.time_DisCharge=myTempVal.DisChargeTime;
                    if(myTestResult.VCap_SWOff< myTestResult.Limit_MIN_VCap_ShutdownVoltage || myTestResult.VCap_SWOff> myTestResult.Limit_MAX_VCap_ShutdownVoltage){
                        showLog((std::ostringstream{} << "Vcap on Shut Down is:" << std::fixed << std::setprecision(2) << myTestResult.VCap_SWOff << "V").str());                            
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
                if (myTestResult.EEPROM_Status==EEPROMProcesSts::Ok){
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
                //if(myArg.EEPROMCFG) myTestResult.VCap_SWOff=0;
                myBoard.myEEPROM.updateBoardInfo(myBoard.boardName,myBoard.boardVer,myBoard.myBoardInfo,myTestResult);
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
    SaveEUI(myArg.StoreFolderPath+myArg.FileName_EUI,(myTestResult.ErrorNo == 0) ? true : false);
    //SaveResult(false,myArg.StoreFolderPath+myArg.FileName_EUI,myArg.StoreFolderPath+myArg.FileName_Test);//testingNoCap);
    SaveResult(myArg.StoreFolderPath+myArg.FileName_Test);
    myTestDevice.setRelay(USV_Test_Interface::Relays::All,false);
        
}



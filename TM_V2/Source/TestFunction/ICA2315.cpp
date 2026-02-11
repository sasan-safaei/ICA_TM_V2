#include "ICA2315.h"
#include <unistd.h>
#include <math.h>
#include <stdint.h>

#include <iostream>
#include <chrono>

//float LTC3350_RSNSI1;
//float LTC3350_RSNSI2;
//float LTC3350_RSNSC;
//float LTC3350_RTST;
//float LTC3350_RT;

//float Board_SupperCapVoltage;
//int Board_SupperCapSingleCap;
//int   Board_SupperCapNum;

//boardInfo LTC3350var;//is a copy of myBoardjust to use in LTC3350_formats 

void ICA2315::LineClear(const char *message){
  printf("\n\r");
  printf("\033[2K");
  printf("\r");
  printf("%s", message);
}

bool ICA2315::RemessCapESR(void){
  IC_LTC3350_write_register(LTC3350_CTL_REG,0x01);
  return true;
}
bool ICA2315::VShuntUp(void){
  if (VShuntValue<VCellMax)
  {
    VShuntValue+=VCellStep;
    if (VShuntValue>VCellMax) VShuntValue=VCellMax;
    IC_LTC3350_write_register(LTC3350_VSHUNT,VShuntValue);//0 ~ 15
    return true;
  }
  return false;
}
bool ICA2315::VShuntDown(void){
  if (VShuntValue>VCellMin)
  {
    VShuntValue-=VCellStep;
    if (VShuntValue<VCellMin) VShuntValue=VCellMin;
    IC_LTC3350_write_register(LTC3350_VSHUNT,VShuntValue);//0 ~ 15
    return true;
  }
  return false;
}
bool ICA2315::CapRefUp(void){
  if (capRafValue<15)
  {
    capRafValue++;
    IC_LTC3350_write_register(LTC3350_VCAPFB_DAC,capRafValue);//0 ~ 15
    return true;
  }
  return false;
}
bool ICA2315::CapRefDown(void){
if (capRafValue>0)
  {
    capRafValue--;
    IC_LTC3350_write_register(LTC3350_VCAPFB_DAC,capRafValue);//0 ~ 15
    return true;
  }
  return false;
}
    
bool ICA2315::GPIO_set(uint16_t GPO_No){
  unsigned char rxBuff[5]={0,0,0,0,0};
  if (GPO_No>=8) return false;
  SP_Write({'R',4,'P'});
  if(serialPort.sp_read(rxBuff, 1)){
    unsigned char data=rxBuff[0];
    SP_Write({'O',(unsigned char)(data | 1<<GPO_No),'P'});
    //if (SP_Write({'w',2,(unsigned char)(data | 1<<GPO_No),'P'}))
    //  return true;
  }
  return false;
}
bool ICA2315::GPIO_clr(uint16_t GPO_No){
  unsigned char rxBuff[5]={0,0,0,0,0};
  if (GPO_No>=8) return false;
  
  SP_Write({'R',4,'P'});
  if(serialPort.sp_read(rxBuff, 1)){
    unsigned char data=rxBuff[0];
    SP_Write({'O',(unsigned char)(data & ~(1<<GPO_No)) ,'P'});
    //if (SP_Write({'w',2,(unsigned char)(data & ~(1<<GPO_No)),'P'}))
    //  return true; 
  }   
  return false;
}
bool ICA2315::GPIO_get(uint16_t GPO_No){
  unsigned char rxBuff[5]={0,0,0,0,0};
  if (GPO_No>=8) return false;
  SP_Write({'R',4,'P'});
  if(serialPort.sp_read(rxBuff, 1)){
    return (bool)((rxBuff[0]>>GPO_No)&0x01);    
  }
  return false;
}
unsigned char ICA2315::GPIO_read(){
  unsigned char rxBuff[5]={0,0,0,0,0};
  SP_Write({'I','P'});
  serialRx_delay();
  if(serialPort.sp_read(rxBuff, 1)){
    return rxBuff[0];    
  }
  return false;
}
bool ICA2315::SP_Write(std::initializer_list<unsigned char> arr){
  const int size = arr.size();
  unsigned char txBuff[size];
  std::copy(arr.begin(), arr.end(), txBuff);
  return serialPort.sp_write(txBuff, size);
}



bool ICA2315::IC_LTC3350_read_register(uint16_t registerinfo, uint16_t *data)
{
  
  int result;
  uint8_t command_code = get_subaddr(registerinfo);
  uint8_t offset = get_offset(registerinfo);
  uint16_t mask = get_mask(registerinfo);
  uint16_t buff;
  unsigned char rxBuff[5]={0,0,0,0,0};
  uint8_t i2c_add=I2C_LT3350_ADD<<1;
  if(SP_Write({'S', (unsigned char)(i2c_add), 1, command_code,'P', 'S', (unsigned char)(i2c_add+1), 2,'P'}))
  {  
    serialRx_delay();
    if(serialPort.sp_read(rxBuff, 2))
    {
      
      buff=(uint16_t)rxBuff[1]<<8;
      buff+=(uint16_t)rxBuff[0]&0xFF;

      //printf("Read:  %X %X  %X \n",rxBuff[0],rxBuff[1],buff);
      //*fullData=buff;
      *data= buff;
      *data &= mask;
      *data = *data >> offset;
      
      return true;
    }
  }
  return false;
}
bool ICA2315::IC_LTC3350_write_register( uint16_t registerinfo, uint16_t data)
{
  uint16_t read_data;

  if (IC_LTC3350_read_register(registerinfo, &read_data))
  {
    uint8_t command_code = get_subaddr(registerinfo);
    if (get_size(registerinfo) != 16)
    {
      uint8_t offset = get_offset(registerinfo);
      uint16_t mask = get_mask(registerinfo);
      data = (read_data & ~mask) | (data << offset);
    }
    if(SP_Write({'S', (unsigned char)(I2C_LT3350_ADD<<1+0), 3, command_code, (unsigned char)(data&0xFF), (unsigned char)((data>>8)&0xFF), 'P'}))
      return true;
    
  }
  return false; 
}



uint16_t ICA2315::IC_LTC3350_GetValue(uint16_t registerinfo, uint16_t data){
      uint8_t offset = get_offset(registerinfo);
      uint16_t mask = get_mask(registerinfo);
      return ((data & mask) >>offset);
}

bool ICA2315::init(char* ttyName){
    uint16_t __data;
    serialPort.sp_close();
    serialPort.sp_init(ttyName, B9600);
    if (serialPort.sp_open()) {
      {
        GPIOResetAll();
        SP_Write({'W',0x02,0x00,0x03,0xFC,'P'});//set GPIO7 GPIO6 GPIO5 to Open-Colector-Output
        
        //IC_LTC3350_write_register(LTC3350_MSK_GPI_UV, 0x1);
        //IC_LTC3350_read_register(LTC3350_MSK_GPI_UV, &__data);
        //IC_LTC3350_write_register(LTC3350_VOUT_OV_LVL_CMD, 0x1234);
        //IC_LTC3350_read_register(LTC3350_VOUT_OV_LVL_CMD, &__data);            
        return true;
      }      
    }
    return false;
}
float write_value_and_average(float value, float arr[], int *index, int *count, int size) {
    // Add the new value at the current index position
    arr[*index] = value;

    // Increment the index and wrap around if necessary (circular buffer)
    *index = (*index + 1) % size;

    // Keep track of how many elements have been added, up to the size of the array
    if (*count < size) {
        (*count)++;
    }

    // Calculate the sum of the array
    float sum = 0;
    for (int i = 0; i < *count; i++) {
        sum += arr[i];
    }

    // Return the average
    return (float)sum / *count;
}

bool ICA2315::SC18IM_I2C_Write_EEPROMBlock(uint8_t ic_i2c_add, uint8_t reg_add, uint8_t *data){
  bool status=false;
  uint8_t i2c_add=ic_i2c_add<<1;
  unsigned char txBuff[50];
  txBuff[0]='S';
  txBuff[1]=(unsigned char)(i2c_add);
  txBuff[2]=9;
  txBuff[3]=(unsigned char)(reg_add);
  for(int i=0;i<8;i++)
    txBuff[4+i]=*data++;
  
  txBuff[12]='P';
  status=serialPort.sp_write(txBuff, 13);
  serialRx_delay(50000);
  return status;
}
bool ICA2315::SC18IM_I2C_Read_EEPROMBlock(uint8_t ic_i2c_add, uint8_t reg_add, uint8_t *data)
{
  int result;
  int __len=8;
  //uint16_t buff;
  unsigned char rxBuff[10]={0,0,0,0,0,0,0,0,0,0};
  uint8_t i2c_add=ic_i2c_add<<1;
  //serialPort.clearInputBuffer();
  if(SP_Write({'S', (unsigned char)(i2c_add), 1, (unsigned char)(reg_add), 'S', (unsigned char)(i2c_add+1), (unsigned char)(__len),'P'}))
  {  
    serialRx_delay(50000);
    
    if(serialPort.sp_read(rxBuff, __len))
    {
      for(int __i=0;__i<__len;__i++)
      data[__i]=rxBuff[__i];      
      return true;
    }
  }
  return false;
}
bool ICA2315::readEUI64(uint8_t *eui64)
{
    return SC18IM_I2C_Read_EEPROMBlock(EEPROM_I2C_ADDRESS,EUI64_ADDRESS,eui64);
}
bool ICA2315::readDataBuff(uint8_t *buff){
  for(int i=0;i<EEPROMDateBufferLen;i++){
    if(SC18IM_I2C_Read_EEPROMBlock(EEPROM_I2C_ADDRESS,EEPROMDateBufferAdd+(i*8),buff))
      buff+=8;
    else
      return false;
  }
  return true;  
}
bool ICA2315::writeDataBuff(uint8_t *buff){
  if (!WriteEEPROMBlock(1,buff)) { printf("Error on EEPROM-Write! Block0");return false;}
  if (!WriteEEPROMBlock(2,buff+8)) { printf("Error on EEPROM-Write! Block1");return false;}
  if (!WriteEEPROMBlock(3,buff+16)) { printf("Error on EEPROM-Write! Block2");return false;}
  if (!WriteEEPROMBlock(4,buff+24)) { printf("Error on EEPROM-Write! Block3");return false;}
  return true;
}

void ICA2315::ReadEEPROMBlock(uint8_t block_add, uint8_t *data){	
	uint8_t reg_add=(block_add-1)*0x08;
	SC18IM_I2C_Read_EEPROMBlock(EEPROM_I2C_ADDRESS,reg_add,data);
	printf("Block%d: %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X\n",block_add,data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);	
}
bool ICA2315::WriteEEPROMBlock(uint8_t block_add, uint8_t *data){
  unsigned char EEPROM_BX_Value[8]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	unsigned char readBlock[8];
	uint8_t __cnt=0;
	uint8_t reg_add=(block_add-1)*0x08;
	bool __status=false;
	while(__status==false && __cnt++<2){
		__status=true;
		SC18IM_I2C_Write_EEPROMBlock(EEPROM_I2C_ADDRESS,reg_add,EEPROM_BX_Value);
		SC18IM_I2C_Write_EEPROMBlock(EEPROM_I2C_ADDRESS,reg_add,data);

    ReadEEPROMBlock(block_add,readBlock);
		for(int i=0;i<8;i++)
			if(readBlock[i]!=data[i])
				__status=false;
	}		
	return __status;	
}

#define avr_max_count 3
void ICA2315::GetVINOUT(float* Vin,float* Vout){
  uint16_t data;
  *Vin=-1;
  *Vout=-1;
  if(IC_LTC3350_read_register(LTC3350_MEAS_VIN,&data)) *Vin= LTC3350_VIN(data);
  if(IC_LTC3350_read_register(LTC3350_MEAS_VOUT,&data)) *Vout=LTC3350_VOUT(data);
  
}
float ICA2315::GetLoadCurrent(){
  uint16_t data;
  float iVal=-1;
  if(IC_LTC3350_read_register(LTC3350_MEAS_IIN,&data))
    iVal=__LTC3350_IIN(data);            
  return iVal;
  
}
float ICA2315::GetVCap(uint8_t _VCapNo){
  uint16_t data;
  switch(_VCapNo){
    case 0: if(IC_LTC3350_read_register(LTC3350_MEAS_VCAP,&data)) return LTC3350_VCAP(data); break;
    case 1: if(IC_LTC3350_read_register(LTC3350_MEAS_VCAP1,&data)) return LTC3350_CELL(data);break;
    case 2: if(IC_LTC3350_read_register(LTC3350_MEAS_VCAP2,&data)) return LTC3350_CELL(data);break;
    case 3: if(IC_LTC3350_read_register(LTC3350_MEAS_VCAP3,&data)) return LTC3350_CELL(data);break; 
    case 4: if(IC_LTC3350_read_register(LTC3350_MEAS_VCAP4,&data)) return LTC3350_CELL(data);break;
  }
  return -1;
}
bool ICA2315::GetICTemp(float* tempVal,bool ShowVal){
	uint16_t data;
	*tempVal =-273;
	if(IC_LTC3350_read_register(LTC3350_MEAS_DTEMP,&data))
	{
		*tempVal=LTC3350_DTEMP(data);		
		if (ShowVal) printf("IC:%.1f°c ",*tempVal);
	}	
	else return false;
	return true;
}
bool ICA2315::GetBatBankTemp(float* tempVal,bool ShowVal){
	uint16_t data;
	*tempVal =-273;
	if(IC_LTC3350_read_register(LTC3350_MEAS_GPI,&data)){
    double _Resister=10000;
    double _VCC=5.0;
    double _V_t=data*0.0001835;
    double _R_t=_Resister*(_V_t/(_VCC-_V_t));
    double _t_c=(1/((1/298.15)+(log(_R_t/_Resister)/4500)))-273.15;
	  *tempVal=(float)_t_c;
    if (ShowVal) printf("Bat-Bank:%.1f°c ",*tempVal);
  	}
	else return false;
	return true;
}

float ICA2315::getInputVoltage(){
  uint16_t data;
  if(IC_LTC3350_read_register(LTC3350_MEAS_VIN,&data))
    return  LTC3350_VOUT(data);
  return 0;
}
bool ICA2315::showValue(){
  static std::chrono::high_resolution_clock::time_point startTimeESR=std::chrono::high_resolution_clock::now();
  static std::chrono::high_resolution_clock::time_point currentTimeESR=std::chrono::high_resolution_clock::now();
  
  uint16_t data,data2;
  static uint16_t cnt1=0;
  float tempVal;
  static float IloadAvr[avr_max_count] = {0};
  static float IChargeAvr[avr_max_count] = {0};
  static int avr_index = 0;
  static int avr_count = 0;
  int avr_size = avr_max_count;
  uint16_t tempValue;
  //SP_Write({'R',0x01,0x02,0x03,0x04,'P'});
  //printf("GPIO: %x \n",GPIO_read());
  if (cnt1++==5){
    IC_LTC3350_write_register(LTC3350_CAP_ESR_PER,uint16_t(60));//run ESR/CAP meassure each 300 second
    IC_LTC3350_write_register(LTC3350_CTL_REG,uint16_t(0x1));
    //IC_LTC3350_write_register(LTC3350_CTL_REG,9);
    IC_LTC3350_write_register(LTC3350_VSHUNT,0x3999);//2.7V
    //IC_LTC3350_write_register(LTC3350_VSHUNT,15800);//2.9V
    IC_LTC3350_write_register(LTC3350_VCAPFB_DAC,15);//0 ~ 15
    IC_LTC3350_write_register(LTC3350_MSK_ALARMS,0x0000);
    IC_LTC3350_write_register(LTC3350_CLR_ALARMS,0xFFFF);
    IC_LTC3350_write_register(LTC3350_MSK_MON_STATUS,0x037F);
  
    IC_LTC3350_write_register(LTC3350_VIN_OV_LVL,uint16_t(22/(2.21e-3)));//22V
    IC_LTC3350_write_register(LTC3350_VIN_UV_LVL,uint16_t(22/(2.21e-3)));

    IC_LTC3350_write_register(LTC3350_VOUT_OV_LVL,uint16_t(22/(2.21e-3)));
    IC_LTC3350_write_register(LTC3350_VOUT_UV_LVL,uint16_t(22/(2.21e-3)));
  }
  
  
  
  float __Vin=0, __Vout=0, __Vcap=0 ,__Iin=0 ,__Icap=0 ,__Iload=0, __VinMess=0;
  bool BackupMode=false;
  LineClear("1. LTC3350 Voltages:");
  LineClear("    In: "); if(IC_LTC3350_read_register(LTC3350_MEAS_VIN,&data)) { __Vin= LTC3350_VIN(data);printf("%.2fv     ",__Vin);}
      printf("Out: ");if(IC_LTC3350_read_register(LTC3350_MEAS_VOUT,&data)) __Vout=LTC3350_VOUT(data); printf("%.2fv",__Vout);
  LineClear("    Cap: "); if(IC_LTC3350_read_register(LTC3350_MEAS_VCAP,&data)) { __Vcap=LTC3350_VCAP(data); printf("%.2fv  [",__Vcap);}
                        if(IC_LTC3350_read_register(LTC3350_MEAS_VCAP1,&data)) printf("%.2fv, ",LTC3350_CELL(data));
                        if(IC_LTC3350_read_register(LTC3350_MEAS_VCAP2,&data)) printf("%.2fv, ",LTC3350_CELL(data)); 
                        if(IC_LTC3350_read_register(LTC3350_MEAS_VCAP3,&data)) printf("%.2fv, ",LTC3350_CELL(data)); 
                        if(IC_LTC3350_read_register(LTC3350_MEAS_VCAP4,&data)) printf("%.2fv ]",LTC3350_CELL(data));
  LineClear("");
  LineClear("2. LTC3350 Currents:");
  LineClear("    Caps Bank: ");
  uint16_t dataIchg=0,dataIIn=0;
  if(IC_LTC3350_read_register(LTC3350_MEAS_ICHG,&dataIchg)) 
  {
    if(IC_LTC3350_read_register(LTC3350_MEAS_IIN,&dataIIn)){
      __Iload=__LTC3350_IIN(dataIIn);

      if(dataIchg>=0x8000)  BackupMode=true;
      std::string strStatus="Normal";
      if(BackupMode){
        __Icap=__LTC3350_ICHARGE((uint16_t)((int32_t)(0-dataIchg)&0xFFFF)); 
        __Iload=__Vcap*__Icap/24;
        __Icap*=-1;
        strStatus="BackUp";
        tempValue=0;
      }else{
        __Icap=__LTC3350_ICHARGE(dataIchg); 
        tempValue=dataIIn;        
        tempValue-=(uint16_t)((float)dataIchg*((myBoardInfo.LTC3350_RSNSI1+myBoardInfo.LTC3350_RSNSI2)/myBoardInfo.LTC3350_RSNSC)*__Vcap/__Vin);
        if(tempValue>=0x8000){
          tempValue*=-1;
          __Iload=-1*__LTC3350_IIN(tempValue);
        }
        else{
            __Iload=__LTC3350_IIN(tempValue);        
        }

      } 
      printf("(Load/Cap): %.2fA / %.2fA (%s) Cap: %.2fA(%.1fV)   ", __Iload,__Icap*__Vcap/24,strStatus.c_str(),__Icap,__Vcap);
      //printf(" ***TEST:[Ichg.Reg:%d IIn.Reg:%d tmpVal:%d]    ", dataIchg,dataIIn,tempValue); 
    }     
  }
  LineClear("");
  LineClear("3. LTC3350 temperature:");
  LineClear("    "); 
  GetICTemp(&tempVal,true);
  GetBatBankTemp(&tempVal,true);
  /*
  if(IC_LTC3350_read_register(LTC3350_MEAS_DTEMP,&data)) printf("IC: %.1f°c",LTC3350_DTEMP(data));
  if(IC_LTC3350_read_register(LTC3350_MEAS_GPI,&data)){
    double _Resister=10000;
    double _VCC=5.0;
    double _V_t=data*0.0001835;
    double _R_t=_Resister*(_V_t/(_VCC-_V_t));
    double _t_c=(1/((1/298.15)+(log(_R_t/_Resister)/4500)))-273.15;
    printf("Bat Bank: %.1f°c ",_t_c); //[GPI-V: %.3fV    R:%.1fKOhm]",_t_c,data*0.0001835,_R_t/1000); 
  }*/

  LineClear("");
  LineClear("4. Cap Control:");
  LineClear("    VCapfb_dac: ");if(IC_LTC3350_read_register(LTC3350_VCAPFB_DAC,&data)) printf("(%X) %.2f  * Use U/D key to change Cap Voltage",data,LTC3350_VCAPFB_DAC_FORMAT(data));
  LineClear("    VShunt: ");if(IC_LTC3350_read_register(LTC3350_VSHUNT,&data)) {printf("(%X) %.2f  * Use P/M key to change Voltage (0.1v) ",data,LTC3350_CELL(data));VShuntValue=data;}
  //LineClear(" VShunt: ");if(IC_LTC3350_read_register(LTC3350_VSHUNT,&data)) printf("(%X)   %.2f",data,LTC3350_CELL(data));
  LineClear("");
  LineClear("5. CAP/ESR mess: ");
  IC_LTC3350_read_register(LTC3350_CAP_ESR_PER,&data);
  printf(" * each %d sec ",data *10);
  IC_LTC3350_read_register(LTC3350_MON_STATUS,&data);
  
  if(IC_LTC3350_GetValue(LTC3350_MON_CAPESR_PENDING,data)) printf(" [Pending!]");
  if(IC_LTC3350_GetValue(LTC3350_MON_CAPESR_ACTIVE,data)) {
    currentTimeESR = std::chrono::high_resolution_clock::now();
    auto __testDuration = std::chrono::duration_cast<std::chrono::seconds>(currentTimeESR - startTimeESR);  
    printf(" [in Progress!  %dsec] ",(int)__testDuration.count());
  }else{
    startTimeESR=std::chrono::high_resolution_clock::now();
  }
  
  printf ("Use R key to ReMessCapESR                    ");

  LineClear("    Cap: ");if(IC_LTC3350_read_register(LTC3350_MEAS_CAP,&data)) printf("(%d) %.2fF",data,__LTC3350_CAP(data)); else printf(" ---  ");
  if(IC_LTC3350_read_register(LTC3350_MEAS_ESR,&data)) printf("     ESR:%.0f mOhm",__LTC3350_ESR(data)*1000);
  LineClear("");
  //LineClear(" __ Control Reg ____________________________________________ !!!");
  //LineClear(" CTL_REG: ");if(IC_LTC3350_read_register(LTC3350_CTL_REG,&data)) printf("(%x)",data);
  //LineClear("");



  LineClear("6. Input Voltage Alarm: ");  
  if(IC_LTC3350_read_register(LTC3350_ALARM_REG,&data))
    if (IC_LTC3350_GetValue(LTC3350_ALARM_VIN_UV,data))
      printf("!!! No input Voltage !!!");
    else
      printf("Normal.");
  else
    printf("!!! unKnown !!!");
  //LineClear(" Alarm_reg: ");if(IC_LTC3350_read_register(LTC3350_ALARM_REG,&data)) printf("%X",data);
  //LineClear(" CAP: ");printf("UV %d OV %d                  GPI: UV %d OV %d",IC_LTC3350_GetValue(LTC3350_ALARM_CAP_UV,data),IC_LTC3350_GetValue(LTC3350_ALARM_CAP_OV,data),
  //                                                                IC_LTC3350_GetValue(LTC3350_ALARM_GPI_UV,data),IC_LTC3350_GetValue(LTC3350_ALARM_GPI_OV,data));
  //LineClear(" Vin: ");printf("UV %d OV %d                   Vout: UV %d OV %d",IC_LTC3350_GetValue(LTC3350_ALARM_VIN_UV,data),IC_LTC3350_GetValue(LTC3350_ALARM_VIN_OV,data),
  //                                                                IC_LTC3350_GetValue(LTC3350_ALARM_VOUT_UV,data),IC_LTC3350_GetValue(LTC3350_ALARM_VOUT_OV,data));
  //LineClear(" VCAP: ");printf("UV %d OV %d",IC_LTC3350_GetValue(LTC3350_ALARM_VCAP_UV,data),IC_LTC3350_GetValue(LTC3350_ALARM_VCAP_OV,data));
  //LineClear(" iin: ");printf("OC %d  input over Current",IC_LTC3350_GetValue(LTC3350_ALARM_IIN_OC,data));
  //LineClear(" ichg: ");printf("UC %d charge under Current",IC_LTC3350_GetValue(LTC3350_ALARM_ICHG_UC,data));
  //LineClear(" ESR: ");printf("Hi %d  CAP: Lo %d",IC_LTC3350_GetValue(LTC3350_ALARM_ESR_HI,data),IC_LTC3350_GetValue(LTC3350_ALARM_CAP_LO,data));
  IC_LTC3350_write_register(LTC3350_CLR_ALARMS,0xFFFF);  
  LineClear("");
  return true;
}
bool ICA2315::showValue_Rest(){
  static std::chrono::high_resolution_clock::time_point startTime, currentTime;
  static int __lmessProgress=0;
  LineClear("___ All Reg (test)_____________________________________ !!!");
  LineClear(" ");
  u_int16_t testData[50];
  std::string regName[0x2B]={
     "00.clr_alarms ","01.msk_alarms ","02.msk_mon_sts","03.------------"
    ,"04.cap_esr_per","05.vcapfb_dac ","06.vshunt     ","07.cap_uv_lvl  "
    ,"08.cap_ov_lvl ","09.gpi_uv_lvl ","0A.gpi_ov_lvl ","0B.vin_uv_lvl  "
    ,"0C.vin_ov_lvl ","0D.vcap_uv_lvl","0E.vcap_ov_lvl","0F.vout_uv_lvl "
    ,"10.vout_ov_lvl","11.iin_oc_lvl ","12.ichg_uc_lvl","13.dtemp_cold_l"
    ,"14.dtemp_hot_l","15.esr_hi_lvl ","16.cap_lo_lvl ","17.ctl_reg     "
    ,"18.-----------","19.-----------","1A.num_caps   ","1B.chrg_status "
    ,"1C.mon_status ","1D.alarm_reg  ","1E.meas_cap   ","1F.meas_esr    "
    ,"20.meas_vcap1 ","21.meas_vcap2 ","22.meas_vcap3 ","23.meas_vcap4  "
    ,"24.meas_gpi   ","25.meas_vin   ","26.meas_vcap  ","27.meas_vout   "
    ,"28.meas_iin   ","29.meas_ichg  ","2A.meas_dtemp "
  };
  uint16_t rNo=0;
  for(int i=0;i<=0x2A;i++){
    testData[i]=0;
    IC_LTC3350_read_register(0x0F00+i,&testData[i]);    
  }

  for(int i=0;i<11;i++){
    LineClear(" ");
    for(int j=0;j<4;j++){
      rNo=i*4+j;
      if(rNo<0x2B){
        if(rNo==0x03 || rNo==0x18 ||rNo==0x19 )
          printf("%s: ---- ",regName[rNo].c_str());
        else
          printf("%s: %04X ",regName[rNo].c_str(),testData[rNo]);
        switch (rNo)
        {
          case 0x00: printf("**CLS");break;  
          case 0x01: printf("**MASK");break;  
          case 0x02: printf("**MASK");break;  
          
          case 0x04: printf("(%dsec)",testData[rNo]*10);break;  
          case 0x05: printf("(%.1fV)",(float)testData[rNo]*0.0375+.6375);break;  
          case 0x06: printf("(%.1fV)",(float)testData[rNo]*0.0001835);break;  
          case 0x07: printf("(%.1fV)",(float)testData[rNo]*0.0001835);break;  
          case 0x08: printf("(%.1fV)",(float)testData[rNo]*0.0001835);break;  
          case 0x09: printf("(%.1fV)",(float)testData[rNo]*0.0001835);break;  
          case 0x0A: printf("(%.1fV)",(float)testData[rNo]*0.0001835);break;  
          

          case 0x0B: printf("(%.1fV)",(float)testData[rNo]*0.00221);break;  
          case 0x0C: printf("(%.1fV)",(float)testData[rNo]*0.00221);break;  
          case 0x0D: printf("(%.1fV)",(float)testData[rNo]*0.001476);break;  
          case 0x0E: printf("(%.1fV)",(float)testData[rNo]*0.001476);break;  
          case 0x0F: printf("(%.1fV)",(float)testData[rNo]*0.00221);break;  
          case 0x10: printf("(%.1fV)",(float)testData[rNo]*0.00221);break;  
          case 0x11: printf("(%.1fA)",(float)testData[rNo]*0.000001983/(myBoardInfo.LTC3350_RSNSI1+myBoardInfo.LTC3350_RSNSI2));break;  
          case 0x12: printf("(%.1fA)",(float)testData[rNo]*0.000001983/myBoardInfo.LTC3350_RSNSC);break;  
          case 0x13: printf("(%.1f°)",(float)testData[rNo]*0.028-251.4);break;  
          case 0x14: printf("(%.1f°)",(float)testData[rNo]*0.028-251.4);break;  
          case 0x15: printf("(%.1fV)",(float)testData[rNo]*myBoardInfo.LTC3350_RSNSC/64);break;  
          case 0x16: printf("(%.1fV)",(float)testData[rNo]*0.000336*myBoardInfo.LTC3350_RT/myBoardInfo.LTC3350_RTST);break;  
          

          case 0x17: printf("**CTL ");break;            
          case 0x1A: printf("(%d)",testData[rNo]+1);break;
          case 0x1B: printf("**STATUS");break;  
          case 0x1C: printf("**STATUS");break;  
          case 0x1D: printf("**ALARM");break;  
          
          case 0x1E: printf("(%.1fF)",(float)testData[rNo]*0.000336*myBoardInfo.LTC3350_RT/myBoardInfo.LTC3350_RTST);break;  
          case 0x1F: printf("(%.1fR)",(float)testData[rNo]*myBoardInfo.LTC3350_RSNSC/64);break;  
          
          case 0x20: printf("(%.1fV)",(float)testData[rNo]*0.0001835);break;  
          case 0x21: printf("(%.1fV)",(float)testData[rNo]*0.0001835);break;  
          case 0x22: printf("(%.1fV)",(float)testData[rNo]*0.0001835);break;  
          case 0x23: printf("(%.1fV)",(float)testData[rNo]*0.0001835);break;  
          case 0x24: printf("(%.1fV)",(float)testData[rNo]*0.0001835);break;  
          case 0x25: printf("(%.1fV)",(float)testData[rNo]*0.00221);break;  
          
          case 0x26: printf("(%.1fV)",(float)testData[rNo]*0.001476);break;  
          case 0x27: printf("(%.1fV)",(float)testData[rNo]*.00221);break;  
          case 0x28: printf("(%.1fA)",(float)testData[rNo]*0.000001983/(myBoardInfo.LTC3350_RSNSI1+myBoardInfo.LTC3350_RSNSI1));break;  
          case 0x29: printf("(%.1fA)",(float)testData[rNo]*0.000001983/myBoardInfo.LTC3350_RSNSC);break;  
        case 0x2A: printf("(%.1f°)",(float)testData[rNo]*.028-251.4);break;
        
        default:
          printf("(-----)");
          break;
        }
        printf("\t");
      }      
    }
  }
  LineClear(" ");
  uint16_t ddd=0;
  for(int i=0;i<=0x2A;i++){
    ddd=testData[i];
    switch (i)
    {
      case 0x17: LineClear(" ");printf("(17) CTRL:");
        if(ddd & 1<<0) printf(" wait_to_start_ESR,");
        if(ddd & 1<<1) printf(" GPI_EN,");
        if(ddd & 1<<2) printf(" ESR_MESS_Stop,");
        if(ddd & 1<<3) printf(" Increass_Cap_Mess_100X,");
        break;            
      case 0x1B: LineClear(" ");printf("(1B) STATUS:");
        if(ddd & 1<<0) printf(" charging(stepDown),");
        if(ddd & 1<<1) printf(" backup(stepUp),");
        if(ddd & 1<<2) printf(" chrg_CV,");
        if(ddd & 1<<3) printf(" chrg_UVLO,");
        if(ddd & 1<<4) printf(" chrg_ilim,");
        if(ddd & 1<<5) printf(" chrg_cappg,");
        if(ddd & 1<<6) printf(" chrg_shnt,");
        if(ddd & 1<<7) printf(" chrg_bal,");
        if(ddd & 1<<8) printf(" chrg_dis,");
        if(ddd & 1<<9) printf(" chrg_ci,");
        if(ddd & 1<<11) printf(" chrg_pfo,");
        
      break;  
      case 0x1C: LineClear(" ");printf("(1C) STATUS:");
        if(ddd & 1<<0) printf(" capESR_Active,");
        if(ddd & 1<<1) printf(" capESR_Scheduled,");
        if(ddd & 1<<2) printf(" capESR_Pending,");
        if(ddd & 1<<3) printf(" cap_mess_Done,");
        if(ddd & 1<<4) printf(" ESR_mess_Done,");
        if(ddd & 1<<5) printf(" cap_mess_failed,");
        if(ddd & 1<<6) printf(" ESR_mess_failed,");
        if(ddd & 1<<8) printf(" POWER_failed,");
        if(ddd & 1<<9) printf(" Power_returned,");
        
      break;  
      case 0x1D: LineClear(" ");printf("(1D) ALARM:");
        if(ddd & 1<<0) printf(" capUV,");
        if(ddd & 1<<1) printf(" capOV,");
        if(ddd & 1<<2) printf(" gpiUV,");
        if(ddd & 1<<3) printf(" gpiOV,");
        if(ddd & 1<<4) printf(" VinUV,");
        if(ddd & 1<<5) printf(" VinOV,");
        if(ddd & 1<<6) printf(" VcapUV,");
        if(ddd & 1<<7) printf(" VcapOV,");
        if(ddd & 1<<8) printf(" VoutUV,");
        if(ddd & 1<<9) printf(" VoutOV,");
        if(ddd & 1<<10) printf(" iinOC,");
        if(ddd & 1<<11) printf(" ichgUC,");
        if(ddd & 1<<12) printf(" dTempCold,");
        if(ddd & 1<<13) printf(" dTemp_hot,");
        if(ddd & 1<<14) printf(" ESR_Hi,");
        if(ddd & 1<<15) printf(" cap_Lo,");        
        break;  

    }
  }
  LineClear(" ");
  
/*
  LineClear("___ Status1 (test)_____________________________________ !!!");
  LineClear(" charg_sts: "); if(IC_LTC3350_read_register(LTC3350_CHRG_STATUS,&data)) printf("(%X)     ",data);
  LineClear(" ");printf("synchronous controller: Charging   %d   Step-up               %d",IC_LTC3350_GetValue(LTC3350_CHRG_STEPDOWN,data),IC_LTC3350_GetValue(LTC3350_CHRG_STEPUP,data));
  LineClear(" ");printf("Charger: Constant Voltage          %d   underVoltage lockout  %d",IC_LTC3350_GetValue(LTC3350_CHRG_CV,data),IC_LTC3350_GetValue(LTC3350_CHRG_UVLO,data));
  LineClear(" ");printf("         Input limit               %d   above Power Good      %d",IC_LTC3350_GetValue(LTC3350_CHRG_INPUT_ILIM,data),IC_LTC3350_GetValue(LTC3350_CHRG_CAPPG,data));
  LineClear(" ");printf("         manager is Shuntinge      %d   or is balancing       %d",IC_LTC3350_GetValue(LTC3350_CHRG_SHNT,data),IC_LTC3350_GetValue(LTC3350_CHRG_BAL,data));
  LineClear(" ");printf("         temporarily disablee      %d   constant current mode %d",IC_LTC3350_GetValue(LTC3350_CHRG_DIS,data),IC_LTC3350_GetValue(LTC3350_CHRG_CI,data));
  LineClear(" ");printf("input voltage below PFI            %d",IC_LTC3350_GetValue(LTC3350_CHRG_PFO,data));
  LineClear("");  
    
  LineClear("___ Status2 (test)_____________________________________ !!!");
  LineClear(" ");if(IC_LTC3350_read_register(LTC3350_MON_STATUS,&data)) printf("Mon_Status (%X)",data);
  
  
  //show Time of Cap/ESR measuring time
  int __messProgress=IC_LTC3350_GetValue(LTC3350_MON_CAPESR_ACTIVE,data);
  if(__messProgress==1){
    if(__lmessProgress==0)
      startTime=std::chrono::high_resolution_clock::now();
    
    currentTime = std::chrono::high_resolution_clock::now();
  }
  __lmessProgress=__messProgress;
  auto __testDuration = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);

  LineClear(" ");printf("Cap/ESR meas in progress   %d (%d sec)",__messProgress,(int)__testDuration.count());
  LineClear(" ");printf("        scheduled          %d   Pending            %d",IC_LTC3350_GetValue(LTC3350_MON_CAPESR_SCHEDULED,data),IC_LTC3350_GetValue(LTC3350_MON_CAPESR_PENDING,data));
  LineClear(" ");printf("        Cap meas done      %d   ESR meas done      %d",IC_LTC3350_GetValue(LTC3350_MON_CAP_DONE,data),IC_LTC3350_GetValue(LTC3350_MON_ESR_DONE,data));
  LineClear(" ");printf("        Cap meas failed    %d   ESR meas failed    %d",IC_LTC3350_GetValue(LTC3350_MON_CAP_FAILED,data),IC_LTC3350_GetValue(LTC3350_MON_ESR_FAILED,data));
  LineClear(" ");printf("        Power failed       %d   Power returned     %d",IC_LTC3350_GetValue(LTC3350_MON_POWER_FAILED,data),IC_LTC3350_GetValue(LTC3350_MON_POWER_RETURNED,data));
  LineClear(" ___________________________________________________________");
  //LineClear(" ___________________________________________________________");
  LineClear("");
*/  
  
  return true;
}

bool ICA2315::CheckCapsVoltage(float* VCap){
  uint16_t data;
  float Vcaps[5]={0,0,0,0,0};
  Vcaps[0] = GetVCap(0);
  Vcaps[1] = GetVCap(1);
  Vcaps[2] = GetVCap(2);
  Vcaps[3] = GetVCap(3);
  Vcaps[4] = GetVCap(4);
  //printf("VCaps: %.1fv(%.1fv %.1fv %.1fv %.1fv )   \n",Vcaps[0],Vcaps[1],Vcaps[2],Vcaps[3],Vcaps[4]);
  *VCap=Vcaps[0];
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
/*
 * Init.cpp
 *
 * Created: 20-07-2023 16:11:53
 *  Author: Tushar (SIO)
 */ 

#include "Init_1.h"





ISR(TIMER2_COMPA_vect)
{
   static unsigned int counter,ct_timer;
   
  if(counter > 1000)
  {
    counter = 0;
    _g1Sec_Time = _kSET;
    Utick++;
	
	_gMonitor_Controller_Response++;
	
    if(_sWireless_Pump_Data.Resend_Timer)
    {
      _sWireless_Pump_Data.Resend_Timer--;
    }

    if(ct_timer > 9)
    {
	  ct_timer = 0;
      _sWireless_Pump_Data.Read_Ct_Value_Timer = _kSET;
    }
    ct_timer++;

    R_Phase_Fault_Counter++;
    Y_Phase_Fault_Counter++;
    B_Phase_Fault_Counter++;

    R_Phase_Fault_Repair++;
    Y_Phase_Fault_Repair++;
    B_Phase_Fault_Repair++;

    _sWireless_Pump_Data.Counter_To_Read_Faults++;


    _gTIMER_For_Wdt++;
    if(_gcheck_Wdt == _kSET)
    {
      if(_gTIMER_For_Wdt < 240)
      {
        wdt_reset();
      }
    }
    else
    {
      wdt_reset();
    }
  }
  
  //Phase_Reading_In_Interrupt();
  
  //Interrupt_Analyse_Read_Phases();
  
  Interrupt_Analyse_Read_Phases_New();
  
  counter++;
  OCR2A = _gReload;
}


void Initialise_Hardware(void)
{
  Timer_Setting();
  
  Serial.begin(9600);
  while (!Serial);
  delay(1000);
  Serial.println(F("START"));
  Serial.print(F("Id: "));
  sprintf((char *)_sWireless_Pump_Data.Wireless_Pump_Id,"PA0035");    /* pump unique ID */
  Serial.println((char *)_sWireless_Pump_Data.Wireless_Pump_Id);


  Int_Lora();    /* parameters init of lora rf95 */

  

   /****************************** output pins *******************/
  pinMode(_kPUMP1,OUTPUT);            /* pump p1 */
  pinMode(_kPUMP2,OUTPUT);            /* pump p2 */
  pinMode(_kR_PHASE_STATUS,OUTPUT);
  pinMode(_kY_PHASE_STATUS,OUTPUT);
  pinMode(_kB_PHASE_STATUS,OUTPUT);
  pinMode(_kDRY_RUN_STATUS,OUTPUT);
  pinMode(_kLINE_FAULT_STATUS,OUTPUT);
  pinMode(_kPUMP_STATUS,OUTPUT);
  pinMode(_kLORA_CHIP_SELECT,OUTPUT);


  /*pinMode(_kR_PHASE_INPUT,INPUT);
  pinMode(_kY_PHASE_INPUT,INPUT);
  pinMode(_kB_PHASE_INPUT,INPUT);*/
  
  pinMode(_kAUTO_MANUAL,INPUT);

  Read_EEPROM();          
  /****************************************** set the CT calibration ************************************************/
  RUBLE_PUMP_CURRENT.current(_kADC_PUMP_CUR, _kCT_CALIBRATION_FACTOR);     /* adc pin and calibration factor to read pump running current */

  Serial.println(F("set up finished"));


   /**************** initially pump should be off */
  digitalWrite(_kPUMP1, _kPUMP_OFF);       
  digitalWrite(_kPUMP2, _kPUMP_OFF);

  
   /************************* all indication should be off ***********************/
  _sWireless_Pump_Data.Indication_Of_Faults  = _kRESET;


   /*************************** asign phases to buffer ******************/ 
  _sWireless_Pump_Data.Phase_List[0] = _kR_PHASE_INPUT;
  _sWireless_Pump_Data.Phase_List[1] = _kY_PHASE_INPUT;
  _sWireless_Pump_Data.Phase_List[2] = _kB_PHASE_INPUT;

 // RUBLE_PUMP_CURRENT.current(_kADC_PUMP_CUR, _kCT_CALIBRATION_FACTOR);
}


void Timer_Setting(void)
{
  cli();//stop interrupts
  //set timer2 interrupt at 8kHz
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = _gReload;// = (16*10^6) / (8000*8) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 8 prescaler
  TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  sei();//allow interrupts
}


void Read_EEPROM(void)
{
  _kEEPROM_READ(_kEEPROM_LOC_PUMP_NOMINAL_CURRENT,_sPump_Parameter.Pump_Nominal_Current);
  _kEEPROM_READ(_kCT_BY_PASS,_sWireless_Pump_Data.By_Pass_CT);
}

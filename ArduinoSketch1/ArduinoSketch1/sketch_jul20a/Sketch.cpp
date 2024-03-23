
#include "Init_1.h"
#include "TrueRMS.h"
#include <LowPower.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <ArduinoSort.h>
#include "EmonLib.h"
#include <EEPROM.h>
#include<avr/wdt.h> /* Header for watchdog timers in AVR */

Rms readRms; // create an instance of Rms.

volatile unsigned int Utick;               /* used for heart beat */
volatile byte _gReload = 8;                /* Reload for timer */
volatile unsigned int _g1Sec_Time;         /* Update in every 1 sec */
volatile uint8_t Transmit_Data[20];
volatile uint8_t buff[RH_RF95_MAX_MESSAGE_LEN];                     //array used to store coming response from controller
volatile unsigned int _gFault_Counter[3];

volatile unsigned char R_Phase_Fault_Counter;
volatile unsigned char Y_Phase_Fault_Counter;
volatile unsigned char B_Phase_Fault_Counter;

volatile unsigned char R_Phase_Fault_Repair;
volatile unsigned char Y_Phase_Fault_Repair;
volatile unsigned char B_Phase_Fault_Repair;

volatile unsigned char _gcheck_Wdt;
volatile unsigned int _gTIMER_For_Wdt;
volatile unsigned char Phase_Reading_Number = 0;
volatile unsigned int _gMonitor_Controller_Response;
volatile unsigned char Reading_Ct_Flag;

volatile unsigned long nextLoop;
volatile int adcVal;
volatile unsigned int cnt=0,cnt1=0,cnt2=0;
volatile float VoltRange = 3.3;

Wireless_Pump_Data _sWireless_Pump_Data;
Pump_Parameter     _sPump_Parameter;

EnergyMonitor RUBLE_PUMP_CURRENT;





unsigned char Phase_Reading_In_Interrupt(void)
{
	static unsigned int donot_read;
	if (donot_read)
	{
		donot_read--;
		return 1;
	}
	
	if(Reading_Ct_Flag)
	{
		return 0;
	}
	
	readRms.start(); //start measuring
	
	switch(Phase_Reading_Number)
	{
		case 0:
		{
			adcVal = analogRead(A0);    // B PHASE
			readRms.update(adcVal); // for BLR_ON or for DC(+AC) signals with BLR_OFF
			cnt++;
			
			if(cnt >= 500)
			{
				 readRms.publish();
				cnt = 0;
				 _sWireless_Pump_Data.Dummy_Phase_Reading[Phase_Reading_Number] = readRms.dcBias;
				_sWireless_Pump_Data.Phase_Read_Flag |= _eRPHASE_FLAG;
				Phase_Reading_Number = 1;
				donot_read = 100;
			}
		}break;
		
		case 1:
		{
			adcVal = analogRead(A1);    // B PHASE
			readRms.update(adcVal); // for BLR_ON or for DC(+AC) signals with BLR_OFF
			cnt++;
			
			if(cnt >= 500)
			{
				 readRms.publish();
				 _sWireless_Pump_Data.Dummy_Phase_Reading[Phase_Reading_Number] = readRms.dcBias;
				cnt = 0;
				_sWireless_Pump_Data.Phase_Read_Flag |= _eYPHASE_FLAG;
				Phase_Reading_Number = 2;
				donot_read = 100;
			}
		}break;
		
		case 2:
		{
			adcVal = analogRead(A2);    // B PHASE
			readRms.update(adcVal); // for BLR_ON or for DC(+AC) signals with BLR_OFF
			cnt++;
			
			if(cnt >= 500)
			{
				 readRms.publish();
				 _sWireless_Pump_Data.Dummy_Phase_Reading[Phase_Reading_Number] = readRms.dcBias;
				cnt = 0;
				_sWireless_Pump_Data.Phase_Read_Flag |= _eBPHASE_FLAG;
				Phase_Reading_Number = 0;
				donot_read = 100;
			}
		}break;
	}
}


void setup() 
{
  // put your setup code here, to run once:
  wdt_disable();  /* Disable the watchdog and wait for more than 2 seconds */
  //delay(3000);  /* Done so that the Arduino doesn't keep resetting infinitely in case of wrong configuration */
  wdt_enable(WDTO_8S);  /* Enable the watchdog with a timeout of 2 seconds */

  _gcheck_Wdt = _kRESET;
  
  Initialise_Hardware();

  readRms.begin(VoltRange, RMS_WINDOW, ADC_10BIT, BLR_ON, CNT_SCAN);
  
  readRms.start(); //start measuring
  
  nextLoop = micros() + LPERIOD; // Set the loop timer variable for the next loop interval.

  analogReference(DEFAULT);

  _gcheck_Wdt = _kSET;
  
  _sWireless_Pump_Data.Indication_Of_Faults  &= ~_eLINE_FAULT_STATUS;  
  _sWireless_Pump_Data.Indication_Of_Faults  &= ~_ePUMP_OVERLOAD;
  _sWireless_Pump_Data.Indication_Of_Faults  &= ~_eDRY_RUN_STATUS;
  _sWireless_Pump_Data.Indication_Of_Faults  &= ~_ePUMP_NOT_WORKING;

}


void loop() 
{
  if(_g1Sec_Time)
  {
    
    _g1Sec_Time = _kRESET;
    _gTIMER_For_Wdt = _kRESET;
    Read_Recieved_Packet();
    Faults_Handler();
    Update_Indication();
    Transmit_Fault_Packet(1);
	Print_Phase();
  }
}


unsigned char Read_Phases(void)
{
    static int R_Phases_Mavg[20];
    static int Y_Phases_Mavg[20];
    static int B_Phases_Mavg[20];
    
    static int R_Phases_Avg[20];
    static int Y_Phases_Avg[20];
    static int B_Phases_Avg[20];

    static unsigned char R_Counter;
    static unsigned char Y_Counter;
    static unsigned char B_Counter;
    
    static unsigned char i=0,check_fault;

    int R_Phase_Output, Y_Phase_Output, B_Phase_Output;

   // Serial.print(F("start reading"));
   
   static unsigned int check_max_voltage;
   
   check_max_voltage = _sWireless_Pump_Data.PHASE_AFTER_Filter[0];
   for(unsigned char i=1; i<3; i++)
   {
	   if(check_max_voltage <  _sWireless_Pump_Data.PHASE_AFTER_Filter[i])
	   {
			check_max_voltage = _sWireless_Pump_Data.PHASE_AFTER_Filter[i];
	   }
   }

    readRms.start(); //start measuring
    
    switch(i)
    {
      case 0:
      {
        adcVal = analogRead(A0);    // B PHASE
        readRms.update(adcVal); // for BLR_ON or for DC(+AC) signals with BLR_OFF
        cnt++;
        if(cnt >= 500) 
        { 
          //Serial.print(F("A0, R "));
          
          readRms.publish();
          
          //Serial.print(readRms.rmsVal,2);
         // Serial.print(", ");
         // Serial.print(readRms.dcBias);
         // Serial.print("    ");
          
          cnt=0;
          i = 1;
         
          if(R_Counter < 20)
          {
            R_Phases_Avg[R_Counter] = readRms.dcBias;
            R_Counter++;
          }
          else
          {
            for(unsigned int i=0; i<19; i++)
            {
              R_Phases_Avg[i] = R_Phases_Avg[i+1];
            }
            R_Phases_Avg[19] =  readRms.dcBias;
            
            R_Phase_Output = 0;
            for(unsigned char i=0;i<20;i++)
            {
              R_Phase_Output += R_Phases_Avg[i];
            }
            //R_Phase_Output = R_Phases_Avg[8] + R_Phases_Avg[9] + R_Phases_Avg[10] + R_Phases_Avg[11] + R_Phases_Avg[12];
            R_Phase_Output = R_Phase_Output/20;
            Serial.print(R_Phase_Output);
            Serial.print(" ");
			_sWireless_Pump_Data.PHASE_AFTER_Filter[0] = R_Phase_Output;
            //Serial.println(R_Phase_Output);

            /*Serial.print("R PHASE READING ");
            for(unsigned char i = 0; i<20 ; i++)
            {
              Serial.print(R_Phases_Avg[i]);
      
              Serial.print(" ");
            }
             Serial.println();*/
            check_fault = 1;
			
			
			if(R_Phase_Output < 260 || R_Phase_Output > 460 ||
			check_max_voltage > R_Phase_Output+65)
			{
				
				Serial.print("R PHASE VOLTAGE ");
				Serial.print(R_Phase_Output);
				Serial.println(F("R PHASE not CUT 0 "));
                
				R_Phase_Fault_Counter = _kRESET;
				if(R_Phase_Fault_Repair > 60)
				{
					Serial.println(F("R PHASE not CUT 1"));
					_sWireless_Pump_Data.RPhase_Status = _kRESET;
				}
				
			}
			else if(R_Phase_Output > 270 && R_Phase_Output < 450)
			{
				//Serial.println(F("R UNSTABALED"));

				if(R_Phase_Fault_Counter > 60)
				{
					R_Phase_Fault_Repair = _kRESET;
					/* cosider fault is there */
					_sWireless_Pump_Data.RPhase_Status = _kSET;
				}
			}
			
          } 
        }
      
        while(nextLoop > micros());  // wait until the end of the loop time interval
        nextLoop += LPERIOD;  // set next loop time to current time + LOOP_PERIOD
      }break;
	  
	  

      case 1:   // y
      {
        adcVal = analogRead(A1);
        readRms.update(adcVal); // for BLR_ON or for DC(+AC) signals with BLR_OFF
        cnt++;
        if(cnt >= 500) 
        { 
          //Serial.print(F("A1, Y "));
          
          readRms.publish();
          
          //Serial.print(readRms.rmsVal,2);
          //Serial.print(", ");
          //Serial.print(readRms.dcBias);
          //Serial.print("    ");
          cnt=0;
          i = 2;

          if(Y_Counter < 20)
          {
            Y_Phases_Avg[Y_Counter] = readRms.dcBias;
            Y_Counter++;
          }
          else
          {
            for(unsigned int i=0; i<19; i++)
            {
              Y_Phases_Avg[i] = Y_Phases_Avg[i+1];
            }
            Y_Phases_Avg[19] =  readRms.dcBias;

            Y_Phase_Output = 0;
            
            for(unsigned char i=0;i<20;i++)
            {
              Y_Phase_Output += Y_Phases_Avg[i];
            }
            Y_Phase_Output = Y_Phase_Output/20;

             Serial.print(Y_Phase_Output);
             Serial.print(" ");
			 
			 _sWireless_Pump_Data.PHASE_AFTER_Filter[1] = Y_Phase_Output;
            
            /*Serial.println(Y_Phase_Output);

             Serial.print("Y PHASE READING ");
             for(unsigned char i = 0; i<20 ; i++)
            {
              Serial.print(Y_Phases_Avg[i]);
      
              Serial.print(" ");
            }
            Serial.println();*/
            
            check_fault = 1;
			
			
			if(Y_Phase_Output < 260 || Y_Phase_Output > 460 ||
			check_max_voltage > Y_Phase_Output+65 )
			{
				Y_Phase_Fault_Counter = _kRESET;
				if(Y_Phase_Fault_Repair > 60)
				{
					_sWireless_Pump_Data.YPhase_Status = _kRESET;
				}
			}
			if(Y_Phase_Output > 270 && Y_Phase_Output < 450)
			{
				// Serial.println(F("Y UNSTABALED"));
				if(Y_Phase_Fault_Counter > 60)
				{
					Y_Phase_Fault_Repair = _kRESET;
					
					_sWireless_Pump_Data.YPhase_Status = _kSET;
				}
			}
			
          }
        }
      
        while(nextLoop > micros());  // wait until the end of the loop time interval
        nextLoop += LPERIOD;  // set next loop time to current time + LOOP_PERIOD
      }break;

      case 2:
      {
        adcVal = analogRead(A2);   // b phase
        readRms.update(adcVal); // for BLR_ON or for DC(+AC) signals with BLR_OFF
        cnt++;
        if(cnt >= 500) 
        { 
          //Serial.print(F("A2, B "));
          
          readRms.publish();
          
          //Serial.print(readRms.rmsVal,2);
          //Serial.print(", ");
          //Serial.println(readRms.dcBias);
          cnt=0;
          i = 0;
           if(B_Counter < 20)
          {
            B_Phases_Avg[R_Counter] = readRms.dcBias;
            B_Counter++;
          }
          else
          {
            for(unsigned int i=0; i<19; i++)
            {
              B_Phases_Avg[i] = B_Phases_Avg[i+1];
            }
            B_Phases_Avg[19] =  readRms.dcBias;
            
            B_Phase_Output = 0;
            for(unsigned char i=0;i<20;i++)
            {
              B_Phase_Output += B_Phases_Avg[i];
            }
           
            B_Phase_Output = B_Phase_Output/20;
            Serial.println(B_Phase_Output);
			
			_sWireless_Pump_Data.PHASE_AFTER_Filter[2] = B_Phase_Output;
            
           /* Serial.println(B_Phase_Output);
            Serial.print("B PHASE READING ");
            for(unsigned char i = 0; i<20 ; i++)
            {
              Serial.print(B_Phases_Avg[i]);
      
              Serial.print(" ");
            }
            Serial.println();*/
            check_fault = 1;
			
			if(B_Phase_Output < 260 || B_Phase_Output > 460 ||
			check_max_voltage > B_Phase_Output+65)
			{
				B_Phase_Fault_Counter = _kRESET;

				if(B_Phase_Fault_Repair > 60)
				{
					_sWireless_Pump_Data.BPhase_Status = _kRESET;
				}
			}
			else if(B_Phase_Output > 270 && B_Phase_Output < 450)
			{
				// Serial.println(F("B UNSTABALED"));

				if(B_Phase_Fault_Counter > 60)
				{
					B_Phase_Fault_Repair = _kRESET;
					/* cosider fault is there */
					_sWireless_Pump_Data.BPhase_Status = _kSET;
				}
				else
				{
					
				}
			}
			
          }
          
        }
      
        while(nextLoop > micros());  // wait until the end of the loop time interval
        nextLoop += LPERIOD;  // set next loop time to current time + LOOP_PERIOD
       }break;
    }
    

    if(check_fault)
    {
      check_fault = 0;  
    }   
}

unsigned char Print_Phase(void)
{
	
	 Serial.print(_sWireless_Pump_Data.PHASE_AFTER_Filter[0]);
	 Serial.print(" ");
	// Serial.println(R_Phase_Output);
	
	 Serial.print(_sWireless_Pump_Data.PHASE_AFTER_Filter[1]);
	 Serial.print(" ");
	 //Serial.println(Y_Phase_Output);
	  
	  Serial.println(_sWireless_Pump_Data.PHASE_AFTER_Filter[2]);
}

unsigned char Interrupt_Analyse_Read_Phases_New(void)
{
	if(Reading_Ct_Flag)
	{
		return 0;
	}
	static int R_Phases_Mavg[25];
	static int Y_Phases_Mavg[25];
	static int B_Phases_Mavg[25];
	
	static int R_Phases_Avg[25];
	static int Y_Phases_Avg[25];
	static int B_Phases_Avg[25];

	static unsigned char R_Counter;
	static unsigned char Y_Counter;
	static unsigned char B_Counter;
	
	static unsigned char i=0,check_fault;

	int R_Phase_Output, Y_Phase_Output, B_Phase_Output;
	
	static unsigned int check_max_voltage;
	
	check_max_voltage = _sWireless_Pump_Data.PHASE_AFTER_Filter[0];
	for(unsigned char i=1; i<3; i++)
	{
		if(check_max_voltage <  _sWireless_Pump_Data.PHASE_AFTER_Filter[i])
		{
			check_max_voltage = _sWireless_Pump_Data.PHASE_AFTER_Filter[i];
		}
	}
	
	switch(i)
	{
		case 0:
		{
			adcVal = analogRead(A0);    // B PHASE
			
			i = 1;
				
			if(R_Counter < 25)
			{
				R_Phases_Avg[R_Counter] = adcVal;
				R_Counter++;
			}
			else
			{
				for(unsigned int i=0; i<24; i++)
				{
					R_Phases_Avg[i] = R_Phases_Avg[i+1];
				}
				R_Phases_Avg[24] =  adcVal;
					
				R_Phase_Output = 0;
				for(unsigned char i=0;i<25;i++)
				{
					R_Phase_Output += R_Phases_Avg[i];
				}
					
				R_Phase_Output = R_Phase_Output/25;
				
				_sWireless_Pump_Data.PHASE_AFTER_Filter[0] = R_Check_Equition(R_Phase_Output);
					
				R_Phase_Output = _sWireless_Pump_Data.PHASE_AFTER_Filter[0];
				
				
				check_fault = 1;
					
				if(1)
				{	
					if(_sWireless_Pump_Data.PHASE_AFTER_Filter[0] < 110)
					{
						R_Phase_Fault_Counter = _kRESET;
						if(R_Phase_Fault_Repair > 13)
						{
							_sWireless_Pump_Data.RPhase_Status = _kRESET;
						}
						
					}
					else if(_sWireless_Pump_Data.PHASE_AFTER_Filter[0] > 120)
					{
						if(R_Phase_Fault_Counter > 13)
						{
							R_Phase_Fault_Repair = _kRESET;
							/* cosider fault is there */
							_sWireless_Pump_Data.RPhase_Status = _kSET;
						}
					}
				
					else
					{
						R_Phase_Fault_Repair = _kRESET;
						R_Phase_Fault_Counter = _kRESET;
					}
				}
				else
				{
					if(_sWireless_Pump_Data.PHASE_AFTER_Filter[0] < 110 || _sWireless_Pump_Data.PHASE_AFTER_Filter[0] > 325)
					{
						R_Phase_Fault_Counter = _kRESET;
						if(R_Phase_Fault_Repair > 13)
						{
							_sWireless_Pump_Data.RPhase_Status = _kRESET;
						}	
					}
					
					else if(_sWireless_Pump_Data.PHASE_AFTER_Filter[0] > 120 && _sWireless_Pump_Data.PHASE_AFTER_Filter[0] < 310)
					{
						if(R_Phase_Fault_Counter > 13)
						{
							R_Phase_Fault_Repair = _kRESET;
							/* cosider fault is there */
							_sWireless_Pump_Data.RPhase_Status = _kSET;
						}
					}
					
					else
					{
						R_Phase_Fault_Repair = _kRESET;
						R_Phase_Fault_Counter = _kRESET;
					}
				}
					
			}
			
		}break;
		
		

		case 1:   // y
		{
			adcVal = analogRead(A1);
			
			i = 2;

			if(Y_Counter < 25)
			{
				Y_Phases_Avg[Y_Counter] = adcVal;
				Y_Counter++;
			}
			else
			{
				for(unsigned int i=0; i<24; i++)
				{
					Y_Phases_Avg[i] = Y_Phases_Avg[i+1];
				}
				Y_Phases_Avg[24] =  adcVal;

				Y_Phase_Output = 0;
					
				for(unsigned char i=0;i<25;i++)
				{
					Y_Phase_Output += Y_Phases_Avg[i];
				}
				Y_Phase_Output = Y_Phase_Output/25;
				
				
				_sWireless_Pump_Data.PHASE_AFTER_Filter[1] = Y_Check_Equition(Y_Phase_Output);
				Y_Phase_Output = _sWireless_Pump_Data.PHASE_AFTER_Filter[1];
					
				check_fault = 1;
					
				if(1)
				{	
					if(_sWireless_Pump_Data.PHASE_AFTER_Filter[1] < 110 )
					{
						Y_Phase_Fault_Counter = _kRESET;
						if(Y_Phase_Fault_Repair > 13)
						{
							_sWireless_Pump_Data.YPhase_Status = _kRESET;
						}
					}
					else if(_sWireless_Pump_Data.PHASE_AFTER_Filter[1] > 120)
					{
						if(Y_Phase_Fault_Counter > 13)
						{
							Y_Phase_Fault_Repair = _kRESET;
							
							_sWireless_Pump_Data.YPhase_Status = _kSET;
						}
					}
					else
					{
						Y_Phase_Fault_Counter = _kRESET;
						Y_Phase_Fault_Repair = _kRESET;
					}
				}
				else
				{
					if(_sWireless_Pump_Data.PHASE_AFTER_Filter[1] < 110 || _sWireless_Pump_Data.PHASE_AFTER_Filter[1] > 325 )
					{
						Y_Phase_Fault_Counter = _kRESET;
						if(Y_Phase_Fault_Repair > 13)
						{
							_sWireless_Pump_Data.YPhase_Status = _kRESET;
						}
					}
					else if(_sWireless_Pump_Data.PHASE_AFTER_Filter[1] > 120 && _sWireless_Pump_Data.PHASE_AFTER_Filter[1] < 310)
					{
						if(Y_Phase_Fault_Counter > 13)
						{
							Y_Phase_Fault_Repair = _kRESET;
							_sWireless_Pump_Data.YPhase_Status = _kSET;
						}
					}
					else
					{
						Y_Phase_Fault_Counter = _kRESET;
						Y_Phase_Fault_Repair = _kRESET;
					}
				}
			}
			
		}break;

		case 2:
		{
			adcVal = analogRead(A2);   // b phase
			i = 0;
			if(B_Counter < 25)
			{
				B_Phases_Avg[B_Counter] = adcVal;
				B_Counter++;
			}
			else
			{
				for(unsigned int i=0; i<24; i++)
				{
					B_Phases_Avg[i] = B_Phases_Avg[i+1];
				}
				B_Phases_Avg[24] =  adcVal;
					
				B_Phase_Output = 0;
				for(unsigned char i=0;i<25;i++)
				{
					B_Phase_Output += B_Phases_Avg[i];
				}
					
				B_Phase_Output = B_Phase_Output/25;
					

				_sWireless_Pump_Data.PHASE_AFTER_Filter[2] = B_Check_Equition(B_Phase_Output);
				B_Phase_Output = _sWireless_Pump_Data.PHASE_AFTER_Filter[2];
				
				
				check_fault = 1;
				
				if(1)
				{	
					if(_sWireless_Pump_Data.PHASE_AFTER_Filter[2] < 110)
					{
						B_Phase_Fault_Counter = _kRESET;
						if(B_Phase_Fault_Repair > 13)
						{
							_sWireless_Pump_Data.BPhase_Status = _kRESET;
						}
					}
					else if(_sWireless_Pump_Data.PHASE_AFTER_Filter[2] > 120)
					{
						if(B_Phase_Fault_Counter > 13)
						{
							B_Phase_Fault_Repair = _kRESET;
							/* cosider fault is there */
							_sWireless_Pump_Data.BPhase_Status = _kSET;
						}
						else
						{
							
						}
					}
					else
					{
						B_Phase_Fault_Repair = _kRESET;
						B_Phase_Fault_Counter = _kRESET;
					}
				}
				else
				{
					if(_sWireless_Pump_Data.PHASE_AFTER_Filter[2] < 110 || _sWireless_Pump_Data.PHASE_AFTER_Filter[2] > 325)
					{
						B_Phase_Fault_Counter = _kRESET;
						if(B_Phase_Fault_Repair > 13)
						{
							_sWireless_Pump_Data.BPhase_Status = _kRESET;
						}
					}
					else if(_sWireless_Pump_Data.PHASE_AFTER_Filter[2] > 120 && _sWireless_Pump_Data.PHASE_AFTER_Filter[2] < 310)
					{
						if(B_Phase_Fault_Counter > 13)
						{
							B_Phase_Fault_Repair = _kRESET;
							/* cosider fault is there */
							_sWireless_Pump_Data.BPhase_Status = _kSET;
						}
						else
						{
							
						}
					}
					else
					{
						B_Phase_Fault_Repair = _kRESET;
						B_Phase_Fault_Counter = _kRESET;
					}
				}
			}
			
		}break;
	}
	

	if(check_fault)
	{
		check_fault = 0;
	}
	
}

unsigned char Interrupt_Analyse_Read_Phases(void)
{
	if(Reading_Ct_Flag)
	{
		return 0;
	}
    static int R_Phases_Mavg[20];
    static int Y_Phases_Mavg[20];
    static int B_Phases_Mavg[20];
    
    static int R_Phases_Avg[20];
    static int Y_Phases_Avg[20];
    static int B_Phases_Avg[20];

    static unsigned char R_Counter;
    static unsigned char Y_Counter;
    static unsigned char B_Counter;
    
    static unsigned char i=0,check_fault;

    int R_Phase_Output, Y_Phase_Output, B_Phase_Output;
   
   static unsigned int check_max_voltage;
   
   check_max_voltage = _sWireless_Pump_Data.PHASE_AFTER_Filter[0];
   for(unsigned char i=1; i<3; i++)
   {
	   if(check_max_voltage <  _sWireless_Pump_Data.PHASE_AFTER_Filter[i])
	   {
			check_max_voltage = _sWireless_Pump_Data.PHASE_AFTER_Filter[i];
	   }
   }

    readRms.start(); //start measuring
    
    switch(i)
    {
      case 0:
      {
        adcVal = analogRead(A0);    // B PHASE
        readRms.update(adcVal); // for BLR_ON or for DC(+AC) signals with BLR_OFF
        cnt++;
        if(cnt >= 500) 
        {   
          readRms.publish();
          
          cnt=0;
          i = 1;
         
          if(R_Counter < 15)
          {
            R_Phases_Avg[R_Counter] = readRms.dcBias;
            R_Counter++;
          }
          else
          {
            for(unsigned int i=0; i<14; i++)
            {
              R_Phases_Avg[i] = R_Phases_Avg[i+1];
            }
            R_Phases_Avg[14] =  readRms.dcBias;
            
            R_Phase_Output = 0;
            for(unsigned char i=0;i<15;i++)
            {
              R_Phase_Output += R_Phases_Avg[i];
            }
       
            R_Phase_Output = R_Phase_Output/15;
           
			_sWireless_Pump_Data.PHASE_AFTER_Filter[0] = R_Phase_Output; 
            check_fault = 1;
			
			
			if(R_Phase_Output < 260 || R_Phase_Output > 460 ||
			check_max_voltage > R_Phase_Output+70)
			{
				R_Phase_Fault_Counter = _kRESET;
				if(R_Phase_Fault_Repair > 25)
				{
					_sWireless_Pump_Data.RPhase_Status = _kRESET;
				}
				
			}
			else if(R_Phase_Output > 270 && R_Phase_Output < 450)
			{
				if(R_Phase_Fault_Counter > 25)
				{
					R_Phase_Fault_Repair = _kRESET;
					/* cosider fault is there */
					_sWireless_Pump_Data.RPhase_Status = _kSET;
				}
			}
			
          } 
        }
      }break;
	  
	  

      case 1:   // y
      {
        adcVal = analogRead(A1);
        readRms.update(adcVal); // for BLR_ON or for DC(+AC) signals with BLR_OFF
        cnt++;
        if(cnt >= 500) 
        {
          readRms.publish();
          cnt=0;
          i = 2;

          if(Y_Counter < 15)
          {
            Y_Phases_Avg[Y_Counter] = readRms.dcBias;
            Y_Counter++;
          }
          else
          {
            for(unsigned int i=0; i<14; i++)
            {
              Y_Phases_Avg[i] = Y_Phases_Avg[i+1];
            }
            Y_Phases_Avg[14] =  readRms.dcBias;

            Y_Phase_Output = 0;
            
            for(unsigned char i=0;i<15;i++)
            {
              Y_Phase_Output += Y_Phases_Avg[i];
            }
            Y_Phase_Output = Y_Phase_Output/15;

             
			 
			 _sWireless_Pump_Data.PHASE_AFTER_Filter[1] = Y_Phase_Output;
          
            check_fault = 1;
			
			if(Y_Phase_Output < 260 || Y_Phase_Output > 460 ||
			check_max_voltage > Y_Phase_Output+70 )
			{
				Y_Phase_Fault_Counter = _kRESET;
				if(Y_Phase_Fault_Repair > 25)
				{
					_sWireless_Pump_Data.YPhase_Status = _kRESET;
				}
			}
			if(Y_Phase_Output > 270 && Y_Phase_Output < 450)
			{
				if(Y_Phase_Fault_Counter > 25)
				{
					Y_Phase_Fault_Repair = _kRESET;
					
					_sWireless_Pump_Data.YPhase_Status = _kSET;
				}
			}
          }
        }
      }break;

      case 2:
      {
        adcVal = analogRead(A2);   // b phase
        readRms.update(adcVal); // for BLR_ON or for DC(+AC) signals with BLR_OFF
        cnt++;
        if(cnt >= 500) 
        { 
          readRms.publish();
          cnt=0;
          i = 0;
           if(B_Counter < 15)
          {
            B_Phases_Avg[R_Counter] = readRms.dcBias;
            B_Counter++;
          }
          else
          {
            for(unsigned int i=0; i<14; i++)
            {
              B_Phases_Avg[i] = B_Phases_Avg[i+1];
            }
            B_Phases_Avg[14] =  readRms.dcBias;
            
            B_Phase_Output = 0;
            for(unsigned char i=0;i<15;i++)
            {
              B_Phase_Output += B_Phases_Avg[i];
            }
           
            B_Phase_Output = B_Phase_Output/15;
           

			_sWireless_Pump_Data.PHASE_AFTER_Filter[2] = B_Phase_Output;
            check_fault = 1;
			
			if(B_Phase_Output < 260 || B_Phase_Output > 460 ||
			check_max_voltage > B_Phase_Output+70)
			{
				B_Phase_Fault_Counter = _kRESET;
				if(B_Phase_Fault_Repair > 25)
				{
					_sWireless_Pump_Data.BPhase_Status = _kRESET;
				}
			}
			else if(B_Phase_Output > 270 && B_Phase_Output < 450)
			{
				if(B_Phase_Fault_Counter > 25)
				{
					B_Phase_Fault_Repair = _kRESET;
					/* cosider fault is there */
					_sWireless_Pump_Data.BPhase_Status = _kSET;
				}
				else
				{
					
				}
			}		
          }     
        }
       }break;
    }
    

    if(check_fault)
    {
      check_fault = 0;  
    }   
}


unsigned char Analyse_Read_Phases(void)
{
    static int R_Phases_Mavg[20];
    static int Y_Phases_Mavg[20];
    static int B_Phases_Mavg[20];
    
    static int R_Phases_Avg[20];
    static int Y_Phases_Avg[20];
    static int B_Phases_Avg[20];
	
	

    static unsigned char R_Counter;
    static unsigned char Y_Counter;
    static unsigned char B_Counter;
    
    static unsigned char i=0,check_fault;

    unsigned int R_Phase_Output, Y_Phase_Output, B_Phase_Output;

   // Serial.print(F("start reading"));
   
   static unsigned int check_max_voltage;
   
   check_max_voltage = _sWireless_Pump_Data.PHASE_AFTER_Filter[0];
   for(unsigned char i=1; i<3; i++)
   {
	   if(check_max_voltage <  _sWireless_Pump_Data.PHASE_AFTER_Filter[i])
	   {
		   check_max_voltage = _sWireless_Pump_Data.PHASE_AFTER_Filter[i];
	   }
   }


    
    
        if((_sWireless_Pump_Data.Phase_Read_Flag & _eRPHASE_FLAG) == _eRPHASE_FLAG) 
        { 
          //Serial.print(F("A0, R "));
		  
		  _sWireless_Pump_Data.Phase_Read_Flag &= ~_eRPHASE_FLAG;
          
         
          
          //Serial.print(readRms.rmsVal,2);
         // Serial.print(", ");
         // Serial.print(readRms.dcBias);
         // Serial.print("    ");
          
          i = 1;
         
          if(R_Counter < 20)
          {
            R_Phases_Avg[R_Counter] =  _sWireless_Pump_Data.Dummy_Phase_Reading[0];
            R_Counter++;
          }
          else
          {
            for(unsigned int i=0; i<19; i++)
            {
              R_Phases_Avg[i] = R_Phases_Avg[i+1];
            }
            R_Phases_Avg[19] =   _sWireless_Pump_Data.Dummy_Phase_Reading[0];
            
            R_Phase_Output = 0;
            for(unsigned char i=0;i<20;i++)
            {
              R_Phase_Output += R_Phases_Avg[i];
            }
            //R_Phase_Output = R_Phases_Avg[8] + R_Phases_Avg[9] + R_Phases_Avg[10] + R_Phases_Avg[11] + R_Phases_Avg[12];
             R_Phase_Output = R_Phase_Output/20;
			 
            /* Serial.print(R_Phase_Output);
             Serial.print(" "); */
			 
			 _sWireless_Pump_Data.PHASE_AFTER_Filter[0] = R_Phase_Output;
            //Serial.println(R_Phase_Output);

            /*Serial.print("R PHASE READING ");
            for(unsigned char i = 0; i<20 ; i++)
            {
              Serial.print(R_Phases_Avg[i]);
      
              Serial.print(" ");
            }
             Serial.println();*/
            check_fault = 1;
			
			
			if(R_Phase_Output < 260 || R_Phase_Output > 460 ||
			check_max_voltage > R_Phase_Output+65)
			{
				
				//Serial.print("R PHASE VOLTAGE ");
				//Serial.print(R_Phase_Output);
				//Serial.println(F("R PHASE not CUT 0 "));
                
				R_Phase_Fault_Counter = _kRESET;
				if(R_Phase_Fault_Repair > 60)
				{
					//Serial.println(F("R PHASE not CUT 1"));
					_sWireless_Pump_Data.RPhase_Status = _kRESET;
				}
				
			}
			if(R_Phase_Output > 270 && R_Phase_Output < 450)
			{
				//Serial.println(F("R UNSTABALED"));

				if(R_Phase_Fault_Counter > 60)
				{
					R_Phase_Fault_Repair = _kRESET;
					/* cosider fault is there */
					_sWireless_Pump_Data.RPhase_Status = _kSET;
				}
			}
          } 
        }
     
	  
	  

     
         if((_sWireless_Pump_Data.Phase_Read_Flag & _eYPHASE_FLAG) == _eYPHASE_FLAG)
        { 
          //Serial.print(F("A1, Y "));
          //Serial.print(readRms.rmsVal,2);
          //Serial.print(", ");
          //Serial.print(readRms.dcBias);
          //Serial.print("    ");
		   _sWireless_Pump_Data.Phase_Read_Flag &= ~_eYPHASE_FLAG;
		  
          i = 2;

          if(Y_Counter < 20)
          {
            Y_Phases_Avg[Y_Counter] =  _sWireless_Pump_Data.Dummy_Phase_Reading[1];
            Y_Counter++;
          }
          else
          {
            for(unsigned int i=0; i<19; i++)
            {
              Y_Phases_Avg[i] = Y_Phases_Avg[i+1];
            }
            Y_Phases_Avg[19] =   _sWireless_Pump_Data.Dummy_Phase_Reading[1];

            Y_Phase_Output = 0;
            
            for(unsigned char i=0;i<20;i++)
            {
              Y_Phase_Output += Y_Phases_Avg[i];
            }
            Y_Phase_Output = Y_Phase_Output/20;

             /* Serial.print(Y_Phase_Output);
              Serial.print(" ");*/
			  
			  _sWireless_Pump_Data.PHASE_AFTER_Filter[1] = Y_Phase_Output;
            
            /*Serial.println(Y_Phase_Output);

             Serial.print("Y PHASE READING ");
             for(unsigned char i = 0; i<20 ; i++)
            {
              Serial.print(Y_Phases_Avg[i]);
      
              Serial.print(" ");
            }
            Serial.println();*/
            
            check_fault = 1;
			
			
			if(Y_Phase_Output < 260 || Y_Phase_Output > 460 ||
			check_max_voltage > Y_Phase_Output+65)
			{
				Y_Phase_Fault_Counter = _kRESET;
				if(Y_Phase_Fault_Repair > 60)
				{
					_sWireless_Pump_Data.YPhase_Status = _kRESET;
				}
			}
			if(Y_Phase_Output > 270 && Y_Phase_Output < 450)
			{
				// Serial.println(F("Y UNSTABALED"));
				if(Y_Phase_Fault_Counter > 60)
				{
					Y_Phase_Fault_Repair = _kRESET;
					
					_sWireless_Pump_Data.YPhase_Status = _kSET;
				}
			}
			
          }
        }
      

         if((_sWireless_Pump_Data.Phase_Read_Flag & _eBPHASE_FLAG) == _eBPHASE_FLAG)
        { 
			
			 _sWireless_Pump_Data.Phase_Read_Flag &= ~_eBPHASE_FLAG;
			 
          //Serial.print(F("A2, B "));
          //Serial.print(readRms.rmsVal,2);
          //Serial.print(", ");
          //Serial.println(readRms.dcBias);
          i = 0;
           if(B_Counter < 20)
          {
            B_Phases_Avg[R_Counter] =  _sWireless_Pump_Data.Dummy_Phase_Reading[2];
            B_Counter++;
          }
          else
          {
            for(unsigned int i=0; i<19; i++)
            {
              B_Phases_Avg[i] = B_Phases_Avg[i+1];
            }
            B_Phases_Avg[19] = _sWireless_Pump_Data.Dummy_Phase_Reading[Phase_Reading_Number];
            
            B_Phase_Output = 0;
            for(unsigned char i=0;i<20;i++)
            {
              B_Phase_Output += B_Phases_Avg[i];
            }
           
            /*B_Phase_Output = B_Phase_Output/20;
            Serial.println(B_Phase_Output);*/
			
			_sWireless_Pump_Data.PHASE_AFTER_Filter[2] = B_Phase_Output;
            
           /* Serial.println(B_Phase_Output);
            Serial.print("B PHASE READING ");
            for(unsigned char i = 0; i<20 ; i++)
            {
              Serial.print(B_Phases_Avg[i]);
      
              Serial.print(" ");
            }
            Serial.println();*/
            check_fault = 1;
			
			if(B_Phase_Output < 260 || B_Phase_Output > 460 ||
			check_max_voltage > B_Phase_Output+65)
			{
				B_Phase_Fault_Counter = _kRESET;

				if(B_Phase_Fault_Repair > 60)
				{
					_sWireless_Pump_Data.BPhase_Status = _kRESET;
				}
			}
			if(B_Phase_Output > 270 && B_Phase_Output < 450)
			{
				// Serial.println(F("B UNSTABALED"));

				if(B_Phase_Fault_Counter > 60)
				{
					B_Phase_Fault_Repair = _kRESET;
					/* cosider fault is there */
					_sWireless_Pump_Data.BPhase_Status = _kSET;
				}
				else
				{
					
				}
			}
          } 
        }
}




unsigned char Read_Ct(void)
{
  _sPump_Parameter.Pump_Run_Current = RUBLE_PUMP_CURRENT.calcIrms(_kCT_AVERAGE_TURNS);
  Serial.println(_sPump_Parameter.Pump_Run_Current);
}

unsigned int R_Check_Equition(unsigned int Phase_Input)
{
	unsigned int return_data;
	
	
	if(Phase_Input >= 1 && Phase_Input <=   310 /*558*/)   
	{
		Phase_Input = 200;
	}
	else if(Phase_Input < 186)
	{
		Phase_Input = 250;
	}
	else if(Phase_Input > 320 /*558*/ )
	{
		Phase_Input = 100;
	}
	
	return Phase_Input;
	
}

unsigned int Y_Check_Equition(unsigned int Phase_Input)
{
	unsigned int return_data;
	
	if(Phase_Input >= 1 && Phase_Input <=   310 /*558*/)
	{
		Phase_Input = 200;
	}
	else if(Phase_Input < 186)
	{
		Phase_Input = 250;
	}
	else if(Phase_Input > 320 /*558*/ )
	{
		Phase_Input = 100;
	}
	
	return Phase_Input;
	
}

unsigned int B_Check_Equition(unsigned int Phase_Input)
{
	unsigned int return_data;
	
	if(Phase_Input >= 1 && Phase_Input <=   310 /*558*/)
	{
		Phase_Input = 200;
	}
	else if(Phase_Input < 186)
	{
		Phase_Input = 250;
	}
	else if(Phase_Input > 320 /*558*/ )
	{
		Phase_Input = 100;
	}
	
	return Phase_Input;
}

/*
 * Lora_Recieved.cpp
 *
 * Created: 20-07-2023 16:12:41
 *  Author: Tushar (SIO)
 */ 

#include "Init_1.h"
RH_RF95 rf95(_kLORA_CHIP_SELECT, _kLORA_INTERRUPT_PIN);

int Read_Recieved_Packet(void)
{
	/**************************** read recieved packets *********************************/
	unsigned char id_len,len,i,count,calibration_response;
	//Serial.println(F("wait available "));
	if (rf95.available())
	{
		memset((char *)buff,0,sizeof(buff));
		Serial.print(F("Packet available..."));
		uint8_t len = sizeof(buff);

		if (rf95.recv((unsigned char *)buff, &len))
		{
			id_len = 0;
			for(i=0;i<_kWIRELESS_PUMP_ID_LEN;i++)
			{
				if(buff[i] != _sWireless_Pump_Data.Wireless_Pump_Id[i])
				{
					Serial.println("serial id not matched");
					return 0;
				}
				id_len++;
			}
		}

		if(id_len == _kWIRELESS_PUMP_ID_LEN)
		{
			if(buff[11] != _kWIRELESS_PUMP_NUMBER && buff[12] != 'S')
			{
				return 0;
			}
			/************************************************* check the packet type *****************************/
			switch(buff[6])     /* check which command had sent from the controller */
			{
				case _eCOMMAND_FROM_WIRELESS_PUMP:     /* if controller wants to turn ON or OFF the valve */
				{
					switch(buff[7])   /* check weather controller wants to turn ON or OFF the data */
					{
						case _eDO_CALIBRATION:  /* if controller wants to do the calibration */
						{
							Serial.println(F("Calibration of PUMP: "));
							Serial.flush();
							_sWireless_Pump_Data.Command_From_Ruble |= _eDO_CALIBRATION;
							
							if(buff[13] == 1)
							{
								_sWireless_Pump_Data.By_Pass_CT = _kRESET;
								_kEEPROM_WRITE(_kCT_BY_PASS,_sWireless_Pump_Data.By_Pass_CT);
								calibration_response = _eCALIBARATION_SUCCESS;
							}
							else
							{
								if(Calibration_Of_Pump())
								{
									calibration_response = _eCALIBARATION_SUCCESS;
								}
								else
								{
									calibration_response = _eCALIBRATION_FAIL;
								}
							}
						}break;
						
						case _ePUMP_ON:      /* if controller wants to turn ON the valve */
						{
							Serial.println(F("Request to turn ON PUMP: "));
							Serial.flush();
							_sWireless_Pump_Data.Command_From_Ruble |= _ePUMP_ON;
							_sWireless_Pump_Data.Command_From_Ruble &= ~_ePUMP_OFF;

							if(Check_To_Turn_On_Pump())
							{
		
								Serial.println(F("pump is ON"));
								Serial.flush();
								if(digitalRead(_kPUMP1))
								{	
									_sWireless_Pump_Data.Indication_Of_Faults  &= ~_ePUMP_OVERLOAD;
									_sWireless_Pump_Data.Indication_Of_Faults  &= ~_eDRY_RUN_STATUS;
									_sWireless_Pump_Data.Indication_Of_Faults  &= ~_ePUMP_NOT_WORKING;
									
								}
							}
							else
							{
								buff[7] = _ePUMP_OR_ELECTRICITY_RELATED;
							}
							
						}break;

						case _ePUMP_OFF:   /* if controller wants to turn OFF the device */
						{
							Serial.println(F("Request to turn OFF PUMP: "));
							Serial.flush();
							_sWireless_Pump_Data.Command_From_Ruble |= _ePUMP_OFF;
							_sWireless_Pump_Data.Command_From_Ruble &= ~_ePUMP_ON;
							if (Pump_Off(buff[11]))
							{
								Serial.println(F("pump is off"));
							}
						}break;

						default:     /* if comman not get match */
						{
							Serial.println("pump is off");
							return 0;
						}break;
						
					}
					
					_sWireless_Pump_Data.Operation_Mode  = buff[8];         /* operation mode */
					_sWireless_Pump_Data.Timer_Duration  = buff[10];        /* Pump run timer */
					_sWireless_Pump_Data.Timer_Duration |= buff[9] >> 8;    /* Pump run timer */
					
					/************************************ make the ack package ***********************************/
					count = 0;
					memset((char *)Transmit_Data,0,sizeof(Transmit_Data));

					for(i=0;i<_kWIRELESS_PUMP_ID_LEN;i++)
					{
						Transmit_Data[count++] = _sWireless_Pump_Data.Wireless_Pump_Id[i];
					}
					
					Transmit_Data[count++] = _eCOMMAND_FROM_WIRELESS_PUMP;
					Transmit_Data[count++] = buff[7];
					Transmit_Data[count++] = _sWireless_Pump_Data.Wireless_Pump_Status;
					Transmit_Data[count++] = _sWireless_Pump_Data.Battery_Voltage >> 8;
					Transmit_Data[count++] = _sWireless_Pump_Data.Battery_Voltage;
					Transmit_Data[count++] = _sWireless_Pump_Data.Indication_Of_Faults >> 8;
					Transmit_Data[count++] = _sWireless_Pump_Data.Indication_Of_Faults;
					
					Transmit_Data[count++] = calibration_response;
					
					Transmit_Data[count++] = (int)_sPump_Parameter.Pump_Nominal_Current >> 24;
					Transmit_Data[count++] = (int)_sPump_Parameter.Pump_Nominal_Current >> 16;
					Transmit_Data[count++] = (int)_sPump_Parameter.Pump_Nominal_Current >> 8;
					Transmit_Data[count++] = (int)_sPump_Parameter.Pump_Nominal_Current;
					

					/******************************** SEND DATA TO EURO VALVE ****************************/
					Serial.println(F("send back 1"));
					rf95.send((unsigned char *)Transmit_Data,count);
					rf95.waitPacketSent(2000);
					Serial.println(F("send back 2"));

					if(_sWireless_Pump_Data.Command_From_Ruble & _ePUMP_ON)
					{
						if(buff[7] != _ePUMP_OR_ELECTRICITY_RELATED)
						{
							Pump_On(buff[11]);
						}
					}
					return 1;
					
				}break;
				default:
				{
					/* if command not match */
					// return;
				}break;
			}
		}
		else
		{
			Serial.println("Id Not Match");
		}
		
	}
}

/*
 * Lora_Transmit.cpp
 *
 * Created: 20-07-2023 16:13:24
 *  Author: Tushar (SIO)
 */ 

#include "Init_1.h"

int Transmit_Fault_Packet(char fault)
{
  unsigned char i,count,len,id_len,resend_counter = 0;

  if(_sWireless_Pump_Data.Resend_Timer)
  {
    return 0;
  }

  
  /******************************************************* response to calibration **************************************************/
  
  if(_sWireless_Pump_Data.Response_To_Ruble & _eCALIBARATION_SUCCESS || _sWireless_Pump_Data.Response_To_Ruble & _eCALIBRATION_FAIL)
  {
	  /* send the data of calibration is fail or success */
    memset((char *)Transmit_Data,0,sizeof(Transmit_Data));
    count = 0;
    id_len = 0;
    
    for(i=0;i<_kWIRELESS_PUMP_ID_LEN;i++)
    {
      Transmit_Data[count++] = _sWireless_Pump_Data.Wireless_Pump_Id[i];
    }
  
    Transmit_Data[count++] = _eCALIBRATION_RESPONSE;
    Transmit_Data[count++] = _sWireless_Pump_Data.Response_To_Ruble;
    //Transmit_Data[count++] = _sWireless_Pump_Data.Response_To_Ruble;
    
   resend_counter = _kRESET;
   while(resend_counter < _kMAX_RETRIES-1)
   {
      resend_counter++;
      len = sizeof(buff);
      if (rf95.available())
      {
        rf95.recv((unsigned char *)buff, &len);
      }
       Serial.println(F("rf95.available() 2"));
       rf95.send((unsigned char *)Transmit_Data,count);
       rf95.waitPacketSent(2000);
       Serial.println(F("rf95.waitPacketSent() 2"));
        
       if (rf95.waitAvailableTimeout(3000))
       {
         if (rf95.recv((unsigned char *)buff, &len))            //copy received msg
         {
            Serial.print(F("ack is: "));
            Serial.print((char)buff[0]);
            Serial.print((char)buff[1]);
            Serial.print((char)buff[2]);
            Serial.print((char)buff[3]);
            Serial.print((char)buff[4]);
            Serial.print((char)buff[5]);
            Serial.print(buff[6]);
            Serial.println(buff[7]);
            Serial.println(buff[8]);
         }
    
          id_len = 0;
          
          for(i=0;i<_kWIRELESS_PUMP_ID_LEN;i++)
          {
            if(buff[i] != _sWireless_Pump_Data.Wireless_Pump_Id[i])
            {
              Serial.println("serial id not matched 0");
            }
            id_len++;
          }
          
          switch(buff[6])
          {
            case _eGOT_DATA:
            {
              _sWireless_Pump_Data.Response_To_Ruble &=  ~_eCALIBARATION_SUCCESS;
              _sWireless_Pump_Data.Response_To_Ruble &=  ~_eCALIBRATION_FAIL;
              _sWireless_Pump_Data.Resend_Timer = _kRESET;
              return 1;
            }break;
    
            default:
            {
              
            }break;
          }
       }
        _sWireless_Pump_Data.Resend_Timer = _kRESEND_TIMER;
   }
   
  }


  /************************************************ UPDATE FAULT STATUS OF WIRELESS PUMP *********************************/

  if(_sWireless_Pump_Data.Response_To_Ruble & _eSTATUS_OF_WIRELESS_PUMP)
  {
	  /* if there is changes in any status then send the data to controller */
    memset((char *)Transmit_Data,0,sizeof(Transmit_Data));
    count = 0;
    id_len = 0;
    
    for(i=0;i<_kWIRELESS_PUMP_ID_LEN;i++)
    {
      Transmit_Data[count++] = _sWireless_Pump_Data.Wireless_Pump_Id[i];
    }
  
    Transmit_Data[count++] = _eWIRELESS_FAULT;
    Transmit_Data[count++] = _sWireless_Pump_Data.Response_To_Ruble;
    Transmit_Data[count++] = _sWireless_Pump_Data.Indication_Of_Faults >> 8;
    Transmit_Data[count++] = _sWireless_Pump_Data.Indication_Of_Faults;
	Transmit_Data[count++] = _kWIRELESS_PUMP_NUMBER;
    
   resend_counter = _kRESET;
   while(resend_counter < _kMAX_RETRIES-1)
   {
      resend_counter++;
      len = sizeof(buff);
      if (rf95.available())
      {
        rf95.recv((unsigned char *)buff, &len);
      }
       Serial.println(F("rf95.available() 1"));
       rf95.send((unsigned char *)Transmit_Data,count);
       rf95.waitPacketSent(2000);
       Serial.println(F("rf95.waitPacketSent() 1"));
        
       if (rf95.waitAvailableTimeout(3000))
       {
		   /* read the response here */
         if (rf95.recv((unsigned char *)buff, &len))            //copy received msg
         {
            Serial.print(F("ack is: "));
            Serial.print((char)buff[0]);
            Serial.print((char)buff[1]);
            Serial.print((char)buff[2]);
            Serial.print((char)buff[3]);
            Serial.print((char)buff[4]);
            Serial.print((char)buff[5]);
            Serial.print(buff[6]);
            Serial.println(buff[7]);
            Serial.println(buff[8]);
         }
    
          id_len = 0;
          
          for(i=0;i<_kWIRELESS_PUMP_ID_LEN;i++)
          {
            if(buff[i] != _sWireless_Pump_Data.Wireless_Pump_Id[i])
            {
              Serial.println("serial id not matched 0");
            }
            id_len++;
          }
          switch(buff[6])
          {
            case _eGOT_DATA:
            {
              _sWireless_Pump_Data.Response_To_Ruble &=  ~_eSTATUS_OF_WIRELESS_PUMP;
              _sWireless_Pump_Data.Resend_Timer = _kRESET;
              return 1;
            }break;
    
            default:
            {
              
            }break;
          }
       }
       _sWireless_Pump_Data.Resend_Timer = _kRESEND_TIMER;
    }
  }
}

unsigned char Int_Lora(void)
{
	if (!rf95.init())
	{
		Serial.println(F("init failed!"));
	}

	if (!rf95.setFrequency(_kLORA_FREQUENCY))
	{
		Serial.println(F("set frequency failed!"));
	}
	
	if (!rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128))
	{
		Serial.println(F("config failed"));
	}

	Serial.println(F("Lora Tx power is : "));
	Serial.println(_kLORA_TX_LORA);
	rf95.setTxPower(20, false);

	Serial.println(F("Spreading factor is : "));
	Serial.println(_kSPREADING_FACTOR);
	rf95.setSpreadingFactor(11);
}
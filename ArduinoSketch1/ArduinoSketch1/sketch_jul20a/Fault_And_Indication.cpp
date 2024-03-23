/*
 * Fault_And_Indication.cpp
 *
 * Created: 20-07-2023 16:11:20
 *  Author: Tushar (SIO)
 */ 

#include "Init_1.h"

void Faults_Handler(void)
{
  static unsigned char temp_back_up;
  static unsigned char faults_check_timer = 0;
  static unsigned long int fault_occure = 0;
  static unsigned long int fault_status;

	if(_gMonitor_Controller_Response > 300)
	{
		_gMonitor_Controller_Response = _kRESET;
		Pump_Off(1);
	}
  /************************************************** Read CT *****************************************/
  if(_sWireless_Pump_Data.Read_Ct_Value_Timer)
  {
	
    if(fault_occure)
    {
      fault_occure = _kRESET;
      faults_check_timer++;
    }
	else
	{
		faults_check_timer = 0;
	}
	
	Reading_Ct_Flag = _kSET;
	
    _sWireless_Pump_Data.Read_Ct_Value_Timer = _kRESET;
    _sPump_Parameter.Pump_Run_Current = RUBLE_PUMP_CURRENT.calcIrms(_kCT_AVERAGE_TURNS);
	
	Reading_Ct_Flag = _kRESET;
	
	_kSERAIL_MON_WRITE("Pump Run Current ");
	_kSERIAL_MON_PRINT_NUM(_sPump_Parameter.Pump_Run_Current);

	_kSERAIL_MON_WRITE(" Pump Nominal Current ");
	_kSERIAL_MON_PRINT_NUM(_sPump_Parameter.Pump_Nominal_Current);
	
	_kSERAIL_MON_WRITE(" Fault Counter ");
	_kSERIAL_MON_PRINT_NUM(_sWireless_Pump_Data.Counter_To_Read_Faults);
	
	_kSERAIL_MON_WRITE(" Calibartion status ");
	_kSERIAL_MON_PRINT_NUM(_sWireless_Pump_Data.By_Pass_CT);
  }


  /****************************************************** CHECK AUTO MANUAL ******************************/
  switch(Auto_Manual_Check())
  {
    case 0:
    {
      //Serial.println(F("manual mode"));
      _sWireless_Pump_Data.Indication_Of_Faults |= _eMANUAL_MODE;
      _sWireless_Pump_Data.Indication_Of_Faults &= ~_eAUTO_MODE;
      /*if(! _sWireless_Pump_Data.Indication_Of_Faults & _eMANUAL_MODE)
      {
        Serial.println(F("manual mode 1"));
        _sWireless_Pump_Data.Indication_Of_Faults |= _eMANUAL_MODE;
      }*/
    }break;

    case 1:
    {
      //Serial.println(F("auto mode"));
       _sWireless_Pump_Data.Indication_Of_Faults |= _eAUTO_MODE;
       _sWireless_Pump_Data.Indication_Of_Faults &= ~_eMANUAL_MODE;
      /*if(! _sWireless_Pump_Data.Indication_Of_Faults & _eAUTO_MODE)
      {
        _sWireless_Pump_Data.Indication_Of_Faults |= _eAUTO_MODE;
      }*/
    }break;
  }


  /******************************************************* CHECK OVERLOAD **********************************************/
  if(_sPump_Parameter.Pump_Run_Current > (_sPump_Parameter.Pump_Nominal_Current) + ((_sPump_Parameter.Pump_Nominal_Current/100) * 32)
  && (_sWireless_Pump_Data.Counter_To_Read_Faults > 500) && _sWireless_Pump_Data.By_Pass_CT == _kSET)
  {
    if((_sWireless_Pump_Data.Indication_Of_Faults & _ePUMP_OVERLOAD) != _ePUMP_OVERLOAD)
    {
      fault_occure = _ePUMP_OVERLOAD;
      _kSERAIL_MON_WRITE("Pump Overlaoded ");
      _kSERIAL_MON_PRINT_NUM(faults_check_timer);
      if(faults_check_timer > 3)
      {
        _sWireless_Pump_Data.Indication_Of_Faults |= _ePUMP_OVERLOAD;
         faults_check_timer = 0;
         _kSERAIL_MON_WRITE("Pump Overlaoded");
      }
      
    }
  }
  else
  {
    _sWireless_Pump_Data.Indication_Of_Faults &= ~_ePUMP_OVERLOAD;
    if(fault_occure == _ePUMP_OVERLOAD)
    {
	  faults_check_timer = 0;	
      fault_occure = _kRESET;
    }
  }


   /******************************************************* CHECK DRY RUN **********************************************/
  if(_sPump_Parameter.Pump_Run_Current < ((_sPump_Parameter.Pump_Nominal_Current/100) * 58)
   && (_sWireless_Pump_Data.Counter_To_Read_Faults > 500) && _sWireless_Pump_Data.By_Pass_CT == _kSET
   && _sPump_Parameter.Pump_Run_Current > ((_sPump_Parameter.Pump_Nominal_Current/100) * 15))
  {
    if(_sPump_Parameter.Pump_Run_Current > ((_sPump_Parameter.Pump_Nominal_Current/100) * 20))
    {
      if((_sWireless_Pump_Data.Indication_Of_Faults & _eDRY_RUN_STATUS) != _eDRY_RUN_STATUS)
      {
        fault_occure = _eDRY_RUN_STATUS;
        _kSERAIL_MON_WRITE("Pump Dry Run ");
        _kSERIAL_MON_PRINT_NUM(faults_check_timer);
        if(faults_check_timer > 3)
        {
          _sWireless_Pump_Data.Indication_Of_Faults |= _eDRY_RUN_STATUS;
          faults_check_timer = 0;
          _kSERAIL_MON_WRITE("Pump Dry Run ");
        }
      }
    }
  }
  else
  {
    _sWireless_Pump_Data.Indication_Of_Faults &= ~_eDRY_RUN_STATUS;
    if(fault_occure == _eDRY_RUN_STATUS)
    {
	  faults_check_timer = 0;
      fault_occure = _kRESET;
    }
  } 
  
  
   /******************************************************* CHECK PUMP WORKING **********************************************/
   if((_sWireless_Pump_Data.Counter_To_Read_Faults > 500) && _sWireless_Pump_Data.By_Pass_CT == _kSET
   && _sWireless_Pump_Data.Pump_Got_On == _kSET &&
   _sPump_Parameter.Pump_Run_Current < ((_sPump_Parameter.Pump_Nominal_Current/100) * 20))
   {
	   if(_sPump_Parameter.Pump_Run_Current < ((_sPump_Parameter.Pump_Nominal_Current/100) * 20))
	   {
		   if((_sWireless_Pump_Data.Indication_Of_Faults & _ePUMP_NOT_WORKING) != _ePUMP_NOT_WORKING)
		   {
			   fault_occure = _ePUMP_NOT_WORKING;
			   _kSERAIL_MON_WRITE("Pump Is Not Working ");
			   _kSERIAL_MON_PRINT_NUM(faults_check_timer);
			   if(faults_check_timer > 3)
			   {
				   _sWireless_Pump_Data.Indication_Of_Faults |= _ePUMP_NOT_WORKING;
				   faults_check_timer = 0;
				   _kSERAIL_MON_WRITE("Pump Is Not Working 1...........");
			   }
		   }
	   }
   }
   else
   {
	   _sWireless_Pump_Data.Indication_Of_Faults &= ~_ePUMP_NOT_WORKING;
	   if(fault_occure == _ePUMP_NOT_WORKING)
	   {
		   faults_check_timer = 0;
		   fault_occure = _kRESET;
	   }
   }
  

  /******************************************************* Read R PHASE **********************************************/
  if(_sWireless_Pump_Data.RPhase_Status)
  {
     _sWireless_Pump_Data.Indication_Of_Faults |= _eR_PHASE_STATUS;
  }
  else
  {
      _sWireless_Pump_Data.Indication_Of_Faults &= ~_eR_PHASE_STATUS;
  }


  /******************************************************* Read Y PHASE **********************************************/
  if(_sWireless_Pump_Data.YPhase_Status)
  {
     _sWireless_Pump_Data.Indication_Of_Faults |= _eY_PHASE_STATUS;
  }
  else
  {
      _sWireless_Pump_Data.Indication_Of_Faults &= ~_eY_PHASE_STATUS;
  }


  /******************************************************* Read B PHASE **********************************************/
  if(_sWireless_Pump_Data.BPhase_Status)
  {
     _sWireless_Pump_Data.Indication_Of_Faults |= _eB_PHASE_STATUS;
  }
  else
  {
      _sWireless_Pump_Data.Indication_Of_Faults &= ~_eB_PHASE_STATUS;
  }


  /******************************************************* CHECK LINE FAULT *******************************************/
  
  /*_sWireless_Pump_Data.Indication_Of_Faults |= _eR_PHASE_STATUS;
  _sWireless_Pump_Data.Indication_Of_Faults |= _eY_PHASE_STATUS;
  _sWireless_Pump_Data.Indication_Of_Faults |= _eB_PHASE_STATUS;*/
  
  if( (_sWireless_Pump_Data.Indication_Of_Faults & _eR_PHASE_STATUS) != _eR_PHASE_STATUS ||
      (_sWireless_Pump_Data.Indication_Of_Faults & _eY_PHASE_STATUS) !=  _eY_PHASE_STATUS||
      (_sWireless_Pump_Data.Indication_Of_Faults & _eB_PHASE_STATUS) !=  _eB_PHASE_STATUS)
  {
    if((_sWireless_Pump_Data.Indication_Of_Faults & _eLINE_FAULT_STATUS) != _eLINE_FAULT_STATUS)
    {
      fault_occure = _eLINE_FAULT_STATUS;
      _kSERAIL_MON_WRITE("Pump line fault ");
      _kSERIAL_MON_PRINT_NUM(faults_check_timer);
      if(faults_check_timer > 3)
      {
        _sWireless_Pump_Data.Indication_Of_Faults |= _eLINE_FAULT_STATUS;
        faults_check_timer = 0;
        _kSERAIL_MON_WRITE("Pump line fault ");
      }
    }
  }
  else
  {
    _sWireless_Pump_Data.Indication_Of_Faults &= ~_eLINE_FAULT_STATUS;
    if(fault_occure == _eLINE_FAULT_STATUS)
    {
      fault_occure = _kRESET;
    }
  }


  /********************************************** Pump off if any fault ******************************************/
  /*Pump_Off_For_Faults(1);*/

  
  /********************************************** check pump on condition ******************************************/
  if(_sWireless_Pump_Data.By_Pass_CT == _kSET)
  {
    if(_sPump_Parameter.Pump_Run_Current < ((_sPump_Parameter.Pump_Nominal_Current/100) * 15))
    {
      if((_sWireless_Pump_Data.Indication_Of_Faults & _ePUMP_STATUS) == _ePUMP_STATUS)
      {
        Serial.println(F("Pump is off"));
        _sWireless_Pump_Data.Indication_Of_Faults &= ~_ePUMP_STATUS;
        fault_occure = _kRESET;
        faults_check_timer = _kRESET;
      } 
    }
	
	 /************************************************** check pump off condition *************************************/
	 if(_sPump_Parameter.Pump_Run_Current > ((_sPump_Parameter.Pump_Nominal_Current/100) * 18))
	 {
		 if((_sWireless_Pump_Data.Indication_Of_Faults & _ePUMP_STATUS) != _ePUMP_STATUS)
		 {
			 Serial.println(F("Pump is on"));
			 _sWireless_Pump_Data.Indication_Of_Faults |= _ePUMP_STATUS;
			 //fault_occure = _kRESET;
			 //faults_check_timer = _kRESET;
		 }
	 }
  }
  else
  {
    if(digitalRead(_kPUMP1))
    {
      _sWireless_Pump_Data.Indication_Of_Faults &= ~_ePUMP_STATUS;
    }
    else
    {
      _sWireless_Pump_Data.Indication_Of_Faults |= _ePUMP_STATUS;
    }
  }

  
 

 // _sWireless_Pump_Data.Indication_Of_Faults |= _eAUTO_MODE;
  //_sWireless_Pump_Data.Indication_Of_Faults &= ~_eMANUAL_MODE;
  
  /*_sWireless_Pump_Data.Indication_Of_Faults |= _eR_PHASE_STATUS;
  _sWireless_Pump_Data.Indication_Of_Faults |= _eY_PHASE_STATUS;
  _sWireless_Pump_Data.Indication_Of_Faults |= _eB_PHASE_STATUS;*/
  
  Pump_Off_For_Faults(1);

  /******************************************** CHECK IF ANY NEW STATUS UPDATED **********************************/
  if(fault_status != _sWireless_Pump_Data.Indication_Of_Faults)
  {
     Serial.println(F("Fault packet is available"));
     fault_status = _sWireless_Pump_Data.Indication_Of_Faults;
     _sWireless_Pump_Data.Response_To_Ruble |= _eSTATUS_OF_WIRELESS_PUMP;
  } 

}


void Update_Indication(void)
{
  if((_sWireless_Pump_Data.Indication_Of_Faults & _eR_PHASE_STATUS) == _eR_PHASE_STATUS)
  {
    //Serial.println(F("_kR_PHASE_STATUS oN"));
    digitalWrite(_kR_PHASE_STATUS, LOW);
  }
  else
  {
     //Serial.println(F("_kR_PHASE_STATUS oFF"));
    digitalWrite(_kR_PHASE_STATUS, HIGH);
  }

  if((_sWireless_Pump_Data.Indication_Of_Faults & _eY_PHASE_STATUS) == _eY_PHASE_STATUS)
  {
    //Serial.println(F("_kY_PHASE_STATUS oN"));
    digitalWrite(_kY_PHASE_STATUS, LOW);
  }
  else
  {
    //Serial.println(F("_kY_PHASE_STATUS oFF"));
    digitalWrite(_kY_PHASE_STATUS, HIGH);
  }

  if((_sWireless_Pump_Data.Indication_Of_Faults & _eB_PHASE_STATUS) == _eB_PHASE_STATUS)
  {
    //Serial.println(F("_kB_PHASE_STATUS oN"));
    digitalWrite(_kB_PHASE_STATUS, LOW);
  }
  else
  {
    //Serial.println(F("_kB_PHASE_STATUS oFF"));
    digitalWrite(_kB_PHASE_STATUS, HIGH);
  }

  

  if((_sWireless_Pump_Data.Indication_Of_Faults & _eDRY_RUN_STATUS) == _eDRY_RUN_STATUS ||
  (_sWireless_Pump_Data.Indication_Of_Faults & _eLINE_FAULT_STATUS) == _eLINE_FAULT_STATUS ||
  (_sWireless_Pump_Data.Indication_Of_Faults & _ePUMP_OVERLOAD)     == _ePUMP_OVERLOAD ||
  (_sWireless_Pump_Data.Indication_Of_Faults & _ePUMP_NOT_WORKING)  == _ePUMP_NOT_WORKING ||
  (_sWireless_Pump_Data.Indication_Of_Faults & _eR_PHASE_STATUS)    != _eR_PHASE_STATUS ||
  (_sWireless_Pump_Data.Indication_Of_Faults & _eY_PHASE_STATUS)    != _eY_PHASE_STATUS ||
  (_sWireless_Pump_Data.Indication_Of_Faults & _eB_PHASE_STATUS)    != _eB_PHASE_STATUS)
  {
    digitalWrite(_kDRY_RUN_STATUS, LOW);
  }
  else
  {
    digitalWrite(_kDRY_RUN_STATUS, HIGH);
  }

  /*if((_sWireless_Pump_Data.Indication_Of_Faults & _eLINE_FAULT_STATUS) == _eLINE_FAULT_STATUS)
  {
    digitalWrite(_kLINE_FAULT_STATUS, LOW);
  }
  else
  {
    digitalWrite(_kLINE_FAULT_STATUS, HIGH);
  }

  if((_sWireless_Pump_Data.Indication_Of_Faults & _ePUMP_OVERLOAD) == _ePUMP_OVERLOAD)
  {
    digitalWrite(_kLINE_FAULT_STATUS, LOW);
  }
  else
  {
    digitalWrite(_kLINE_FAULT_STATUS, HIGH);
  }*/

  /*if(_sWireless_Pump_Data.Indication_Of_Faults & _ePUMP_OVERLOAD)
  {
    digitalWrite(_kLINE_FAULT_STATUS, LOW);
  }
  else
  {
    digitalWrite(_kLINE_FAULT_STATUS, HIGH);
  }*/


  if((_sWireless_Pump_Data.Indication_Of_Faults & _eR_PHASE_STATUS) == _eR_PHASE_STATUS &&
  (_sWireless_Pump_Data.Indication_Of_Faults & _eY_PHASE_STATUS) == _eY_PHASE_STATUS &&
  (_sWireless_Pump_Data.Indication_Of_Faults & _eB_PHASE_STATUS) == _eB_PHASE_STATUS)
  {
    digitalWrite(_kPUMP_STATUS, LOW);
  }
  else
  {
    digitalWrite(_kPUMP_STATUS, HIGH);
  }

  /*if((_sWireless_Pump_Data.Indication_Of_Faults & _ePUMP_STATUS) == _ePUMP_STATUS)
  {
   // Serial.println(F("_ePUMP_STATUS 1 off"));
    digitalWrite(_kPUMP_STATUS, LOW);
  }
  else
  {
   // Serial.println(F("_ePUMP_STATUS 1 on"));
    digitalWrite(_kPUMP_STATUS, HIGH);
  }*/
  
}


unsigned char Auto_Manual_Check(void)
{
  if(digitalRead(_kAUTO_MANUAL))
  {
    return 1;
  }
  return 0;
}


unsigned char Check_To_Turn_On_Pump(void)
{
    Serial.println(_sWireless_Pump_Data.Indication_Of_Faults,BIN);

    Serial.println(_eMANUAL_MODE);

    Serial.println(_eLINE_FAULT_STATUS);

    if((_sWireless_Pump_Data.Indication_Of_Faults & _eMANUAL_MODE) != _eMANUAL_MODE)
    {
      Serial.println(F("1.1"));
    }

    if((_sWireless_Pump_Data.Indication_Of_Faults & _eLINE_FAULT_STATUS) != _eLINE_FAULT_STATUS)
    {
      Serial.println(F("1.2"));
    }
    
    
    if( (_sWireless_Pump_Data.Indication_Of_Faults & _eMANUAL_MODE) != (unsigned int)_eMANUAL_MODE
    &&  (_sWireless_Pump_Data.Indication_Of_Faults & _eLINE_FAULT_STATUS) != (unsigned int)_eLINE_FAULT_STATUS)
    {
      return 1;
    }  
    
    return 0;
  
    if( (_sWireless_Pump_Data.Indication_Of_Faults & _eMANUAL_MODE) != _eMANUAL_MODE
    &&  (_sWireless_Pump_Data.Indication_Of_Faults & _eLINE_FAULT_STATUS) != _eLINE_FAULT_STATUS )
    {
      return 1;
    }  
    
    return 0;
}

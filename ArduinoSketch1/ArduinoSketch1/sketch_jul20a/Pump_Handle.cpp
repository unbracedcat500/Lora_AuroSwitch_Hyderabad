/*
 * Pump_Handle.cpp
 *
 * Created: 20-07-2023 16:14:37
 *  Author: Tushar (SIO)
 */ 

#include "Init_1.h"

unsigned char Pump_On(unsigned char Pump_Number)
{
	_gMonitor_Controller_Response = _kRESET;
	if(_sWireless_Pump_Data.Command_From_Ruble & _ePUMP_ON)
	{
		if(digitalRead(_kPUMP1))
		{
			_sWireless_Pump_Data.Counter_To_Read_Faults = 0;
			digitalWrite(_kPUMP1, _kPUMP_ON);
			digitalWrite(_kPUMP2, _kPUMP_ON);
			delay(7000);
			digitalWrite(_kPUMP2, _kPUMP_OFF);
			_sWireless_Pump_Data.Command_From_Ruble &= ~_ePUMP_ON;
			
			_sWireless_Pump_Data.Pump_Got_On = _kSET;
			//_sWireless_Pump_Data.Indication_Of_Faults  &= ~_eLINE_FAULT_STATUS;
			_sWireless_Pump_Data.Indication_Of_Faults  &= ~_ePUMP_OVERLOAD;
			_sWireless_Pump_Data.Indication_Of_Faults  &= ~_eDRY_RUN_STATUS;
			_sWireless_Pump_Data.Indication_Of_Faults  &= ~_ePUMP_NOT_WORKING;	
		}
	}
	return 1;
}

unsigned char Pump_Off(unsigned char Pump_Number)
{
	_gMonitor_Controller_Response = _kRESET;
	if(_sWireless_Pump_Data.Command_From_Ruble |= _ePUMP_OFF)
	{
		_sWireless_Pump_Data.Command_From_Ruble &= ~_ePUMP_OFF;
	}
	digitalWrite(_kPUMP1, _kPUMP_OFF);
	digitalWrite(_kPUMP2, _kPUMP_OFF);
	
	_sWireless_Pump_Data.Pump_Got_On = _kRESET;
	return 1;
}

unsigned char Pump_Off_For_Faults(unsigned char Pump_Number)
{
	if((_sWireless_Pump_Data.Indication_Of_Faults & _eLINE_FAULT_STATUS) ==  _eLINE_FAULT_STATUS||
	(_sWireless_Pump_Data.Indication_Of_Faults & _eDRY_RUN_STATUS) ==  _eDRY_RUN_STATUS  ||
	(_sWireless_Pump_Data.Indication_Of_Faults & _ePUMP_OVERLOAD) == _ePUMP_OVERLOAD ||
	(_sWireless_Pump_Data.Indication_Of_Faults & _eMANUAL_MODE) == _eMANUAL_MODE ||
	(_sWireless_Pump_Data.Indication_Of_Faults & _ePUMP_NOT_WORKING) == _ePUMP_NOT_WORKING)
	{
		_kSERAIL_MON_WRITE(" Pump Fault Occure ");
		_kSERIAL_MON_PRINT_NUM(_sWireless_Pump_Data.Indication_Of_Faults,BIN);
		_sWireless_Pump_Data.Pump_Got_On = _kRESET;
		if(!digitalRead(_kPUMP1))
		{
			digitalWrite(_kPUMP1, _kPUMP_OFF);
			digitalWrite(_kPUMP2, _kPUMP_OFF);
			delay(5000);
			return 1;
		}
	}
}
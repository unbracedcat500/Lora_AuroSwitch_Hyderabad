/*
 * Calibration.cpp
 *
 * Created: 20-07-2023 16:10:25
 *  Author: Tushar (SIO)
 */ 
#include "Init_1.h"

int Calibration_Of_Pump(void)
{
	unsigned long previous,timeout;
	unsigned char answer =0;
	float filter_data[5],filter_data_1;
	char i,j;
	
	timeout = 7000;
	
	if(_sWireless_Pump_Data.Command_From_Ruble & _eDO_CALIBRATION)
	{
		Reading_Ct_Flag = _kSET;
		
		_sWireless_Pump_Data.Command_From_Ruble &= ~_eDO_CALIBRATION;
		previous = millis();
		do
		{
			for(i=0;i<5;i++)
			{
				filter_data[i] = RUBLE_PUMP_CURRENT.calcIrms(_kCT_AVERAGE_TURNS);
			}
			for (i = 0; i < 5; i++)
			{
				for (j = i + 1; j < 5; j++)
				{
					if (filter_data[i] > filter_data[j])
					{
						filter_data_1  = filter_data[i];
						filter_data[i] = filter_data[j];
						filter_data[j] = filter_data_1;
					}
				}
			}
			_sPump_Parameter.Pump_Run_Current =  filter_data[2];      // RUBLE_PUMP_CURRENT.calcIrms(_kCT_AVERAGE_TURNS);
			
			if(_sPump_Parameter.Pump_Run_Current > 2)
			{
				_kEEPROM_WRITE(_kEEPROM_LOC_PUMP_NOMINAL_CURRENT,_sPump_Parameter.Pump_Run_Current);     /* save the pump current */
				_kEEPROM_READ(_kEEPROM_LOC_PUMP_NOMINAL_CURRENT,_sPump_Parameter.Pump_Nominal_Current);  /* read pump nominal current */
				_kSERAIL_MON_WRITE("Pump Calibration is done");                /* for debugging purpose */
				answer = 1;
			}
		}while ((answer == 0) && ((millis() - previous) < timeout));
		
		Reading_Ct_Flag = _kRESET;
		
		if(answer) /*************************************************** transmit data to ruble **************************************/
		{
			_sWireless_Pump_Data.By_Pass_CT = _kSET;
			_kEEPROM_WRITE(_kCT_BY_PASS,_sWireless_Pump_Data.By_Pass_CT);
			_sWireless_Pump_Data.Response_To_Ruble |= _eCALIBARATION_SUCCESS;
			_sWireless_Pump_Data.Response_To_Ruble &= ~_eCALIBRATION_FAIL;
		}
		else
		{
			_kSERAIL_MON_WRITE("Pump Calibration is fail");
			_sWireless_Pump_Data.Response_To_Ruble &= ~_eCALIBARATION_SUCCESS;
			_sWireless_Pump_Data.Response_To_Ruble |= _eCALIBRATION_FAIL;
		}
	}
}

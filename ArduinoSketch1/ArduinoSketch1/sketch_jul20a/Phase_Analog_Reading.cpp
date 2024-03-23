/*
 * Phase_Analog_Reading.cpp
 *
 * Created: 20-07-2023 16:14:03
 *  Author: Tushar (SIO)
 */ 


#include "TrueRMS.h"
#include "Init_1.h"

// unsigned char Read_All_Phase(void)
// {
//   unsigned int readings[20],output;
//   unsigned char read_index = 0;
//   //unsigned char fault_counter[3];
//   static unsigned char fault_exist[3],phase = 0;
// 
//   //clock_prescale_set(clock_div_2);
// 
//   
//   
//   if(phase > 2)
//   {
//     phase = 0;
//   }
// 
//   //Serial.println(F("READING INIT!"));
// 
//   for(read_index = 0; read_index < 20; read_index++)
//   {
//     /*(void)analogRead(_sWireless_Pump_Data.Phase_List[phase]);
//     readings[read_index] = analogRead(_sWireless_Pump_Data.Phase_List[phase]);
//     Serial.println(readings[read_index]);*/
// 
//     (void)analogRead(A0);
//     readings[read_index] = analogRead(A0);
//     Serial.println(readings[read_index]);
//     delay(10);
//   }
//   
//   sortArray(readings, 20);
//   output =  readings[8] + readings[9] + readings[10] + readings[11] + readings[12];
//   
//   output = output/5;
//   
//   if(output > 900 ||  output < 200)
//   {
//     fault_exist[phase] = _kSET;
//   }
//   else
//   {
//     fault_exist[phase] = _kRESET;
//     _gFault_Counter[phase] = _kRESET;
//      _sWireless_Pump_Data.RPhase_Status = _kRESET;
//   }
//   
//   if(fault_exist[phase] == _kSET && _gFault_Counter[phase] > 120 )
//   {
//     
//     /* fault occure */
//     switch(phase)
//     {
//       case 0:
//       {
//         _sWireless_Pump_Data.RPhase_Status = _kSET;
//       }break;
//       case 1:
//       {
//         _sWireless_Pump_Data.YPhase_Status = _kSET;
//       }break;
//       case 2:
//       {
//         _sWireless_Pump_Data.BPhase_Status = _kSET;
//       }break;
//     }
//     
//   }
//   
//    phase++;
// }


unsigned char Phase_Detection(void)
{
  unsigned int r_phase,y_phase,b_phase;
  unsigned char y_phase_detected;

  y_phase_detected = 0;
  
  r_phase = analogRead(A0);

  
  
  if(r_phase > 850 && r_phase < 950)
  {
    delay(6);
    y_phase = analogRead(A1);
    b_phase = analogRead(A2);

    if((y_phase > 850 && y_phase < 950) || (b_phase > 850 && b_phase < 950))
    {
      delay(6);

      if((y_phase > 850 && y_phase < 950))
      {
        y_phase_detected = 1;
      }

       y_phase = analogRead(A1);
       b_phase = analogRead(A2);
 
    }
    else
    {
      return 2;
    }

    if(!y_phase_detected)
    {
      if((y_phase > 850 && y_phase < 950))
      {
        return 1;
      }
    }
    if((b_phase > 850 && b_phase < 950))
    {
      return 1;
    }


    return 0;
    
  }

  return 4;
}
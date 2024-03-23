// /*
//  * Lora_Transmit.cpp
//  *
//  * Created: 20-07-2023 16:13:24
//  *  Author: Tushar (SIO)
//  */ 
// 
// #include "Init_1.h"
// 
// int Transmit_Fault_Packet(char fault)
// {
//   unsigned char i,count,len,id_len,resend_counter = 0;
// 
//   if(_sWireless_Pump_Data.Resend_Timer)
//   {
//     return 0;
//   }
// 
//   
//   /******************************************************* response to calibration **************************************************/
//   
//   if(_sWireless_Pump_Data.Response_To_Ruble & _eCALIBARATION_SUCCESS || _sWireless_Pump_Data.Response_To_Ruble & _eCALIBRATION_FAIL)
//   {
//     memset(Transmit_Data,0,sizeof(Transmit_Data));
//     count = 0;
//     id_len = 0;
//     
//     for(i=0;i<_kWIRELESS_PUMP_ID_LEN;i++)
//     {
//       Transmit_Data[count++] = _sWireless_Pump_Data.Wireless_Pump_Id[i];
//     }
//   
//     Transmit_Data[count++] = _eCALIBRATION_RESPONSE;
//     Transmit_Data[count++] = _sWireless_Pump_Data.Response_To_Ruble;
//     //Transmit_Data[count++] = _sWireless_Pump_Data.Response_To_Ruble;
//     
//    resend_counter = _kRESET;
//    while(resend_counter < _kMAX_RETRIES-1)
//    {
//       resend_counter++;
//       len = sizeof(buff);
//       if (rf95.available())
//       {
//         rf95.recv(buff, &len);
//       }
//        Serial.println(F("rf95.available() 2"));
//        rf95.send(Transmit_Data,count);
//        rf95.waitPacketSent(2000);
//        Serial.println(F("rf95.waitPacketSent() 2"));
//         
//        if (rf95.waitAvailableTimeout(3000))
//        {
//          if (rf95.recv(buff, &len))            //copy received msg
//          {
//             Serial.print(F("ack is: "));
//             Serial.print((char)buff[0]);
//             Serial.print((char)buff[1]);
//             Serial.print((char)buff[2]);
//             Serial.print((char)buff[3]);
//             Serial.print((char)buff[4]);
//             Serial.print((char)buff[5]);
//             Serial.print(buff[6]);
//             Serial.println(buff[7]);
//             Serial.println(buff[8]);
//          }
//     
//           id_len = 0;
//           
//           for(i=0;i<_kWIRELESS_PUMP_ID_LEN;i++)
//           {
//             if(buff[i] != _sWireless_Pump_Data.Wireless_Pump_Id[i])
//             {
//               Serial.println("serial id not matched 0");
//             }
//             id_len++;
//           }
//           
//           switch(buff[6])
//           {
//             case _eGOT_DATA:
//             {
//               _sWireless_Pump_Data.Response_To_Ruble &=  ~_eCALIBARATION_SUCCESS;
//               _sWireless_Pump_Data.Response_To_Ruble &=  ~_eCALIBRATION_FAIL;
//               _sWireless_Pump_Data.Resend_Timer = _kRESET;
//               return 1;
//             }break;
//     
//             default:
//             {
//               
//             }break;
//           }
//        }
// 
//         _sWireless_Pump_Data.Resend_Timer = _kRESEND_TIMER;
// 
//        /*if(resend_counter > _kMAX_RETRIES)
//        {
//           resend_counter = _kRESET;
//           _sWireless_Pump_Data.Resend_Timer = _kRESEND_TIMER;
//        }
//        else
//        {
//           delay(5000);
//        }*/
//    }
//    
//   }
// 
// 
//   /************************************************ UPDATE FAULT STATUS OF WIRELESS PUMP *********************************/
// 
//   if(_sWireless_Pump_Data.Response_To_Ruble & _eSTATUS_OF_WIRELESS_PUMP)
//   {
//     memset(Transmit_Data,0,sizeof(Transmit_Data));
//     count = 0;
//     id_len = 0;
//     
//     for(i=0;i<_kWIRELESS_PUMP_ID_LEN;i++)
//     {
//       Transmit_Data[count++] = _sWireless_Pump_Data.Wireless_Pump_Id[i];
//     }
//   
//     Transmit_Data[count++] = _eWIRELESS_FAULT;
//     Transmit_Data[count++] = _sWireless_Pump_Data.Response_To_Ruble;
//     Transmit_Data[count++] = _sWireless_Pump_Data.Indication_Of_Faults >> 8;
//     Transmit_Data[count++] = _sWireless_Pump_Data.Indication_Of_Faults;
//     
//    resend_counter = _kRESET;
//    while(resend_counter < _kMAX_RETRIES-1)
//    {
//       resend_counter++;
//       len = sizeof(buff);
//       if (rf95.available())
//       {
//         rf95.recv(buff, &len);
//       }
//        Serial.println(F("rf95.available() 1"));
//        rf95.send(Transmit_Data,count);
//        rf95.waitPacketSent(2000);
//        Serial.println(F("rf95.waitPacketSent() 1"));
//         
//        if (rf95.waitAvailableTimeout(3000))
//        {
//          if (rf95.recv(buff, &len))            //copy received msg
//          {
//             Serial.print(F("ack is: "));
//             Serial.print((char)buff[0]);
//             Serial.print((char)buff[1]);
//             Serial.print((char)buff[2]);
//             Serial.print((char)buff[3]);
//             Serial.print((char)buff[4]);
//             Serial.print((char)buff[5]);
//             Serial.print(buff[6]);
//             Serial.println(buff[7]);
//             Serial.println(buff[8]);
//          }
//     
//           id_len = 0;
//           
//           for(i=0;i<_kWIRELESS_PUMP_ID_LEN;i++)
//           {
//             if(buff[i] != _sWireless_Pump_Data.Wireless_Pump_Id[i])
//             {
//               Serial.println("serial id not matched 0");
//             }
//             id_len++;
//           }
//           switch(buff[6])
//           {
//             case _eGOT_DATA:
//             {
//               _sWireless_Pump_Data.Response_To_Ruble &=  ~_eSTATUS_OF_WIRELESS_PUMP;
//               _sWireless_Pump_Data.Resend_Timer = _kRESET;
//               return 1;
//             }break;
//     
//             default:
//             {
//               
//             }break;
//           }
//        }
// 
//        _sWireless_Pump_Data.Resend_Timer = _kRESEND_TIMER;
// 
//        /*if(resend_counter > _kMAX_RETRIES)
//        {
//           resend_counter = _kRESET;
//           _sWireless_Pump_Data.Resend_Timer = _kRESEND_TIMER;
//        }
//        else
//        {
//           delay(5000);
//        }*/
//     }
//   }
// }
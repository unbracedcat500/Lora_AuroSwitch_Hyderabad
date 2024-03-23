/*
 * Init_1.h
 *
 * Created: 20-07-2023 16:16:03
 *  Author: Tushar (SIO)
 */ 


#ifndef INIT_1_H_
#define INIT_1_H_

//#include "MY_INC/TrueRMS.h"
//#include "Init_1.h"
#include <LowPower.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <ArduinoSort.h>
#include "EmonLib.h"
#include <EEPROM.h>
#include<avr/wdt.h> /* Header for watchdog timers in AVR */


/******************************************************* setting for true rms ******************************************/
#define LPERIOD      1000    // loop period time in us. In this case 1.0ms
#define ADC_INPUT     A0     // define the used ADC input channel
#define RMS_WINDOW    40   // rms window of 40 samples, means 2 periods @50Hz
//#define RMS_WINDOW  50   // rms window of 50 samples, means 3 periods @60Hz



#define PIN_DEBUG 4




// input scaling circuit in front of the ADC.







/********************************************* DIGITAL INPUT OUTPUT *****************************************************/
#define _kBATTERY_VOLATGE_PIN        A0   /* Analog pin to measure battery voltage */
#define _kADC_PUMP_CUR               A6   /* Analog pin to measure Pump Current */

#define _kPUMP1                      9    /* Digital output pin to turn on P1 */
#define _kPUMP2                      8    /* Digital output pin to turn on P2 */
#define _kR_PHASE_STATUS             17   /* Digital output pin to update indication of R Phase */
#define _kY_PHASE_STATUS             A4   /* Digital output pin to update indication of Y Phase */
#define _kB_PHASE_STATUS             A5   /* Digital output pin to update indication of B Phase */
#define _kDRY_RUN_STATUS             3    /* Digital output pin to update indication of Dry Run */
#define _kLINE_FAULT_STATUS          3    /* Digital output pin to update indication of Line Fault */
#define _kPUMP_STATUS                7    /* Digital output pin to update indication of Pump Status */
#define _kLORA_CHIP_SELECT           10   /* Digital output pin for CS lora */


#define _kR_PHASE_INPUT              A0   /* Digital input pin to read  R phase status */
#define _kY_PHASE_INPUT              A1   /* Digital intput pin to read Y phase status */
#define _kB_PHASE_INPUT              A2   /* Digital intput pin to read B phase status */
#define _kLORA_INTERRUPT_PIN          2   /* Digital intput pin to check Interrupt of lora */
#define _kAUTO_MANUAL                 4
#define _kMOVING_AVG                  10







/********************************************** OTHER MACROS ************************************************************/
#define _kLORA_FREQUENCY                          865.0625                // lora frequency
#define _kOFF                                        0
#define _kON                                         1
#define _kOFFLINE                                    0
#define _kONLINE                                     1
#define _kRESET                                      0
#define _kSET                                        1
#define _kMAX_RETRIES                                3
#define _kWIRELESS_PUMP_ID_LEN                       6
#define _kCT_CALIBRATION_FACTOR                      15
#define _kCT_AVERAGE_TURNS                          1480
#define _kKEEP_MONITOR_WIRELESS_PUMP_COMMAND        330
#define _kPUMP_ON                                    0
#define _kPUMP_OFF                                   1
#define _kRESEND_TIMER                              1*25



#define _kEEPROM_LOC_PUMP_NOMINAL_CURRENT            1
#define _kCT_BY_PASS                                 2
#define _kSPREADING_FACTOR                           11
#define _kLORA_TX_LORA                               20
#define _kWIRELESS_PUMP_NUMBER                       2




/************************************************* HARDWARE MACROS ******************************************************/
#define _kSERIAL_MON                                 Serial
#define _KSERIAL_MON_INIT                            Serial.begin
#define _kSERAIL_MON_WRITE(string)                   Serial.println(F(string))
#define _kSERAIL_MON_WRITE_NO_LN(string)             Serial.print(F(string))
#define _kSERIAL_MON_CLEAR                           Serial.flush
#define _kSERIAL_MON_PRINT_NUM                       Serial.println
#define _kSERIAL_MON_WRITE                           Serial.write
#define _kSERIAL_MON_PRINT_NUM_NO_NEW_LINE           Serial.print

#define _kEEPROM_WRITE                               EEPROM.put
#define _kEEPROM_READ                                EEPROM.get


#define _kCHANEL0                                     A0


/*********************************************** ENUMS ******************************************************************/
enum _eUPDATE_STATUS_OF_FAULTS
{
	_eR_PHASE_STATUS      = 0x01,
	_eY_PHASE_STATUS      = 0x02,
	_eB_PHASE_STATUS      = 0x04,
	_eDRY_RUN_STATUS      = 0x08,
	_eLINE_FAULT_STATUS   = 0x10,
	_ePUMP_OVERLOAD       = 0X20,
	_ePUMP_STATUS         = 0x40,
	_eMANUAL_MODE         = 0x80,
	_eAUTO_MODE           = 0X100,
	_ePUMP_NOT_WORKING    = 0X200
};




enum _eCOMMAND_FROM_RUBLE
{
	_eDO_CALIBRATION   = 0X01,
	_ePUMP_ON          = 0X02,
	_ePUMP_OFF         = 0X04
};

enum _eRESPONSE_TO_RUBLE
{
	_eCALIBARATION_SUCCESS         = 0x01,
	_eCALIBRATION_FAIL             = 0X02,
	_eSTATUS_OF_WIRELESS_PUMP      = 0X04,
	_ePUMP_OR_ELECTRICITY_RELATED  = 0X08
	
};

enum _eWIRELESS_PUMP_COMMNAD_OR_RESP
{
	_eCOMMAND_FROM_WIRELESS_PUMP = 1,
	_eCALIBRATION_RESPONSE,
	_eWIRELESS_FAULT,
};

enum _eFEEDBACK_FROM_RUBLE
{
	_eGOT_DATA = 0X01,
};



/************************************************ INIT LIBRARARIES ******************************************************/


extern EnergyMonitor RUBLE_PUMP_CURRENT;







/****************************************************STRUCTURES ********************************************************/

typedef struct Wireless_Pump_Data
{
	unsigned char Wireless_Pump_Id[_kWIRELESS_PUMP_ID_LEN+1];
	unsigned char Battery_Status;
	unsigned char Operation_Mode;
	unsigned int  Battery_Voltage;
	unsigned int  Timer_Duration;
	unsigned char Wireless_Pump_Status;
	unsigned char Wireless_Pump_Online;
	unsigned char Fault;
	unsigned int  Indication_Of_Faults;
	unsigned int  Read_Ct_Value_Timer;
	unsigned char Command_From_Ruble;
	unsigned char Response_To_Ruble;
	unsigned int  Resend_Timer;
	unsigned char Phase_List[3];
	unsigned char RPhase_Status;
	unsigned char YPhase_Status;
	unsigned char BPhase_Status;
	unsigned int Counter_To_Read_Faults;
	unsigned char By_Pass_CT;
	unsigned char Phase_Read_Flag;
	unsigned int Dummy_Phase_Reading[3];
	unsigned char Pump_Got_On;
	unsigned int PHASE_AFTER_Filter[3];
}Wireless_Pump_Data;


typedef struct Pump_Parameter
{
	float Pump_Nominal_Current;                           /**< current consumption of pump at first time */
	float Pump_Run_Current;                               /**< power consumption */
	unsigned char Irrigation_Status;                      /**< ongoing,dry run etc */
	unsigned char Pump_Status;                            /**< pump on or off */
	
}Pump_Parameter;


enum _ePHASE_READ_FLAG
{
	_eRPHASE_FLAG = 0x01,
	_eYPHASE_FLAG = 0x02,
	_eBPHASE_FLAG = 0x04
	
};



/********************************************************** variables *************************************************/
extern volatile unsigned int Utick;               /* used for heart beat */
extern volatile byte _gReload;                /* Reload for timer */
extern volatile unsigned int _g1Sec_Time;         /* Update in every 1 sec */
extern volatile uint8_t Transmit_Data[20];
extern volatile uint8_t buff[RH_RF95_MAX_MESSAGE_LEN];                     //array used to store coming response from controller
extern volatile unsigned int _gFault_Counter[3];
extern volatile unsigned long nextLoop;
extern volatile int adcVal;
extern volatile unsigned int cnt,cnt1,cnt2;
extern volatile float VoltRange; // The full scale value is set to 5.00 Volts but can be changed when using an
extern volatile unsigned char Phase_Reading_Number;
extern volatile unsigned char R_Phase_Fault_Counter;
extern volatile unsigned char Y_Phase_Fault_Counter;
extern volatile unsigned char B_Phase_Fault_Counter;

extern volatile unsigned char R_Phase_Fault_Repair;
extern volatile unsigned char Y_Phase_Fault_Repair;
extern volatile unsigned char B_Phase_Fault_Repair;

extern volatile unsigned char _gcheck_Wdt;
extern volatile unsigned int _gTIMER_For_Wdt;
extern volatile unsigned int _gMonitor_Controller_Response;
extern volatile unsigned char Reading_Ct_Flag;


extern Wireless_Pump_Data _sWireless_Pump_Data;
extern Pump_Parameter     _sPump_Parameter;

/*************************************************** Functions ********************************************************/
void Timer_Setting(void);
void Initialise_Hardware(void);
void Update_Indication(void);
unsigned char Pump_On(unsigned char Pump_Number);
unsigned char Pump_Off(unsigned char Pump_Number);
void Read_EEPROM(void);
int Read_Recieved_Packet(void);
int Transmit_Fault_Packet(char fault);
unsigned char Auto_Manual_Check(void);
unsigned char Read_All_Phase(void);
unsigned char Phase_Detection(void);
unsigned char Read_Phases(void);
void Faults_Handler(void);
unsigned char Check_To_Turn_On_Pump(void);
int Calibration_Of_Pump(void);
unsigned char Int_Lora(void);
unsigned char Phase_Reading_In_Interrupt(void);
unsigned char Pump_Off_For_Faults(unsigned char Pump_Number);
unsigned char Interrupt_Analyse_Read_Phases(void);
unsigned char Print_Phase(void);
unsigned int Check_Equition(unsigned int Phase_Input);
unsigned char Interrupt_Analyse_Read_Phases_New(void);
unsigned int R_Check_Equition(unsigned int Phase_Input);
unsigned int Y_Check_Equition(unsigned int Phase_Input);
unsigned int B_Check_Equition(unsigned int Phase_Input);






#endif /* INIT_1_H_ */
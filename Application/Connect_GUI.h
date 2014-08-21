/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Connect_GUI.c
Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
/*
    Connect to GUI USE USART3 via RS-232 Protocol
    USART3 - Tx -> Port D Pin PD8
    USART3 - Rx -> Port D Pin PD9
    Baud Rate = 115200
    package = 8-n-1
    
*/
#include "main.h"
#include "DefinePin.h"
#include "GLCD5110.h"

#ifndef __CONNECT_GUI
#define __CONNECT_GUI

// Define ----------------------------------------------------------------------
#define PROFILE_NOTUPLOAD               0
#define PROFILE_SETTING_COMPLETE				1
#define PROFILE_JUST_UPLOAD							2
#define TEST_COMPLETE										5

#define RUN_BUTTON_SET									3
#define RUN_BUTTON_RESET								4
//------------------------------------------------------------------------------
/*
  Define Structer for Pre-term Infants' Profile
*/
typedef struct
{
  char cHospital_Number[13];
  uint8_t uiOxygenSaturation_Maximum;
  uint8_t uiOxygenSaturation_Minimum; 
  uint8_t uiFiO2_Maximum; 
  uint8_t uiFiO2_Minimum; 
  uint8_t uiRespondsTime; 
  uint8_t uiPrefered_FiO2; 
  uint16_t uiAlarm_Level1; 
  uint16_t uiAlarm_Level2; 
  uint8_t uiMode; 
  uint8_t uiProfile_Status;
}Profile;

#endif 
//------------------------------------------------------------------------------
// Function
void USART_GUI_Connect (void);
void CRC_CALCULATE_TX(void);
unsigned int TX_CRC(unsigned int crc, unsigned int data);
void connect_command(void);
void Update_Rule(void);

//------------------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
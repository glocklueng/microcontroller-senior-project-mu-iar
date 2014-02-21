/*
Project : Programmable Feedback Control of Airflow System for Pre-term infant oxygen saturation
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

// Define ----------------------------------------------------------------------
#define PROFILE_NOTUPLOAD				0
#define PROFILE_SETTING_COMPLETE                        1
#define PROFILE_JUST_UPLOAD				2

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
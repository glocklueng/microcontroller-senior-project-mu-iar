/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term Infants
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : DAC_LTC1661.h

Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University

Function : Transfer Data to LTC1661. LTC1661 is Ditital to Analog Convertor IC
           Use Serial to Periphral (SPI) to Transfer Data
          
LTC 1661 
  - DAC Resolution 10 bits
  - Header code for transfer SPI to LTC 1661 (DAC 10 bits)
	Control Code (A3 A2 A1 A0)
	1 0 0 1 	- 	Load Input Reg A (Load DAC A), Output Update, HEX : 0x9000
	1 0 1 0 	-	Load Input Reg B (Load DAC B), Output Update, HEX : 0xA000
	1 1 1 1		-  	Load Input Reg A and B (Load DAC channal A and channal B), Output Update, HEX : 0xF000
		
	Data DAC Reg is 10 bits (D9 - D0)
		
	Don't care 2 bits (X1,X0)
*/

//------------------------------------------------------------------------------
#include "main.h"

//------------------------------------------------------------------------------
#define Air_Valve			1
#define Oxygen_Valve		2
// Define Function -------------------------------------------------------------
void SPI2_SetUp(void);
void LTC1661_Setup(void);
void SentData_DAC (uint16_t DAC_real, uint8_t channel);

/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/

/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term Infants
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Oxygen_Pulse_Meter.h

Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
// -----------------------------------------------------------------------------
#include "main.h"
#include <stdlib.h>
//------------------------------------------------------------------------------
/* 0x40004800 (USART3 Address base) + 0x04 (USART_DR register Address, Address offset) = 0x40004804*/
#define   USART3_DR_ADDRESS     (uint32_t)0x40004804                            // define for DMA
// Function --------------------------------------------------------------------
void usart_OPM_setup (void);
//int fputc(int ch, FILE *f);
int Get_OxygenSat(char cOPM_protocal[]);
void clear_OPM_buffer(void);

//------------------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/

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
// Function --------------------------------------------------------------------
void Oxygen_PM_Setup(void);
//int fputc(int ch, FILE *f);
int Get_OxygenSat(void);
void clear_OPM_buffer(void);

//------------------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/

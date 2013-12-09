/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term Infants
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Oxygen_Pulse_Meter.h
*/
// -----------------------------------------------------------------------------
#include "main.h"
#include <stdlib.h>
// Function --------------------------------------------------------------------
void Oxygen_PM_Setup(void);
int fputc(int ch, FILE *f);
int GET_FiO2(void);

//------------------------------------------------------------------------------

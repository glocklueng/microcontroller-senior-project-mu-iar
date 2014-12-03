/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : system_init.h

Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "DAC_LTC1661.h"
#include "Oxygen_sensor.h"
#include "Oxygen_Pulse_Meter.h"
#include "GLCD5110.h"

//------------------------------------------------------------------------------
void EXTILine0_Config(void);
void Button_EXTI_Config (void);
void Button_Up_Down_Init(void);
void Alarm_Timer_SetUp(void);
void timer4_setup(void);

// End of File -------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
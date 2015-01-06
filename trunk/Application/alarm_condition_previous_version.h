/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Connect_GUI.c
Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "Connect_GUI.h"
#include "Control_valve.h"
#include "alarm_condition.h"
//------------------------------------------------------------------------------
// Define ----------------------------------------------------------------------
#define ALARM_ENABLE            1
#define ALARM_DISABLE           0
//------------------------------------------------------------------------------
void Alarm_Function(uint8_t Command);

// End of File ------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : check_status_profile_v3.h
Deverloper : Phattaradanai Kiratiwudhikul
Reseach & Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "DAC_LTC1661.h"
#include "Oxygen_Pulse_Meter.h"
#include "Connect_GUI.h"
#include "alarm_condition_v3.h"
#include "Control_valve.h"
#include "GLCD5110.h"
//------------------------------------------------------------------------------
uint8_t check_status(uint8_t uiCurrent_SpO2);
static uint8_t check_profile_rule_base(uint8_t uiSpO2_check_profile);
// End of File -----------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : check_status_v1.c

Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "DAC_LTC1661.h"
#include "Connect_GUI.h"
#include "Oxygen_Pulse_Meter.h"
#include "alarm_condition.h"
#include "Control_valve.h"
#include "GLCD5110.h"
// Define ----------------------------------------------------------------------
#define TIMER_DISABLE         0
#define TIMER_ENABLE          1

#define Status_Normal                              0
#define Status_OxygenSat_Below_L1                  1
#define Status_OxygenSat_Below_L2                  2
#define Status_OxygenSat_Behigh_L1                 3
#define Status_OxygenSat_Behigh_L2                 4
#define Status_Alarm                               5
//-------------------------------------------------------------------------------
uint8_t check_status_previous_version (uint8_t uiCurrent_oxygenSaturation);
// End of File ------------------------------------------------------------------
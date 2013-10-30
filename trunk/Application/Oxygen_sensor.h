/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term Infants
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Oxygen_sensor.h
*/
//------------------------------------------------------------------------------
#include "main.h"

//define Function --------------------------------------------------------------
void OxygenSensor_Setup(void);
float Oxygen_convert(void);
void timer_setting (void);
void Calibrate_OxygenSensor(void);
void Timer6_SetUp (void);
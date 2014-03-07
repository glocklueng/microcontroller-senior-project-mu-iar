/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Control_valve.c

Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "DAC_LTC1661.h"
#include "Oxygen_Pulse_Meter.h"
#include "Oxygen_sensor.h"
#include "GLCD5110.h"
#include "DefinePin.h"
#include "Connect_GUI.h"
//------------------------------------------------------------------------------
extern float FiO2_Current;
//------------------------------------------------------------------------------
uint16_t Convert_Voltage_to_Hex (uint8_t voltage)
{
  uint16_t Hex_Value;
  Hex_Value = voltage * 204;
  return Hex_Value;
}

void FiO2_Range (uint8_t FiO2_Value)
{
  uint8_t DAC_Voltage_Air, DAC_Voltage_Oxygen;

  if (FiO2_Value == 21)
  {
    SentData_DAC(0x3FF, Air_Valve);
    SentData_DAC(0x000, Oxygen_Valve);
  }
  else if (FiO2_Value > 21 & FiO2_Value < 30)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(4.3);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(2.4);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);

    while(FiO2_Current != FiO2_Value)
    {
			
    }
  }
  else if (FiO2_Value >= 30 & FiO2_Value < 35)
  {

  }
  else if (FiO2_Value >= 35 & FiO2_Value < 41)
  {

  }
  else if (FiO2_Value >= 41 & FiO2_Value < 45)
  {
  	/* code */
  }
  else if (FiO2_Value >= 45 & FiO2_Value < 47)
  {

  }
  else if (FiO2_Value >= 47 & FiO2_Value < 50)
  {

  }
  else if (FiO2_Value >= 50 & FiO2_Value < 52)
  {
    /* code */
  }
  else if (FiO2_Value >= 52 & FiO2_Value < 56)
  {
    /* code */
  }
  else if (FiO2_Value >= 56 & FiO2_Value < 58)
  {
    /* code */
  }
  else if (FiO2_Value >= 58 & FiO2_Value < 60)
  {
    /* code */
  }
  else if (FiO2_Value >= 60 & FiO2_Value < 64)
  {
    /* code */
  }
  else if (FiO2_Value >= 64 & FiO2_Value < 70)
  {
    /* code */
  }
  else if (FiO2_Value >= 70 & FiO2_Value < 73)
  {
    /* code */
  }
  else if (FiO2_Value >= 73 & FiO2_Value < 75)
  {
    /* code */
  }
  else if (FiO2_Value >= 75 & FiO2_Value < 80)
  {
    /* code */
  }
  else if (FiO2_Value >= 80 & FiO2_Value < 85)
  {
    /* code */
  }
  else if (FiO2_Value >= 85 & FiO2_Value < 90)
  {
    /* code */
  }
  else if (FiO2_Value >= 90 & FiO2_Value < 95)
  {
    /* code */
  }
  else if (FiO2_Value >= 95 & FiO2_Value < 98)
  {

  }
  else if (FiO2_Value >=98 & FiO2_Value <= 100)
  {
    /* code */
  }
}



// End of File -------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
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
/*
  Function : Convert_Voltage_to_Hex
  Input : float voltage
  Return: None
  Description : This Function is convert voltage value (0-5 V) to Hex value (10 bits, 0x000-0x03FF).
*/
uint16_t Convert_Voltage_to_Hex (float voltage)
{
  uint16_t Hex_Value;
  Hex_Value = voltage*204;
  
  return Hex_Value;
}

/*
  Function : FiO2_Range
  Input : uint8_t FiO2_Value
  Return: None
  Description : Select FiO2 Value. This Function is control valve follow FiO2 range.
*/
void FiO2_Range (uint8_t FiO2_Value)
{
  uint16_t DAC_Voltage_Air, DAC_Voltage_Oxygen;

  if (FiO2_Value == 21)
  {
    SentData_DAC(0x3FF, Air_Valve);
    SentData_DAC(0x000, Oxygen_Valve);
  }
  else if (FiO2_Value >= 22 & FiO2_Value < 25)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(4.6);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(2.2);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 25 & FiO2_Value < 27)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(4.5);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(2.3);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 27 & FiO2_Value < 30)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(4.4);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(2.4);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 30 & FiO2_Value < 33)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(4.3);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(2.5);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 33 & FiO2_Value < 35)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(4.2);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(2.6);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 35 & FiO2_Value < 40)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(4.1);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(2.7);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 40 & FiO2_Value < 43)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(4.0);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(2.8);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 43 & FiO2_Value < 45)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(3.9);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(2.9);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 45 & FiO2_Value < 47)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(3.8);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(3.0);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 47 & FiO2_Value < 49)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(3.7);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(3.1);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 49 & FiO2_Value < 55)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(3.8);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(3.0);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 55 & FiO2_Value < 61)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(3.2);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(3.6);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 61 & FiO2_Value < 68)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(3.6);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(3.2);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 68 & FiO2_Value < 80)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(3.4);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(3.4);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 80 & FiO2_Value < 85)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(3.2);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(3.6);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 85 & FiO2_Value < 93)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(2.9);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(3.9);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (FiO2_Value >= 93 & FiO2_Value < 95)
  {
    DAC_Voltage_Air = Convert_Voltage_to_Hex(2.7);
    SentData_DAC(DAC_Voltage_Air, Air_Valve);
    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(4.1);
    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
  }
//  else if (FiO2_Value >= 94 & FiO2_Value < 97)
//  {
//    DAC_Voltage_Air = Convert_Voltage_to_Hex(2.7);
//    SentData_DAC(DAC_Voltage_Air, Air_Valve);
//    DAC_Voltage_Oxygen = Convert_Voltage_to_Hex(4.2);
//    SentData_DAC(DAC_Voltage_Oxygen, Oxygen_Valve);
//  }
  else if (FiO2_Value >= 95 & FiO2_Value <=100)
  {
    SentData_DAC(0x3FF, Oxygen_Valve);
    SentData_DAC(0x000, Air_Valve);
  }
}



// End of File -------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
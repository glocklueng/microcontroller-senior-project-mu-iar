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
/*
  Function : Convert_Voltage_to_Hex
  Input : float voltage
  Return: uint16_t Hex_Value
  Description : This Function is convert voltage value (0-5 V) to Hex value (10 bits, 0x000-0x03FF).
*/
uint16_t Convert_Voltage_to_Hex (float fVoltage)
{
  uint16_t uiHex_Value;
  uiHex_Value = (uint16_t)((fVoltage * 204) / 1);                                // 0D204 = 0.1 v
  
  return uiHex_Value;
}
//-------------------------------------------------------------------------------
/*
  Function : FiO2_Range
  Input : uint8_t uiFiO2_Value
  Return: None
  Description : Select FiO2 Value. This Function is control valve follow FiO2 range.
*/
void FiO2_Range (uint8_t uiFiO2_Value)
{
  uint16_t uiDAC_Voltage_Air, uiDAC_Voltage_Oxygen;

  if (uiFiO2_Value >= 21 & uiFiO2_Value < 23)
  {
    SentData_DAC(0x3FF, Air_Valve);
    SentData_DAC(0x000, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 23 & uiFiO2_Value < 28) // FiO2 = 25%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(0.2);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(2.6);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 28 & uiFiO2_Value < 33) //FiO2 = 30%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(0.2);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(1.2);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 33 & uiFiO2_Value < 38) //FiO2 = 35%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(1.6);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(1.2);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 38 & uiFiO2_Value < 43)
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(0.2); // FiO2 = 40%
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(0.6);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 43 & uiFiO2_Value < 48) //Fio2 = 45%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(0.8);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(1.8);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 48 & uiFiO2_Value < 53) //FiO2 = 50%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(0.6);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(1.0);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 53 & uiFiO2_Value < 58) //FiO2 = 55%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(1.8);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(2.4);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 58 & uiFiO2_Value < 63) // FiO2 = 60%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(2.4);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(2.4);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 63 & uiFiO2_Value < 68) // FiO2 = 65%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(2.2);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(1.8);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 68 & uiFiO2_Value < 73) // FiO2 = 70%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(1.6);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(1);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 73 & uiFiO2_Value < 78) //FiO2 = 75%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(2.6);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(1.4);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 78 & uiFiO2_Value < 83) //Fio2 = 80%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(2.2);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(0.8);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 83 & uiFiO2_Value < 88) //FiO2 = 85%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(3.6);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(1.0);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 88 & uiFiO2_Value < 93) //FiO2 = 90%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(2.8);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(0.4);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 93 & uiFiO2_Value < 97) //Fio2 = 95%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(3.4);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(0.2);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
  else if (uiFiO2_Value >= 97 & uiFiO2_Value <= 100) //FiO2 = 100%
  {
    uiDAC_Voltage_Air = Convert_Voltage_to_Hex(3.6);
    SentData_DAC(uiDAC_Voltage_Air, Air_Valve);
    uiDAC_Voltage_Oxygen = Convert_Voltage_to_Hex(0);
    SentData_DAC(uiDAC_Voltage_Oxygen, Oxygen_Valve);
  }
}



// End of File -------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
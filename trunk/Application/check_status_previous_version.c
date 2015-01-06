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

#include "check_status_previous_version.h"
#include "alarm_condition_previous_version.h"

//------------------------------------------------------------------------------
extern Profile SProfile;
extern uint8_t uiCurrent_Status;
extern uint8_t uiInitial_SpO2;
extern uint8_t uiuiPurpose_FiO2;
extern uint8_t uiTime_AlarmLevel;
extern uint8_t uiSD_Card_index;
extern uint8_t uiRespond_time;
extern float fFiO2_SDCard_buffer_sim[3];
extern uint8_t uiPurpose_FiO2;

uint8_t Drive_FiO2;
//------------------------------------------------------------------------------
uint8_t check_status_previous_version (uint8_t uiCurrent_oxygenSaturation)
{  
  /* Check Oxygen Saturation condition */
  if (uiCurrent_oxygenSaturation < SProfile.uiSpO2_Minimum)
  {
    // Current Oxygen Saturation less than Minimum Oxygen Saturation
    if ((uiCurrent_Status != Status_OxygenSat_Below_L1) & (uiCurrent_Status != Status_OxygenSat_Below_L2) & (uiCurrent_Status != Status_Alarm))
    {
      Alarm_Function(ALARM_DISABLE);
      uiCurrent_Status = Status_OxygenSat_Below_L1;
      uiPurpose_FiO2 = SProfile.uiPrefered_FiO2 + 15;
      if(uiPurpose_FiO2 > SProfile.uiFiO2_Maximum)
      {
        Drive_FiO2 = SProfile.uiFiO2_Maximum;
        FiO2_Range(Drive_FiO2);
      }
      else
      {
        Drive_FiO2 = uiPurpose_FiO2;
        FiO2_Range(Drive_FiO2);
      }
	      
      if(uiCurrent_Status != Status_Alarm)
      {
        Alarm_Function(ALARM_ENABLE);
        lcdString(1,5,"Status: Below ");
        lcdString(1,6,"Alarm Level 1");
      }
    }      
  }
  else if (uiCurrent_oxygenSaturation > SProfile.uiSpO2_Maximum)
  {
    // Current Oxygen Saturation more than maximum Oxygen Saturation
    if (uiCurrent_Status != Status_OxygenSat_Behigh_L1 & uiCurrent_Status != Status_OxygenSat_Behigh_L2 & uiCurrent_Status != Status_Alarm)
    {
      Alarm_Function(ALARM_DISABLE);
      uiCurrent_Status = Status_OxygenSat_Behigh_L1;
      uiPurpose_FiO2 = SProfile.uiPrefered_FiO2 - 15;
      if (uiPurpose_FiO2 < SProfile.uiFiO2_Minimum)
      {
        Drive_FiO2 = SProfile.uiFiO2_Minimum;
        FiO2_Range(Drive_FiO2);
      }
      else
      {
        Drive_FiO2 = uiPurpose_FiO2;
        FiO2_Range(Drive_FiO2);
      }
      Alarm_Function(ALARM_ENABLE);
      lcdString(1,5,"Status: Behigh");
      lcdString(1,6,"Alarm Level 1");
    }
  }
  else if (uiCurrent_oxygenSaturation - SProfile.uiSpO2_Minimum == 1)
  {
    if (uiCurrent_Status != Status_Normal)
    {
      uiCurrent_Status = Status_Normal;
      uiPurpose_FiO2 = SProfile.uiPrefered_FiO2 + 5;
      if(uiPurpose_FiO2 > SProfile.uiFiO2_Maximum)
      {
        Drive_FiO2 = SProfile.uiFiO2_Maximum;
        FiO2_Range(Drive_FiO2);
      }
      else
      {
        Drive_FiO2 = uiPurpose_FiO2;
        FiO2_Range(Drive_FiO2);
      }
      lcdString(1,5,"Status: Normal");
      lcdString(1,6,"              ");
      Alarm_Function(ALARM_DISABLE);  
    }
  }
  else if (SProfile.uiSpO2_Maximum - uiCurrent_oxygenSaturation == 1)
  {
    if (uiCurrent_Status!= Status_Normal)
    {
      uiCurrent_Status = Status_Normal;
      uiPurpose_FiO2 = SProfile.uiPrefered_FiO2 - 5;
      if(uiPurpose_FiO2 < SProfile.uiFiO2_Minimum)
      {
        Drive_FiO2 = SProfile.uiFiO2_Minimum;
        FiO2_Range(Drive_FiO2);
      }
      else
      {
        Drive_FiO2 = uiPurpose_FiO2;
        FiO2_Range(Drive_FiO2);
      }
      lcdString(1,5,"Status: Normal");
      lcdString(1,6,"              ");
      Alarm_Function(ALARM_DISABLE); 
    }
  }
  else if (uiCurrent_oxygenSaturation >= SProfile.uiSpO2_Minimum & uiCurrent_oxygenSaturation <= SProfile.uiSpO2_Maximum)
  {
    // Current Oxygen Saturaiton is between Maximum Oxygen Saturation and Minimum Oxygen Saturation
    if (uiCurrent_Status!= Status_Normal)
    {
      Drive_FiO2 = SProfile.uiPrefered_FiO2;
      FiO2_Range(Drive_FiO2);
      lcdString(1,5,"Status: Normal");
      lcdString(1,6,"              ");
      uiCurrent_Status = Status_Normal;
      Alarm_Function(ALARM_DISABLE); 
    }
  }

  return uiCurrent_Status;
}

// End of File ------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/


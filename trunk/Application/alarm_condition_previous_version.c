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
#include "check_status_previous_version.h"
#include "alarm_condition_previous_version.h"
//------------------------------------------------------------------------------
uint8_t uiTime_AlarmLevel;

extern Profile SProfile;
extern uint8_t uiCurrent_Status;
extern uint8_t uiTime_AlarmLevel1;
extern uint8_t uiPurpose_FiO2;
extern uint8_t Drive_FiO2;
//------------------------------------------------------------------------------
// Alarm Function --------------------------------------------------------------
/*
  Function : Alarm_Function
  Input : uint8_t Command
          Command : ALARM_ENABLRE, ALARM_DISABLE
  Return: None
  Description : If command is "ALARM_ENABLE", Timer 2 will enable.
                If command is "ALARM_DISABLE", Timer 2 will disable and reset Timer_AlarmLevel variable.
*/
void Alarm_Function(uint8_t Command)
{
  if (Command == ALARM_ENABLE)
  {
    TIM_Cmd(TIM2, ENABLE);
  }
  else if (Command == ALARM_DISABLE)
  {
    TIM_Cmd(TIM2, DISABLE);
    uiTime_AlarmLevel = 0;
  }
}
//------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void TIM2_IRQHandler(void)
{
  if (TIM_GetITStatus (TIM2, TIM_IT_Update) != RESET)
  {
    uiTime_AlarmLevel = uiTime_AlarmLevel + 1;
    STM_EVAL_LEDOff(LED5);
    if (uiCurrent_Status == Status_OxygenSat_Below_L1 | uiCurrent_Status == Status_OxygenSat_Behigh_L1)
    {
      if (uiTime_AlarmLevel >= SProfile.uiAlarm_Level1)
      {
        uiTime_AlarmLevel = 0;
        if (uiCurrent_Status == Status_OxygenSat_Below_L1)
        {
          uiCurrent_Status = Status_OxygenSat_Below_L2;
          uiPurpose_FiO2 = SProfile.uiPrefered_FiO2 + 25;
          if (uiPurpose_FiO2 > SProfile.uiFiO2_Maximum)
          {
            Drive_FiO2 = SProfile.uiFiO2_Maximum;
            FiO2_Range(Drive_FiO2);
          }
          else
          {
            Drive_FiO2 = uiPurpose_FiO2;
            FiO2_Range(Drive_FiO2);
          }
          lcdString(1,5,"Status: Below ");
          lcdString(1,6,"Alarm Level 2");
        }
        else if (uiCurrent_Status == Status_OxygenSat_Behigh_L1)
        {
          uiCurrent_Status = Status_OxygenSat_Behigh_L2;
          uiPurpose_FiO2 = SProfile.uiPrefered_FiO2 - 25;
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
          lcdString(1,5,"Status: Behigh");
          lcdString(1,6,"Alarm Level 2");
        }
      }
    }
    if (uiCurrent_Status == Status_OxygenSat_Below_L2 | uiCurrent_Status == Status_OxygenSat_Behigh_L2)
    {
      /* Alarm Level 2 */
      if (uiTime_AlarmLevel >= SProfile.uiAlarm_Level2)
      {
        GPIO_SetBits(Alarm_Set_GPIO_Port, Alarm_Set_Pin);
        uiCurrent_Status = Status_Alarm;
        USART_Cmd(OPM_USART, DISABLE);                                          // ENABLE Oxygen Pulse Meter USART
        TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
        TIM_Cmd(TIM3, DISABLE);
      
        //Time_AlarmLevel = 0;

        /* Notification Alarm Board (Toggle Pin to Alarm Circuit) */
        lcdClear();
        lcdUpdate();
        lcdString(3,1,"ALARM !!!");
        lcdString(2,2,"PLEASE PUSH");
        lcdString(2,3,"ALARM BUTTON");
        Alarm_Function(ALARM_DISABLE);
      }
    }
    TIM_ClearITPendingBit (TIM2, TIM_IT_Update);
  }
}

// End of File ------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : alarm_condition_v3.c

Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University

*/
//------------------------------------------------------------------------------
#include "main.h"
#include "Connect_GUI.h"
#include "Control_valve.h"
#include "alarm_condition_v3.h"
//------------------------------------------------------------------------------
extern Profile SProfile;
extern uint8_t uiCurrent_Status;
extern uint8_t uiPurpose_FiO2;
extern uint16_t uiResult_SpO2;
extern uint8_t uiInitial_SpO2;

uint8_t uiRespond_time;
uint8_t uiTime_AlarmLevel;
//------------------------------------------------------------------------------
// Alarm Function --------------------------------------------------------------
/*
  Function : alarm_timer
  Input : uint8_t Command
          Command : TIMER_ENABLRE, TIMER_DISABLE
  Return: None
  Description : If command is "TIMER_ENABLE", Timer 2 will enable.
                If command is "TIMER_DISABLE", Timer 2 will disable and reset Time_AlarmLevel variable.
*/

void alarm_timer(uint8_t Command)
{
  if (Command == TIMER_ENABLE)
  {
    TIM_Cmd(TIM2, ENABLE);
  }
  else if (Command == TIMER_DISABLE)
  {
    TIM_Cmd(TIM2, DISABLE);
    uiTime_AlarmLevel = 0;
    uiRespond_time = 0;
  }
}
//------------------------------------------------------------------------------
/*
  Function : TIM2_IRQHandler
  Input : None
  Return : None
  Description : Interrupt Service Routine of Timer for count Alarm Statu? of Profile
*/
//------------------------------------------------------------------------------
void TIM2_IRQHandler(void)
{
  if (TIM_GetITStatus (TIM2, TIM_IT_Update) != RESET)
  {
    uiTime_AlarmLevel = uiTime_AlarmLevel + 1;
    uiRespond_time = uiRespond_time + 1;
    STM_EVAL_LEDOff(LED5);

    if (uiCurrent_Status == STATUS_SpO2_BELOW_L1 | uiCurrent_Status == STATUS_SpO2_BEHIGH_L1)
    {
      if (uiTime_AlarmLevel > SProfile.uiAlarm_Level1)
      {
        uiTime_AlarmLevel = 0;                                                  // Reset Time_AlarmLevel
        
        /* If Time alarm more than alarm level 1 set */
        if (uiCurrent_Status == STATUS_SpO2_BELOW_L1)
        {
          uiCurrent_Status = STATUS_SpO2_BELOW_ALARM_L1;
          uiPurpose_FiO2 = uiPurpose_FiO2 + 6;
          
          if (uiPurpose_FiO2 > SProfile.uiFiO2_Maximum)
          {
            /* if uiPurpose_FiO2 more than uiFiO2_Maximum */
            uiPurpose_FiO2 = SProfile.uiFiO2_Maximum;
          }

          FiO2_Range(uiPurpose_FiO2);
          
          /* Update LCD */
          lcdString(1,5,"Status: Below ");
          lcdString(1,6,"Alarm Level 2");
        }
        else if (uiCurrent_Status == STATUS_SpO2_BEHIGH_L1)
        {
          uiCurrent_Status = STATUS_SpO2_BEHIGH_ALARM_L1;
          uiPurpose_FiO2 = uiPurpose_FiO2 - 6;

          if (uiPurpose_FiO2 < SProfile.uiFiO2_Minimum)
          {
            /* if uiPurpose_FiO2 more than uiFiO2_Minimum */
            uiPurpose_FiO2 = SProfile.uiFiO2_Minimum;
          }

          FiO2_Range(uiPurpose_FiO2);
          
          /* Update LCD */
          lcdString(1,5,"Status: Behigh ");
          lcdString(1,6,"Alarm Level 2");          
        }
      }
    }
    else if (uiCurrent_Status == STATUS_SpO2_BEHIGH_L2 | uiCurrent_Status == STATUS_SpO2_BELOW_L2)
    {
      /* Alarm Level 2 */
      if (uiTime_AlarmLevel >= SProfile.uiAlarm_Level2)
      {
        GPIO_SetBits(Alarm_Set_GPIO_Port, Alarm_Set_Pin);
        uiCurrent_Status = STATUS_ALARM;
        USART_Cmd(OPM_USART, ENABLE);                                          // ENABLE Oxygen Pulse Meter USART
        TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
        TIM_Cmd(TIM3, DISABLE);
      
//        NVIC_InitTypeDef   NVIC_InitStructure;
        
        /* Enable and set Alarm_Button_EXTI Line Interrupt to the lowest priority */
//        NVIC_InitStructure.NVIC_IRQChannel = Alarm_Button_IRQn;
//        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
//        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
//        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//        NVIC_Init(&NVIC_InitStructure);


        //Time_AlarmLevel = 0;

        /* Notification Alarm Board (Toggle Pin to Alarm Circuit) */
        lcdClear();
        lcdUpdate();
        lcdString(3,1,"ALARM !!!");
        lcdString(2,2,"PLEASE PUSH");
        lcdString(2,3,"ALARM BUTTON");
        alarm_timer(TIMER_DISABLE);
      }
    }
    else if ((uiCurrent_Status == STATUS_MIDDLE_SpO2_BELOW) | (uiCurrent_Status == STATUS_MIDDLE_SpO2_BEHIGH))
    {
      /* if time over than Responds time, FiO2 will increse or decrease 2 percent */
      uiTime_AlarmLevel = 0;                                                    // clear Time_AlarmLevel
        
      if(uiRespond_time >= SProfile.uiRespondsTime)
      {
        uiRespond_time = 0;                                                     // clear value of Respond time
        if (uiResult_SpO2 < SProfile.uiSpO2_middleRange)
        {
          uiPurpose_FiO2 = uiPurpose_FiO2 + 2;                                    // increase FiO2 more than present 2 percent
            
          if (uiPurpose_FiO2 > SProfile.uiFiO2_Maximum)
          {
            uiPurpose_FiO2 = SProfile.uiFiO2_Maximum;
          }
          uiInitial_SpO2 = uiResult_SpO2;
        }
        else if (uiResult_SpO2 > SProfile.uiSpO2_middleRange)
        {
          uiPurpose_FiO2 = uiPurpose_FiO2 - 2;                                    // decrease FiO2 more than present 2 percent
            
          if (uiPurpose_FiO2 < SProfile.uiFiO2_Minimum)
          {
            uiPurpose_FiO2 = SProfile.uiFiO2_Minimum;
          }
            
          uiInitial_SpO2 = uiResult_SpO2;
        }
      }
      
      uiInitial_SpO2 = uiResult_SpO2;
      FiO2_Range(uiPurpose_FiO2);
    }
  }
  TIM_ClearITPendingBit (TIM2, TIM_IT_Update);
}

//----------------- GPIO Interrupt Service Routine -----------------------------
//Button Down IRQHandler -------------------------------------------------------
//void Button_Down_IRQHandler(void)
//{
//  if(EXTI_GetITStatus(Button_Down_EXTI_Line) != RESET)
//  {
//    uiDrive_FiO2 = uiDrive_FiO2 - 5;
//    FiO2_Range(uiDrive_FiO2);
//    
//    delay_ms(80);
//    /* Clear the EXTI line pending bit */
//    EXTI_ClearITPendingBit(Button_Down_EXTI_Line);
//  }
//}


// Button Up IRQHandler --------------------------------------------------------
//void Button_Up_IRQHandler(void)
//{
//  if (EXTI_GetITStatus(Button_Up_EXTI_Line) != RESET)
//  {
//    uiDrive_FiO2 = uiDrive_FiO2 + 5;
//    FiO2_Range(uiDrive_FiO2);
//    
//    delay_ms(80);
//    /* Clear the EXTI line pending bit */
//    EXTI_ClearITPendingBit(Button_Up_EXTI_Line);
//  }
//}

// Run Button IRQHandler -------------------------------------------------------
void Run_Button_IRQHandler(void)
{
  if (EXTI_GetITStatus(Run_Button_EXTI_Line) != RESET)
  {
    if (SProfile.uiProfile_Status == PROFILE_SETTING_COMPLETE)
    {
      SProfile.uiProfile_Status = RUN_BUTTON_SET;
      USART_Cmd(OPM_USART, ENABLE); 
      TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
      TIM_Cmd(TIM3, ENABLE);
      FiO2_Range(SProfile.uiPrefered_FiO2);
    }
    else if (SProfile.uiProfile_Status == RUN_BUTTON_SET)
    {
      SProfile.uiProfile_Status = PROFILE_SETTING_COMPLETE;
      USART_Cmd(OPM_USART, ENABLE);    
      TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
      TIM_Cmd(TIM3, DISABLE);
      alarm_timer(TIMER_DISABLE);
      FiO2_Range(21);
      SentData_DAC(0, Oxygen_Valve);
      SentData_DAC(0, Air_Valve);
    }
    delay_ms(80);
    /* Clear the EXTI line pending bit */
    EXTI_ClearITPendingBit(Run_Button_EXTI_Line);
  }
}

// Alarm Button IRQHandler -----------------------------------------------------
//void Alarm_Button_IRQHandler(void)
//{
//  if (EXTI_GetITStatus(Alarm_Button_EXTI_Line) != RESET)
//  {
//    lcdClear();
//    lcdUpdate();
//    
//    Button_Up_Down_Init();                                                      // Config Button Up and down with Interrupt Vector
//    
//    delay_ms(60);
//    /* Clear the EXTI line pending bit */
//    EXTI_ClearITPendingBit(Alarm_Button_EXTI_Line);
//  }
//}

// End of File -----------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
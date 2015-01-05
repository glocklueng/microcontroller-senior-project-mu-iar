/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : check_status_profile.c
Deverloper : Phattaradanai Kiratiwudhikul
Reseach & Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "check_status_profile.h"
#include "DAC_LTC1661.h"
#include "Connect_GUI.h"
#include "Oxygen_Pulse_Meter.h"
#include "alarm_condition.h"
#include "Control_valve.h"
#include "GLCD5110.h"
//------------------------------------------------------------------------------
extern Profile SProfile;
extern uint8_t uiCurrent_Status;
extern uint8_t uiInitial_SpO2;
extern uint8_t uiPurpose_FiO2;
extern uint8_t uiTime_AlarmLevel;
extern uint8_t uiSD_Card_index;
extern uint8_t uiRespond_time;
extern float fFiO2_SDCard_buffer_sim[3];
//------------------------------------------------------------------------------
/* 
  Function : check_status
  Input : uint8_t uiCurrent_SpO2
  Output : uint8_t uiCurrent_Status
  Description : Check profile status from a value of current Oxygen Saturation.
*/
uint8_t check_status(uint8_t uiCurrent_SpO2)
{ 
  if ((uiCurrent_SpO2 < SProfile.uiSpO2_Minimum) & (uiCurrent_Status != STATUS_ALARM))
  {
    /*
      In case of current SpO2 is lower than Minimum SpO2
    */
    
    /* Current Oxygen Saturation less than Minimum Oxygen Saturation */
    if (uiCurrent_SpO2 < (SProfile.uiSpO2_Minimum - 3))
    {
      /* Status is STATUS_SpO2_BELOW_L2 */
      if (uiCurrent_Status != STATUS_SpO2_BELOW_L2)
      {        
        /* Update Status */
        uiCurrent_Status = STATUS_SpO2_BELOW_L2;
        
        uiInitial_SpO2 = uiCurrent_SpO2;                                        // save current SpO2 at initial
        
        /* Update Text on LCD*/
        lcdString(1,5,"Status: Below ");
        lcdString(1,6,"Alarm Level 2");

        uiPurpose_FiO2 = uiPurpose_FiO2 + 6;                                    // increse FiO2 more than present 6 percent
        uiTime_AlarmLevel = 0;
        uiRespond_time = 0;
        alarm_timer(TIMER_ENABLE);
      }
      
      /* Check Limit of FiO2 */
      if (uiPurpose_FiO2 > SProfile.uiFiO2_Maximum)
      {
        uiPurpose_FiO2 = SProfile.uiFiO2_Maximum;
      }
      
      FiO2_Range(uiPurpose_FiO2);                                               // control valve
    }
    else if ((uiCurrent_SpO2 >= (SProfile.uiSpO2_Minimum - 3)) & (uiCurrent_SpO2 < SProfile.uiSpO2_Minimum)) 
    {
      /* 
        In case of current SpO2 is betwwne (Minimum SpO2 - 3) and Minimum SpO2 
      */
      
      /* Status is STATUS_SpO2_BELOW_L1 */
      if ((uiCurrent_Status != STATUS_SpO2_BELOW_L1) & (uiCurrent_Status != STATUS_SpO2_BELOW_ALARM_L1))
      {
        /* Update Status */
        uiCurrent_Status = STATUS_SpO2_BELOW_L1;
        
        uiInitial_SpO2 = uiCurrent_SpO2;                                        // save current SpO2 at initial

        /* Updata Text on LCD*/
        lcdString(1,5,"Status: Below ");
        lcdString(1,6,"Alarm Level 1");
    
        uiPurpose_FiO2 = uiPurpose_FiO2 + 4;                                    // increase FiO2 more than present 4 percent
    
        uiTime_AlarmLevel = 0;
        uiRespond_time = 0;
        alarm_timer(TIMER_ENABLE);                                           
      }

      /* Check Limit of FiO2 */
      if (uiPurpose_FiO2 > SProfile.uiFiO2_Maximum)
      {
        uiPurpose_FiO2 = SProfile.uiFiO2_Maximum;
      }
      FiO2_Range(uiPurpose_FiO2);
    }
  }
  else if ((uiCurrent_SpO2 > SProfile.uiSpO2_Maximum) & (uiCurrent_Status != STATUS_ALARM))
  {
    /* 
      In case of current SpO2 have more than Maximum SpO2.
    */
    
    if (uiCurrent_SpO2 > (SProfile.uiSpO2_Maximum + 3))
    {
      /*
        In case of current SpO2 is higher than (Maximum SpO2 + 3).
      */
      
      /* Current Oxygen Saturation more than maximum Oxygen Saturation */
      if (uiCurrent_Status != STATUS_SpO2_BEHIGH_L2)
      {
        /* Update Status */
        uiCurrent_Status = STATUS_SpO2_BEHIGH_L2;
        
        uiInitial_SpO2 = uiCurrent_SpO2;                                        // save current SpO2 at initial
                
        /* Update Text on LCD*/
        lcdString(1,5,"Status: Behigh ");
        lcdString(1,6,"Alarm Level 2");

        uiPurpose_FiO2 = uiPurpose_FiO2 - 6;                                    // decrese FiO2 more than present 6 percent

        uiTime_AlarmLevel = 0;
        uiRespond_time = 0;
        alarm_timer(TIMER_ENABLE);
      }
      
      /* Check Limit of FiO2 */
      if (uiPurpose_FiO2 < SProfile.uiFiO2_Minimum)
      {
        uiPurpose_FiO2 = SProfile.uiFiO2_Minimum;
      }
      FiO2_Range(uiPurpose_FiO2);
    }
    else if ((uiCurrent_SpO2 <= (SProfile.uiSpO2_Maximum + 3)) & (uiCurrent_SpO2 > (SProfile.uiSpO2_Maximum)))
    {
      /*
        In case of current SpO2 is between Maximum SpO2 and (Maximum SpO2 + 3).
      */
      
      /* Status is STATUS_SpO2_BEHIGH_L1 */
      if ((uiCurrent_Status != STATUS_SpO2_BEHIGH_L1) & (uiCurrent_Status != STATUS_SpO2_BEHIGH_ALARM_L1))
      {
        /* Update Status */
        uiCurrent_Status = STATUS_SpO2_BEHIGH_L1;
        
        uiInitial_SpO2 = uiCurrent_SpO2;                                        // save current SpO2 at initial

        /* Updata Text on LCD*/
        lcdString(1,5,"Status: Behigh ");
        lcdString(1,6,"Alarm Level 1");

        uiPurpose_FiO2 = uiPurpose_FiO2 - 4;                                    // decrease FiO2 more than present 4 percent
          
        uiTime_AlarmLevel = 0;
        uiRespond_time = 0;
        alarm_timer(TIMER_ENABLE);
      }
      
      /* Check Limit of FiO2 */
      if (uiPurpose_FiO2 < SProfile.uiFiO2_Minimum)
      {
        uiPurpose_FiO2 = SProfile.uiFiO2_Minimum;
      }
      FiO2_Range(uiPurpose_FiO2);
    }
  }
  else if ((uiCurrent_SpO2 >= SProfile.uiSpO2_Minimum) & (uiCurrent_SpO2 <= SProfile.uiSpO2_Maximum))
  {
    /* Oxygen Saturaiton is Normal Range */
    lcdString(1,5,"Status: Normal ");
    lcdString(1,6,"               ");

    if (uiCurrent_SpO2 < SProfile.uiSpO2_middleRange)
    {
      if (uiCurrent_Status != STATUS_MIDDLE_SpO2_BELOW)
      {
        /* Update Status*/
        uiCurrent_Status = STATUS_MIDDLE_SpO2_BELOW;
                        
        uiPurpose_FiO2 = uiPurpose_FiO2 + 2;                                    // increase FiO2 more than present 2 percent
        
        if (uiPurpose_FiO2 > SProfile.uiFiO2_Maximum)
        {
          uiPurpose_FiO2 = SProfile.uiFiO2_Maximum;
        }

        uiTime_AlarmLevel = 0;
        uiRespond_time = 0;
        alarm_timer(TIMER_ENABLE);
      }

    }
    else if (uiCurrent_SpO2 > SProfile.uiSpO2_middleRange)
    {
      if (uiCurrent_Status != STATUS_MIDDLE_SpO2_BEHIGH)
      {
        /* Update Status */
        uiCurrent_Status = STATUS_MIDDLE_SpO2_BEHIGH;
        
        uiInitial_SpO2 = uiCurrent_SpO2;                                        // save current SpO2 at initial
                
        uiPurpose_FiO2 = uiPurpose_FiO2 - 2;                                    // decrease FiO2 more than present 2 percent
        
        if (uiPurpose_FiO2 < SProfile.uiFiO2_Minimum)
        {
          uiPurpose_FiO2 = SProfile.uiFiO2_Minimum;
        }

        uiTime_AlarmLevel = 0;
        alarm_timer(TIMER_ENABLE);
      }
    }
    else
    {
      /* Current SpO2 = SpO2 Middle Range */
      uiPurpose_FiO2 = uiPurpose_FiO2;
      uiCurrent_Status = STATUS_MIDDLE_SpO2;
      uiTime_AlarmLevel = 0;
      uiRespond_time = 0;
      uiInitial_SpO2 = uiCurrent_SpO2;
      alarm_timer(TIMER_DISABLE);
    }
    FiO2_Range(uiPurpose_FiO2);
  }
  fFiO2_SDCard_buffer_sim[uiSD_Card_index] = (float)(uiPurpose_FiO2);           // using in simulation
  
  return uiCurrent_Status;
}
// End of File -----------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
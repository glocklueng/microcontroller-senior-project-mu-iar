/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : check_status_profile_v3.c
Deverloper : Phattaradanai Kiratiwudhikul
Reseach & Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "check_status_profile_v3.h"
#include "DAC_LTC1661.h"
#include "Connect_GUI.h"
#include "Oxygen_Pulse_Meter.h"
#include "alarm_condition_v3.h"
#include "Control_valve.h"
#include "GLCD5110.h"
//------------------------------------------------------------------------------
/* Extern Variable*/
extern Profile SProfile;
extern uint8_t uiCurrent_Status;
extern uint8_t uiInitial_SpO2;
extern uint8_t uiPurpose_FiO2;
extern uint8_t uiTime_AlarmLevel;
extern uint8_t uiSD_Card_index;
extern uint8_t uiRespond_time;
extern float fFiO2_SDCard_buffer_sim[3];
extern bool bSpO2ReadComplete;
//------------------------------------------------------------------------------
#define SIZE_OF_WINDOWS_FRAME   30

/* Define Variable: use in this file ONLY*/
uint8_t uiSpO2_window_time[256];                                                // create buffer that can correct data up to 256 bytes
uint8_t uiIndex_window_time = 0;
uint8_t uiFiO2_SDCard_index;
uint16_t uiResult_SpO2;
//------------------------------------------------------------------------------
/* 
  Function : check_status
  Input : uint8_t uiCurrent_SpO2
  Output : uint8_t uiCurrent_status
  Description : Check profile status from a value of current Oxygen Saturation.
*/
uint8_t check_status(uint8_t uiCurrent_SpO2)
{
  uint8_t uiIndex_avg = 0;
  uint8_t uiCurrent_status;
  uint8_t uiSpO2_avg_decimal;
  float fSpO2_avg;
  
  if(bSpO2ReadComplete == true)
  {
    uiSpO2_window_time[uiIndex_window_time] = uiCurrent_SpO2;
    
//    fSpO2_avg = fSpO2_avg + uiSpO2_window_time[uiIndex_avg];
    
    uiIndex_window_time++;
  
    bSpO2ReadComplete = false;                                                  // extern form Oxygen_Pulse_Meter.c
  }
      
  if(uiIndex_window_time >= SProfile.uiRespondsTime)
  {
    uiIndex_window_time = 0;                                                    // result inIndex_window_time
    
    fSpO2_avg = 0;
    /* check preterm infants profile */
    
    /* Summation of SpO2 buffer */
    for(uiIndex_avg = 0; uiIndex_avg < SProfile.uiRespondsTime; uiIndex_avg++)
    {
      fSpO2_avg = fSpO2_avg + uiSpO2_window_time[uiIndex_avg];
    }
    
    /* avg of SpO2 buffer */
    fSpO2_avg = fSpO2_avg / SProfile.uiRespondsTime;
    
    uiResult_SpO2 = (uint16_t)(fSpO2_avg / 1);                                  // convert to unsinged int format for reduce memory
    
    /* if decimal of fSpO2_avg more than 5, increase uiResult_SpO2 1 percent */
    uiSpO2_avg_decimal = (((uint32_t)fSpO2_avg * 10) % 10);
    
    if(uiSpO2_avg_decimal >= 5)
    {
      uiResult_SpO2 = uiResult_SpO2 + 1;
      
      /* Set upper bound and lower bound*/
      if(uiResult_SpO2 > 100)
      {
        uiResult_SpO2 = 100;
      }
      else if (uiResult_SpO2 < 21)
      {
        uiResult_SpO2 = 21;
      }
    }
    //fSpO2_avg = 0;                                                              // clear value in fSpO2_avg
    uiCurrent_status = check_profile_rule_base(uiResult_SpO2);
  }
  //uiCurrent_status = check_profile_rule_base(uiResult_SpO2);
  
  return uiCurrent_status;
}
//------------------------------------------------------------------------------
/*
  Function : check_profile_rule_base
  Input : uint8_t uiCurrent_SpO2
  Output : uint8_t uiCurrent_Status
  Description : 
*/
static uint8_t check_profile_rule_base(uint8_t uiSpO2_check_profile)
{
  if ((uiSpO2_check_profile < SProfile.uiSpO2_Minimum) & (uiCurrent_Status != STATUS_ALARM))
  {
    /*
      In case of current SpO2 is lower than Minimum SpO2 and no alarm
    */
    
    /* Current Oxygen Saturation less than Minimum Oxygen Saturation */
    if (uiSpO2_check_profile < (SProfile.uiSpO2_Minimum - 3))
    {
      /* Status is STATUS_SpO2_BELOW_L2 */
      if (uiCurrent_Status != STATUS_SpO2_BELOW_L2)
      {        
        /* Update Status */
        uiCurrent_Status = STATUS_SpO2_BELOW_L2;
        
        uiInitial_SpO2 = uiSpO2_check_profile;                                  // save current SpO2 at initial
        
        /* Update Text on LCD*/
        lcdString(1,5,"Status: Below ");
        lcdString(1,6,"Alarm Level 2");

        uiPurpose_FiO2 = uiPurpose_FiO2 + 6;                                    // increse FiO2 more than present 6 percent
        uiTime_AlarmLevel = 0;
        uiRespond_time = 0;
        alarm_timer(TIMER_ENABLE);
      }
      
      /* Check Limit of FiO2 at uper bound of Maximum FiO2*/
      if (uiPurpose_FiO2 > SProfile.uiFiO2_Maximum)
      {
        uiPurpose_FiO2 = SProfile.uiFiO2_Maximum;
      }
      
      FiO2_Range(uiPurpose_FiO2);                                               // control valve
    }
    else if ((uiSpO2_check_profile >= (SProfile.uiSpO2_Minimum - 3)) & (uiSpO2_check_profile < SProfile.uiSpO2_Minimum)) 
    {
      /* 
        In case of current SpO2 is between (Minimum SpO2 - 3) and Minimum SpO2; (BELOW THAN MINIMUM SpO2).
      */
      
      /* Status is STATUS_SpO2_BELOW_L1 */
      if ((uiCurrent_Status != STATUS_SpO2_BELOW_L1) & (uiCurrent_Status != STATUS_SpO2_BELOW_ALARM_L1))
      {
        /* Update Status to STATUS_SpO2_BELOW_L1 */
        uiCurrent_Status = STATUS_SpO2_BELOW_L1;
        
        uiInitial_SpO2 = uiSpO2_check_profile;                                  // save current SpO2 at initial

        /* Updata Text on LCD*/
        lcdString(1,5,"Status: Below ");
        lcdString(1,6,"Alarm Level 1");
    
        uiPurpose_FiO2 = uiPurpose_FiO2 + 4;                                    // increase FiO2 more than present 4 percent
    
        uiTime_AlarmLevel = 0;
        uiRespond_time = 0;
        alarm_timer(TIMER_ENABLE);                                           
      }

      /* Check Limit of FiO2 at upper bound of Maximum of FiO2*/
      if (uiPurpose_FiO2 > SProfile.uiFiO2_Maximum)
      {
        uiPurpose_FiO2 = SProfile.uiFiO2_Maximum;
      }
      FiO2_Range(uiPurpose_FiO2);
    }
  }
  else if ((uiSpO2_check_profile > SProfile.uiSpO2_Maximum) & (uiCurrent_Status != STATUS_ALARM))
  {
    /* 
      In case of current SpO2 have more than Maximum SpO2.
    */
    
    if (uiSpO2_check_profile > (SProfile.uiSpO2_Maximum + 3))
    {
      /*
        In case of current SpO2 is higher than (Maximum SpO2 + 3).
      */
      
      /* Current Oxygen Saturation more than maximum Oxygen Saturation */
      if (uiCurrent_Status != STATUS_SpO2_BEHIGH_L2)
      {
        /* Update Status to STATUS_SpO2_BEHIGH_L2 */
        uiCurrent_Status = STATUS_SpO2_BEHIGH_L2;
        
        uiInitial_SpO2 = uiSpO2_check_profile;                                  // save current SpO2 at initial
                
        /* Update Text on LCD*/
        lcdString(1,5,"Status: Behigh ");
        lcdString(1,6,"Alarm Level 2");

        uiPurpose_FiO2 = uiPurpose_FiO2 - 6;                                    // decrese FiO2 more than present 6 percent

        uiTime_AlarmLevel = 0;
        uiRespond_time = 0;
        alarm_timer(TIMER_ENABLE);
      }
      
      /* Check Limit of FiO2 at lower bound of Minimum FiO2 */
      if (uiPurpose_FiO2 < SProfile.uiFiO2_Minimum)
      {
        uiPurpose_FiO2 = SProfile.uiFiO2_Minimum;
      }
      FiO2_Range(uiPurpose_FiO2);
    }
    else if ((uiSpO2_check_profile <= (SProfile.uiSpO2_Maximum + 3)) & (uiSpO2_check_profile > (SProfile.uiSpO2_Maximum)))
    {
      /*
        In case of current SpO2 is between Maximum SpO2 and (Maximum SpO2 + 3); Higher than MAXIMUM FiO2.
      */
      
      /* Status is STATUS_SpO2_BEHIGH_L1 */
      if ((uiCurrent_Status != STATUS_SpO2_BEHIGH_L1) & (uiCurrent_Status != STATUS_SpO2_BEHIGH_ALARM_L1))
      {
        /* Update Status to STATUS_SpO2_BEHIGH_L1 */
        uiCurrent_Status = STATUS_SpO2_BEHIGH_L1;
        
        uiInitial_SpO2 = uiSpO2_check_profile;                                  // save current SpO2 at initial

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
  else if ((uiSpO2_check_profile >= SProfile.uiSpO2_Minimum) & (uiSpO2_check_profile <= SProfile.uiSpO2_Maximum))
  {
    /* Oxygen Saturaiton is Normal Range */
    lcdString(1,5,"Status: Normal ");
    lcdString(1,6,"               ");

    if (uiSpO2_check_profile < SProfile.uiSpO2_middleRange)
    {
      /*
        In case of SpO2 is below than Middile range of SpO2 but higher than Minimum SpO2. The system will increase FiO2 2 percent.
      */
      if (uiCurrent_Status != STATUS_MIDDLE_SpO2_BELOW)
      {
        /* Update Status to STATUS_MIDDLE_SpO2_BELOW */
        uiCurrent_Status = STATUS_MIDDLE_SpO2_BELOW;
                        
        uiPurpose_FiO2 = uiPurpose_FiO2 + 2;                                    // increase FiO2 more than present 2 percent
        
        /* Check Limit of Maximum of FiO2 */
        if (uiPurpose_FiO2 > SProfile.uiFiO2_Maximum)
        {
          uiPurpose_FiO2 = SProfile.uiFiO2_Maximum;
        }

        uiTime_AlarmLevel = 0;
        uiRespond_time = 0;
        alarm_timer(TIMER_ENABLE);
      }

    }
    else if (uiSpO2_check_profile > SProfile.uiSpO2_middleRange)
    {
      /*
        In case of SpO2 is higher than Middle range of SpO2, but lower than Maximum of FiO2. The system will decrease FiO2 2 percent.
      */
      if (uiCurrent_Status != STATUS_MIDDLE_SpO2_BEHIGH)
      {
        /* Update Status to STATUS_MIDDLE_SpO2_BEHIGH */
        uiCurrent_Status = STATUS_MIDDLE_SpO2_BEHIGH;
        
        uiInitial_SpO2 = uiSpO2_check_profile;                                  // save current SpO2 at initial
                
        uiPurpose_FiO2 = uiPurpose_FiO2 - 2;                                    // decrease FiO2 more than present 2 percent
        
        /* check limit of FiO2 at Minimum of FiO2*/
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
      /* Current SpO2 is the Middle range of SpO2 */
      uiPurpose_FiO2 = uiPurpose_FiO2;
      uiCurrent_Status = STATUS_MIDDLE_SpO2;
      uiTime_AlarmLevel = 0;
      uiRespond_time = 0;
      uiInitial_SpO2 = uiSpO2_check_profile;
      alarm_timer(TIMER_DISABLE);
    }
    FiO2_Range(uiPurpose_FiO2);
  }
//  fFiO2_SDCard_buffer_sim[uiFiO2_SDCard_index] = (float)(uiPurpose_FiO2);      // using in simulation
//  uiFiO2_SDCard_index++;
  
  return uiCurrent_Status;
}


// End of File -----------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
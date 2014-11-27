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
extern uint8_t uiPurpose_FiO2;
extern uint8_t Time_AlarmLevel;
//------------------------------------------------------------------------------
/* Check Oxygen Saturation condition */
uint8_t check_status(uint8_t uiCurrent_SpO2)
{ 
  if ((uiCurrent_SpO2 < SProfile.uiSpO2_Minimum) & (uiCurrent_Status != STATUS_ALARM))
  {
	/* Current Oxygen Saturation less than Minimum Oxygen Saturation */
	if (uiCurrent_SpO2 < (SProfile.uiSpO2_Minimum - 3))
	{
		/* Status is STATUS_SpO2_BELOW_L2 */
		if (uiCurrent_Status != STATUS_SpO2_BELOW_L2)
		{
			/* Update Status */
			uiCurrent_Status = STATUS_SpO2_BELOW_L2;
			/* Update Text on LCD*/
			lcdString(1,5,"Status: Below ");
		lcdString(1,6,"Alarm Level 2");

		uiPurpose_FiO2 = uiPurpose_FiO2 + 6;                                        // increse FiO2 more than present 6 percent
		Time_AlarmLevel = 0;
		alarm_timer(TIMER_ENABLE);
		}
		
		/* Check Limit of FiO2 */
		if (uiPurpose_FiO2 > SProfile.uiFiO2_Maximum)
		{
			uiPurpose_FiO2 = SProfile.uiFiO2_Maximum;
		}
		FiO2_Range(uiPurpose_FiO2);
	}
	else if ((uiCurrent_SpO2 >= (SProfile.uiSpO2_Minimum - 3)) & (uiCurrent_SpO2 < SProfile.uiSpO2_Minimum)) 
	{
		/* Status is STATUS_SpO2_BELOW_L1 */
		if ((uiCurrent_Status != STATUS_SpO2_BELOW_L1) & (uiCurrent_Status != STATUS_SpO2_BELOW_ALARM_L1))
		{
			/* Update Status */
			uiCurrent_Status = STATUS_SpO2_BELOW_L1;

			/* Updata Text on LCD*/
			lcdString(1,5,"Status: Below ");
			lcdString(1,6,"Alarm Level 1");
	
			uiPurpose_FiO2 = uiPurpose_FiO2 + 4;                                      // increase FiO2 more than present 4 percent
	
			Time_AlarmLevel = 0;
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
	if (uiCurrent_SpO2 > (SProfile.uiSpO2_Maximum + 3))
	{
		/* Current Oxygen Saturation more than maximum Oxygen Saturation */
		if (uiCurrent_Status != STATUS_SpO2_BEHIGH_L2)
		{
			/* Update Status */
			uiCurrent_Status = STATUS_SpO2_BEHIGH_L2;
			/* Update Text on LCD*/
			lcdString(1,5,"Status: Behigh ");
			lcdString(1,6,"Alarm Level 2");

			uiPurpose_FiO2 = uiPurpose_FiO2 - 6;                                      // decrese FiO2 more than present 6 percent

			Time_AlarmLevel = 0;
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
		/* Status is STATUS_SpO2_BEHIGH_L1 */
		if ((uiCurrent_Status != STATUS_SpO2_BEHIGH_L1) & (uiCurrent_Status != STATUS_SpO2_BEHIGH_ALARM_L1))
		{
			/* Update Status */
			uiCurrent_Status = STATUS_SpO2_BEHIGH_L1;

			/* Updata Text on LCD*/
			lcdString(1,5,"Status: Behigh ");
			lcdString(1,6,"Alarm Level 1");

			uiPurpose_FiO2 = uiPurpose_FiO2 - 4;                                      // decrease FiO2 more than present 4 percent
			  
			Time_AlarmLevel = 0;
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

		Time_AlarmLevel = 0;
		alarm_timer(TIMER_ENABLE);
	  }

	}
	else if (uiCurrent_SpO2 > SProfile.uiSpO2_middleRange)
	{
	  if (uiCurrent_Status != STATUS_MIDDLE_SpO2_BEHIGH)
	  {
		/* Update Status */
		uiCurrent_Status = STATUS_MIDDLE_SpO2_BEHIGH;
		uiPurpose_FiO2 = uiPurpose_FiO2 - 2;                                    // decrease FiO2 more than present 2 percent
		
		if (uiPurpose_FiO2 < SProfile.uiFiO2_Minimum)
		{
		  uiPurpose_FiO2 = SProfile.uiFiO2_Minimum;
		}

		Time_AlarmLevel = 0;
		alarm_timer(TIMER_ENABLE);
	  }
	}
	else
	{
		/* Current SpO2 = SpO2 Middle Range */
	  uiPurpose_FiO2 = uiPurpose_FiO2;
		uiCurrent_Status = STATUS_MIDDLE_SpO2;
	  Time_AlarmLevel = 0;
	  alarm_timer(TIMER_DISABLE);
	}
	FiO2_Range(uiPurpose_FiO2);
  }
  
  return uiCurrent_Status;
}
// End of File -----------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
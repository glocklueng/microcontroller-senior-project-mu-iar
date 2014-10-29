/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : testControlValve.c

Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "Oxygen_sensor.h"
#include "MCP3202.h"
#include "GLCD5110.h"
#include "DAC_LTC1661.h"
#include "testControlValve.h"
//------------------------------------------------------------------------------
float FiO2_DataTest[24];
extern uint16_t time;

//------------------------------------------------------------------------------
void TestControlValve (void)
{
  // Flow Rate Variable
  float OxygenFlow, AirFlow;
  float OxygenFlow_SLM, AirFlow_SLM;
  char OxygenFlow_Text[24];
  char AirFlow_Text[27];
  float AVG_FiO2;
  float FiO2_Test_Buffer[50];
  
  uint16_t Air_Drive, Oxygen_Drive;
  uint8_t count = 0, adc_time,index;
  float current_FiO2[5];
  float FiO2_P;
  char FiO2_Percent_Ch_TEST[16];
  
  lcdClear();
  lcdUpdate();
  lcdString(1,1,"Test Control Valve");
  Air_Drive = 0x0000;                                                           // 0x0000 = 0 (0V)
  Oxygen_Drive = 0x0000;                                                        // 0x0000 = 0 (0V)
  count = 0;
  time = 0;
  char count_text[4];
  SentData_DAC(Air_Drive, Air_Valve);
  SentData_DAC(Oxygen_Drive, Oxygen_Valve);
  TIM_Cmd(TIM6, ENABLE);
  
  for (uint8_t O2_voltage = 0; O2_voltage <= 50; O2_voltage = O2_voltage + 2)
  {
    Air_Drive = 0;
    for (uint8_t Air_voltage = 0; Air_voltage <= 50; Air_voltage = Air_voltage + 2)
    {
      // ADC every 15 Seconds.
      while(time <= 15)
      {  
        if(TIM_GetFlagStatus(TIM6, TIM_FLAG_Update) != RESET)
        {
          if (time >= 15)
          {
            count = count + 1;
            //Sampling voltage 5 times
            for(adc_time = 0; adc_time < 5 ; adc_time++)                                             
            {   
              current_FiO2[adc_time] = '\0';
              current_FiO2[adc_time] = Oxygen_convert();
              
              /* Sampling until 5 samples*/
              if(adc_time == 4)
              {
                /* Show Number of data line*/
                count_text[0] = '0'+((uint32_t)count/100);
                count_text[1] = '0'+((uint32_t)count%100)/10;
                count_text[2] = '0'+((uint32_t)count%10)/1;
                count_text[3] = ' ';
                // import crate function send data via USART
//                textTransmission_USART (count_text);
                for(index = 0 ; index < 4 ; index++)
                {
                  while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
                  USART_SendData(USART3, count_text[index]); 
                  while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
                }
                
                AVG_FiO2 = ((current_FiO2[0] + current_FiO2[1] + current_FiO2[2] + current_FiO2[3] + current_FiO2[4])/5);
                FiO2_Test_Buffer[count] = AVG_FiO2;
                  
                FiO2_DataTest[count] = Convert_FiO2(AVG_FiO2);
                FiO2_P = Convert_FiO2(AVG_FiO2);
                FiO2_LCD_Display (FiO2_P);
            
                FiO2_Percent_Ch_TEST[0] = '0'+((uint32_t)FiO2_P/100);
                FiO2_Percent_Ch_TEST[1] = '0'+((uint32_t)FiO2_P%100)/10;
                FiO2_Percent_Ch_TEST[2] = '0'+((uint32_t)FiO2_P%10)/1;
                FiO2_Percent_Ch_TEST[3] = '.';
                FiO2_Percent_Ch_TEST[4] = '0'+((uint32_t)((FiO2_P)*10.0))%10;
                FiO2_Percent_Ch_TEST[5] = '%';
                FiO2_Percent_Ch_TEST[6] = ' ';
                FiO2_Percent_Ch_TEST[7] = '0'+((uint32_t)AVG_FiO2%10)/1;
                FiO2_Percent_Ch_TEST[8] = '.';
                FiO2_Percent_Ch_TEST[9] = '0'+((uint32_t)((AVG_FiO2)*10.0))%10;
                FiO2_Percent_Ch_TEST[10] = '0'+((uint32_t)((AVG_FiO2)*100.0))%10;
                FiO2_Percent_Ch_TEST[11] = '0'+((uint32_t)((AVG_FiO2)*1000.0))%10;
                FiO2_Percent_Ch_TEST[12] = '0'+((uint32_t)((AVG_FiO2)*10000.0))%10;
                FiO2_Percent_Ch_TEST[13] = 'V';            
                FiO2_Percent_Ch_TEST[14] = ' ';
                FiO2_Percent_Ch_TEST[15] = ' ';
            
                OxygenFlow = Get_FlowRate(OxygenFlowRate);
                OxygenFlow_SLM = (OxygenFlow*4);                                    // Convert Oxygen Flow Rate in SLM unit
            
                OxygenFlow_Text[0] = 'O';
                OxygenFlow_Text[1] = '2';
                OxygenFlow_Text[2] = 'F';
                OxygenFlow_Text[3] = 'l';
                OxygenFlow_Text[4] = 'o';
                OxygenFlow_Text[5] = 'w';
                OxygenFlow_Text[6] = ' ';
                OxygenFlow_Text[7] = '0'+((uint32_t)OxygenFlow_SLM/10);
                OxygenFlow_Text[8] = '0'+((uint32_t)OxygenFlow_SLM%10)/1;
                OxygenFlow_Text[9] = '.';
                OxygenFlow_Text[10] = '0'+((uint32_t)(OxygenFlow_SLM*10.0)%10);
                OxygenFlow_Text[11] = 'S';
                OxygenFlow_Text[12] = 'L';
                OxygenFlow_Text[13] = 'M';
                OxygenFlow_Text[14] = ' ';
                OxygenFlow_Text[15] = '0'+((uint32_t)OxygenFlow%10);
                OxygenFlow_Text[16] = '.';
                OxygenFlow_Text[17] = '0'+((uint32_t)(OxygenFlow*10.0)%10);
                OxygenFlow_Text[18] = '0'+((uint32_t)(OxygenFlow*100.0)%10);
                OxygenFlow_Text[19] = '0'+((uint32_t)(OxygenFlow*1000.0)%10);
                OxygenFlow_Text[20] = '0'+((uint32_t)(OxygenFlow*10000.0)%10);
                OxygenFlow_Text[21] = 'V';
                OxygenFlow_Text[22] = ' ';
                OxygenFlow_Text[23] = ' ';
            
                AirFlow = Get_FlowRate(AirFlowRate);
                AirFlow_SLM = (AirFlow*4);                                          // Convert Air Flow Rate in SLM unit
                AirFlow_Text[0] = 'A';
                AirFlow_Text[1] = 'i';
                AirFlow_Text[2] = 'r';
                AirFlow_Text[3] = 'F';
                AirFlow_Text[4] = 'l';
                AirFlow_Text[5] = 'o';
                AirFlow_Text[6] = 'w';
                AirFlow_Text[7] = ' ';
                AirFlow_Text[8] = '0'+((uint32_t)AirFlow_SLM/10);
                AirFlow_Text[9] = '0'+((uint32_t)AirFlow_SLM%10)/1;
                AirFlow_Text[10] = '.';
                AirFlow_Text[11] = '0'+((uint32_t)(AirFlow_SLM*10.0)%10);
                AirFlow_Text[12] = 'S';
                AirFlow_Text[13] = 'L';
                AirFlow_Text[14] = 'M';
                AirFlow_Text[15] = ' ';
                AirFlow_Text[16] = '0'+((uint32_t)AirFlow%10);
                AirFlow_Text[17] = '.';
                AirFlow_Text[18] = '0'+((uint32_t)(AirFlow*10.0)%10);
                AirFlow_Text[19] = '0'+((uint32_t)(AirFlow*100.0)%10);
                AirFlow_Text[20] = '0'+((uint32_t)(AirFlow*1000.0)%10);
                AirFlow_Text[21] = '0'+((uint32_t)(AirFlow*10000.0)%10);
                AirFlow_Text[22] = 'V';
                AirFlow_Text[23] = ' ';
                AirFlow_Text[24] = ' ';
                AirFlow_Text[25] = '\n';
                AirFlow_Text[26] = '\r';
            
//                textTransmission_USART(FiO2_Percent_Ch_TEST);
                for(index = 0 ; index < 16 ; index++)
                {
                  while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
                  USART_SendData(USART3, FiO2_Percent_Ch_TEST[index]); 
                  while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
                }

//                textTransmission_USART(OxygenFlow_Text);
                for (index = 0; index < 24; index++)
                {
                  while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
                  USART_SendData(USART3, OxygenFlow_Text[index]); 
                  while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
                }
//                textTransmission_USART(AirFlow_Text);
                for (index = 0; index < 27; index++)
                {
                  while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
                  USART_SendData(USART3, AirFlow_Text[index]); 
                  while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
                }
              }
            }
          }
          time = time + 1;
          TIM_ClearFlag(TIM6, TIM_FLAG_Update);
        }
      }
      time = 0;
    
      Air_Drive = Air_Drive + 0x0028;                                           // add 0x0028 = 0.2 V
      SentData_DAC(Air_Drive, Air_Valve);
      SentData_DAC(Oxygen_Drive, Oxygen_Valve);
    }
    Oxygen_Drive = Oxygen_Drive + 0x0028;                                       // add 0x0028 = 0.2 V
    SentData_DAC(Oxygen_Drive, Oxygen_Valve);
  }
  //SProfile.uiProfile_Status = TEST_COMPLETE;
  TIM_Cmd(TIM6, DISABLE);
}

//-------------------------------------------------------------------------------
void textTransmission_USART(char cTextString[])
{
  uint16_t uiIndex;
  for (uiIndex = 0; uiIndex < sizeof(cTextString); uiIndex++)
  {
    while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    USART_SendData(USART3, cTextString[uiIndex]);
    while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
  }
}
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Oxygen_sensor.c

Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/

//------------------------------------------------------------------------------
#include "main.h"
#include "Oxygen_sensor.h"
#include "Connect_GUI.h"
#include "GLCD5110.h"
#include "DAC_LTC1661.h"
#include "testControlValve.h"
#include "MCP3202.h"
//------------------------------------------------------------------------------
// Define Variable -------------------------------------------------------------
volatile uint16_t  time = 0;
float fFiO2_Upper, fFiO2_Lower;
float fFiO2_Percent;
float fADC_Voltage;

float fFiO2_Buffer[10];
float fFiO2[30];

uint16_t uiADC_Value;

//extern struct Profile SProfile;
// Function --------------------------------------------------------------------
/*
  Funciton : OxygenSensor_Config
  @ Input : None
  @ Return : None
  Description : Configuration ADC for FiO2, Resolution 10-bits
*/
void OxygenSensor_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  ADC_InitTypeDef ADC_InitStruct;
  ADC_CommonInitTypeDef ADC_CommonInitStruct;
  /*
    use OxygenSensor_IN9  Port B pin PB1
  */
  RCC_APB2PeriphClockCmd(OxygenSensor_ADC_CLK, ENABLE); 
  RCC_AHB1PeriphClockCmd(OxygenSensor_Pin_CLK, ENABLE); 
	
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = OxygenSensor_Pin;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(OxygenSensor_Port, &GPIO_InitStruct);
	
  /*ADC_Initialize */
  /* Initialize the ADC_Mode member */
  ADC_InitStruct.ADC_Resolution = ADC_Resolution_10b;
  /* initialize the ADC_ScanConvMode member */
  ADC_InitStruct.ADC_ScanConvMode = DISABLE;
  /* Initialize the ADC_ContinuousConvMode member */
  ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
  /* Initialize the ADC_ExternalTrigConvEdge member */
  ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  /* Initialize the ADC_ExternalTrigConv member */
  ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  /* Initialize the ADC_DataAlign member */
  ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
  /* Initialize the ADC_NbrOfConversion member */
  ADC_InitStruct.ADC_NbrOfConversion = 1;
  ADC_Init(OxygenSensor,&ADC_InitStruct);
	
  /*ADC_CommonStructInit*/
   /* Initialize the ADC_Mode member */
  ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;
  /* initialize the ADC_Prescaler member */
  ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div2;
  /* Initialize the ADC_DMAAccessMode member */
  ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  /* Initialize the ADC_TwoSamplingDelay member */
  ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStruct);
  
  ADC_RegularChannelConfig(OxygenSensor, ADC_Channel_3, 1,ADC_SampleTime_28Cycles);
  
  //Enable OxygenSensor
  ADC_Cmd(OxygenSensor, ENABLE);
}
//------------------------------------------------------------------------------
/*
  Function : Timer6_SetUp
  Input : None
  Output : None
  Description : Set timer for count time measure oxygen sensor (use timer 6, 16 bits, auto-reload, every 100 ms)
                Use Interrupt Timer6
*/
void Timer6_SetUp(void)
{
  /*Timer Interrupt*/
  /* Set interrupt: NVIC_Setup */
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  /* Enable the TIM6 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
   
  /* TIM2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 2000;                                       // 1 MHz down to 1 KHz (1 ms)
  TIM_TimeBaseStructure.TIM_Prescaler = 42000;                                  // 24 MHz Clock down to 1 MHz (adjust per your clock)
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
  /* TIM IT enable */
  TIM_ITConfig(TIM6, TIM_IT_Update, DISABLE);
  /* TIM2 disable counter */
  TIM_Cmd(TIM6, DISABLE);
}

// FiO2 Check Timer Config -----------------------------------------------------
/*
  Function: FiO2_Check_Timer
  @ Input : None
  @ Return : None
  Description: Timer 3 will get Analog to Digital Convertor of FiO2 every 1 sec.
*/
void FiO2_Check_Timer_Config(void)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the TIM3 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
   
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 2000;                                       // 1 MHz down to 1 KHz (1 ms)
  TIM_TimeBaseStructure.TIM_Prescaler = 42000;                                  // 24 MHz Clock down to 1 MHz (adjust per your clock)
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  /* TIM IT enable */
  TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
  /* TIM3 enable counter */
  TIM_Cmd(TIM3, DISABLE);
}

//------------------------------------------------------------------------------
/*
  Function : timer7_setup
  Input : None
  Output : None
  Description : timer 7 use for ADC every 10 ms in 1 sec (Sampling rate = 10 Hz)
*/
void timer7_setup (void)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the TIM7 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
   
  /* TIM9 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 200;                                        // 1 MHz down to 1 KHz (1 ms)
  TIM_TimeBaseStructure.TIM_Prescaler = 42000;                                  // 24 MHz Clock down to 1 MHz (adjust per your clock)
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
  /* TIM IT enable */
  TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
  /* TIM7 enable counter */
  TIM_Cmd(TIM7, DISABLE);
}

//------------------------------------------------------------------------------
/*
  Function: Oxygen_convert
  Input : None
  Return: float fADC_Voltage
  Description: Start ADC Voltage form Oxygen Sensor and calculate Hexdicimal to Voltage_Valve
*/
float Oxygen_convert(void)
{
  ADC_SoftwareStartConv(OxygenSensor);
  while(ADC_GetFlagStatus(OxygenSensor, ADC_FLAG_EOC) == RESET);
  /*Store ADC Sample*/
  uiADC_Value = ADC_GetConversionValue(OxygenSensor);
  
  fADC_Voltage = '\0';
  fADC_Voltage = (uiADC_Value*2.91)/1024;                                       //VDD is vary (time1 = 3.02, time2 = 2.94)

  return fADC_Voltage;
}

//------------------------------------------------------------------------------
/*
Function : Calibrate_OxygenSensor
Input : None
Output : None
Description : This function use Calibrate the new Oxygen Sensor
*/
void Calibrate_OxygenSensor(void)
{
  float fFiO2_PureOxygen[60], fFiO2_PureAir[60];
  //Updata LCD
  lcdClear();
  lcdString(3,1,"Waitting....");
  lcdString(3,3,"Set  Up...");
  lcdString(1,4,"Oxygen Sensor");

  //LED 6 is off
  STM_EVAL_LEDOff(LED6);
  
  // Calibrate Oxygen Concentration (Pure Oxygen)
  SentData_DAC(0x03FF, 1);
  SentData_DAC(0x0000, 2);
  time = 0;
  
  TIM_Cmd(TIM6, ENABLE);

  while(time <= 30)                                                             
  {
    if(TIM_GetFlagStatus(TIM6, TIM_FLAG_Update) != RESET)
    {
      fFiO2_PureOxygen[time] = Oxygen_convert();
      fFiO2_Percent = Convert_FiO2(fFiO2_PureOxygen[time]);
      time = time + 1;
      TIM_ClearFlag(TIM6, TIM_FLAG_Update);
    }
  }
  TIM_Cmd(TIM6, DISABLE);
  
  //Average PureAir Voltage Data
  fFiO2_Upper = (fFiO2_PureOxygen[25] + fFiO2_PureOxygen[26] +fFiO2_PureOxygen[27] +fFiO2_PureOxygen[28] +fFiO2_PureOxygen[29])/5;
  
  //Calibrate Oxygen Concentration (Pure Air)
  time = 0;
  SentData_DAC(0x0000, 1);
  SentData_DAC(0x03FF, 2);
  
  TIM_Cmd(TIM6, ENABLE);
  while(time <= 60)
  {  
    if(TIM_GetFlagStatus(TIM6, TIM_FLAG_Update) != RESET)
    {      
      fFiO2_PureAir[time] = Oxygen_convert();
      fFiO2_Percent = Convert_FiO2(fFiO2_PureAir[time]);
      time = time + 1;
      TIM_ClearFlag(TIM6, TIM_FLAG_Update);
    }
  } 
  TIM_Cmd(TIM6, DISABLE);
  time = 0;
  
  //Average PureAir Voltage Data
  fFiO2_Lower = (fFiO2_PureAir[55] + fFiO2_PureAir[56] +fFiO2_PureAir[57] +fFiO2_PureAir[58] +fFiO2_PureAir[59])/5;
  
  //Close Air and Oxygen Valve
  SentData_DAC(0x0000, 3);
  
  //LED 6 is on
  STM_EVAL_LEDOn(LED6);
  lcdClear();
  lcdUpdate();
}

//------------------------------------------------------------------------------
/*
  Function : testOxygenSensor
  Input : None
  Output : None
  Description : 

*/
void testOxygenSensor (void)
{
  //float fFiO2_Buffer[10];                                                       // Buffer for store FiO2 every 100 ms
  //float fFiO2[30];
  float fFiO2_avg;
  uint8_t index;

  //LED 6 is off
  STM_EVAL_LEDOff(LED6);
  
  // Calibrate Oxygen Concentration (Pure Oxygen)
  SentData_DAC(0x03FF, 1);
  SentData_DAC(0x0000, 2);
  time = 0;
  
  TIM_Cmd(TIM6, ENABLE);

  while(time < 30)                                                             
  {
    if(TIM_GetFlagStatus(TIM6, TIM_FLAG_Update) != RESET)
    {
      TIM_Cmd(TIM7, ENABLE);
      while(index < 10)
      {
        fFiO2_avg = 0;
        // Set ADC every 10 ms
        if(TIM_GetFlagStatus(TIM7, TIM_FLAG_Update) != RESET)
        {
          fFiO2_Buffer[index] = Oxygen_convert();
          fFiO2_Percent = Convert_FiO2(fFiO2_Buffer[index]);
          index++;
          TIM_ClearFlag(TIM7, TIM_FLAG_Update);
        }
      }
      index = 0;
      fFiO2_avg = 0;
      for(uint8_t index_buffer = 0; index_buffer < 10; index_buffer++)
      {
        fFiO2_avg = fFiO2_avg + fFiO2_Buffer[index_buffer];
      }
      fFiO2_avg = fFiO2_avg/10.0;
      
      for(uint8_t index_buffer = 0; index_buffer < 10; index_buffer++)
      {
        fFiO2_Buffer[index_buffer] = 0;
      }
      
      fFiO2[time] = fFiO2_avg;
      TIM_Cmd(TIM7, DISABLE);
      time = time + 1;
      TIM_ClearFlag(TIM6, TIM_FLAG_Update);
    }
  }
  TIM_Cmd(TIM6, DISABLE);
  
  //Average PureAir Voltage Data
  fFiO2_Upper = (fFiO2[25] + fFiO2[26] + fFiO2[27] + fFiO2[28] + fFiO2[29])/5.0;
  
  //Close Air and Oxygen Valve
  SentData_DAC(0x0000, 3);
  
  //LED 6 is on
  STM_EVAL_LEDOn(LED6);
  lcdClear();
  lcdUpdate();
}
//------------------------------------------------------------------------------
/* Interrupt Push Botton User (Blue Botton) */ 
void EXTI0_IRQHandler(void)
{
  if (EXTI_GetFlagStatus(EXTI_Line0) == SET)
  {
    STM_EVAL_LEDOff(LED5);
    USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
    //TestControlValve();
    //Calibrate_OxygenSensor();
    testOxygenSensor();
    STM_EVAL_LEDOn(LED5);
  }
  
  // Clear Flag Interrupt
  EXTI_ClearITPendingBit(EXTI_Line0);
}
//------------------------------------------------------------------------------
/* Timer 6 Interrupt Sevice Rountine */
void TIM6_DAC_IRQHandler(void)
{
  if (TIM_GetITStatus (TIM6, TIM_IT_Update) != RESET)
  {
    time = time + 1;
    //fFiO2_PureOxygen[time] = Oxygen_convert();
    STM_EVAL_LEDOff(LED5);
  }
  TIM_ClearITPendingBit (TIM6, TIM_IT_Update);
}

//------------------------------------------------------------------------------
/*
  Function: Convert_FiO2
  Input: float FiO2_ADC
  return: float fFiO2_Percent
  Description: Convert Voltage of FiO2 to Percent of FiO2 and Show on LCD Display
*/
float Convert_FiO2 (float fFiO2_ADC)
{
  float fFiO2_mv;
  fFiO2_mv = ((fFiO2_ADC) - 0.983) / 26.53;  //26.17
  fFiO2_Percent = fFiO2_mv * 21 / 0.0115;

  return fFiO2_Percent;
}

//------------------------------------------------------------------------------
/*
  @Function: FiO2_LCD_Display
  @Input: float FiO2_Current_Percent
  @Retrurn: None
  @Description: convert float to string for showing on LCD display
*/
void FiO2_LCD_Display (float fFiO2_Current_Percent)
{
  char cFiO2_Percent_Ch[7];
  cFiO2_Percent_Ch[0] = '0'+((uint32_t)fFiO2_Current_Percent/100);
  cFiO2_Percent_Ch[1] = '0'+((uint32_t)fFiO2_Current_Percent%100)/10;
  cFiO2_Percent_Ch[2] = '0'+((uint32_t)fFiO2_Current_Percent%10)/1;
  cFiO2_Percent_Ch[3] = '.';
  cFiO2_Percent_Ch[4] = '0'+((uint32_t)((fFiO2_Current_Percent)*10.0))%10;
  cFiO2_Percent_Ch[5] = '%';
  cFiO2_Percent_Ch[6] = '\0';

  lcdString(1,3,"FiO2: ");
  lcdString(7,3,cFiO2_Percent_Ch);
}
//--------------------------------------------------------------------------------


/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
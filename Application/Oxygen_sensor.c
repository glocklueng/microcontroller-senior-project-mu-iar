/*
Project : Programmable Feedback Control of Airflow System for Pre-term infant oxygen saturation
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Oxygen_sensor.c
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "Oxygen_sensor.h"
#include "DAC_LTC1661.h"
//------------------------------------------------------------------------------
// Define Variable -------------------------------------------------------------
uint16_t ADC_Value, ADC_V;
uint16_t time = 0;
uint16_t ADC_Voltage;
double ADC_fValue;
float FiO2_PureOxygen[60], FiO2_PureAir[60];
float FiO2_Upper, FiO2_Lower;
// Function --------------------------------------------------------------------
void OxygenSensor_Setup(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  ADC_InitTypeDef ADC_InitStruct;
  ADC_CommonInitTypeDef ADC_CommonInitStruct;
  /*
    use ADC1_IN9  Port B pin PB1
  */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); 
	
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_1;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
	
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
  ADC_Init(ADC1,&ADC_InitStruct);
	
  //Enable ADC1
  ADC_Cmd(ADC1, ENABLE);
  
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
  
  ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1,ADC_SampleTime_28Cycles);
}

//------------------------------------------------------------------------------
void Timer6_SetUp(void)
{
  /*
    Set timer for count time measure oxygen sensor
    use timer 6, 16 bits, auto-reload, every 1 minite 
  */
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);                          //Timer 6 use APB1
  
  /* Set the default configuration */
  TIM_TimeBaseInitStruct.TIM_Period = 2000;
  TIM_TimeBaseInitStruct.TIM_Prescaler = 42000;
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0x0000;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStruct);
  
  /*Timer Interrupt*/
  /* Set interrupt: NVIC_Setup */
  NVIC_InitTypeDef NVIC_InitStruct;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  //ENABLE TIM6 Interruper
  NVIC_InitStruct.NVIC_IRQChannel = TIM6_DAC_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannel = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
  
  //Set condition interrupt
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE); 
  
  //Enable Timer6
  TIM_Cmd(TIM6, ENABLE);
}

//------------------------------------------------------------------------------
float Oxygen_convert(void)
{
  ADC_SoftwareStartConv(ADC1);
  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
  /*Store ADC Sample*/
  ADC_Value = ADC_GetConversionValue(ADC1);
  
  ADC_Voltage = (ADC_Value*2940)/1023;                                          //VDD is vary (time1 = 3.02, time2 = 2.94)
  
  ADC_V = ((float)(ADC_Value*2.94))/1023;
  
  //Convert int to float
  ADC_fValue = ADC_Voltage/1000;
  ADC_fValue = ADC_fValue + (float)((((uint32_t)ADC_Voltage % 1000)/100) * 0.1);
  ADC_fValue = ADC_fValue + (float)((((uint32_t)ADC_Voltage % 100)/10) * 0.01);
  ADC_fValue = ADC_fValue + (float)(((uint32_t)ADC_Voltage % 10/1) * 0.001);
  
  return ADC_Voltage;
}

//------------------------------------------------------------------------------
void Calibrate_OxygenSensor(void)
{
  // Calibrate Oxygen Concentration (Pure Oxygen)
  SentData_DAC(0x03FF, 1);
  SentData_DAC(0x0000, 2);
  time = 0;

  while(time <= 60)                                                             
  {
    TIM_Cmd(TIM6, ENABLE);
    if(TIM_GetFlagStatus(TIM6, TIM_FLAG_Update) != RESET)
    {
      FiO2_PureOxygen[time] = Oxygen_convert();
      time = time + 1;
      TIM_ClearFlag(TIM6, TIM_FLAG_Update);
    }
  }
  TIM_Cmd(TIM6, DISABLE);
  FiO2_Upper = (FiO2_PureOxygen[55] + FiO2_PureOxygen[56] +FiO2_PureOxygen[57] +FiO2_PureOxygen[58] +FiO2_PureOxygen[59])/5;
  
  //Calibrate Oxygen Concentration (Pure Air)
  time = 0;
  SentData_DAC(0x0000, 1);
  SentData_DAC(0x03FF, 2);
  
  while(time <= 60)
  {  
    TIM_Cmd(TIM6, ENABLE);
    if(TIM_GetFlagStatus(TIM6, TIM_FLAG_Update) != RESET)
    {      
      FiO2_PureAir[time] = Oxygen_convert();
      time = time + 1;
      TIM_ClearFlag(TIM6, TIM_FLAG_Update);
    }
  } 
  TIM_Cmd(TIM6, DISABLE);
  
  FiO2_Lower = (FiO2_PureAir[55] + FiO2_PureAir[56] +FiO2_PureAir[57] +FiO2_PureAir[58] +FiO2_PureAir[59])/5;
  
  //Close Air and Oxygen Valve
  SentData_DAC(0x0000, 3);
}

//------------------------------------------------------------------------------
// Interrupt Push Botton User (Blue Botton)
void EXTI0_IRQHandler(void)
{
  Calibrate_OxygenSensor();
  // Clear Flag Interrupt
  EXTI_ClearITPendingBit(EXTI_Line0);
}

//void TIM6_DAC_IRQHandler(void)
//{
//  if (TIM_GetITStatus (TIM6, TIM_IT_Update) == RESET)
//  {
//    FiO2_PureOxygen[time] = Oxygen_convert();
//    time = time + 1;
//    STM_EVAL_LEDOff(LED5);
//    TIM_ClearITPendingBit (TIM6, TIM_IT_Update);
//  }
//}


//------------------------------------------------------------------------------
/*
Project : Programmable Feedback Control of Airflow System for Pre-term infant oxygen saturation
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Oxygen_sensor.c
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "Oxygen_sensor.h"
#include "GLCD5110.h"
#include "DAC_LTC1661.h"
//------------------------------------------------------------------------------
// Define Variable -------------------------------------------------------------
uint16_t ADC_Value, ADC_V;
uint16_t volatile time = 0;
uint16_t ADC_Voltage;
double ADC_fValue;
float FiO2_PureOxygen[60], FiO2_PureAir[60];
float FiO2_Upper, FiO2_Lower;
uint8_t FiO2_Percent;
// Function --------------------------------------------------------------------

void OxygenSensor_Setup(void)
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
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
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
	
  //Enable OxygenSensor
  ADC_Cmd(OxygenSensor, ENABLE);
  
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
  
  ADC_RegularChannelConfig(OxygenSensor, ADC_Channel_9, 1,ADC_SampleTime_28Cycles);
}

//------------------------------------------------------------------------------
void Timer6_SetUp(void)
{
  /*
    Set timer for count time measure oxygen sensor
    use timer 6, 16 bits, auto-reload, every 1 minite 
  */
  /*Timer Interrupt*/
  /* Set interrupt: NVIC_Setup */
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  /* Enable the TIM6 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
   
  /* TIM2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 2000; // 1 MHz down to 1 KHz (1 ms)
  TIM_TimeBaseStructure.TIM_Prescaler = 42000; // 24 MHz Clock down to 1 MHz (adjust per your clock)
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
  /* TIM IT enable */
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
  /* TIM2 enable counter */
  TIM_Cmd(TIM6, ENABLE);
  TIM_Cmd(TIM6, DISABLE);
}

//------------------------------------------------------------------------------
float Oxygen_convert(void)
{
  ADC_SoftwareStartConv(OxygenSensor);
  while(ADC_GetFlagStatus(OxygenSensor, ADC_FLAG_EOC) == RESET);
  /*Store ADC Sample*/
  ADC_Value = ADC_GetConversionValue(OxygenSensor);
  
  ADC_Voltage = '\0';
  if(ADC_Value > 348)
  {
    ADC_Voltage = (ADC_Value*2940)/1023;                                          //VDD is vary (time1 = 3.02, time2 = 2.94)
  }
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

  while(time <= 60)                                                             
  {
    if(TIM_GetFlagStatus(TIM6, TIM_FLAG_Update) != RESET)
    {
      FiO2_PureOxygen[time] = Oxygen_convert();
      time = time + 1;
      TIM_ClearFlag(TIM6, TIM_FLAG_Update);
    }
  }
  TIM_Cmd(TIM6, DISABLE);
  
  //Average PureAir Voltage Data
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
  time = 0;
  
  //Average PureAir Voltage Data
  FiO2_Lower = (FiO2_PureAir[55] + FiO2_PureAir[56] +FiO2_PureAir[57] +FiO2_PureAir[58] +FiO2_PureAir[59])/5;
  
  //Close Air and Oxygen Valve
  SentData_DAC(0x0000, 3);
  
  //LED 6 is off
  STM_EVAL_LEDOn(LED6);
  lcdClear();
  lcdUpdate();
}

//------------------------------------------------------------------------------
// Interrupt Push Botton User (Blue Botton)
void EXTI0_IRQHandler(void)
{
  Calibrate_OxygenSensor();
  
  // Clear Flag Interrupt
  EXTI_ClearITPendingBit(EXTI_Line0);
}
//------------------------------------------------------------------------------

void TIM6_DAC_IRQHandler(void)
{
  if (TIM_GetITStatus (TIM6, TIM_IT_Update) != RESET)
  {
    time = time + 1;
    STM_EVAL_LEDOff(LED5);
    TIM_ClearITPendingBit (TIM6, TIM_IT_Update);
  }
}

//------------------------------------------------------------------------------
// Not Complete
uint8_t Convert_FiO2 (float FiO2_ADC)
{
  FiO2_Percent = (FiO2_ADC);
  

}
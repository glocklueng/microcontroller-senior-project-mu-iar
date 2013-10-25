/*
Project : Programmable Feedback Control of Airflow System for Pre-term infant oxygen saturation
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Oxygen_sensor.c
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "Oxygen_sensor.h"
//------------------------------------------------------------------------------
// Define Variable -------------------------------------------------------------
uint16_t ADC_Value;
float ADC_Voltage;

// Function -------------------------------------------------------------------
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

float Oxygen_convert(void)
{
  ADC_SoftwareStartConv(ADC1);
  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
  /*Store ADC Sample*/
  ADC_Value = ADC_GetConversionValue(ADC1);
  ADC_Voltage = ADC_Value*(float)3.02/(float)1023;                              //VDD = 3.02 V
  
  return ADC_Voltage;
}

void GetSpO2 (void)
{
  
}

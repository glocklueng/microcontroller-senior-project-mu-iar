/*
Project : Programmable Feedback Control of Airflow System for Pre-term infant oxygen saturation
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : main.c
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "DAC_LTC1661.h"
#include "Oxygen_Pulse_Meter.h"
#include "Oxygen_sensor.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void delay(void);
// Variable --------------------------------------------------------------------
unsigned char msg ;
uint32_t count;

// Main Function ---------------------------------------------------------------
int main()
{	
  /* Set Up config System*/
  LTC1661_Setup();
  OxygenSensor_Setup();
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
  Timer6_SetUp();
  // LED Set UP
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
  STM_EVAL_LEDOn(LED3);
  STM_EVAL_LEDOn(LED4);
  STM_EVAL_LEDOn(LED5);
  STM_EVAL_LEDOn(LED6);

  // Disable Timer 6
  TIM_Cmd(TIM6, DISABLE);
  
  while(1)
  {
    SentData_DAC (0x03FF, 1);
    delay();
    SentData_DAC (0x0128, 1);
    delay();
  }
}
	
// delay function --------------------------------------------------------------
void delay(void)
{
  unsigned int i,j;
  for(i=0;i<5000;i++)
  {
    for(j=0;j<500;j++);
  }
}

//void TIM6_DAC_IRQHandler(void)
//{
//  if (TIM_GetITStatus (TIM6, TIM_IT_Update) != RESET)
//  {
//    TIM_ClearITPendingBit (TIM6, TIM_IT_Update);
//    time = time + 1;
//    STM_EVAL_LEDOff(LED5);
//    delay();
//  }
//}

// Interrupt Push Botton User (Blue Botton)
void EXTI0_IRQHandler(void)
{
  Calibrate_OxygenSensor();
}

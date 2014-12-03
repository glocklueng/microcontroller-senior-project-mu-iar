/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : system_init.c

Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "system_init.h"
//------------------------------------------------------------------------------
/**
  * @brief  Configures EXTI Line0 (connected to PA0 pin) in interrupt mode
  * @param  None
  * @retval None
  * Description : 
  */
void EXTILine0_Config(void)
{
  EXTI_InitTypeDef   EXTI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;

  /* Enable GPIOA clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  /* Configure PA0 pin as input floating */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Connect EXTI Line0 to PA0 pin */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

  /* Configure EXTI Line0 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

// Button_Up_Down_init -----------------------------------------------------
//void Button_Up_Down_Init(void)
//{
//	NVIC_InitTypeDef   NVIC_InitStructure;
//	/* Enable and set Button_Up_EXTI Line Interrupt to the lowest priority */
//  NVIC_InitStructure.NVIC_IRQChannel = Button_Up_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
//
//   /* Enable and set Button_Down_EXTI Line Interrupt to the lowest priority */
//  NVIC_InitStructure.NVIC_IRQChannel = Button_Down_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
//
//  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
//  TIM_Cmd(TIM3, ENABLE);
//  USART_Cmd(OPM_USART, ENABLE);
//  
//  GPIO_ResetBits(Alarm_Set_GPIO_Port, Alarm_Set_Pin);
//}
//------------------------------------------------------------------------
//void Button_EXTI_Config (void)
//{
//  EXTI_InitTypeDef   EXTI_InitStructure;
//  GPIO_InitTypeDef   GPIO_InitStructure;
//  NVIC_InitTypeDef   NVIC_InitStructure;
//
//  // Button Set Up
//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
//  /* Configure PB0, PB1, PB4, PB5 pin as Input */
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
//  GPIO_InitStructure.GPIO_Pin = Run_Button_Pin | Alarm_Button_Pin | Button_Up_Pin | Button_Down_Pin;
//  GPIO_Init(GPIOB, &GPIO_InitStructure);
//
//  /* Configure Alarm_Set_Pin (PC2) */
//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
//  
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//  GPIO_InitStructure.GPIO_Pin = Alarm_Set_Pin;
//  GPIO_Init(GPIOC, &GPIO_InitStructure);
//  
//  GPIO_ResetBits(GPIOC, GPIO_Pin_2);
//
//  /* Connect EXTI Line0 to PB0 pin (Button Down) */
//  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource0);
//
//  /* Configure EXTI Line0 (Button Down) */
//  EXTI_InitStructure.EXTI_Line = Button_Down_EXTI_Line;
//  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
//  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//  EXTI_Init(&EXTI_InitStructure);
//
//  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
//  NVIC_InitStructure.NVIC_IRQChannel = Button_Down_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
//  NVIC_Init(&NVIC_InitStructure);
//
//  /* Connect Run_Button_EXTI Line to PB1 pin (Run Button) */
//  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource1);
//
//  /* Configure Run_Button_EXTI_Line */
//  EXTI_InitStructure.EXTI_Line = Run_Button_EXTI_Line;
//  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
//  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//  EXTI_Init(&EXTI_InitStructure);
//
//  /* Disable Run_Button_EXTI Line Interrupt to the lowest priority */
//  NVIC_InitStructure.NVIC_IRQChannel = Run_Button_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
//  NVIC_Init(&NVIC_InitStructure);
//
//  /* Connect Alarm_Button_EXTI_Line to PB4 pin (Alarm Button) */
//  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);
//
//  /* Configure Alarm_Button_EXTI_Line */
//  EXTI_InitStructure.EXTI_Line = Alarm_Button_EXTI_Line;
//  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
//  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//  EXTI_Init(&EXTI_InitStructure);
//
//  /* Disable Alarm_Button_EXTI_Line Interrupt to the lowest priority */
//  NVIC_InitStructure.NVIC_IRQChannel = Alarm_Button_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
//  NVIC_Init(&NVIC_InitStructure);
//
//  /* Connect Button_Up_EXTI Line to PB5 pin (Button Up) */
//  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource5);
//
//  /* Configure Button_Up_EXTI Line */
//  EXTI_InitStructure.EXTI_Line = Button_Up_EXTI_Line;
//  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
//  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//  EXTI_Init(&EXTI_InitStructure);
//
//  /* Disable Button_Up_EXTI Line Interrupt to the lowest priority */
//  NVIC_InitStructure.NVIC_IRQChannel = Button_Up_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
//  NVIC_Init(&NVIC_InitStructure);
//
//}

//------------------------------------------------------------------------------
/*
  Function : Alarm_Timer_Setup
  Input : None
  Output : None
  Description : 
      Timer 2 use count time of Alarm Level 1 and Alarm Level 2
      when Timer 2 is Enable. it will set TIM_CMD(TIM2, ENABLE) function. 
      Timer 2 is count and interrupt every a second
      interrupt servies routine : TIM2_IRQHandler
*/
void Alarm_Timer_SetUp (void)
{
  /*
    Timer 2 use count time of Alarm Level 1 and Alarm Level 2
    when Timer 2 is Enable. it will set TIM_CMD(TIM2, ENABLE) function. 
    Timer 2 is count and interrupt every a second
    interrupt servies routine : TIM2_IRQHandler
  */
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  /* Enable the TIM2 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
   
  /* TIM2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 2000;                                      // 1 MHz down to 1 KHz (1 ms)
  TIM_TimeBaseStructure.TIM_Prescaler = 42000;                                  // 24 MHz Clock down to 1 MHz (adjust per your clock)
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  /* TIM IT enable */
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  /* TIM2 ALARM_DISABLE counter */
  TIM_Cmd(TIM2, DISABLE);
}
//------------------------------------------------------------------------------
/*
Function : timer4_setup
Input : None
Output : None
Description : Timer 4 use timing receiving data from Oxygen Pulse Meter.
*/
void timer4_setup(void)
{
  /* Set Up Timer 4 */
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
   /* TIM4 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 1000;                                      // setting overflow every 0.5 second
  TIM_TimeBaseStructure.TIM_Prescaler = 42000;                                  // 24 MHz Clock down to 1 MHz (adjust per your clock)
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  
  /* Enable the TIM4 Gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
 
  /* TIM IT enable */
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
  
  /* TIM2 ALARM_DISABLE counter */
  TIM_Cmd(TIM4, DISABLE);
}
// End of File -------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
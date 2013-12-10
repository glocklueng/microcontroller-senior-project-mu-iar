/*
Project : Programmable Feedback Control of Airflow System for Pre-term infant oxygen saturation
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Connect_GUI.c
*/
//------------------------------------------------------------------------------
/*
    Connect to GUI USE USART3 via RS-232 Protocol
    USART3 - Tx -> Port D Pin PD8
    USART3 - Rx -> Port D Pin PD9
    Baud Rate = 115200
    package = 8-n-1
    
*/
#include "main.h"
#include "Connect_GUI.h"
#include "DefinePin.h"
//-------------------------------------------------------------------------------
unsigned char DataFromGUI[50];
uint8_t rx_index_GUI=0;
uint8_t tx_index_GUI=0;

//------------------------------------------------------------------------------
void USART_GUI_Connect(void)
{
  /*
      NFC_Setup - USE USART6 
        PC6 - NFC_TX
        PC7 - NFC_RX
  */
  //Set Up USART
  //USE USART6 Set up for NFC
  
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;
	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
  /*
    Use Port C Pin PC6 - NFC_TX
    Use Port C Pin PC7 - NFC_RX
  */
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
  
  /* Connect PXx to USARTx_Tx*/
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig( GPIOC, GPIO_PinSource7, GPIO_AF_USART6);
  
  
  /* USART_InitStruct members default value */
  USART_InitStruct.USART_BaudRate = 115200;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
  USART_Init(USART6, &USART_InitStruct);
  
   /*USART Interrupt*/
  /* Set interrupt: NVIC_Setup */
  NVIC_InitTypeDef NVIC_InitStruct;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  //ENABLE USART4 Interruper
  NVIC_InitStruct.NVIC_IRQChannel = USART6_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  /* Set Interrupt Mode*/
  //ENABLE the USART Receive Interrupt
  USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);

  //Enable USART6
  USART_Cmd(USART6, ENABLE);
  
//  //Set Up Timer 3
//  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//  NVIC_InitTypeDef NVIC_InitStructure;
//  //Enable Clock Timer3
//  RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM3, ENABLE);
//
//  /* Enable the TIM3 gloabal Interrupt */
//  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 8;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
//   
//  /*
//
//  Pre-Scale : APB1 Prescale 4
//  System Clock 168MHz /4 = 42 MHz
//  Timer Prescale 4200
//  */
//  /* Time base configuration */
//  TIM_TimeBaseStructure.TIM_Period = 20;            
//  TIM_TimeBaseStructure.TIM_Prescaler = 42000;        // 42 MHz Clock down to 1 kHz (adjust per your clock)
//  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
//  /* TIM IT enable */
//  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
//  /* TIM3 enable counter */
//  TIM_Cmd(TIM3, DISABLE);
}

//-------------------------------------------------------------------------------



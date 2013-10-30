/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term Infants
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Oxygen_Pulse_Meter.c
Function : Receive Data form Oxygen Pulse Meter such as Oxygen Saturation (SaO2)
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "Oxygen_Pulse_Meter.h"
//------------------------------------------------------------------------------
unsigned char DataFromOPM[133];                                                 //Variable store for Data input from Oxygen Pulse Meter, Buffer size 150 Bytes But real 133 Bytes
uint8_t tx_index = 0;
uint8_t rx_index = 0;

// Set Up ----------------------------------------------------------------------
void Oxygen_PM_Setup(void)
{
  //Set Up USART
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;
	
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  /* Connect PXx to USARTx_Tx*/
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig( GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
	
  /*
    Use Port A Pin PA2 to Tx
    Use Port A Pin PA3 to Rx
  */
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* USART_InitStruct members default value */
  USART_InitStruct.USART_BaudRate = 9600;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No ;
  USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
  USART_Init(USART2, &USART_InitStruct);
  
  /*USART Interrupt*/
  /* Set interrupt: NVIC_Setup */
  NVIC_InitTypeDef NVIC_InitStruct;
  //ENABLE USART2 Interruper
  NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannel = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  /* Set Interrupt Mode*/
  //ENABLE the USART Receive Interrupt
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
  //DISABLE the USART Transmit and Receive Interrupt
  USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
  
  //Enable USART2
  USART_Cmd(USART2, ENABLE);
}

//USART_IRQHandler -------------------------------------------------------------
void USART2_IRQHandler(void)
{
  
  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
  {
    DataFromOPM[rx_index++] = USART_ReceiveData(USART2);
  
    if(rx_index >= (sizeof(DataFromOPM) - 1))
    {  
      rx_index = 0;
    }
  }
  if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
  {
    USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
  }
}


// Function can use printf(); in sent data
int fputc(int ch, FILE *f)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART2, (uint8_t) ch);
  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
  {}
  return ch;
}
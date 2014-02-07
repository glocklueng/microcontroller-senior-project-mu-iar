/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term Infants
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Oxygen_Pulse_Meter.c
Function : Receive Data form Oxygen Pulse Meter such as Oxygen Saturation (SaO2)
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "Oxygen_Pulse_Meter.h"
#include <stdlib.h>
//------------------------------------------------------------------------------                                              
//Variable store for Data input from Oxygen Pulse Meter, Buffer size 133 Bytes
unsigned char DataFromOPM[133]; 
//------------------------------------------------------------------------------
uint8_t OxygenSat_Percent;
uint8_t tx_index_OPM = 0;
uint8_t rx_index_OPM = 0;
// Set Up ----------------------------------------------------------------------
void Oxygen_PM_Setup(void)
{
  //Set Up USART
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;
	
  RCC_APB2PeriphClockCmd(OPM_USART_CLK, ENABLE);
  RCC_AHB1PeriphClockCmd(OPM_Port_CLK, ENABLE);
  /* Connect PXx to USARTx_Tx*/
  GPIO_PinAFConfig(OPM_Port, OPM_TX_Souce, OPM_TX_AF);
  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig(OPM_Port, OPM_RX_Souce, OPM_RX_AF);
	
  /*
    Use Port A Pin PA2 to Tx
    Use Port A Pin PA3 to Rx
  */
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = OPM_TX_Pin | OPM_RX_Pin;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(OPM_Port, &GPIO_InitStruct);
  
  /* USART_InitStruct members default value */
  USART_InitStruct.USART_BaudRate = 4800;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No ;
  USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
  USART_Init(OPM_USART, &USART_InitStruct);
  
  /*USART Interrupt*/
  /* Set interrupt: NVIC_Setup */
  //Data form Oxygen Pulse Meter : Priority 5
  NVIC_InitTypeDef NVIC_InitStruct;
  //ENABLE USART2 Interruper
  NVIC_InitStruct.NVIC_IRQChannel = OPM_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannel = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  /* Set Interrupt Mode*/
  //ENABLE the USART Receive Interrupt
  USART_ITConfig(OPM_USART, USART_IT_RXNE, ENABLE);
  //DISABLE the USART Transmit and Receive Interrupt
  USART_ITConfig(OPM_USART, USART_IT_TXE, DISABLE);
  
  //Enable USART2
  USART_Cmd(OPM_USART, DISABLE);

  //Set Up Timer 4
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  //Enable Clock Timer4
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM4, ENABLE);

  /* Enable the TIM4 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
   
  /*

  Pre-Scale : APB1 Prescale 4
  System Clock 168MHz /4 = 42 MHz
  Timer Prescale 4200
  */
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 20;            
  TIM_TimeBaseStructure.TIM_Prescaler = 42000;        // 42 MHz Clock down to 1 kHz (adjust per your clock)
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  /* TIM IT enable */
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
  /* TIM4 enable counter */
  TIM_Cmd(TIM4, DISABLE);
}

//USART_IRQHandler -------------------------------------------------------------
void OPM_IRQHandler(void)
{
  if(USART_GetITStatus(OPM_USART, USART_IT_RXNE) != RESET)
  {
    if (rx_index_OPM == 0)
    {
      //Start Receive Data from Oxygen Pulse Meter
      TIM_Cmd(TIM4, ENABLE);
    }
    DataFromOPM[rx_index_OPM++] = USART_ReceiveData(OPM_USART);
  
    if(rx_index_OPM >= (sizeof(DataFromOPM) - 1))
    {  
      TIM_Cmd(TIM4, DISABLE);
      rx_index_OPM = 0;
      OxygenSat_Percent = Get_OxygenSat();
    }
  }
  if(USART_GetITStatus(OPM_USART, USART_IT_TXE) != RESET)
  {
    USART_ITConfig(OPM_USART, USART_IT_TXE, DISABLE);
  }
}
//--------------------------------------------------------------------------------------

int Get_OxygenSat(void)
{
  /* 
    This Function is use for getting Oxygen Saturation Value (Percentage) from Oxygen Pulse Meter via RS-232
    Oxygen Saturation Address = number 37 to 39 (start 0) (SpO2=099%)
  */
  char OxygenSat_string[3];
  OxygenSat_Percent = 0 ;
  uint8_t i;
  //check this command is getting SaO2 or Headding Command
  if (DataFromOPM[0] == '+' && DataFromOPM[4] == 'P' && DataFromOPM[5] == 'V' && DataFromOPM[6] == 'I')
  {
    // Case : Correct
    for(i=0;i<3;i++)
    {
      OxygenSat_string[i] = DataFromOPM[37+i];
    }
    OxygenSat_Percent = atoi(OxygenSat_string);                                             // atoi is function convert from String to Int 
  }
  else
  {
    // Case : Error
    for (i = 0; i < 133; i++)
    {
      rx_index_OPM = 0;
      DataFromOPM[i] = '\0';
    }
    OxygenSat_Percent = '\0';
    
  }
  
  return OxygenSat_Percent;
  
//  if(DataFromOPM[37] == '1')
//  {
//    OxygenSat_Percent = 100;
//  }
//  else if(DataFromOPM[37] == '0')
//  {
//    switch DataFromOPM[38]
//    {
//      case '1':
//      OxygenSat_Percent = 10;
//      break;
//
//      case '2':
//      OxygenSat_Percent = 20;
//      break;
//
//      case '3':
//      OxygenSat_Percent = 30;
//      break;
//
//      case '4':
//      OxygenSat_Percent = 40;
//      break;
//
//      case '5':
//      OxygenSat_Percent = 50;
//      break;
//
//      case '6':
//      OxygenSat_Percent = 60;
//      break
//
//      case '7':
//      OxygenSat_Percent = 70;
//      break;
//
//      case '8':
//      OxygenSat_Percent = 80;
//      break;
//
//      case '9':
//      OxygenSat_Percent = 90;
//      break;
//
//      case '0':
//      OxygenSat_Percent = 0;
//      break;
//    }
//
//    switch DataFromOPM[39]
//    {
//      case '1':
//      OxygenSat_Percent = OxygenSat_Percent + 1;
//      break;
//
//      case '2':
//      OxygenSat_Percent = OxygenSat_Percent + 2;
//      break;
//
//      case '3':
//      OxygenSat_Percent = OxygenSat_Percent + 3;
//      break;
//
//      case '4':
//      OxygenSat_Percent = OxygenSat_Percent + 4;
//      break;
//
//      case '5':
//      OxygenSat_Percent = OxygenSat_Percent + 5;
//      break;
//
//      case '6':
//      OxygenSat_Percent = OxygenSat_Percent + 6;
//      break
//
//      case '7':
//      OxygenSat_Percent = OxygenSat_Percent + 7;
//      break;
//
//      case '8':
//      OxygenSat_Percent = OxygenSat_Percent + 8;
//      break;
//
//      case '9':
//      OxygenSat_Percent = OxygenSat_Percent + 9;
//      break;
//
//      case '0':
//      OxygenSat_Percent = OxygenSat_Percent + 0;
//      break;
//    }
//  }
//
//  return OxygenSat_Percent; 
}

//------------------------------------------------------------------------------------
//Timer 4 Check Timer Out of Receving data from Oxygen Pulse Meter?
void TIM4_IRQHandler (void)
{
  if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
  {
    uint8_t rx_index_OPM = 0;
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    //Clear Buffer Data from Oxygen Pulse Meter (OPM)
    for (rx_index_OPM = 0; rx_index_OPM < 133; rx_index_OPM++)
    {
      DataFromOPM[rx_index_OPM] = '\0';
    }
    //Diable Timer4
    TIM_Cmd(TIM4, DISABLE);
  }
}
//------------------------------------------------------------------------------
// Function can use printf(); in sent data
//int fputc(int ch, FILE *f)
//{
//  /* Place your implementation of fputc here */
//  /* e.g. write a character to the USART */
//  USART_SendData(OPM_USART, (uint8_t) ch);
//  /* Loop until the end of transmission */
//  while (USART_GetFlagStatus(OPM_USART, USART_FLAG_TC) == RESET)
//  {}
//  return ch;
//}

//------------------------------------------------------------------------------



// End of File -----------------------------------------------------------------

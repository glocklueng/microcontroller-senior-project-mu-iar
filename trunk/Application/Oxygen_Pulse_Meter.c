/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term Infants
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Oxygen_Pulse_Meter.c
Function : Receive Data form Oxygen Pulse Meter such as Oxygen Saturation (SpO2)
Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/

/*
                               Note
1. Function 
    1. Oxygen_PM_Setup : Setting USART for communication to Oxygen Pulse Meter via USART
    2. OPM_IRQHandler (Interrupt S.R.)
    3. Get_OxygenSat : find Oxygen Saturation value from ucDataFromOPM Buffer and convert String to uint
    4. TIM4_IRQHandler : 
2. Define global variable :
    char ucDataFromOPM[133]
    uint8_t uiCurrent_SpO2
    uint8_t uiSD_Card_index
    uint8_t uiRx_index_OPM
    uint8_t uiOxygenSat_buffer[10]
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "Oxygen_Pulse_Meter.h"
#include <stdlib.h>
//------------------------------------------------------------------------------                                              
//Variable store for Data input from Oxygen Pulse Meter, Buffer size 133 Bytes
char ucDataFromOPM[133]; 
//------------------------------------------------------------------------------
uint8_t uiCurrent_SpO2;
uint8_t uiSD_Card_index = 0;
uint8_t uiRx_index_OPM = 0;
uint8_t uiOxygenSat_buffer[10];                                                 // Oxygen Saturation Buffer for Store Data to SD Card
//------------------------------------------------------------------------------
/*
  Function : Oxygen_PM_Setup
  Input : None
  Return : None
  Description : Setup driver for connecting Oxygen Pulse Meter with USART
                Set Baud rate 9600
                Format 8-N-1
                Enable Rx_interrupt
*/
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
  USART_InitStruct.USART_BaudRate = 9600;
  //USART_InitStruct.USART_BaudRate = 115200;
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
  
  //DIsable OPM_USART (USART6)
  USART_Cmd(OPM_USART, ENABLE);

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
  TIM_TimeBaseStructure.TIM_Period = 5000;            
  TIM_TimeBaseStructure.TIM_Prescaler = 42000;                                  // 42 MHz Clock down to 1 kHz (adjust per your clock)
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
    if (uiRx_index_OPM == 0)
    {
      //Start Receive Data from Oxygen Pulse Meter
      TIM_Cmd(TIM4, ENABLE);
    }
    ucDataFromOPM[uiRx_index_OPM++] = USART_ReceiveData(OPM_USART);
  
    if(uiRx_index_OPM >= (sizeof(ucDataFromOPM) - 1))
    {  
      TIM_Cmd(TIM4, DISABLE);
      uiRx_index_OPM = 0;
      uiCurrent_SpO2 = Get_OxygenSat();
      uiOxygenSat_buffer[uiSD_Card_index] = uiCurrent_SpO2;
      uiSD_Card_index++;
    }
  }
  if(USART_GetITStatus(OPM_USART, USART_IT_TXE) != RESET)
  {
    USART_ITConfig(OPM_USART, USART_IT_TXE, DISABLE);
  }
  USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}
//--------------------------------------------------------------------------------------
/*
  Function : Get_OxygenSat
  Input : None
  Return : int cOxygenSat_Percent
  Description : This Function is use for getting Oxygen Saturation Value (Percentage) from Oxygen Pulse Meter
                via RS-232 Oxygen Saturation Address = number 37 to 39 (start 0) (SpO2=099%)
*/

int Get_OxygenSat(void)
{
  /* 
    This Function is use for getting Oxygen Saturation Value (Percentage) from Oxygen Pulse Meter via RS-232
    Oxygen Saturation Address = number 37 to 39 (start 0) (SpO2=099%)
  */
  char cOxygenSat_string[3];
  uint8_t uiOxygenSat_Percent, uiIndexString;
  uiOxygenSat_Percent = 0 ;

  /* check this command is getting SaO2 or Headding Command */
  if (ucDataFromOPM[0] == '+' && ucDataFromOPM[4] == 'P' && ucDataFromOPM[5] == 'V' && ucDataFromOPM[6] == 'I')
  {
    // Case : Correct
    for(uiIndexString = 0; uiIndexString < 3; uiIndexString++)
    {
      cOxygenSat_string[uiIndexString] = ucDataFromOPM[37 + uiIndexString];
    }
    uiOxygenSat_Percent = atoi(cOxygenSat_string);                              // atoi is function convert from String to Int 
    uiCurrent_SpO2 = uiOxygenSat_Percent;
  }
  else
  {
    // Case : Error
    for (uiIndexString = 0; uiIndexString < 133; uiIndexString++)
    {
      uiRx_index_OPM = 0;
      ucDataFromOPM[uiIndexString] = '\0';
    }
    uiOxygenSat_Percent = '\0';
    uiCurrent_SpO2 = uiOxygenSat_Percent;
  }
  
  return uiOxygenSat_Percent;
}
//------------------------------------------------------------------------------------
/* Timer 4 Check Timer Out of Receving data from Oxygen Pulse Meter? */
void TIM4_IRQHandler (void)
{
  if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
  {
    uint8_t uiRx_index_OPM = 0;
    //Clear Buffer Data from Oxygen Pulse Meter (OPM)
    for (uiRx_index_OPM = 0; uiRx_index_OPM < 133; uiRx_index_OPM++)
    {
      ucDataFromOPM[uiRx_index_OPM] = '\0';
    }
    //Diable Timer4
    TIM_Cmd(TIM4, DISABLE);
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
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
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/

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
    4. clear_OPM_buffer : clear data in ucDataFromOPM Buffer
    5. TIM4_IRQHandler : 
2. Define global variable :
    char ucDataFromOPM[174]
    uint8_t uiCurrent_SpO2
    uint8_t uiSD_Card_index
    uint8_t uiRx_index_OPM
    uint8_t uiOxygenSat_buffer[10]
    char cDataTime[17][3] : Store the date and time (lenght 17 Bytes per a time)
    bool bReadCorrect : boolean in case of Read data from Oxygen Pulse Meter correct or incorrect ?
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "DefinePin.h"
#include "Oxygen_Pulse_Meter.h"
#include <stdlib.h>
//------------------------------------------------------------------------------                                              
//Variable store for Data input from Oxygen Pulse Meter, Buffer size 174 Bytes
char ucDataFromOPM[174]; 
//------------------------------------------------------------------------------
uint8_t uiCurrent_SpO2;
uint8_t uiSD_Card_index = 0;
uint8_t uiRx_index_OPM = 0;
uint8_t uiOxygenSat_buffer[10];                                                 // Oxygen Saturation Buffer for Store Data to SD Card

char cDateTime[17][3];                                                          // create array 2D for stroe Date and time
bool bReadCorrect = false; 

extern uint8_t uiSpO2_SDcard_buffer[3];
//------------------------------------------------------------------------------
/*
  Function : Oxygen_PM_Setup
  Input : None
  Return : None
  Description : Setup driver for connecting Oxygen Pulse Meter with USART
                Set Baud rate 9600, 8-N-1
                Enable Rx communication ONLY
                Enable Rx_interrupt
*/
void usart_OPM_setup(void)
{
  //Set Up USART
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;
	
  RCC_APB2PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
  /*
    Use Port A Pin PA2 to Tx
    Use Port A Pin PA3 to Rx
  */
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStruct);
  
   /* Connect PXx to USARTx_Tx*/
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);
  
  /* USART_InitStruct members default value */
  USART_InitStruct.USART_BaudRate = 9600;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No ;
  USART_InitStruct.USART_Mode = USART_Mode_Rx;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
  USART_Init(USART3, &USART_InitStruct);
  
  /*USART Interrupt*/
  /* Set interrupt: NVIC_Setup */
  //Data form Oxygen Pulse Meter : Priority 0
  NVIC_InitTypeDef NVIC_InitStruct;
  //ENABLE USART2 Interruper
  NVIC_InitStruct.NVIC_IRQChannel = OPM_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannel = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  /* Set Interrupt Mode*/
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);                                // ENABLE the USART Receive Interrupt

  USART_ITConfig(USART3, USART_IT_TXE, DISABLE);                                // DISABLE the USART Transmit  Interrupt
  
  /* Disable OPM_USART (USART3) */
  USART_Cmd(USART3, DISABLE);
}

//USART_IRQHandler -------------------------------------------------------------
/*
  Function : OPM_IRQHandler
  Input : None
  Output : None
  Description : Interrupt Service Routine from OPM_USART
                Enable IT source : USART_IT_RXNE
*/
void USART3_IRQHandler(void)
{
  if(USART_GetITStatus(OPM_USART, USART_IT_RXNE) != RESET)
  {
    if (uiRx_index_OPM == 0)
    {
      /* Start Receive Data from Oxygen Pulse Meter and 
      Enable Timer 4 for timimg in receive SpO2 from Oxygen Pulse Meter */
      TIM_Cmd(TIM4, ENABLE);                                                    // Enable Timer 4
    }
    
    ucDataFromOPM[uiRx_index_OPM] = USART_ReceiveData(OPM_USART);
    uiRx_index_OPM = uiRx_index_OPM + 1;
    
    if(uiRx_index_OPM == (sizeof(ucDataFromOPM)))
    {  
      TIM_Cmd(TIM4, DISABLE);                                                   // Disable Timer 4
      TIM4->CNT = 0;                                                            // Clear counter value
      uiRx_index_OPM = 0;
      uiCurrent_SpO2 = Get_OxygenSat();
      if(bReadCorrect == true)
      {
        uiSpO2_SDcard_buffer[uiSD_Card_index] = uiCurrent_SpO2;                 // Store SpO2 in Buffer
        uiSD_Card_index++;
      }
    }
  }
  USART_ClearITPendingBit(OPM_USART, USART_IT_RXNE);
}
//------------------------------------------------------------------------------
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
  if ((ucDataFromOPM[18] == 'S') && (ucDataFromOPM[19] == 'N') && (ucDataFromOPM[126] == 'P') && (ucDataFromOPM[127] == 'V') && (ucDataFromOPM[128] == 'I'))
  {
    /* Case : Read Correct */
    for(uiIndexString = 0; uiIndexString < 3; uiIndexString++)
    {
      cOxygenSat_string[uiIndexString] = ucDataFromOPM[37 + uiIndexString];
    }
    
    uiOxygenSat_Percent = atoi(cOxygenSat_string);                              // atoi is function convert from String to Int 
    uiCurrent_SpO2 = uiOxygenSat_Percent;
    
    /* Transfer data and time from Oxygen Pulse Meter to Buffer*/
    for (uint8_t uiIndexTime = 0; uiIndexTime < 17; uiIndexTime++)
    {
      cDateTime[uiIndexTime][uiSD_Card_index] = ucDataFromOPM[uiIndexTime];
    }
    
    bReadCorrect = true;
  }
  else
  {
    /* Case :  Read Error */
    clear_OPM_buffer();
    uiOxygenSat_Percent = '\0';
    uiCurrent_SpO2 = uiOxygenSat_Percent;
    bReadCorrect = false;
  }
  
  return uiOxygenSat_Percent;
}
//------------------------------------------------------------------------------
/*
  Function : clear_OPM_buffer
  Input : none
  Output : none
  Description : Clear data in ucDataFromOPM Buffer in case of incorrect.
*/
void clear_OPM_buffer(void)
{
  uint8_t uiIndex_OPM_buffer = 0;
  /*Clear data in Buffer*/
  for (uiIndex_OPM_buffer = 0; uiIndex_OPM_buffer < (sizeof(ucDataFromOPM)); uiIndex_OPM_buffer++)
  {
    ucDataFromOPM[uiIndex_OPM_buffer] = '\0';
  }
  uiRx_index_OPM = 0;
}
//------------------------------------------------------------------------------
/* 
  Function : TIM4_IRQHandler
  Input : None (Timer 4 Interrupt)
  Output : None
  Description : Timer 4 overflow of Receving data from Oxygen Pulse Meter ? 
                clear ucDataFromOPM buffer and reset uiRx_index_OPM.
*/
void TIM4_IRQHandler (void)
{
  if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
  {
    /* Clear Buffer Data from Oxygen Pulse Meter (OPM) */
    clear_OPM_buffer();                                                         // Clear OPM Buffer
    uiRx_index_OPM = 0;
    TIM_Cmd(TIM4, DISABLE);                                                     // Diable Timer4
  }
  TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
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

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
    3. DMA1_Stream1_IRQHandler (Interrupt S.R.)
    4. Get_OxygenSat : find Oxygen Saturation value from ucDataFromOPM Buffer and convert String to uint
    5. clear_OPM_buffer : clear data in ucDataFromOPM Buffer
    6. TIM4_IRQHandler : 
2. Define global variable :
    static char scDataFromOPM[174]
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
static char scDataFromOPM[180];                                                 // 174 bytes for simulation with MCU
//------------------------------------------------------------------------------
uint8_t uiCurrent_SpO2, uiInitial_SpO2;
uint8_t uiSD_Card_index = 0;
static uint16_t uiRx_index_OPM = 0;
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
                Disable Rx_interrupt
                Setting DMA for transfer data from USART 3 Peripheral to Memory (DMA1 Channel 4 Stream 1)
                Enable DMA1_Stream1 Interrupt
*/
void usart_OPM_setup(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
   
  /*
    Use Port B Pin PD6 
    Use Port B Pin PD7 
  */
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_9;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  /* Connect PD8 to USART3_Tx*/
  //GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
  /* Connect PD9 to USART3_Rx*/
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);
  
  
  /* USART_InitStruct members default value */
  USART_InitStruct.USART_BaudRate = 9600;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_Mode = USART_Mode_Rx;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
  USART_Init(USART3, &USART_InitStruct);
  
  /* DMA 1 Stream 1 Channel 4 (USART3_Rx) Configuration */
  DMA_InitTypeDef DMA_InitStruct;
  
  DMA_DeInit(DMA1_Stream1);
  
  DMA_InitStruct.DMA_Channel = DMA_Channel_4;                                   // Setting DMA channel 4
  DMA_InitStruct.DMA_BufferSize = sizeof(scDataFromOPM);                        // DMA buffer size 174 bytes
  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR;                // set usart3 address USART3_DR_ADDRESS is 0x40004804
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)&scDataFromOPM;                // set address to scDataFromOPM variable for receiving data
  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;                          // transfer data from Peripheral to memory
  DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Enable;
  DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStruct.DMA_Priority = DMA_Priority_High;
  DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream1, &DMA_InitStruct);
    
  /* Set Interrupt Mode*/
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Comment USART Interrupt : Switch to use DMA transfer data from Peripheral to Memory*/
  /*ENABLE the USART Receive Interrupt*/
//  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
//  
//  USART_ITConfig(USART3, USART_IT_TXE, DISABLE);

  /*Enable USART 3 DMA */
  USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
  

  /* Enable DMA Stream Half Transfer and Transfer Complete interrupt */
  DMA_ITConfig(DMA1_Stream1, DMA_IT_TC , ENABLE);
  DMA_ITConfig(DMA1_Stream1, DMA_IT_HT , ENABLE);
  
  /*Enable DMA1 Stream1*/
  DMA_Cmd(DMA1_Stream1, ENABLE);
  
  /*Enable USART3 */
  USART_Cmd(USART3, DISABLE);
}

// DMA1_Stream1_IRQHandler -----------------------------------------------------
/*
  Function : DMA1_Stream1_IRQHandler
  Input : None
  Output : None
  Description : Interrupt when DMA stream transfer data complete
*/
void DMA1_Stream1_IRQHandler(void)
{
  /* Test on DMA Stream Transfer Complete interrupt */
  if (DMA_GetITStatus(DMA1_Stream1, DMA_IT_TCIF1) != RESET)
  {
    uiCurrent_SpO2 = Get_OxygenSat(scDataFromOPM);
    if(bReadCorrect == true)
    {
      uiSpO2_SDcard_buffer[uiSD_Card_index] = uiCurrent_SpO2;                   // Store SpO2 in Buffer
      uiSD_Card_index++;
      bReadCorrect = false;
    }
    
    /* Clear DMA Stream Transfer Complete interrupt pending bit */
    DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_TCIF1);
  }
  
  if(DMA_GetITStatus(DMA1_Stream1, DMA_IT_HTIF1) != RESET)
  {
    DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_HTIF1);
  }
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
    if (uiRx_index_OPM == 1)
    {
      /* Start Receive Data from Oxygen Pulse Meter and 
      Enable Timer 4 for timimg in receive SpO2 from Oxygen Pulse Meter */
      TIM_Cmd(TIM4, ENABLE);                                                    // Enable Timer 4
      TIM4->CNT = 0;
    }
    
    //scDataFromOPM[uiRx_index_OPM] = USART_ReceiveData(OPM_USART);
    //uiRx_index_OPM = uiRx_index_OPM + 1;
    
    if(uiRx_index_OPM >= (sizeof(scDataFromOPM)))
    {  
      TIM_Cmd(TIM4, DISABLE);                                                   // Disable Timer 4
      TIM4->CNT = 0;                                                            // Clear counter value
      
      uiCurrent_SpO2 = Get_OxygenSat(scDataFromOPM);
      
      if(bReadCorrect == true)
      {
        uiSpO2_SDcard_buffer[uiSD_Card_index] = uiCurrent_SpO2;                 // Store SpO2 in Buffer
        uiSD_Card_index++;
        bReadCorrect = false;
      }
      uiRx_index_OPM = 0;
    }
  }
  else if (USART_GetITStatus(OPM_USART, USART_IT_TXE) != RESET)
  {
    USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
    USART_ClearITPendingBit(OPM_USART, USART_IT_TXE);
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

int Get_OxygenSat(char cOPM_protocal[])
{
  /* 
    This Function is use for getting Oxygen Saturation Value (Percentage) from Oxygen Pulse Meter via RS-232
    Oxygen Saturation Address = number 37 to 39 (start 0) (SpO2=099%)
  */
  char cOxygenSat_string[3];
  uint8_t uiSpO2_percent, uiIndexString;

  /* check this command is getting SaO2 or Headding Command */
  if ((cOPM_protocal[18] == 'S') && (cOPM_protocal[19] == 'N') && (cOPM_protocal[126] == 'P') && (cOPM_protocal[127] == 'V') && (cOPM_protocal[128] == 'I'))
  {
    /* Case : Read Correct */
    for(uiIndexString = 0; uiIndexString < 3; uiIndexString++)
    {
      cOxygenSat_string[uiIndexString] = cOPM_protocal[37 + uiIndexString];
    }
    uiSpO2_percent = atoi(cOxygenSat_string);                                   // atoi is function convert from String to Int 
    
    /* Transfer data and time from Oxygen Pulse Meter to Buffer*/
    for (uint8_t uiIndexTime = 0; uiIndexTime < 17; uiIndexTime++)
    {
      cDateTime[uiIndexTime][uiSD_Card_index] = cOPM_protocal[uiIndexTime];
    }
    
    bReadCorrect = true;
  }
  else
  {
    /* Case :  Read Error */
    clear_OPM_buffer();
    uiSpO2_percent = 0;
    bReadCorrect = false;
  }
  
  return uiSpO2_percent;
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
  for (uiIndex_OPM_buffer = 0; uiIndex_OPM_buffer < (sizeof(scDataFromOPM)); uiIndex_OPM_buffer++)
  {
    scDataFromOPM[uiIndex_OPM_buffer] = '1';
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
    //clear_OPM_buffer();                                                         // Clear OPM Buffer
    uiRx_index_OPM = 0;
  }
  TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
  TIM_Cmd(TIM4, DISABLE); 
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

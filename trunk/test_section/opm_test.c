/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : opm_test.c
Deverloper : Phattaradanai Kiratiwudhikul
Reseach & Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
/*
Note: 
1. Use for receive data from Oxygen Pulse Meter and send data to Computer via RS-232
2. Use USART 3 for receiving data from oxygen pulse meter 
3. USART 3 is setting : Baud rate 9600, 8-N-1, Full duplex communication, enable receive interrupt
4. Size of buffer for storing information from oxygen pulse meter per a time is 181 Bytes
5. Use Timer4 for count time in receive data per a time. If time is more than 0.5 sec, the system will clear Buffer.
*/
#include "main.h"
#include "DAC_LTC1661.h"
#include "MCP3202.h"
#include "system_init.h"
#include "Oxygen_Pulse_Meter.h"
#include "Oxygen_sensor.h"
#include "GLCD5110.h"
#include "DefinePin.h"
#include "Connect_GUI.h"
#include "testControlValve.h"
#include "check_status_profile.h"
#include "alarm_condition.h"
#include "ff.h"
#include "usbd_cdc_vcp.h"
#include <stdlib.h>
//------------------------------------------------------------------------------
//Variable store for Data input from Oxygen Pulse Meter, Size of Buffer is 181 Bytes
char ucDataFromOPM[181]; 
//------------------------------------------------------------------------------
uint8_t uiCurrent_SpO2;
uint8_t uiSD_Card_index = 0;
uint8_t uiRx_index_OPM = 0;
uint8_t uiOxygenSat_buffer[10];                                                 // Oxygen Saturation Buffer for Store Data to SD Card
//------------------------------------------------------------------------------
__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END;
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

// variable for SD card
/* Private typedef -----------------------------------------------------------*/
SD_Error Status = SD_OK;
FATFS filesystem;		                                                            // volume lable
FRESULT ret;			                                                              // Result code
FIL file_F, file_O, file;		                                                    // File object
DIR dir;				                                                                // Directory object
FILINFO fno;			                                                              // File information object
UINT bw, br;
uint8_t buff[128];
//------------------------------------------------------------------------------
// Golbol Function -------------------------------------------------------------
void system_init(void);
void usart_OPM_setup(void);

//------------------------------------------------------------------------------
// Profile Variable ------------------------------------------------------------
char HospitalNumber_File[13];
char Buffer[180];
uint8_t uiCurrent_Status;
uint8_t Time_AlarmLevel = 0;
uint8_t uiPurpose_FiO2;

// Main Function ---------------------------------------------------------------

int main(void)
{
  /* Set Up config  System*/
  system_init();
   
  while(1)
  {
    
  }
}
// End of Main Function --------------------------------------------------------

/*
  Function : system_init
  Input : None
  Output : None
  Description : set up and config all periph driver
*/
void system_init(void)
{
  usart_OPM_setup();
}
//------------------------------------------------------------------------------
/*
  Function : usart_OPM_setup
  Input : none
  Output : none
  Description : set up usart for receive infromation from oxygen pulse meter
                use USART3 (PD8, PD9), Baud Rate = 9600, 8-N-1, Enable Rx_interrupt
*/
void usart_OPM_setup(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  /*
    Use Port D Pin PD8
    Use Port D Pin PD9 
  */
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  /* Connect PD8 to USART3_Tx*/
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
  /* Connect PD9 to USART3_Rx*/
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);
  
  /* USART_InitStruct members default value */
  USART_InitStruct.USART_BaudRate = 9600;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
  USART_Init(USART3, &USART_InitStruct);
  
  /*USART Interrupt*/
  /* Set interrupt: NVIC_Setup */
  NVIC_InitTypeDef NVIC_InitStruct;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /*ENABLE USART3 Interruper*/
  NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  /* Set Interrupt Mode*/
  /*ENABLE the USART Receive Interrupt*/
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

  /*Enable USART3 */
  USART_Cmd(USART3, ENABLE);
  
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
  TIM_TimeBaseStructure.TIM_Period = 1000;            
  TIM_TimeBaseStructure.TIM_Prescaler = 42000;                                  // 42 MHz Clock down to 1 kHz (adjust per your clock)
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  /* TIM IT enable */
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
  /* TIM4 enable counter */
  TIM_Cmd(TIM4, DISABLE);
}
//------------------------------------------------------------------------------
/*
  Note : USART3_IRQHandler is at Oxygen_Pulse_Meter.c 
*/
//USART_IRQHandler -------------------------------------------------------------
void USART3_IRQHandler(void)
{
  uint8_t uiIndexString;
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
  if ((ucDataFromOPM[18] == 'S') && (ucDataFromOPM[19] == 'N') && (ucDataFromOPM[126] == 'P') && (ucDataFromOPM[127] == 'V') && (ucDataFromOPM[128] == 'I'))
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
    for (uiIndexString = 0; uiIndexString < (sizeof(ucDataFromOPM) - 1); uiIndexString++)
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
    for (uiRx_index_OPM = 0; uiRx_index_OPM < (sizeof(ucDataFromOPM) - 1); uiRx_index_OPM++)
    {
      ucDataFromOPM[uiRx_index_OPM] = '\0';
    }
    uiRx_index_OPM = 0;
    //Diable Timer4
    TIM_Cmd(TIM4, DISABLE);
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
  }
}
//------------------------------------------------------------------------------
void fault_err (FRESULT rc)
{
  const char *str =
                    "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
                    "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
                    "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
                    "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
  FRESULT i;

  for (i = (FRESULT)0; i != rc && *str; i++) 
  {
    while (*str++) ;
  }
  printf("rc=%u FR_%s\n\r", (UINT)rc, str);
  STM_EVAL_LEDOff(LED6);
  while(1);
}
//------------------------------------------------------------------------------
#ifdef USE_FULL_ASSERT
/**
* @brief  assert_failed
*         Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  File: pointer to the source file name
* @param  Line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
 
  /* Infinite loop */
  while (1)
  {}
}
#endif
// End of File -----------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
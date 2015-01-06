/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : opm.c
Deverloper : Phattaradanai Kiratiwudhikul
Reseach & Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
/*
Note: 
  This file MCU will working as same as Oxygen pulse meter. It will read data form SD card
  and send data, Oxygen saturation information, to Microcontroller every 1 second.

  1. Use for read data from SD card and send to MCU via USART
  2. Use USART 3 for sending data to another Microcontroller
  3. USART 3 is setting : Baud rate 9600, 8-N-1, Half-duplex communication (Tx), disable interrupt USART3
  4. Size of buffer for storing Oxygen Saturation Protocal per a time is 174 Bytes
  5. Use Timer3 for checking time every 1 sec.
  6. File name for store information in SD card is "r_SpO2.TXT"
*/
#include "main.h"
#include "stdbool.h"                                                            // use declar boolean variable type
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
char ucDataFromOPM[174]; 
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
FATFS filesystem;		                                                // volume lable
FRESULT ret;			                                                // Result code
FIL file_F, file_O, file;		                                        // File object
DIR dir;				                                        // Directory object
FILINFO fno;			                                                // File information object
UINT bw, br;
uint8_t buff[174];
//------------------------------------------------------------------------------
// Golbol Function -------------------------------------------------------------
void system_init(void);
void usart_OPM_setup(void);
void Timer3_SetUp(void);
//------------------------------------------------------------------------------
// Profile Variable ------------------------------------------------------------
char HospitalNumber_File[] = "s_SpO2.TXT";
char Buffer[174];
uint8_t uiCurrent_Status;
uint8_t Time_AlarmLevel = 0;
uint8_t uiPurpose_FiO2;

bool bRead_data_permission = FALSE;
uint16_t length;

extern uint8_t time;

// Main Function ---------------------------------------------------------------

int main(void)
{
  /* Set Up config  System*/
  system_init();
   
  /*Check SD card mount*/
  if (f_mount(0, &filesystem) != FR_OK)
  {
    while(1);
  }
  
  /* find profile Record */
  ret = f_open(&file, HospitalNumber_File, FA_READ);
  if (ret) 
  {
    printf("r_SpO2.TXT file error\n\r");
  } 
  else if (ret == 0)
  {
    printf("Type the file content(r_SpO2.TXT)\n\r");
    for (;;) 
    {
      if (bRead_data_permission == TRUE)
      {
        ret = f_read(&file, buff, sizeof(buff), &br); /* Read a chunk of file */
        if (ret || !br) 
        {
          break;      /* Error or end of file */
        }
        buff[br] = 0;
        for(length = 0; length < sizeof(buff); length++)
        {
          Buffer[length] = buff[length];
          USART_SendData(USART3, Buffer[length]);
          while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
        }
        bRead_data_permission = FALSE;
      }
    }
    
    if (ret) 
    {
      printf("Read file (r_SpO2.TXT) error\n\r");
      fault_err(ret);
    }
    printf("Close the file (r_SpO2.TXT)\n\r");
    ret = f_close(&file);
    if (ret) 
    {
      printf("Close the file (r_SpO2.TXT) error\n\r");
    }
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
  Timer3_SetUp();
}
//------------------------------------------------------------------------------
/*
  Function : usart_OPM_setup
  Input : none
  Output : none
  Description : set up usart for sending infromation to Microcontroller
                use USART3 (PD8, PD9), Baud Rate = 9600, 8-N-1, Enable Rx_interrupt, Half-duplex communication (Tx)
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
  USART_InitStruct.USART_Mode = USART_Mode_Tx;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
  USART_Init(USART3, &USART_InitStruct);

  /*Enable USART3 */
  USART_Cmd(USART3, ENABLE);
}
//------------------------------------------------------------------------------
/*
  Function : Timer3_SetUp
  Input : None
  Output : None
  Description : count every 1 sec.
*/
void Timer3_SetUp(void)
{
  /*Timer Interrupt*/
  /* Set interrupt: NVIC_Setup */
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  /* Enable the TIM3 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
   
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 2000;                                      // 1 MHz down to 1 KHz (1 ms)
  TIM_TimeBaseStructure.TIM_Prescaler = 42000;                                  // 24 MHz Clock down to 1 MHz (adjust per your clock)
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  /* TIM IT enable */
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
}

//------------------------------------------------------------------------------
/* Timer 3 count every 1 second */
void TIM3_IRQHandler (void)
{
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  {
    time = time + 1;
    bRead_data_permission = TRUE;
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
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
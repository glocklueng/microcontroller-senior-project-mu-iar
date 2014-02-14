/*
Project : Programmable Feedback Control of Airflow System for Pre-term infant oxygen saturation
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : main.c
Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "DAC_LTC1661.h"
#include "Oxygen_Pulse_Meter.h"
#include "Oxygen_sensor.h"
#include "GLCD5110.h"
#include "DefinePin.h"
#include "Connect_GUI.h"
#include "ff.h"
#include <stdlib.h>

//------------------------------------------------------------------------------
__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END;
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
// Define ----------------------------------------------------------------------


/* Private typedef -----------------------------------------------------------*/
SD_Error Status = SD_OK;
FATFS filesystem;		                                                // volume lable
FRESULT ret;			                                                // Result code
FIL file;				                                        // File object
DIR dir;				                                        // Directory object
FILINFO fno;			                                                // File information object
UINT bw, br;
uint8_t buff[128];


//------------------------------------------------------------------------------
void delay(void);
void System_Init(void);
void INTTIM_Config(void);
void EXTILine0_Config(void);
void Alarm_Timer_SetUp (void);
void Alarm_Function(uint8_t Command);

/* Private function prototypes -----------------------------------------------*/
void ConvertInttoString(uint8_t DataInt[]);
void USART_HyperTermianl_Connect(void);
void Create_file(char Hospital_Number[], uint8_t File_Type);

// Variable --------------------------------------------------------------------
unsigned char msg ;
char Character;
uint32_t count;
extern uint16_t time;
extern uint8_t rx_index_GUI;
uint8_t Data_GUI[28];
uint8_t SD_Test[50];
char SD_String[250];
uint8_t index = 0;                                                                  //for count receving Data form Hyperterminal for controling Drive Circuit

extern uint8_t OxygenSat_buffer[100];
extern uint8_t Current_OyxgenSat;
// Status ----------------------------------------------------------------------
#define ALARM_DISABLE         0
#define ALARM_ENABLE          1

#define Status_Normal                         0
#define Status_OxygenSat_Below_L1             1
#define Status_OxygenSat_Below_L2             2
#define Status_OxygenSat_Behigh_L1            3
#define Status_OxygenSat_Behigh_L2            4

uint8_t Current_Status;
uint8_t Time_AlarmLevel = 0;

// Profile Variable ------------------------------------------------------------
char Hospital_Number[13];
char HospitalNumber_File[13];
char Drive_command_Data[5];
uint8_t OxygenSaturaiton_Maximum, OxygenSaturation_Minimum;
uint8_t FiO2_Maximum, FiO2_Minimum;
uint8_t RespondsTime;
uint8_t Prefered_FiO2;
uint8_t Alarm_Level1, Alarm_Level2;
uint8_t Mode;

uint8_t Profile_Upload;
// Main Function ---------------------------------------------------------------
int main()
{  
 /* Set Up config System*/
  System_Init();
  lcdString (1,1,"Please Upload Profile");
  
  Profile_Upload = PROFILE_NOTUPLOAD;
 
  
  //Test Transfer Data to SD Card
//  uint8_t count;
//  for(count = 0; count < 50; count++)
//  {
//    SD_Test[count] = count;
//  }
//  ConvertInttoString(SD_Test);
//  SD_Write("OXY.TXT", SD_String, 250);

  while(1)
  {
    
    if (Profile_Upload == PROFILE_JUST_UPLOAD)
    {
      USART_Cmd(OPM_USART, ENABLE);                                             // ENABLE Oxygen Pulse Meter USART
      Create_file(Hospital_Number, OxygenSaturation_file);                      // Create Oxygen Saturation file
      Create_file(Hospital_Number, FiO2_file);                                  // Create FiO2 file
      Profile_Upload = PROFILE_SETTING_COMPLETE;
    }
    else if (Profile_Upload == PROFILE_NOTUPLOAD)
    {
      USART_Cmd(OPM_USART, DISABLE);                                            // DISABLE Oxygen Pulse Meter USART
      SentData_DAC(0x00,3);                                                     // Close air and oxygen valve
    }

    // Check Oxygen Saturation condition
    if (Current_OyxgenSat < OxygenSaturation_Minimum)
    {
      // Current Oxygen Saturation less than Minimum Oxygen Saturation
      if (Current_Status == Status_Normal)
      {
        Current_Status = Status_OxygenSat_Below_L1;
        Alarm_Function(ALARM_ENABLE);
        lcdString(1,5,"Status: Below L1");
      }      
    }
    else if (Current_OyxgenSat > OxygenSaturaiton_Maximum)
    {
      // Current Oxygen Saturation more than maximum Oxygen Saturation
      if (Current_Status == Status_Normal)
      {
        Current_Status = Status_OxygenSat_Behigh_L1;
        Alarm_Function(ALARM_ENABLE);
        lcdString(1,5,"Status: Behigh L1");
      }

    }
    else if (Current_OyxgenSat - OxygenSaturation_Minimum <= 1)
    {
      if (Current_Status!= Status_Normal)
      {
        Current_Status = Status_Normal;
        Alarm_Function(ALARM_DISABLE);  
      }
    }
    else if (OxygenSaturaiton_Maximum - Current_OyxgenSat >= 1)
    {
      if (Current_Status!= Status_Normal)
      {
        Current_Status = Status_Normal;
        Alarm_Function(ALARM_DISABLE); 
      }
    }
    else
    {
      // Current Oxygen Saturaiton is between Maximum Oxygen Saturation and Minimum Oxygen Saturation
      if (Current_Status!= Status_Normal)
      {
        Current_Status = Status_Normal;
        Alarm_Function(ALARM_DISABLE); 
      }
    }
  }
  
}

// Alarm Function --------------------------------------------------------------
void Alarm_Function(uint8_t Command)
{
  if (Command == ALARM_ENABLE)
  {
    TIM_Cmd(TIM2, ENABLE);
  }
  else if (Command == ALARM_DISABLE)
  {
    TIM_Cmd(TIM2, DISABLE);
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

//------------------------------------------------------------------------------
void System_Init(void)
{
  SPI2_SetUp();
  LTC1661_Setup();
  OxygenSensor_Setup();
  Oxygen_PM_Setup();
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
  Timer6_SetUp();
  USART_GUI_Connect();
  USART_HyperTermianl_Connect();

  //INTTIM_Config();

  //Alarm Timer Setup
  Alarm_Timer_SetUp();
  
  // LED Set UP
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
  STM_EVAL_LEDOn(LED3);
  STM_EVAL_LEDOn(LED4);
  STM_EVAL_LEDOn(LED5);
  STM_EVAL_LEDOn(LED6);
  
  lcdInit();                                                                //LCD Set Up
 
  //SD Card : Check Mount Card
  if (f_mount(0, &filesystem) != FR_OK)
  {
    lcdString(4,1,"ERROR");
    lcdString(1,3,"Please inset SD Card");
  }

  /* Initialize USB available on STM32F4-Discovery board */
  USBD_Init(&USB_OTG_dev,
  #ifdef USE_USB_OTG_HS 
    USB_OTG_HS_CORE_ID,
  #else            
    USB_OTG_FS_CORE_ID,
  #endif  
    &USR_desc, 
    &USBD_CDC_cb,
    &USR_cb);
}

//------------------------------------------------------------------------------
void ConvertInttoString(uint8_t DataInt[])
{
  uint8_t i,j;
  for(i = 0; i < 50; i++)
  {
    j = i*5;
    SD_String[j] = '0' + (DataInt[i]/100);
    SD_String[j+1] = '0' + ((DataInt[i]%100)/10);
    SD_String[j+2] = '0' + ((DataInt[i]%10)/1);
    SD_String[j+3] = '\r';
    SD_String[j+4] = '\n';
  }
  
}
//------------------------------------------------------------------------------
//void INTTIM_Config(void)
//{
//  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//  NVIC_InitTypeDef NVIC_InitStructure;
//  /* Enable the TIM2 gloabal Interrupt */
//  NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
//   
//  /* TIM2 clock enable */
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
//  /* Time base configuration */
//  TIM_TimeBaseStructure.TIM_Period = 2000; // 1 MHz down to 1 KHz (1 ms)
//  TIM_TimeBaseStructure.TIM_Prescaler = 42000; // 24 MHz Clock down to 1 MHz (adjust per your clock)
//  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
//  /* TIM IT enable */
//  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
//  /* TIM2 enable counter */
//  TIM_Cmd(TIM6, ENABLE);
//  TIM_Cmd(TIM6, DISABLE);
//}
//void TIM6_DAC_IRQHandler(void)
//{
//  if (TIM_GetITStatus (TIM6, TIM_IT_Update) != RESET)
//  {
//    time = time + 1;
//    STM_EVAL_LEDOff(LED5);
//    TIM_ClearITPendingBit (TIM6, TIM_IT_Update);
//  }
//}

//------------------------------------------------------------------------------
/**
  * @brief  Configures EXTI Line0 (connected to PA0 pin) in interrupt mode
  * @param  None
  * @retval None
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



//------------------------------------------------------------------------------
static void fault_err (FRESULT rc)
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

// Delay ---------------------------------------------------------------------
/**
  * @brief  Delay
  * @param  None
  * @retval None
  */
static void Delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0; 
  for (index = (100000 * nCount); index != 0; index--);
}

//-----------------------------------------------------------------------------
void USART_HyperTermianl_Connect(void)
{  
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
  /*
    Use Port B Pin PD6 
    Use Port B Pin PD7 
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
  USART_InitStruct.USART_BaudRate = 115200;
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
  //ENABLE USART1 Interruper
  NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  /* Set Interrupt Mode*/
  //ENABLE the USART Receive Interrupt
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

  //Enable USART1
  USART_Cmd(USART3, ENABLE);
}
//------------------------------------------------------------------------------
void USART3_IRQHandler(void)
{
  uint16_t Drive_Data;
  if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
  {
    Drive_command_Data[index] = USART_ReceiveData(USART3);
    while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    USART_SendData(USART3, Drive_command_Data[index]); 
    while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
    index++;
    if (index >= 5)
    {
      index = 0;
      if (Drive_command_Data[0] == '1')
      {
        Drive_command_Data[0] = '0';
        Drive_Data = atoi(Drive_command_Data);
        SentData_DAC (Drive_Data, 1);
      }
      else if (Drive_command_Data[0] == '2')
      {
        Drive_command_Data[0] = '0';
        Drive_Data = atoi(Drive_command_Data);
        SentData_DAC (Drive_Data, 2);
      }
    } 
  }
  USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}

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
  TIM_TimeBaseStructure.TIM_Period = 2000; // 1 MHz down to 1 KHz (1 ms)
  TIM_TimeBaseStructure.TIM_Prescaler = 42000; // 24 MHz Clock down to 1 MHz (adjust per your clock)
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  /* TIM IT enable */
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  /* TIM2 ALARM_DISABLE counter */
  TIM_Cmd(TIM2, DISABLE);
}

void TIM2_IRQHandler(void)
{
  if (TIM_GetITStatus (TIM2, TIM_IT_Update) != RESET)
  {
    Time_AlarmLevel = Time_AlarmLevel + 1;
    STM_EVAL_LEDOff(LED5);
    if (Current_Status == Status_OxygenSat_Below_L1 | Current_Status == Status_OxygenSat_Behigh_L1)
    {
      if (Time_AlarmLevel >= Alarm_Level1)
      {
        Time_AlarmLevel = 0;
        if (Current_Status == Status_OxygenSat_Below_L1)
        {
          Current_Status = Status_OxygenSat_Below_L2;
          lcdString(1,5,"Status: Below L2");
        }
        else if (Current_Status == Status_OxygenSat_Behigh_L1)
        {
          Current_Status = Status_OxygenSat_Behigh_L2;
          lcdString(1,5,"Status: Behigh L2");
        }
      }
    }
    if (Current_Status == Status_OxygenSat_Below_L2 | Current_Status == Status_OxygenSat_Behigh_L2)
    {
      if (Time_AlarmLevel >= Alarm_Level2)
      {
        /* Notification Alarm Board (Toggle Pin to Alarm Circuit) */
      }
    }
    TIM_ClearITPendingBit (TIM2, TIM_IT_Update);
  }
}

// SD Card Section -------------------------------------------------------------
void Create_file(char Hospital_Number[], uint8_t File_Type)
{
  //ret = f_mount(0, &filesystem);
  for (int i = 0; i < 7; i++)
  {
    HospitalNumber_File[i] = Hospital_Number[i+6];
  }
    HospitalNumber_File[8] = '.';
    HospitalNumber_File[9] = 'T';
    HospitalNumber_File[10] = 'X';
    HospitalNumber_File[11] = 'T';
    HospitalNumber_File[12] = '\0';
  if(File_Type == 0)
  {
    HospitalNumber_File[7] = 'O';
    
    // Create Oxygen Saturation file
    ret = f_open(&file, HospitalNumber_File, FA_WRITE | FA_CREATE_ALWAYS);
    if (ret) 
    {
      fault_err(ret);
    } 
    else 
    {  
      ret = f_write(&file, "Hospital Number : ", 20, &bw);
      ret = f_lseek(&file,f_size(&file));
      ret = f_write(&file, HospitalNumber_File, 30, &bw);
      ret = f_lseek(&file,f_size(&file));
      ret = f_write(&file, "\r\nFile: Oxygen Saturation\r\n", 32, &bw);
      ret = f_close(&file);
    }  
  }
  else if (File_Type == 1)
  {
    HospitalNumber_File[7] = 'F';
    // Create FiO2 File
    ret = f_open(&file, HospitalNumber_File, FA_WRITE | FA_CREATE_ALWAYS);
    if (ret) 
    {
      fault_err(ret);
    } 
    else 
    {
      ret = f_write(&file, "Hospital Number : ", 20, &bw);
      ret = f_lseek(&file,f_size(&file));
      ret = f_write(&file, HospitalNumber_File, 30, &bw);
      ret = f_lseek(&file,f_size(&file));
      ret = f_write(&file, "\r\nFile: FiO2\r\n", 15, &bw);
      ret = f_close(&file);
    }  

  }
}
//------------------------------------------------------------------------------
void SD_Write(char FileName[], char SD_Data[], UINT Data_size)
{
  ret = f_open(&file, FileName, FA_WRITE);
  if (ret) 
  {
    fault_err(ret);
  } 
  else 
  {
    ret = f_lseek (&file,f_size(&file));
    ret = f_write(&file, SD_Data, Data_size, &bw);
    ret = f_close(&file);
  }  
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
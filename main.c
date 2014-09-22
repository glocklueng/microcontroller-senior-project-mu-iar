/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : main.c
Deverloper : Phattaradanai Kiratiwudhikul
Reseach & Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "DAC_LTC1661.h"
#include "MCP3202.h"
#include "Control_valve.h"
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
__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END;
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
// extern Profile Structger-----------------------------------------------------
extern Profile SProfile;

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
void delay(void);
static void system_init(void);
void Alarm_Function(uint8_t Command);
void Timer3_Config(void);

/* Private function prototypes -----------------------------------------------*/
void Convert_SaO2_InttoString(uint8_t DataInt[]);
void Convert_FiO2_FloattoString(float FiO2_float[], uint8_t Size_Buffer);
void USART_HyperTermianl_Connect(void);
void Create_file(char Hospital_Number[], uint8_t File_Type);

// Variable --------------------------------------------------------------------
unsigned char msg ;
char Character;
uint32_t count;
uint8_t uiSD_Test[50];
char cSD_String[50];
uint8_t index = 0;                                                                  //for count receving Data form Hyperterminal for controling Drive Circuit
char cDataFromOPM_TEST[3];

float FiO2_Current;
float FiO2_Buffer[10];
char cFiO2_Buffer_String[50];
char FiO2_Test_String[120];
uint8_t uiFiO2_index;
uint8_t uiPurpose_FiO2, uiDrive_FiO2;
float FiO2_percentage;

uint8_t Sampling_time;

extern uint8_t uiOxygenSat_buffer[10];
extern uint8_t uiSD_Card_index;
extern uint8_t uiRx_index_OPM;
extern uint8_t uiCurrent_SpO2;
extern float FiO2_DataTest[24];

uint8_t uiCurrent_Status;
uint8_t Time_AlarmLevel = 0;

// Profile Variable ------------------------------------------------------------
char HospitalNumber_File[13];
char Buffer[128];
  
// Main Function ---------------------------------------------------------------
int main()
{  

  /* Set Up config System*/
  system_init();
  lcdString (1,1,"Please Upload Profile");
  
  SProfile.uiProfile_Status = PROFILE_NOTUPLOAD;
   
  while(1)
  {    
//    printf("read the file (hello.txt)\n\r");
//    ret = f_open(&file, "2222222F.TXT", FA_READ);
//    if (ret) 
//    {
//      printf("2222222F.TXT file error\n\r");
//    } 
//    else 
//    {
//      printf("Type the file content(2222222F.TXT)\n\r");
//      for (;;) 
//      {
//        ret = f_read(&file, buff, sizeof(buff), &br);	/* Read a chunk of file */
//        if (ret || !br) 
//        {
//          break;			/* Error or end of file */
//        }
//        buff[br] = 0;
//        for(uint16_t length = 0; length < sizeof(buff); length++)
//        {
//          Buffer[length] = buff[length];
//          USART_SendData(USART6, Buffer[length]);
//          while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);
//        }
////        printf("%s",Buffer);
//        printf("\n\r");
//      }
//      if (ret) 
//      {
//        printf("Read file (2222222F.TXT) error\n\r");
//        fault_err(ret);
//      }
//      printf("Close the file (2222222F.TXT)\n\r");
//      ret = f_close(&file);
//      if (ret) 
//      {
//        printf("Close the file (2222222F.TXT) error\n\r");
//      }
//    }
    if (SProfile.uiProfile_Status == PROFILE_JUST_UPLOAD)
    {
//      USART_Cmd(OPM_USART, ENABLE);                                           // ENABLE Oxygen Pulse Meter USART
      Create_file(SProfile.cHospital_Number, OxygenSaturation_file);                      // Create Oxygen Saturation file
      Create_file(SProfile.cHospital_Number, FiO2_file);                                  // Create FiO2 file
      SProfile.uiProfile_Status = PROFILE_SETTING_COMPLETE;
//      TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
//      TIM_Cmd(TIM3, ENABLE);
      
      SentData_DAC(0x0000, Oxygen_Valve);
      SentData_DAC(0x0000, Air_Valve);
      uiPurpose_FiO2 = SProfile.uiPrefered_FiO2;
      NVIC_InitTypeDef   NVIC_InitStructure;

      /* Enable and Set Run_Button_EXTI Line Interrupt to the lowest priority */
      NVIC_InitStructure.NVIC_IRQChannel = Run_Button_IRQn;
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
      NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
      NVIC_Init(&NVIC_InitStructure);

    }
    else if (SProfile.uiProfile_Status == PROFILE_NOTUPLOAD)
    {
      USART_Cmd(OPM_USART, DISABLE);                                            // DISABLE Oxygen Pulse Meter USART
      SentData_DAC(0x00,3);                                                     // Close air and oxygen valve
    }

//------------------------------------------------------------------------------    
    //if (SProfile.uiProfile_Status == RUN_BUTTON_SET)
    if (SProfile.uiProfile_Status == PROFILE_SETTING_COMPLETE)
    {
      uiCurrent_Status = check_status(uiCurrent_SpO2);
     
//      /* Check Oxygen Saturation condition */
//      if (uiCurrent_SpO2 < SProfile.uiSpO2_Minimum)
//      {
//        /* Current Oxygen Saturation less than Minimum Oxygen Saturation */
//        if ((uiCurrent_Status != STATUS_SpO2_BELOW_L1) & (uiCurrent_Status != STATUS_SpO2_BELOW_L2) & (uiCurrent_Status != STATUS_ALARM))
//        {
//          Alarm_Function(ALARM_DISABLE);
//          uiCurrent_Status = STATUS_SpO2_BELOW_L1;
//          uiPurpose_FiO2 = SProfile.uiPrefered_FiO2 + 4;
//          if(uiPurpose_FiO2 > SProfile.uiFiO2_Maximum)
//          {
//            uiDrive_FiO2 = SProfile.uiFiO2_Maximum;
//            FiO2_Range(uiDrive_FiO2);
//          }
//          else
//          {
//            uiDrive_FiO2 = uiPurpose_FiO2;
//            FiO2_Range(uiDrive_FiO2);
//          }
//            
//          if(uiCurrent_Status != STATUS_ALARM)
//          {
//            Alarm_Function(ALARM_ENABLE);
//            lcdString(1,5,"Status: Below ");
//            lcdString(1,6,"Alarm Level 1");
//          }
//        }      
//      }
//      else if (uiCurrent_SpO2 > SProfile.uiSpO2_Maximum)
//      {
//        /* Current Oxygen Saturation more than maximum Oxygen Saturation */
//        if (uiCurrent_Status != STATUS_SpO2_BEHIGH_L1 & uiCurrent_Status != STATUS_SpO2_BEHIGH_L2 & uiCurrent_Status != STATUS_ALARM)
//        {
//          Alarm_Function(ALARM_DISABLE);
//          uiCurrent_Status = STATUS_SpO2_BEHIGH_L1;
//          uiPurpose_FiO2 = SProfile.uiPrefered_FiO2 - 4;
//          if (uiPurpose_FiO2 < SProfile.uiFiO2_Minimum)
//          {
//            uiDrive_FiO2 = SProfile.uiFiO2_Minimum;
//            FiO2_Range(uiDrive_FiO2);
//          }
//          else
//          {
//            uiDrive_FiO2 = uiPurpose_FiO2;
//            FiO2_Range(uiDrive_FiO2);
//          }
//          Alarm_Function(ALARM_ENABLE);
//          lcdString(1,5,"Status: Behigh");
//          lcdString(1,6,"Alarm Level 1");
//        }
//      }
//      else if (uiCurrent_SpO2 - SProfile.uiSpO2_Minimum == 1)
//      {
//        if (uiCurrent_Status != STATUS_NORMAL)
//        {
//          uiCurrent_Status = STATUS_NORMAL;
//          uiPurpose_FiO2 = SProfile.uiPrefered_FiO2 + 5;
//          if(uiPurpose_FiO2 > SProfile.uiFiO2_Maximum)
//          {
//            uiDrive_FiO2 = SProfile.uiFiO2_Maximum;
//            FiO2_Range(uiDrive_FiO2);
//          }
//          else
//          {
//            uiDrive_FiO2 = uiPurpose_FiO2;
//            FiO2_Range(uiDrive_FiO2);
//          }
//          lcdString(1,5,"Status: Normal");
//          lcdString(1,6,"              ");
//          Alarm_Function(ALARM_DISABLE);  
//        }
//      }
//      else if (SProfile.uiSpO2_Maximum - uiCurrent_SpO2 == 1)
//      {
//        if (uiCurrent_Status!= STATUS_NORMAL)
//        {
//          uiCurrent_Status = STATUS_NORMAL;
//          uiPurpose_FiO2 = SProfile.uiPrefered_FiO2 - 5;
//          if(uiPurpose_FiO2 < SProfile.uiFiO2_Minimum)
//          {
//            uiDrive_FiO2 = SProfile.uiFiO2_Minimum;
//            FiO2_Range(uiDrive_FiO2);
//          }
//          else
//          {
//            uiDrive_FiO2 = uiPurpose_FiO2;
//            FiO2_Range(uiDrive_FiO2);
//          }
//          lcdString(1,5,"Status: Normal");
//          lcdString(1,6,"              ");
//          Alarm_Function(ALARM_DISABLE); 
//        }
//      }
//      else if (uiCurrent_SpO2 >= SProfile.uiSpO2_Minimum & uiCurrent_SpO2 <= SProfile.uiSpO2_Maximum)
//      {
//        /* Current Oxygen Saturaiton is between Maximum Oxygen Saturation and Minimum Oxygen Saturation */
//        if (uiCurrent_Status!= STATUS_NORMAL)
//        {
//          uiDrive_FiO2 = SProfile.uiPrefered_FiO2;
//          FiO2_Range(uiDrive_FiO2);
//          lcdString(1,5,"Status: Normal");
//          lcdString(1,6,"              ");
//          uiCurrent_Status = STATUS_NORMAL;
//          Alarm_Function(ALARM_DISABLE); 
//        }
//        if (uiCurrent_SpO2 >= SProfile.uiSpO2_Minimum & uiCurrent_SpO2 < SProfile.uiSpO2_middleRange)
//        {
//          if (uiCurrent_Status != STATUS_MIDDLE_SpO2_BELOW)
//          {
//            uiCurrent_Status = STATUS_MIDDLE_SpO2_BELOW;
//          }
//        }
//*/
    }

//------------------------------------------------------------------------------
      /* Store uiOxygenSat_buffer in SD Card */
      if (uiSD_Card_index >= sizeof(uiOxygenSat_buffer))
      {
        HospitalNumber_File[7] = 'O';
        uiSD_Card_index = 0;
        Convert_SaO2_InttoString(uiOxygenSat_buffer);
        ret = f_open(&file_O, HospitalNumber_File, FA_WRITE);
        if (ret) 
        {
          fault_err(ret);
        } 
        else 
        {  
          ret = f_lseek(&file_O,f_size(&file_O));
          ret = f_write(&file_O, cSD_String, 50, &bw);
          ret = f_close(&file_O);
        }  
      }
      
      /* Store FiO2_Buffer in SD Card */
      if ((uiFiO2_index*4) >= sizeof(FiO2_Buffer))
      {
        HospitalNumber_File[7] = 'F';
        uiFiO2_index = 0;
        Convert_FiO2_FloattoString(FiO2_Buffer,10);
        ret = f_open(&file_F, HospitalNumber_File, FA_WRITE);
        if (ret)
        {
          fault_err(ret);
        } 
        else
        {  
          ret = f_lseek(&file_F,f_size(&file_F));
          ret = f_write(&file_F, cFiO2_Buffer_String, 50, &bw);
          ret = f_close(&file_F);
        } 
      }
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
//-------------------------------------------------------------------------------
/*
  Function : System_Init
  Input: None
  Return: None
  Description : Initial and Configurate all peripheral and driver
    - SPI2
    - DAC for LTC1661
    - ADC (Oxygen Sensor)
    - USART (connect to GUI, Oxygen Pulse Meter)
    - LCD Display (GLCD5110)
    - Timer
    - STM EVAL LED
    - SD Card
    - Button
*/
static void system_init(void)
{
  SPI2_SetUp();
  LTC1661_Setup();
  MCP3202_SetUp();
  OxygenSensor_Config();
  Oxygen_PM_Setup();
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
  lcdInit();                                                                    //LCD Set Up
  USART_GUI_Connect();                                                          //Set up USART for connecting GUI
  USART_HyperTermianl_Connect();
  Timer6_SetUp();
  FiO2_Check_Timer_Config();                                                    //Timer 3 will get ADC of FiO2 every 1 sec.
  
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
  

  // Button Set Up ---------------------------------------------------------
  // Button Config (Interrupt)
  //Button_EXTI_Config();
  EXTILine0_Config();                                                           // Set PA0 is input and config enable interrupt

  //SD Card : Check Mount Card8
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

//----------------- GPIO Interrupt Service Routine -------------------------
//Button Down IRQHandler --------------------------------------------------
//void Button_Down_IRQHandler(void)
//{
//  if(EXTI_GetITStatus(Button_Down_EXTI_Line) != RESET)
//  {
//    uiDrive_FiO2 = uiDrive_FiO2 - 5;
//    FiO2_Range(uiDrive_FiO2);
//    
//    delay_ms(80);
//    /* Clear the EXTI line pending bit */
//    EXTI_ClearITPendingBit(Button_Down_EXTI_Line);
//  }
//}


// Button Up IRQHandler ------------------------------------------------------
//void Button_Up_IRQHandler(void)
//{
//  if (EXTI_GetITStatus(Button_Up_EXTI_Line) != RESET)
//  {
//    uiDrive_FiO2 = uiDrive_FiO2 + 5;
//    FiO2_Range(uiDrive_FiO2);
//    
//    delay_ms(80);
//    /* Clear the EXTI line pending bit */
//    EXTI_ClearITPendingBit(Button_Up_EXTI_Line);
//  }
//}

// Run Button IRQHandler ------------------------------------------------------
void Run_Button_IRQHandler(void)
{
  if (EXTI_GetITStatus(Run_Button_EXTI_Line) != RESET)
  {
    if (SProfile.uiProfile_Status == PROFILE_SETTING_COMPLETE)
    {
      SProfile.uiProfile_Status = RUN_BUTTON_SET;
      USART_Cmd(OPM_USART, ENABLE); 
      TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
      TIM_Cmd(TIM3, ENABLE);
      FiO2_Range(SProfile.uiPrefered_FiO2);
    }
    else if (SProfile.uiProfile_Status == RUN_BUTTON_SET)
    {
      SProfile.uiProfile_Status = PROFILE_SETTING_COMPLETE;
      USART_Cmd(OPM_USART, DISABLE);    
      TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
      TIM_Cmd(TIM3, DISABLE);
      alarm_timer(TIMER_DISABLE);
      FiO2_Range(21);
      SentData_DAC(0, Oxygen_Valve);
      SentData_DAC(0, Air_Valve);
    }
    delay_ms(80);
    /* Clear the EXTI line pending bit */
    EXTI_ClearITPendingBit(Run_Button_EXTI_Line);
  }
}

// Alarm Button IRQHandler -----------------------------------------------------
//void Alarm_Button_IRQHandler(void)
//{
//  if (EXTI_GetITStatus(Alarm_Button_EXTI_Line) != RESET)
//  {
//    lcdClear();
//    lcdUpdate();
//    
//    Button_Up_Down_Init();                                                      // Config Button Up and down with Interrupt Vector
//    
//    delay_ms(60);
//    /* Clear the EXTI line pending bit */
//    EXTI_ClearITPendingBit(Alarm_Button_EXTI_Line);
//  }
//}
//------------------------------------------------------------------------------
/*
  Function : TIM3_IRQHandler
  Input : None
  Return: None
  Description : Timer 3 Interrupt Sevice Routine 
                FiO2 Analog to Digital Converter every 1 second.
*/
void TIM3_IRQHandler (void)
{
  uint8_t j;
  float current_FiO2_Sampling[5], AVG_FiO2_Current;
  
  if (TIM_GetITStatus (TIM3, TIM_IT_Update) != RESET)
  {
    if(Sampling_time >= 5)
    {
      STM_EVAL_LEDOff(LED5);
      for(j = 0; j < 5 ; j++)
      {
        current_FiO2_Sampling[j] = '\0';
        current_FiO2_Sampling[j] = Oxygen_convert();
        if(j == 4)
        {
          AVG_FiO2_Current = ((current_FiO2_Sampling[0] + current_FiO2_Sampling[1] + current_FiO2_Sampling[2] + current_FiO2_Sampling[3] + current_FiO2_Sampling[4])/5);
          FiO2_percentage = Convert_FiO2(AVG_FiO2_Current);
          FiO2_LCD_Display (FiO2_percentage);
          FiO2_Buffer[uiFiO2_index] = FiO2_percentage;
          uiFiO2_index++;
        }
      }
      Sampling_time = 0;
    }
    Sampling_time++;
  }
  STM_EVAL_LEDOn(LED5);
  TIM_ClearITPendingBit (TIM3, TIM_IT_Update);
}


//// Delay ---------------------------------------------------------------------
///**
//  * @brief  Delay
//  * @param  None
//  * @retval None
//  */
//static void Delay(__IO uint32_t nCount)
//{
//  __IO uint32_t index = 0; 
//  for (index = (100000 * nCount); index != 0; index--);
//}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/* USART 3: This function use for simulation Oxygen Pulse Meter */
/*
  Function : USART_HyperTermianl_Connect (USART3)
  @ Input : None
  @ Return: None
  Description : Configuration of USART3 for simulating Oxygen Pulse Meter
*/
//------------------------------------------------------------------------------
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
  
  /* Connect PD8 to USART3_Tx*/
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
  /* Connect PD9 to USART3_Rx*/
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
}
//------------------------------------------------------------------------------
// USART3 use for Debug and send command to Microcontroller
//------------------------------------------------------------------------------
//void USART3_IRQHandler(void)
//{
//  uint16_t Drive_Data;
//  if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
//  {
//    Drive_command_Data[index] = USART_ReceiveData(USART3);
//    while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
//    USART_SendData(USART3, Drive_command_Data[index]); 
//    while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
//    index++;
//    if (index >= 5)
//    {
//      index = 0;
//      if (Drive_command_Data[0] == '1')
//      {
//        Drive_command_Data[0] = '0';
//        Drive_Data = atoi(Drive_command_Data);
//        SentData_DAC (Drive_Data, 1);
//      }
//      else if (Drive_command_Data[0] == '2')
//      {
//        Drive_command_Data[0] = '0';
//        Drive_Data = atoi(Drive_command_Data);
//        SentData_DAC (Drive_Data, 2);
//      }
//    } 
//  }
//  USART_ClearITPendingBit(USART3, USART_IT_RXNE);
//}

//------------------------------------------------------------------------------
/* USART 3: This function use for simulation Oxygen Pulse Meter */
/*
  Function : USART3_IRQHandler
  @ Input : None
  @ Return: None
  Description : Simulate as Oxygen Pulse Meter. It will send Oxygen Saturation value.
*/
//------------------------------------------------------------------------------
void USART3_IRQHandler(void)
{
  if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
  {
    cDataFromOPM_TEST[uiRx_index_OPM] = USART_ReceiveData(USART3);
    while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    USART_SendData(USART3, cDataFromOPM_TEST[uiRx_index_OPM]); 
    while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
    uiRx_index_OPM++;
    if(uiRx_index_OPM >= 3)
    {  
      uiRx_index_OPM = 0;
      uiCurrent_SpO2 = atoi(cDataFromOPM_TEST);
      uiOxygenSat_buffer[uiSD_Card_index] = uiCurrent_SpO2;
      uiSD_Card_index++;
      if(uiCurrent_Status != STATUS_ALARM)
      {
        lcdString(7,2,cDataFromOPM_TEST);
        lcdString(10,2,"%  ");
      }
      
    }
    
    if((uiCurrent_Status == STATUS_ALARM) & (uiCurrent_SpO2 >= SProfile.uiSpO2_Minimum) & (uiCurrent_SpO2 <= SProfile.uiSpO2_Maximum))
    {
      uiCurrent_Status = STATUS_NORMAL;
      TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
      TIM_Cmd(TIM3, ENABLE);
      lcdClear();
      lcdUpdate();
      lcdString (1,1,SProfile.cHospital_Number);
      lcdString (1,2,"SaO2 : ");
      lcdString (7,2,cDataFromOPM_TEST);
      lcdString (10,2, "%");
      lcdString (1,5,"Status: Normal ");
      lcdString (1,6,"               ");
      if (SProfile.uiMode == 0xB7)
      {
        /* Select Range Mode */
        lcdString(1,4,"Mode: Range");
      }
      else if (SProfile.uiMode == 0xA2)
      {
        /* Selecte Auto Mode */
        lcdString(1,4,"Mode: Auto");
      }
    }
    
  }
  USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}

// SD Card Section -------------------------------------------------------------
void Create_file(char Hospital_Number[], uint8_t File_Type)
{
  //ret = f_mount(0, &filesystem);
  /* crate text file that have 7 last digit form Hospital Number(HN) */
  for (int i = 0; i < 7; i++)
  {
    HospitalNumber_File[i] = Hospital_Number[i+6];
  }
  HospitalNumber_File[8] = '.';
  HospitalNumber_File[9] = 'T';
  HospitalNumber_File[10] = 'X';
  HospitalNumber_File[11] = 'T';
  HospitalNumber_File[12] = '\0';
    
  /*
  @  File_Type = 0 : Oxygen Saturation record
  @  File_Type = 1 : FiO2 record
  */
  if(File_Type == 0)
  {
    HospitalNumber_File[7] = 'O';
    
    // Create Oxygen Saturation file
    ret = f_open(&file_O, HospitalNumber_File, FA_WRITE | FA_CREATE_ALWAYS);
    if (ret) 
    {
      /* ERROR */
      fault_err(ret);
    } 
    else 
    {  
      ret = f_write(&file_O, "Hospital Number : ", 20, &bw);
      ret = f_lseek(&file_O,f_size(&file_O));
      ret = f_write(&file_O, HospitalNumber_File, 13, &bw);
      ret = f_lseek(&file_O,f_size(&file_O));
      ret = f_write(&file_O, "\r\nFile: Oxygen Saturation\r\n", 27, &bw);
      ret = f_close(&file_O);
    }  
  }
  else if (File_Type == 1)
  {
    HospitalNumber_File[7] = 'F';
    // Create FiO2 File
    ret = f_open(&file_F, HospitalNumber_File, FA_WRITE | FA_CREATE_ALWAYS);
    if (ret) 
    {
      /* ERROR */
      fault_err(ret);
    } 
    else 
    {
      ret = f_write(&file_F, "Hospital Number : ", 20, &bw);
      ret = f_lseek(&file_F,f_size(&file_F));
      ret = f_write(&file_F, HospitalNumber_File, 13, &bw);
      ret = f_lseek(&file_F,f_size(&file_F));
      ret = f_write(&file_F, "\r\nFile: FiO2\r\n", 15, &bw);
      ret = f_close(&file_F);
    }
  }
}

//------------------------------------------------------------------------------
void Convert_SaO2_InttoString(uint8_t DataInt[])
{
  uint8_t i,j;
  for(i = 0; i < 10; i++)
  {
    j = i*5;
    cSD_String[j] = '0' + (DataInt[i]/100);
    cSD_String[j+1] = '0' + ((DataInt[i]%100)/10);
    cSD_String[j+2] = '0' + ((DataInt[i]%10)/1);
    cSD_String[j+3] = '\r';
    cSD_String[j+4] = '\n';
  }
}

//------------------------------------------------------------------------------
void Convert_FiO2_FloattoString(float FiO2_float[], uint8_t Size_Buffer)
{
  uint8_t i,j;
  for(i = 0; i < Size_Buffer; i++)
  {
    j = i*5;
    cFiO2_Buffer_String[j] = '0'+(uint32_t)FiO2_float[i]/100;
    cFiO2_Buffer_String[j+1] = '0'+((uint32_t)FiO2_float[i]%100)/10;
    cFiO2_Buffer_String[j+2] = '0'+((uint32_t)FiO2_float[i]%10)/1;
    cFiO2_Buffer_String[j+3] = '\r';
    cFiO2_Buffer_String[j+4] = '\n';
  }
}

//------------------------------------------------------------------------------
void SD_Write(char FileName[], char SD_Data[], unsigned int Data_size)
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
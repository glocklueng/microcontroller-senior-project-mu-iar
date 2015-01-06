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
#include "check_status_previous_version.h"
#include "alarm_condition_previous_version.h"
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

// variable for SD card --------------------------------------------------------
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
void Convert_SpO2_InttoString(uint8_t DataInt);
void Convert_FiO2_FloattoString(float FiO2_float);
void USART_HyperTermianl_Connect(void);
void Create_file(char Hospital_Number[]);
void SD_Write(char FileName[], char cDataTimeSD[], uint8_t uiSpO2_SD, float fFiO2_SD, uint8_t uiStatus);
// Variable --------------------------------------------------------------------
unsigned char msg ;
char Character;
uint32_t count;
uint8_t uiSD_Test[50];
uint8_t uiIndex_SD_buffer;                                                      // uiIndex_SD_buffer variable for checking data in buffer
uint8_t index = 0;                                                              //for count receving Data form Hyperterminal for controling Drive Circuit
char cDataFromOPM_TEST[3];

float FiO2_Current;
float fFiO2_Percent;
float fFiO2_Buffer[10];
char FiO2_Test_String[120];
uint8_t uiFiO2_index;
uint8_t uiPurpose_FiO2, uiDrive_FiO2;
float FiO2_percentage;

uint8_t Sampling_time;

extern uint8_t uiOxygenSat_buffer[10];
extern uint8_t uiSD_Card_index;
extern uint8_t uiCurrent_SpO2;
extern uint8_t uiInitial_SpO2;
extern float FiO2_DataTest[24];
extern bool bReadCorrect;
extern bool bReceiveCorrect;

/* Variable for store in SD card */
extern char cDateTime[17][3];                                                   //from file : Oxygen_Pulse_Meter.c
uint8_t uiSpO2_SDcard_buffer[3];
float fFiO2_SDcard_buffer[3];
float fFiO2_SDCard_buffer_sim[3];                                               // FiO2 buffer for store data in case of simulation
uint8_t uiStatus_Buffer[3];
char cFiO2_SDcard[6];
char cSpO2_SDcard[3];

uint8_t uiCurrent_Status;

// Profile Variable ------------------------------------------------------------
char cHospitalNumber_File[13];
char Buffer[128];
  
// Main Function ---------------------------------------------------------------
int main()
{
  /* Set Up config  System*/
  system_init();
  lcdString (1,1,"Please Upload Profile");
  
  SProfile.uiProfile_Status = PROFILE_NOTUPLOAD;
   
  while(1)
  {    
    if (SProfile.uiProfile_Status == PROFILE_JUST_UPLOAD)
    {
      USART_Cmd(USART3, ENABLE);                                                // ENABLE Oxygen Pulse Meter USART
      Create_file(SProfile.cHospital_Number);                                   // Create textfile
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
      USART_Cmd(USART3, DISABLE);                                               // DISABLE Oxygen Pulse Meter USART
      SentData_DAC(0x00,3);                                                     // Close air and oxygen valve
    }

//------------------------------------------------------------------------------    
    //if (SProfile.uiProfile_Status == RUN_BUTTON_SET)
    if (SProfile.uiProfile_Status == PROFILE_SETTING_COMPLETE)
    {
      uiCurrent_Status = check_status_previous_version(uiCurrent_SpO2);         // test algorithm old control model
      
//      uiCurrent_Status = check_status(uiCurrent_SpO2);                        // test algorithm new control model
      uiStatus_Buffer[uiSD_Card_index] = uiCurrent_Status;
    }

//------------------------------------------------------------------------------
//    if(bReceiveCorrect == true)
//    {
//      bReceiveCorrect = false;
//      uiCurrent_SpO2 = Get_OxygenSat();
//      if(bReadCorrect == true)
//      {
//        uiSpO2_SDcard_buffer[uiSD_Card_index] = uiCurrent_SpO2;                 // Store SpO2 in Buffer
//        uiSD_Card_index++;
//      }
//    }
      /* Store uiOxygenSat_buffer in SD Card */
      if (uiSD_Card_index >= sizeof(uiSpO2_SDcard_buffer))
      {
        uiSD_Card_index = 0;
        char cDateTimeBuffer[17];
        for (int i = 0; i < 3; i++)
        {
          for (int j =0; j < 17; j++)
          {
            cDateTimeBuffer[j] = cDateTime[j][i];
          }
          SD_Write(cHospitalNumber_File, cDateTimeBuffer, uiSpO2_SDcard_buffer[i], fFiO2_SDCard_buffer_sim[i], uiStatus_Buffer[i]);
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
//------------------------------------------------------------------------------
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
  /* Setup NVIC group */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    
  SPI2_SetUp();
  LTC1661_Setup();
  MCP3202_SetUp();
  OxygenSensor_Config();
  usart_OPM_setup();
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
  lcdInit();                                                                    //LCD Set Up
  USART_GUI_Connect();                                                          //Set up USART for connecting GUI
//  USART_HyperTermianl_Connect();
  Timer6_SetUp();
  timer7_setup();                                                               // Timer7 use count for setting sampling rate 10 Hz
  FiO2_Check_Timer_Config();                                                    //Timer 3 will get ADC of FiO2 every 1 sec.
  
  timer4_setup();
  
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
  float fFiO2_avg;
  
  if (TIM_GetITStatus (TIM3, TIM_IT_Update) != RESET)
  {
    /* Sampling 10 Hz */
    TIM_Cmd(TIM7, ENABLE);
    /* wait unit 10 ms */
    while(index < 10)
    {
      fFiO2_avg = 0;
      /* Set ADC every 10 ms */
      if(TIM_GetFlagStatus(TIM7, TIM_FLAG_Update) != RESET)
      {
        fFiO2_Buffer[index] = Oxygen_convert();                                 // Function :Oxygen_convert return voltage value from ADC to fFiO2_Buffer
        index++;
        TIM_ClearFlag(TIM7, TIM_FLAG_Update);
      }
    }
    TIM_Cmd(TIM7, DISABLE);

    index = 0;
    fFiO2_avg = 0;

    /* Avg. FiO2 value */
    for(uint8_t index_buffer = 0; index_buffer < 10; index_buffer++)
    {
      fFiO2_avg = fFiO2_avg + fFiO2_Buffer[index_buffer];
    }
    fFiO2_avg = fFiO2_avg/10.0;                                                 // Avarage fFiO2 value
    fFiO2_SDcard_buffer[uiSD_Card_index]  = Convert_FiO2(fFiO2_avg);            // Converting voltage value to FiO2 percentage
    
    /* Clear Buffer */
    for(uint8_t index_buffer = 0; index_buffer < 10; index_buffer++)
    {
      fFiO2_Buffer[index_buffer] = 0;
    }
  }
  fFiO2_SDCard_buffer_sim[uiSD_Card_index] = (float)(uiPurpose_FiO2);           // using in simulation
  STM_EVAL_LEDOn(LED5);
  TIM_ClearITPendingBit (TIM3, TIM_IT_Update);
}
//------------------------------------------------------------------------------
/* USART 3: This function use for simulation Oxygen Pulse Meter */
/*
  Function : USART_HyperTermianl_Connect (USART3)
  @ Input : None
  @ Return: None
  Description : Configuration of USART3 for simulating Oxygen Pulse Meter
                Baud rate 9600, 8-N-1, Enable Rx only, Enable Rx interrupt
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
  USART_InitStruct.USART_BaudRate = 9600;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_Mode = USART_Mode_Rx;
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
  USART_Cmd(USART3, DISABLE);
}
//USART_IRQHandler -------------------------------------------------------------
///*
//  Function : OPM_IRQHandler
//  Input : None
//  Output : None
//  Description : Interrupt Service Routine from OPM_USART
//                Enable IT source : USART_IT_RXNE
//*/
//void USART3_IRQHandler(void)
//{
//  if(USART_GetITStatus(OPM_USART, USART_IT_RXNE) != RESET)
//  {
//    if (uiRx_index_OPM == 0)
//    {
//      //Start Receive Data from Oxygen Pulse Meter
//      TIM_Cmd(TIM4, DISABLE);
//    }
//    ucDataFromOPM[uiRx_index_OPM] = USART_ReceiveData(OPM_USART);
//    uiRx_index_OPM = uiRx_index_OPM + 1;
//    if(uiRx_index_OPM == (sizeof(ucDataFromOPM)))
//    {  
//      TIM_Cmd(TIM4, DISABLE);
//      uiRx_index_OPM = 0;
//      uiCurrent_SpO2 = Get_OxygenSat();
//      if(bReadCorrect == true)
//      {
//        uiSpO2_SDcard_buffer[uiSD_Card_index] = uiCurrent_SpO2;
//        uiSD_Card_index++;
//      }
//    }
//  }
//  if(USART_GetITStatus(OPM_USART, USART_IT_TXE) != RESET)
//  {
//    USART_ITConfig(OPM_USART, USART_IT_TXE, DISABLE);                                   // Disable USART_IT_TXE flag
//    USART_ClearITPendingBit(OPM_USART, USART_IT_TXE);                                   // Clear  USART_IT_TXE flag
//  }
//  USART_ClearITPendingBit(OPM_USART, USART_IT_RXNE);
//}
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
//void USART3_IRQHandler(void)
//{
//  if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
//  {
//    cDataFromOPM_TEST[uiRx_index_OPM] = USART_ReceiveData(USART3);
//    while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
//    USART_SendData(USART3, cDataFromOPM_TEST[uiRx_index_OPM]); 
//    while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
//    uiRx_index_OPM++;
//    if(uiRx_index_OPM >= 3)
//    {  
//      uiRx_index_OPM = 0;
//      uiCurrent_SpO2 = atoi(cDataFromOPM_TEST);
//      uiOxygenSat_buffer[uiSD_Card_index] = uiCurrent_SpO2;
//      uiSD_Card_index++;
//      if(uiCurrent_Status != STATUS_ALARM)
//      {
//        lcdString(7,2,cDataFromOPM_TEST);
//        lcdString(10,2,"%  ");
//      }
//      
//    }
//    
//    if((uiCurrent_Status == STATUS_ALARM) & (uiCurrent_SpO2 >= SProfile.uiSpO2_Minimum) & (uiCurrent_SpO2 <= SProfile.uiSpO2_Maximum))
//    {
//      uiCurrent_Status = STATUS_NORMAL;
//      TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
//      TIM_Cmd(TIM3, ENABLE);
//      lcdClear();
//      lcdUpdate();
//      lcdString (1,1,SProfile.cHospital_Number);
//      lcdString (1,2,"SaO2 : ");
//      lcdString (7,2,cDataFromOPM_TEST);
//      lcdString (10,2, "%");
//      lcdString (1,5,"Status: Normal ");
//      lcdString (1,6,"               ");
//      if (SProfile.uiMode == 0xB7)
//      {
//        /* Select Range Mode */
//        lcdString(1,4,"Mode: Range");
//      }
//      else if (SProfile.uiMode == 0xA2)
//      {
//        /* Selecte Auto Mode */
//        lcdString(1,4,"Mode: Auto");
//      }
//    }
//    
//  }
//  USART_ClearITPendingBit(USART3, USART_IT_RXNE);
//}

// SD Card Section -------------------------------------------------------------
/*
Function : Create_file
Input : char Hospital_Number[], uint8_t File_Type
Output : None
Description : Create text file in SD card. The filename is last 8 digit of HN (Limitation of FATFS)
              Start of textfile has Profile setting.
*/
void Create_file(char Hospital_Number[])
{
  //ret = f_mount(0, &filesystem);
  /* crate text file that have 8 last digit form Hospital Number(HN) */
  for (int i = 0; i < 8; i++)
  {
    cHospitalNumber_File[i] = Hospital_Number[i+5];
  }
  cHospitalNumber_File[8] = '.';
  cHospitalNumber_File[9] = 'T';
  cHospitalNumber_File[10] = 'X';
  cHospitalNumber_File[11] = 'T';
  cHospitalNumber_File[12] = '\0';
    
  /* Convert int to String */
  char cSpO2_Minimum[3]; 
  char cSpO2_Maximum[3];
  char cFiO2_Minimum[3];
  char cFiO2_Maximum[3];
  char cPrefered_FiO2[3];
  char cRespondsTime[3];
  char cAlarm_Level1[3];
  char cAlarm_Level2[3];

  cSpO2_Minimum[0] = '0' + (SProfile.uiSpO2_Minimum / 100); 
  cSpO2_Minimum[1] = '0' + ((SProfile.uiSpO2_Minimum % 100)/10); 
  cSpO2_Minimum[2] = '0' + ((SProfile.uiSpO2_Minimum % 10)/1);

  cSpO2_Maximum[0] = '0' + (SProfile.uiSpO2_Maximum / 100);
  cSpO2_Maximum[1] = '0' + ((SProfile.uiSpO2_Maximum % 100)/10);
  cSpO2_Maximum[2] = '0' + ((SProfile.uiSpO2_Maximum % 10)/1);
 
  cFiO2_Minimum[0] = '0' + (SProfile.uiFiO2_Minimum / 100);
  cFiO2_Minimum[1] = '0' + ((SProfile.uiFiO2_Minimum % 100)/10);
  cFiO2_Minimum[2] = '0' + ((SProfile.uiFiO2_Minimum % 10)/1);

  cFiO2_Maximum[0] = '0' + (SProfile.uiFiO2_Maximum / 100);
  cFiO2_Maximum[1] = '0' + ((SProfile.uiFiO2_Maximum % 100)/10);
  cFiO2_Maximum[2] = '0' + ((SProfile.uiFiO2_Maximum % 10)/1);


  cPrefered_FiO2[0] = '0' + (SProfile.uiPrefered_FiO2 / 100);
  cPrefered_FiO2[1] = '0' + ((SProfile.uiPrefered_FiO2 % 100) / 10);
  cPrefered_FiO2[2] = '0' + ((SProfile.uiPrefered_FiO2 % 10) / 1);

  cRespondsTime[0] = '0' + (SProfile.uiRespondsTime / 100);
  cRespondsTime[1] = '0' + ((SProfile.uiRespondsTime % 100) / 10);
  cRespondsTime[2] = '0' + ((SProfile.uiRespondsTime % 10) / 1);

  cAlarm_Level1[0] = '0' + (SProfile.uiAlarm_Level1 / 100);
  cAlarm_Level1[1] = '0' + ((SProfile.uiAlarm_Level1 % 100)/10);
  cAlarm_Level1[2] = '0' + ((SProfile.uiAlarm_Level1 % 10)/1);

  cAlarm_Level2[0] = '0' + (SProfile.uiAlarm_Level2 / 100);
  cAlarm_Level2[1] = '0' + ((SProfile.uiAlarm_Level2 % 100)/10);
  cAlarm_Level2[2] = '0' + ((SProfile.uiAlarm_Level2 % 10)/1);

  // Create Oxygen Saturation file
  ret = f_open(&file, cHospitalNumber_File, FA_WRITE | FA_CREATE_ALWAYS);
  if (ret) 
  {
    /* ERROR */
    fault_err(ret);
  } 
  else 
  {  
    /* Write Header of File and record profile setting */
    ret = f_write(&file, "Hospital Number : ", 20, &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, cHospitalNumber_File, 13, &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, "\r\nProfile Setting\r\n", 19, &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, "Minimum SpO2 : ", 15, &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, cSpO2_Minimum, sizeof(cSpO2_Minimum), &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, " %\r\nMaximum SpO2 : ", 19, &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, cSpO2_Maximum, sizeof(cSpO2_Maximum), &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, " %\r\nPrepered FiO2 : ", 20, &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, cPrefered_FiO2, sizeof(cPrefered_FiO2), &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, " %\r\nMinimum FiO2 : ", 19, &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, cFiO2_Minimum, sizeof(cFiO2_Minimum), &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, " %\r\nMaximum FiO2 : ", 19, &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, cFiO2_Maximum, sizeof(cFiO2_Maximum), &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, " %\r\nResponse time : ", 20, &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, cRespondsTime, sizeof(cRespondsTime), &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, " sec\r\nAlarm Level 1 : ", 22, &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, cAlarm_Level1, sizeof(cAlarm_Level1), &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, " sec\r\nAlarm Level 2 : ", 22, &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, cAlarm_Level2, sizeof(cAlarm_Level2), &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, " sec\r\n\r\n", 8, &bw);
    ret = f_close(&file);
    if (ret)
    {
      fault_err(ret);   //Error
    }
  }  

}
//------------------------------------------------------------------------------
/*
Function : Convert_SpO2_IntiString
Input : uint8_ DataInt
Output : None
Description : Convert unsigned int 8 bit SpO2 to String 3 Bytes in cSpO2_SDcard variable for writing to SDcard.
*/
void Convert_SpO2_InttoString(uint8_t DataInt)
{
  cSpO2_SDcard[0] = '0' + (DataInt / 100);
  cSpO2_SDcard[1] = '0' + ((DataInt % 100)/10);
  cSpO2_SDcard[2] = '0' + ((DataInt % 10)/1);
}

//------------------------------------------------------------------------------
/*
Function : Convert_FiO2_FloattoString 
Input : float FiO2_float
Output : None
Description : Convert FiO2 in float variable to String cFiO2_SDcard 5 bytes (2 digit decimal).
*/
void Convert_FiO2_FloattoString(float FiO2_float)
{ 
  cFiO2_SDcard[0] = '0' + (uint32_t)FiO2_float / 100;
  cFiO2_SDcard[1] = '0' + ((uint32_t)FiO2_float % 100) /10;
  cFiO2_SDcard[2] = '0' + ((uint32_t)FiO2_float % 10) /1;
  cFiO2_SDcard[3] = '.';
  cFiO2_SDcard[4] = '0' + ((uint32_t)FiO2_float * 10) % 10;
  cFiO2_SDcard[5] = '0' + ((uint32_t)FiO2_float * 100) % 10;
}

//------------------------------------------------------------------------------
/*
Function : SD_Write
Input : char cFileName[], char cDataTimeSD[], uint8_t uiSpO2_SD_buffer[], float fFiO2_SD_buffer[]
        uint8_t uiStatus;
Output : None
Description : 
*/
void SD_Write(char FileName[], char cDataTimeSD[], uint8_t uiSpO2_SD, float fFiO2_SD, uint8_t uiStatus)
{
  char cStatus[2];

  /* Convert to String */
  Convert_SpO2_InttoString(uiSpO2_SD);
  Convert_FiO2_FloattoString(fFiO2_SD);

  /* check Status */
  switch(uiStatus)
  {
  case STATUS_NORMAL :
    strcpy(cStatus, "NN");
    break;
  case STATUS_SpO2_BELOW_L1:
    strcpy(cStatus ,"L1");
    break;
  case STATUS_SpO2_BELOW_L2:
    strcpy(cStatus, "L2");
    break;
  case STATUS_SpO2_BEHIGH_L1:
    strcpy(cStatus, "H1");
    break;
  case STATUS_SpO2_BEHIGH_L2:
    strcpy(cStatus, "H2");
    break;
  case STATUS_ALARM:
    strcpy(cStatus, "AA");
    break;
  case STATUS_MIDDLE_SpO2_BELOW:
    strcpy(cStatus, "ML");
    break;
  case STATUS_MIDDLE_SpO2_BEHIGH:
    strcpy(cStatus, "MH");
    break;
  case STATUS_MIDDLE_SpO2:
    strcpy(cStatus, "MM");
    break;
  case STATUS_SpO2_BEHIGH_ALARM_L1:
    strcpy(cStatus, "H2");
    break;
  case STATUS_SpO2_BELOW_ALARM_L1:
    strcpy(cStatus, "L2");
    break;
  }
    
//  if (uiStatus == STATUS_NORMAL)
//  {
//    strcpy(cStatus, "NN");
//  }
//  else if (uiStatus == STATUS_SpO2_BELOW_L1)
//  {
//    strcpy(cStatus, "L1");
//  }
//  else if (uiStatus == STATUS_SpO2_BELOW_L2)
//  {
//    strcpy(cStatus, "L2");
//  }
//  else if (uiStatus == STATUS_SpO2_BEHIGH_L1)
//  {
//    strcpy(cStatus, "H1");
//  }
//  else if (uiStatus == STATUS_SpO2_BEHIGH_L2)
//  {
//    strcpy(cStatus, "H2");
//  }
//  else if (uiStatus == STATUS_ALARM)
//  {
//    strcpy(cStatus, "AA");
//  }
//  else if (uiStatus == STATUS_MIDDLE_SpO2_BELOW)
//  {
//    strcpy(cStatus, "ML");
//  }
//  else if (uiStatus == STATUS_MIDDLE_SpO2_BEHIGH)
//  {
//    strcpy(cStatus, "MH");
//  }
//  else if (uiStatus == STATUS_MIDDLE_SpO2)
//  {
//    strcpy(cStatus, "MM");
//  }

  /* Start write data to SD card*/
  ret = f_open(&file, FileName, FA_WRITE);
  if (ret) 
  {
    fault_err(ret);
  } 
  else 
  {
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, cDataTimeSD, 17, &bw);     // DataTime
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, " SpO2 = ", 8, &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, cSpO2_SDcard, sizeof(cSpO2_SDcard), &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, " % FiO2 = ", 10, &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, cFiO2_SDcard, sizeof(cFiO2_SDcard), &bw); //FiO2
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, " % STATUS = ", 12, &bw);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, cStatus, 2, &bw);   //Status
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, "\r\n", 2, &bw);
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
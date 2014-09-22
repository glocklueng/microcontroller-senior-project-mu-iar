/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : Connect_GUI.c

Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University

*/
//------------------------------------------------------------------------------
/*
    Connect to GUI USE USART1 via RS-232 Protocol
    USART1 - Tx -> Port D Pin PB6
    USART1 - Rx -> Port D Pin PB7
    Baud Rate = 115200
    package = 8-n-1
    
*/
/*
           Package of Data 
         * Length 28 Bytes
         * Byte 0 : Head of Bytes : "$"
         * Byte 1 : Command : Connect (0xE8), Upload (0xD5)
         * Byte 2 : SOH     : 0xFF
         * Byte 3 - 24: Data (if don't have data replace with Padding Bytes (0x23) (Padding Bytes : 22 - Data))
         *      1.) Hospital Number 13 Bytes (Bytes : 3-15)
         *      2.) Oxygen Saturation max (1 Byte) (Byte: 16)
         *      3.) Oxygen Saturation min (1 Byte) (Byte: 17)
         *      4.) Respond time (1 Byte) (Byte: 18)
         *      5.) Prefered FiO2 (1 Byte)(Byte: 19)
         *      6.) Select Mode (Byte: 20)
         *              Range Mode(0xB7)
         *              Auto Mode (0xA2)
         *      7.) if selected Range mode : FiO2 Max (Byte 21), FiO2 Min (Byte 22)
         *          else selected Auto mode: Padding (0x91) (2 Bytes) (Byte : 21-22)
         *      8.) Alarm Level 1(Byte 23) and Alarm Level 2 (Byte 24)
         * Byte 25-26: CRC-16 Modbus(2 Bytes) (Byte 25: CRC-High, Byte 26: CRC-Low)
         * Byte 27 : End of Transmission (ETX) (0x03)
         *      
         *       
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "Connect_GUI.h"
#include "DefinePin.h"
#include "GLCD5110.h"
#include "SD_Card.h"

//------------------------------------------------------------------------------
#define ERROR_ACK    0x65                                                       //Data Error = 'e' (0x65)
#define ACK          0x41                                                       //Data Correct = 'A' (0x41)
#define FOUND        0xC3                                                       //Found Profile Record in SD card = 0xC3
#define NOTFOUND     0x45                                                       //Cannot found Profile Record of this HN
#define OxygenSaturation_file           0
#define FiO2_file                       1
// Define Structure ---------------------------------------------------------------
Profile SProfile;
//------------------------------------------------------------------------------

uint8_t tx_index_GUI=0;

static uint8_t suiData_GUI[28];

extern char cSD_String[50];
extern char HospitalNumber_File[13];

extern char Buffer[128];

//extern uint8_t uiPurpose_FiO2;

// extern variable for SD card
/* Private typedef -----------------------------------------------------------*/
//extern SD_Error Status = SD_OK;
extern FATFS filesystem;		                                                    // volume lable
extern FRESULT ret;			                                                        // Result code
extern FIL file_F, file_O, file;		                                            // File object
extern DIR dir;				                                                          // Directory object
extern FILINFO fno;			                                                        // File information object
extern UINT bw, br;
extern uint8_t buff[128];

//Define Variable for CRC ------------------------------------------------------
uint16_t Crc;
uint8_t CRC_Low, CRC_High;
uint8_t Length_Data = 25;
uint8_t uiRx_Index_GUI = 0;
//------------------------------------------------------------------------------
//Define Value for Data Packaging
const uint8_t kPadding = 0x91;                                                  //Padding Bytes for dummy Bytes of Data (Value = 0x23)
const uint8_t kConnect_Command = 0xE8;                                          //0xE8 is Connect Command
const uint8_t kUpload_Command = 0xD5;                                           //0xD5 is Upload Profile data Command
const uint8_t kRead_Command = 0x3A;                                             //0x3A is Read data form SD card for plot graph on GUI
const uint8_t kETX = 0x33;                                                      //End of Package Transmittion
//const char ERROR_ACK = 'e';  

// Extern Profile Variable -----------------------------------------------------
//extern char Hospital_Number[13];
//extern uint8_t OxygenSaturaiton_Maximum, OxygenSaturation_Minimum;
//extern uint8_t FiO2_Maximum, FiO2_Minimum;
//extern uint8_t RespondsTime;
//extern uint8_t Prefered_FiO2;
//extern uint16_t Alarm_Level1, Alarm_Level2;
//extern uint8_t Mode;
//extern uint8_t Profile_Status;

//------------------------------------------------------------------------------
/*
  Function : USART_GUI_Connect
  Input : None
  Output: None
  Description : Configuration USART1 for connecting with GUI
                Baud Rate = 115200
                Tx Pin : PB6
                Rx Pin : PB7
*/
void USART_GUI_Connect(void)
{  
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;
	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
  /*
    Use Port B Pin PD6 
    Use Port B Pin PD7 
  */
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  /* Connect PXx to USARTx_Tx*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
  
  
  /* USART_InitStruct members default value */
  USART_InitStruct.USART_BaudRate = 115200;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
  USART_Init(USART1, &USART_InitStruct);
  
  /*USART Interrupt*/
  /* Set interrupt: NVIC_Setup */
  NVIC_InitTypeDef NVIC_InitStruct;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  //ENABLE USART1 Interruper
  NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  /* Set Interrupt Mode*/
  //ENABLE the USART Receive Interrupt
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

  //Enable USART1
  USART_Cmd(USART1, ENABLE);
  
//  //Set Up Timer 3
//  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//  NVIC_InitTypeDef NVIC_InitStructure;
//  //Enable Clock Timer3
//  RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM3, ENABLE);
//
//  /* Enable the TIM3 gloabal Interrupt */
//  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 8;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
//  /*
//
//  Pre-Scale : APB1 Prescale 4
//  System Clock 168MHz /4 = 42 MHz
//  Timer Prescale 4200
//  */
//  /* Time base configuration */
//  TIM_TimeBaseStructure.TIM_Period = 20;            
//  TIM_TimeBaseStructure.TIM_Prescaler = 42000;                                // 42 MHz Clock down to 1 kHz (adjust per your clock)
//  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
//  /* TIM IT enable */
//  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
//  /* TIM3 enable counter */
//  TIM_Cmd(TIM3, DISABLE);
}

// CRC Calculate ---------------------------------------------------------------
/*
    CRC 16 - ModBus 
    Input : None
    Output: None
    Description : Calculate CRC-16 (ModBus Standard), Length 2 Bytes (CRC-High, CRC-Low), Polynomial(0xA001)
*/
void CRC_CALCULATE_TX(void)
{  
  uint8_t i;
  Crc = 0xFFFF;
  for (i = 0; i < Length_Data; i++) 
  {
    Crc = TX_CRC(Crc , suiData_GUI[i]);
  }
  CRC_Low = (Crc & 0x00FF);                                                     //Low byte calculation
  CRC_High = (Crc & 0xFF00)/256;                                                //High byte calculation
} 

unsigned int TX_CRC(unsigned int crc, unsigned int data)
{
  const unsigned int Poly16 = 0xA001;
  unsigned int LSB;
  uint8_t i;
  
  crc = ((crc^data) | 0xFF00) & (crc | 0x00FF);
  for (i=0; i<8; i++) 
  {
    LSB = (crc & 0x0001);
    crc = crc/2;
    
    if (LSB)
      crc=crc^Poly16;
    
    /*
    if (LSB == 0x0001)
    {
      crc=crc^Poly16;
    }
    */
  }
  return(crc);
}

// GUI Interrupt Service Routine -----------------------------------------------
/*
  Function : GUI_IRQHandler
  Input : None
  Return: None
  Description : USART Interrupt Service Routine - get profile data in buffer and calculate CRC-16 (Modbus)
                if data is correct, it will update profile with Update_Rule() Function
                if data is error, it will abandon data in buffer
*/
void GUI_IRQHandler (void)
{
  uint8_t Data_in;

  if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
  {
    Data_in = USART_ReceiveData(USART1);
    suiData_GUI[uiRx_Index_GUI] = Data_in;  
    uiRx_Index_GUI++;
  
    if(uiRx_Index_GUI == (sizeof(suiData_GUI) - 1))
    {  
      uiRx_Index_GUI = 0;

      /* Check CRC16 - ModBus */
      CRC_CALCULATE_TX();

      if (suiData_GUI[25] == CRC_High & suiData_GUI[26] == CRC_Low)
      {
        /* CRC is Correct */
        USART_SendData(USART1, ACK);                                            //Send Acknowlege to GUI
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        
        if (suiData_GUI[1] == kUpload_Command)
        {
          lcdClear();
          /* Update Rule */
          Update_Rule();
          SProfile.uiProfile_Status = PROFILE_JUST_UPLOAD;
          lcdString(1,2,"SaO2: ");
          lcdString(1,3,"FiO2:");
        }
        else if (suiData_GUI[1] == kConnect_Command)
        {
          /* Clear Data out from Buffer */
          for (uint8_t uiClear_index = 0; uiClear_index < sizeof(suiData_GUI); uiClear_index++)
          {
            suiData_GUI[uiClear_index] = 0;
          }
        }
        else if (suiData_GUI[1] == kRead_Command)
        {
          /* find profile Record */
          ret = f_open(&file_O, HospitalNumber_File, FA_READ);
//        if (ret) 
//        {
//          printf("2222222F.TXT file error\n\r");
//        } 
//        else 
          if (ret == 0)
          {
            printf("Type the file content(2222222F.TXT)\n\r");
            for (;;) 
            {
              ret = f_read(&file, buff, sizeof(buff), &br); /* Read a chunk of file */
              if (ret || !br) 
              {
                break;      /* Error or end of file */
              }
              buff[br] = 0;
              for(uint16_t length = 0; length < sizeof(buff); length++)
              {
                Buffer[length] = buff[length];
                USART_SendData(USART6, Buffer[length]);
                while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);
              }
//          printf("%s",Buffer);
              printf("\n\r");
            }
            if (ret) 
            {
              printf("Read file (2222222F.TXT) error\n\r");
              fault_err(ret);
            }
            printf("Close the file (2222222F.TXT)\n\r");
            ret = f_close(&file);
            if (ret) 
            {
              printf("Close the file (2222222F.TXT) error\n\r");
            }
            /* Send ACK to GUI*/

          }
        }
      }
      else
      {
        /* CRC is ERROR, Send ERROR ACK to GUI (ERROR ACK = 0xEA ('e')) */
        USART_SendData(USART1, ERROR_ACK);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
      }
    }
  }
  
//  if(USART_GetITStatus(USART1, USART_IT_TXE) == RESET)
//  {
//    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
//  }
  
  USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}

// Update Rule -----------------------------------------------------------------
/*
  Function : Update_Rule
  Input : None
  Output: None
  Description : 
*/
void Update_Rule(void)
{
  uint8_t HN_index;
  /* Update Hospital Number */
  for(HN_index = 3; HN_index < 16 ; HN_index++)
  {
    SProfile.cHospital_Number[HN_index-3] = '0' + suiData_GUI[HN_index];
  }
  //Show Hospital Number to LCD Display
  lcdString (1,1,SProfile.cHospital_Number);

  SProfile.uiSpO2_Maximum = suiData_GUI[16];
  SProfile.uiSpO2_Minimum = suiData_GUI[17];
  SProfile.uiRespondsTime = suiData_GUI[18];
  SProfile.uiPrefered_FiO2 = suiData_GUI[19];
  SProfile.uiMode = suiData_GUI[20];
  /* Select Mode */
  if (SProfile.uiMode == 0xB7)
  {
    /* Select Range Mode */
    lcdString(1,4,"Mode: Range");
    SProfile.uiFiO2_Maximum = suiData_GUI[21];
    SProfile.uiFiO2_Minimum = suiData_GUI[22];
  }
  else if (SProfile.uiMode == 0xA2)
  {
    /* Selecte Auto Mode */
    lcdString(1,4,"Mode: Auto");
    SProfile.uiFiO2_Maximum = 100;
    SProfile.uiFiO2_Minimum = 21;
  }

  SProfile.uiAlarm_Level1 = suiData_GUI[23] * 60;
  SProfile.uiAlarm_Level2 = suiData_GUI[24] * 60;
  
  /*Set SpO2 Middle Range */
  SProfile.uiSpO2_middleRange = (uint8_t)((SProfile.uiSpO2_Maximum + SProfile.uiSpO2_Minimum)/2);
//  uiPurpose_FiO2 = SProfile.uiPrefered_FiO2;

}

//------------------------------------------------------------------------------
// Function can use printf(); in sent data
int fputc(int ch, FILE *f)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART1, (uint8_t) ch);
  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {}
  return ch;
}
////------------------------------------------------------------------------------
///* TEST READ DATA FORM SD CARD SECTION */
//void USART_GUI_Connect(void)
//{  
//  GPIO_InitTypeDef GPIO_InitStruct;
//  USART_InitTypeDef USART_InitStruct;
//	
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
//	
//  /*
//    Use Port C Pin PC6 
//    Use Port C Pin PC7 
//  */
//  /* set GPIO init structure parameters values */
//  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_6 | GPIO_Pin_7;
//  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
//  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
//  GPIO_Init(GPIOC, &GPIO_InitStruct);
//  
//  /* Connect PXx to USARTx_Tx*/
//  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
//  /* Connect PXx to USARTx_Rx*/
//  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);
//  
//  
//  /* USART_InitStruct members default value */
//  USART_InitStruct.USART_BaudRate = 115200;
//  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
//  USART_InitStruct.USART_StopBits = USART_StopBits_1;
//  USART_InitStruct.USART_Parity = USART_Parity_No;
//  USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
//  USART_Init(USART6, &USART_InitStruct);
//  
//  /*USART Interrupt*/
//  /* Set interrupt: NVIC_Setup */
//  NVIC_InitTypeDef NVIC_InitStruct;
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
//  //ENABLE USART1 Interruper
//  NVIC_InitStruct.NVIC_IRQChannel = USART6_IRQn;
//  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
//  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStruct);
//
//  /* Set Interrupt Mode*/
//  //ENABLE the USART Receive Interrupt
//  USART_ITConfig(USART6, USART_IT_RXNE, DISABLE);
//
//  //Enable USART1
//  USART_Cmd(USART6, ENABLE);
//}
// End of File -------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
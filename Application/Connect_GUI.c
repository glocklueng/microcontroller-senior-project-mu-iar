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
#define ERROR   0x65                                                            //Data Error = 'e' (0x65)
#define ACK     0x41                                                            //Data Correct = 'A' (0x41)
#define OxygenSaturation_file           0
#define FiO2_file                       1


uint8_t rx_index_GUI=0;
uint8_t tx_index_GUI=0;

extern uint8_t Data_GUI [28];


//Define Variable for CRC ------------------------------------------------------
uint16_t Crc;
uint8_t CRC_Low, CRC_High;
uint8_t Length_Data = 25;

//------------------------------------------------------------------------------
//Define Value for Data Packaging
const uint8_t Padding = 0x91;                                                   //Padding Bytes for dummy Bytes of Data (Value = 0x23)
const uint8_t Connect_Command = 0xE8;                                           //0xE8 is Connect Command
const uint8_t Upload_Command = 0xD5;                                            //0xD5 is Upload Profile data Command
const uint8_t ETX = 0x33;                                                       //End of Package Transmittion

// Extern Profile Variable -----------------------------------------------------
extern char Hospital_Number[13];
extern uint8_t OxygenSaturaiton_Maximum, OxygenSaturation_Minimum;
extern uint8_t FiO2_Maximum, FiO2_Minimum;
extern uint8_t RespondsTime;
extern uint8_t Prefered_FiO2;
extern uint16_t Alarm_Level1, Alarm_Level2;
extern uint8_t Mode;
extern uint8_t Profile_Status;
//------------------------------------------------------------------------------
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
    Length : 16 bits (2 Bytes - CRC_High, CRC_Low)
    Polynomial : 0xA001
*/
void CRC_CALCULATE_TX(void)
{  
  uint8_t i;
  Crc = 0xFFFF;
  for (i = 0; i < Length_Data; i++) 
  {
    Crc = TX_CRC(Crc , Data_GUI[i]);
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
    Data_GUI[rx_index_GUI] = Data_in;
    //Data_GUI[rx_index_GUI] = USART_ReceiveData(GUI_USART);
    rx_index_GUI++;
  
    if(rx_index_GUI >= (sizeof(Data_GUI) - 1))
    {  
      rx_index_GUI = 0;

      //Check CRC16 - ModBus
      CRC_CALCULATE_TX();
      if (Data_GUI[25] == CRC_High & Data_GUI[26] == CRC_Low)
      {
        //CRC is Correct
        USART_SendData(USART1, ACK);                                            //Send Acknowlege to GUI
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        
        if (Data_GUI[1] == Upload_Command)
        {
          lcdClear();
          //Update Rule
          Update_Rule();
          Profile_Status = PROFILE_JUST_UPLOAD;
          lcdString(1,2,"SaO2: ");
          lcdString(1,3,"FiO2:");
        }
        else if (Data_GUI[1] == Connect_Command)
        {
          //Clear Data out from Buffer
          for (uint8_t i = 0; i < 28; i++)
          {
            Data_GUI[i] = 0;
          }
        }
      }
      else
      {
        //CRC is ERROR, Send ERROR ACK to GUI (ERROR ACK = 0xEA)
        USART_SendData(USART1, ERROR);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
      }
    }
  }
  
  if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
  {
    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
  }
  
  USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}

// Update Rule -----------------------------------------------------------------
void Update_Rule(void)
{
  uint8_t HN_index;
  // Update Hospital Number
  for(HN_index = 3; HN_index < 16 ; HN_index++)
  {
    Hospital_Number[HN_index-3] = '0' + Data_GUI[HN_index];
  }
  //Show Hospital Number to LCD Display
  lcdString (1,1,Hospital_Number);

  OxygenSaturaiton_Maximum = Data_GUI[16];
  OxygenSaturation_Minimum = Data_GUI[17];
  RespondsTime = Data_GUI[18];
  Prefered_FiO2 = Data_GUI[19];
  Mode = Data_GUI[20];
  //Select Mode
  if (Mode == 0xB7)
  {
    //Select Range Mode
    lcdString(1,4,"Mode: Range");
    FiO2_Maximum = Data_GUI[21];
    FiO2_Minimum = Data_GUI[22];
  }
  else if (Mode == 0xA2)
  {
    //Selecte Auto Mode
    lcdString(1,4,"Mode: Auto");
    FiO2_Maximum = 100;
    FiO2_Minimum = 21;
  }

  Alarm_Level1 = Data_GUI[23] * 60;
  Alarm_Level2 = Data_GUI[24] * 60;

}

//------------------------------------------------------------------------------
// Function can use printf(); in sent data
int fputc(int ch, FILE *f)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(GUI_USART, (uint8_t) ch);
  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(OPM_USART, USART_FLAG_TC) == RESET)
  {}
  return ch;
}

// End of File -------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
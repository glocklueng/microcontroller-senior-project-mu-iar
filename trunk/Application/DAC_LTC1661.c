/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term Infants
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : DAC_LTC1661.c

Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
/*   
Note :
Function : Transfer Data to LTC1661. LTC1661 is Ditital to Analog Convertor IC
           Use Serial to Periphral (SPI) to Transfer Data    
LTC 1661 
  - DAC Resolution 10 bits
  - Header code for transfer SPI to LTC 1661 (DAC 10 bits)
	Control Code (A3 A2 A1 A0)
	1 0 0 1 	- 	Load Input Reg A (Load DAC A), Output Update, HEX : 0x9000
	1 0 1 0 	-	Load Input Reg B (Load DAC B), Output Update, HEX : 0xA000
	1 1 1 1		-  	Load Input Reg A and B (Load DAC channal A and channal B), Output Update, HEX : 0xF000
		
	Data DAC Reg is 10 bits (D9 - D0)
		
	Don't care 2 bits (X1,X0)
*/
//------------------------------------------------------------------------------
#include "stm32f4xx.h"
#include "main.h"
#include <stdio.h>
#include "DAC_LTC1661.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_tim.h"
//------------------------------------------------------------------------------
uint16_t  DAC_data,DAC_sent;
uint8_t channel;

// Function --------------------------------------------------------------------
void SPI2_SetUp(void)
{
    /*use SPI2 for Transfer data to DAC IC (LTC 1661)*/
  
  GPIO_InitTypeDef GPIO_InitStruct;
  SPI_InitTypeDef SPI_InitStruct;
  
  /*
    PB12 = SPI2_NSS
    PB10 = SPI2_CLK
    PB14 = SPI2_MISO (Master in Slave out)
    PC3  = SPI2_MOIS (Master out Slave in)
  Note : In the Master Mode and Tx Only , use MOSI and CLK 
  */
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  RCC_AHB1PeriphClockCmd(SPI2_PortB_CLK, ENABLE);
  RCC_AHB1PeriphClockCmd(SPI2_PortC_CLK, ENABLE);
    
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_10;                    //Set for SCK Pin
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB , &GPIO_InitStruct);
  
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_3;                    //Set for MOSI Pin
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC , &GPIO_InitStruct);

  //Enable Altinate Function for SPI Protocal (PB10, PC3)
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10  ,GPIO_AF_SPI2);
  GPIO_PinAFConfig(GPIOC ,GPIO_PinSource3 ,GPIO_AF_SPI2);

  //Config SPI                        
  SPI_InitStruct.SPI_Direction = SPI_Direction_1Line_Tx;      // Tx Only
  SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
  SPI_InitStruct.SPI_DataSize = SPI_DataSize_16b;       //Data size is 16 bits for transfer data 10 bits to DAC IC
  SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
  SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_Init(SPI2, &SPI_InitStruct);
  
  SPI_NSSInternalSoftwareConfig(SPI2, SPI_NSSInternalSoft_Set);
  //Enable select output
  SPI_SSOutputCmd(SPI2, ENABLE);
  //Enable SPI2
  SPI_Cmd(SPI2,ENABLE);
}

void LTC1661_Setup(void)
{
  /*use SPI2 for Transfer data to DAC IC (LTC 1661)*/
  
  GPIO_InitTypeDef GPIO_InitStruct;
  
  /*
    PB12 = SPI2_NSS
    PB13 = SPI2_CLK
    PB14 = SPI2_MISO (Master in Slave out)
    PB15 = SPI2_MOIS (Master out Slave in)
  Note : In the Master Mode and Tx Only , use MOSI and CLK 
  */
	
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  RCC_AHB1PeriphClockCmd(SPI2_PortB_CLK, ENABLE);
  
  /* set GPIO init structure parameters values (NSS Pin is PB12) */ 
  GPIO_InitStruct.GPIO_Pin  = DAC_NSS_Pin;                                      //Set for NSS Pin
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(DAC_NSS_Port, &GPIO_InitStruct);
  
  GPIO_SetBits(DAC_NSS_Port, DAC_NSS_Pin);
	
}

/*
    uint16_t DAC_data   : Data for convert, 10 Bits, since 0x0000 to 0x03FF
    uint8_t Channel     : Select Channel 
                          Air Valve - Channel 1
                          Oxygen Valve - Channel 2
                          3 - Channel 1 and 2
*/

void SentData_DAC (uint16_t DAC_data, uint8_t channel)
{
  //Check Datasize config is 8 bits or 16 bits
  if ((SPI2->CR1 & 0x0800) == 0x0000)
  {
      // if Datasize is equal 8bits, then reconfig to 16 bits
      // Reconfig Size Data for DAC 
      SPI_DataSizeConfig(SPI2, SPI_DataSize_16b);
  }
 
  /* Select Channel */
  if(channel == Air_Valve)
  {
    DAC_data = DAC_data;
    DAC_sent = (DAC_data << 2) | 0x9000;
  }
  else if(channel == Oxygen_Valve)
  {
    DAC_data = DAC_data;
    DAC_sent = (DAC_data << 2) | 0xA000;    
  }
  else if(channel == 3)
  {
    DAC_data = DAC_data;
    DAC_sent = (DAC_data << 2) | 0xF000;      
  }
  
  /* Sent Data */
  uint16_t i;
  GPIO_ResetBits(DAC_NSS_Port, DAC_NSS_Pin);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == RESET)
  {
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == SET)
    {
      SPI_I2S_SendData(SPI2, DAC_sent);
    }
  }
  for(i=0;i<750;i++);
  GPIO_SetBits(DAC_NSS_Port, DAC_NSS_Pin);
}

//------------------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
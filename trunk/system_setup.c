/*
Project : Programmable Feedback Control of Airflow System for Pre-term infant oxygen saturation
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : system_setup.c
*/

#include "stm32f4xx.h"
#include <stdio.h>
#include "main.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_tim.h"
//------------------------------------------------------------------------------

// USART Setup -----------------------------------------------------------------
void usart_setup(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;
	
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  /* Connect PXx to USARTx_Tx*/
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig( GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
	
  /*
    Use Port A Pin PA2 to Tx
    Use Port A Pin PA3 to Rx
  */
	
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	
  /* USART_InitStruct members default value */
  USART_InitStruct.USART_BaudRate = 9600;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No ;
  USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
  USART_Init(USART2, &USART_InitStruct);

  //Enable USART1
  USART_Cmd(USART2, ENABLE);
}

// ADC_setup -------------------------------------------------------------------
void adc_setup(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  ADC_InitTypeDef ADC_InitStruct;
  /*
    use ADC1_IN6  Port A pin PA6
  */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 
	
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_6;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	
  /* Initialize the ADC_Mode member */
  ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
  /* initialize the ADC_ScanConvMode member */
  ADC_InitStruct.ADC_ScanConvMode = DISABLE;
  /* Initialize the ADC_ContinuousConvMode member */
  ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
  /* Initialize the ADC_ExternalTrigConvEdge member */
  ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  /* Initialize the ADC_ExternalTrigConv member */
  ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  /* Initialize the ADC_DataAlign member */
  ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
  /* Initialize the ADC_NbrOfConversion member */
  ADC_InitStruct.ADC_NbrOfConversion = 1;
  ADC_Init(ADC1,&ADC_InitStruct);
	
  //Enable ADC1
  ADC_Cmd(ADC1, ENABLE);
	
  ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1,ADC_SampleTime_28Cycles);
}

// SPI setup for DAC  ----------------------------------------------------------
void spi_setup(void)
{
  /*
    use SPI2 for Transfer data to DAC IC
  */
  GPIO_InitTypeDef GPIO_InitStruct;
  SPI_InitTypeDef SPI_InitStruct;
  
  /*
    PB12 = SPI2_NSS
    PB13 = SPI2_CLK
    PB14 = SPI2_MISO (Master in Slave out)
    PB15 = SPI2_MOIS (Master out Slave in)
  Note : In the Master Mode and Tx Only , use MOSI and CLK 
  */
	
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_13 | GPIO_Pin_15 ;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_12;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  //Enable Altinate Function for SPI Protocal (PB12,PB13,PB14,PB15)
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource12 ,GPIO_AF_SPI2);
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource13 ,GPIO_AF_SPI2);
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource15 ,GPIO_AF_SPI2);
	
  //Config SPI                       	
  SPI_InitStruct.SPI_Direction = SPI_Direction_1Line_Tx;			// Tx Only
  SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
  SPI_InitStruct.SPI_DataSize = SPI_DataSize_16b;				//Data size is 16 bits for transfer data 10 bits to DAC IC
  SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
  //SPI_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set;
  SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
  SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_Init(SPI2, &SPI_InitStruct);
  
  //Enable select output
  SPI_SSOutputCmd(SPI2, ENABLE);
  //Enable SPI2
  SPI_Cmd(SPI2,ENABLE);

}


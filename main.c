/*
Project : Programmable Feedback Control of Airflow System for Pre-term infant oxygen saturation
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : main.c
*/
//******************************************************************************
#include "stm32f4xx.h"
#include "main.h"
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

//******************************************************************************
void usart_setup(void);
void adc_setup(void);
void spi_setup(void);
int fputc(int ch, FILE *f);
void delay(void);
void scanf1(void);
void adc_printf(void);
//******************************* Variable **************************************
unsigned char msg ;
char channal_DAC;
uint16_t adc_value, DAC_data, DAC_real;

//******************************* Main Function ********************************
int main()
{	
  /* Set Up config System*/
  usart_setup();
  adc_setup();
  spi_setup();
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDOn(LED3);
  STM_EVAL_LEDOn(LED4);
  STM_EVAL_LEDOn(LED5);
	
  /*
  while(1)
  {
	//Start ADC Convertor
	ADC_SoftwareStartConv(ADC1);
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	adc_value = ADC_GetConversionValue(ADC1);
	printf("The Value from ADC = %d \n",&adc_value);
	delay();
	STM_EVAL_LEDOff(LED3);
	delay();
  }
  */
	
  //SPI transfer data to DAC IC (LTC 1661,10 bits)
  /*
	Header code for transfer SPI to LTC 1661 (DAC 10 bits)
	Control Code (A3 A2 A1 A0)
	1 0 0 1 	- 	Load Input Reg A (Load DAC A), Output Update, HEX : 0x9000
	1 0 1 0 	-	Load Input Reg B (Load DAC B), Output Update, HEX : 0xA000
	1 1 1 1		-  	Load Input Reg A and B (Load DAC channal A and channal B), Output Update, HEX : 0xF000
		
	Data DAC Reg is 10 bits (D9 - D0)
		
	Don't care 2 bits (X1,X0)
  */

  DAC_real = 0x0128;
  DAC_data =(DAC_real<<2) | 0xA000;
  while(1)
  {
   
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == RESET)
    {
      while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == SET)
      {
        SPI_I2S_SendData(SPI2, DAC_data);
      }
    }
    GPIO_SetBits(GPIOB, GPIO_Pin_12);
    delay();
//    DAC_real = DAC_real + 0x0005;
//    DAC_data =(DAC_real<<2) | 0xF000;

  }
}
	

//******************************************************************************


void scanf1(void)
{
  while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);
  msg = USART_ReceiveData(USART2);
}
int fputc(int ch, FILE *f)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART2, (uint8_t) ch);
  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
  {}
  return ch;
}
//*********************** SPI setup ************************************
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

//*********************** delay function *******************************
void delay(void)
{
  unsigned int i,j;
  for(i=0;i<5000;i++)
  {
    for(j=0;j<500;j++);
  }
  
}

/*This Function is Analog to Digital Converter and Sent data via RS-232 (USART)(Printf) */
void adc_printf(void)
{
  adc_value = 0;
  //Start ADC Convertor
  ADC_SoftwareStartConv(ADC1);
  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
  adc_value = ADC_GetConversionValue(ADC1);
  printf("The Value from ADC = %d \n", &adc_value);
}
//*************************** Interrupt ****************************************

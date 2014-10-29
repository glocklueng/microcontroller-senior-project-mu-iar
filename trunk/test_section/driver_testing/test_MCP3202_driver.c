/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : MCP3202.c
Deverloper : Phattaradanai Kiratiwudhikul
Reseach & Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "MCP3202.h"
#include "DAC_LTC1661.h"
//------------------------------------------------------------------------------
uint16_t uiFlowRate;
float fVoltageFlowRate;
//------------------------------------------------------------------------------
/*
  Function : MCP3202_SetUp
  @ Input : None
  @ Return : None
  Description : Set Up and Configuration the NSS pin for MCP3202 (USE SPI2 Communication)
*/
void MCP3202_SetUp(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  RCC_AHB1PeriphClockCmd(SPI2_PortA_CLK, ENABLE);
  
  /* set GPIO init structure parameters values (NSS Pin is PA15) */ 
  GPIO_InitStruct.GPIO_Pin  = ADC_MCP_NSS_Pin;                                  //Set for NSS Pin for MCP3202
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(ADC_MCP_NSS_Port, &GPIO_InitStruct);
  
  GPIO_SetBits(ADC_MCP_NSS_Port, ADC_MCP_NSS_Pin);
}

//------------------------------------------------------------------------------
/*
  Function: Get_FlowRate
  @ Input : (uint8_t) uiChannel - CH0,CH1,OxygenFlowRate,AirFlowRate
  @ Return : (float) fVoltageFlowRate
  Description : Receiving Information form MCP3202 (Analog to Digital Converter IC). The value return is the voltage flow rate.
*/

float Get_FlowRate(uint8_t uiChannel)
{
  uint16_t uiDataOut;
  uint8_t uiSamplingTime;
  float fVoltageFlow[10];
  fVoltageFlowRate = 0;
  
  if ((SPI2->CR1 & 0x0800) == 0x0000)
  {
    /* 
      if Datasize is equal 8bits, then reconfig to 16 bits
      Reconfig Size Data for DAC 
    */
    SPI_DataSizeConfig(SPI2, SPI_DataSize_16b);
  }
  
  /* Check Channel inpit of signal of ADC IC */
  if (uiChannel == CH0 | uiChannel == OxygenFlowRate)
  {
    uiDataOut = 0xD000;                                                         // Command for ADC channel 0
  }
  else if (uiChannel == CH1 | uiChannel == AirFlowRate)
  {
    uiDataOut = 0xF000;                                                         // Command for ADC channel 1
  }
  
  /* Sampling 10 samples */
  for(uiSamplingTime = 0; uiSamplingTime < 10; uiSamplingTime++)
  {
    /* Part Send data out */
    GPIO_ResetBits(ADC_MCP_NSS_Port, ADC_MCP_NSS_Pin);                          // Set NSS is Low
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI2, 0x01);                                               // Send Start bit (logic '1')
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
    
    /* Enable the Rx buffer not empty interrupt */
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI2, uiDataOut<<1);                                       // Send data out shift left 1
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
    uiFlowRate = SPI_I2S_ReceiveData(SPI2);                                     // Receive Data from MCP3202
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
    GPIO_SetBits(ADC_MCP_NSS_Port, ADC_MCP_NSS_Pin);                            // Set NSS Pin is High
    
    uiFlowRate = uiFlowRate & 0x0FFF;
    fVoltageFlow[uiSamplingTime] = (uiFlowRate * 5.0)/4096;                     // Convert Digital Value to Voltage 
    
    /* Average  value */
    fVoltageFlowRate = fVoltageFlowRate + fVoltageFlow[uiSamplingTime];
  }
  fVoltageFlowRate = fVoltageFlowRate / (uiSamplingTime + 1);
    
  return fVoltageFlowRate;
  
}

//------------------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
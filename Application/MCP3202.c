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
uint16_t FlowRate;
uint8_t  FlowBuffer = 10;
uint8_t FlowIndex = 0;
float VoltageFlowRate;
//------------------------------------------------------------------------------
/*
  Function : MCP3202_SetUp
  Input : None
  Return : None
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
  Input : (uint8_t) channel - CH0,CH1,OxygenFlowRate,AirFlowRate
  Return : (float) VoltageFlowRate
  Description : Receiving Information form MCP3202 (Analog to Digital Converter IC). The value return is the voltage flow rate.
*/

float Get_FlowRate(uint8_t channel)
{
  uint16_t DataOut;
  
  if ((SPI2->CR1 & 0x0800) == 0x0000)
  {
    // if Datasize is equal 8bits, then reconfig to 16 bits
    // Reconfig Size Data for DAC 
    SPI_DataSizeConfig(SPI2, SPI_DataSize_16b);
  }
  
  if (channel == CH0 | channel == OxygenFlowRate)
  {
    DataOut = 0xD000;
    //DataOut = (DataOut << 12);
  }
  else if (channel == CH1 | channel == AirFlowRate)
  {
    DataOut = 0x000F;
    DataOut = (DataOut << 12);
  }
  
  // Send data out
  GPIO_ResetBits(ADC_MCP_NSS_Port, ADC_MCP_NSS_Pin);                            // Set NSS is Low
  
  /* Enable the Rx buffer not empty interrupt */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI2, DataOut);                                              // Send data out
  while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
  FlowRate = SPI_I2S_ReceiveData(SPI2);                                         // Receive Data from MCP3202
  FlowRate = FlowRate & 0x0FFF;
  VoltageFlowRate = (FlowRate*5.0)/4095;                                        // Convert Digital Value to Voltage 
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);

  GPIO_SetBits(ADC_MCP_NSS_Port, ADC_MCP_NSS_Pin);                              // Set NSS Pin is High
  
  return VoltageFlowRate;
  
}

////------------------------------------------------------------------------------
//void SPI2_IRQHandler (void)
//{
//  if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_RXNE) == SET)
//  {
//    if (FlowIndex < FlowBuffer)
//    {
//      /* Receive Transaction data */
//      FlowRate = SPI_I2S_ReceiveData(SPI2);
//      FlowRate = FlowRate & 0x0FFF;
//      //FlowIndex++;
////      if(FlowIndex >= FlowBuffer)
////      {
////        FlowIndex = 0;
////      }
//  
//    }
//    else
//    {
//      /* Disable the Rx buffer not empty interrupt */
//      SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
//    }
//  }
//  SPI_I2S_ClearITPendingBit(SPI2, SPI_I2S_IT_RXNE);
//}



//------------------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
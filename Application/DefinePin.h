/*
Project : Programmable Feedback Control of Airflow System for Pre-term infant oxygen saturation
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : DefinePin.h
*/
/*----------------------------------------------------------------------------------------------
Credit:
  Deverloper : Phattaradanai Kiratiwudhikul
  Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
----------------------------------------------------------------------------------------------*/
//------------------------------------------------------------------------------
#include "main.h"
//------------------------------------------------------------------------------
//Define RS-232 for Oxygen Pulse Meter
#define  OPM_USART				USART6
#define  OPM_USART_CLK				RCC_APB2Periph_USART6
#define  OPM_UART_CLK_INIT			RCC_APB2PeriphClockCmd
#define  OPM_IRQn				USART6_IRQn
#define  OPM_IRQHandler				USART6_IRQHandler
#define  OPM_Port				GPIOC
#define  OPM_Port_CLK				RCC_AHB1Periph_GPIOC

#define  OPM_TX_Pin				GPIO_Pin_6
#define  OPM_TX_GPIO_Port			GPIOC
#define  OPM_TX_GPIO_CLK			RCC_AHB1Periph_GPIOC
#define  OPM_TX_Souce				GPIO_PinSource6
#define  OPM_TX_AF				GPIO_AF_USART6

#define  OPM_RX_Pin				GPIO_Pin_7
#define  OPM_RX_GPIO_Port			GPIOC
#define  OPM_RX_GPIO_CLK			RCC_AHB1Periph_GPIOC
#define  OPM_RX_Souce				GPIO_PinSource7
#define  OPM_RX_AF				GPIO_AF_USART6

//------------------------------------------------------------------------------
//Define SPI2
#define  SPI2_PortB 				GPIOB
#define  SPI2_PortC 				GPIOC
#define  SPI2_PortB_CLK				RCC_AHB1Periph_GPIOB
#define  SPI2_PortC_CLK				RCC_AHB1Periph_GPIOC

#define  SPI2_CLK_Pin				GPIO_Pin_10
#define  SPI2_CLK_Port				GPIOB
#define  SPI2_CLK_Source			GPIO_PinSource10
#define  SPI2_CLK_AF				GPIO_AF_SPI2	

#define  SPI2_MOSI_Pin				GPIO_Pin_3
#define  SPI2_MOSI_Port				GPIOC	
#define  SPI2_MOSI_Source			GPIO_PinSource3
#define  SPI2_MOSI_AF				GPIO_AF_SPI2

//-------------------------------------------------------------------------------			
//Define DAC_LTC1661
#define  DAC_NSS_Pin				GPIO_Pin_12
#define  DAC_NSS_Port				GPIOB

//-------------------------------------------------------------------------------
//Define GLCD5110
#define  GLCD_CLK					RCC_AHB1Periph_GPIOD
#define  GLCD_Port 					GPIOD

#define  GLCD_NSS_Pin				GPIO_Pin_1
#define  GLCD_NSS_Port				GPIOD

#define  GLCD_RES_Pin				GPIO_Pin_6
#define  GLCD_RES_Port				GPIOD

#define  GLCD_DC_Pin				GPIO_Pin_4
#define  GLCD_DC_Port				GPIOD

#define  GLCD_LED_Pin				GPIO_Pin_5
#define  GLCD_LED_Port				GPIOD
//--------------------------------------------------------------------------------
//Define Oxygen Sensor (ADC)
#define  OxygenSensor 				ADC1 
#define  OxygenSensor_ADC_CLK		        RCC_APB2Periph_ADC1
#define  OxygenSensor_Pin_CLK		        RCC_AHB1Periph_GPIOB

#define  OxygenSensor_Pin 			GPIO_Pin_1
#define  OxygenSensor_Port			GPIOB

//---------------------------------------------------------------------------------
//Define Connect GUI (Tx-PD8, Rx-PD9)
#define  GUI_USART				USART1
#define  GUI_USART_CLK				RCC_APB1Periph_USART1
#define  GUI_Port				GPIOB
#define  GUI_Port_CLK				RCC_AHB1Periph_GPIOB
#define  GUI_IRQn				USART1_IRQn
#define  GUI_IRQHandler				USART1_IRQHandler

#define  GUI_TX_Pin				GPIO_Pin_6
#define  GUI_TX_GPIO_Port			GPIOB
#define  GUI_TX_GPIO_CLK			RCC_AHB1Periph_GPIOB
#define  GUI_TX_Souce				GPIO_PinSource6
#define  GUI_TX_AF				GPIO_AF_USART1

#define  GUI_RX_Pin				GPIO_Pin_7
#define  GUI_RX_GPIO_Port			GPIOB
#define  GUI_RX_GPIO_CLK			RCC_AHB1Periph_GPIOB
#define  GUI_RX_Souce				GPIO_PinSource7
#define  GUI_RX_AF				GPIO_AF_USART1

//------------------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
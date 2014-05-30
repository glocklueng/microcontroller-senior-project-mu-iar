/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : DefinePin.h

Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
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
#define  OPM_USART					USART6
#define  OPM_USART_CLK				RCC_APB2Periph_USART6
#define  OPM_UART_CLK_INIT			RCC_APB2PeriphClockCmd
#define  OPM_IRQn					USART6_IRQn
#define  OPM_IRQHandler				USART6_IRQHandler
#define  OPM_Port					GPIOC
#define  OPM_Port_CLK				RCC_AHB1Periph_GPIOC

#define  OPM_TX_Pin					GPIO_Pin_6
#define  OPM_TX_GPIO_Port			GPIOC
#define  OPM_TX_GPIO_CLK			RCC_AHB1Periph_GPIOC
#define  OPM_TX_Souce				GPIO_PinSource6
#define  OPM_TX_AF					GPIO_AF_USART6

#define  OPM_RX_Pin					GPIO_Pin_7
#define  OPM_RX_GPIO_Port			GPIOC
#define  OPM_RX_GPIO_CLK			RCC_AHB1Periph_GPIOC
#define  OPM_RX_Souce				GPIO_PinSource7
#define  OPM_RX_AF					GPIO_AF_USART6

//------------------------------------------------------------------------------
//Define SPI2
#define  SPI2_PortA                             GPIOA
#define  SPI2_PortB 				GPIOB
#define  SPI2_PortC 				GPIOC
#define  SPI2_PortA_CLK                         RCC_AHB1Periph_GPIOA
#define  SPI2_PortB_CLK				RCC_AHB1Periph_GPIOB
#define  SPI2_PortC_CLK				RCC_AHB1Periph_GPIOC

#define  SPI2_CLK_Pin				GPIO_Pin_10
#define  SPI2_CLK_Port				GPIOB
#define  SPI2_CLK_Source			GPIO_PinSource10
#define  SPI2_CLK_AF				GPIO_AF_SPI2	

#define  SPI2_MISO_Pin                          GPIO_Pin_2
#define  SPI2_MISO_Port                         GPIOC
#define  SPI2_MISO_Source                       GPIO_PinSource2
#define  SPI2_MISO_AF                           GPIO_AF_SPI2

#define  SPI2_MOSI_Pin				GPIO_Pin_3
#define  SPI2_MOSI_Port				GPIOC	
#define  SPI2_MOSI_Source			GPIO_PinSource3
#define  SPI2_MOSI_AF				GPIO_AF_SPI2

//-------------------------------------------------------------------------------			
//Define DAC_LTC1661
#define  DAC_NSS_Pin				GPIO_Pin_14
#define  DAC_NSS_Port				GPIOB

//------------------------------------------------------------------------------
//Define ADC_MCP3202
#define  ADC_MCP_NSS_Pin                        GPIO_Pin_15
#define  ADC_MCP_NSS_Port                       GPIOA

//-------------------------------------------------------------------------------
//Define GLCD5110
#define  GLCD_CLK				RCC_AHB1Periph_GPIOB
#define  GLCD_Port 				GPIOB

#define  GLCD_NSS_Pin				GPIO_Pin_4
#define  GLCD_NSS_Port				GPIOB

#define  GLCD_RES_Pin				GPIO_Pin_5
#define  GLCD_RES_Port				GPIOB

#define  GLCD_DC_Pin				GPIO_Pin_0
#define  GLCD_DC_Port				GPIOB

#define  GLCD_LED_Pin				GPIO_Pin_1
#define  GLCD_LED_Port				GPIOB
//------------------------------------------------------------------------------
//Define Oxygen Sensor (ADC)
#define  OxygenSensor 				ADC1
#define  OxygenSensor_ADC_CLK		        RCC_APB2Periph_ADC1
#define  OxygenSensor_Pin_CLK		        RCC_AHB1Periph_GPIOA

#define  OxygenSensor_Pin 			GPIO_Pin_3
#define  OxygenSensor_Port			GPIOA

//------------------------------------------------------------------------------
//Define Connect GUI (Tx-PD8, Rx-PD9)
#define  GUI_USART					USART1
#define  GUI_USART_CLK				RCC_APB1Periph_USART1
#define  GUI_Port					GPIOB
#define  GUI_Port_CLK				RCC_AHB1Periph_GPIOB
#define  GUI_IRQn					USART1_IRQn
#define  GUI_IRQHandler				USART1_IRQHandler

#define  GUI_TX_Pin					GPIO_Pin_6
#define  GUI_TX_GPIO_Port			GPIOB
#define  GUI_TX_GPIO_CLK			RCC_AHB1Periph_GPIOB
#define  GUI_TX_Souce				GPIO_PinSource6
#define  GUI_TX_AF					GPIO_AF_USART1

#define  GUI_RX_Pin					GPIO_Pin_7
#define  GUI_RX_GPIO_Port			GPIOB
#define  GUI_RX_GPIO_CLK			RCC_AHB1Periph_GPIOB
#define  GUI_RX_Souce				GPIO_PinSource7
#define  GUI_RX_AF					GPIO_AF_USART1

// Button Down -------------------------------------------------------------------
//#define Button_Down_Pin				GPIO_Pin_0
//#define Button_Down_GPIO_Port		GPIOB
//#define Button_Down_GPIO_CLK		RCC_AHB1Periph_GPIOB
//
//#define Button_Down_EXTI_Line		EXTI_Line0
//#define Button_Down_IRQn			EXTI0_IRQn
//#define Button_Down_IRQHandler      EXTI0_IRQHandler

// Run Button --------------------------------------------------------------------
#define Run_Button_Pin				GPIO_Pin_1
#define Run_Button_GPIO_Port		GPIOB
#define Run_Button_GPIO_CLK			RCC_AHB1Periph_GPIOB

#define Run_Button_EXTI_Line		EXTI_Line1
#define Run_Button_IRQn				EXTI1_IRQn
#define Run_Button_IRQHandler		EXTI1_IRQHandler

// Alarm Set Pin -----------------------------------------------------------------
#define Alarm_Set_Pin				GPIO_Pin_2
#define Alarm_Set_GPIO_Port			GPIOC
#define Alarm_Set_GPIO_CLK			RCC_AHB1Periph_GPIOC


// Alarm Button ------------------------------------------------------------------
//#define Alarm_Button_Pin			GPIO_Pin_4
//#define Alarm_Button_GPIO_Port		GPIOB
//#define Alarm_Button_GPIO_CLK		RCC_AHB1Periph_GPIOB
//
//#define Alarm_Button_EXTI_Line		EXTI_Line4
//#define Alarm_Button_IRQn			EXTI4_IRQn
//#define Alarm_Button_IRQHandler		EXTI4_IRQHandler
//
//// Button UP ---------------------------------------------------------------------
//#define Button_Up_Pin				GPIO_Pin_5	
//#define Button_Up_GPIO_Port			GPIOB
//#define Button_Up_GPIO_CLK			RCC_AHB1Periph_GPIOB
//
//#define Button_Up_EXTI_Line			EXTI_Line5
//#define Button_Up_IRQn				EXTI9_5_IRQn
//#define Button_Up_IRQHandler		EXTI9_5_IRQHandler

/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
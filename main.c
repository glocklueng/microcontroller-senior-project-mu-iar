/*
Project : Programmable Feedback Control of Airflow System for Pre-term infant oxygen saturation
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : main.c
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "DAC_LTC1661.h"
#include "Oxygen_Pulse_Meter.h"
#include "Oxygen_sensor.h"
#include "GLCD5110.h"
#include "DefinePin.h"
#include "Connect_GUI.h"
//------------------------------------------------------------------------------
__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END;
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

//------------------------------------------------------------------------------
void delay(void);
void System_Init(void);
void INTTIM_Config(void);
void EXTILine0_Config(void);
// Variable --------------------------------------------------------------------
unsigned char msg ;
char Character;
uint32_t count;
extern uint16_t time;
extern uint8_t rx_index_GUI;
uint8_t Data_GUI[40];
uint8_t Oxygen_Sat[14], FiO2[14];
// Main Function ---------------------------------------------------------------
int main()
{  
 /* Set Up config System*/
  System_Init();
  lcdInit();
  lcdString (1,1,"Hello");
  lcdString(2,1, "Phattaradanai");
  
  while(1)
  {
    delay();
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
void System_Init(void)
{
  SPI2_SetUp();
  LTC1661_Setup();
  OxygenSensor_Setup();
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
  Timer6_SetUp();
  USART_GUI_Connect();
  //INTTIM_Config();
  
  // LED Set UP
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
  STM_EVAL_LEDOn(LED3);
  STM_EVAL_LEDOn(LED4);
  STM_EVAL_LEDOn(LED5);
  STM_EVAL_LEDOn(LED6);
  
  //LCD Set Up
  lcdInit();
  
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


//void INTTIM_Config(void)
//{
//  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//  NVIC_InitTypeDef NVIC_InitStructure;
//  /* Enable the TIM2 gloabal Interrupt */
//  NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
//   
//  /* TIM2 clock enable */
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
//  /* Time base configuration */
//  TIM_TimeBaseStructure.TIM_Period = 2000; // 1 MHz down to 1 KHz (1 ms)
//  TIM_TimeBaseStructure.TIM_Prescaler = 42000; // 24 MHz Clock down to 1 MHz (adjust per your clock)
//  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
//  /* TIM IT enable */
//  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
//  /* TIM2 enable counter */
//  TIM_Cmd(TIM6, ENABLE);
//  TIM_Cmd(TIM6, DISABLE);
//}
//void TIM6_DAC_IRQHandler(void)
//{
//  if (TIM_GetITStatus (TIM6, TIM_IT_Update) != RESET)
//  {
//    time = time + 1;
//    STM_EVAL_LEDOff(LED5);
//    TIM_ClearITPendingBit (TIM6, TIM_IT_Update);
//  }
//}

//------------------------------------------------------------------------------
/**
  * @brief  Configures EXTI Line0 (connected to PA0 pin) in interrupt mode
  * @param  None
  * @retval None
  */
static void EXTILine0_Config(void)
{
  EXTI_InitTypeDef   EXTI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;

  /* Enable GPIOA clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  /* Configure PA0 pin as input floating */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Connect EXTI Line0 to PA0 pin */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

  /* Configure EXTI Line0 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
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
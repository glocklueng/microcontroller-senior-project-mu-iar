/*
Project : Programmable Feedback Control of Airflow System for Pre-term infant oxygen saturation
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : main.c
*/
//******************************************************************************
#include "main.h"
#include "DAC_LTC1661.h"

//------------------------------------------------------------------------------
uint16_t DAC_data;
uint8_t channel;


//******************************************************************************
void adc_setup(void);
void spi_setup(void);
int fputc(int ch, FILE *f);
void delay(void);
void scanf1(void);
void adc_printf(void);
//******************************* Variable *************************************
unsigned char msg ;
char channal_DAC;
uint16_t DAC_data;

//******************************* Main Function ********************************
int main()
{	
  /* Set Up config System*/
  LTC1661_Setup();
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDOn(LED3);
  STM_EVAL_LEDOn(LED4);
  STM_EVAL_LEDOn(LED5);

    delay();

    while(1)
  {
 
  }
  
}
	

//------------------------------------------------------------------------------

// delay function --------------------------------------------------------------
void delay(void)
{
  unsigned int i,j;
  for(i=0;i<5000;i++)
  {
    for(j=0;j<500;j++);
  }
  
}

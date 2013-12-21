/*
Project : Programmable Feedback Control of Airflow System for Pre-term infant oxygen saturation
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : SD_Card.h
*/
//------------------------------------------------------------------------------
#include "main.h"

//------------------------------------------------------------------------------
/* Private function prototypes -----------------------------------------------*/
void SD_Card_Setup(void);
static void NVIC_Configuration(void);
static void SD_EraseTest(void);
static void SD_SingleBlockTest(void);
static void SD_MultiBlockTest(void);
static void Fill_Buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset);
static TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength);
static TestStatus eBuffercmp(uint8_t* pBuffer, uint32_t BufferLength);
//-------------------------------------------------------------------------------
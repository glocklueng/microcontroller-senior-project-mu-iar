/*
Project : Programmable Feedback Control of Airflow System for Pre-term infant oxygen saturation
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : SD_Card.c
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "ff.h"
// Define ----------------------------------------------------------------------
#define OxygenSaturation_file           0
#define FiO2_file                       1
//------------------------------------------------------------------------------
static void Delay(__IO uint32_t nCount);
static void fault_err (FRESULT rc);

void Check_Mount(void);
static void Delay(__IO uint32_t nCount);
void Create_file(char FileName[], uint8_t File_Type);
void SD_Write(char FileName[], char SD_Data[], UINT Data_size);
//------------------------------------------------------------------------------
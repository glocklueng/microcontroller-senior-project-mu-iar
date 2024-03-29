/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : SD_Card.c
------------------------------------------------------------------------------------------------
Credit:
  Deverloper : Phattaradanai Kiratiwudhikul
  Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
----------------------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
#include "main.h"
#include "ff.h"
#include "diskio.h"
// Define ----------------------------------------------------------------------
#define OxygenSaturation_file           0
#define FiO2_file                       1
//------------------------------------------------------------------------------

static void Delay(__IO uint32_t nCount);
void fault_err (FRESULT rc);

void Check_Mount(void);
static void Delay(__IO uint32_t nCount);
void Create_file(char FileName[]);
void SD_Write(char FileName[], char cDataTimeSD[], uint8_t uiSpO2_SD, float fFiO2_SD, uint8_t uiStatus);
//------------------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
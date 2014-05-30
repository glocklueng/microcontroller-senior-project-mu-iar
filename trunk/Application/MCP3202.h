/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : MCP3202.h
Deverloper : Phattaradanai Kiratiwudhikul
Reseach & Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "DAC_LTC1661.h"
//------------------------------------------------------------------------------
/*
    Note:
    OxygenFlowRate      use         CH0
    AirFlowRate         use         CH1
*/
// Define-----------------------------------------------------------------------
#define CH0             0
#define CH1             1
#define OxygenFlowRate  0
#define AirFlowRate     1
//------------------------------------------------------------------------------
void MCP3202_SetUp(void);
float Get_FlowRate (uint8_t channel);
//------------------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
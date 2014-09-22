/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : main.h

Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/ 
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdlib.h>
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_audio_codec.h"
#include "stm32f4_discovery_lis302dl.h"
#include <stdio.h>
#include "stm32f4xx_it.h"
#include "waveplayer.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_sdio.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_exti.h"
#include "stm32f4_discovery_sdio_sd.h"
#include "DefinePin.h"
#include "usbd_cdc_core.h"
#include "usbd_cdc_vcp.h"
#include "usbd_usr.h"
#include "usbd_desc.h"

#include "ff.h"
#include "SD_Card.h"

#ifdef MEDIA_USB_KEY
 #include "waverecorder.h"
 #include "usb_hcd_int.h"
 #include "usbh_usr.h"
 #include "usbh_core.h"
 #include "usbh_msc_core.h"
 #include "pdm_filter.h"
#endif
 
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Select the media where the Wave file is stored */
#if !defined (MEDIA_IntFLASH) && !defined (MEDIA_USB_KEY) 
 //#define MEDIA_IntFLASH /* Wave file stored in internal flash */
 //#define MEDIA_USB_KEY  /* Wave file stored in USB flash */
#endif

/* Uncomment this define to disable repeat option */
//#define PLAY_REPEAT_OFF

#if defined MEDIA_USB_KEY
  /* You can change the Wave file name as you need, but do not exceed 11 characters */
  #define WAVE_NAME "0:audio.wav"
  #define REC_WAVE_NAME "0:rec.wav"

  /* Defines for the Audio recording process */
  #define RAM_BUFFER_SIZE         1500  /* 3Kbytes (1500 x 16 bit) as a RAM buffer size.
                                           More the size is higher, the recorded quality is better */ 
  #define TIME_REC                3000 /* Recording time in millisecond(Systick Time Base*TIME_REC= 10ms*3000)
                                         (default: 30s) */
#endif /* MEDIA_USB_KEY */

// Status ----------------------------------------------------------------------
#define TIMER_DISABLE         0
#define TIMER_ENABLE          1

#define STATUS_NORMAL                         0
#define STATUS_SpO2_BELOW_L1                  1
#define STATUS_SpO2_BELOW_L2                  2
#define STATUS_SpO2_BEHIGH_L1                 3
#define STATUS_SpO2_BEHIGH_L2                 4
#define STATUS_ALARM                          5
#define STATUS_MIDDLE_SpO2_BELOW              6
#define STATUS_MIDDLE_SpO2_BEHIGH             7
#define STATUS_MIDDLE_SpO2                    8
#define STATUS_SpO2_BEHIGH_ALARM_L1           9
#define STATUS_SpO2_BELOW_ALARM_L1            10


/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void TimingDelay_Decrement(void);
void Delay(__IO uint32_t nTime);

void EXTILine0_Config(void);
void delay();

#endif /* __MAIN_H */

// End of File -------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/

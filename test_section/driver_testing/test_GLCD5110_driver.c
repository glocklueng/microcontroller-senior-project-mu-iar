/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : GLCD5110.c

Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University


Note : Maximum line 6

----------------------------------------------------------------------------------------------*/
//------------------------------------------------------------------------------
#include "stm32f4xx.h"
#include "main.h"
#include "DAC_LTC1661.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_tim.h"
#include "GLCD5110.h"
//------------------------------------------------------------------------------
/*------------------------------------------------------------------------------
                                      Global Variables
------------------------------------------------------------------------------*/
static unsigned char  LcdCache [ LCD_CACHE_SIZE ];

static int   LcdCacheIdx;
static int   LoWaterMark;
static int   HiWaterMark;
__IO static char  UpdateLcd;
unsigned char glcd_ini=0;

//------------------------------------------------------------------------------
//--------------------------- Function delay -----------------------------------
//------------------------------------------------------------------------------
void delay_ms(unsigned long ms)                                                 // delay 1 ms per count @ Crystal 8.0 MHz and PLL9x or SYSCLK = 72 MHz
{
   __IO unsigned long i,j;
	for (i = 0; i < ms; i++ )
        {
          for (j = 0; j < 5525; j++ );
        }
}

void port_init()
{
  /*use SPI2 for Transfer data to PCD8544 (Nokia LCD)*/
  GPIO_InitTypeDef GPIO_InitStruct;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  RCC_AHB1PeriphClockCmd(GLCD_CLK, ENABLE);

  //For NSS Pin
  GPIO_InitStruct.GPIO_Pin  = GLCD_NSS_Pin ;                                     
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GLCD_Port, &GPIO_InitStruct);
  
  /*
  	set output for NSS, RESET, D/C(Data/Command)
                (3)NSS           |      PA8
		(4)RESET	 |      PA10
		(5)D/C           |      PA15
                (6)LED           |      PA5
  */

  /* set GPIO init structure parameters values */
  GPIO_InitStruct.GPIO_Pin  = GLCD_RES_Pin | GLCD_DC_Pin | GLCD_LED_Pin;        //Set for RESET, D/C Pin
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GLCD_Port, &GPIO_InitStruct);
  
  GPIO_SetBits(GLCD_NSS_Port, GLCD_NSS_Pin);                                    // NSS Pin is High
	
  // Reconfig Size Data
  SPI_DataSizeConfig(SPI2, SPI_DataSize_8b);
  
  //Set LED On
  GPIO_SetBits(GLCD_LED_Port, GLCD_LED_Pin);
}

//------------------------------------------------------------------------------
void lcdInit(void)
{
   port_init();
   delay_ms(10); // Ensure delay for initial
   GPIO_ResetBits(GLCD_RES_Port, GLCD_RES_Pin);					//PD6 is RESET Pin
   GPIO_SetBits(GLCD_RES_Port, GLCD_RES_Pin);
   
   GPIO_SetBits(GLCD_NSS_Port, GLCD_NSS_Pin);					//NSS Pin is High
   lcdSend( 0x21, LCD_CMD );  // LCD Extended Commands.
   lcdSend( 0xC0, LCD_CMD );  // Set LCD Vop (Contrast).
   lcdSend( 0x06, LCD_CMD );  // Set Temp coefficent.
   lcdSend( 0x13, LCD_CMD );  // LCD bias mode 1:48.
   lcdSend( 0x20, LCD_CMD );  // LCD Standard Commands, Horizontal addressing mode.
   lcdSend( 0x0C, LCD_CMD );  // LCD in normal mode.
    
   //  Reset watermark pointers.
   LoWaterMark = LCD_CACHE_SIZE;
   HiWaterMark = 0;
   lcdClear();
   lcdUpdate();
}

/*------------------------------------------------------------------------------

  Name         :  lcdClear

  Description  :  Clears the display. lcdUpdate must be called next.

  Argument(s)  :  None.

  Return value :  None.

------------------------------------------------------------------------------*/
void lcdClear ( void )
{
    int i;

    for ( i = 0; i < LCD_CACHE_SIZE; i++ )
    {
        LcdCache[i] = 0x00;
    }

    //  Reset watermark pointers.
    LoWaterMark = 0;
    HiWaterMark = LCD_CACHE_SIZE - 1;

    UpdateLcd = TRUE;
    lcdUpdate();
}


/*------------------------------------------------------------------------------

  Name         :  lcdGotoXY

  Description  :  Sets cursor location to xy location corresponding to basic font size.

  Argument(s)  :  x, y -> Coordinate for new cursor position. Range: 1,1 .. 14,6

  Return value :  None.

------------------------------------------------------------------------------*/
void lcdGotoXY ( unsigned char x, unsigned char y )
{
    configGlcd();
    LcdCacheIdx = (x - 1) * 6 + (y - 1) * 84;
}

void configGlcd()
{
    if(glcd_ini==0)
    {
        glcd_ini=1;
        lcdInit();
        lcdClear();
    }
}
/*------------------------------------------------------------------------------

  Name         :  lcdSend

  Description  :  Sends data to display controller.

  Argument(s)  :  data -> Data to be sent
                  cd   -> Command or data (see/use enum)
  Note:
   PD7 		: 	NSS Pin
   PD5 		:	D/C Pin (Data/Command)
   PD6 		: 	

  Return value :  None.

------------------------------------------------------------------------------*/
void lcdSend ( unsigned char Data_Send, LcdCmdData cd )
{
    /*
        PD5 is D/C Pin 
        D/C Pin is High, Select Send Data
        D/C Pin is Low, Select Send Command
    */
    if ((SPI2->CR1 & 0x0800) == 0x0800)
    {
      // if Datasize is equal 16bits, then reconfig to 8 bits
      // Reconfig Size Data for GLCD5110
      SPI_DataSizeConfig(SPI2, SPI_DataSize_8b);
    }
  
    uint16_t i;
    if ( cd == LCD_DATA )
    {
      //Select sent to Data Register
      GPIO_SetBits(GLCD_DC_Port, GLCD_DC_Pin);
    }
    else 
    {
      //Select sent command
      GPIO_ResetBits(GLCD_DC_Port, GLCD_DC_Pin);
    }
    
    GPIO_ResetBits(GLCD_NSS_Port, GLCD_NSS_Pin);				// NSS Pin is Low
    
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == RESET)
    {
      while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == SET)
      {
        SPI_I2S_SendData(SPI2, Data_Send);
      }
    }
    for(i=0;i<625;i++);
    GPIO_SetBits(GLCD_NSS_Port, GLCD_NSS_Pin);					// NSS Pin is High
}

/*------------------------------------------------------------------------------

  Name         :  lcdChar	

  Description  :  Displays a character at current cursor location and increment cursor location.

  Argument(s)  :  size -> Font size. See enum.
                  ch   -> Character to write.

  Return value :  None.

--------------------------------------------------------------------------------*/
void lcdChar (char ch )
{
    unsigned char i;
    configGlcd();
    if ( LcdCacheIdx < LoWaterMark )
    {
        //  Update low marker.
        LoWaterMark = LcdCacheIdx;
    }

    if ( (ch < 0x20) || (ch > 0x7b) )
    {
        //  Convert to a printable character.
        ch = 92;
    }

    for ( i = 0; i < 5; i++ )
    {
        LcdCache[LcdCacheIdx++] = FontLookup[ch - 32][i] << 1;
    }
    if ( LcdCacheIdx > HiWaterMark )
    {
        //  Update high marker.
        HiWaterMark = LcdCacheIdx;
    }

    //  Horizontal gap between characters.
    LcdCache[LcdCacheIdx++] = 0x00;
}

/*------------------------------------------------------------------------------

  Name         :  lcdString

  Description  :  Displays a character at current cursor location and increment cursor location
                  according to font size.

  Argument(s)  :  size    -> Font size. See enum.
                  dataPtr -> Pointer to null terminated ASCII string to display.

  Return value :  None.

------------------------------------------------------------------------------*/
void lcdString (unsigned char _x,unsigned char _y,char *dataPtr)
{
    configGlcd();
    lcdGotoXY(_x,_y);
    while (*dataPtr)
    {
        lcdChar(*dataPtr++);
    }
    lcdUpdate();
}
/*------------------------------------------------------------------------------

  Name         :  lcdPixel

  Description  :  Displays a pixel at given absolute (x, y) location.

  Argument(s)  :  x, y -> Absolute pixel coordinates
                  mode -> Off, On or Xor. See enum.

  Return value :  None.

------------------------------------------------------------------------------*/
void lcdPixel(unsigned char x, unsigned char y, unsigned char mode)
{
    unsigned int  index;
    unsigned char  offset;
    unsigned char  data;

    configGlcd();
    if ( x > LCD_X_RES ) return;
    if ( y > LCD_Y_RES ) return;

    index = ((y / 8) * 84) + x;
    offset  = y - ((y / 8) * 8);

    data = LcdCache[index];

    if ( mode == PIXEL_OFF )
	{
        data &= (~(0x01 << offset));
    }
    else if ( mode == PIXEL_ON )
    {
        data |= (0x01 << offset);
    }
    else if ( mode  == PIXEL_XOR )
    {
        data ^= (0x01 << offset);
    }

    LcdCache[index] = data;

    if ( index < LoWaterMark )
    {
        //  Update low marker.
        LoWaterMark = index;
    }

    if ( index > HiWaterMark )
    {
        //  Update high marker.
        HiWaterMark = index;
    }
}

/*------------------------------------------------------------------------------

  Name         :  lcdLine

  Description  :  Draws a line between two points on the display.

  Argument(s)  :  x1, y1 -> Absolute pixel coordinates for line origin.
                  x2, y2 -> Absolute pixel coordinates for line end.
                  mode   -> Off, On or Xor. See enum.

  Return value :  None.

------------------------------------------------------------------------------*/
void lcdLine ( unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char mode )
{
    int dx, dy, stepx, stepy, fraction;
    configGlcd();
    dy = y2 - y1;
    dx = x2 - x1;

    if ( dy < 0 )
    {
        dy    = -dy;
        stepy = -1;
    }
    else
    {
        stepy = 1;
    }

    if ( dx < 0 )
    {
        dx    = -dx;
        stepx = -1;
    }
    else
    {
        stepx = 1;
    }

    dx <<= 1;
    dy <<= 1;

    lcdPixel( x1, y1, mode );

    if ( dx > dy )
    {
        fraction = dy - (dx >> 1);
        while ( x1 != x2 )
        {
            if ( fraction >= 0 )
            {
                y1 += stepy;
                fraction -= dx;
            }
            x1 += stepx;
            fraction += dy;
            lcdPixel( x1, y1, mode );
        }
    }
    else
    {
        fraction = dx - (dy >> 1);
        while ( y1 != y2 )
        {
            if ( fraction >= 0 )
            {
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;
            lcdPixel( x1, y1, mode );
        }
    }

    UpdateLcd = TRUE;
}

/*------------------------------------------------------------------------------

  Name         :  lcdUpdate

  Description  :  Copies the LCD cache into the device RAM.

  Argument(s)  :  None.

  Return value :  None.

------------------------------------------------------------------------------*/
void lcdUpdate ( void )
{
    int i;

    if ( LoWaterMark < 0 )
        LoWaterMark = 0;
    else if ( LoWaterMark >= LCD_CACHE_SIZE )
        LoWaterMark = LCD_CACHE_SIZE - 1;

    if ( HiWaterMark < 0 )
        HiWaterMark = 0;
    else if ( HiWaterMark >= LCD_CACHE_SIZE )
        HiWaterMark = LCD_CACHE_SIZE - 1;

    //  Set base address according to LoWaterMark.
    lcdSend( 0x80 | (LoWaterMark % LCD_X_RES), LCD_CMD );
    lcdSend( 0x40 | (LoWaterMark / LCD_X_RES), LCD_CMD );

    //  Serialize the video buffer.
    for ( i = LoWaterMark; i <= HiWaterMark; i++ )
    {
        lcdSend( LcdCache[i], LCD_DATA );
    }

    //  Reset watermark pointers.
    LoWaterMark = LCD_CACHE_SIZE - 1;
    HiWaterMark = 0;

    UpdateLcd = FALSE;
}
/*------------------------------------------------------------------------------
// Purpose:       Draw a rectangle on a graphic LCD
// Inputs:        (x1, y1) - the start coordinate
//                (x2, y2) - the end coordinate
//                fill  - YES or NO
//                color - ON or OFF
// Dependencies:  glcd_pixel(), glcd_line()
------------------------------------------------------------------------------*/
void lcdRect(	unsigned char x1, 
				unsigned char y1, 
				unsigned char x2, 
				unsigned char y2, 
				unsigned char fill, 
				unsigned char color)
{
   if(fill)
   {
      unsigned char  i, xmin, xmax, ymin, ymax;
      
      if(x1 < x2)     //  Find x min and max
      {
         xmin = x1;
         xmax = x2;
      }
      else
      {
         xmin = x2;
         xmax = x1;
      }

      if(y1 < y2)                            // Find the y min and max
      {
         ymin = y1;
         ymax = y2;
      }
      else
      {
         ymin = y2;
         ymax = y1;
      }

      for(; xmin <= xmax; ++xmin)
      {
         for(i=ymin; i<=ymax; ++i)
         {
            lcdPixel(xmin, i, color);
         }
      }
   }
   else
   {
      lcdLine(x1, y1, x2, y1, color);      // Draw the 4 sides
      lcdLine(x1, y2, x2, y2, color);
      lcdLine(x1, y1, x1, y2, color);
      lcdLine(x2, y1, x2, y2, color);
   }
}
//------------------------------------------------------------------------------
//------------------------Function for draw Progress Bar -----------------------
//------------------------------------------------------------------------------
void lcdProgBar(    unsigned char _x, 
				    unsigned char _y, 
				    unsigned char width, 
				    unsigned char high, 
				    unsigned char percentage)
{
    lcdRect(_x,_y,_x+width,_y+high,0,PIXEL_ON);   // Draw Beyon
    lcdRect(_x+1,_y+1,_x+width-1,_y+high-1,1,PIXEL_OFF);    // Clear fill
    lcdRect(_x,_y,_x+((percentage*width)/100),_y+high,1,PIXEL_ON); // Paint color @ percentage

}
void lcdBackLight(char set)
{
  if(set == SET)
  {
    GPIO_SetBits(GLCD_LED_Port,GLCD_LED_Pin);                                   // Control LED back light on
  }
  else if (set ==  RESET)
  {
    GPIO_ResetBits(GLCD_LED_Port, GLCD_LED_Pin);                                // Control LED back light off
  }
}

//--------------------------------- END of File --------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
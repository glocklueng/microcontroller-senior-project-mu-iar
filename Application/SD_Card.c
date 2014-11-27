/*
Project : Programmable Control of Airflow System for Maintaining Oxygen Saturation in Pre-term infant 
Microcontroller : STM32F4 Discovery (STM32F407VG)
File : SD_Card.c
Deverloper : Phattaradanai Kiratiwudhikul
Deverloped by Department of Electrical Engineering, Faculty of Engineering, Mahidol University
*/
//------------------------------------------------------------------------------
#include "main.h"
#include "SD_Card.h"
#include "diskio.h"
#include "ff.h"
//------------------------------------------------------------------------------
#define OxygenSaturation_file           0
#define FiO2_file                       1


// SD Card Typedef -------------------------------------------------------------
/* Private typedef -----------------------------------------------------------*/
//SD_Error Status = SD_OK;
extern FATFS filesystem;		                                                    // volume lable
extern FRESULT ret;			                                                        // Result code
extern FIL file;				                                                        // File object
extern DIR dir;				                                                          // Directory object
extern FILINFO fno;			                                                        // File information object
extern UINT bw, br;
extern uint8_t buff[128];
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void fault_err (FRESULT rc)
{
  const char *str =
                    "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
                    "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
                    "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
                    "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
  FRESULT i;

  for (i = (FRESULT)0; i != rc && *str; i++) 
  {
    while (*str++) ;
  }
  printf("rc=%u FR_%s\n\r", (UINT)rc, str);
  STM_EVAL_LEDOff(LED6);
  while(1);
}

//------------------------------------------------------------------------------
///**
//  * @brief  Delay
//  * @param  None
//  * @retval None
//  */
//static void Delay(__IO uint32_t nCount)
//{
//  __IO uint32_t index = 0; 
//  for (index = (100000 * nCount); index != 0; index--);
//}

//------------------------------------------------------------------------------
void Check_Mount(void)
{
    /*Write Data to SD Card */
  if (f_mount(0, &filesystem) != FR_OK)
  {
    
  }
  ret = f_open(&file, "OXY.TXT", FA_WRITE | FA_CREATE_ALWAYS);
    if (ret) 
    {
      fault_err(ret);
    } 
    else 
    {
      ret = f_write(&file, "HR : ", 5, &bw);
      ret = f_close(&file);
    }
}

//------------------------------------------------------------------------------
/*
    File Name : Hospital Number
    File Type : Select between OxygenSatuation_file or FiO2_file
*/
void Create_file(char Hospital_Number[], uint8_t File_Type)
{
  char HospitalNumber_File[34];
  for (int i = 0; i < 13; i++)
  {
    HospitalNumber_File[i] = Hospital_Number[i];
  }

  if(File_Type == 0)
  {

    HospitalNumber_File[13] = 'O';
    HospitalNumber_File[14] = 'x';
    HospitalNumber_File[15] = 'y';
    HospitalNumber_File[16] = 'g';
    HospitalNumber_File[17] = 'e';
    HospitalNumber_File[18] = 'n';
    HospitalNumber_File[19] = 'S';
    HospitalNumber_File[20] = 'a';
    HospitalNumber_File[21] = 't';
    HospitalNumber_File[22] = 'u';
    HospitalNumber_File[23] = 'r';
    HospitalNumber_File[24] = 'a';
    HospitalNumber_File[25] = 't';
    HospitalNumber_File[26] = 'i';
    HospitalNumber_File[27] = 'o';
    HospitalNumber_File[28] = 'n';
    HospitalNumber_File[29] = '.';
    HospitalNumber_File[30] = 'T';
    HospitalNumber_File[31] = 'X';
    HospitalNumber_File[32] = 'T';
    HospitalNumber_File[33] = '\0';
    
    ret = f_open(&file, HospitalNumber_File, FA_WRITE | FA_CREATE_ALWAYS);
    //ret = f_open(&file, "HospitalNumber_File", FA_WRITE | FA_CREATE_ALWAYS);
    if (ret) 
    {
      fault_err(ret);
    } 
    else 
    {
      ret = f_write(&file, "HR : ", 5, &bw);
      ret = f_lseek(&file,f_size(&file));
      ret = f_write(&file, HospitalNumber_File, 30, &bw);
      ret = f_lseek(&file,f_size(&file));
      ret = f_write(&file, "\r\nFile: Oxygen Saturation\r\n", 32, &bw);
      ret = f_close(&file);
    }  
  }
  else if (File_Type == FiO2_file)
  {
    HospitalNumber_File[13] = 'F';
    HospitalNumber_File[14] = 'i';
    HospitalNumber_File[15] = 'O';
    HospitalNumber_File[16] = '2';
    HospitalNumber_File[17] = '.';
    HospitalNumber_File[18] = 'T';
    HospitalNumber_File[19] = 'X';
    HospitalNumber_File[20] = 'T';
    HospitalNumber_File[21] = '\0';
    for (int j = 22; j < 33; j++)
    {
      HospitalNumber_File[j] = '\0';
    }

    //ret = f_open(&file, HospitalNumber_File, FA_WRITE | FA_CREATE_ALWAYS);
    ret = f_open(&file, HospitalNumber_File, FA_WRITE | FA_CREATE_ALWAYS);
    if (ret) 
    {
      fault_err(ret);
    } 
    else 
    {
      ret = f_write(&file, "HR : ", 5, &bw);
      ret = f_lseek(&file,f_size(&file));
      ret = f_write(&file, HospitalNumber_File, 30, &bw);
      ret = f_lseek(&file,f_size(&file));
      ret = f_write(&file, "\r\nFile: FiO2_File\r\n", 25, &bw);
      ret = f_close(&file);
    }  

  }
}
//------------------------------------------------------------------------------
void SD_Write(char FileName[], char SD_Data[], UINT Data_size)
{
  ret = f_open(&file, FileName, FA_WRITE);
  if (ret) 
  {
    fault_err(ret);
  } 
  else 
  {
    ret = f_lseek (&file,f_size(&file));
    ret = f_write(&file, SD_Data, Data_size, &bw);
    ret = f_close(&file);
  }  
}

// End of File -----------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------
(C) Copyright 2014, Department of Electrical Engineering, Faculty of Engineering, Mahidol University
--------------------------------------------------------------------------------------------------*/
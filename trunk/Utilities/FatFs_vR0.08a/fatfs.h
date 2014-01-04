#ifndef _FAT_FS_H
#define _FAT_FS_H

#include "ff.h"
#include "diskio.h"

#define FATFS_MES(x)                    ERROR_TEXT_STRING[(x)]

extern const TCHAR* ERROR_TEXT_STRING[];

BYTE SD_Card_Init(void);
void SD_Card_DeInit(void);
BYTE Is_SD_Card_Init(void);

#endif /* _FAT_FS_H */
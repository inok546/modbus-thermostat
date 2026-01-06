#include "stm32f4xx.h"

#include "usart.h"
#include "gpio.h"

#include "sdcard.h"
#include "ff.h"
#include "ffconf.h"
#include "diskio.h"

#include <stdio.h>
#include <stdlib.h>   //for malloc()
#include <string.h>   //for strlen()

#define MAX_BYTES_TO_READ (uint16_t)2048
#define USART1_RX_BUF_SIZE 64
#define SD_FATFS_FILENAME_MAX 64

FRESULT SD_CardMount(void);
FRESULT SD_CardFileRead(const char* name);              // not supported in this project
FRESULT SD_CardCreateFile(const char* name);

FRESULT SD_Initialization(void);
FRESULT SD_CardUnmount(void);
FRESULT SD_CardOpenFile(const char* name);
FRESULT SD_WriteStr(const char* name, const char* s);

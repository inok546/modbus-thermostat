#include "sd_fatfs.h"
#include "rtc.h"

void Logger_Init(void);
void Logger_WriteLog(const char* str);

char* Logger_BuildFileName(char *out, size_t out_sz);
char* RTC_GetISO8601_ForFilename(char *out, size_t out_sz);

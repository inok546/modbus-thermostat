#include "logger.h"

static char log_file_name[SD_FATFS_FILENAME_MAX];        // Создание имени файла
FRESULT FS_SD_CARD_STATE;

void Logger_Init(void)
{
    RTC_Init_LSE_1Hz();     // Инициализация RTC для получения текущего времени
    
    FS_SD_CARD_STATE = SD_Initialization();    // Инициализация SD-карты и ее монтирование
    FS_SD_CARD_STATE = SD_CardMount();

    if(FS_SD_CARD_STATE != FR_OK){
      printf("[ERROR][FS]");
    }
    //char log_file_name[SD_FATFS_FILENAME_MAX] = "THERMOSTAT-2026-01-06T20-13-3128Z.log";        // Создание имени файла
    Logger_BuildFileName(log_file_name, sizeof(log_file_name));
    
    SD_CardCreateFile(log_file_name);
    SD_CardOpenFile(log_file_name);                   // Создание файла лога
    
    //SD_CardUnmount();
}

void Logger_WriteLog(const char* str){
    SD_WriteStr(log_file_name, str);
}



/*
Добавляет к строке out временную метку в формате ISO8601
Пример: "THERMOSTAT-2026-01-06T12-32-00Z.log"
*/
char* Logger_BuildFileName(char *out, size_t out_sz) {
    char iso[32];
    RTC_GetISO8601_ForFilename(iso, sizeof(iso));

    (void)snprintf(out, out_sz, "THERMOSTAT-%s.log", iso);
    return out;
}

// Запишет ISO в out (например "2026-01-06T12-32-00Z")
char* RTC_GetISO8601_ForFilename(char *out, size_t out_sz) {
    rtc_datetime_t now;
    RTC_GetDateTime(&now);

    // ВАЖНО: без двоеточий, чтобы FAT не ругался
    // THERMOSTAT - 2026-01-06T12-32-00Z.log
    // но пробелы тоже иногда не любят, так что лучше без пробелов:
    // THERMOSTAT-2026-01-06T12-32-00Z.log

    (void)snprintf(out, out_sz, "%04u-%02u-%02uT%02u-%02u-%02uZ",
                   (unsigned)now.year, (unsigned)now.month, (unsigned)now.day,
                   (unsigned)now.hour, (unsigned)now.min, (unsigned)now.sec);
    return out;
}


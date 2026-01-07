#include "sd_fatfs.h"

FATFS fs;
FRESULT FR_res;
FIL file;
FILINFO file_info;
DIR dir;

SD_CardInfo SDCardInfo;


FRESULT SD_Initialization(void) {
  printf("[INFO][FS] SD-card initialization started \n");
  SD_Error res = SD_Init();

  if (res == SD_OK) {
    printf("[INFO][FS] SD-card initialization completed! \n");

    // Получаем информацию о карте
    printf("[INFO][FS] SD-card getting information: \n");
    SD_GetCardInfo(&SDCardInfo);
    printf("\tBlock size: %d\n", SDCardInfo.CardBlockSize);
    printf("\tCapacity: %d\n", SDCardInfo.CardCapacity);
    printf("\tCard type: %d\n", SDCardInfo.CardType);
    printf("\tRCA: %d\n", SDCardInfo.RCA);
    printf("\tSN: %d\n", SDCardInfo.SD_cid.ProdSN);
    printf("\tProduct Name: %d %d\n", SDCardInfo.SD_cid.ProdName1, SDCardInfo.SD_cid.ProdName2);
    printf("\n");

    // Выбор карты
    printf("[INFO][FS] SD-card selecting \n");
    SD_SelectDeselect((uint32_t)(SDCardInfo.RCA << 16));
    // режим работы карты - POLLING MODE
    SD_SetDeviceMode(SD_POLLING_MODE);
  } else {
    printf("[ERROR][FS] SD-card not found \n");
    return FR_NOT_READY;
  }

  return FR_OK;
}



FRESULT SD_CardMount(void) {

  printf("[INFO][FS] SD-card FATFS mounting \n");
  FR_res = f_mount(&fs, "/", 1);    // примонтировать раздел немедленно

  if (FR_res != FR_OK) {
    printf("[ERROR][FS] Mounting failed, ErrorCode = %d\r\n", FR_res);
    return FR_INT_ERR;
  }
  
  printf("[INFO][FS] Mounting done!\r\n");

  return FR_OK;
}



FRESULT SD_CardUnmount(void){
  
  // Проверка что файл закрыт
  if(file.flag != 0){
    printf("[WARNING][FS] File \"%s\" does not closed. Closing ... \n", file_info.fname);
    FR_res = f_sync(&file);
    FR_res = f_close(&file);
    
    if (FR_res == FR_OK) 
      printf("[INFO][FS] File \"%s\" succses closed. \n", file_info.fname);
    else
      printf("[ERROR][FS] Error while closing file: \"%s\" \n", file_info.fname);
  }

  FR_res = f_mount(NULL, "0:", 0);    // размонтировать раздел

  if (FR_res != FR_OK) {
    printf("[ERROR][FS] Unmount failed, ErrorCode = %d\r\n", FR_res);
    return FR_INT_ERR;
  }

  printf("[INFO][FS] Unmount done!\r\n");

  return FR_OK;
}


FRESULT SD_CardOpenFile(const char* file_name) {
  uint8_t readed_data[MAX_BYTES_TO_READ];
  unsigned int BytesReaded = 0;

  printf("[INFO][FS] Checking for an existing file \"%s\" on the SD-card \n", file_name);
  FR_res = f_stat(file_name, &file_info);

  // Проверка на существование файла
  if (FR_res == FR_NO_FILE) {                                        
    // Если НЕ нашли файл, создаем новый
    printf("[WARNING][FS] File \"%s\" does not exist on SD-card. Creating new one... \n", file_name);
  }
  else if (FR_res == FR_OK){
    // Если нашли, выводим метаданные файла
    printf("[INFO][FS] Found file \"%s\" \n", file_info.fname);
    printf("[INFO][FS] File size = %d bytes \n", (uint32_t)file_info.fsize);
    printf("[INFO][FS] Opening file \"%s\" ... \n", file_info.fname);  
  }
  else{
    printf("[ERROR][FS] Error opening file. ErrorCode res = %d \n", FR_res);
    return FR_res;
  }

  // Открываем с указателем на конец файла
  FR_res = f_open(&file, file_name, FA_OPEN_APPEND | FA_WRITE);

  // Лог после открытия файла
  if (FR_res == FR_OK) {
    printf("[INFO][FS] Opening file completed successfully \n", FR_res);
  } else
    printf("[ERROR][FS] Error opening file. ErrorCode = %d \n", FR_res);
    
  return FR_res;
}



FRESULT SD_WriteStr(const char* file_name, const char* str){
  
   unsigned int writedBytes = 0;

  // Проверка что файл открыт
  if(file.flag == 0){
    printf("[ERROR][FS] File \"%s\" does not opened \n", file_name);
    FR_res = SD_CardOpenFile(file_name);
  }
  // Синхрон перед записью
  FR_res = f_sync(&file);
  

  if (FR_res == FR_OK){
      FR_res = f_write(&file, str, strlen(str), &writedBytes); // write string file_text into file
  }
  else
    printf("[ERROR][FS] Error opening file. ErrorCode = %d \n", FR_res);

  if(FR_res == FR_OK){
    printf("[INFO][FS] Writing %d bytes into file \"%s\" successfull \n", writedBytes, file_name);
    FR_res = f_sync(&file); //WARNING: Всегда выдает ошибку FR_DISK_ERR, мб из-за низкой задержки между командами
  }
  else{
    printf("[ERROR][FS] Error writing string into file. ErrorCode = %d \n", FR_res);
    FR_res = f_sync(&file); //WARNING: Всегда выдает ошибку FR_DISK_ERR, мб из-за низкой задержки между командами
  }

  return FR_res;
}



FRESULT SD_CardCreateFile(const char* file_name){
    uint16_t WritedBytes = 0;

    FR_res = f_open(&file, file_name, FA_CREATE_ALWAYS|FA_READ|FA_WRITE);

    if (FR_res == FR_OK) {
        printf("[INFO][FS] File \"%s\" was created successfully \n", file_name);
        FR_res = f_close(&file);
    }
    else
      printf("[ERROR][FS] Creating file \"%s\" failed! Error code = %d \n", file_name, FR_res);

    return FR_res;
}

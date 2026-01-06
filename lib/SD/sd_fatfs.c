#include "sd_fatfs.h"

FATFS fs;
FRESULT FR_res;
FIL file;
FILINFO file_info;
DIR dir;

SD_CardInfo SDCardInfo;


FRESULT SD_Initialization(void) {
  printf("[INFO] SD-card initialization started \n");
  SD_Error res = SD_Init();

  if (res == SD_OK) {
    printf("[INFO] SD-card initialization completed! \n");

    // Получаем информацию о карте
    printf("[INFO] SD-card getting information: \n");
    SD_GetCardInfo(&SDCardInfo);
    printf("Block size: %d\n", SDCardInfo.CardBlockSize);
    printf("Capacity: %d\n", SDCardInfo.CardCapacity);
    printf("Card type: %d\n", SDCardInfo.CardType);
    printf("RCA: %d\n", SDCardInfo.RCA);
    printf("SN: %d\n", SDCardInfo.SD_cid.ProdSN);
    printf("Product Name: %d %d\n", SDCardInfo.SD_cid.ProdName1, SDCardInfo.SD_cid.ProdName2);
    printf("\n");

    // Выбор карты
    printf("[INFO] SD-card selecting \n");
    SD_SelectDeselect((uint32_t)(SDCardInfo.RCA << 16));
    // режим работы карты - POLLING MODE
    SD_SetDeviceMode(SD_POLLING_MODE);
  } else {
    printf("[ERROR] SD-card not found \n");
    return FR_NOT_READY;
  }

  //================== MOUNTING ==================

  printf("[INFO] SD-card FATFS Mouning \n");
  FR_res = SD_CardMount();

  return FR_OK;
}



FRESULT SD_CardMount(void) {
  FR_res = f_mount(&fs, "/", 1);    // примонтировать раздел немедленно

  if (FR_res != FR_OK) {
    printf("[ERROR] Mounting failed, ErrorCode = %d\r\n", FR_res);
    return FR_INT_ERR;
  }
  
  printf("[INFO] Mounting done!\r\n");

  return FR_OK;
}



FRESULT SD_CardUnmount(void){
  
  // Проверка что файл закрыт
  if(file.flag != 0){
    printf("[WARNING] File \"%s\" does not closed. Closing ... \n", file_info.fname);
    FR_res = f_sync(&file);
    FR_res = f_close(&file);
    
    if (FR_res == FR_OK) 
      printf("[INFO] File \"%s\" succses closed. \n", file_info.fname);
    else
      printf("[ERROR] Error while closing file: \"%s\" \n", file_info.fname);
  }

  FR_res = f_mount(NULL, "0:", 0);    // размонтировать раздел

  if (FR_res != FR_OK) {
    printf("[ERROR] Unmount failed, ErrorCode = %d\r\n", FR_res);
    return FR_INT_ERR;
  }

  printf("[INFO] Unmount done!\r\n");

  return FR_OK;
}


FRESULT SD_CardOpenFile(const char* file_name) {
  uint8_t readed_data[MAX_BYTES_TO_READ];
  unsigned int BytesReaded = 0;

  printf("[INFO] Checking for an existing file \"%s\" on the SD-card \n", file_name);
  FR_res = f_stat(file_name, &file_info);

  // Проверка на существование файла
  if (FR_res == FR_NO_FILE) {                                        
    // Если НЕ нашли файл, создаем новый
    printf("[WARNING] File \"%s\" does not exist on SD-card. Creating new one... \n", file_name);
  }
  else if (FR_res == FR_OK){
    // Если нашли, выводим метаданные файла
    printf("[INFO] Found file \"%s\" \n", file_info.fname);
    printf("[INFO] File size = %d bytes \n", (uint32_t)file_info.fsize);
    printf("[INFO] Opening file \"%s\" ... \n", file_info.fname);  
  }
  else{
    printf("[ERROR] Error opening file. ErrorCode res = %d \n", FR_res);
    return FR_res;
  }

  // Открываем с указателем на конец файла
  FR_res = f_open(&file, file_name, FA_OPEN_APPEND | FA_WRITE);

  // Лог после открытия файла
  if (FR_res == FR_OK) {
    printf("[INFO] Opening file completed successfully \n", FR_res);
  } else
    printf("[ERROR] Error opening file. ErrorCode = %d \n", FR_res);
    
  return FR_res;
}



FRESULT SD_WriteStr(const char* file_name, const char* str){
  
   unsigned int writedBytes = 0;

  // Проверка что файл открыт
  if(file.flag == 0){
    printf("[ERROR] File \"%s\" does not opened \n", file_name);
    FR_res = SD_CardOpenFile(file_name);
  }
  // Синхрон перед записью
  FR_res = f_sync(&file);
  

  if (FR_res == FR_OK){
      FR_res = f_write(&file, str, strlen(str), &writedBytes); // write string file_text into file
  }
  else
    printf("[ERROR] Error opening file. ErrorCode = %d \n", FR_res);

  if(FR_res == FR_OK){
    printf("[INFO] Writing %d bytes into file \"%s\" successfull \n", writedBytes, file_name);
    FR_res = f_sync(&file); //WARNING: Всегда выдает ошибку FR_DISK_ERR, мб из-за низкой задержки между командами
  }
  else{
    printf("[ERROR] Error writing string into file. ErrorCode = %d \n", FR_res);
    FR_res = f_sync(&file); //WARNING: Всегда выдает ошибку FR_DISK_ERR, мб из-за низкой задержки между командами
  }

  return FR_res;
}



FRESULT SD_CardCreateFile(const char* file_name){
    uint16_t WritedBytes = 0;

    FR_res = f_open(&file, file_name, FA_CREATE_ALWAYS|FA_READ|FA_WRITE);

    if (FR_res == FR_OK) {
        printf("[INFO] File \"%s\" was created successfully \n", file_name);
        FR_res = f_close(&file);
    }
    else
      printf("[ERROR] Creating file \"%s\" failed! Error code = %d \n", file_name, FR_res);

    return FR_res;
}

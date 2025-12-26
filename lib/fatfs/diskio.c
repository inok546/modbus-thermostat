/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h" /* Declarations of disk functions */
#include "ff.h"     /* Obtains integer types */
#include "sdcard.h" /* Declarations of disk functions */

/* Definitions of physical drive number for each drive */
#define DEV_RAM 0 /* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC 1 /* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB 2 /* Example: Map USB MSD to physical drive 2 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(BYTE pdrv /* Physical drive nmuber to identify the drive */
) {
  // DSTATUS stat;
  // int result;

  // switch (pdrv) {
  // case DEV_RAM :
  //	result = RAM_disk_status();

  //	// translate the reslut code here

  //	return stat;

  // case DEV_MMC :
  //	result = MMC_disk_status();

  //	// translate the reslut code here

  //	return stat;

  // case DEV_USB :
  //	result = USB_disk_status();

  //	// translate the reslut code here

  //	return stat;
  //}
  // return STA_NOINIT;
  return 0;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE pdrv /* Physical drive nmuber to identify the drive */
) {
  DSTATUS stat;
  int result;

  // switch (pdrv) {
  // case DEV_RAM :
  //	result = RAM_disk_initialize();

  //	// translate the reslut code here

  //	return stat;

  // case DEV_MMC :
  //	result = MMC_disk_initialize();

  //	// translate the reslut code here

  //	return stat;

  // case DEV_USB :
  //	result = USB_disk_initialize();

  //	// translate the reslut code here

  //	return stat;
  //}
  // return STA_NOINIT;
  return 0;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(BYTE pdrv,    /* Physical drive nmuber to identify the drive */
                  BYTE *buff,   /* Data buffer to store read data */
                  LBA_t sector, /* Start sector in LBA */
                  UINT count    /* Number of sectors to read */
) {
  // check values here: sector, buff, count passed here! Find how they used inside SD_ReadBlockBytes()
  SD_Error errorstatus = SD_OK;

  if (count > 1) {
    errorstatus = SD_ReadMultiBlocksBytes(sector, buff, SD_BLOCK_SIZE_BYTES, count);
    if (errorstatus != SD_OK) {
      return RES_ERROR;
    }
  } else {
    if (count == 1) {
      errorstatus = SD_ReadBlockBytes(sector, buff, SD_BLOCK_SIZE_BYTES);
      if (errorstatus != SD_OK) {
        return RES_ERROR;
      }
    }
  }

  if (errorstatus != SD_OK) {
    return RES_ERROR;
  }

  return RES_OK;
}
//{
//	DRESULT res;
//	int result;

//	switch (pdrv) {
//	case DEV_RAM :
//		// translate the arguments here

//		result = RAM_disk_read(buff, sector, count);

//		// translate the reslut code here

//		return res;

//	case DEV_MMC :
//		// translate the arguments here

//		result = MMC_disk_read(buff, sector, count);

//		// translate the reslut code here

//		return res;

//	case DEV_USB :
//		// translate the arguments here

//		result = USB_disk_read(buff, sector, count);

//		// translate the reslut code here

//		return res;
//	}

//	return RES_PARERR;
//}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE pdrv,        /* Physical drive nmuber to identify the drive */
                   const BYTE *buff, /* Data to be written */
                   LBA_t sector,     /* Start sector in LBA */
                   UINT count        /* Number of sectors to write */
) {
  SD_Error errorstatus = SD_OK;
  if (count > 1) {
    errorstatus = SD_WriteMultiBlockBytes(sector, buff, SD_BLOCK_SIZE_BYTES, count);
    if (errorstatus != SD_OK) {
      return RES_ERROR;
    }
  } else {
    if (count == 1) {
      errorstatus = SD_WriteBlockBytes(sector, buff, SD_BLOCK_SIZE_BYTES);
      if (errorstatus != SD_OK) {
        return RES_ERROR;
      }
    }
  }

  if (errorstatus != SD_OK) {
    return RES_ERROR;
  }

  return RES_OK;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
  if (pdrv != DEV_MMC) {
    return RES_PARERR;   // если у тебя только SD-карта
  }

  switch (cmd) {
  case CTRL_SYNC:
    // дождаться готовности карты
    // например:
    // while (SD_GetStatus() != SD_OK) { }
    return RES_OK;

  case GET_SECTOR_SIZE:
    *(WORD *)buff = SD_BLOCK_SIZE_BYTES;    // обычно 512
    return RES_OK;

  case GET_BLOCK_SIZE:
    *(DWORD *)buff = 1; // либо размер erase-блока в секторах, если знаешь
    return RES_OK;

  default:
    return RES_PARERR;
  }
}


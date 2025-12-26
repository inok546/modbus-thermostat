#ifndef I2C_H
#define I2C_H
  #include "stm32f407xx.h"

  #define I2C_DEV_ADDR  0xA0    // адрес EEPROM = 1010_0000 в бинарном виде. Используется старшие 7 бит
  #define I2C_WR_BIT    0x00    // запрос на запись в I2C-устройство (в EEPROM)
  #define I2C_RD_BIT    0x01    // запрос на чтение из I2C-устройство (из EEPROM)
  #define I2C_DEV_ADDR_WR (I2C_DEV_ADDR + I2C_WR_BIT)   // младший бит выставляем в WR=1 => 0xA0
  #define I2C_DEV_ADDR_RD (I2C_DEV_ADDR + I2C_RD_BIT)   // младший бит выставляем в RD=1 => 0xA1
  
  void I2C_Init_EEPROM(void);
  void I2C_Start_gen(void);
  void I2C_TxDeviceADDR(char device_addr, char RW_bit);
  void I2C_Wrtire(char init_addr, char tx_data[], uint16_t data_len);
  void I2C_Read(char init_addr, char rt_data[], uint16_t data_len);

#endif
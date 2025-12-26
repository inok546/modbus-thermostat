#include "eeprom.h"
#include "i2c.h"
#include "delay.h"

#define EEPROM_HEADER_ADDR  ((char)0x00)  // Адрес записи с 1 ячейки на странице 1

void EEPROM_Init(void) {
  I2C_Init_EEPROM();
}

eeprom_status_t eeprom_save_sequence(const uint8_t *seq, uint8_t len) {
  if (!seq) return EEPROM_ERR_BAD_LEN;
  if (len == 0 || len > EEPROM_PAGE_LEN_BYTES) return EEPROM_ERR_BAD_LEN;

  uint8_t page[EEPROM_PAGE_LEN_BYTES] = {0};

  for (uint8_t i = 0; i < len; i++) {
    page[i] = seq[i];
  }

  I2C_Wrtire(EEPROM_HEADER_ADDR, (char*)page, (uint16_t)EEPROM_PAGE_LEN_BYTES);
  Delay_ms(5);
  return EEPROM_OK;
}

eeprom_status_t eeprom_load_sequence(uint8_t *seq, uint8_t len) {
  uint8_t page[EEPROM_PAGE_LEN_BYTES] = {0};
  I2C_Read(EEPROM_HEADER_ADDR, (char*)seq, (uint16_t)EEPROM_PAGE_LEN_BYTES);
  return EEPROM_OK;
}

/*
eeprom_status_t eeprom_clear_sequence(void) {
  uint8_t page[EEPROM_PAGE_LEN_BYTES] = {0};
  I2C_Wrtire(EEPROM_HEADER_ADDR, (char*)page, (uint16_t)EEPROM_PAGE_LEN_BYTES);
  Delay_ms(5);
  return EEPROM_OK;
}
*/
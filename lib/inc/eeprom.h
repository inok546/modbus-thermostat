#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>
#include <stddef.h>

// TODO: Поправить формат записи в EEPROM по адресу 0x00 (8 байт):
/*
*/

//#define EEPROM_MAGIC           0xAAu
#define EEPROM_PAGE_LEN_BYTES  8u

typedef enum {
  EEPROM_OK = 0,
  EEPROM_ERR_EMPTY,     // magic не совпал -> данных нет
  EEPROM_ERR_BAD_LEN    // len битый/не влезает
} eeprom_status_t;

void EEPROM_Init(void);

eeprom_status_t eeprom_save_sequence(const uint8_t *seq, uint8_t len);
eeprom_status_t eeprom_load_sequence(uint8_t *seq, uint8_t len);
eeprom_status_t eeprom_clear_sequence(void);

#endif
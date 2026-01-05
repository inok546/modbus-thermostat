#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>
#include <stddef.h>


//-------- EEPROM AT24C02B max page size --------
#define EEPROM_PAGE_LEN_BYTES  8u
#define EEPROM_CONFIG_SEQ_LEN 8


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
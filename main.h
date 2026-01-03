#ifndef MAIN_H
#define MAIN_H

#include "stm32f4xx.h"
#include <stdio.h>
#include <stdlib.h>   //for malloc()
#include <string.h>   //for strlen()

#include "delay.h"
#include "modbus_rtu.h"
#include "lcd1602.h"

#include "DS18B20.h"
#include "one_wire.h"

#include "i2c.h"
#include "eeprom.h"

#include "sdcard.h"
#include "ff.h"
#include "ffconf.h"
#include "diskio.h"

#include "gpio.h"

//-------- EEPROM AT24C02B max page size --------
#define EEPROM_CONFIG_SEQ_LEN 8

//-------- LCD1602 maximum symbols and strings number --------
#define LCD_CHAR_NUM_MAX	16
#define LCD_STRING_NUM_MAX	2


// TODO: FSM

typedef enum {
  IDLE = 0,
  HEATING,
  COOLING
} thermostat_state;

// Храним половинками, дабы влезть в страницу EEPROM
typedef struct {
  uint16_t forced_heat_hs;      // полсекунды
  uint16_t forced_cool_hs;

  uint16_t heat_off_hyst_x2;    // °C * 2
  uint16_t cool_off_hyst_x2;
  uint16_t heat_on_hyst_x2;
  uint16_t cool_on_hyst_x2;

  uint16_t t_low;    // пороги целевой температуры
  uint16_t t_high;
} thermo_settings_t;

// CONFIGURATION VARIABLES


void RCC_Init(void);

// ============ MODULES ============
// DISPLAY
void LCD1602_Init(volatile thermo_settings_t *s, volatile thermostat_state *st);
void RenderDisplay(float temperature);
void RenderLED(void);

// LOGGER
void Logging(void);

// THERMOSTAT
void Thermostat_Init(volatile thermo_settings_t *s, volatile thermostat_state *st);
void SetMode(float cur_temp);
void Force_SetMode(void);
void UpdateTemperature(float *cur_temp);

void Setting_Set(uint8_t *seq);
void Setting_Get(void);
void EncodeSeqConfig(uint8_t *seq);

// OTHER
static inline float half_to_float_u16(uint16_t hs); // Convert halfseconds to seconds


#endif

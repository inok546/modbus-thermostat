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


#define EEPROM_CONFIG_SEQ_LEN 8

// TODO: FSM

typedef enum {
  IDLE = 0,
  HEATING,
  COOLING
} thermostat_state;


// CONFIGURATION VARIABLES


void RCC_Init(void);

// ============ MODULES ============
// DISPLAY
void RenderDisplay(float temp);
void RenderLED(thermostat_state t_state);

// LOGGER
void Logging(void);

// THERMOSTAT
void ReadConfiguration(void);           // Чтение EEPROM по I2C
thermostat_state SetMode(float cur_temp);
void Force_SetMode(void);
void UpdateTemperature(float* cur_temp);

void Setting_Set(uint8_t *seq);
void Setting_Get(void);
void EncodeSeqConfig(uint8_t *seq);

// OTHER
static inline float half_to_float_u16(uint16_t hs); // Convert halfseconds to seconds


#endif

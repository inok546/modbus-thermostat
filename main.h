#ifndef MAIN_H
#define MAIN_H

#include "stm32f4xx.h"
#include <stdio.h>
#include <stdlib.h>   //for malloc()
#include <string.h>   //for strlen()
#include "thermostat_types.h"

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


void RCC_Init(void);

// ============ MODULES ============
// DISPLAY
void LCD1602_Init(volatile thermostat_settings_t *s, volatile thermostat_state *st, volatile uint8_t *override_state_flag);
void RenderDisplay(float temperature);
void RenderLED(void);

// LOGGER
void Logging(void);

// THERMOSTAT
void Thermostat_Init(volatile thermostat_settings_t *s, volatile thermostat_state *st);
void SetMode(float cur_temp);
void ForceSetMode(volatile uint8_t *force_state_flag);
void UpdateTemperature(float *cur_temp);

void SettingSet(uint8_t *seq);
void SettingGet(void);
void EncodeSeqConfig(uint8_t *seq);

// OTHER
static inline float half_to_float_u16(uint16_t hs); // Convert halfseconds to seconds
static inline float half_sec_to_ms(uint16_t hs); // Convert halfseconds to milliseconds

#endif

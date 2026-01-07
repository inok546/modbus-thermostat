#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include "stm32f4xx.h"

// Значения подобраны по ТЗ
typedef enum {
  IDLE = 2,
  HEATING = 6,
  COOLING = 3
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
} thermostat_settings_t;

typedef struct {
  uint32_t uptime;
  float temperature;
  thermostat_state state;
} thermostat_log_data;
#endif

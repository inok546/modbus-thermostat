#include "main.h"
#include "thermostat_types.h"

static volatile thermostat_settings_t *settings;
static volatile thermostat_state *state;

void Thermostat_Init(volatile thermostat_settings_t *s, volatile thermostat_state *st) {
  settings = s;
  state = st;
  uint8_t config_seq[EEPROM_CONFIG_SEQ_LEN];

  eeprom_status_t epprom_st = eeprom_load_sequence(config_seq, EEPROM_CONFIG_SEQ_LEN);

  if (epprom_st == EEPROM_OK) {
    SettingSet(config_seq);
    //TODO: Вынести это все функцию в модуль Logging
    printf("[INFO][EEPROM] Read configuration:\n");
    printf("\tforced_heat: %.1f sec\n", half_to_float_u16(s->forced_heat_hs));
    printf("\tforced_cool: %.1f sec\n", half_to_float_u16(s->forced_cool_hs));
    printf("\theat_off_hyst: %.1f °C\n", half_to_float_u16(s->heat_off_hyst_x2));
    printf("\tcool_off_hyst: %.1f °C\n", half_to_float_u16(s->cool_off_hyst_x2));
    printf("\theat_on_hyst: %.1f °C\n", half_to_float_u16(s->heat_on_hyst_x2));
    printf("\tcool_on_hyst: %.1f °C\n", half_to_float_u16(s->cool_on_hyst_x2));
    printf("\tt_low: %d °C\n", s->t_low);
    printf("\tt_high: %d °C\n", s->t_high);
    
    //NOTE: Для отладки
    //printf("hyst cool_on %f\n", (s->t_high+half_to_float_u16(s->cool_on_hyst_x2)));
    //printf("hyst heat_on %f\n", (s->t_low-half_to_float_u16(s->heat_on_hyst_x2)));
    //printf("hyst cool_off %f\n", (s->t_high-half_to_float_u16(s->cool_off_hyst_x2)));
    //printf("hyst heat_off %f\n", (s->t_low+half_to_float_u16(s->heat_off_hyst_x2)));
    
  } else {
    printf("[ERROR][EEPROM] EEPROM code error: %d", epprom_st);
    printf("[WARNING][EEPROM] Using default configuration!");
    uint8_t def_config[EEPROM_CONFIG_SEQ_LEN] = {0x0A, 0x0A, 0x04, 0x04, 0x02, 0x02, 0x18, 0x1B}; // конфигурация по умолчанию
    SettingSet(def_config);
  }
}



void SetMode(float t) {
                        
  switch (*state) {
  case IDLE:
    if (t < settings->t_high && t > settings->t_low)
      *state = IDLE;
    else if (t > (settings->t_high+half_to_float_u16(settings->cool_on_hyst_x2)))
      *state = COOLING;
    else if (t < (settings->t_low-half_to_float_u16(settings->heat_on_hyst_x2)))
      *state = HEATING;
    break;

  case HEATING:
    if(t > (settings->t_low+half_to_float_u16(settings->heat_off_hyst_x2)))
      *state = IDLE;
    break;

  case COOLING:
    if(t < (settings->t_high-half_to_float_u16(settings->cool_off_hyst_x2)))
      *state = IDLE;
    break;
  }
  
  printf("[INFO][THERMOSTAT] thermostat state: %d\n",*state); //TODO: Вынести это все функцию в модуль Logging
}


void ForceSetMode(volatile uint8_t *force_state_flag){
  static uint32_t t0 = 0;

  switch (*state) {
  case COOLING:
  //TODO: Добавить ф-цию перевода полсекунд в миллисекунды
    if (systick_elapsed(t0, half_sec_to_ms(settings->forced_cool_hs))){    // По истчении времени сбрасываем флаг
      *force_state_flag = 0;
      t0 += half_to_float_u16(settings->forced_cool_hs)*1000;
    }
    else
      printf("[INFO][THERMOSTAT] Thermostat forse state COOLING by X seconds \n"); //TODO: Вынести это все функцию в модуль Logging
    break;

  case HEATING:
    if (systick_elapsed(t0, half_sec_to_ms(settings->forced_heat_hs))){    // По истчении времени сбрасываем флаг
      *force_state_flag = 0;
      t0 += half_to_float_u16(settings->forced_heat_hs)*1000;
    }
    else
      printf("[INFO][THERMOSTAT] Thermostat forse state HEATING by X seconds \n"); //TODO: Вынести это все функцию в модуль Logging
    break;
  
  default:
    printf("[ERROR][THERMOSTAT] Thermostat forse state not defined \n"); //TODO: Вынести это все функцию в модуль Logging
    break;
  }
}


void SettingSet(uint8_t *seq) {
  settings->forced_heat_hs = seq[0];
  settings->forced_cool_hs = seq[1];

  settings->heat_off_hyst_x2 = seq[2];
  settings->cool_off_hyst_x2 = seq[3];
  settings->heat_on_hyst_x2  = seq[4];
  settings->cool_on_hyst_x2  = seq[5];

  settings->t_low  = seq[6];
  settings->t_high = seq[7];
}

void UpdateTemperature(float* cur_temp) {
  static uint32_t t0 = 0;
  if (systick_elapsed(t0, DS18B20_DELAY_READ)) {    // чтение температуры каждые 3ms
    t0 += DS18B20_DELAY_READ;
    Convert_Temperature();
    *cur_temp = DS18B20_ReadTemperature();
  }
}

static inline float half_to_float_u16(uint16_t hs) {
  return (float)hs * 0.5f;
}

static inline float half_sec_to_ms(uint16_t hs) {
  return half_to_float_u16(hs)*1000;
}
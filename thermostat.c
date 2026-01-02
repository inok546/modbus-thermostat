#include "main.h"

// Храним половинками, дабы влезть в страницу EEPROM
typedef struct {
  uint16_t forced_heat_hs;   // полсекунды
  uint16_t forced_cool_hs;

  uint16_t heat_off_hyst_x2; // °C * 2
  uint16_t cool_off_hyst_x2;
  uint16_t heat_on_hyst_x2;
  uint16_t cool_on_hyst_x2;

  uint16_t t_low;            // пороги целевой температуры
  uint16_t t_high;
} thermo_settings_t;

// Инициализируем со значениями по-умолчанию
//volatile thermo_settings_t settings;
volatile thermo_settings_t s = {
                                        10,   // 5 сек. принудительного нагрева
                                        10,   // 5 сек. принудительного охлаждения
                                        4,    // 2 сек. гистерезис отк. нагрева
                                        4,    // 2 сек. гистерезис отк. охлаждения
                                        4,    // 2 сек. гистерезис вкл. нагрева
                                        4,    // 2 сек. гистерезис вкл. охлаждения
                                        23,   // нижний порог
                                        27    // верхний порог
                                      };

void ReadConfiguration(void) {
  uint8_t seq[EEPROM_CONFIG_SEQ_LEN];

  eeprom_status_t st = eeprom_load_sequence(seq, sizeof(seq));
  if (st == EEPROM_OK) {
    Setting_Set(seq);
    printf("[INFO] READED CONFIGURATION:\n");
    printf("\tforced_heat: %.1f sec\n", half_to_float_u16(s.forced_heat_hs));
    printf("\tforced_cool: %.1f sec\n", half_to_float_u16(s.forced_cool_hs));
    printf("\theat_off_hyst: %.1f °C\n", half_to_float_u16(s.heat_off_hyst_x2));
    printf("\tcool_off_hyst: %.1f °C\n", half_to_float_u16(s.cool_off_hyst_x2));
    printf("\theat_on_hyst : %.1f °C\n", half_to_float_u16(s.heat_on_hyst_x2));
    printf("\tcool_on_hyst : %.1f °C\n", half_to_float_u16(s.cool_on_hyst_x2));
    printf("\tt_low : %d °C\n", s.t_low);
    printf("\tt_high : %d °C\n", s.t_high);
  } else {
    printf("[ERROR] EEPROM code error: %d",st);
  }
}


thermostat_state SetMode(float cur_temp){

  thermostat_state state;

  //if(cur_temp < && cur_temp >)

    switch(state){
      case IDLE:
          break;
    
      case HEATING:
          break;
    
      case COOLING:
          break;
    }

    return state;
}

void Setting_Set(uint8_t *seq){
  s.forced_heat_hs = seq[0];
  s.forced_cool_hs = seq[1];

  s.heat_off_hyst_x2 = seq[2];
  s.cool_off_hyst_x2 = seq[3];
  s.heat_on_hyst_x2 = seq[4];
  s.cool_on_hyst_x2 = seq[5];
  
  s.t_low = seq[6];
  s.t_high = seq[7];
}

void UpdateTemperature(float* cur_temp){
  static uint32_t t0 = 0;
  if(systick_elapsed(t0, 3)){   // чтение температуры каждые 3ms
    t0+=3;
    Convert_Temperature();
    *cur_temp = DS18B20_ReadTemperature();
  }
}


static inline float half_to_float_u16(uint16_t hs) {
  return (float)hs * 0.5f;
}
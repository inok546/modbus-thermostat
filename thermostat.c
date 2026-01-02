#include "main.h"

// Храним половинками, дабы влезть в страницу EEPROM
typedef struct {
  uint16_t forced_heat_hs;   // полсекунды
  uint16_t forced_cool_hs;

  uint16_t heat_off_hyst_x2; // °C * 2
  uint16_t cool_off_hyst_x2;
  uint16_t heat_on_hyst_x2;
  uint16_t cool_on_hyst_x2;
} thermo_settings_t;

// Инициализируем со значениями по-умолчанию
//volatile thermo_settings_t settings;
volatile thermo_settings_t settings = {
                                        10,   // 5 сек. принудительного нагрева
                                        10,   // 5 сек. принудительного охлаждения
                                        4,    // 2 сек. гистерезис отк. нагрева
                                        4,    // 2 сек. гистерезис отк. охлаждения
                                        4,    // 2 сек. гистерезис вкл. нагрева
                                        4,    // 2 сек. гистерезис вкл. охлаждения
                                      };

void ReadConfiguration(void) {
  uint8_t seq[EEPROM_CONFIG_SEQ_LEN];

  eeprom_status_t st = eeprom_load_sequence(seq, sizeof(seq));
  if (st == EEPROM_OK) {
    Setting_Set(seq);
    printf("[INFO] READED CONFIGURATION:\n");
    printf("\tforced_heat: %.1f sec\n", half_to_float_u16(settings.forced_heat_hs));
    printf("\tforced_cool: %.1f sec\n", half_to_float_u16(settings.forced_cool_hs));
    printf("\theat_off_hyst: %.1f °C\n", half_to_float_u16(settings.heat_off_hyst_x2));
    printf("\tcool_off_hyst: %.1f °C\n", half_to_float_u16(settings.cool_off_hyst_x2));
    printf("\theat_on_hyst : %.1f °C\n", half_to_float_u16(settings.heat_on_hyst_x2));
    printf("\tcool_on_hyst : %.1f °C\n", half_to_float_u16(settings.cool_on_hyst_x2));

  } else {
    printf("[ERROR] EEPROM code error: %d",st);
    // данных нет или битые
  }
}


void SetMode(thermostat_state t_state){

    switch(t_state){
      case IDLE:
          break;
    
      case HEATING:
          break;
    
      case COOLING:
          break;
    }

}

void Setting_Set(uint8_t *seq){
  settings.forced_heat_hs = seq[0];
  settings.forced_cool_hs = seq[1];

  settings.heat_off_hyst_x2 = seq[2];
  settings.cool_off_hyst_x2 = seq[3];
  settings.heat_on_hyst_x2 = seq[4];
  settings.cool_on_hyst_x2 = seq[5];
}

void UpdateTemperature(float* cur_temp){
  //TODO: Добавить проверку 3ms
  *cur_temp = DS18B20_ReadTemperature();
}


static inline float half_to_float_u16(uint16_t hs) {
  return (float)hs * 0.5f;
}
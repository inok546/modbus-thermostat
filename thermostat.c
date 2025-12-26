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

volatile thermo_settings_t settings;

void ReadConfiguration(void) {
  uint8_t seq[EEPROM_CONFIG_SEQ_LEN];

  eeprom_status_t st = eeprom_load_sequence(seq, sizeof(seq));
  if (st == EEPROM_OK) {
    Setting_Set(seq);
    //TODO: По-красоте вывод
    printf("[INFO] READED: %d, %d, %d, %d, %d, %d", seq[0],seq[1],seq[2],seq[3],seq[4],seq[5]);

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
  settings.forced_cool_hs = seq[0];
  settings.forced_heat_hs = seq[1];

  settings.heat_off_hyst_x2 = seq[2];
  settings.cool_off_hyst_x2 = seq[3];
  settings.heat_on_hyst_x2 = seq[4];
  settings.cool_on_hyst_x2 = seq[5];
}

void UpdateTemperature(float* cur_temp){

}

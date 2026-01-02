#include "main.h"

thermostat_state t_state = IDLE;
float cur_temp;

//

// TODO: Определить глобальную конфигурацию
// uint8_t config_seq[EEPROM_CONFIG_SEQ_LEN];
uint8_t config_seq[EEPROM_CONFIG_SEQ_LEN] = {0x0A, 0x0A, 0x04, 0x04, 0x02, 0x02, 0x18, 0x1B}; // конфигурация по умолчанию

int main(void) {
  RCC_Init();
  SystemCoreClockUpdate();                    // Обновление SystemCoreClock
  Delay_Init();                               // Запуск DWT для отчета микросекунд
  SysTick_Config(SystemCoreClock / 1000U);    // 1ms

  APP_GPIO_Init();                            // BTNs LEDs
  LED1_OFF();LED2_OFF();LED3_OFF();

  EEPROM_Init();
  //ModBUS_Init();
  // LCD1602_Init();

  // 1Wire
  OneWire_Init();
  release_1wire();
  DS18B20_Init();


  // LOGGING
  // SD_Init();

  // TODO: Добавить обработчик - если конфиг пустой/невалидный используем значения по умолчанию
  //ReadConfiguration();    // Чтение конфигурации из EEPROM

  // NOTE: Место для отладки модулей по отдельности
  eeprom_save_sequence(config_seq, sizeof(config_seq));
  ReadConfiguration();    // Чтение конфигурации из EEPROM
  // eeprom_load_sequence(config_seq, sizeof(config_seq));

  while (1) {
    UpdateTemperature(&cur_temp);    // Считываем текущую температуру по таймеру раз в 3ms

    SetMode(cur_temp);                // Обновляем состояние термостата

    // Logging();
    // RenderLED(t_state);
    // RenderDisplay(cur_temp);
  }
}

void SysTick_Handler(void) {
  //TODO: Отсчет времени работы с момента включения
  timer_counter();
}

// TODO: Обработчик прерывания по кнопке

// TODO: Обработчик прерывания по приему ModBUS
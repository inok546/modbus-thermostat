#include "main.h"

float cur_temp;

// Определение глобальной конфигурации
volatile thermostat_state t_state = IDLE;
volatile thermo_settings_t t_settings = {
    10,    // 5 сек. принудительного нагрева
    10,    // 5 сек. принудительного охлаждения
    2,     // 2 сек. гистерезис отк. нагрева
    2,     // 2 сек. гистерезис отк. охлаждения
    1,     // 2 сек. гистерезис вкл. нагрева
    1,     // 2 сек. гистерезис вкл. охлаждения
    23,    // нижний порог
    27     // верхний порог
};
//uint8_t config_seq[EEPROM_CONFIG_SEQ_LEN] = {0x0A, 0x0A, 0x04, 0x04, 0x02, 0x02, 0x18, 0x1B}; // конфигурация по умолчанию

int main(void) {
  RCC_Init();
  SystemCoreClockUpdate();                    // Обновление SystemCoreClock
  Delay_Init();                               // Запуск DWT для отчета микросекунд
  SysTick_Config(SystemCoreClock / 1000U);    // 1ms

  APP_GPIO_Init();                            // BTNs LEDs
  LED1_OFF();LED2_OFF();LED3_OFF();

  EEPROM_Init();
  //ModBUS_Init();
  LCD1602_Init(&t_settings, &t_state);

  // 1Wire
  OneWire_Init();
  release_1wire();
  DS18B20_Init();

  // LOGGING
  // SD_Init();

  // TODO: Добавить обработчик - если конфиг пустой/невалидный используем значения по умолчанию
  Thermostat_Init(&t_settings, &t_state);    // Чтение конфигурации из EEPROM 

  // NOTE: Место для отладки модулей по отдельности
  //eeprom_save_sequence(config_seq, sizeof(config_seq));
  //ReadConfiguration();    // Чтение конфигурации из EEPROM
  // eeprom_load_sequence(config_seq, sizeof(config_seq));

  while (1) {
    UpdateTemperature(&cur_temp);    // Считываем текущую температуру по таймеру раз в 3ms

    SetMode(cur_temp);                // Обновляем состояние термостата

    // Logging();
    RenderLED();   //BUG: При НАГРЕВЕ горит только LED3
    RenderDisplay(cur_temp);   // BUG: Выводиться весь массив. Если как-то из элементов пустой (обычно последний) то отображается на дисплее некореектно
  }
}

void SysTick_Handler(void) {
  //TODO: Отсчет времени работы с момента включения
  timer_counter();
}

// TODO: Обработчик прерывания по кнопке

// TODO: Обработчик прерывания по приему ModBUS
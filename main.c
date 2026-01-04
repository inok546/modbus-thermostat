#include "main.h"

float cur_temp;

// Определение глобальной конфигурации
volatile uint8_t override_state_flag = 0;
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

    if(override_state_flag)          // Проверка на принудительный режим термостата
      ForceSetMode(&override_state_flag);               // Принудительно устанавливаем режим термостата
    else
      SetMode(cur_temp);             // Устанавливаем режим термостата в соответсвии с температурой

    // Logging();
    RenderLED();   //BUG: При НАГРЕВЕ горит только LED3
    RenderDisplay(cur_temp);   // BUG: Выводится весь массив. Если как-то из элементов пустой (обычно последний) то отображается на дисплее некореектно
  }
}

void SysTick_Handler(void) {
  //TODO: Отсчет времени работы с момента включения
  timer_counter();
}

// Обработчик прерывания по кнопкам
void EXTI15_10_IRQHandler(void) {
  // Проверяем по какому прерыванию был вызов
  uint32_t regval = EXTI->PR & (EXTI_PR_PR10 | EXTI_PR_PR12);

  switch (regval) {
    // BTN1
    case EXTI_PR_PR10:
      t_state = COOLING;
      break;
    // BTN3
    case EXTI_PR_PR12:
      t_state = HEATING;
      break;
  }

  override_state_flag = 1;

  EXTI->PR = EXTI_PR_PR10 | EXTI_PR_PR12;    // Сброс запроса прерывания
}

// TODO: Обработчик прерывания по приему ModBUS
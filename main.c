#include "main.h"

float cur_temp;

// Определение глобальной конфигурации
volatile thermostat_log_data t_data;       // для записи на карту и чтения из modbus
volatile uint8_t override_state_flag = 0;
volatile thermostat_state t_state = IDLE;
volatile thermostat_settings_t t_settings = {
    10,    // 5 сек. принудительного нагрева
    10,    // 5 сек. принудительного охлаждения
    2,     // 2 сек. гистерезис отк. нагрева
    2,     // 2 сек. гистерезис отк. охлаждения
    1,     // 2 сек. гистерезис вкл. нагрева
    1,     // 2 сек. гистерезис вкл. охлаждения
    23,    // нижний порог
    27     // верхний порог
};

int main(void) {
  RCC_Init();
  SystemCoreClockUpdate();                    // Обновление SystemCoreClock
  Delay_Init();                               // Запуск DWT для отчета микросекунд
  SysTick_Config(SystemCoreClock / 1000U);    // 1ms

  APP_GPIO_Init();                            // BTNs LEDs
  LED1_OFF();LED2_OFF();LED3_OFF();           // Заранее выключаем все LED

  EEPROM_Init();
  ModBUS_Init(&t_settings, &t_state);

  // 1Wire - DS18B20
  OneWire_Init();
  release_1wire();
  DS18B20_Init();

  // LOGGING
  LCD1602_Init(&t_settings, &t_state, &override_state_flag);
  // SD_Init();

  // TODO: Добавить обработчик - если конфиг пустой/невалидный используем значения по умолчанию
  Thermostat_Init(&t_settings, &t_state);    // Чтение конфигурации из EEPROM 

  __enable_irq();   // Вкл. глобальные прерывания 

  while (1) {
    RequestParsingOperationExec();  //TODO: Подумать над оберткой
    UpdateTemperature(&cur_temp);    // Считываем текущую температуру по таймеру раз в 3ms

    //BUG почему-то в режиме force, может перезаписываться режим
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

// TODO: добавить антидребезг
// Обработчик прерывания по кнопкам
void EXTI15_10_IRQHandler(void) {
  uint32_t regval = EXTI->PR & (EXTI_PR_PR10 | EXTI_PR_PR12);   // Проверяем по какому прерыванию был вызов

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

// Обработчик прерывания по приему ModBUS по USART
void USART6_IRQHandler(void) {
  if (USART6->SR & USART_SR_RXNE)
    ModbusReception();
 
  NVIC_ClearPendingIRQ(USART6_IRQn);
}

// Таймер паузы Modbus RTU (T3.5): определение конца кадра по тишине на линии
void TIM2_IRQHandler(void) {
  TIM2->SR &= ~(TIM_SR_UIF);
  ModbusTimersIRQ();
  NVIC_ClearPendingIRQ(TIM2_IRQn);
}
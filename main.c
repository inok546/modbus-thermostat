#include "main.h"

float cur_temp;

// Определение глобальной конфигурации
thermostat_log_data t_data;       // для записи на карту и чтения из modbus
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
  Logger_Init();

  // TODO: Добавить обработчик - если конфиг пустой/невалидный используем значения по умолчанию
  Thermostat_Init(&t_settings, &t_state);    // Чтение конфигурации из EEPROM 

  __enable_irq();   // Вкл. глобальные прерывания 
  
  uint32_t t0 = systick_ms();   // Таймер для отчета секунд записи лога

  while (1) {
    RequestParsingOperationExec();  //TODO: Подумать над оберткой
    UpdateTemperature(&cur_temp);    // Считываем текущую температуру по таймеру раз в 3ms

    //BUG почему-то в режиме force, может перезаписываться режим
    if(override_state_flag)          // Проверка на принудительный режим термостата
      ForceSetMode(&override_state_flag);               // Принудительно устанавливаем режим термостата
    else
      SetMode(cur_temp);             // Устанавливаем режим термостата в соответсвии с температурой

    RenderLED();                    // Индикация состояния термостата
    RenderDisplay(cur_temp);        // Отрисовка параметров на дисплее 

    UpdateThermostatData(&t_data);  // Постоянно обновляем данные термостата для логов
    
    // Запись текущего состояния термостата в файл лога (раз в секунду)
    if ((uint32_t)(systick_ms() - t0) >= LOGGING_DELAY_MS) {
      t0 += LOGGING_DELAY_MS;      
      Logging(&t_data);
    }

  } // while(1)
}

void SysTick_Handler(void) {
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


void Logging(thermostat_log_data *data){
    char log_str[64];
    thermostatLog2str(log_str, sizeof(log_str), data);    // Перевод структуры в строку
    Logger_WriteLog(log_str);                             // Отправка строки на запись
}

// struct->string
size_t thermostatLog2str(char *out, size_t out_sz, const thermostat_log_data *d){
  return (size_t)snprintf(out, out_sz,
                         "%d\t%.1f\t%s\r\n",
                          d->uptime,
                          d->temperature,
                          thermostat_state_to_str(d->state));
}

// enum->stirng 
static const char* thermostat_state_to_str(thermostat_state s){
  switch (s) {
    case HEATING:  return "HEAT";
    case COOLING:  return "COOL";
    case IDLE:     return "IDLE";
    default:       return "UNK";
  }
}

static void UpdateThermostatData(thermostat_log_data *data){
    data->uptime = systick_ms()/1000;
    data->temperature = cur_temp;
    data->state = t_state;
}
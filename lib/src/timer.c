#include "timer.h"

void TIM1_Init(void) {
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;     // Вкл. тактирование таймер TIM1 на шине APB2 (84мгц)
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;    // Вкл. тактирования порта E

  /* Настройка 4-ого вывода TIM1_CH4 на порте PE4 */
  GPIOE->MODER &= ~(GPIO_MODER_MODE14_0 | GPIO_MODER_MODE14_1);    // Очищаем биты PE14
  GPIOE->MODER |= GPIO_MODER_MODE14_1;                             // Альт. ф-ция для PE14
  GPIOE->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR14_1;                     // режим скорости Highspeed
  GPIOE->AFR[1] |= GPIO_AFRH_AFRH6_0;                              // AF1 для PE14 (6й набор бит в регистре)

  /* Конфигурация TIM1 */
  TIM1->PSC = 83;              // APB2/(83+ 1) => 84/84=1MHz
  TIM1->CR1 |= TIM_CR1_CMS;    // Выравнивание по центру
  TIM1->ARR =
      999;    // Т.к. таймер считает до регистра ARR+1, то чтобы получить работу таймера в 1кГЦ мы установим значние 999
  TIM2->CCR4  = 1;                     // Коэф. заполнения для 4ого канала
  TIM1->CCMR2 = TIM_CCMR2_OC4M;        // PWM Mode 2
  TIM1->CCMR2 &= ~(TIM_CCMR2_CC4S);    // Режим работы 4ого канала TIM1 на выход
  TIM1->CCER |= TIM_CCER_CC4E;         // Вкл. выхода 4ого канала OC4
  TIM1->BDTR |= TIM_BDTR_MOE;          // Вкл. выхода в блоке dead-time

  TIM1->CR1 |= TIM_CR1_CEN;            // Вкл. TIM1

  TIM1->EGR |= TIM_EGR_UG;             // Обновление регистров (событие UEV)
}

void TIM2_Start(uint16_t cycles_number) {
  TIM2->ARR = cycles_number - 1;
  TIM2->CNT = 0x00000000;
  TIM2->SR &= ~(TIM_SR_UIF);  // сброс флага прерывания
  TIM2->CR1 |= TIM_CR1_CEN;
}

void TIM2_InitOnePulseIRQ(void) {
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

  TIM2->PSC = 41;              // TIM2 clock 1 MHz
  TIM2->CR1 |= TIM_CR1_OPM;    // One pulse counting mode
  TIM2->DIER |= TIM_DIER_UIE;
  TIM2->EGR |= TIM_EGR_UG;
  TIM2->SR &= ~TIM_SR_UIF;

  NVIC_EnableIRQ(TIM2_IRQn);    // timer 2 interrupt enable
}
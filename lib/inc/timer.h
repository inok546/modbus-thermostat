#ifndef TIMER_H
#define TIMER_H
  #include "stm32f407xx.h"

  #define TIM2_PSC   83        // APB1/(41+ 1) => 42/42=1MHz
  #define TIM2_ARR   999      // период: 10 kHz / (9+1) = 1 kHz
  
  void TIM1_Init(void);
  void TIM2_Init(void);
  void TIM2_Start(uint16_t cycles_number);


#endif
#ifndef DELAY_H
#define DELAY_H

#include "stm32f407xx.h"

#define AHB1_FREQ_HZ              84000000
#define SYSTICK_TIMER_PERIOD_US   1
#define SYSTICK_TIMER_CONST     (((AHB1_FREQ_HZ * (SYSTICK_TIMER_PERIOD_US)) / 1000000) - 1)

// Function for SysTick_Hanlder() period = 1 us
void timer_counter();

void Delay_Init(void);

void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);
void Delay_sec(uint32_t sec);
uint32_t systick_ms(void);
uint8_t systick_elapsed(uint32_t start, uint32_t delay_ms);


#endif
#include "delay.h" 

//uint32_t delay_us;
volatile uint32_t delay_ms;
volatile uint32_t delay_sec;
static uint32_t cycles_per_us = 0;

/******
Инициализация DWT->CYCCNT для микросекундного счётчика.
Вызывать один раз после настройки тактирования (RCC_Init).
*******/
void Delay_Init(void)
{
    // Разрешаем блок трассировки (там живет DWT)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // Обнуляем и запускаем счётчик циклов
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    // Сколько тактов ядра в одной микросекунде
    cycles_per_us = SystemCoreClock / 1000000U;
}


/*
 * Счётчик миллисекунд и секунд.
 * Период SysTick должен быть 1 мс.
 * Вызывается только из SysTick_Handler().
 */
void timer_counter(void)
{
    static uint16_t ms_counter = 0;

    delay_ms++;  // каждое прерывание – 1 мс

    if (++ms_counter >= 1000U) {
        ms_counter = 0;
        delay_sec++;  // каждую 1000-ю мс добавляем 1 секунду
    }
}

/*
 * Блокирующая задержка по миллисекундам.
 */
void Delay_ms(uint32_t ms)
{
    uint32_t start = delay_ms;
    while ((delay_ms - start) < ms) {
        __NOP();
    }
}

/*
 * Блокирующая задержка по секундам.
 */
void Delay_sec(uint32_t sec)
{
    uint32_t start = delay_sec;
    while ((delay_sec - start) < sec) {
        __NOP();
    }
}

/*
 * Блокирующая задержка по микросекундам.
 * Использует DWT->CYCCNT (такты ядра).
 */
void Delay_us(uint32_t us)
{
    uint32_t ticks = (SystemCoreClock / 1000000U) * us;
    uint32_t start = DWT->CYCCNT;

    while ((DWT->CYCCNT - start) < ticks)
    {
        __NOP();
    }
}
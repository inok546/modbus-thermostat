#ifndef USART_H
#define USART_H
#include "stm32f4xx.h"

#define USART_OK    0
#define USART_ERR   1

// Шина APB2 тактируется на 84MHz
// Формула: Частота шины APB2 / Желаемый baudrate / 16
// Выводим мантису и дробную часть в 16-ричной системе
// В нашем случае: 84MHz / 19200 / 16 = 273.44 => M=273(0x111) F=0.44*16=7(0x07) => 0x1117
// По домашке: 84MHz / 115200 / 16 = 45.572 => M=45(0x2D) F=0.572*16=9(0x09) => 0x2D9
// По домашке: 84MHz / 9600 / 16 = 546.6875 => M=546(0x222) F=0.875*16=14(0x0E) => 0x222E
#define BAUDRATE_115200 0x2D9
#define BAUDRATE_19200 0x1117
#define BAUDRATE_9600 0x222E

#define USART_CR1_8N1 (~(USART_CR1_M | USART_CR1_PCE))

void USART6_Init(void);
void usart6_send(uint8_t data[], uint8_t len);
uint8_t USART6_ReciveByte(uint8_t *rxbyte);

#endif

#include "usart.h"

void USART6_Init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;                         // Вкл. тактирование GPIOC
  RCC->APB2ENR |= RCC_APB2ENR_USART6EN;                        // Вкл. тактирование USART6

  GPIOC->MODER |= GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1;    // Работа вывода PC6-PC7 на альт. ф-цию
  GPIOC->AFR[0] |= (8 << GPIO_AFRL_AFSEL6_Pos) | (8 << GPIO_AFRL_AFSEL7_Pos);    // AF8 для PC6-PC7

  USART6->BRR = BAUDRATE_115200;       // Установка baudrate 9600
  USART1->CR1 |= USART_CR1_TE | USART_CR1_RE; // вкл передачу и прием
  USART6->CR1 &= USART_CR1_8N1;        // Ставим длину посылки 8-бит, без контроля четности
  USART6->CR2 &= ~(USART_CR2_STOP);    // 1 стоповый бит в USART посылке
  USART6->CR1 |= USART_CR1_RXNEIE;    // вкл. прервание по приему
    
  USART6->CR1 |= USART_CR1_UE;    // Включение модуля USART

  NVIC_EnableIRQ(USART6_IRQn);
}

// Отправка строки по USART
void usart6_send(uint8_t data[], uint8_t len){
    for (uint8_t i = 0; i < len; i++) {
        USART6->DR = data[i];                        // пишем байт (8-бит режим)
        while ((USART6->SR & USART_SR_TXE) == 0) { } // ждем, пока DR пуст
    }
    //while ((USART1->SR & USART_SR_TC) == 0) { }      // опционально: дождаться окончания всей передачи
}

uint8_t USART6_ReciveByte(uint8_t *rxbyte){
  uint8_t timer=0;
  while (!(USART6->SR & USART_SR_RXNE)) {
    if(timer<32) timer++;
    else return USART_ERR;
  }
  *rxbyte = USART6->DR;
  return USART_OK;
}



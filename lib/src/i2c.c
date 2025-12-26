#include "i2c.h"

void I2C_Init_EEPROM(void) {
  RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;     // включение тактирования модуля I2C1
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;    // включение тактирования порта GPIOB (PB8 = SCL, PB9 = SDA)

  GPIOB->MODER |= GPIO_MODER_MODE8_1;     // PB8 в режиме альт. функции
  GPIOB->MODER |= GPIO_MODER_MODE9_1;     // PB9 в режиме альт. функции

  GPIOB->AFR[1] |= GPIO_AFRH_AFRH0_2;     // Для PB8 выбрана альт. ф-ция AF4 = I2C1
  GPIOB->AFR[1] |= GPIO_AFRH_AFRH1_2;     // Для PB8 выбрана альт. ф-ция AF4 = I2C1

  GPIOB->OTYPER |= (GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9);     // настройка выходов на open-drain, чтобы slaves могли сгенерировать низкий сигнал на шине I2C


  // Если в МК нет встроенных подтягивающих резистров,
  // то прописываем их здесь (в нашем случае не требуется)

  /*======= настройка модуля I2C1 ========
  По умолчанию I2C работает в подчиненном (slave) режиме. Интерфейс автоматически переключается с подчиненного на
  мастер, после того как он сгенерирует условие START и от ведущего к ведомому переключается, в случае
  арбитражного проигрыша или если на шине происходит генерация STOP-условия, что обеспечивает возможность
  работы с несколькими ведущими устройствами.

    режим работы                 = мастер
    скорость передачи            = 100 кбит/сек
    адресация устройств на шине I2C = 7 битная
    DMA не используется          = Эти биты по умолчанию равны 0
    прерывания не используются   = Эти биты по умолчанию равны 0
    адрес микросхемы памяти на шине I2C = 0x4A = 0b1010_000. Используются старшие 7 бит!
  */

  I2C1->CR2 |= (42 << I2C_CR2_FREQ_Pos);    // 42 т.к. мы настроим Freq_APB1 = 42MHz

  /*=====Настройка генерации клоков на линии SCL======
  I2C работает на частоте 100 кГц - Standard mode
  Thigh = CCR * T_plck1
  Tlow = CCR * T_pclk1
  Tsm = 1/(I2C_freq) = 1/100000 = Thigh + Tlow;
  1/100000 = 2 * CCR * T_pclk1
  CCR = 1 / (2*100000*T_pclk1)
  T_pclk1 = 1 / Freq_APB1;
  Freq_APB1 = 42 MHz
  T_Pclk1 = 1 / 42000000
  CCR = 42000000 / (2*100000) = 210;
  */
  I2C1->CCR |= (210 << I2C_CCR_CCR_Pos);         // 100 КГц
  I2C1->CCR &= ~(I2C_CCR_FS);                    // явный сброс бита FS, работа на частоте 100 кГц (Standard Mode)
  I2C1->TRISE |= (43 << I2C_TRISE_TRISE_Pos);    // значение поля = I2C1_CR2_FREQ + 1 = 42+1 = 43

  I2C1->OAR1 &= ~(I2C_OAR1_ADDMODE);             // сбрасываем бит ADDMODE в 0 для 7-битной адресации

  I2C1->CR1 |= I2C_CR1_PE;                       // I2C1 enabled.
  I2C1->CR1 |= I2C_CR1_ACK;                      // разрешение генерации ACK после приема байтов ТОЛЬКО после PE=1
}

// Генерация START-условия
void I2C_Start_gen(void) {
  I2C1->CR1 |= I2C_CR1_START;
  while ((I2C1->SR1 & I2C_SR1_SB) == 0) {        // Ожидания START-условия на шине I2C
  };    
}

// Отправить адрес устройства I2C
void I2C_TxDeviceADDR(char device_addr, char RW_bit) {
  I2C1->DR = (device_addr + RW_bit);    // Выставляем адрес устройства + флаг записи/чтения
  while ((I2C1->SR1 & I2C_SR1_ADDR) == 0) {
  };                                    // Ждем флаг выставления нашего адреса на шине I2C

  (void)I2C1->SR1;                      // Сбрасываем бит ADDR путем чтения регистров SR1 и SR2
  (void)I2C1->SR2;
}

// Запись данных на устройство по шине I2C 
void I2C_Wrtire(char init_addr, char tx_data[], uint16_t data_len) {
  I2C1->CR1 |= I2C_CR1_ACK;                      // Включить генерацию битов ACK

  while ((I2C1->SR2 & I2C_SR2_BUSY) != 0) {
  };                                             // Проверяем в цикле свободная ли шина I2C

  I2C_Start_gen();                               // Генерация START-условия
  I2C_TxDeviceADDR(I2C_DEV_ADDR, I2C_WR_BIT);    // Отправить адрес устройства I2C

  I2C1->DR = init_addr;                          // Отправляем начальный адрес ячейки памяти
  while((I2C1 -> SR1 & I2C_SR1_TXE) == 0){};     // ждем выстваление ACK от слейва, путем чтения флага I2C_SR1_TXE = 1.

  for (uint16_t i = 0; i < data_len; i++) {
    I2C1->DR = tx_data[i];                          // Отправляем 1 байт данных
    while((I2C1 -> SR1 & I2C_SR1_TXE) == 0){};   // ждем выстваление ACK от слейва, путем чтения флага I2C_SR1_TXE = 1.
  }
  
  I2C1->CR1 |= I2C_CR1_STOP;                     // генерация STOP-условия
}

// Чтение данных из устройства на шине I2C 
void I2C_Read(char init_addr, char rx_data[], uint16_t data_len){
  I2C1->CR1 |= I2C_CR1_ACK;                      // Включить генерацию битов ACK

  while ((I2C1->SR2 & I2C_SR2_BUSY) != 0) {
  };                                             // Проверяем в цикле свободная ли шина I2C

  I2C_Start_gen();                               // Генерация START-условия
  I2C_TxDeviceADDR(I2C_DEV_ADDR, I2C_WR_BIT);    // Отправить адрес устройства I2C с битом ЗАПИСИ

  I2C1->DR = init_addr;                          // Отправляем начальный адрес ячейки памяти
  while((I2C1 -> SR1 & I2C_SR1_TXE) == 0){};     // ждем выстваление ACK от слейва, путем чтения флага I2C_SR1_TXE = 1.
  
  I2C_Start_gen();                               // ПОВТОРНАЯ Генерация START-условия
  I2C_TxDeviceADDR(I2C_DEV_ADDR, I2C_RD_BIT);    // Отправить адрес устройства I2C  с битом ЧТЕНИЯ

  for (uint16_t i = 0; i < data_len-1; i++) {     // Цикл на одну итерацию меньше
    while((I2C1 -> SR1 & I2C_SR1_RXNE) == 0){};   // ждем выстваление ACK от слейва, путем чтения флага I2C_SR1_RXNE = 1.
    rx_data[i] = I2C1->DR;                        // Считываем 1 байт данных
  }

  I2C1->CR1 &= ~(I2C_CR1_ACK);                    // Зануляем бит ACK
  while((I2C1 -> SR1 & I2C_SR1_RXNE) == 0){};     // ждем выстваление ACK от слейва, путем чтения флага I2C_SR1_RXNE = 1.
  rx_data[data_len-1] = I2C1->DR;                 // Считываем последний байт данных


  I2C1->CR1 |= I2C_CR1_STOP;                     // генерация STOP-условия

}

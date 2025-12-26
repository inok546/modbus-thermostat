#include "main.h"

void RenderDisplay(float temp){

  // LCD1602
  LCD1602_PinsInit4bits();
  LCD1602_ScreenInit4bits();
  LCD1602_CursorBlink_OFF();

  // Отображение несменяемого текста
  /*
  LCD1602_SetDDRAMAddress(0x00);    // Установка курсора на первую строку
  LCD1602_WriteString4bits(Line1_ADC_Text, sizeof(Line1_ADC_Text));
  LCD1602_SetDDRAMAddress(0x40);    // Установка курсора на вторую строку
  LCD1602_WriteString4bits(Line2_TemperatureText, sizeof(Line2_TemperatureText));

  Convert_Temperature();
  Delay_ms(3); // Задержка для корректной работы 1Wire
  curr_t = DS18B20_ReadTemperature();
  snprintf((char*)curr_temp_string, sizeof(curr_temp_string), "%.1f", curr_t);
    
  // BUG: Выводиться весь массив. Если как-то из элементов пустой (обычно последний) то отображается на дисплее некореектно
  LCD1602_SetDDRAMAddress(0x49);    // Установка курсора на место значения температуры
  LCD1602_WriteString4bits(curr_temp_string, sizeof(curr_temp_string));
  */
}
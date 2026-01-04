#include "main.h"

static volatile thermo_settings_t *settings;
static volatile thermostat_state *state;

// Несменяемый текст
uint8_t Line1_CurrentTemperatureText[LCD_CHAR_NUM_MAX - 11] = "Temp:";
uint8_t Line1_ThermostatStatus_IDLE[LCD_CHAR_NUM_MAX - 12]  = "IDLE";
uint8_t Line1_ThermostatStatus_HEAT[LCD_CHAR_NUM_MAX - 12]  = "HEAT";
uint8_t Line1_ThermostatStatus_COOL[LCD_CHAR_NUM_MAX - 12]  = "COOL";
uint8_t Line2_TempHigh[LCD_CHAR_NUM_MAX - 12]               = "T_h:";
uint8_t Line2_TempLow[LCD_CHAR_NUM_MAX - 12]                = "T_l:";

// Массив для преобразования температуры в строку
uint8_t temerature_string[6];


void RenderDisplay(float t) {
  // переменные для сокращения обновления экрана
  static float last_t;
  static uint8_t last_T_low;
  static uint8_t last_T_high;
  static thermostat_state last_state;

  // Вывод текущей температуры
  if(last_t != t){
    snprintf((char*)temerature_string, sizeof(temerature_string), "%.1f", t);
    LCD1602_SetDDRAMAddress(0x05);    
    LCD1602_WriteString4bits(temerature_string, sizeof(temerature_string));
  }

  //Вывод нижнего порога
  if(last_T_low != settings->t_low){
    snprintf((char*)temerature_string, sizeof(temerature_string), "%d", settings->t_low);
    LCD1602_SetDDRAMAddress(0x44);    
    LCD1602_WriteString4bits(temerature_string, strlen(temerature_string));
  }

  //Вывод верхнего порога
  if(last_T_high != settings->t_high){
    snprintf((char*)temerature_string, sizeof(temerature_string), "%d", settings->t_high);
    LCD1602_SetDDRAMAddress(0x4E);    
    LCD1602_WriteString4bits(temerature_string, strlen(temerature_string));
  }
  
  //Вывод состояние термостата
  if(last_state != *state){
    LCD1602_SetDDRAMAddress(0x0C);
    switch (*state) {
    case IDLE:
      LCD1602_WriteString4bits(Line1_ThermostatStatus_IDLE, sizeof(Line1_ThermostatStatus_IDLE));
      break;
    case HEATING:
      LCD1602_WriteString4bits(Line1_ThermostatStatus_HEAT, sizeof(Line1_ThermostatStatus_HEAT));
      break;
    case COOLING:
      LCD1602_WriteString4bits(Line1_ThermostatStatus_COOL, sizeof(Line1_ThermostatStatus_COOL));
      break;
    }
  }

  last_t = t;
  last_state = *state;
  last_T_low = settings->t_low;
  last_T_high = settings->t_high;
}

void LCD1602_Init(volatile thermo_settings_t *s, volatile thermostat_state *st) {
  settings = s;
  state = st;

  LCD1602_PinsInit4bits();
  LCD1602_ScreenInit4bits();
  LCD1602_CursorBlink_OFF();

  // Отображение несменяемого текста
  LCD1602_SetDDRAMAddress(0x00);    // Установка курсора на начало первой строки
  LCD1602_WriteString4bits(Line1_CurrentTemperatureText, sizeof(Line1_CurrentTemperatureText));
  LCD1602_SetDDRAMAddress(0x40);    // Установка курсора на начало второй строку
  LCD1602_WriteString4bits(Line2_TempLow, sizeof(Line2_TempLow));
  LCD1602_SetDDRAMAddress(0x4A);    // Установка курсора на начало второй строку
  LCD1602_WriteString4bits(Line2_TempHigh, sizeof(Line2_TempHigh));
  //Отображение IDLE состояния
  LCD1602_SetDDRAMAddress(0x0C);
  LCD1602_WriteString4bits(Line1_ThermostatStatus_IDLE, sizeof(Line1_ThermostatStatus_IDLE));
}

/*
          LED1  LED2  LED3  dec
IDLE       0     1     0     2
HEATING    0     1     1     3
COOLING    1     1     0     5
*/
void RenderLED(void) {
  uint8_t leds = *state + 2;    // Смещаем биты на, чтобы получить значения с 2 по 4 (включая)
  GPIOE->ODR   = ((~(leds & 0x07)) << 13);
}
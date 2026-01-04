#ifndef DS18B20_H
#define DS18B20_H
#include "one_wire.h"
#include "stm32f407xx.h"

#define DS18B20_DELAY_READ 3

uint8_t DS18B20_Init(void);
uint8_t ReadScratchpad(uint8_t scratch_array[]);
uint8_t WriteScratch(uint8_t tx_array[]);
uint8_t Convert_Temperature(void);
float DS18B20_ReadTemperature(void);

#endif
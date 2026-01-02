#include "DS18B20.h"
#include "one_wire.h"

uint8_t family_byte    = 0;
uint8_t ser_number[6]  = {};
uint8_t crc_rx         = 0;
uint8_t scratch_mem[9] = {};

uint8_t DS18B20_Init(void) {
  uint8_t err_1wire = Read_ROM64(&family_byte, ser_number, &crc_rx);
  if (err_1wire == OK_1WIRE) {
    printf("+++ DS18B20 found +++ \n");
    printf("+++ FAMILY_CODE = %X \n", family_byte);
    printf("+++ SERIAL NUMBER = ");

    for (uint8_t i = 0; i < 6; i++) {
      printf("%X ", ser_number[i]);
      printf("\n");
    }
  } else
    printf("--- ERROR: 1-Wire DS18B20 not found \n");

  /************ CONFIG settings for DS18B20 ************/
  scratch_mem[0] = 0x64;    // TH = 0x64 = 100
  scratch_mem[1] = 0x0A;    // TL = 0x0A = 10
  //scratch_mem[2] = 0x3F;    // CONFIG = 0x3F; 10-bit temperature format; R0 = 1, R1 = 0
  scratch_mem[2] = 0x1F;    // CONFIG = 0x; 9-bit temperature format; R0 = 0, R1 = 0

  // ------- config, start temper conversion, read temperature --------
  err_1wire = WriteScratch(scratch_mem);    // write config scratchpad

  return err_1wire;
}

uint8_t ReadScratchpad(uint8_t scratch_array[]) {
  uint8_t err_code                    = 0;
  uint16_t temp                       = 0;
  uint8_t scratch_tmp[ROM64_BYTE_LEN] = {};
  uint8_t crc_calculated              = 0;

  if (!Start_1wire()) {    // 1-wire device found
    WriteByte_1wire(SKIP_ROM);
    Delay_us(100);
    WriteByte_1wire(READ_SCRATCH);
    Delay_us(100);
    for (uint8_t i = 0; i < SCRATCH_BYTE_LEN; i++) {    // read all 9 bytes from scratchpad
      scratch_tmp[i] = ReadByte_1wire();
      Delay_us(100);
    }

    crc_calculated = CRC_Calc(scratch_tmp, (SCRATCH_BYTE_LEN - 1), CRC_POLYNOM);

    if (crc_calculated == scratch_tmp[SCRATCH_BYTE_LEN - 1]) {
      for (uint8_t i = 0; i < SCRATCH_BYTE_LEN; i++)
        scratch_array[i] = scratch_tmp[i];
      return OK_1WIRE;
    } else {
      printf("--- ERROR: Scratch Read CRC mismatch \n");
      return NO_DEVICE_1WIRE;
    }
  } else
    return NO_DEVICE_1WIRE;
}

uint8_t WriteScratch(uint8_t tx_array[]) {    // write only 3 bytes from array [0 1 2] will be writed
  if (!Start_1wire()) {                       // 1-wire device found
    WriteByte_1wire(SKIP_ROM);
    Delay_us(100);
    WriteByte_1wire(WRITE_SCRATCH);
    Delay_us(100);

    for (uint8_t i = 0; i < 3; i++) {    // write only 3 bytes from tx_array
      WriteByte_1wire(tx_array[i]);
      Delay_us(100);
    }

    Start_1wire();    // final reset pulse
    return OK_1WIRE;
  } else {
    return NO_DEVICE_1WIRE;
  }
}

uint8_t Convert_Temperature(void) {
  if (!Start_1wire()) {    // 1-wire device found
    WriteByte_1wire(SKIP_ROM);
    Delay_us(100);
    WriteByte_1wire(CONVERT_T);
    Delay_ms(200);
    return OK_1WIRE;
  } else {
    return NO_DEVICE_1WIRE;
  }
}

float DS18B20_ReadTemperature(void) {
  uint8_t err = ReadScratchpad(scratch_mem);
  if (err != OK_1WIRE) return -99.9f;

  int16_t raw = (int16_t)((scratch_mem[1] << 8) | scratch_mem[0]);
  float t = (float)raw / 16.0f;     // LSB = 1/16 °C, при 10-bit младшие биты будут нулями

  printf("T = %f\r\n", t);
  return t;
}
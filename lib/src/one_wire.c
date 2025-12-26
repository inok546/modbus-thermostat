#include "one_wire.h"

void OneWire_Init(void){
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;    // Вкл. тактирования порта E
  
  //GPIO settings for 1-WIRE PE2 pin
  GPIOE -> MODER |= GPIO_MODER_MODE2_0;   // PE2 output mode
  GPIOE -> OTYPER |= GPIO_OTYPER_OT2;     // PE2 output open-drain
}

uint8_t Start_1wire(void){
  tx_mode_1wire();
  pull_low_1wire();
  Delay_us(500);
  release_1wire();
  rx_mode_1wire();
  Delay_us(100);
  if(check_1wire()) return 1;
  else{
    Delay_us(200);
    return 0;
  }
}

void WriteByte_1wire(uint8_t byte_value){
  tx_mode_1wire();
  for (uint8_t i=0; i<8; i++) {
    uint8_t write_bit_code = (byte_value >> i) & 1;
    pull_low_1wire();
    Delay_us(5);

    if(write_bit_code !=0) release_1wire();

    Delay_us(55);
    release_1wire();
    Delay_us(2);
  }
  rx_mode_1wire();
}

uint8_t ReadByte_1wire(void){
  uint8_t rx_byte = 0;
  for(uint8_t i=0; i<8; i++){
    tx_mode_1wire();
    pull_low_1wire();
    Delay_us(2);

    release_1wire();
    rx_mode_1wire();
    Delay_us(12);

    if(check_1wire()) rx_byte |= (1u << i);
    
    Delay_us(60-14);                      // read bit slot time = 60 us
    tx_mode_1wire();
    release_1wire();
    Delay_us(2);                          // pause between bits = 2 us
  }
  return rx_byte;
}

/*
 * Алгоритм вычисления CRC:
 * ========================
 * Циклически сдвигаем CRC и вычисляем бит shift_in_bit = CRC[7] XOR data_bit_in
 * Если shift_in_bit == 1, то после сдвига выполняем еще (CRC xor POLY)
 * Пока не кончатся биты в последовательности данных
 * data_bit_in — это младший бит в байте.
 * В CRC в младший бит задвигаются байты входных данных начиная с младшего бита.
 */

uint8_t CRC_Calc(uint8_t mass[], uint8_t mass_size, uint8_t POLY){
    uint8_t crc = 0, crc_out = 0;
    uint8_t in_data;
    uint8_t in_bits;
    for(uint8_t j = 0; j < mass_size; j++){
        in_data = mass[j];
        for(uint8_t i = 0; i < 8; i++){
            if(((crc & 0x80) >> 7) != (in_data & 0x01)){
                crc = crc << 1;
                crc = crc ^ POLY;
            }
            else
                crc = crc << 1;
            in_data = in_data >> 1;
        }
    }

    for(uint8_t i = 0; i < 8; i++){ // разворачиваем CRC биты в правильном порядке
        if(crc & (1 << i)) crc_out |= (1 << (7 - i));
    }

    return crc_out;
}

uint8_t Read_ROM64(uint8_t *family_code, uint8_t *ser_num, uint8_t *crc){
    uint8_t tmp_array[ROM64_BYTE_LEN];
    uint8_t crc_calculated = 0;
    uint8_t err_code = 0;
    if(!Start_1wire()){                     // 1-wire device found
        WriteByte_1wire(READ_ROM);
        Delay_us(100);
        
        *family_code = ReadByte_1wire();
        tmp_array[0] = *family_code;
        for(uint8_t i = 0; i < 6; i++){
            ser_num[i] = ReadByte_1wire();
            tmp_array[i+1] = ser_num[i];
        }

        *crc = ReadByte_1wire();
        tmp_array[7] = *crc;

        printf("=========== \n");
        printf("==== READ ROM 64 bits ..... \n");
        printf("==== SCRATCH = ");
        for(uint8_t i = 0; i < ROM64_BYTE_LEN; i++){
            printf("0x%X ", tmp_array[i]);
        }

        printf("\n==== CRC Rx = 0x%X \n", tmp_array[7]);

        crc_calculated = CRC_Calc(tmp_array, 7, CRC_POLYNOM);
        printf("==== CRC calculated = 0x%X \n", crc_calculated);

        if(crc_calculated == tmp_array[7]) return OK_1WIRE;
        else return CRC_ERR_1WIRE;          // error ROM64 read
    }
    else{
        return NO_DEVICE_1WIRE;             // error. 1-wire device are not found
    }
}

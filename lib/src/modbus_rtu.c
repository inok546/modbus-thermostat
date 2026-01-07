#include "modbus_rtu.h"

volatile uint8_t timer15_state = MB_TIM_IDLE;
volatile uint8_t timer35_state = MB_TIM_IDLE;
volatile uint8_t ModbusRxState = MB_RX_IDLE;

volatile uint8_t ModbusRxArray[256];    // global array for modbus request reception in USART interrupt
volatile uint8_t RxByteNum;

static volatile thermostat_settings_t *settings;
static volatile thermostat_state *state;
static volatile thermostat_log_data *data;
volatile uint8_t g_settings_dirty = 0;           // Флаг обновления настроек

void ModBUS_Init(volatile thermostat_settings_t *s, volatile thermostat_state *st){
  settings = s;
  state = st;

  USART6_Init();
  TIM2_InitOnePulseIRQ();

  ModbusTimerStart(DELAY_3_5_BYTE_US);
}

void ModbusTimersIRQ(void) {
  if (timer35_state == MB_TIM_STARTED) {        // wait for 3.5 byte silent on the modbus bud
    timer35_state = MB_TIM_DONE;
    timer15_state = MB_TIM_IDLE;
    RxByteNum     = 0;                          // clear reception byte number
  } else {
    if (timer35_state == MB_TIM_DONE) {
      if (timer15_state == MB_TIM_STARTED) {    // end of Modbus request reception
        ModbusTimerStart(DELAY_3_5_BYTE_US);    // start timer45 for 3.5 bytes silent on modbus bus
        timer15_state = MB_TIM_DONE;
        ModbusRxState = MB_RX_DONE;
      } else {
        timer35_state = MB_TIM_IDLE;
        timer15_state = MB_TIM_IDLE;
      }
    } else {
      timer35_state = MB_TIM_IDLE;
      timer15_state = MB_TIM_IDLE;
    }
  }
}

void ModbusReception(void) {
  uint8_t RxByte = USART6->DR;

  if (timer35_state == MB_TIM_STARTED) {
    ModbusTimerStart(DELAY_3_5_BYTE_US);          // restart timer45
  } else {
    if (timer35_state == MB_TIM_DONE) {           // если паузу на шине выждали и пришел байт по USART6

      if ((timer15_state == MB_TIM_IDLE) ||
          (timer15_state == MB_TIM_STARTED)) {    // if timer15 not started or not done receive data byte

        if (RxByteNum < 255) {
          ModbusRxArray[RxByteNum] = RxByte;      // read USART6 DR into ModbusRxArray[]
          RxByteNum++;
          ModbusTimerStart(DELAY_1_5_BYTE_US);    // restart timer15
          ModbusRxState = MB_RX_STARTED;
        } else {                                  // игнорируем слишком длинные пакеты.
          timer15_state = MB_TIM_IDLE;
          ModbusTimerStart(DELAY_3_5_BYTE_US);    // start timer35 for bus pause wait
          ModbusRxState = MB_RX_IDLE;
        }
      } else {                                    // reception not started. Wait for bus pause.
        timer15_state = MB_TIM_IDLE;
        ModbusTimerStart(DELAY_3_5_BYTE_US);      // start timer35 for bus pause wait
        ModbusRxState = MB_RX_IDLE;
      }
    } else {
      ModbusTimerStart(DELAY_3_5_BYTE_US);    // start timer35 for bus pause wait
    }
  }
}

void ModbusTimerStart(uint16_t timer_cycles) {
  TIM2_Start(timer_cycles);
  if (timer_cycles == DELAY_1_5_BYTE_US)
    timer15_state = MB_TIM_STARTED;
  else {
    if (timer_cycles == DELAY_3_5_BYTE_US)
      timer35_state = MB_TIM_STARTED;
  }
}

uint8_t GetOperationCode(uint8_t rx_request[], uint8_t *op_code_out) {
  uint8_t op_code_rx = rx_request[1];

  *op_code_out = op_code_rx;

  if ((op_code_rx == READ_HOLDING_REGISTERS) || 
      (op_code_rx == READ_INPUT_REGISTERS) || 
      (op_code_rx == WRITE_SINGLE_REGISTER) ||
      (op_code_rx == WRITE_MULTI_REGISTERS)) {
    return MODBUS_OK;
  } else {
    return ERROR_OP_CODE;
  }
}

uint8_t CheckDataAddress(uint8_t op_code_in, uint8_t rx_request[]) {
  uint16_t start_addr_rx = (rx_request[2] << 8) + rx_request[3];
  uint16_t quantity_rx   = ((uint16_t)rx_request[4] << 8) | rx_request[5];

  switch (op_code_in) {
  
  case (READ_INPUT_REGISTERS):
    if ((quantity_rx >= 1) && (start_addr_rx < INPUT_REGISTERS_NUM) &&
      ((start_addr_rx + quantity_rx) <= INPUT_REGISTERS_NUM)){
      return MODBUS_OK;  
    }
    return ERROR_DATA_ADDR;
    
  case READ_HOLDING_REGISTERS:
    if ((quantity_rx >= 1) &&
        (start_addr_rx < HOLDING_REGISTERS_NUM) &&
        ((start_addr_rx + quantity_rx) <= HOLDING_REGISTERS_NUM)) {
      return MODBUS_OK;
    }
    return ERROR_DATA_ADDR;
  
  case WRITE_MULTI_REGISTERS:
    // 0x10: start + quantity должны попадать в holding-карту
    if ((quantity_rx >= 1u) &&
        (start_addr_rx < HOLDING_REGISTERS_NUM) &&
        ((start_addr_rx + quantity_rx) <= HOLDING_REGISTERS_NUM)) {
      return MODBUS_OK;
    }
    return ERROR_DATA_ADDR;

  case WRITE_SINGLE_REGISTER:
    // 0x06: после адреса идёт VALUE, а не quantity → проверяем только адрес
    if (start_addr_rx < HOLDING_REGISTERS_NUM) {
      return MODBUS_OK;
    }
    return ERROR_DATA_ADDR;
    

  default:
    return ERROR_DATA_ADDR;
  }
}

/*
дискретный выход с номером 1 адресуется как 0.
*/
uint8_t CheckDataValue(uint8_t op_code_in, uint8_t rx_request[], uint8_t req_len) {
  uint16_t start_addr_rx = ((uint16_t)rx_request[2] << 8) | rx_request[3];
  uint16_t quantity_rx   = (rx_request[4] << 8) + rx_request[5];

  switch (op_code_in) {
  
  case (READ_INPUT_REGISTERS):
    if ((quantity_rx >= 1) && (quantity_rx <= INPUT_REGISTERS_NUM))
      return MODBUS_OK;
    return ERROR_DATA_VAL;

  case (READ_HOLDING_REGISTERS):
    if ((quantity_rx >= 1) && (quantity_rx <= HOLDING_REGISTERS_NUM))
      return MODBUS_OK;
    return ERROR_DATA_VAL;
  
  case (WRITE_MULTI_REGISTERS):
    // Минимум для RTU 0x10: 9 байт (Addr+Func+Start2+Qty2+ByteCount+CRC2)
    if (req_len < 9u) return ERROR_DATA_VAL;

    // ByteCount находится в rx_request[6]
    uint8_t byte_count = rx_request[6];

    // quantity не может быть 0
    if (quantity_rx < 1) return ERROR_DATA_VAL;

    // ByteCount обязан быть quantity*2
    uint16_t expected_byte_count = (uint16_t)(quantity_rx * 2);
    if (byte_count != expected_byte_count) return ERROR_DATA_VAL;

    // Длина кадра обязана быть: 7 (до data включительно) + byte_count + 2 CRC
    uint16_t expected_len = (uint16_t)(7 + (uint16_t)byte_count + 2);
    if ((uint16_t)req_len != expected_len) return ERROR_DATA_VAL;


    return MODBUS_OK;

  //TODO
  case (WRITE_SINGLE_REGISTER):

  default:
    return ERROR_DATA_VAL;
  }
}

// Чтение настроек термостата
uint8_t Exec_READ_HOLDING_REGISTERS(uint16_t start_addr_in,
                                    uint16_t quantity_in,
                                    uint8_t answer_tx[],
                                    uint8_t *answer_len) {
  if (settings == 0)
    return ERROR_DATA_VAL; 



  // Cнапшот значений для согласованности
  uint16_t forced_heat_hs   = settings->forced_heat_hs;
  uint16_t forced_cool_hs   = settings->forced_cool_hs;

  uint16_t heat_off_hyst_x2 = settings->heat_off_hyst_x2;
  uint16_t cool_off_hyst_x2 = settings->cool_off_hyst_x2;
  uint16_t heat_on_hyst_x2  = settings->heat_on_hyst_x2;
  uint16_t cool_on_hyst_x2  = settings->cool_on_hyst_x2;

  int16_t  t_low_x2         = settings->t_low_x2;
  int16_t  t_high_x2        = settings->t_high_x2;

  // Заполняем modbus ответ
  answer_tx[0] = DEVICE_ADDR;
  answer_tx[1] = READ_HOLDING_REGISTERS;          // 0x03
  answer_tx[2] = (uint8_t)(quantity_in * 2u);     // byte count

  for (uint16_t i = 0; i < quantity_in; i++) {
    uint16_t reg_addr = (uint16_t)(start_addr_in + i);
    uint16_t reg_val  = 0;

    switch (reg_addr) {
      case HR_FORCED_HEAT_HS:    reg_val = forced_heat_hs; break;
      case HR_FORCED_COOL_HS:    reg_val = forced_cool_hs; break;

      case HR_HEAT_OFF_HYST_C_X2:  reg_val = heat_off_hyst_x2; break;
      case HR_COOL_OFF_HYST_C_X2:  reg_val = cool_off_hyst_x2; break;
      case HR_HEAT_ON_HYST_C_X2:   reg_val = heat_on_hyst_x2;  break;
      case HR_COOL_ON_HYST_C_X2:   reg_val = cool_on_hyst_x2;  break;

      // int16 передаём как raw 16-bit (два's complement)
      case HR_T_LOW_LIMIT_C_X2:          reg_val = (uint16_t)t_low_x2;  break;
      case HR_T_HIGH_LIMIT_C_X2:         reg_val = (uint16_t)t_high_x2; break;

      default:
        return ERROR_DATA_ADDR;
    }

    // big-endian внутри регистра
    answer_tx[3u + 2u*i] = (uint8_t)(reg_val >> 8);
    answer_tx[4u + 2u*i] = (uint8_t)(reg_val & 0xFFu);
  }

  *answer_len = (uint8_t)(3u + (uint8_t)(quantity_in * 2u)); // answer_len = all listed bytes, without CRC16 bytes

  return MODBUS_OK;
}

// Чтение состояния термостата
uint8_t Exec_READ_INPUT_REGISTERS(uint16_t start_addr_in,
                                  uint16_t quantity_in,
                                  uint8_t answer_tx[],
                                  uint8_t *answer_len) {
    if (data == 0)
      return ERROR_DATA_VAL; 

  // Cнапшот значений для согласованности
  uint32_t uptime = data->uptime;
  int16_t  temp_x2 = temp_to_x2(data->temperature);
  uint16_t state_code = (uint16_t)data->state;

  // Заполняем modbus ответ
  answer_tx[0] = DEVICE_ADDR;                                 // addr
  answer_tx[1] = READ_INPUT_REGISTERS;                        // command
  answer_tx[2] = (uint8_t)(quantity_in * 2);                  // byte count

  // Заполняем  в соответсвии с запрашиваемым количеством регистров
  for (uint16_t i = 0; i < quantity_in; i++) {
      uint16_t reg_addr = (uint16_t)(start_addr_in + i);
      uint16_t reg_val  = 0;

      switch (reg_addr) {
        case IR_UPTIME_HI:      reg_val = (uint16_t)(uptime >> 16); break;
        case IR_UPTIME_LO:      reg_val = (uint16_t)(uptime & 0xFFFF); break;
        case IR_TEMPERATURE_X2: reg_val = (uint16_t)temp_x2; break;       // int16 → raw bits
        case IR_STATE_CODE:     reg_val = state_code; break;

        default:
          return ERROR_DATA_ADDR;
      }

      // big-endian внутри регистра
      answer_tx[3 + 2*i] = (uint8_t)(reg_val >> 8);
      answer_tx[4 + 2*i] = (uint8_t)(reg_val & 0xFF);
  }

  *answer_len = (uint8_t)(3u + (uint8_t)(quantity_in * 2));                   // answer_len = all listed bytes, without CRC16 bytes

  return MODBUS_OK;
}


// Запись одного параметра конфигруации
uint8_t Exec_WRITE_SINGLE_REGISTER(uint16_t start_addr_in,
                                   uint16_t quantity_in,
                                   const uint8_t rx_request[],
                                   uint16_t req_len,
                                   uint8_t answer_tx[],
                                   uint8_t *answer_len){                          
  //TODO
  return MODBUS_OK;
}

// Запись пришедшей конфигруации
uint8_t Exec_WRITE_MULTI_REGISTERS(uint16_t start_addr_in,
                                   uint16_t quantity_in,
                                   const uint8_t rx_request[],
                                   uint16_t req_len,
                                   uint8_t answer_tx[],
                                   uint8_t *answer_len){
  if (settings == 0) return ERROR_DATA_VAL;
  // Минимальная длина RTU запроса 0x10: Addr(1)+Func(1)+Start(2)+Qty(2)+ByteCount(1)+CRC(2)=9
  if (req_len < 9u) return ERROR_DATA_VAL;

  uint8_t byte_count = rx_request[6];
  uint16_t expected_byte_count = (uint16_t)(quantity_in * 2u);
  if (byte_count != expected_byte_count) {
    return ERROR_DATA_VAL;
  }

  // Длина кадра: 7 байт заголовка (до data) + byte_count + 2 CRC
  uint16_t expected_len = (uint16_t)(7u + (uint16_t)byte_count + 2u);
  if (req_len != expected_len) {
    return ERROR_DATA_VAL;
  }

  // Копия настроек для “атомарного” применения и проверки t_low < t_high
  thermostat_settings_t tmp = *settings;

  const uint8_t *data = &rx_request[7];

  for (uint16_t i = 0; i < quantity_in; i++) {
    uint16_t reg_addr = (uint16_t)(start_addr_in + i);
    uint16_t raw_u16  = MB_ReadU16BE(&data[2u*i]);

    // проверка диапазона значения конкретного регистра
    uint8_t v = ValidateHoldingValue(reg_addr, raw_u16);
    if (v != MODBUS_OK) return v;

    // применяем
    switch (reg_addr) {
      case HR_FORCED_HEAT_HS:    tmp.forced_heat_hs   = raw_u16; break;
      case HR_FORCED_COOL_HS:    tmp.forced_cool_hs   = raw_u16; break;

      case HR_HEAT_OFF_HYST_C_X2:  tmp.heat_off_hyst_x2 = raw_u16; break;
      case HR_COOL_OFF_HYST_C_X2:  tmp.cool_off_hyst_x2 = raw_u16; break;
      case HR_HEAT_ON_HYST_C_X2:   tmp.heat_on_hyst_x2  = raw_u16; break;
      case HR_COOL_ON_HYST_C_X2:   tmp.cool_on_hyst_x2  = raw_u16; break;

      case HR_T_LOW_LIMIT_C_X2:          tmp.t_low_x2  = (int16_t)raw_u16; break;
      case HR_T_HIGH_LIMIT_C_X2:         tmp.t_high_x2 = (int16_t)raw_u16; break;

      default:
        return ERROR_DATA_ADDR;
    }
  }

  // Сквозная проверка после применения всех полей
  if (tmp.t_low_x2 >= tmp.t_high_x2) {
    return ERROR_DATA_VAL;
  }

  // Коммит
  *settings = tmp;
  g_settings_dirty = 1u; // дальше в main можно записать EEPROM “в фоне”

  // Ответ на 0x10: Addr, Func, StartHi, StartLo, QtyHi, QtyLo (+CRC добавит AnswerTransmit)
  answer_tx[0] = DEVICE_ADDR;
  answer_tx[1] = WRITE_MULTI_REGISTERS;
  answer_tx[2] = rx_request[2];
  answer_tx[3] = rx_request[3];
  answer_tx[4] = rx_request[4];
  answer_tx[5] = rx_request[5];

  *answer_len = 6u; // без CRC
  return MODBUS_OK;
}


uint8_t ExecOperation(uint8_t op_code, 
                      uint8_t rx_request[], 
                      uint8_t req_len, 
                      uint8_t tx_answer[], 
                      uint8_t *answer_len) {
  uint16_t start_addr_rx = (rx_request[2] << 8) + rx_request[3];
  uint16_t quantity_rx   = (rx_request[4] << 8) + rx_request[5];
  uint8_t err;
  uint8_t answer_array[256];
  uint8_t array_answer_len = 0;

  switch (op_code) {
  
  case (READ_HOLDING_REGISTERS):
    err = Exec_READ_HOLDING_REGISTERS(start_addr_rx, quantity_rx, answer_array, &array_answer_len);
    break;
  
  case (READ_INPUT_REGISTERS):
    err = Exec_READ_INPUT_REGISTERS(start_addr_rx, quantity_rx, answer_array, &array_answer_len);
    break;
  
  case (WRITE_SINGLE_REGISTER):
    err = Exec_WRITE_SINGLE_REGISTER(start_addr_rx, quantity_rx,
                                     rx_request, req_len,
                                     answer_array, &array_answer_len);
    break;

  case (WRITE_MULTI_REGISTERS):
    err = Exec_WRITE_MULTI_REGISTERS(start_addr_rx, quantity_rx,
                                     rx_request, req_len,
                                     answer_array, &array_answer_len);
    break;
  }

  *answer_len = array_answer_len;
  for (uint8_t i = 0; i < array_answer_len; i++)
    tx_answer[i] = answer_array[i];    // copy answer_array into tx_answer

  return err;
}

// Адрес данных верный?
// Значение данных верное? В нашем диапазоне?
// Выполнение требуемой операции
// вычисление CRC16 для ответного пакета
// формирование ответного пакета
uint8_t RequestParsingOperationExec(void) {
  uint8_t err = MODBUS_OK;
  uint16_t crc;
  uint16_t crc_rx;
  uint8_t op_code_rx;
  uint8_t tx_answer_tmp[256];
  uint8_t answer_len_tmp;

  uint8_t RxArraySafe[256];    // safe array for modbus request reception
  uint8_t RxByteNumSafe;

  if (ModbusRxState == MB_RX_DONE) {
    RxByteNumSafe = RxByteNum;
    for (uint8_t i = 0; i < RxByteNumSafe; i++) {    // save received request into internal array
      RxArraySafe[i] = ModbusRxArray[i];
    }

    // if Device Address match and packet length is not short
    if ((RxArraySafe[0] == DEVICE_ADDR) && (RxByteNumSafe > 4)) {
      crc_rx = (RxArraySafe[((RxByteNumSafe)-1)] << 8) + (RxArraySafe[((RxByteNumSafe)-2)] & 0x00FF);
      // CRC16 compare
      crc = CRC16_Calc(RxArraySafe, ((RxByteNumSafe)-2));

      if (crc == crc_rx) {             // Get OperationCode value
        err = GetOperationCode(RxArraySafe, &op_code_rx);

        if (err == MODBUS_OK) {        // check data address
          err = CheckDataAddress(op_code_rx, RxArraySafe);

          //TODO: Дописать! Заглушки
          if (err == MODBUS_OK) {      // check data value
            err = CheckDataValue(op_code_rx, RxArraySafe, RxByteNumSafe);

            //TODO: Дописать! 
            if (err == MODBUS_OK) {    // operation execution
              err = ExecOperation(op_code_rx, RxArraySafe, RxByteNumSafe, tx_answer_tmp, &answer_len_tmp);
            }
          }
        }
      } else {
        err = ERROR_CRC;
      }

      AnswerTransmit(err, tx_answer_tmp, &answer_len_tmp, op_code_rx);
      return err;
    } else {
      err = ERROR_DEV_ADDR;
    } 
  }    

  return err;
}

// answer array assebmly, CRC16 calculation
uint8_t AnswerTransmit(uint8_t err_code, uint8_t tx_array[], uint8_t *tx_array_len, uint8_t op_code) {
  uint16_t crc_calc;

  if (err_code != MODBUS_OK) {
    tx_array[1]   = op_code + ERR_ANSWER_ADD;
    tx_array[2]   = err_code;
    *tx_array_len = 3;                                  // CRC16 2-bytes will be calculated later
  }

  crc_calc = CRC16_Calc(tx_array, *tx_array_len);       // answer CRC16 calculation

  tx_array[*tx_array_len]     = (crc_calc & 0x00FF);    // CRC16 LSB send first
  tx_array[*tx_array_len + 1] = (crc_calc >> 8);        // CRC16 MSB send last

  usart6_send(tx_array, (*tx_array_len + 2));

  ModbusRxState = MB_RX_IDLE;

  return MODBUS_OK;
}


static uint8_t ValidateHoldingValue(uint16_t reg_addr, uint16_t raw_u16){
  switch (reg_addr) {
    case HR_FORCED_HEAT_HS:
    case HR_FORCED_COOL_HS:
      if (raw_u16 <= 20) return MODBUS_OK;
      return ERROR_DATA_VAL;

    case HR_HEAT_OFF_HYST_C_X2:
    case HR_COOL_OFF_HYST_C_X2:
    case HR_HEAT_ON_HYST_C_X2:
    case HR_COOL_ON_HYST_C_X2:
      if ((raw_u16 >= 2) && (raw_u16 <= 10)) return MODBUS_OK;
      return ERROR_DATA_VAL;

    case HR_T_LOW_LIMIT_C_X2:
    case HR_T_HIGH_LIMIT_C_X2: {
      int16_t v = (int16_t)raw_u16;
      if ((v >= -40) && (v <= 170)) return MODBUS_OK;
      return ERROR_DATA_VAL;
    }

    default:
      return ERROR_DATA_ADDR;
  }
}

// round-half-away-from-zero
static int16_t temp_to_x2(float t_celsius){
  float v = t_celsius * 2.0f;
  v = (v >= 0.0f) ? (v + 0.5f) : (v - 0.5f);

  return (int16_t)v;
}

static inline uint16_t MB_ReadU16BE(const uint8_t *p){
  return ((uint16_t)p[0] << 8) | (uint16_t)p[1];
}
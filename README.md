# Modbus thermostat (STM32F407 + CMSIS)
Embedded thermostat controller based on STM32F407VE using CMSIS, featuring Modbus communication, temperature control with hysteresis, EEPROM configuration storage, SD card logging, and support for heating/cooling control.

# TODO
- Better logger (error handling, optional feature, enum-tags)
- Split Modbus into an independent module
- Store slave address in EEPROM
- CMake build configuration
- Pinout description

# Functionality

## Using modules
- **Modbus RTU (RS-485)**: configuration, reading state and temperature
- **LCD1602**: display current temperature, thermostat state and limits
- **DS18B20 (1-Wire)**: temperature measurement (9-bit, 0.5 °C resolution, 1Hz)
- **AT24C02B I²C EEPROM**: Configuration storage
- **SDIO + FATFS**: create a log file on each boot
- **RTC**: timestamp for log file name

## Thermostat
The thermostat uses separate ON/OFF hysteresis values for heating and cooling. All settings are stored in EEPROM and can be updated via Modbus. Manual override is available via buttons S1 (cooling) and S3 (heating).

| State       | Description                                                                                                                                                                                                                        | Code   |
| ----------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------ |
| **IDLE**    | Temperature is within the allowed range between upper and lower limits. Heating and cooling outputs are disabled.                                                                                                                  | `0x02` |
| **COOLING** | Cooling is enabled when the temperature exceeds the upper temperature limit plus the **cooling ON hysteresis**. Cooling remains active until the temperature falls below the upper limit minus the **cooling OFF hysteresis**.     | `0x03` |
| **HEATING** | Heating is enabled when the temperature falls below the lower temperature limit minus the **heating ON hysteresis**. Heating remains active until the temperature rises above the lower limit plus the **heating OFF hysteresis**. | `0x06` |

### LEDs state indication

| State | LED1 | LED2 | LED3 |
| ----- | ---- | ---- | ---- |
| IDLE  | OFF | ON | OFF |
| HEATING  | OFF | ON | ON |
| COOLING  | ON | ON | OFF |
## Logging
- A log file is created on each boot:
  - `THERMOSTAT-{TIMESTAMP}.log`
- Use a filesystem-safe ISO-8601 timestamp:
  - `YYYY-MM-DDTHH-MM-SSZ` (colons `:` are not allowed in FAT filenames on many systems)
- Log columns (tab-separated):
  - `Uptime (s)`    `Temperature (°C)`    `Thermostat state`


## EEPROM configuration map
The device uses **AT24C02B I²C EEPROM** for non-volatile storage of thermostat configuration parameters.
All configuration data is stored contiguously on a single EEPROM page, starting at address `0x00`.

Settings are loaded from EEPROM during device startup and updated when configuration parameters are changed via the Modbus interface.

### EEPROM memory layout
| EEPROM Address (hex) | Size (bytes) | Parameter                 |
| -------------------- | ------------ | ------------------------- |
| `0x00`               | 1            | Forced Heating Time       |
| `0x01`               | 1            | Forced Cooling Time       |
| `0x02`               | 1            | Heating OFF Hysteresis ×2 |
| `0x03`               | 1            | Cooling OFF Hysteresis ×2 |
| `0x04`               | 1            | Heating ON Hysteresis ×2  |
| `0x05`               | 1            | Cooling ON Hysteresis ×2  |
| `0x06`               | 1            | Low Temperature Limit ×2  |
| `0x07`               | 1            | High Temperature Limit ×2  |


> All temperature-related parameters are transmitted in **value × 2 format**, corresponding to a 0.5 °C resolution.

## Modbus RTU interface description
The device supports Modbus RTU and operates as a slave. Data exchange is performed via the RS-485 (half-duplex) interface

The Modbus interface is intended for:
 - configuration of thermostat parameters;
 - reading current operating state and measured values.
### Communication parameters
| Parameter          | Value        |
| ------------------ | ------------ |
| Protocol           | Modbus RTU   |
| Role               | Slave        |
| Physical interface | RS-485       |
| Baud rate          | 115200 baud  |
| Frame format       | 8N1          |
| Byte order         | Big-Endian   |
| CRC                | CRC16 Modbus |
> **Slave address**: fixed `0xAD` (173)

### Supported Modbus functions

| Code | Function | Description|
| --- | --- | --- |
| 0x03      | Read holding registers| Get current thermostat configuration |
| 0x04      | Read input registers| Get current state and measured temperature |
| ~~0x06~~      | ~~Write single register~~| ~~Set thermostat one parameter of configuration~~  |
| 0x10      | Write multiple registers| Set thermostat configuration |

### Register map
#### Input registers
| Address (IR) | Name         | Type   | Unit | Format | Note                         |
| -----------: | ------------ | ------ | ---- | ------ | ---------------------------- |
|       0x0000 | Uptime high | uint16 | sec  | word   | High word of uptime          |
|       0x0001 | Uptime low  | uint16 | sec  | word   | Low word of uptime           |
|       0x0002 | Temperature x2 | int16  | °C   | x2     | Sensor temperature, 0.5 step |
|       0x0003 | Thermostat state | uint16 | —    | enum   | Current thermostat state code |


> **Uptime (32-bit)** is transmitted as two 16-bit registers: first high word, then low word

#### Holding registers
| Address (HR) | Name                      | Type   | Unit | Format | Range (physical/stored)   | Step |
| ----------- | ------------------------- | ------ | ---- | ------ | --------- | ---- |
| 0x0000      | Forced Heating Time ×2    | uint16 | sec  | x2     | 0...10 / 0...20 | 0.5  |
| 0x0001      | Forced Cooling Time ×2    | uint16 | sec  | x2     | 0...10 / 0...20 | 0.5  |
| 0x0002      | Heat OFF Hysteresis ×2    | uint16 | °C   | x2     | 1...5 / 2...10  | 0.5  |
| 0x0003      | Cool OFF Hysteresis ×2    | uint16 | °C   | x2     | 1...5 / 2...10  | 0.5  |
| 0x0004      | Heat ON Hysteresis ×2     | uint16 | °C   | x2     | 1...5 / 2...10  | 0.5  |
| 0x0005      | Cool ON Hysteresis ×2     | uint16 | °C   | x2     | 1...5 / 2...10  | 0.5  |
| 0x0006      | Low Temperature Limit ×2  | int16  | °C   | x2     | -55...125 / -110...250 | 0.5  |
| 0x0007      | High Temperature Limit ×2 | int16  | °C   | x2     | -55...125 / -110...250 | 0.5  |


> All temperature-related parameters are transmitted in **value × 2 format**, corresponding to a 0.5 °C resolution.

### Request examples
#### Reading current temperature
Request:
```
AD 04 00 02 00 01 CRC
```

Response:
```
AD 04 02 00 1E CRC
```
(Temperature = 0x001E / 2 = 15.0 °C)

#### Writing full configuration (all parameters at once)

Example values:
- Forced Heating Time = 5.0 s → ×2 = 10 → `0x000A`
- Forced Cooling Time = 9.0 s → ×2 = 18 → `0x0012`
- Heat OFF Hysteresis = 1.0 °C → ×2 = 2 → `0x0002`
- Cool OFF Hysteresis = 1.0 °C → ×2 = 2 → `0x0002`
- Heat ON Hysteresis = 1.0 °C → ×2 = 2 → `0x0002`
- Cool ON Hysteresis = 1.0 °C → ×2 = 2 → `0x0002`
- Low Temperature Limit = 18.0 °C → ×2 = 36 → `0x0024`
- High Temperature Limit = 25.0 °C → ×2 = 50 → `0x0032`

Request (write 8 registers starting at 0x0100):
```
AD 10 01 00 00 08 10
00 0A 00 12 00 02 00 02 00 02 00 02 00 24 00 32
CRC
```

Response:
```
AD 10 01 00 00 08 CRC
```

## Pinout
Project is based on the STM32F407VET board.
### LCD1602
| LCD1602 Signal | MCU Pin (P4) |
|--------------|--------------|
| A0 / RS      | PB7          |
| RW           | PD4          |
| E            | PE1          |
| DB7          | PC13         |
| DB6          | PE6          |
| DB5          | PE5          |
| DB4          | PE4          |


### GPIO peripherals (buttons and LEDs)
#### Buttons (active-low, internal pull-up not used)
| Function | MCU Pin | External circuit |
|---------|---------|------------------|
| SW1     | PE10    | 3.3 kΩ pull-up to +3.3 V, button to DGND |
| SW2     | PE11    | 3.3 kΩ pull-up to +3.3 V, button to DGND |
| SW3     | PE12    | 3.3 kΩ pull-up to +3.3 V, button to DGND |

> Buttons are **active-low**: pressed = `0`, released = `1`.

#### LEDs (active-low, current limited)
| LED | MCU Pin | External circuit |
|-----|---------|------------------|
| LED1 | PE13   | 3.3 kΩ to +3.3 V, LED to MCU pin |
| LED2 | PE14   | 3.3 kΩ to +3.3 V, LED to MCU pin |
| LED3 | PE15   | 3.3 kΩ to +3.3 V, LED to MCU pin |

> LEDs are **active-low**: write `0` to turn ON, write `1` to turn OFF.

### EEPROM (AT24C02B)
| AT24C02B Pin | Signal | Connection |
|------------:|--------|------------|
| 1           | A0     | DGND (tied low) |
| 2           | A1     | DGND (tied low) |
| 3           | A2     | DGND (tied low) |
| 4           | GND    | DGND |
| 5           | SDA    | PB9 (I2C1_SDA), pulled up to +3.3V with 4.7 kΩ |
| 6           | SCL    | PB8 (I2C1_SCL), pulled up to +3.3V with 4.7 kΩ |
| 7           | WP     | DGND (write enabled) |
| 8           | VCC    | +3.3V (0.1 µF decoupling capacitor to DGND) |


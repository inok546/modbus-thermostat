#ifndef RTC_H
#define RTC_H

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdio.h>

typedef struct {
  uint16_t year;   // 2000..2099
  uint8_t  month;  // 1..12
  uint8_t  day;    // 1..31
  uint8_t  hour;   // 0..23
  uint8_t  min;    // 0..59
  uint8_t  sec;    // 0..59
} rtc_datetime_t;

static inline uint8_t bcd2bin(uint8_t bcd) { return (uint8_t)(((bcd >> 4) * 10U) + (bcd & 0x0FU)); }
static inline uint8_t bin2bcd(uint8_t bin) { return (uint8_t)(((bin / 10U) << 4) | (bin % 10U)); }


#ifndef RCC_BDCR_RTCSEL_LSE
  #define RCC_BDCR_RTCSEL_LSE (RCC_BDCR_RTCSEL_0)
  #define RCC_BDCR_RTCSEL_LSI (RCC_BDCR_RTCSEL_1)
  #define RCC_BDCR_RTCSEL_HSE (RCC_BDCR_RTCSEL_0 | RCC_BDCR_RTCSEL_1)
#endif

static int64_t days_from_civil(int y, unsigned m, unsigned d);
static uint8_t iso_weekday_from_days_since_1970(int64_t days);
static void rtc_write_protect_disable(void);
static void rtc_write_protect_enable(void);
static inline void rtc_wait_sync(void);
static inline uint8_t rtc_time_is_set(void);


void RTC_Init_LSE_1Hz(void);
void RTC_GetDateTime(rtc_datetime_t *out);
void RTC_SetDateTime(const rtc_datetime_t *dt);
int64_t RTC_ToUnixTimestamp(const rtc_datetime_t *dt);
void RTC_ToISO8601_Z(const rtc_datetime_t *dt, char *buf, size_t buf_sz);

#endif
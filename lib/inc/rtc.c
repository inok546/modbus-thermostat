#include "rtc.h"

#define RTC_BKP_MAGIC 0xA5A55A5AU

void RTC_Init_LSE_1Hz(void) {
  // Доступ к backup domain
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  PWR->CR |= PWR_CR_DBP;

  // Запускаем LSE (если ещё не запущен)
  if ((RCC->BDCR & RCC_BDCR_LSERDY) == 0) {
    RCC->BDCR |= RCC_BDCR_LSEON;
    while ((RCC->BDCR & RCC_BDCR_LSERDY) == 0) {}
  }

  // Включаем RTC clock. RTCSEL трогаем только если он уже LSE или вообще ещё не выбран.
  // ВАЖНО: если тут был выбран НЕ LSE — “без потерь” переехать нельзя (нужен BDRST).
  if ((RCC->BDCR & RCC_BDCR_RTCSEL) != RCC_BDCR_RTCSEL_LSE) {
    // Если время уже было установлено, лучше НЕ ломать его автоматом.
    // В таком случае просто оставим как есть (RTC может не работать как ожидается).
    if (!rtc_time_is_set()) {
      RCC->BDCR &= ~RCC_BDCR_RTCSEL;
      RCC->BDCR |= RCC_BDCR_RTCSEL_LSE;
    }
  }

  RCC->BDCR |= RCC_BDCR_RTCEN;

  // Если время уже выставляли — больше ничего не настраиваем
  if (rtc_time_is_set()) {
    rtc_wait_sync();
    return;
  }

  // Первый запуск: настраиваем только прескалеры на 1 Гц (TR/DR НЕ трогаем)
  rtc_write_protect_disable();

  RTC->ISR |= RTC_ISR_INIT;
  while ((RTC->ISR & RTC_ISR_INITF) == 0) {}

  RTC->CR &= ~RTC_CR_FMT;              // 24h
  RTC->PRER = (127U << 16) | 255U;     // 32768 -> 1 Hz

  RTC->ISR &= ~RTC_ISR_INIT;

  rtc_write_protect_enable();
  rtc_wait_sync();
}

// Чтение “стабильно”: TR/DR могут смениться на границе секунды — читаем до совпадения.
void RTC_GetDateTime(rtc_datetime_t *out) {
  uint32_t tr1, dr1, tr2, dr2;

  do {
    tr1 = RTC->TR;
    dr1 = RTC->DR;
    tr2 = RTC->TR;
    dr2 = RTC->DR;
  } while ((tr1 != tr2) || (dr1 != dr2));

  const uint8_t sec  = bcd2bin((uint8_t)(tr1 & 0x7F));
  const uint8_t min  = bcd2bin((uint8_t)((tr1 >> 8) & 0x7F));
  const uint8_t hour = bcd2bin((uint8_t)((tr1 >> 16) & 0x3F));

  const uint8_t day   = bcd2bin((uint8_t)(dr1 & 0x3F));
  const uint8_t month = bcd2bin((uint8_t)((dr1 >> 8) & 0x1F));
  const uint8_t year  = bcd2bin((uint8_t)((dr1 >> 16) & 0xFF));

  out->year  = (uint16_t)(2000U + year);
  out->month = month;
  out->day   = day;
  out->hour  = hour;
  out->min   = min;
  out->sec   = sec;
}

void RTC_SetDateTime(const rtc_datetime_t *dt) {
  // Предполагаем dt->year 2000..2099
  const uint8_t yy = (uint8_t)(dt->year - 2000U);

  const int64_t days = days_from_civil((int)dt->year, dt->month, dt->day);
  const uint8_t wday = iso_weekday_from_days_since_1970(days);    // 1..7

  const uint32_t tr = ((uint32_t)(bin2bcd(dt->hour) & 0x3F) << 16) | ((uint32_t)(bin2bcd(dt->min) & 0x7F) << 8) |
      ((uint32_t)(bin2bcd(dt->sec) & 0x7F) << 0);

  const uint32_t dr = ((uint32_t)(bin2bcd(yy) & 0xFF) << 16) | ((uint32_t)(wday & 0x07) << 13) |
      ((uint32_t)(bin2bcd(dt->month) & 0x1F) << 8) | ((uint32_t)(bin2bcd(dt->day) & 0x3F) << 0);

  rtc_write_protect_disable();

  RTC->ISR |= RTC_ISR_INIT;
  while ((RTC->ISR & RTC_ISR_INITF) == 0) {}

  RTC->TR = tr;
  RTC->DR = dr;

  RTC->ISR &= ~RTC_ISR_INIT;

  // МЕТКА: время установлено
  RTC->BKP0R = RTC_BKP_MAGIC;

  rtc_write_protect_enable();
  rtc_wait_sync();
}

int64_t RTC_ToUnixTimestamp(const rtc_datetime_t *dt) {
  const int64_t days = days_from_civil((int)dt->year, dt->month, dt->day);
  return days * 86400LL + (int64_t)dt->hour * 3600LL + (int64_t)dt->min * 60LL + (int64_t)dt->sec;
}

void RTC_ToISO8601_Z(const rtc_datetime_t *dt, char *buf, size_t buf_sz) {
  // UTC-вариант: ...Z
  (void)snprintf(buf,
                 buf_sz,
                 "%04u-%02u-%02uT%02u:%02u:%02uZ",
                 (unsigned)dt->year,
                 (unsigned)dt->month,
                 (unsigned)dt->day,
                 (unsigned)dt->hour,
                 (unsigned)dt->min,
                 (unsigned)dt->sec);
}

// Алгоритм Howard Hinnant: дни с 1970-01-01 (UTC)
static int64_t days_from_civil(int y, unsigned m, unsigned d) {
  y -= (m <= 2);
  const int era      = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = (unsigned)(y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return (int64_t)era * 146097 + (int64_t)doe - 719468;    // 719468 = offset to 1970-01-01
}

static uint8_t iso_weekday_from_days_since_1970(int64_t days) {
  // ISO: Monday=1..Sunday=7. 1970-01-01 was Thursday (4).
  int w = (int)((days + 3) % 7);
  if (w < 0)
    w += 7;
  return (uint8_t)(w + 1);
}

static void rtc_write_protect_disable(void) {
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;
}

static void rtc_write_protect_enable(void) {
  RTC->WPR = 0xFF;
}

// Синхронизация shadow-регистров
static void rtc_wait_sync(void) {
  RTC->ISR &= ~RTC_ISR_RSF;
  while ((RTC->ISR & RTC_ISR_RSF) == 0) {
  }
}

// true, если когда-то уже устанавливали время
static inline uint8_t rtc_time_is_set(void) {
  return (RTC->BKP0R == RTC_BKP_MAGIC) ? 1U : 0U;
}


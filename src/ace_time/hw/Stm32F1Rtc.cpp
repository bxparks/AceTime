/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 *
 * Extracted from https://github.com/ZulNs/STM32F1_RTC/
 * Copyright (c) 2019 ZulNs
 */

#include "Stm32F1Rtc.h"

#if defined(STM32F1xx)

namespace ace_time {
namespace hw {

bool Stm32F1Rtc::begin() {
  bool isInit = isInitialized();
  enableClockInterface();
  if (isInit)
    waitSync();
  else
    init();
  return isInit;
}

void Stm32F1Rtc::init() {
  enableBackupWrites();
  RCC_BDCR |= RCC_BDCR_BDRST; // Resets the entire Backup domain
  RCC_BDCR &= ~RCC_BDCR_BDRST; // Deactivates reset of entire Backup domain
  RCC_BDCR |= RCC_BDCR_LSEON; // Enables external low-speed oscillator (LSE)
  while ((RCC_BDCR & RCC_BDCR_LSERDY) == 0); // Waits for LSE ready
  RCC_BDCR |= RCC_BDCR_RTCSEL_LSE; // Selects LSE as RTC clock
  RCC_BDCR |= RCC_BDCR_RTCEN; // Enables the RTC
  waitSync();
  waitFinished();
  enterConfigMode();
  RTC_PRLL = 0x7FFF;
  exitConfigMode();
  waitFinished();
  RTC_INIT_REG |= RTC_INIT_FLAG; // Signals that RTC initilized
  disableBackupWrites();
}

// The 32-bit RTC counter is spread over 2 registers so it cannot be read
// atomically. We need to read the high word twice and check if it has rolled
// over. If it has, then read the low word a second time to get its new, rolled
// over value. See the RTC_ReadTimeCounter() in
// system/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rtc.c.
uint32_t Stm32F1Rtc::getTime() {
  uint16_t high1 = RTC_CNTH;
  uint16_t low = RTC_CNTL;
  uint16_t high2 = RTC_CNTH;

  if (high1 != high2) {
    low = RTC_CNTL;
    high1 = high2;
  }

  return (high1 << 16) | low;
}

void Stm32F1Rtc::setTime(uint32_t time) {
  enableBackupWrites();
  waitFinished();
  enterConfigMode();
  RTC_CNTH = time >> 16;
  RTC_CNTL = time & 0xFFFF;
  exitConfigMode();
  waitFinished();
  disableBackupWrites();
}

} // hw
} // ace_time

#endif // defined(STM32F1xx)

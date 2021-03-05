/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 *
 * Extracted from https://github.com/ZulNs/STM32F1_RTC/
 * Copyright (c) 2019 ZulNs
 */

#ifndef STM32_F1_RTC_H
#define STM32_F1_RTC_H

#if defined(STM32F1xx)

#include <Arduino.h> // RTC, RCC, PWR

// TODO(brian): Replace the following direct access to registers
// with functions/macros in the STM32 HAL.

#define RTC_CRH  RTC->CRH
#define RTC_CRL  RTC->CRL
#define RTC_PRLH RTC->PRLH
#define RTC_PRLL RTC->PRLL
#define RTC_CNTH RTC->CNTH
#define RTC_CNTL RTC->CNTL

#define RCC_APB1ENR RCC->APB1ENR
#define RCC_BDCR    RCC->BDCR
#define PWR_CR      PWR->CR

#define BKP_DR        (((volatile uint32_t*)0x40006C00))
#define RTC_INIT_REG  1
#define RTC_INIT_BIT  0
#define RTC_INIT_FLAG (1 << RTC_INIT_BIT)

#include <stdint.h>

namespace ace_time {
namespace hw {

/**
 * A thin abstraction above the RTC_CNTL and RTC_CNTH registers which are in
 * the "backup domain". When powered by LSE_CLOCK (Low Speed External) the
 * counter continues to count as long as the VBat is powered.
 */
class Stm32F1Rtc {
  public:
    bool begin();

    void setTime(uint32_t time);

    uint32_t getTime() {
      return (RTC_CNTH << 16) | RTC_CNTL;
    }

    bool isInitialized() {
      return (BKP_DR[RTC_INIT_REG] & RTC_INIT_FLAG) == RTC_INIT_FLAG;
    }

  private:
    void init();

    void waitSync() {
      RTC_CRL &= ~RTC_CRL_RSF;
      while ((RTC_CRL & RTC_CRL_RSF) == 0);
    }

    void waitFinished() {
      while ((RTC_CRL & RTC_CRL_RTOFF) == 0);
    }

    void enableBackupWrites() {
      PWR_CR |= PWR_CR_DBP;
    }

    void disableBackupWrites() {
      PWR_CR &= ~PWR_CR_DBP;
    }

    void enterConfigMode() {
      RTC_CRL |= RTC_CRL_CNF;
    }

    void exitConfigMode() {
      RTC_CRL &= ~RTC_CRL_CNF;
    }

    void enableClockInterface() {
      RCC_APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
    }
};

} // hw
} // ace_time

#endif // defined(STM32F1xx)

#endif

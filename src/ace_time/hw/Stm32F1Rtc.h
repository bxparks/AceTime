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

#include <Arduino.h> // RTC, RCC, PWR, BKP

#define RTC_CRH  RTC->CRH
#define RTC_CRL  RTC->CRL
#define RTC_PRLH RTC->PRLH
#define RTC_PRLL RTC->PRLL
#define RTC_CNTH RTC->CNTH
#define RTC_CNTL RTC->CNTL

#define RCC_APB1ENR RCC->APB1ENR
#define RCC_BDCR    RCC->BDCR
#define PWR_CR      PWR->CR

//
#define RTC_INIT_REG  BKP->DR1
#define RTC_INIT_BIT  0
#define RTC_INIT_FLAG (1 << RTC_INIT_BIT)

#include <stdint.h>

namespace ace_time {
namespace hw {

/**
 * A thin abstraction above the RTC_CNTL and RTC_CNTH registers which are in
 * the "backup domain". When powered by LSE_CLOCK (Low Speed External) the
 * counter continues to count as long as the VBat is powered.
 *
 * The STM32RTC library (https://github.com/stm32duino/STM32RTC) uses SRAM on
 * STM32F1 to hold the date fields, which means that the date fields are not
 * preserved during power loss. This class completely bypasses the HAL (hardware
 * abstarction layer) and writes the 32-bit epochSeconds quantity directly into
 * the `RTC->CNTH` and `RTC->CNTL` registers.
 *
 * The Backup DR2 register is used to hold a single bit, indicating whether or
 * not the RTC has been initialized.
 */
class Stm32F1Rtc {
  public:
    /**
     * Call this to initialize this class at the start of the application. If
     * the underlying RTC requires initialization, then this will call init(),
     * and a flag will be set to indicate that the RTC has been initialized. The
     * flag is preserved through a power cycle if power is supplied to VBat.
     * When this is called again after the processor is reset, the init() will
     * not be called, thereby preserving the current time on the RTC.
     */
    bool begin();

    /** Call this manually to reset the internal RTC counter. */
    void init();

    /** Set the internal 32-bit RTC clock to the given value. */
    void setTime(uint32_t time);

    /** Return the internal 32-bit RTC clock. */
    uint32_t getTime();

    /**
     * Returns true if the internal RTC has been initialized. This information
     * is preserved through a power cycle if backup power is supplied to VBat.
     */
    bool isInitialized() {
      return (RTC_INIT_REG & RTC_INIT_FLAG) == RTC_INIT_FLAG;
    }

  private:
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

/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park, Anatoli Arkhipenko
 */

#if ! defined(UNIX_HOST_DUINO)
#if defined(ARDUINO_ARCH_STM32)

#include <Print.h> // Print
#include <AceCommon.h> // bcdToDec(), decToBcd()
#include "HardwareDateTime.h"
#include "StmRtc.h"

using ace_common::bcdToDec;
using ace_common::decToBcd;

namespace ace_time {

namespace hw {

StmRtc::StmRtc() {
  
}

bool StmRtc::begin(const sourceClock_t clockSource, const hourFormat_t hourFormat) {
  
  mRtc = &STM32RTC::getInstance();
  if ( mRtc ) {
  // Select RTC clock source: LSI_CLOCK, LSE_CLOCK or HSE_CLOCK.
    mRtc->setClockSource((STM32RTC::Source_Clock) clockSource);
    mRtc->begin(hourFormat);
    return true;
  }
  return false;
}


void StmRtc::readDateTime(HardwareDateTime* dateTime) const {

  if ( mRtc && mRtc->isTimeSet() ) {
    
//  Serial.println("StmRtc::readDateTime rtc is set");
    dateTime->second = mRtc->getSeconds(); 
    dateTime->minute = mRtc->getMinutes();
    dateTime->hour = mRtc->getHours();
    dateTime->dayOfWeek = mRtc->getWeekDay();
    dateTime->day = mRtc->getDay();
    dateTime->month = mRtc->getMonth();
    dateTime->year = mRtc->getYear();
  } 
  else {
//  Serial.println("StmRtc::readDateTime rtc is NOT set");
    dateTime->second = 0;
    dateTime->minute = 0;
    dateTime->hour = 0;
    dateTime->dayOfWeek = 0;
    dateTime->day = 1;
    dateTime->month = 1;
    dateTime->year = 0;
  }
}


//  always set in 24h format
void StmRtc::setDateTime(const HardwareDateTime& dateTime) const {

  if ( mRtc ) {
//  Serial.println("STMRTC::setDateTime rtc is set");
    mRtc->setTime(dateTime.hour, dateTime.minute, dateTime.second);
    mRtc->setDate(dateTime.day, dateTime.month, dateTime.year);
  }
}

bool StmRtc::isTimeSet() const {
  if ( mRtc ) {
    return mRtc->isTimeSet();
  }
  return false;
}

}
}
#endif  //  #if ! defined(UNIX_HOST_DUINO)
#endif  //  #if defined(ARDUINO_ARCH_STM32)

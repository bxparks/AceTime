/*
 * MIT License
 * Copyright (c) 2020 Brian T. Park, Anatoli Arkhipenko
 */

#if ! defined(EPOXY_DUINO)
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

  mRtc = &STM32RTC::getInstance();
  if (mRtc) {
    mRtc->setClockSource(STM32RTC::LSI_CLOCK);
    mRtc->begin(HOUR_FORMAT_24);
  }
}


void StmRtc::readDateTime(HardwareDateTime* dateTime) const {

  if (mRtc && mRtc->isTimeSet()) {
    
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

  if (mRtc) {
//  Serial.println("STMRTC::setDateTime rtc is set");
    mRtc->setTime(dateTime.hour, dateTime.minute, dateTime.second);
    mRtc->setDate(
        dateTime.dayOfWeek, dateTime.day, dateTime.month, dateTime.year
    );
  }
}

bool StmRtc::isTimeSet() const {
  if (mRtc) {
    return mRtc->isTimeSet();
  }
  return false;
}

} // hw
} // ace_time

#endif  //  #if ! defined(EPOXY_DUINO)
#endif  //  #if defined(ARDUINO_ARCH_STM32)

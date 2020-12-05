/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park, Anatoli Arkhipenko
 */

#if ! defined(UNIX_HOST_DUINO)
#if defined(ARDUINO_ARCH_STM32)

#include <Print.h> // Print
#include <AceCommon.h> // bcdToDec(), decToBcd()
#include "HardwareDateTime.h"
#include "stmrtc.h"

using ace_common::bcdToDec;
using ace_common::decToBcd;

namespace ace_time {

namespace hw {

STMRTC::STMRTC() {
  rtc = NULL;
  rtc = &STM32RTC::getInstance();
  if ( rtc ) {
    rtc->begin();
    rtc->setClockSource(STM32RTC::LSI_CLOCK);
    // rtc->setClockSource(STM32RTC::LSE_CLOCK);
  }
}


void STMRTC::readDateTime(HardwareDateTime* dateTime) const {

  if ( rtc && rtc->isTimeSet() ) {
    
//  Serial.println("STMRTC::readDateTime rtc is set");
    dateTime->second = rtc->getSeconds(); 
    dateTime->minute = rtc->getMinutes();
    dateTime->hour = rtc->getHours();
    dateTime->dayOfWeek = rtc->getWeekDay();
    dateTime->day = rtc->getDay();
    dateTime->month = rtc->getMonth();
    dateTime->year = rtc->getYear();
  } 
  else {
//  Serial.println("STMRTC::readDateTime rtc is NOT set");
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
void STMRTC::setDateTime(const HardwareDateTime& dateTime) const {

  if ( rtc ) {
//  Serial.println("STMRTC::setDateTime rtc is set");
    rtc->setTime(dateTime.hour, dateTime.minute, dateTime.second);
    //    rtc->setSeconds(dateTime.second);
    //    rtc->setMinutes(dateTime.minute);
    //    rtc->setHours(dateTime.hour);
    rtc->setDate(dateTime.dayOfWeek, dateTime.day, dateTime.month, dateTime.year);
    // rtc->setWeekDay(dateTime.dayOfWeek);
    // rtc->setDay(dateTime.day);
    // rtc->setMonth(dateTime.month);
    // rtc->setYear(dateTime.year);
  }
}

bool STMRTC::isTimeSet() const {
  if ( rtc ) {
    return rtc->isTimeSet();
  }
  return false;
}

}
}
#endif  //  #if ! defined(UNIX_HOST_DUINO)
#endif  //  #if defined(ARDUINO_ARCH_STM32)

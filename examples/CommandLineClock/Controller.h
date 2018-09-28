#ifndef COMMAND_LINE_CLOCK_CONTROLLER_H
#define COMMAND_LINE_CLOCK_CONTROLLER_H

#include <AceTime.h>
#include "config.h"
#include "PersistentStore.h"
#include "StoredInfo.h"

using namespace ace_time;

// Set to 1 to set the TimeProvider::pollNow() method.
class Controller {
  public:
    static const int8_t kDefaultTzCode = -28; // Pacific Daylight Time, -07:00

    Controller(PersistentStore& persistentStore, TimeKeeper& systemTimeKeeper):
        mPersistentStore(persistentStore),
        mSystemTimeKeeper(systemTimeKeeper),
        mTimeZone(0) {}

    void setup() {
      // Retrieve time zone from persistent storage.
      StoredInfo storedInfo;
      bool isValid = mPersistentStore.readStoredInfo(storedInfo);
      mTimeZone = TimeZone(isValid ? storedInfo.tzCode : kDefaultTzCode);
    }

    void preserveInfo() {
      StoredInfo storedInfo;
      // TODO: check isValid return type
      mPersistentStore.readStoredInfo(storedInfo);
      storedInfo.tzCode = mTimeZone.tzCode();
      mPersistentStore.writeStoredInfo(storedInfo);
    }

    void setTimeZone(const TimeZone& timeZone) {
      mTimeZone = timeZone;
      preserveInfo();
    }

    void setDateTime(const DateTime& newDateTime) {
      uint32_t seconds = newDateTime.toSecondsSinceEpoch();
      mSystemTimeKeeper.setNow(seconds);
    }

    DateTime now() const {
      return DateTime(mSystemTimeKeeper.getNow(), mTimeZone);
    }

    TimeZone timeZone() const {
      return mTimeZone;
    }

  private:
    PersistentStore& mPersistentStore;
    TimeKeeper& mSystemTimeKeeper;
    TimeZone mTimeZone;
};

#endif

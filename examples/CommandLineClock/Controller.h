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
        mSystemTimeKeeper(systemTimeKeeper) {}

    void setup() {
      mIsStoredInfoValid = mPersistentStore.readStoredInfo(mStoredInfo);
    }

    /** Set the time zone of the clock and preserve it. */
    void setTimeZone(const TimeZone& timeZone) {
      mStoredInfo.tzCode = timeZone.tzCode();
      preserveInfo();
    }

    /** Return the current time zone. */
    TimeZone getTimeZone() const { return TimeZone(mStoredInfo.tzCode); }

#if defined(USE_NTP)
    /**
     * Set the wifi credentials and setup the NtpTimeProvider.
     * Return the number of bytes written.
     */
    uint16_t setWiFi(const char* ssid, const char* password) {
      strncpy(mStoredInfo.ssid, ssid, StoredInfo::kSsidMaxLength);
      mStoredInfo.ssid[StoredInfo::kSsidMaxLength - 1] = '\0';
      strncpy(mStoredInfo.password, password, StoredInfo::kPasswordMaxLength);
      mStoredInfo.password[StoredInfo::kPasswordMaxLength - 1] = '\0';
      return preserveInfo();
    }
#endif

    /** Set the current time of the system time keeper. */
    void setNow(const DateTime& newDateTime) {
      uint32_t seconds = newDateTime.toSecondsSinceEpoch();
      mSystemTimeKeeper.setNow(seconds);
    }

    /** Return the current time from the system time keeper. */
    DateTime getNow() const {
      return DateTime(mSystemTimeKeeper.getNow(), TimeZone(mStoredInfo.tzCode));
    }

    /** Return true if the initial setup() retrieved a valid storedInfo. */
    bool isStoredInfoValid() const { return mIsStoredInfoValid; }

    /** Return the stored info. */
    const StoredInfo& getStoredInfo() const { return mStoredInfo; }

  private:
    uint16_t preserveInfo() {
      return mPersistentStore.writeStoredInfo(mStoredInfo);
    }

    PersistentStore& mPersistentStore;
    TimeKeeper& mSystemTimeKeeper;

    StoredInfo mStoredInfo;
    bool mIsStoredInfoValid = false;
};

#endif

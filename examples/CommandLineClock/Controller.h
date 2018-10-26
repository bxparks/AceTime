#ifndef COMMAND_LINE_CLOCK_CONTROLLER_H
#define COMMAND_LINE_CLOCK_CONTROLLER_H

#include <AceTime.h>
#include "config.h"
#include "PersistentStore.h"
#include "StoredInfo.h"

using namespace ace_time;
using namespace ace_time::provider;

// Set to 1 to set the TimeProvider::pollNow() method.
class Controller {
  public:
    Controller(PersistentStore& persistentStore, TimeKeeper& systemTimeKeeper):
        mPersistentStore(persistentStore),
        mSystemTimeKeeper(systemTimeKeeper) {}

    void setup() {
      mIsStoredInfoValid = mPersistentStore.readStoredInfo(mStoredInfo);
      // TODO: I think this needs to set the default timezone and
      // set the ssid and password to some initial blank state, so that
      // the user can be notified that they need to be provided.
    }

    /** Set the time zone of the clock and preserve it. */
    void setTimeZone(const TimeZone& timeZone) {
      mStoredInfo.timeZone = timeZone;
      preserveInfo();
    }

    /** Return the current time zone. */
    TimeZone getTimeZone() const { return mStoredInfo.timeZone; }

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
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
      uint32_t seconds = newDateTime.toEpochSeconds();
      mSystemTimeKeeper.setNow(seconds);
    }

    /** Return the current time from the system time keeper. */
    DateTime getNow() const {
      return DateTime::forEpochSeconds(
          mSystemTimeKeeper.getNow(), mStoredInfo.timeZone);
    }

    /** Return true if the initial setup() retrieved a valid storedInfo. */
    bool isStoredInfoValid() const { return mIsStoredInfoValid; }

    /** Return the stored info. */
    const StoredInfo& getStoredInfo() const { return mStoredInfo; }

    /** Return DST mode. */
    bool isDst() const { return mStoredInfo.timeZone.isDst(); }

    /** Set DST on or off */
    void setDst(bool status) {
      mStoredInfo.timeZone.isDst(status);
      preserveInfo();
    }

  private:
    uint16_t preserveInfo() {
      mIsStoredInfoValid = true;
      return mPersistentStore.writeStoredInfo(mStoredInfo);
    }

    PersistentStore& mPersistentStore;
    TimeKeeper& mSystemTimeKeeper;

    StoredInfo mStoredInfo;
    bool mIsStoredInfoValid = false;
};

#endif

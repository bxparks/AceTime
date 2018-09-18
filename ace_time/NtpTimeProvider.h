#ifndef ACE_TIME_NTP_TIME_PROVIDER_H
#define ACE_TIME_NTP_TIME_PROVIDER_H

#if defined(ESP8266)

#include <stdint.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "TimeKeeper.h"
#include "common/logger.h"

#ifndef ACE_TIME_NTP_TIME_PROVIDER_DEBUG
#define ACE_TIME_NTP_TIME_PROVIDER_DEBUG 1
#endif

namespace ace_time {

using common::logger;

/**
 * A TimeProvider that retrieves the time from an NTP server.
 *
 * Borrowed from
 * https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/NTPClient/NTPClient.ino
 * and
 * https://github.com/PaulStoffregen/Time/blob/master/examples/TimeNTP/TimeNTP.ino
 */
class NtpTimeProvider: public TimeProvider {
  public:
    /** Default NTP Server */
    static const char kNtpServerName[];

    /** Default port used for UDP packets. */
    static const uint16_t kLocalPort = 8888;

    /** Request time out milliseconds. */
    static const uint16_t kRequestTimeout = 1500;

    /**
     * Constructor.
     * @param ssid wireless SSID
     * @param password password of the SSID
     * @param server name of the NTP server (default us.pool.ntp.org)
     * @param localPort used by the UDP client (default 8888)
     * @paran requestTimeout milliseconds for a request timesout (default 1000)
     */
    explicit NtpTimeProvider(const char* ssid, const char* password,
            const char* server = kNtpServerName,
            uint16_t localPort = kLocalPort,
            uint16_t requestTimeout = kRequestTimeout):
        mSsid(ssid),
        mPassword(password),
        mServer(server),
        mLocalPort(localPort),
        mRequestTimeout(requestTimeout) {}

    virtual void setup() override {
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      Serial.print("Connecting to ");
      Serial.println(mSsid);
#endif

      WiFi.begin(mSsid, mPassword);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
        Serial.print(".");
#endif
      }

#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      Serial.print("IP number assigned by DHCP is ");
      Serial.println(WiFi.localIP());
#endif

#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      Serial.println("Starting UDP");
#endif
      mUdp.begin(mLocalPort);

#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      Serial.print("Local port: ");
      Serial.println(mUdp.localPort());
#endif
    }

    virtual uint32_t getNow() const override {
      uint32_t secondsSince1900 = getNtpTime();
      return (secondsSince1900 == 0) ? 0
          : secondsSince1900 - kSecondsSinceNtpEpoch;
    }

    virtual bool pollNow(uint8_t& status, uint32_t& seconds) const override {
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      uint16_t methodTime = millis();
#endif

      // send off a request, then return
      if (!mIsRequestPending) {
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
        logger("NtpTimeProvider::pollNow(): Sending NTP request");
#endif
        sendRequest();
        mRequestStartTime = millis();
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
        mRequestPendingCount = 0;
#endif
        mIsRequestPending = true;
        return false;
      }

      // there's request pending, check for time out
      uint16_t waitTime= (uint16_t) millis() - mRequestStartTime;
      if (waitTime > mRequestTimeout) {
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
        logger("NtpTimeProvider::pollNow(): Timed out after %u polls and %u ms",
            mRequestPendingCount, waitTime);
#endif
        status = kStatusTimedOut;
        mIsRequestPending = false;
        return true;
      }

      // keep waiting if the response has not arrived
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      mRequestPendingCount++;
#endif
      if (mUdp.parsePacket() < kNtpPacketSize) return false;

#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      waitTime = (uint16_t) millis() - mRequestStartTime;
      logger("NtpTimeProvider::pollNow(): Received after %u polls and %u ms",
          mRequestPendingCount, waitTime);
#endif

      // process the response
      uint32_t secondsSince1900 = readResponse();
      seconds = (secondsSince1900 == 0) ? 0
          : secondsSince1900 - kSecondsSinceNtpEpoch;
      mIsRequestPending = false;
      status = kStatusOk;

#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      logger("NtpTimeProvider::pollNow(): Returning response: %u ms",
          (uint16_t) ((uint16_t) millis() - mRequestStartTime));
      logger("NtpTimeProvider::pollNow(): method duration: %u ms",
          (uint16_t) ((uint16_t) millis() - methodTime));
#endif
      return true;
    }

  private:
    /** NTP time is in the first 48 bytes of message. */
    static const uint8_t kNtpPacketSize = 48;

    /**
     * Number of seconds between NTP epoch (1900-01-01T00:00:00Z) and
     * AceTime epoch (2000-01-01T00:00:00Z).
     */
    static const uint32_t kSecondsSinceNtpEpoch = 3155673600;

    void sendRequest() const {
      uint16_t methodStart = millis();

      // discard any previously received packets
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      uint16_t startTime = millis();
      uint16_t packets = 0;
#endif
      while (mUdp.parsePacket() > 0) {
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
        packets++;
#endif
      }
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      logger("NtpTimeProvider::sendRequest(): Discarded %u packets: %hu ms",
          packets, (uint16_t) ((uint16_t) millis() - startTime));
#endif

      // Get a random server from the pool. Unfortunately, hostByName() is a
      // blocking is a blocking call. So if the DNS resolver goes flaky,
      // everything stops. TODO: Change to a non-blocking NTP library.
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      startTime = millis();
#endif
      IPAddress ntpServerIP; // NTP server's ip address
      // TODO: check return value for errors
      // When there is an error, the ntpServerIP seems to become "0.0.0.0".
      WiFi.hostByName(mServer, ntpServerIP);
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      Serial.print((uint16_t) millis());
      Serial.print(": NtpTimeProvider::sendRequest(): resolved ");
      Serial.print(mServer);
      Serial.print(" to (");
      Serial.print(ntpServerIP);
      Serial.print("): ");
      Serial.print((uint16_t) (millis() - startTime));
      Serial.println(" ms");
#endif

      sendNtpPacket(ntpServerIP);

#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      logger("NtpTimeProvider::sendRequest(): method duration: %u ms",
          (uint16_t) (millis() - methodStart));
#endif
    }

    /** Returns number of seconds since NTP epoch (1900-01-01). */
    uint32_t readResponse() const {
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      uint16_t startTime = millis();
#endif

      // read packet into the buffer
      mUdp.read(mPacketBuffer, kNtpPacketSize);
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      logger("NtpTimeProvider::readResponse(): %u ms",
          (uint16_t) (millis() - startTime));
#endif

      // convert four bytes starting at location 40 to a long integer
      uint32_t secsSince1900 =  (uint32_t) mPacketBuffer[40] << 24;
      secsSince1900 |= (uint32_t) mPacketBuffer[41] << 16;
      secsSince1900 |= (uint32_t) mPacketBuffer[42] << 8;
      secsSince1900 |= (uint32_t) mPacketBuffer[43];
      return secsSince1900;
    }

    /**
     * Return seconds from NTP eposh 1900-01-01. Return 0 if error.
     * This is a blocking call.
     */
    uint32_t getNtpTime() const {
      sendRequest();

      uint16_t startTime = millis();
      uint16_t waitTime;
      while ((waitTime = (uint16_t) millis() - startTime) < 1000) {
        if (mUdp.parsePacket() >= kNtpPacketSize) {
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
          Serial.print("NtpTimeProvider::getNtpTime(): Received NTP response: ");
          Serial.print(waitTime);
          Serial.println(" ms");
#endif
          return readResponse();
        }
      }
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      Serial.println("NtpTimeProvider::getNtpTime(): Timed out");
#endif
      return 0; // return 0 if unable to get the time
    }

    /** Send an NTP request to the time server at the given address. */
    void sendNtpPacket(const IPAddress& address) const {
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      uint16_t startTime = millis();
#endif
      // set all bytes in the buffer to 0
      memset(mPacketBuffer, 0, kNtpPacketSize);
      // Initialize values needed to form NTP request
      // (see URL above for details on the packets)
      mPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
      mPacketBuffer[1] = 0;     // Stratum, or type of clock
      mPacketBuffer[2] = 6;     // Polling Interval
      mPacketBuffer[3] = 0xEC;  // Peer Clock Precision
      // 8 bytes of zero for Root Delay & Root Dispersion
      mPacketBuffer[12] = 49;
      mPacketBuffer[13] = 0x4E;
      mPacketBuffer[14] = 49;
      mPacketBuffer[15] = 52;
      // all NTP fields have been given values, now
      // you can send a packet requesting a timestamp:
      mUdp.beginPacket(address, 123); //NTP requests are to port 123
      mUdp.write(mPacketBuffer, kNtpPacketSize);
      mUdp.endPacket();
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
      logger("NtpTimeProvider::sendNtpPacket(): %u ms",
          (uint16_t) (millis() - startTime));
#endif
    }

    const char* const mSsid;
    const char* const mPassword;
    const char* const mServer;
    uint16_t const mLocalPort;
    uint16_t const mRequestTimeout;

    mutable WiFiUDP mUdp;
    // buffer to hold incoming & outgoing packets
    mutable uint8_t mPacketBuffer[kNtpPacketSize];
    mutable bool mIsRequestPending;
    mutable uint16_t mRequestStartTime;
#if ACE_TIME_NTP_TIME_PROVIDER_DEBUG == 1
    mutable uint16_t mRequestPendingCount;
#endif
};

}

#endif // defined(ESP8266)

#endif // ACE_TIME_NTP_TIME_PROVIDER_H

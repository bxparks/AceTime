#ifndef ACE_TIME_NTP_TIME_PROVIDER_H
#define ACE_TIME_NTP_TIME_PROVIDER_H

#if defined(ESP8266) || defined(ESP32)

#include <stdint.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include "TimeKeeper.h"
#include "common/logger.h"

#ifndef ACE_TIME_NTP_TIME_PROVIDER_DEBUG
#define ACE_TIME_NTP_TIME_PROVIDER_DEBUG 0
#endif

namespace ace_time {

/**
 * A TimeProvider that retrieves the time from an NTP server. This class has the
 * deficiency that the DNS name resolver WiFi.hostByName() is a blocking call.
 * So every now and then, it can take 5-6 seconds for the call to return,
 * blocking everything (e.g. display refresh, button clicks) until it times out.
 *
 * TODO: Create a version that uses a non-blocking DNS look up.
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
    static const uint16_t kRequestTimeout = 1000;

    /**
     * Constructor.
     * @param ssid wireless SSID
     * @param password password of the SSID
     * @param server name of the NTP server (default us.pool.ntp.org)
     * @param localPort used by the UDP client (default 8888)
     * @paran requestTimeout milliseconds for a request timesout (default 1000)
     */
    explicit NtpTimeProvider(
            const char* server = kNtpServerName,
            uint16_t localPort = kLocalPort,
            uint16_t requestTimeout = kRequestTimeout):
        mServer(server),
        mLocalPort(localPort),
        mRequestTimeout(requestTimeout) {}

    /** Set up using the provided ssid and password. */
    void setup(const char* ssid, const char* password);

    const char* getServer() const { return mServer; }

    bool isSetup() const { return mIsSetUp; }

    uint32_t getNow() const override {
      if (!mIsSetUp) return 0;

      sendRequest();

      uint16_t startTime = millis();
      while ((uint16_t) (millis() - startTime) < mRequestTimeout) {
        if (isResponseReady()) {
          return readResponse();
        }
      }
      return 0; // return 0 if unable to get the time
    }

    void sendRequest() const override {
      if (!mIsSetUp) return;

      // discard any previously received packets
      while (mUdp.parsePacket() > 0) {}

      // Get a random server from the pool. Unfortunately, hostByName() is a
      // blocking is a blocking call. So if the DNS resolver goes flaky,
      // everything stops.
      //
      // TODO: Change to a non-blocking NTP library.
      // TODO: check return value of hostByName() for errors
      // When there is an error, the ntpServerIP seems to become "0.0.0.0".
      IPAddress ntpServerIP;
      WiFi.hostByName(mServer, ntpServerIP);
      sendNtpPacket(ntpServerIP);
    }

    bool isResponseReady() const override {
      if (!mIsSetUp) return false;
      return mUdp.parsePacket() >= kNtpPacketSize;
    }

    uint32_t readResponse() const override {
      if (!mIsSetUp) return 0;

      // read packet into the buffer
      mUdp.read(mPacketBuffer, kNtpPacketSize);

      // convert four bytes starting at location 40 to a long integer
      uint32_t secsSince1900 =  (uint32_t) mPacketBuffer[40] << 24;
      secsSince1900 |= (uint32_t) mPacketBuffer[41] << 16;
      secsSince1900 |= (uint32_t) mPacketBuffer[42] << 8;
      secsSince1900 |= (uint32_t) mPacketBuffer[43];
      return (secsSince1900 == 0) ? 0 : secsSince1900 - kSecondsSinceNtpEpoch;
    }

  private:
    /** NTP time is in the first 48 bytes of message. */
    static const uint8_t kNtpPacketSize = 48;

    /**
     * Number of seconds between NTP epoch (1900-01-01T00:00:00Z) and
     * AceTime epoch (2000-01-01T00:00:00Z).
     */
    static const uint32_t kSecondsSinceNtpEpoch = 3155673600;

    /** Number of millis to wait during connect before timing out. */
    static const uint16_t kConnectTimeoutMillis = 5000;

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
      common::logger("NtpTimeProvider::sendNtpPacket(): %u ms",
          (uint16_t) (millis() - startTime));
#endif
    }

    const char* const mServer; // TODO: make this configurable
    uint16_t const mLocalPort;
    uint16_t const mRequestTimeout;

    mutable WiFiUDP mUdp;
    // buffer to hold incoming & outgoing packets
    mutable uint8_t mPacketBuffer[kNtpPacketSize];
    bool mIsSetUp = false;
};

}

#endif // defined(ESP8266) || defined(ESP32)

#endif

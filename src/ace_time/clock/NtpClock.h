/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_NTP_CLOCK_H
#define ACE_TIME_NTP_CLOCK_H

#if defined(ESP8266) || defined(ESP32)

#include <stdint.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include "../common/logging.h"
#include "Clock.h"

extern "C" unsigned long millis();

#ifndef ACE_TIME_NTP_CLOCK_DEBUG
#define ACE_TIME_NTP_CLOCK_DEBUG 0
#endif

namespace ace_time {
namespace clock {

/**
 * A Clock that retrieves the time from an NTP server. This class has the
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
class NtpClock: public Clock {
  public:
    /** Default NTP Server */
    static const char kNtpServerName[];

    /** Default port used for UDP packets. */
    static const uint16_t kLocalPort = 8888;

    /** Request time out milliseconds. */
    static const uint16_t kRequestTimeout = 1000;

    /**
     * Constructor.
     * @param server name of the NTP server (default us.pool.ntp.org)
     * @param localPort used by the UDP client (default 8888)
     * @paran requestTimeout milliseconds for a request timeout (default 1000)
     */
    explicit NtpClock(
            const char* server = kNtpServerName,
            uint16_t localPort = kLocalPort,
            uint16_t requestTimeout = kRequestTimeout):
        mServer(server),
        mLocalPort(localPort),
        mRequestTimeout(requestTimeout) {}

    /**
     * Set up the WiFi connection using the given ssid and password, and
     * prepare the UDP connection. If the WiFi connection was set up elsewhere,
     * you can call the method with no arguments to bypass the WiFi setup.
     *
     * @param server wireless SSID (default nullptr)
     * @param password password of the SSID (default nullptr)
     * @param connectTimeoutMillis how long to wait for a WiFi connection
     *    (default 10000 ms)
     */
    void setup(const char* ssid = nullptr, const char* password = nullptr,
        uint16_t connectTimeoutMillis = kConnectTimeoutMillis) {
      if (ssid) {
        WiFi.begin(ssid, password);
        uint16_t startMillis = millis();
        while (WiFi.status() != WL_CONNECTED) {
          uint16_t elapsedMillis = millis() - startMillis;
          if (elapsedMillis >= connectTimeoutMillis) {
            mIsSetUp = false;
            return;
          }

          delay(500);
        }
      }

      mUdp.begin(mLocalPort);

    #if ACE_TIME_NTP_CLOCK_DEBUG == 1
      #if defined(ESP8266)
        SERIAL_PORT_MONITOR.print(F("Local port: "));
        SERIAL_PORT_MONITOR.println(mUdp.localPort());
      #endif
    #endif

      mIsSetUp = true;
    }

    const char* getServer() const { return mServer; }

    bool isSetup() const { return mIsSetUp; }

    acetime_t getNow() const override {
      if (!mIsSetUp || WiFi.status() != WL_CONNECTED) return kInvalidSeconds;

      sendRequest();

      uint16_t startTime = millis();
      while ((uint16_t) (millis() - startTime) < mRequestTimeout) {
        if (isResponseReady()) {
          return readResponse();
        }
      }
      return kInvalidSeconds;
    }

    void sendRequest() const override {
      if (!mIsSetUp || WiFi.status() != WL_CONNECTED) return;

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
      if (!mIsSetUp || WiFi.status() != WL_CONNECTED) return false;
      return mUdp.parsePacket() >= kNtpPacketSize;
    }

    acetime_t readResponse() const override {
      if (!mIsSetUp || WiFi.status() != WL_CONNECTED) return kInvalidSeconds;

      // read packet into the buffer
      mUdp.read(mPacketBuffer, kNtpPacketSize);

      // convert four bytes starting at location 40 to a long integer
      uint32_t secsSince1900 =  (uint32_t) mPacketBuffer[40] << 24;
      secsSince1900 |= (uint32_t) mPacketBuffer[41] << 16;
      secsSince1900 |= (uint32_t) mPacketBuffer[42] << 8;
      secsSince1900 |= (uint32_t) mPacketBuffer[43];

      return (secsSince1900 == 0)
          ? kInvalidSeconds
          : secsSince1900 - kSecondsSinceNtpEpoch;
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
    static const uint16_t kConnectTimeoutMillis = 10000;

    /** Send an NTP request to the time server at the given address. */
    void sendNtpPacket(const IPAddress& address) const {
#if ACE_TIME_NTP_CLOCK_DEBUG == 1
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
#if ACE_TIME_NTP_CLOCK_DEBUG == 1
      logging::printf("NtpClock::sendNtpPacket(): %u ms\n",
          (unsigned) ((uint16_t) millis() - startTime));
#endif
    }

    const char* const mServer;
    uint16_t const mLocalPort;
    uint16_t const mRequestTimeout;

    mutable WiFiUDP mUdp;
    // buffer to hold incoming & outgoing packets
    mutable uint8_t mPacketBuffer[kNtpPacketSize];
    bool mIsSetUp = false;
};

}
}

#endif // defined(ESP8266) || defined(ESP32)

#endif

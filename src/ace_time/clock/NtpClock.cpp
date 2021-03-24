/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <Arduino.h>
#include "../common/compat.h"
#include "../common/logging.h"
#include "NtpClock.h"

#if defined(ESP8266) || defined(ESP32)

namespace ace_time {
namespace clock {

const char NtpClock::kNtpServerName[] = "us.pool.ntp.org";

void NtpClock::setup(
    const char* ssid,
    const char* password,
    uint16_t connectTimeoutMillis
) {
  if (ssid) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    uint16_t startMillis = millis();
    while (WiFi.status() != WL_CONNECTED) {
      uint16_t elapsedMillis = millis() - startMillis;
      if (elapsedMillis >= connectTimeoutMillis) {
      #if ACE_TIME_NTP_CLOCK_DEBUG >= 1
        SERIAL_PORT_MONITOR.println(F("NtpClock::setup(): failed"));
      #endif
        mIsSetUp = false;
        return;
      }

      delay(500);
    }
  }

  mUdp.begin(mLocalPort);

#if ACE_TIME_NTP_CLOCK_DEBUG >= 1
  SERIAL_PORT_MONITOR.print(F("NtpClock::setup(): connected to"));
  SERIAL_PORT_MONITOR.println(WiFi.localIP());
  #if defined(ESP8266)
    SERIAL_PORT_MONITOR.print(F("Local port: "));
    SERIAL_PORT_MONITOR.println(mUdp.localPort());
  #endif
#endif

  mIsSetUp = true;
}

acetime_t NtpClock::getNow() const {
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

void NtpClock::sendRequest() const {
  if (!mIsSetUp) return;
  if (WiFi.status() != WL_CONNECTED) {
  #if ACE_TIME_NTP_CLOCK_DEBUG >= 1
    SERIAL_PORT_MONITOR.println(
        F("NtpClock::sendRequest(): not connected"));
  #endif
    return;
  }

  // discard any previously received packets
  while (mUdp.parsePacket() > 0) {}

  #if ACE_TIME_NTP_CLOCK_DEBUG >= 2
    SERIAL_PORT_MONITOR.println(F("NtpClock::sendRequest(): sending request"));
  #endif

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

bool NtpClock::isResponseReady() const {
#if ACE_TIME_NTP_CLOCK_DEBUG >= 3
  static uint8_t rateLimiter;
#endif

  if (!mIsSetUp) return false;
  if (WiFi.status() != WL_CONNECTED) {
  #if ACE_TIME_NTP_CLOCK_DEBUG >= 3
    if (++rateLimiter == 0) {
      SERIAL_PORT_MONITOR.print("F[256]");
    }
  #endif
    return false;
  }
  #if ACE_TIME_NTP_CLOCK_DEBUG >= 3
    if (++rateLimiter == 0) {
      SERIAL_PORT_MONITOR.print(".[256]");
    }
  #endif

  return mUdp.parsePacket() >= kNtpPacketSize;
}

acetime_t NtpClock::readResponse() const {
  if (!mIsSetUp) return kInvalidSeconds;
  if (WiFi.status() != WL_CONNECTED) {
  #if ACE_TIME_NTP_CLOCK_DEBUG >= 2
    SERIAL_PORT_MONITOR.println("NtpClock::readResponse(): not connected");
  #endif
    return kInvalidSeconds;
  }

  // read packet into the buffer
  mUdp.read(mPacketBuffer, kNtpPacketSize);

  // convert four bytes starting at location 40 to a long integer
  uint32_t secsSince1900 =  (uint32_t) mPacketBuffer[40] << 24;
  secsSince1900 |= (uint32_t) mPacketBuffer[41] << 16;
  secsSince1900 |= (uint32_t) mPacketBuffer[42] << 8;
  secsSince1900 |= (uint32_t) mPacketBuffer[43];

  acetime_t epochSeconds = (secsSince1900 == 0)
      ? kInvalidSeconds
      : secsSince1900 - kSecondsSinceNtpEpoch;
  #if ACE_TIME_NTP_CLOCK_DEBUG >= 1
    SERIAL_PORT_MONITOR.print(F("NtpClock::readResponse(): epoch="));
    SERIAL_PORT_MONITOR.println(epochSeconds);
  #endif
  return epochSeconds;
}

void NtpClock::sendNtpPacket(const IPAddress& address) const {
#if ACE_TIME_NTP_CLOCK_DEBUG >= 2
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

#if ACE_TIME_NTP_CLOCK_DEBUG >= 2
  SERIAL_PORT_MONITOR.print(F("NtpClock::sendNtpPacket(): "));
  SERIAL_PORT_MONITOR.print((unsigned) ((uint16_t) millis() - startTime));
  SERIAL_PORT_MONITOR.println(" ms");
#endif
}

} // clock
} // ace_time

#endif

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
#include "Clock.h"

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
 * Warning: If you are using an ESP8266, AND you are using the `analogRead()`
 * function, calling `analogRead()` too quickly will cause the WiFi connection
 * to disconnect after 5-10 seconds. Calling NtpClock::setup() will *not* fix
 * the disconnect. See https://github.com/esp8266/Arduino/issues/1634 and
 * https://github.com/esp8266/Arduino/issues/5083. The solution is to add a
 * slightly delay between calls to analogRead(). I don't know what the minimum
 * value should be, but using a 10ms delay seems to work for me.
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
    static const uint16_t kLocalPort = 2390;

    /** Request time out milliseconds. */
    static const uint16_t kRequestTimeout = 1000;

    /**
     * Constructor.
     * @param server name of the NTP server (default us.pool.ntp.org)
     * @param localPort used by the UDP client (default 8888)
     * @param requestTimeout milliseconds for a request timeout (default 1000)
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
    void setup(
        const char* ssid = nullptr,
        const char* password = nullptr,
        uint16_t connectTimeoutMillis = kConnectTimeoutMillis);

    /** Return the name of the NTP server. */
    const char* getServer() const { return mServer; }

    /** Return true if setup() suceeded. */
    bool isSetup() const { return mIsSetUp; }

    acetime_t getNow() const override;

    void sendRequest() const override;

    bool isResponseReady() const override;

    acetime_t readResponse() const override;

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
    void sendNtpPacket(const IPAddress& address) const;

  private:
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

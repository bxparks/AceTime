#ifndef ACE_TIME_NTP_TIME_KEEPER_H
#define ACE_TIME_NTP_TIME_KEEPER_H

#if defined(ESP8266)

#include <stdint.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "TimeKeeper.h"

namespace ace_time {

class NtpTimeKeeper: public TimeKeeper {
  public:
    explicit NtpTimeKeeper(const char* ssid, const char* password):
        mSsid(ssid),
        mPassword(password) {}

    virtual bool isSettable() const override { return false; }

    virtual void setNow(uint32_t /*secondsSinceEpoch*/) override {}

    virtual void setup() override {
      Serial.println("TimeNTP Example");
      Serial.print("Connecting to ");
      Serial.println(mSsid);
      WiFi.begin(mSsid, mPassword);

      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }

      Serial.print("IP number assigned by DHCP is ");
      Serial.println(WiFi.localIP());

      Serial.println("Starting UDP");
      mUdp.begin(kLocalPort);

      Serial.print("Local port: ");
      Serial.println(mUdp.localPort());
    }

    virtual uint32_t now() const override {
      uint32_t secondsSince1900 = getNtpTime();
      return (secondsSince1900 == 0) ? 0
          : secondsSince1900 - kSecondsSinceNtpEpoch;
    }

    /**
     * Return the current time asynchronously. Returns false if response is
     * not ready, true if a result is ready or the request has timed out.
     */
    virtual bool nowAsync(uint8_t& status, uint32_t& seconds) const override {
      // send off a request, then return
      if (!mIsRequestPending) {
        sendRequest();
        mRequestStartTime = millis();
        mIsRequestPending = true;
        return false;
      }

      // there's request pending, check for time out
      if (millis() - mRequestStartTime > kRequestTimeOut) {
        Serial.println("Timed out");
        status = kStatusTimedOut;
        mIsRequestPending = false;
        return true;
      }

      // keep waiting if the response has not arrived
      if (mUdp.parsePacket() < kNtpPacketSize) return false;

      // process the response
      uint32_t secondsSince1900 = readResponse();
      seconds = (secondsSince1900 == 0) ? 0
          : secondsSince1900 - kSecondsSinceNtpEpoch;
      mIsRequestPending = false;
      status = kStatusOk;
      return true;
    }

  private:
    /** NTP Server */
    static const char kNtpServerName[];

    /** Port used for UDP packets. */
    static const uint16_t kLocalPort = 8888;

    /** NTP time is in the first 48 bytes of message. */
    static const uint8_t kNtpPacketSize = 48;

    /**
     * Number of seconds between NTP epoch (1900-01-01T00:00:00Z) and
     * AceTime epoch (2000-01-01T00:00:00Z).
     */
    static const uint32_t kSecondsSinceNtpEpoch = 3155673600;

    /** Request time out milliseconds. */
    static const uint16_t kRequestTimeOut = 1500;

    void sendRequest() const {
      while (mUdp.parsePacket() > 0); // discard any previously received packets

      // get a random server from the pool
      IPAddress ntpServerIP; // NTP server's ip address
      WiFi.hostByName(kNtpServerName, ntpServerIP);

      Serial.print("Transmit NTP request to ");
      Serial.print(kNtpServerName);
      Serial.print(" (");
      Serial.print(ntpServerIP);
      Serial.println(")");

      sendNTPpacket(ntpServerIP);
    }

    /** Returns number of seconds since NTP epoch (1900-01-01). */
    uint32_t readResponse() const {
      // read packet into the buffer
      mUdp.read(mPacketBuffer, kNtpPacketSize);

      // convert four bytes starting at location 40 to a long integer
      uint32_t secsSince1900 =  (uint32_t) mPacketBuffer[40] << 24;
      secsSince1900 |= (uint32_t) mPacketBuffer[41] << 16;
      secsSince1900 |= (uint32_t) mPacketBuffer[42] << 8;
      secsSince1900 |= (uint32_t) mPacketBuffer[43];
      return secsSince1900;
    }

    /** Return seconds from NTP eposh 1900-01-01. */
    uint32_t getNtpTime() const {
      sendRequest();

      uint16_t startTime = millis();
      uint16_t waitTime;
      while ((waitTime = millis() - startTime) < 1500) {
        if (mUdp.parsePacket() >= kNtpPacketSize) {
          Serial.print("Received NTP response: ");
          Serial.print(waitTime);
          Serial.println(" ms");
          return readResponse();
        }
      }
      Serial.println("Timed out after 1500 ms");
      return 0; // return 0 if unable to get the time
    }

    /** Send an NTP request to the time server at the given address. */
    void sendNTPpacket(const IPAddress& address) const {
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
    }

    const char* const mSsid;
    const char* const mPassword;

    mutable WiFiUDP mUdp;
    //buffer to hold incoming & outgoing packets
    mutable uint8_t mPacketBuffer[kNtpPacketSize];
    mutable bool mIsRequestPending;
    mutable uint16_t mRequestStartTime;

};

}

#endif

#endif

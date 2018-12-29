#ifndef ACE_TIME_COMMON_UTIL_H
#define ACE_TIME_COMMON_UTIL_H

#include <stdint.h>
#include <Print.h>

namespace ace_time {
namespace common {

/**
 * Increment 'd' mod 'm', avoiding '%' operator which is expensive
 * for 8-bit processors.
 */
template<typename T>
void incrementMod(T& d, T m) {
  d++;
  if (d >= m) d = 0;
}

/**
 * Increment 'd' mod 'm', with an offset, avoiding '%' operator which is
 * expensive for 8-bit processors.
 */
template<typename T>
void incrementMod(T& d, T m, T offset) {
  d -= offset;
  d++;
  if (d >= m) d = 0;
  d += offset;
}

/** Convert normal decimal numbers to binary coded decimal. */
inline uint8_t decToBcd(uint8_t val) {
  return (val/10*16) + (val%10);
}

/** Convert binary coded decimal to normal decimal numbers. */
inline uint8_t bcdToDec(uint8_t val) {
  return (val/16*10) + (val%16);
}

/** Print an unsigned 2-digit integer to 'printer'. */
inline void printPad2(Print& printer, uint8_t value, char padChar = '0') {
  if (value < 10) printer.print(padChar);
  printer.print(value);
}

/** Print an unsigned 3-digit integer to 'printer'. */
inline void printPad3(Print& printer, uint16_t val, char padChar = '0') {
  if (val < 100) printer.print(padChar);
  if (val < 10) printer.print(padChar);
  printer.print(val);
}

}
}

#endif

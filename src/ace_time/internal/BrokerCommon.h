/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BROKER_COMMON_H
#define ACE_TIME_BROKER_COMMON_H

/**
 * @file BrokerCommon.h
 *
 * Helper functions are used in both Basic brokers and Extended brokers.
 */

#include <stdint.h>

namespace ace_time {
namespace internal {

/** Convert (timeCode, timeModifier) fields in ZoneInfo to minutes. */
inline uint16_t timeCodeToMinutes(uint8_t code, uint8_t modifier) {
  return code * (uint16_t) 15 + (modifier & 0x0f);
}

/**
 * Extract the 'w', 's' 'u' suffix from the 'modifier' field, so that they can
 * be compared against kSuffixW, kSuffixS and kSuffixU.
 */
inline uint8_t toSuffix(uint8_t modifier) {
  return modifier & 0xf0;
}

} // internal
} // ace_time

#endif

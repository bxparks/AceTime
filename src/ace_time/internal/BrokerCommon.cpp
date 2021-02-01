/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#include <stddef.h>  // size_t
#include <string.h>  // strlen()
#include "../common/compat.h" // strlen_P()
#include "BrokerCommon.h"

namespace ace_time {
namespace internal {

const char* findShortName(const char* name) {
  size_t len = strlen(name);
  const char* begin = name + len;
  while (len--) {
    begin--;
    char c = *begin;
    if (c == '/' || (0 < c && c < 32)) {
      begin++;
      break;
    }
  }
  return begin;
}

const __FlashStringHelper* findShortName(const __FlashStringHelper* fname) {
  const char* name = (const char*) fname;
  size_t len = strlen_P(name);
  const char* begin = name + len;
  while (len--) {
    begin--;
    char c = pgm_read_byte(begin);
    if (c == '/' || (0 < c && c < 32)) {
      begin++;
      break;
    }
  }
  return (const __FlashStringHelper*) begin;
}

} // internal
} // ace_time

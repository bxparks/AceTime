#ifndef ACE_TIME_UTC_OFFSET_MUTATOR_H
#define ACE_TIME_UTC_OFFSET_MUTATOR_H

#include <stdint.h>
#include "common/flash.h"
#include "common/util.h"

namespace ace_time {

/**
 * A wrapper class that provides mutation operations on a UtcOffset object. The
 * recommended usage is to create a temporary object which wraps the target
 * UtcOffset object, then call the desired operation. The temporary object will
 * automatically be destroyed. Optimizing compilers should optimize out the
 * temporary object completely, thereby providing a zero-overhead abstraction.
 *
 * Using a separate mutator object provides 2 benefits. 1) It reduces the
 * complexity of the API provided by the various UtcOffset classes. 2) It
 * collects mutation operations in a single place which can be updated or
 * modified as needed.
 *
 * Example:
 *
 * @code{.cpp}
 * UtcOffset offset(...);
 * UtcOffsetMutator(offset).incrementHour();
 * @code
 */
class UtcOffsetMutator {
  public:
    /** Constructor */
    UtcOffsetMutator(UtcOffset& offset):
      mUtcOffset(offset) {}

    /**
     * Increment the UtcOffset by one hour, keeping the minute component
		 * unchanged. For usability, limit the hour to [-15, -15].
     * In other words, (UTC+15:45) by one hour wraps to (UTC-15:45).
     */
    void incrementHour() {
      int8_t code = mUtcOffset.code();
      code += 4;
      if (code >= 64) {
        code = -code + 4; // preserve the minute component
      }
      mUtcOffset.code(code);
    }

    /**
     * Increment the UtcOffset by one zone (i.e. 15 minutes) keeping the hour
     * component unchanged. If the offsetCode is negative, the cycle looks like:
     * (-01:00, -01:15, -01:30, -01:45, -01:00, ...).
     */
    void increment15Minutes() {
      int8_t code = mUtcOffset.code();
      uint8_t ucode = (code < 0) ? -code : code;
      ucode = (ucode & 0xFC) | (((ucode & 0x03) + 1) & 0x03);
      mUtcOffset.code((code < 0) ? -ucode : ucode);
    }

  private:
    UtcOffset& mUtcOffset;
};

}

#endif

#ifndef ACE_TIME_ZONE_SPEC_H
#define ACE_TIME_ZONE_SPEC_H

#include "common/common.h"
#include "UtcOffset.h"

namespace ace_time {

/**
 * Base interface for ZoneSpec classes. This contains just the getType()
 * discriminator distinguish between the 2 implementation classes
 * (ManualZoneSpec and AutoZoneSpec). The TimeZone class will use the
 * discriminator to determine which of the 2 subclasses to use at runtime.
 *
 * An alternative design was to lift various common methods (getUtcOffset(),
 * getDeltaOffset(), getAbbrev()) into this interface as virtual methods, then
 * add a virtual equals() method to implement the operator==(). I thought that
 * if the application used only AutoZoneSpec or ManualZoneSpec (but not both),
 * the compiler might be able to reduce the program code size by removing the
 * code for the unused class. However, in reality, this alternative design
 * caused the program size to *increase* by 200-300 bytes. I'm not exactly sure
 * why but that was the reality.
 */
class ZoneSpec {
  public:
    static const uint8_t kTypeManual = 0;
    static const uint8_t kTypeAuto = 1;

    /** Return the type of the zone spec. */
    virtual uint8_t getType() const = 0;
};

}

#endif

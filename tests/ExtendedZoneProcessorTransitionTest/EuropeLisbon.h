/*
 * ZoneInfo data for Europe/Lisbon that covers 1992. Extracted from
 * AceTimeValidation/ExtendedHinnantDateTest/zonedbxhd which contains zoneinfo
 * data from 1974 until 2050.
 */

#ifndef EUROPE_LISBON_H
#define EUROPE_LISBON_H

#include <AceTime.h>

namespace ace_time {
namespace zonedbxtest {

extern const extended::ZonePolicy kPolicyPort;
extern const extended::ZonePolicy kPolicyW_Eur;
extern const extended::ZonePolicy kPolicyEU;

extern const extended::ZoneInfo kZoneEurope_Lisbon; // Europe/Lisbon
const uint32_t kZoneIdEurope_Lisbon = 0x5c00a70b; // Europe/Lisbon
const uint8_t kZoneBufSizeEurope_Lisbon = 6;  // Europe/Lisbon in 1983

extern const internal::ZoneContext kZoneContext;

}
}

#endif

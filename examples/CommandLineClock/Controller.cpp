#include "Controller.h"

#if ENABLE_TIME_ZONE_TYPE_BASIC

const basic::ZoneInfo* const Controller::kBasicZoneRegistry[] ACE_TIME_PROGMEM = {
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_New_York,
};

const uint16_t Controller::kBasicZoneRegistrySize =
    sizeof(Controller::kBasicZoneRegistry) / sizeof(basic::ZoneInfo*);

#endif

#if ENABLE_TIME_ZONE_TYPE_EXTENDED

const extended::ZoneInfo* const Controller::kExtendedZoneRegistry[]
    ACE_TIME_PROGMEM = {
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_New_York,
};

const uint16_t Controller::kExtendedZoneRegistrySize =
    sizeof(Controller::kExtendedZoneRegistry) / sizeof(extended::ZoneInfo*);

#endif

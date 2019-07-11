#include "Controller.h"

#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC

const basic::ZoneInfo* const Controller::kZoneRegistry[]
    ACE_TIME_BASIC_PROGMEM = {
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_New_York,
};

const uint16_t Controller::kZoneRegistrySize =
    sizeof(Controller::kZoneRegistry) / sizeof(basic::ZoneInfo*);

#elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED

const extended::ZoneInfo* const Controller::kZoneRegistry[]
    ACE_TIME_EXTENDED_PROGMEM = {
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_New_York,
};

const uint16_t Controller::kZoneRegistrySize =
    sizeof(Controller::kZoneRegistry) / sizeof(basic::ZoneInfo*);

#endif

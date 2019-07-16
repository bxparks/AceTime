#include "Controller.h"

const basic::ZoneInfo* const Controller::kBasicZoneRegistry[]
    ACE_TIME_BASIC_PROGMEM = {
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_New_York,
};

const uint16_t Controller::kBasicZoneRegistrySize =
    sizeof(Controller::kBasicZoneRegistry) / sizeof(basic::ZoneInfo*);

// This file was copied from src/zonedb/zone_registry.cpp, then reduced
// to 85 zones so that AutoBenchmark can fit inside the 32kB limit of an
// Arduino Nano.

#include <AceTime.h>
#include "basic_registry.h"

using namespace ace_time;

//---------------------------------------------------------------------------
// Zone registry. Sorted by zoneId.
//---------------------------------------------------------------------------
const basic::Info::ZoneInfo* const kBasicRegistry[] ACE_TIME_PROGMEM = {
  &zonedb::kZoneAsia_Kuala_Lumpur, // 0x014763c4, Asia/Kuala_Lumpur
  &zonedb::kZoneIndian_Cocos, // 0x021e86de, Indian/Cocos
  &zonedb::kZoneAmerica_Mazatlan, // 0x0532189e, America/Mazatlan
  &zonedb::kZoneAmerica_Guatemala, // 0x0c8259f7, America/Guatemala
  &zonedb::kZoneAmerica_Yellowknife, // 0x0f76c76f, America/Yellowknife
  &zonedb::kZoneAmerica_Sao_Paulo, // 0x1063bfc9, America/Sao_Paulo
  &zonedb::kZoneAmerica_Indiana_Vevay, // 0x10aca054, America/Indiana/Vevay
  &zonedb::kZoneAsia_Dhaka, // 0x14c07b8b, Asia/Dhaka
  &zonedb::kZoneAsia_Qatar, // 0x15a8330b, Asia/Qatar
  &zonedb::kZoneAmerica_Guayaquil, // 0x17e64958, America/Guayaquil
  &zonedb::kZoneAmerica_New_York, // 0x1e2a7654, America/New_York
  &zonedb::kZoneAsia_Ho_Chi_Minh, // 0x20f2d127, Asia/Ho_Chi_Minh
  &zonedb::kZonePacific_Wake, // 0x23416c2b, Pacific/Wake
  &zonedb::kZoneAmerica_Monterrey, // 0x269a1deb, America/Monterrey
  &zonedb::kZoneAmerica_Vancouver, // 0x2c6f6b1f, America/Vancouver
  &zonedb::kZoneAustralia_Hobart, // 0x32bf951a, Australia/Hobart
  &zonedb::kZoneAmerica_Cayenne, // 0x3c617269, America/Cayenne
  &zonedb::kZoneEurope_Athens, // 0x4318fa27, Europe/Athens
  &zonedb::kZoneIndian_Chagos, // 0x456f7c3c, Indian/Chagos
  &zonedb::kZoneAmerica_Chicago, // 0x4b92b5d4, America/Chicago
  &zonedb::kZonePacific_Gambier, // 0x53720c3a, Pacific/Gambier
  &zonedb::kZoneAmerica_Jamaica, // 0x565dad6c, America/Jamaica
  &zonedb::kZonePacific_Marquesas, // 0x57ca7135, Pacific/Marquesas
  &zonedb::kZoneAsia_Jerusalem, // 0x5becd23a, Asia/Jerusalem
  &zonedb::kZoneEurope_London, // 0x5c6a84ae, Europe/London
  &zonedb::kZonePacific_Pago_Pago, // 0x603aebd0, Pacific/Pago_Pago
  &zonedb::kZoneEurope_Prague, // 0x65ee5d48, Europe/Prague
  &zonedb::kZoneAsia_Makassar, // 0x6aa21c85, Asia/Makassar
  &zonedb::kZoneAmerica_Dawson_Creek, // 0x6cf24e5b, America/Dawson_Creek
  &zonedb::kZoneAsia_Kolkata, // 0x72c06cd9, Asia/Kolkata
  &zonedb::kZoneAmerica_El_Salvador, // 0x752ad652, America/El_Salvador
  &zonedb::kZoneAmerica_Toronto, // 0x792e851b, America/Toronto
  &zonedb::kZoneIndian_Mauritius, // 0x7b09c02a, Indian/Mauritius
  &zonedb::kZoneAsia_Kuching, // 0x801b003b, Asia/Kuching
  &zonedb::kZoneAmerica_Atikokan, // 0x81b92098, America/Atikokan
  &zonedb::kZonePacific_Chuuk, // 0x8a090b23, Pacific/Chuuk
  &zonedb::kZonePacific_Nauru, // 0x8acc41ae, Pacific/Nauru
  &zonedb::kZonePacific_Kwajalein, // 0x8e216759, Pacific/Kwajalein
  &zonedb::kZoneAmerica_Detroit, // 0x925cfbc1, America/Detroit
  &zonedb::kZoneAmerica_Denver, // 0x97d10b2a, America/Denver
  &zonedb::kZoneAmerica_Belem, // 0x97da580b, America/Belem
  &zonedb::kZonePacific_Rarotonga, // 0x9981a3b0, Pacific/Rarotonga
  &zonedb::kZoneAsia_Baghdad, // 0x9ceffbed, Asia/Baghdad
  &zonedb::kZoneAfrica_Ndjamena, // 0x9fe09898, Africa/Ndjamena
  &zonedb::kZoneAmerica_Havana, // 0xa0e15675, America/Havana
  &zonedb::kZoneEurope_Oslo, // 0xa2c3fba1, Europe/Oslo
  &zonedb::kZoneEurope_Rome, // 0xa2c58fd7, Europe/Rome
  &zonedb::kZoneAmerica_Inuvik, // 0xa42189fc, America/Inuvik
  &zonedb::kZoneAmerica_Juneau, // 0xa6f13e2e, America/Juneau
  &zonedb::kZoneAustralia_Lord_Howe, // 0xa748b67d, Australia/Lord_Howe
  &zonedb::kZonePacific_Port_Moresby, // 0xa7ba7f68, Pacific/Port_Moresby
  &zonedb::kZoneAsia_Beirut, // 0xa7f3d5fd, Asia/Beirut
  &zonedb::kZoneAfrica_Nairobi, // 0xa87ab57e, Africa/Nairobi
  &zonedb::kZoneAsia_Brunei, // 0xa8e595f7, Asia/Brunei
  &zonedb::kZonePacific_Galapagos, // 0xa952f752, Pacific/Galapagos
  &zonedb::kZoneAmerica_La_Paz, // 0xaa29125d, America/La_Paz
  &zonedb::kZoneAmerica_Manaus, // 0xac86bf8b, America/Manaus
  &zonedb::kZoneAmerica_Merida, // 0xacd172d8, America/Merida
  &zonedb::kZoneEurope_Chisinau, // 0xad58aa18, Europe/Chisinau
  &zonedb::kZoneAmerica_Nassau, // 0xaedef011, America/Nassau
  &zonedb::kZoneEurope_Uzhgorod, // 0xb066f5d6, Europe/Uzhgorod
  &zonedb::kZoneAustralia_Broken_Hill, // 0xb06eada3, Australia/Broken_Hill
  &zonedb::kZoneAmerica_Paramaribo, // 0xb319e4c4, America/Paramaribo
  &zonedb::kZoneAmerica_Panama, // 0xb3863854, America/Panama
  &zonedb::kZoneAmerica_Los_Angeles, // 0xb7f7e8f2, America/Los_Angeles
  &zonedb::kZoneAmerica_Regina, // 0xb875371c, America/Regina
  &zonedb::kZoneAmerica_Halifax, // 0xbc5b7183, America/Halifax
  &zonedb::kZoneAsia_Riyadh, // 0xcd973d93, Asia/Riyadh
  &zonedb::kZoneAsia_Singapore, // 0xcf8581fa, Asia/Singapore
  &zonedb::kZoneAsia_Tehran, // 0xd1f02254, Asia/Tehran
  &zonedb::kZoneAfrica_Johannesburg, // 0xd5d157a0, Africa/Johannesburg
  &zonedb::kZoneEtc_UTC, // 0xd8e31abc, Etc/UTC
  &zonedb::kZoneEurope_Brussels, // 0xdee07337, Europe/Brussels
  &zonedb::kZoneAntarctica_Syowa, // 0xe330c7e1, Antarctica/Syowa
  &zonedb::kZoneMST7MDT, // 0xf2af9375, MST7MDT
  &zonedb::kZoneEurope_Gibraltar, // 0xf8e325fc, Europe/Gibraltar
  &zonedb::kZoneAmerica_Montevideo, // 0xfa214780, America/Montevideo
  &zonedb::kZoneEurope_Bucharest, // 0xfb349ec5, Europe/Bucharest
  &zonedb::kZoneEurope_Paris, // 0xfb4bc2a3, Europe/Paris
  &zonedb::kZoneEurope_Sofia, // 0xfb898656, Europe/Sofia
  &zonedb::kZoneAtlantic_Canary, // 0xfc23f2c2, Atlantic/Canary
  &zonedb::kZoneAmerica_Campo_Grande, // 0xfec3e7a6, America/Campo_Grande

};

const uint16_t kBasicRegistrySize =
    sizeof(kBasicRegistry) / sizeof(basic::Info::ZoneInfo*);

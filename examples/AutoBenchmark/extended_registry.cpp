// This file was copied from src/zonedb/zone_registry.cpp, then reduced
// to 85 zones so that AutoBenchmark can fit inside the 32kB limit of an
// Arduino Nano.

#include <AceTime.h>
#include "extended_registry.h"

using namespace ace_time;

//---------------------------------------------------------------------------
// Zone registry. Sorted by zoneId.
//---------------------------------------------------------------------------
const extended::Info::ZoneInfo* const kExtendedRegistry[] ACE_TIME_PROGMEM = {
  &zonedbx::kZoneAsia_Kuala_Lumpur, // 0x014763c4, Asia/Kuala_Lumpur
  &zonedbx::kZoneIndian_Cocos, // 0x021e86de, Indian/Cocos
  &zonedbx::kZoneAmerica_Mazatlan, // 0x0532189e, America/Mazatlan
  &zonedbx::kZoneAmerica_Guatemala, // 0x0c8259f7, America/Guatemala
  &zonedbx::kZoneAmerica_Yellowknife, // 0x0f76c76f, America/Yellowknife
  &zonedbx::kZoneAmerica_Sao_Paulo, // 0x1063bfc9, America/Sao_Paulo
  &zonedbx::kZoneAmerica_Indiana_Vevay, // 0x10aca054, America/Indiana/Vevay
  &zonedbx::kZoneAsia_Dhaka, // 0x14c07b8b, Asia/Dhaka
  &zonedbx::kZoneAsia_Qatar, // 0x15a8330b, Asia/Qatar
  &zonedbx::kZoneAmerica_Guayaquil, // 0x17e64958, America/Guayaquil
  &zonedbx::kZoneAmerica_New_York, // 0x1e2a7654, America/New_York
  &zonedbx::kZoneAsia_Ho_Chi_Minh, // 0x20f2d127, Asia/Ho_Chi_Minh
  &zonedbx::kZonePacific_Wake, // 0x23416c2b, Pacific/Wake
  &zonedbx::kZoneAmerica_Monterrey, // 0x269a1deb, America/Monterrey
  &zonedbx::kZoneAmerica_Vancouver, // 0x2c6f6b1f, America/Vancouver
  &zonedbx::kZoneAustralia_Hobart, // 0x32bf951a, Australia/Hobart
  &zonedbx::kZoneAmerica_Cayenne, // 0x3c617269, America/Cayenne
  &zonedbx::kZoneEurope_Athens, // 0x4318fa27, Europe/Athens
  &zonedbx::kZoneIndian_Chagos, // 0x456f7c3c, Indian/Chagos
  &zonedbx::kZoneAmerica_Chicago, // 0x4b92b5d4, America/Chicago
  &zonedbx::kZoneAmerica_Asuncion, // 0x50ec79a6, America/Asuncion
  &zonedbx::kZonePacific_Gambier, // 0x53720c3a, Pacific/Gambier
  &zonedbx::kZoneAmerica_Jamaica, // 0x565dad6c, America/Jamaica
  &zonedbx::kZonePacific_Marquesas, // 0x57ca7135, Pacific/Marquesas
  &zonedbx::kZoneAsia_Jerusalem, // 0x5becd23a, Asia/Jerusalem
  &zonedbx::kZoneEurope_London, // 0x5c6a84ae, Europe/London
  &zonedbx::kZonePacific_Pago_Pago, // 0x603aebd0, Pacific/Pago_Pago
  &zonedbx::kZoneEurope_Prague, // 0x65ee5d48, Europe/Prague
  &zonedbx::kZoneAsia_Makassar, // 0x6aa21c85, Asia/Makassar
  &zonedbx::kZoneAmerica_Dawson_Creek, // 0x6cf24e5b, America/Dawson_Creek
  &zonedbx::kZoneAsia_Kolkata, // 0x72c06cd9, Asia/Kolkata
  &zonedbx::kZoneAmerica_El_Salvador, // 0x752ad652, America/El_Salvador
  &zonedbx::kZoneAmerica_Toronto, // 0x792e851b, America/Toronto
  &zonedbx::kZoneIndian_Mauritius, // 0x7b09c02a, Indian/Mauritius
  &zonedbx::kZoneAsia_Kuching, // 0x801b003b, Asia/Kuching
  &zonedbx::kZoneAmerica_Atikokan, // 0x81b92098, America/Atikokan
  &zonedbx::kZonePacific_Chuuk, // 0x8a090b23, Pacific/Chuuk
  &zonedbx::kZonePacific_Nauru, // 0x8acc41ae, Pacific/Nauru
  &zonedbx::kZonePacific_Kwajalein, // 0x8e216759, Pacific/Kwajalein
  &zonedbx::kZoneAmerica_Detroit, // 0x925cfbc1, America/Detroit
  &zonedbx::kZoneAmerica_Denver, // 0x97d10b2a, America/Denver
  &zonedbx::kZoneAmerica_Belem, // 0x97da580b, America/Belem
  &zonedbx::kZonePacific_Rarotonga, // 0x9981a3b0, Pacific/Rarotonga
  &zonedbx::kZoneAsia_Baghdad, // 0x9ceffbed, Asia/Baghdad
  &zonedbx::kZoneAfrica_Ndjamena, // 0x9fe09898, Africa/Ndjamena
  &zonedbx::kZoneAmerica_Havana, // 0xa0e15675, America/Havana
  &zonedbx::kZoneEurope_Oslo, // 0xa2c3fba1, Europe/Oslo
  &zonedbx::kZoneEurope_Rome, // 0xa2c58fd7, Europe/Rome
  &zonedbx::kZoneAmerica_Inuvik, // 0xa42189fc, America/Inuvik
  &zonedbx::kZoneAmerica_Juneau, // 0xa6f13e2e, America/Juneau
  &zonedbx::kZoneAustralia_Lord_Howe, // 0xa748b67d, Australia/Lord_Howe
  &zonedbx::kZonePacific_Port_Moresby, // 0xa7ba7f68, Pacific/Port_Moresby
  &zonedbx::kZoneAsia_Beirut, // 0xa7f3d5fd, Asia/Beirut
  &zonedbx::kZoneAfrica_Nairobi, // 0xa87ab57e, Africa/Nairobi
  &zonedbx::kZoneAsia_Brunei, // 0xa8e595f7, Asia/Brunei
  &zonedbx::kZonePacific_Galapagos, // 0xa952f752, Pacific/Galapagos
  &zonedbx::kZoneAmerica_La_Paz, // 0xaa29125d, America/La_Paz
  &zonedbx::kZoneAmerica_Manaus, // 0xac86bf8b, America/Manaus
  &zonedbx::kZoneAmerica_Merida, // 0xacd172d8, America/Merida
  &zonedbx::kZoneEurope_Chisinau, // 0xad58aa18, Europe/Chisinau
  &zonedbx::kZoneAmerica_Nassau, // 0xaedef011, America/Nassau
  &zonedbx::kZoneEurope_Uzhgorod, // 0xb066f5d6, Europe/Uzhgorod
  &zonedbx::kZoneAustralia_Broken_Hill, // 0xb06eada3, Australia/Broken_Hill
  &zonedbx::kZoneAmerica_Paramaribo, // 0xb319e4c4, America/Paramaribo
  &zonedbx::kZoneAmerica_Panama, // 0xb3863854, America/Panama
  &zonedbx::kZoneAmerica_Los_Angeles, // 0xb7f7e8f2, America/Los_Angeles
  &zonedbx::kZoneAmerica_Regina, // 0xb875371c, America/Regina
  &zonedbx::kZoneAmerica_Halifax, // 0xbc5b7183, America/Halifax
  &zonedbx::kZoneAsia_Riyadh, // 0xcd973d93, Asia/Riyadh
  &zonedbx::kZoneAsia_Singapore, // 0xcf8581fa, Asia/Singapore
  &zonedbx::kZoneAsia_Tehran, // 0xd1f02254, Asia/Tehran
  &zonedbx::kZoneAfrica_Johannesburg, // 0xd5d157a0, Africa/Johannesburg
  &zonedbx::kZoneEtc_UTC, // 0xd8e31abc, Etc/UTC
  &zonedbx::kZoneEurope_Brussels, // 0xdee07337, Europe/Brussels
  &zonedbx::kZoneAntarctica_Syowa, // 0xe330c7e1, Antarctica/Syowa
  &zonedbx::kZoneMST7MDT, // 0xf2af9375, MST7MDT
  &zonedbx::kZoneEurope_Gibraltar, // 0xf8e325fc, Europe/Gibraltar
  &zonedbx::kZoneAmerica_Montevideo, // 0xfa214780, America/Montevideo
  &zonedbx::kZoneEurope_Bucharest, // 0xfb349ec5, Europe/Bucharest
  &zonedbx::kZoneEurope_Paris, // 0xfb4bc2a3, Europe/Paris
  &zonedbx::kZoneEurope_Sofia, // 0xfb898656, Europe/Sofia
  &zonedbx::kZoneAtlantic_Canary, // 0xfc23f2c2, Atlantic/Canary
  &zonedbx::kZoneAmerica_Campo_Grande, // 0xfec3e7a6, America/Campo_Grande

};

const uint16_t kExtendedRegistrySize =
    sizeof(kExtendedRegistry) / sizeof(extended::Info::ZoneInfo*);

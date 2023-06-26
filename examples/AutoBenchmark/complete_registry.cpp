// This file was copied from src/zonedb/zone_registry.cpp, then reduced
// to 85 zones so that AutoBenchmark can fit inside the 32kB limit of an
// Arduino Nano.

#include <AceTime.h>
#include "complete_registry.h"

using namespace ace_time;

//---------------------------------------------------------------------------
// Zone registry. Sorted by zoneId.
//---------------------------------------------------------------------------
const complete::ZoneInfo* const kCompleteRegistry[kCompleteRegistrySize]
    ACE_TIME_PROGMEM = {
  &zonedbc::kZoneAsia_Kuala_Lumpur, // 0x014763c4, Asia/Kuala_Lumpur
  &zonedbc::kZoneIndian_Cocos, // 0x021e86de, Indian/Cocos
  &zonedbc::kZoneAmerica_Mazatlan, // 0x0532189e, America/Mazatlan
  &zonedbc::kZoneAmerica_Guatemala, // 0x0c8259f7, America/Guatemala
  &zonedbc::kZoneAmerica_Yellowknife, // 0x0f76c76f, America/Yellowknife
  &zonedbc::kZoneAmerica_Sao_Paulo, // 0x1063bfc9, America/Sao_Paulo
  &zonedbc::kZoneAmerica_Indiana_Vevay, // 0x10aca054, America/Indiana/Vevay
  &zonedbc::kZoneAsia_Dhaka, // 0x14c07b8b, Asia/Dhaka
  &zonedbc::kZoneAsia_Qatar, // 0x15a8330b, Asia/Qatar
  &zonedbc::kZoneAmerica_Guayaquil, // 0x17e64958, America/Guayaquil
  &zonedbc::kZoneAmerica_New_York, // 0x1e2a7654, America/New_York
  &zonedbc::kZoneAsia_Ho_Chi_Minh, // 0x20f2d127, Asia/Ho_Chi_Minh
  &zonedbc::kZonePacific_Wake, // 0x23416c2b, Pacific/Wake
  &zonedbc::kZoneAmerica_Monterrey, // 0x269a1deb, America/Monterrey
  &zonedbc::kZoneAmerica_Vancouver, // 0x2c6f6b1f, America/Vancouver
  &zonedbc::kZoneAustralia_Hobart, // 0x32bf951a, Australia/Hobart
  &zonedbc::kZoneAmerica_Cayenne, // 0x3c617269, America/Cayenne
  &zonedbc::kZoneEurope_Athens, // 0x4318fa27, Europe/Athens
  &zonedbc::kZoneIndian_Chagos, // 0x456f7c3c, Indian/Chagos
  &zonedbc::kZoneAmerica_Chicago, // 0x4b92b5d4, America/Chicago
  &zonedbc::kZoneAmerica_Asuncion, // 0x50ec79a6, America/Asuncion
  &zonedbc::kZonePacific_Gambier, // 0x53720c3a, Pacific/Gambier
  &zonedbc::kZoneAmerica_Jamaica, // 0x565dad6c, America/Jamaica
  &zonedbc::kZonePacific_Marquesas, // 0x57ca7135, Pacific/Marquesas
  &zonedbc::kZoneAsia_Jerusalem, // 0x5becd23a, Asia/Jerusalem
  &zonedbc::kZoneEurope_London, // 0x5c6a84ae, Europe/London
  &zonedbc::kZonePacific_Pago_Pago, // 0x603aebd0, Pacific/Pago_Pago
  &zonedbc::kZoneEurope_Prague, // 0x65ee5d48, Europe/Prague
  &zonedbc::kZoneAsia_Makassar, // 0x6aa21c85, Asia/Makassar
  &zonedbc::kZoneAmerica_Dawson_Creek, // 0x6cf24e5b, America/Dawson_Creek
  &zonedbc::kZoneAsia_Kolkata, // 0x72c06cd9, Asia/Kolkata
  &zonedbc::kZoneAmerica_El_Salvador, // 0x752ad652, America/El_Salvador
  &zonedbc::kZoneAmerica_Toronto, // 0x792e851b, America/Toronto
  &zonedbc::kZoneIndian_Mauritius, // 0x7b09c02a, Indian/Mauritius
  &zonedbc::kZoneAsia_Kuching, // 0x801b003b, Asia/Kuching
  &zonedbc::kZoneAmerica_Atikokan, // 0x81b92098, America/Atikokan
  &zonedbc::kZonePacific_Chuuk, // 0x8a090b23, Pacific/Chuuk
  &zonedbc::kZonePacific_Nauru, // 0x8acc41ae, Pacific/Nauru
  &zonedbc::kZonePacific_Kwajalein, // 0x8e216759, Pacific/Kwajalein
  &zonedbc::kZoneAmerica_Detroit, // 0x925cfbc1, America/Detroit
  &zonedbc::kZoneAmerica_Denver, // 0x97d10b2a, America/Denver
  &zonedbc::kZoneAmerica_Belem, // 0x97da580b, America/Belem
  &zonedbc::kZonePacific_Rarotonga, // 0x9981a3b0, Pacific/Rarotonga
  &zonedbc::kZoneAsia_Baghdad, // 0x9ceffbed, Asia/Baghdad
  &zonedbc::kZoneAfrica_Ndjamena, // 0x9fe09898, Africa/Ndjamena
  &zonedbc::kZoneAmerica_Havana, // 0xa0e15675, America/Havana
  &zonedbc::kZoneEurope_Oslo, // 0xa2c3fba1, Europe/Oslo
  &zonedbc::kZoneEurope_Rome, // 0xa2c58fd7, Europe/Rome
  &zonedbc::kZoneAmerica_Inuvik, // 0xa42189fc, America/Inuvik
  &zonedbc::kZoneAmerica_Juneau, // 0xa6f13e2e, America/Juneau
  &zonedbc::kZoneAustralia_Lord_Howe, // 0xa748b67d, Australia/Lord_Howe
  &zonedbc::kZonePacific_Port_Moresby, // 0xa7ba7f68, Pacific/Port_Moresby
  &zonedbc::kZoneAsia_Beirut, // 0xa7f3d5fd, Asia/Beirut
  &zonedbc::kZoneAfrica_Nairobi, // 0xa87ab57e, Africa/Nairobi
  &zonedbc::kZoneAsia_Brunei, // 0xa8e595f7, Asia/Brunei
  &zonedbc::kZonePacific_Galapagos, // 0xa952f752, Pacific/Galapagos
  &zonedbc::kZoneAmerica_La_Paz, // 0xaa29125d, America/La_Paz
  &zonedbc::kZoneAmerica_Manaus, // 0xac86bf8b, America/Manaus
  &zonedbc::kZoneAmerica_Merida, // 0xacd172d8, America/Merida
  &zonedbc::kZoneEurope_Chisinau, // 0xad58aa18, Europe/Chisinau
  &zonedbc::kZoneAmerica_Nassau, // 0xaedef011, America/Nassau
  &zonedbc::kZoneEurope_Uzhgorod, // 0xb066f5d6, Europe/Uzhgorod
  &zonedbc::kZoneAustralia_Broken_Hill, // 0xb06eada3, Australia/Broken_Hill
  &zonedbc::kZoneAmerica_Paramaribo, // 0xb319e4c4, America/Paramaribo
  &zonedbc::kZoneAmerica_Panama, // 0xb3863854, America/Panama
  &zonedbc::kZoneAmerica_Los_Angeles, // 0xb7f7e8f2, America/Los_Angeles
  &zonedbc::kZoneAmerica_Regina, // 0xb875371c, America/Regina
  &zonedbc::kZoneAmerica_Halifax, // 0xbc5b7183, America/Halifax
  &zonedbc::kZoneAsia_Riyadh, // 0xcd973d93, Asia/Riyadh
  &zonedbc::kZoneAsia_Singapore, // 0xcf8581fa, Asia/Singapore
  &zonedbc::kZoneAsia_Tehran, // 0xd1f02254, Asia/Tehran
  &zonedbc::kZoneAfrica_Johannesburg, // 0xd5d157a0, Africa/Johannesburg
  &zonedbc::kZoneEtc_UTC, // 0xd8e31abc, Etc/UTC
  &zonedbc::kZoneEurope_Brussels, // 0xdee07337, Europe/Brussels
  &zonedbc::kZoneAntarctica_Syowa, // 0xe330c7e1, Antarctica/Syowa
  &zonedbc::kZoneMST7MDT, // 0xf2af9375, MST7MDT
  &zonedbc::kZoneEurope_Gibraltar, // 0xf8e325fc, Europe/Gibraltar
  &zonedbc::kZoneAmerica_Montevideo, // 0xfa214780, America/Montevideo
  &zonedbc::kZoneEurope_Bucharest, // 0xfb349ec5, Europe/Bucharest
  &zonedbc::kZoneEurope_Paris, // 0xfb4bc2a3, Europe/Paris
  &zonedbc::kZoneEurope_Sofia, // 0xfb898656, Europe/Sofia
  &zonedbc::kZoneAtlantic_Canary, // 0xfc23f2c2, Atlantic/Canary
  &zonedbc::kZoneAmerica_Campo_Grande, // 0xfec3e7a6, America/Campo_Grande

};

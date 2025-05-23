// This file was generated by the following script:
//
//   $ /home/brian/src/AceTimeSuite/compiler/src/acetimecompiler/tzcompiler.py
//     --input_dir /home/brian/src/AceTimeSuite/libraries/AceTimeLib/src/testingzonedbx/tzfiles
//     --output_dir /home/brian/src/AceTimeSuite/libraries/AceTimeLib/src/testingzonedbx
//     --tz_version 2025b
//     --action zonedb
//     --language arduino
//     --scope extended
//     --db_namespace testingzonedbx
//     --zi_namespace extended::Info
//     --include_list include_list.txt
//     --nocompress
//     --start_year 1980
//     --until_year 2200
//
// using the TZ Database files
//
//   africa
//   antarctica
//   asia
//   australasia
//   backward
//   etcetera
//   europe
//   northamerica
//   southamerica
//
// from https://github.com/eggert/tz/releases/tag/2025b
//
// Supported Zones: 16 (15 zones, 1 links)
// Unsupported Zones: 581 (325 zones, 256 links)
//
// Requested Years: [1980,2200]
// Accurate Years: [1980,32767]
//
// Original Years:  [1844,2087]
// Generated Years: [1945,2087]
// Lower/Upper Truncated: [True,False]
//
// Estimator Years: [1945,2090]
// Max Buffer Size: 7
//
// Records:
//   Infos: 16
//   Eras: 30
//   Policies: 11
//   Rules: 220
//
// Memory (8-bits):
//   Context: 16
//   Rules: 1980
//   Policies: 33
//   Eras: 330
//   Zones: 195
//   Links: 13
//   Registry: 32
//   Formats: 47
//   Letters: 23
//   Fragments: 0
//   Names: 260 (original: 260)
//   TOTAL: 2929
//
// Memory (32-bits):
//   Context: 24
//   Rules: 2640
//   Policies: 88
//   Eras: 480
//   Zones: 360
//   Links: 24
//   Registry: 64
//   Formats: 47
//   Letters: 33
//   Fragments: 0
//   Names: 260 (original: 260)
//   TOTAL: 4020
//
// DO NOT EDIT

#ifndef ACE_TIME_TESTINGZONEDBX_ZONE_REGISTRY_H
#define ACE_TIME_TESTINGZONEDBX_ZONE_REGISTRY_H

#include <zoneinfo/infos.h>

namespace ace_time {
namespace testingzonedbx {

// Zones
const uint16_t kZoneRegistrySize = 15;
extern const extended::Info::ZoneInfo* const kZoneRegistry[15];

// Zones and Links
const uint16_t kZoneAndLinkRegistrySize = 16;
extern const extended::Info::ZoneInfo* const kZoneAndLinkRegistry[16];

}
}
#endif

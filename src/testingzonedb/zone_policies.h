// This file was generated by the following script:
//
//   $ /home/brian/src/AceTimeSuite/compiler/src/acetimecompiler/tzcompiler.py
//     --input_dir /home/brian/src/AceTimeSuite/libraries/AceTimeLib/src/testingzonedb/tzfiles
//     --output_dir /home/brian/src/AceTimeSuite/libraries/AceTimeLib/src/testingzonedb
//     --tz_version 2025b
//     --action zonedb
//     --language arduino
//     --scope basic
//     --db_namespace testingzonedb
//     --zi_namespace basic::Info
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
// Supported Zones: 12 (11 zones, 1 links)
// Unsupported Zones: 585 (329 zones, 256 links)
//
// Requested Years: [1980,2200]
// Accurate Years: [1980,32767]
//
// Original Years:  [1844,2087]
// Generated Years: [1945,2007]
// Lower/Upper Truncated: [True,False]
//
// Estimator Years: [1945,2009]
// Max Buffer Size: 6
//
// Records:
//   Infos: 12
//   Eras: 15
//   Policies: 8
//   Rules: 32
//
// Memory (8-bits):
//   Context: 16
//   Rules: 288
//   Policies: 24
//   Eras: 165
//   Zones: 143
//   Links: 13
//   Registry: 24
//   Formats: 27
//   Letters: 11
//   Fragments: 0
//   Names: 202 (original: 202)
//   TOTAL: 913
//
// Memory (32-bits):
//   Context: 24
//   Rules: 384
//   Policies: 64
//   Eras: 240
//   Zones: 264
//   Links: 24
//   Registry: 48
//   Formats: 27
//   Letters: 17
//   Fragments: 0
//   Names: 202 (original: 202)
//   TOTAL: 1294
//
// DO NOT EDIT

#ifndef ACE_TIME_TESTINGZONEDB_ZONE_POLICIES_H
#define ACE_TIME_TESTINGZONEDB_ZONE_POLICIES_H

#include <zoneinfo/infos.h>

namespace ace_time {
namespace testingzonedb {

//---------------------------------------------------------------------------
// Supported policies: 8
//---------------------------------------------------------------------------

extern const basic::Info::ZonePolicy kZonePolicyAus;
extern const basic::Info::ZonePolicy kZonePolicyCanada;
extern const basic::Info::ZonePolicy kZonePolicyEcuador;
extern const basic::Info::ZonePolicy kZonePolicyEdm;
extern const basic::Info::ZonePolicy kZonePolicySA;
extern const basic::Info::ZonePolicy kZonePolicyUS;
extern const basic::Info::ZonePolicy kZonePolicyVanc;
extern const basic::Info::ZonePolicy kZonePolicyWinn;


//---------------------------------------------------------------------------
// Unsupported policies: 126
//---------------------------------------------------------------------------

// AN {unused}
// AQ {unused}
// AS {unused}
// AT {unused}
// AV {unused}
// AW {unused}
// Albania {unused}
// Algeria {unused}
// Arg {unused}
// Armenia {unused}
// Austria {unused}
// Azer {unused}
// Barb {unused}
// Belgium {unused}
// Belize {unused}
// Bermuda {unused}
// Brazil {unused}
// Bulg {unused}
// C-Eur {unused}
// CA {unused}
// CO {unused}
// CR {unused}
// Chatham {unused}
// Chicago {unused}
// Chile {unused}
// Cook {unused}
// Cuba {unused}
// Cyprus {unused}
// Czech {unused}
// DR {unused}
// Denver {unused}
// Detroit {unused}
// Dhaka {unused}
// E-Eur {unused}
// E-EurAsia {unused}
// EU {unused}
// EUAsia {unused}
// Egypt {unused}
// EgyptAsia {unused}
// Eire {unused}
// Falk {unused}
// Fiji {unused}
// Finland {unused}
// France {unused}
// GB-Eire {unused}
// Germany {unused}
// Greece {unused}
// Guam {unused}
// Guat {unused}
// HK {unused}
// Haiti {unused}
// Halifax {unused}
// Holiday {unused}
// Hond {unused}
// Hungary {unused}
// Indianapolis {unused}
// Iran {unused}
// Iraq {unused}
// Italy {unused}
// Japan {unused}
// Jordan {unused}
// Kyrgyz {unused}
// LH {unused}
// Latvia {unused}
// Lebanon {unused}
// Libya {unused}
// Louisville {unused}
// Macau {unused}
// Malta {unused}
// Marengo {unused}
// Mauritius {unused}
// Menominee {unused}
// Mexico {unused}
// Moldova {unused}
// Moncton {unused}
// Mongol {unused}
// Morocco {unused}
// NBorneo {unused}
// NC {unused}
// NT_YK {unused}
// NYC {unused}
// NZ {unused}
// Namibia {unused}
// Nic {unused}
// PRC {unused}
// Pakistan {unused}
// Palestine {unused}
// Para {unused}
// Perry {unused}
// Peru {unused}
// Phil {unused}
// Pike {unused}
// Poland {unused}
// Port {unused}
// Pulaski {unused}
// ROK {unused}
// Regina {unused}
// Romania {unused}
// Russia {unused}
// RussiaAsia {unused}
// Salv {unused}
// SanLuis {unused}
// Shang {unused}
// SovietZone {unused}
// Spain {unused}
// SpainAfrica {unused}
// StJohns {unused}
// Starke {unused}
// Sudan {unused}
// Swift {unused}
// Swiss {unused}
// Syria {unused}
// Taiwan {unused}
// Thule {unused}
// Tonga {unused}
// Toronto {unused}
// Troll {unused}
// Tunisia {unused}
// Turkey {unused}
// Uruguay {unused}
// Vanuatu {unused}
// Vincennes {unused}
// W-Eur {unused}
// WS {unused}
// Yukon {unused}
// Zion {unused}


//---------------------------------------------------------------------------
// Notable zone policies: 0
//---------------------------------------------------------------------------



}
}

#endif

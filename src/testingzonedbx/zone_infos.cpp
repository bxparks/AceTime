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

#include <zoneinfo/compat.h>
#include "zone_policies.h"
#include "zone_infos.h"

namespace ace_time {
namespace testingzonedbx {

//---------------------------------------------------------------------------
// ZoneContext
//---------------------------------------------------------------------------

static const char kVersionString[] ACE_TIME_PROGMEM = "2025b";
const __FlashStringHelper* const kTzDatabaseVersion =
    (const __FlashStringHelper*) kVersionString;


const char* const kFragments[] ACE_TIME_PROGMEM = {
  nullptr, // '\x00' cannot exist

};

static const char kLetter0[] ACE_TIME_PROGMEM = "";
static const char kLetter1[] ACE_TIME_PROGMEM = "CAT";
static const char kLetter2[] ACE_TIME_PROGMEM = "D";
static const char kLetter3[] ACE_TIME_PROGMEM = "S";
static const char kLetter4[] ACE_TIME_PROGMEM = "WAT";

const char* const kLetters[] ACE_TIME_PROGMEM = {
  kLetter0, // ""
  kLetter1, // "CAT"
  kLetter2, // "D"
  kLetter3, // "S"
  kLetter4, // "WAT"

};

const extended::Info::ZoneContext kZoneContext ACE_TIME_PROGMEM = {
  1980 /*startYear*/,
  2200 /*untilYear*/,
  1980 /*startYearAccurate*/,
  32767 /*untilYearAccurate*/,
  2100 /*baseYear*/,
  7 /*maxTransitions*/,
  kVersionString /*tzVersion*/,
  1 /*numFragments*/,
  5 /*numLetters*/,
  kFragments /*fragments*/,
  kLetters /*letters*/,
};

//---------------------------------------------------------------------------
// Zones: 15
// Eras: 30
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Zone name: Africa/Casablanca
// Eras: 4
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraAfrica_Casablanca[] ACE_TIME_PROGMEM = {
  //              0:00    Morocco    %z    1984 Mar 16
  {
    &kZonePolicyMorocco /*zonePolicy*/,
    "" /*format*/,
    0 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -116 /*untilYearTiny*/,
    3 /*untilMonth*/,
    16 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },
  //              1:00    -    %z    1986
  {
    nullptr /*zonePolicy*/,
    "" /*format*/,
    4 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -114 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },
  //              0:00    Morocco    %z    2018 Oct 28  3:00
  {
    &kZonePolicyMorocco /*zonePolicy*/,
    "" /*format*/,
    0 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -82 /*untilYearTiny*/,
    10 /*untilMonth*/,
    28 /*untilDay*/,
    12 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },
  //              1:00    Morocco    %z
  {
    &kZonePolicyMorocco /*zonePolicy*/,
    "" /*format*/,
    4 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameAfrica_Casablanca[] ACE_TIME_PROGMEM = "Africa/Casablanca";

const extended::Info::ZoneInfo kZoneAfrica_Casablanca ACE_TIME_PROGMEM = {
  kZoneNameAfrica_Casablanca /*name*/,
  0xc59f1b33 /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  4 /*numEras*/,
  kZoneEraAfrica_Casablanca /*eras*/,
  nullptr /*targetInfo*/,
};

//---------------------------------------------------------------------------
// Zone name: Africa/Windhoek
// Eras: 2
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraAfrica_Windhoek[] ACE_TIME_PROGMEM = {
  //             2:00    -    SAST    1990 Mar 21
  {
    nullptr /*zonePolicy*/,
    "SAST" /*format*/,
    8 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -110 /*untilYearTiny*/,
    3 /*untilMonth*/,
    21 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },
  //             2:00    Namibia    %s
  {
    &kZonePolicyNamibia /*zonePolicy*/,
    "%" /*format*/,
    8 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameAfrica_Windhoek[] ACE_TIME_PROGMEM = "Africa/Windhoek";

const extended::Info::ZoneInfo kZoneAfrica_Windhoek ACE_TIME_PROGMEM = {
  kZoneNameAfrica_Windhoek /*name*/,
  0x789c9bd3 /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  2 /*numEras*/,
  kZoneEraAfrica_Windhoek /*eras*/,
  nullptr /*targetInfo*/,
};

//---------------------------------------------------------------------------
// Zone name: America/Caracas
// Eras: 3
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraAmerica_Caracas[] ACE_TIME_PROGMEM = {
  //             -4:00    -    %z    2007 Dec  9  3:00
  {
    nullptr /*zonePolicy*/,
    "" /*format*/,
    -16 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -93 /*untilYearTiny*/,
    12 /*untilMonth*/,
    9 /*untilDay*/,
    12 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },
  //             -4:30    -    %z    2016 May  1  2:30
  {
    nullptr /*zonePolicy*/,
    "" /*format*/,
    -18 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -84 /*untilYearTiny*/,
    5 /*untilMonth*/,
    1 /*untilDay*/,
    10 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },
  //             -4:00    -    %z
  {
    nullptr /*zonePolicy*/,
    "" /*format*/,
    -16 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameAmerica_Caracas[] ACE_TIME_PROGMEM = "America/Caracas";

const extended::Info::ZoneInfo kZoneAmerica_Caracas ACE_TIME_PROGMEM = {
  kZoneNameAmerica_Caracas /*name*/,
  0x3be064f4 /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  3 /*numEras*/,
  kZoneEraAmerica_Caracas /*eras*/,
  nullptr /*targetInfo*/,
};

//---------------------------------------------------------------------------
// Zone name: America/Chicago
// Eras: 1
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraAmerica_Chicago[] ACE_TIME_PROGMEM = {
  //             -6:00    US    C%sT
  {
    &kZonePolicyUS /*zonePolicy*/,
    "C%T" /*format*/,
    -24 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameAmerica_Chicago[] ACE_TIME_PROGMEM = "America/Chicago";

const extended::Info::ZoneInfo kZoneAmerica_Chicago ACE_TIME_PROGMEM = {
  kZoneNameAmerica_Chicago /*name*/,
  0x4b92b5d4 /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  1 /*numEras*/,
  kZoneEraAmerica_Chicago /*eras*/,
  nullptr /*targetInfo*/,
};

//---------------------------------------------------------------------------
// Zone name: America/Denver
// Eras: 1
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraAmerica_Denver[] ACE_TIME_PROGMEM = {
  //             -7:00    US    M%sT
  {
    &kZonePolicyUS /*zonePolicy*/,
    "M%T" /*format*/,
    -28 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameAmerica_Denver[] ACE_TIME_PROGMEM = "America/Denver";

const extended::Info::ZoneInfo kZoneAmerica_Denver ACE_TIME_PROGMEM = {
  kZoneNameAmerica_Denver /*name*/,
  0x97d10b2a /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  1 /*numEras*/,
  kZoneEraAmerica_Denver /*eras*/,
  nullptr /*targetInfo*/,
};

//---------------------------------------------------------------------------
// Zone name: America/Edmonton
// Eras: 2
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraAmerica_Edmonton[] ACE_TIME_PROGMEM = {
  //             -7:00    Edm    M%sT    1987
  {
    &kZonePolicyEdm /*zonePolicy*/,
    "M%T" /*format*/,
    -28 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -113 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },
  //             -7:00    Canada    M%sT
  {
    &kZonePolicyCanada /*zonePolicy*/,
    "M%T" /*format*/,
    -28 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameAmerica_Edmonton[] ACE_TIME_PROGMEM = "America/Edmonton";

const extended::Info::ZoneInfo kZoneAmerica_Edmonton ACE_TIME_PROGMEM = {
  kZoneNameAmerica_Edmonton /*name*/,
  0x6cb9484a /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  2 /*numEras*/,
  kZoneEraAmerica_Edmonton /*eras*/,
  nullptr /*targetInfo*/,
};

//---------------------------------------------------------------------------
// Zone name: America/Los_Angeles
// Eras: 1
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraAmerica_Los_Angeles[] ACE_TIME_PROGMEM = {
  //             -8:00    US    P%sT
  {
    &kZonePolicyUS /*zonePolicy*/,
    "P%T" /*format*/,
    -32 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameAmerica_Los_Angeles[] ACE_TIME_PROGMEM = "America/Los_Angeles";

const extended::Info::ZoneInfo kZoneAmerica_Los_Angeles ACE_TIME_PROGMEM = {
  kZoneNameAmerica_Los_Angeles /*name*/,
  0xb7f7e8f2 /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  1 /*numEras*/,
  kZoneEraAmerica_Los_Angeles /*eras*/,
  nullptr /*targetInfo*/,
};

//---------------------------------------------------------------------------
// Zone name: America/New_York
// Eras: 1
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraAmerica_New_York[] ACE_TIME_PROGMEM = {
  //             -5:00    US    E%sT
  {
    &kZonePolicyUS /*zonePolicy*/,
    "E%T" /*format*/,
    -20 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameAmerica_New_York[] ACE_TIME_PROGMEM = "America/New_York";

const extended::Info::ZoneInfo kZoneAmerica_New_York ACE_TIME_PROGMEM = {
  kZoneNameAmerica_New_York /*name*/,
  0x1e2a7654 /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  1 /*numEras*/,
  kZoneEraAmerica_New_York /*eras*/,
  nullptr /*targetInfo*/,
};

//---------------------------------------------------------------------------
// Zone name: America/Toronto
// Eras: 1
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraAmerica_Toronto[] ACE_TIME_PROGMEM = {
  //             -5:00    Canada    E%sT
  {
    &kZonePolicyCanada /*zonePolicy*/,
    "E%T" /*format*/,
    -20 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameAmerica_Toronto[] ACE_TIME_PROGMEM = "America/Toronto";

const extended::Info::ZoneInfo kZoneAmerica_Toronto ACE_TIME_PROGMEM = {
  kZoneNameAmerica_Toronto /*name*/,
  0x792e851b /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  1 /*numEras*/,
  kZoneEraAmerica_Toronto /*eras*/,
  nullptr /*targetInfo*/,
};

//---------------------------------------------------------------------------
// Zone name: America/Vancouver
// Eras: 2
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraAmerica_Vancouver[] ACE_TIME_PROGMEM = {
  //             -8:00    Vanc    P%sT    1987
  {
    &kZonePolicyVanc /*zonePolicy*/,
    "P%T" /*format*/,
    -32 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -113 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },
  //             -8:00    Canada    P%sT
  {
    &kZonePolicyCanada /*zonePolicy*/,
    "P%T" /*format*/,
    -32 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameAmerica_Vancouver[] ACE_TIME_PROGMEM = "America/Vancouver";

const extended::Info::ZoneInfo kZoneAmerica_Vancouver ACE_TIME_PROGMEM = {
  kZoneNameAmerica_Vancouver /*name*/,
  0x2c6f6b1f /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  2 /*numEras*/,
  kZoneEraAmerica_Vancouver /*eras*/,
  nullptr /*targetInfo*/,
};

//---------------------------------------------------------------------------
// Zone name: America/Whitehorse
// Eras: 3
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraAmerica_Whitehorse[] ACE_TIME_PROGMEM = {
  //             -8:00    -    PST    1980
  {
    nullptr /*zonePolicy*/,
    "PST" /*format*/,
    -32 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -120 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },
  //             -8:00    Canada    P%sT    2020 Nov  1
  {
    &kZonePolicyCanada /*zonePolicy*/,
    "P%T" /*format*/,
    -32 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -80 /*untilYearTiny*/,
    11 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },
  //             -7:00    -    MST
  {
    nullptr /*zonePolicy*/,
    "MST" /*format*/,
    -28 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameAmerica_Whitehorse[] ACE_TIME_PROGMEM = "America/Whitehorse";

const extended::Info::ZoneInfo kZoneAmerica_Whitehorse ACE_TIME_PROGMEM = {
  kZoneNameAmerica_Whitehorse /*name*/,
  0x54e0e3e8 /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  3 /*numEras*/,
  kZoneEraAmerica_Whitehorse /*eras*/,
  nullptr /*targetInfo*/,
};

//---------------------------------------------------------------------------
// Zone name: America/Winnipeg
// Eras: 2
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraAmerica_Winnipeg[] ACE_TIME_PROGMEM = {
  //             -6:00    Winn    C%sT    2006
  {
    &kZonePolicyWinn /*zonePolicy*/,
    "C%T" /*format*/,
    -24 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -94 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },
  //             -6:00    Canada    C%sT
  {
    &kZonePolicyCanada /*zonePolicy*/,
    "C%T" /*format*/,
    -24 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameAmerica_Winnipeg[] ACE_TIME_PROGMEM = "America/Winnipeg";

const extended::Info::ZoneInfo kZoneAmerica_Winnipeg ACE_TIME_PROGMEM = {
  kZoneNameAmerica_Winnipeg /*name*/,
  0x8c7dafc7 /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  2 /*numEras*/,
  kZoneEraAmerica_Winnipeg /*eras*/,
  nullptr /*targetInfo*/,
};

//---------------------------------------------------------------------------
// Zone name: Australia/Darwin
// Eras: 1
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraAustralia_Darwin[] ACE_TIME_PROGMEM = {
  //              9:30    Aus    AC%sT
  {
    &kZonePolicyAus /*zonePolicy*/,
    "AC%T" /*format*/,
    38 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameAustralia_Darwin[] ACE_TIME_PROGMEM = "Australia/Darwin";

const extended::Info::ZoneInfo kZoneAustralia_Darwin ACE_TIME_PROGMEM = {
  kZoneNameAustralia_Darwin /*name*/,
  0x2876bdff /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  1 /*numEras*/,
  kZoneEraAustralia_Darwin /*eras*/,
  nullptr /*targetInfo*/,
};

//---------------------------------------------------------------------------
// Zone name: Europe/Lisbon
// Eras: 4
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraEurope_Lisbon[] ACE_TIME_PROGMEM = {
  //              0:00    Port    WE%sT    1986
  {
    &kZonePolicyPort /*zonePolicy*/,
    "WE%T" /*format*/,
    0 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -114 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },
  //              0:00    EU    WE%sT    1992 Sep 27  1:00u
  {
    &kZonePolicyEU /*zonePolicy*/,
    "WE%T" /*format*/,
    0 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -108 /*untilYearTiny*/,
    9 /*untilMonth*/,
    27 /*untilDay*/,
    4 /*untilTimeCode*/,
    32 /*untilTimeModifier (kSuffixU + minute=0)*/,
  },
  //              1:00    EU    CE%sT    1996 Mar 31  1:00u
  {
    &kZonePolicyEU /*zonePolicy*/,
    "CE%T" /*format*/,
    4 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -104 /*untilYearTiny*/,
    3 /*untilMonth*/,
    31 /*untilDay*/,
    4 /*untilTimeCode*/,
    32 /*untilTimeModifier (kSuffixU + minute=0)*/,
  },
  //              0:00    EU    WE%sT
  {
    &kZonePolicyEU /*zonePolicy*/,
    "WE%T" /*format*/,
    0 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameEurope_Lisbon[] ACE_TIME_PROGMEM = "Europe/Lisbon";

const extended::Info::ZoneInfo kZoneEurope_Lisbon ACE_TIME_PROGMEM = {
  kZoneNameEurope_Lisbon /*name*/,
  0x5c00a70b /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  4 /*numEras*/,
  kZoneEraEurope_Lisbon /*eras*/,
  nullptr /*targetInfo*/,
};

//---------------------------------------------------------------------------
// Zone name: Pacific/Apia
// Eras: 2
//---------------------------------------------------------------------------

static const extended::Info::ZoneEra kZoneEraPacific_Apia[] ACE_TIME_PROGMEM = {
  //             -11:00    WS    %z    2011 Dec 29 24:00
  {
    &kZonePolicyWS /*zonePolicy*/,
    "" /*format*/,
    -44 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    -89 /*untilYearTiny*/,
    12 /*untilMonth*/,
    29 /*untilDay*/,
    96 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },
  //              13:00    WS    %z
  {
    &kZonePolicyWS /*zonePolicy*/,
    "" /*format*/,
    52 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNamePacific_Apia[] ACE_TIME_PROGMEM = "Pacific/Apia";

const extended::Info::ZoneInfo kZonePacific_Apia ACE_TIME_PROGMEM = {
  kZoneNamePacific_Apia /*name*/,
  0x23359b5e /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  2 /*numEras*/,
  kZoneEraPacific_Apia /*eras*/,
  nullptr /*targetInfo*/,
};



//---------------------------------------------------------------------------
// Links: 1
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Link name: US/Pacific -> America/Los_Angeles
//---------------------------------------------------------------------------

static const char kZoneNameUS_Pacific[] ACE_TIME_PROGMEM = "US/Pacific";

const extended::Info::ZoneInfo kZoneUS_Pacific ACE_TIME_PROGMEM = {
  kZoneNameUS_Pacific /*name*/,
  0xa950f6ab /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  1 /*numEras*/,
  kZoneEraAmerica_Los_Angeles /*eras*/,
  &kZoneAmerica_Los_Angeles /*targetInfo*/,
};


}
}

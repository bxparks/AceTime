/*
 * ZoneInfo data for Europe/Lisbon that covers 1992. Extracted from
 * AceTimeValidation/ExtendedHinnantDateTest/zonedbxhd which contains zoneinfo
 * data from 1974 until 2050.
 */

#include "EuropeLisbon.h"

namespace ace_time {
namespace zonedbxtest {

//---------------------------------------------------------------------------
// Policy name: Port
// Rules: 9
// Memory (8-bit): 87
// Memory (32-bit): 120
//---------------------------------------------------------------------------

const extended::ZoneRule kZoneRulesPort[] ACE_TIME_PROGMEM = {
  // Rule    Port    1951    1965    -    Oct    Sun>=1     2:00s    0    -
  {
    1951 /*fromYear*/,
    1965 /*toYear*/,
    10 /*inMonth*/,
    7 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    16 /*atTimeModifier (kSuffixS + minute=0)*/,
    4 /*deltaCode ((deltaMinutes=0)/15 + 4)*/,
    '-' /*letter*/,
  },
  // Rule    Port    1977    only    -    Mar    27     0:00s    1:00    S
  {
    1977 /*fromYear*/,
    1977 /*toYear*/,
    3 /*inMonth*/,
    0 /*onDayOfWeek*/,
    27 /*onDayOfMonth*/,
    0 /*atTimeCode*/,
    16 /*atTimeModifier (kSuffixS + minute=0)*/,
    8 /*deltaCode ((deltaMinutes=60)/15 + 4)*/,
    'S' /*letter*/,
  },
  // Rule    Port    1977    only    -    Sep    25     0:00s    0    -
  {
    1977 /*fromYear*/,
    1977 /*toYear*/,
    9 /*inMonth*/,
    0 /*onDayOfWeek*/,
    25 /*onDayOfMonth*/,
    0 /*atTimeCode*/,
    16 /*atTimeModifier (kSuffixS + minute=0)*/,
    4 /*deltaCode ((deltaMinutes=0)/15 + 4)*/,
    '-' /*letter*/,
  },
  // Rule    Port    1978    1979    -    Apr    Sun>=1     0:00s    1:00    S
  {
    1978 /*fromYear*/,
    1979 /*toYear*/,
    4 /*inMonth*/,
    7 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    0 /*atTimeCode*/,
    16 /*atTimeModifier (kSuffixS + minute=0)*/,
    8 /*deltaCode ((deltaMinutes=60)/15 + 4)*/,
    'S' /*letter*/,
  },
  // Rule    Port    1978    only    -    Oct     1     0:00s    0    -
  {
    1978 /*fromYear*/,
    1978 /*toYear*/,
    10 /*inMonth*/,
    0 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    0 /*atTimeCode*/,
    16 /*atTimeModifier (kSuffixS + minute=0)*/,
    4 /*deltaCode ((deltaMinutes=0)/15 + 4)*/,
    '-' /*letter*/,
  },
  // Rule    Port    1979    1982    -    Sep    lastSun     1:00s    0    -
  {
    1979 /*fromYear*/,
    1982 /*toYear*/,
    9 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    4 /*atTimeCode*/,
    16 /*atTimeModifier (kSuffixS + minute=0)*/,
    4 /*deltaCode ((deltaMinutes=0)/15 + 4)*/,
    '-' /*letter*/,
  },
  // Rule    Port    1980    only    -    Mar    lastSun     0:00s    1:00    S
  {
    1980 /*fromYear*/,
    1980 /*toYear*/,
    3 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    0 /*atTimeCode*/,
    16 /*atTimeModifier (kSuffixS + minute=0)*/,
    8 /*deltaCode ((deltaMinutes=60)/15 + 4)*/,
    'S' /*letter*/,
  },
  // Rule    Port    1981    1982    -    Mar    lastSun     1:00s    1:00    S
  {
    1981 /*fromYear*/,
    1982 /*toYear*/,
    3 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    4 /*atTimeCode*/,
    16 /*atTimeModifier (kSuffixS + minute=0)*/,
    8 /*deltaCode ((deltaMinutes=60)/15 + 4)*/,
    'S' /*letter*/,
  },
  // Rule    Port    1983    only    -    Mar    lastSun     2:00s    1:00    S
  {
    1983 /*fromYear*/,
    1983 /*toYear*/,
    3 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    16 /*atTimeModifier (kSuffixS + minute=0)*/,
    8 /*deltaCode ((deltaMinutes=60)/15 + 4)*/,
    'S' /*letter*/,
  },

};

const extended::ZonePolicy kZonePolicyPort ACE_TIME_PROGMEM = {
  kZoneRulesPort /*rules*/,
  nullptr /*letters*/,
  9 /*numRules*/,
  0 /*numLetters*/,
};

//---------------------------------------------------------------------------
// Policy name: W_Eur
// Rules: 6
// Memory (8-bit): 60
// Memory (32-bit): 84
//---------------------------------------------------------------------------

static const extended::ZoneRule kZoneRulesW_Eur[] ACE_TIME_PROGMEM = {
  // Anchor: Rule    W-Eur    1977    only    -    Sep    lastSun     1:00s    0    -
  {
    0 /*fromYear*/,
    0 /*toYear*/,
    1 /*inMonth*/,
    0 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    0 /*atTimeCode*/,
    0 /*atTimeModifier (kSuffixW + minute=0)*/,
    4 /*deltaCode ((deltaMinutes=0)/15 + 4)*/,
    '-' /*letter*/,
  },
  // Rule    W-Eur    1977    1980    -    Apr    Sun>=1     1:00s    1:00    S
  {
    1977 /*fromYear*/,
    1980 /*toYear*/,
    4 /*inMonth*/,
    7 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    4 /*atTimeCode*/,
    16 /*atTimeModifier (kSuffixS + minute=0)*/,
    8 /*deltaCode ((deltaMinutes=60)/15 + 4)*/,
    'S' /*letter*/,
  },
  // Rule    W-Eur    1977    only    -    Sep    lastSun     1:00s    0    -
  {
    1977 /*fromYear*/,
    1977 /*toYear*/,
    9 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    4 /*atTimeCode*/,
    16 /*atTimeModifier (kSuffixS + minute=0)*/,
    4 /*deltaCode ((deltaMinutes=0)/15 + 4)*/,
    '-' /*letter*/,
  },
  // Rule    W-Eur    1978    only    -    Oct     1     1:00s    0    -
  {
    1978 /*fromYear*/,
    1978 /*toYear*/,
    10 /*inMonth*/,
    0 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    4 /*atTimeCode*/,
    16 /*atTimeModifier (kSuffixS + minute=0)*/,
    4 /*deltaCode ((deltaMinutes=0)/15 + 4)*/,
    '-' /*letter*/,
  },
  // Rule    W-Eur    1979    1995    -    Sep    lastSun     1:00s    0    -
  {
    1979 /*fromYear*/,
    1995 /*toYear*/,
    9 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    4 /*atTimeCode*/,
    16 /*atTimeModifier (kSuffixS + minute=0)*/,
    4 /*deltaCode ((deltaMinutes=0)/15 + 4)*/,
    '-' /*letter*/,
  },
  // Rule    W-Eur    1981    max    -    Mar    lastSun     1:00s    1:00    S
  {
    1981 /*fromYear*/,
    9999 /*toYear*/,
    3 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    4 /*atTimeCode*/,
    16 /*atTimeModifier (kSuffixS + minute=0)*/,
    8 /*deltaCode ((deltaMinutes=60)/15 + 4)*/,
    'S' /*letter*/,
  },

};



const extended::ZonePolicy kZonePolicyW_Eur ACE_TIME_PROGMEM = {
  kZoneRulesW_Eur /*rules*/,
  nullptr /*letters*/,
  6 /*numRules*/,
  0 /*numLetters*/,
};

//---------------------------------------------------------------------------
// Policy name: EU
// Rules: 7
// Memory (8-bit): 69
// Memory (32-bit): 96
//---------------------------------------------------------------------------

static const extended::ZoneRule kZoneRulesEU[] ACE_TIME_PROGMEM = {
  // Anchor: Rule    EU    1977    only    -    Sep    lastSun     1:00u    0    -
  {
    0 /*fromYear*/,
    0 /*toYear*/,
    1 /*inMonth*/,
    0 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    0 /*atTimeCode*/,
    0 /*atTimeModifier (kSuffixW + minute=0)*/,
    4 /*deltaCode ((deltaMinutes=0)/15 + 4)*/,
    '-' /*letter*/,
  },
  // Rule    EU    1977    1980    -    Apr    Sun>=1     1:00u    1:00    S
  {
    1977 /*fromYear*/,
    1980 /*toYear*/,
    4 /*inMonth*/,
    7 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    4 /*atTimeCode*/,
    32 /*atTimeModifier (kSuffixU + minute=0)*/,
    8 /*deltaCode ((deltaMinutes=60)/15 + 4)*/,
    'S' /*letter*/,
  },
  // Rule    EU    1977    only    -    Sep    lastSun     1:00u    0    -
  {
    1977 /*fromYear*/,
    1977 /*toYear*/,
    9 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    4 /*atTimeCode*/,
    32 /*atTimeModifier (kSuffixU + minute=0)*/,
    4 /*deltaCode ((deltaMinutes=0)/15 + 4)*/,
    '-' /*letter*/,
  },
  // Rule    EU    1978    only    -    Oct     1     1:00u    0    -
  {
    1978 /*fromYear*/,
    1978 /*toYear*/,
    10 /*inMonth*/,
    0 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    4 /*atTimeCode*/,
    32 /*atTimeModifier (kSuffixU + minute=0)*/,
    4 /*deltaCode ((deltaMinutes=0)/15 + 4)*/,
    '-' /*letter*/,
  },
  // Rule    EU    1979    1995    -    Sep    lastSun     1:00u    0    -
  {
    1979 /*fromYear*/,
    1995 /*toYear*/,
    9 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    4 /*atTimeCode*/,
    32 /*atTimeModifier (kSuffixU + minute=0)*/,
    4 /*deltaCode ((deltaMinutes=0)/15 + 4)*/,
    '-' /*letter*/,
  },
  // Rule    EU    1981    max    -    Mar    lastSun     1:00u    1:00    S
  {
    1981 /*fromYear*/,
    9999 /*toYear*/,
    3 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    4 /*atTimeCode*/,
    32 /*atTimeModifier (kSuffixU + minute=0)*/,
    8 /*deltaCode ((deltaMinutes=60)/15 + 4)*/,
    'S' /*letter*/,
  },
  // Rule    EU    1996    max    -    Oct    lastSun     1:00u    0    -
  {
    1996 /*fromYear*/,
    9999 /*toYear*/,
    10 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    4 /*atTimeCode*/,
    32 /*atTimeModifier (kSuffixU + minute=0)*/,
    4 /*deltaCode ((deltaMinutes=0)/15 + 4)*/,
    '-' /*letter*/,
  },

};



const extended::ZonePolicy kZonePolicyEU ACE_TIME_PROGMEM = {
  kZoneRulesEU /*rules*/,
  nullptr /*letters*/,
  7 /*numRules*/,
  0 /*numLetters*/,
};

//---------------------------------------------------------------------------
// ZoneContext (should not be in PROGMEM)
//---------------------------------------------------------------------------

const char kTzDatabaseVersion[] = "2021a";

const char* const kFragments[] = {
/*\x00*/ nullptr,
/*\x01*/ "Africa/",
/*\x02*/ "America/",
/*\x03*/ "Antarctica/",
/*\x04*/ "Argentina/",
/*\x05*/ "Asia/",
/*\x06*/ "Atlantic/",
/*\x07*/ "Australia/",
/*\x08*/ "Brazil/",
/*\x09*/ "Canada/",
/*\x0a*/ "Etc/",
/*\x0b*/ "Europe/",
/*\x0c*/ "Indian/",
/*\x0d*/ "Indiana/",
/*\x0e*/ "Pacific/",

};

const internal::ZoneContext kZoneContext = {
  1974 /*startYear*/,
  2050 /*untilYear*/,
  kTzDatabaseVersion /*tzVersion*/,
  15 /*numFragments*/,
  kFragments /*fragments*/,
};

//---------------------------------------------------------------------------
// Zone name: Europe/Lisbon
// Zone Eras: 5
// Strings (bytes): 32 (originally 38)
// Memory (8-bit): 98
// Memory (32-bit): 132
//---------------------------------------------------------------------------

static const extended::ZoneEra kZoneEraEurope_Lisbon[] ACE_TIME_PROGMEM = {
  //              1:00    -    CET    1976 Sep 26  1:00
  {
    nullptr /*zonePolicy*/,
    "CET" /*format*/,
    4 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    1976 /*untilYear*/,
    9 /*untilMonth*/,
    26 /*untilDay*/,
    4 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },
  //              0:00    Port    WE%sT    1983 Sep 25  1:00s
  {
    &kZonePolicyPort /*zonePolicy*/,
    "WE%T" /*format*/,
    0 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    1983 /*untilYear*/,
    9 /*untilMonth*/,
    25 /*untilDay*/,
    4 /*untilTimeCode*/,
    16 /*untilTimeModifier (kSuffixS + minute=0)*/,
  },
  //              0:00    W-Eur    WE%sT    1992 Sep 27  1:00s
  {
    &kZonePolicyW_Eur /*zonePolicy*/,
    "WE%T" /*format*/,
    0 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    1992 /*untilYear*/,
    9 /*untilMonth*/,
    27 /*untilDay*/,
    4 /*untilTimeCode*/,
    16 /*untilTimeModifier (kSuffixS + minute=0)*/,
  },
  //              1:00    EU    CE%sT    1996 Mar 31  1:00u
  {
    &kZonePolicyEU /*zonePolicy*/,
    "CE%T" /*format*/,
    4 /*offsetCode*/,
    4 /*deltaCode (((offsetMinute=0) << 4) + ((deltaMinutes=0)/15 + 4))*/,
    1996 /*untilYear*/,
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
    10000 /*untilYear*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    0 /*untilTimeModifier (kSuffixW + minute=0)*/,
  },

};

static const char kZoneNameEurope_Lisbon[] ACE_TIME_PROGMEM = "\x0b" "Lisbon";

const extended::ZoneInfo kZoneEurope_Lisbon ACE_TIME_PROGMEM = {
  kZoneNameEurope_Lisbon /*name*/,
  0x5c00a70b /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  5 /*numEras*/,
  kZoneEraEurope_Lisbon /*eras*/,
};

}
}

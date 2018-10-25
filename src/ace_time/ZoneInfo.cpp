#include "ZoneRule.h"
#include "ZoneInfo.h"

namespace ace_time {

//---------------------------------------------------------------------------
// America/Los_Angeles
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntryLosAngeles[] = {
  {
    -32 /*offsetCode*/,
    &ZoneRules::kUS /*zone*/,
    "P%sT" /*format*/,
    255 /*untilYear*/,
  }
};

const ZoneInfo ZoneInfo::kLosAngeles = {
  "America/Los_Angeles" /*name*/,
  kZoneInfoEntryLosAngeles /*entries*/,
  sizeof(kZoneInfoEntryLosAngeles)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Toronto
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntryToronto[] = {
  {
    -20 /*offsetCode*/,
    &ZoneRules::kUS /*zone*/,
    "E%sT" /*format*/,
    255 /*fromYear*/,
  }
};

const ZoneInfo ZoneInfo::kToronto = {
  "America/Toronto" /*name*/,
  kZoneInfoEntryToronto /*zoneRules*/,
  sizeof(kZoneInfoEntryToronto)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Indiana/Indianapolis
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntryIndianapolis[] = {
  {
    -20 /*offsetCode*/,
    nullptr /*zoneRules*/,
    "EST" /*format*/,
    6 /*untilYear*/,
  },
  {
    -20 /*offsetCode*/,
    &ZoneRules::kUS /*zone*/,
    "E%sT" /*format*/,
    255 /*untilYear*/,
  },
};
const ZoneInfo ZoneInfo::kIndianapolis = {
  "America/Indiana/Indianapolis" /*name*/,
  kZoneInfoEntryIndianapolis /*zoneRules*/,
  sizeof(kZoneInfoEntryIndianapolis)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Indiana/Petersburg
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntryPetersburg[] = {
  {
    -20 /*offsetCode*/,
    nullptr /*zone*/,
    "EST" /*format*/,
    6 /*untilYear*/,
  },
  {
    -24 /*offsetCode*/,
    &ZoneRules::kUS /*zone*/,
    "C%sT" /*format*/,
    7 /*untilYear*/,
  },
  {
    -20 /*offsetCode*/,
    &ZoneRules::kUS /*zone*/,
    "E%sT" /*format*/,
    255 /*untilYear*/,
  },
};

const ZoneInfo ZoneInfo::kPetersburg = {
  "America/Indiana/Petersburg" /*name*/,
  kZoneInfoEntryPetersburg /*zoneRules*/,
  sizeof(kZoneInfoEntryPetersburg)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// Europe/London
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntryLondon[] = {
  {
    0 /*offsetCode*/,
    &ZoneRules::kEU /*zone*/,
    "GMT/BST" /*format*/,
    255 /*untilYear*/,
  }
};

const ZoneInfo ZoneInfo::kLondon = {
  "Europe/London" /*name*/,
  kZoneInfoEntryLondon /*zoneRules*/,
  sizeof(kZoneInfoEntryLondon)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

}

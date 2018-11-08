#include "../ZoneRule.h"
#include "../ZoneInfo.h"
#include "zone_policies.h"
#include "zone_infos.h"

namespace ace_time {
namespace zonedb {

//---------------------------------------------------------------------------
// America/Los_Angeles
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntryLosAngeles[] = {
  // -8:00   US      P%sT
  {
    -32 /*offsetCode*/,
    &kPolicyUS /*zonePolicy*/,
    "P%T" /*format*/,
    255 /*untilYear*/,
  }
};

ZoneInfo const kLosAngeles = {
  "America/Los_Angeles" /*name*/,
  kZoneInfoEntryLosAngeles /*entries*/,
  sizeof(kZoneInfoEntryLosAngeles)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Denver
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntryDenver[] = {
  // -7:00   US      M%sT
  {
    -24 /*offsetCode*/,
    &kPolicyUS /*zonePolicy*/,
    "M%T" /*format*/,
    255 /*untilYear*/,
  }
};

ZoneInfo const kDenver = {
  "America/Denver" /*name*/,
  kZoneInfoEntryDenver /*entries*/,
  sizeof(kZoneInfoEntryDenver)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Chicago
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntryChicago[] = {
  // -6:00   US      C%sT
  {
    -24 /*offsetCode*/,
    &kPolicyUS /*zonePolicy*/,
    "C%T" /*format*/,
    255 /*untilYear*/,
  }
};

ZoneInfo const kChicago = {
  "America/Chicago" /*name*/,
  kZoneInfoEntryChicago /*entries*/,
  sizeof(kZoneInfoEntryChicago)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/New_York
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntryNewYork[] = {
  // -5:00 US  E%sT
  {
    -20 /*offsetCode*/,
    &kPolicyUS /*zonePolicy*/,
    "E%T" /*format*/,
    255 /*untilYear*/,
  }
};

ZoneInfo const kNewYork = {
  "America/New_York" /*name*/,
  kZoneInfoEntryNewYork /*entries*/,
  sizeof(kZoneInfoEntryNewYork)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Toronto
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntryToronto[] = {
  // -5:00   Canada  E%sT
  {
    -20 /*offsetCode*/,
    &kPolicyUS /*zonePolicy*/,
    "E%T" /*format*/,
    255 /*fromYear*/,
  }
};

ZoneInfo const kToronto = {
  "America/Toronto" /*name*/,
  kZoneInfoEntryToronto /*zoneRules*/,
  sizeof(kZoneInfoEntryToronto)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Indiana/Indianapolis
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntryIndianapolis[] = {
  // -5:00   -       EST     2006
  {
    -20 /*offsetCode*/,
    nullptr /*zoneRules*/,
    "EST" /*format*/,
    6 /*untilYear*/,
  },
  // -5:00   US      E%sT
  {
    -20 /*offsetCode*/,
    &kPolicyUS /*zonePolicy*/,
    "E%T" /*format*/,
    255 /*untilYear*/,
  },
};
ZoneInfo const kIndianapolis = {
  "America/Indiana/Indianapolis" /*name*/,
  kZoneInfoEntryIndianapolis /*zoneRules*/,
  sizeof(kZoneInfoEntryIndianapolis)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Indiana/Petersburg
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntryPetersburg[] = {
  // -5:00   -       EST     2006 Apr  2  2:00
  {
    -20 /*offsetCode*/,
    nullptr /*zonePolicy*/,
    "EST" /*format*/,
    6 /*untilYear*/,
  },
  // -6:00   US      C%sT    2007 Nov  4  2:00
  {
    -24 /*offsetCode*/,
    &kPolicyUS /*zonePolicy*/,
    "C%T" /*format*/,
    7 /*untilYear*/,
  },
  // -5:00   US      E%sT
  {
    -20 /*offsetCode*/,
    &kPolicyUS /*zonePolicy*/,
    "E%T" /*format*/,
    255 /*untilYear*/,
  },
};

ZoneInfo const kPetersburg = {
  "America/Indiana/Petersburg" /*name*/,
  kZoneInfoEntryPetersburg /*zoneRules*/,
  sizeof(kZoneInfoEntryPetersburg)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// Europe/London
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntryLondon[] = {
  // 0:00   EU      GMT/BST
  {
    0 /*offsetCode*/,
    &kPolicyEU /*zonePolicy*/,
    "GMT/BST" /*format*/,
    255 /*untilYear*/,
  }
};

ZoneInfo const kLondon = {
  "Europe/London" /*name*/,
  kZoneInfoEntryLondon /*zoneRules*/,
  sizeof(kZoneInfoEntryLondon)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// Australia/Sydney
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntrySydney[] = {
  // 10:00   AN      AE%sT
  {
    40 /*offsetCode*/,
    &kPolicyAN /*zonePolicy*/,
    "AE%T" /*format*/,
    255 /*untilYear*/,
  }
};

ZoneInfo const kSydney = {
  "Australia/Sydney" /*name*/,
  kZoneInfoEntrySydney /*zoneRules*/,
  sizeof(kZoneInfoEntrySydney)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// Africa/Johannesburg
//---------------------------------------------------------------------------

static ZoneInfoEntry const kZoneInfoEntryJohannesburg[] = {
  // 2:00    SA      SAST
  {
    8 /*offsetCode*/,
    nullptr /*zonePolicy*/,
    "SAST" /*format*/,
    255 /*untilYear*/,
  }
};

ZoneInfo const kJohannesburg = {
  "Africa/Johannesburg" /*name*/,
  kZoneInfoEntryJohannesburg /*zoneRules*/,
  sizeof(kZoneInfoEntryJohannesburg)/sizeof(ZoneInfoEntry) /*numEntries*/,
};

}
}

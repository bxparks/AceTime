#include "../ZoneRule.h"
#include "../ZoneInfo.h"
#include "zone_policies.h"
#include "zone_infos.h"

namespace ace_time {
namespace zonedb {

//---------------------------------------------------------------------------
// America/Los_Angeles
//---------------------------------------------------------------------------

static ZoneEntry const kZoneEntryLosAngeles[] = {
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
  kZoneEntryLosAngeles /*entries*/,
  sizeof(kZoneEntryLosAngeles)/sizeof(ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Denver
//---------------------------------------------------------------------------

static ZoneEntry const kZoneEntryDenver[] = {
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
  kZoneEntryDenver /*entries*/,
  sizeof(kZoneEntryDenver)/sizeof(ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Chicago
//---------------------------------------------------------------------------

static ZoneEntry const kZoneEntryChicago[] = {
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
  kZoneEntryChicago /*entries*/,
  sizeof(kZoneEntryChicago)/sizeof(ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/New_York
//---------------------------------------------------------------------------

static ZoneEntry const kZoneEntryNewYork[] = {
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
  kZoneEntryNewYork /*entries*/,
  sizeof(kZoneEntryNewYork)/sizeof(ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Toronto
//---------------------------------------------------------------------------

static ZoneEntry const kZoneEntryToronto[] = {
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
  kZoneEntryToronto /*zoneRules*/,
  sizeof(kZoneEntryToronto)/sizeof(ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Indiana/Indianapolis
//---------------------------------------------------------------------------

static ZoneEntry const kZoneEntryIndianapolis[] = {
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
  kZoneEntryIndianapolis /*zoneRules*/,
  sizeof(kZoneEntryIndianapolis)/sizeof(ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Indiana/Petersburg
//---------------------------------------------------------------------------

static ZoneEntry const kZoneEntryPetersburg[] = {
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
  kZoneEntryPetersburg /*zoneRules*/,
  sizeof(kZoneEntryPetersburg)/sizeof(ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// Europe/London
//---------------------------------------------------------------------------

static ZoneEntry const kZoneEntryLondon[] = {
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
  kZoneEntryLondon /*zoneRules*/,
  sizeof(kZoneEntryLondon)/sizeof(ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// Australia/Sydney
//---------------------------------------------------------------------------

static ZoneEntry const kZoneEntrySydney[] = {
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
  kZoneEntrySydney /*zoneRules*/,
  sizeof(kZoneEntrySydney)/sizeof(ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// Australia/Darwin
//---------------------------------------------------------------------------

static ZoneEntry const kZoneEntryDarwin[] = {
  // 9:30   Aus     AC%sT
  {
    38 /*offsetCode*/,
    &kPolicyAus /*zonePolicy*/,
    "AC%T" /*format*/,
    255 /*untilYear*/,
  }
};

ZoneInfo const kDarwin = {
  "Australia/Darwin" /*name*/,
  kZoneEntryDarwin /*zoneRules*/,
  sizeof(kZoneEntryDarwin)/sizeof(ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// Africa/Johannesburg
//---------------------------------------------------------------------------

static ZoneEntry const kZoneEntryJohannesburg[] = {
  // 2:00    SA      SAST
  {
    8 /*offsetCode*/,
    &kPolicySA /*zonePolicy*/,
    "SAST" /*format*/,
    255 /*untilYear*/,
  }
};

ZoneInfo const kJohannesburg = {
  "Africa/Johannesburg" /*name*/,
  kZoneEntryJohannesburg /*zoneRules*/,
  sizeof(kZoneEntryJohannesburg)/sizeof(ZoneEntry) /*numEntries*/,
};

}
}

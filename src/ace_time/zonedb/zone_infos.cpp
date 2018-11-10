#include "../common/ZoneInfo.h"
#include "zone_policies.h"
#include "zone_infos.h"

namespace ace_time {
namespace zonedb {

//---------------------------------------------------------------------------
// America/Los_Angeles
//---------------------------------------------------------------------------

static common::ZoneEntry const kZoneEntryLosAngeles[] = {
  // -8:00   US      P%sT
  {
    -32 /*offsetCode*/,
    &kPolicyUS /*zonePolicy*/,
    "P%T" /*format*/,
    255 /*untilYear*/,
  }
};

common::ZoneInfo const kLosAngeles = {
  "America/Los_Angeles" /*name*/,
  kZoneEntryLosAngeles /*entries*/,
  sizeof(kZoneEntryLosAngeles)/sizeof(common::ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Denver
//---------------------------------------------------------------------------

static common::ZoneEntry const kZoneEntryDenver[] = {
  // -7:00   US      M%sT
  {
    -24 /*offsetCode*/,
    &kPolicyUS /*zonePolicy*/,
    "M%T" /*format*/,
    255 /*untilYear*/,
  }
};

common::ZoneInfo const kDenver = {
  "America/Denver" /*name*/,
  kZoneEntryDenver /*entries*/,
  sizeof(kZoneEntryDenver)/sizeof(common::ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Chicago
//---------------------------------------------------------------------------

static common::ZoneEntry const kZoneEntryChicago[] = {
  // -6:00   US      C%sT
  {
    -24 /*offsetCode*/,
    &kPolicyUS /*zonePolicy*/,
    "C%T" /*format*/,
    255 /*untilYear*/,
  }
};

common::ZoneInfo const kChicago = {
  "America/Chicago" /*name*/,
  kZoneEntryChicago /*entries*/,
  sizeof(kZoneEntryChicago)/sizeof(common::ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/New_York
//---------------------------------------------------------------------------

static common::ZoneEntry const kZoneEntryNewYork[] = {
  // -5:00 US  E%sT
  {
    -20 /*offsetCode*/,
    &kPolicyUS /*zonePolicy*/,
    "E%T" /*format*/,
    255 /*untilYear*/,
  }
};

common::ZoneInfo const kNewYork = {
  "America/New_York" /*name*/,
  kZoneEntryNewYork /*entries*/,
  sizeof(kZoneEntryNewYork)/sizeof(common::ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Toronto
//---------------------------------------------------------------------------

static common::ZoneEntry const kZoneEntryToronto[] = {
  // -5:00   Canada  E%sT
  {
    -20 /*offsetCode*/,
    &kPolicyUS /*zonePolicy*/,
    "E%T" /*format*/,
    255 /*fromYear*/,
  }
};

common::ZoneInfo const kToronto = {
  "America/Toronto" /*name*/,
  kZoneEntryToronto /*zoneRules*/,
  sizeof(kZoneEntryToronto)/sizeof(common::ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Indiana/Indianapolis
//---------------------------------------------------------------------------

static common::ZoneEntry const kZoneEntryIndianapolis[] = {
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
common::ZoneInfo const kIndianapolis = {
  "America/Indiana/Indianapolis" /*name*/,
  kZoneEntryIndianapolis /*zoneRules*/,
  sizeof(kZoneEntryIndianapolis)/sizeof(common::ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// America/Indiana/Petersburg
//---------------------------------------------------------------------------

static common::ZoneEntry const kZoneEntryPetersburg[] = {
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

common::ZoneInfo const kPetersburg = {
  "America/Indiana/Petersburg" /*name*/,
  kZoneEntryPetersburg /*zoneRules*/,
  sizeof(kZoneEntryPetersburg)/sizeof(common::ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// Europe/London
//---------------------------------------------------------------------------

static common::ZoneEntry const kZoneEntryLondon[] = {
  // 0:00   EU      GMT/BST
  {
    0 /*offsetCode*/,
    &kPolicyEU /*zonePolicy*/,
    "GMT/BST" /*format*/,
    255 /*untilYear*/,
  }
};

common::ZoneInfo const kLondon = {
  "Europe/London" /*name*/,
  kZoneEntryLondon /*zoneRules*/,
  sizeof(kZoneEntryLondon)/sizeof(common::ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// Australia/Sydney
//---------------------------------------------------------------------------

static common::ZoneEntry const kZoneEntrySydney[] = {
  // 10:00   AN      AE%sT
  {
    40 /*offsetCode*/,
    &kPolicyAN /*zonePolicy*/,
    "AE%T" /*format*/,
    255 /*untilYear*/,
  }
};

common::ZoneInfo const kSydney = {
  "Australia/Sydney" /*name*/,
  kZoneEntrySydney /*zoneRules*/,
  sizeof(kZoneEntrySydney)/sizeof(common::ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// Australia/Darwin
//---------------------------------------------------------------------------

static common::ZoneEntry const kZoneEntryDarwin[] = {
  // 9:30   Aus     AC%sT
  {
    38 /*offsetCode*/,
    &kPolicyAus /*zonePolicy*/,
    "AC%T" /*format*/,
    255 /*untilYear*/,
  }
};

common::ZoneInfo const kDarwin = {
  "Australia/Darwin" /*name*/,
  kZoneEntryDarwin /*zoneRules*/,
  sizeof(kZoneEntryDarwin)/sizeof(common::ZoneEntry) /*numEntries*/,
};

//---------------------------------------------------------------------------
// Africa/Johannesburg
//---------------------------------------------------------------------------

static common::ZoneEntry const kZoneEntryJohannesburg[] = {
  // 2:00    SA      SAST
  {
    8 /*offsetCode*/,
    &kPolicySA /*zonePolicy*/,
    "SAST" /*format*/,
    255 /*untilYear*/,
  }
};

common::ZoneInfo const kJohannesburg = {
  "Africa/Johannesburg" /*name*/,
  kZoneEntryJohannesburg /*zoneRules*/,
  sizeof(kZoneEntryJohannesburg)/sizeof(common::ZoneEntry) /*numEntries*/,
};

}
}

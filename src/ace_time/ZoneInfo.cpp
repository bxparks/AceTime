#include "ZoneRule.h"
#include "ZoneInfo.h"

namespace ace_time {

static const char kZonePst[] = "P%sT"; // PST
static const char kZoneCst[] = "C%sT"; // CST
static const char kZoneMst[] = "M%sT"; // MST
static const char kZoneEst[] = "E%sT"; // EST

static const char kZoneIdLosAngeles[] = "America/Los_Angeles";
static const char kZoneIdToronto[] = "America/Toronto";
static const char kZoneIdNewYork[] = "America/New_York";

const ZoneInfo ZoneInfo::kZoneInfoLosAngeles = {
  kZoneIdLosAngeles,
  -32 /*offsetCode*/,
  ZoneRule::kUsRules,
  2 /*numRules*/,
  kZonePst
};

const ZoneInfo ZoneInfo::kZoneInfoToronto = {
  kZoneIdToronto,
  -20 /*offsetCode*/,
  ZoneRule::kUsRules,
  2 /*numRules*/,
  kZoneEst
};

}

#include "../ZonePolicy.h"
#include "zone_policies.h"

namespace ace_time {
namespace zonedb {

//---------------------------------------------------------------------------
// US Rules
//---------------------------------------------------------------------------

static const ZoneRule kZoneRulesUS[] = {
  // Rule	US	1967	2006	-	Oct	lastSun	2:00	0	S
  {
    1967 /*fromYearFull*/,
    2006 /*toYearFull*/,
    10 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    0 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    'w' /*atHourModifier*/,
    0 /*deltaCode*/,
    'S' /*letter*/
  },

  // Rule	US	1987	2006	-	Apr	Sun>=1	2:00	1:00	D
  {
    1987 /*fromYearFull*/,
    2006 /*toYearFull*/,
    4 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    1 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    'w' /*atHourModifier*/,
    4 /*deltaCode*/,
    'D' /*letter*/
  },

  // Rule	US	2007	max	-	Mar	Sun>=8	2:00	1:00	D
  {
    2007 /*fromYearFull*/,
    ZoneRule::kMaxYear /*toYearFull*/,
    3 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    8 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    'w' /*atHourModifier*/,
    4 /*deltaCode*/,
    'D' /*letter*/
  },

  // Rule	US	2007	max	-	Nov	Sun>=1	2:00	0	S
  {
    2007 /*fromYearFull*/,
    ZoneRule::kMaxYear /*toYearFull*/,
    11 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    1 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    'w' /*atHourModifier*/,
    0 /*deltaCode*/,
    'S' /*letter*/
  },
};

const ZonePolicy kPolicyUS = {
  sizeof(kZoneRulesUS) / sizeof(ZoneRule) /*numRules*/,
  kZoneRulesUS /*rules*/,
};

//---------------------------------------------------------------------------
// EU rules
//---------------------------------------------------------------------------

static const ZoneRule kZoneRulesEU[] = {
  // Rule	EU	1981	max	-	Mar	lastSun	 1:00u	1:00	S
  {
    1981 /*fromYearFull*/,
    ZoneRule::kMaxYear /*toYearFull*/,
    3 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    0 /*onDayOfMonth*/,
    1 /*atHour=2:00*/,
    'u' /*atHourModifier*/,
    4 /*deltaCode*/,
    'S' /*letter*/
  },
  // Rule	EU	1996	max	-	Oct	lastSun	 1:00u	0	-
  {
    1996 /*fromYearFull*/,
    ZoneRule::kMaxYear /*toYearFull*/,
    10 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    0 /*onDayOfMonth*/,
    1 /*atHour=2:00*/,
    'u' /*atHourModifier*/,
    0 /*deltaCode*/,
    '-' /*letter*/
  },
};

const ZonePolicy kPolicyEU = {
  sizeof(kZoneRulesEU) / sizeof(ZoneRule) /*numRules*/,
  kZoneRulesEU /*rules*/,
};

//---------------------------------------------------------------------------
// Australia New South Wales AN rules
//---------------------------------------------------------------------------

static const ZoneRule kZoneRulesAN[] = {
  // Rule    AN      1996    2005    -       Mar     lastSun 2:00s   0       S
  {
    1996 /*fromYearFull*/,
    2005 /*toYearFull*/,
    3 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    0 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    's' /*atHourModifier*/,
    0 /*deltaCode*/,
    'S' /*letter*/
  },
  // Rule    AN      2000    only    -       Aug     lastSun 2:00s   1:00    D
  {
    2000 /*fromYearFull*/,
    2000 /*toYearFull*/,
    8 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    0 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    's' /*atHourModifier*/,
    4 /*deltaCode*/,
    'D' /*letter*/
  },
  // Rule    AN      2001    2007    -       Oct     lastSun 2:00s   1:00    D
  {
    2001 /*fromYearFull*/,
    2007 /*toYearFull*/,
    10 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    0 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    's' /*atHourModifier*/,
    4 /*deltaCode*/,
    'D' /*letter*/
  },
  // Rule    AN      2006    only    -       Apr     Sun>=1  2:00s   0       S
  {
    2006 /*fromYearFull*/,
    2006 /*toYearFull*/,
    4 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    1 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    's' /*atHourModifier*/,
    0 /*deltaCode*/,
    'S' /*letter*/
  },
  // Rule    AN      2007    only    -       Mar     lastSun 2:00s   0       S
  {
    2007 /*fromYearFull*/,
    2007 /*toYearFull*/,
    3 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    0 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    's' /*atHourModifier*/,
    0 /*deltaCode*/,
    'S' /*letter*/
  },
  // Rule    AN      2008    max     -       Apr     Sun>=1  2:00s   0       S
  {
    2008 /*fromYearFull*/,
    ZoneRule::kMaxYear /*toYearFull*/,
    4 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    1 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    's' /*atHourModifier*/,
    0 /*deltaCode*/,
    'S' /*letter*/
  },
  // Rule    AN      2008    max     -       Oct     Sun>=1  2:00s   1:00    D
  {
    2008 /*fromYearFull*/,
    ZoneRule::kMaxYear /*toYearFull*/,
    10 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    1 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    's' /*atHourModifier*/,
    4 /*deltaCode*/,
    'D' /*letter*/
  },
};

const ZonePolicy kPolicyAN = {
  sizeof(kZoneRulesAN) / sizeof(ZoneRule) /*numRules*/,
  kZoneRulesAN /*rules*/,
};

//---------------------------------------------------------------------------
// Australia Aus rules
//---------------------------------------------------------------------------

static const ZoneRule kZoneRulesAus[] = {
  // Rule    Aus     1943    1944    -       Mar     lastSun 2:00    0       S
  {
    1943 /*fromYearFull*/,
    1944 /*toYearFull*/,
    3 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    0 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    'w' /*atHourModifier*/,
    0 /*deltaCode*/,
    'S' /*letter*/
  },
};

const ZonePolicy kPolicyAus = {
  sizeof(kZoneRulesAus) / sizeof(ZoneRule) /*numRules*/,
  kZoneRulesAus /*rules*/,
};


//---------------------------------------------------------------------------
// South Africa SA rules
//---------------------------------------------------------------------------

static const ZoneRule kZoneRulesSA[] = {
  // Rule    SA      1943    1944    -       Mar     Sun>=15 2:00    0       -
  {
    1943 /*fromYearFull*/,
    1944 /*toYearFull*/,
    3 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    15 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    'w' /*atHourModifier*/,
    0 /*deltaCode*/,
    '-' /*letter*/
  },
};

const ZonePolicy kPolicySA = {
  sizeof(kZoneRulesSA) / sizeof(ZoneRule) /*numRules*/,
  kZoneRulesSA /*rules*/,
};

}
}

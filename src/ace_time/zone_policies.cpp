#include "ZoneRule.h"
#include "ZoneInfo.h"
#include "zone_policies.h"

namespace ace_time {
namespace zonedb {

//---------------------------------------------------------------------------
// US Rules
//---------------------------------------------------------------------------

static const ZoneRule kZoneRulesUS[] = {
  // Rule	US	1967	2006	-	Oct	lastSun	2:00	0	S
  {
    0 /*fromYear*/,
    6 /*toYear*/,
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
    0 /*fromYear*/,
    6 /*toYear*/,
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
    7 /*fromYear*/,
    255 /*toYear*/,
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
    7 /*fromYear*/,
    255 /*toYear*/,
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
    0 /*fromYear*/,
    255 /*toYear*/,
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
    0 /*fromYear*/,
    255 /*toYear*/,
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
// Australia AN rules
//---------------------------------------------------------------------------

static const ZoneRule kZoneRulesAN[] = {
  // Rule    AN      2000    only    -       Aug     lastSun 2:00s   1:00    D
  {
    0 /*fromYear*/,
    0 /*toYear*/,
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
    1 /*fromYear*/,
    7 /*toYear*/,
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
    6 /*fromYear*/,
    6 /*toYear*/,
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
    7 /*fromYear*/,
    7 /*toYear*/,
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
    8 /*fromYear*/,
    255 /*toYear*/,
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
    8 /*fromYear*/,
    255 /*toYear*/,
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

}
}

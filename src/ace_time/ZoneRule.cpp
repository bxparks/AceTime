#include "ZoneRule.h"
#include "ZoneInfo.h"

namespace ace_time {

//---------------------------------------------------------------------------
// US Rules
//---------------------------------------------------------------------------

static const ZoneRule kZoneRulesUS[] = {
  // Rule	US	1967	2006	-	Oct	lastSun	2:00	0	S
  {
    0 /*fromYear*/,
    6 /*toYear*/,
    10 /*inMonth*/,
    1 /*onDayOfWeek=Sunday*/,
    24 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    0 /*atHourModifier=wall*/,
    0 /*offsetHour*/,
    'S' /*letter*/
  },

  // Rule	US	1987	2006	-	Apr	Sun>=1	2:00	1:00	D
  {
    0 /*fromYear*/,
    6 /*toYear*/,
    4 /*inMonth*/,
    1 /*onDayOfWeek=Sunday*/,
    1 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    0 /*atHourModifier=wall*/,
    1 /*offsetHour*/,
    'D' /*letter*/
  },

  // Rule	US	2007	max	-	Mar	Sun>=8	2:00	1:00	D
  {
    7 /*fromYear*/,
    255 /*toYear*/,
    3 /*inMonth*/,
    1 /*onDayOfWeek=Sunday*/,
    8 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    0 /*atHourModifier=wall*/,
    1 /*offsetHour*/,
    'D' /*letter*/
  },

  // Rule	US	2007	max	-	Nov	Sun>=1	2:00	0	S
  {
    7 /*fromYear*/,
    255 /*toYear*/,
    11 /*inMonth*/,
    1 /*onDayOfWeek=Sunday*/,
    1 /*onDayOfMonth*/,
    2 /*atHour=2:00*/,
    0 /*atHourModifier=wall*/,
    0 /*offsetHour*/,
    'S' /*letter*/
  },
};

const ZoneRules ZoneRules::kUS = {
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
    24 /*onDayOfMonth*/,
    1 /*atHour=2:00*/,
    2 /*atHourModifier=u*/,
    1 /*offsetHour*/,
    'S' /*letter*/
  },
  // Rule	EU	1996	max	-	Oct	lastSun	 1:00u	0	-
  {
    0 /*fromYear*/,
    255 /*toYear*/,
    10 /*inMonth*/,
    7 /*onDayOfWeek=Sunday*/,
    25 /*onDayOfMonth*/,
    1 /*atHour=2:00*/,
    2 /*atHourModifier=u*/,
    0 /*offsetHour*/,
    '-' /*letter*/
  },
};

const ZoneRules ZoneRules::kEU = {
  sizeof(kZoneRulesEU) / sizeof(ZoneRule) /*numRules*/,
  kZoneRulesEU /*rules*/,
};

}

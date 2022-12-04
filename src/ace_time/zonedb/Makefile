TARGETS := zone_infos.cpp zone_infos.h zone_policies.cpp zone_policies.h

TOOLS := $(abspath ../../../../AceTimeTools)

TZ_VERSION := 2022g
START_YEAR := 2000
UNTIL_YEAR := 10000

all:
	$(TOOLS)/tzcompiler.sh \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language arduino \
		--scope basic \
		--generate_int16_years \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

zonedb.json:
	$(TOOLS)/tzcompiler.sh \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language json \
		--scope basic \
		--generate_int16_years \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

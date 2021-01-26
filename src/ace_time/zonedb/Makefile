TARGETS := zone_infos.cpp zone_infos.h zone_policies.cpp zone_policies.h

TZ_VERSION = 2021a
START_YEAR = 2000
UNTIL_YEAR = 2050

all:
	../../../tools/tzcompiler.sh \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language arduino \
		--scope basic \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

zonedb.json:
	../../../tools/tzcompiler.sh \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language json \
		--scope basic \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

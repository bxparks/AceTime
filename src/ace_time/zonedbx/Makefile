TARGETS := zone_infos.cpp zone_infos.h zone_policies.cpp zone_policies.h

TZ_VERSION = 2020d
START_YEAR = 2000
UNTIL_YEAR = 2050

all:
	../../../tools/tzcompiler.sh \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language arduino \
		--scope extended \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

tzdb.json:
	../../../tools/tzcompiler.sh \
		--tag $(TZ_VERSION) \
		--action tzdb \
		--scope extended \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

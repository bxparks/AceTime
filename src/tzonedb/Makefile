TARGETS := zone_infos.cpp zone_infos.h zone_policies.cpp zone_policies.h

TOOLS := $(abspath ../../../../../AceTimeTools)
TZ_REPO := $(abspath $(TOOLS)/../tz)
TZ_VERSION := 2022g
START_YEAR := 1980
UNTIL_YEAR := 10000

all:
	$(TOOLS)/tzcompiler.sh \
		--tzrepo $(TZ_REPO) \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language arduino \
		--scope basic \
		--db_namespace tzonedb \
		--generate_int16_years \
		--include_list include_list.txt \
		--nocompress \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

clean:
	rm -rf tzfiles

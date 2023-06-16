TARGETS := zone_infos.cpp zone_infos.h zone_policies.cpp zone_policies.h

TOOLS := $(abspath ../../../AceTimeTools)
TZ_REPO := $(abspath $(TOOLS)/../tz)
TZ_VERSION := 2023c
START_YEAR := 2000
UNTIL_YEAR := 2200

all:
	$(TOOLS)/tzcompiler.sh \
		--tzrepo $(TZ_REPO) \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language arduino \
		--scope basic \
		--db_namespace zonedb \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

zonedb.json:
	$(TOOLS)/tzcompiler.sh \
		--tzrepo $(TZ_REPO) \
		--tag $(TZ_VERSION) \
		--action json \
		--language arduino \
		--scope basic \
		--db_namespace zonedb \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

clean:
	rm -rf tzfiles zonedb.json *.o

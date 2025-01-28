TARGETS := zone_infos.cpp zone_infos.h zone_policies.cpp zone_policies.h

COMPILER := $(abspath ../../../../compiler)
TZ_REPO := $(abspath ../../../../../tz)
TZ_VERSION := 2025a
START_YEAR := 1800
UNTIL_YEAR := 2200

all:
	$(COMPILER)/tzcompiler.sh \
		--tzrepo $(TZ_REPO) \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language arduino \
		--scope complete \
		--db_namespace zonedbc \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

zonedb.json:
	$(COMPILER)/tzcompiler.sh \
		--tzrepo $(TZ_REPO) \
		--tag $(TZ_VERSION) \
		--action json \
		--language arduino \
		--scope complete \
		--db_namespace zonedbc \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

clean:
	rm -rf tzfiles zonedb.json *.o

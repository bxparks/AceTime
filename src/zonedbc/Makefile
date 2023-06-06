TARGETS := zone_infos.cpp zone_infos.h zone_policies.cpp zone_policies.h

TOOLS := $(abspath ../../../AceTimeTools)
TZ_REPO := $(abspath $(TOOLS)/../tz)
TZ_VERSION := 2023c
START_YEAR := 1800
UNTIL_YEAR := 10000

all:
	$(TOOLS)/tzcompiler.sh \
		--tzrepo $(TZ_REPO) \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language arduino \
		--scope complete \
		--offset_granularity 1 \
		--delta_granularity 60 \
		--until_at_granularity 1 \
		--generate_int16_years \
		--generate_hires \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

zonedb.json:
	$(TOOLS)/tzcompiler.sh \
		--tzrepo $(TZ_REPO) \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language json \
		--scope complete \
		--offset_granularity 1 \
		--delta_granularity 60 \
		--until_at_granularity 1 \
		--generate_int16_years \
		--generate_hires \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

clean:
	rm -rf tzfiles zonedb.json *.o

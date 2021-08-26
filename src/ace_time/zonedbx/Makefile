TARGETS := zone_infos.cpp zone_infos.h zone_policies.cpp zone_policies.h

# Don't use $(abspath) here because the value of $(TOOLS) is included in the
# generated files and I want that to be relative to the current project,
# instead of being expanded to my home directory on a particular machine.
TOOLS := ../../../../AceTimeTools

TZ_VERSION := 2021a
START_YEAR := 2000
UNTIL_YEAR := 2050

all:
	$(TOOLS)/tzcompiler.sh \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language arduino \
		--scope extended \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

zonedbx.json:
	$(TOOLS)/tzcompiler.sh \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language json \
		--scope extended \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

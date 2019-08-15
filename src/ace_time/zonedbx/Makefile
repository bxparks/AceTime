TARGETS := zone_infos.cpp zone_infos.h zone_policies.cpp zone_policies.h

all:
	../../../tools/tzcompiler.sh --tag 2019b --action zonedb \
		--language arduino --scope extended \
		--until_at_granularity 60 --offset_granularity 900 \
		--start_year 2000 --until_year 2050

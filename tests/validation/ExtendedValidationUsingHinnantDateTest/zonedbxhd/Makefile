TARGETS := zone_infos.cpp zone_infos.h zone_policies.cpp zone_policies.h

all:
	../../../../tools/tzcompiler.sh --tag 2019b --action zonedb \
		--language arduino --scope extended --db_namespace zonedbxhd \
		--until_at_granularity 60 --offset_granularity 60 \
		--start_year 1974 --until_year 2050

TARGETS := zone_infos.cpp zone_infos.h zone_policies.cpp zone_policies.h

START_YEAR=2000
UNTIL_YEAR=2050

all:
	../../../../tools/tzcompiler.sh --tag 2018g --action zonedb \
		--language arduino --scope extended --db_namespace zonedbx2018g \
		--start_year $(START_YEAR) --until_year $(UNTIL_YEAR)

TARGETS := zone_infos.py zone_policies.py

TZ_VERSION := 2020f
START_YEAR := 1974
UNTIL_YEAR := 2050

all: $(TARGETS)

.PHONY: $(TARGETS)

zone_infos.py:
	../tzcompiler.sh \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language python \
		--scope extended \
		--ignore_buf_size_too_large \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

zone_policies.py: zone_infos.py

clean:
	rm -f $(TARGETS)

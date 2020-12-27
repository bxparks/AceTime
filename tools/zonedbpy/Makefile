TARGETS := zone_infos.py zone_policies.py

TZ_VERSION := 2020d

all: $(TARGETS)

.PHONY: $(TARGETS)

zone_infos.py:
	../tzcompiler.sh \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language python \
		--scope extended

zone_policies.py: zone_infos.py

clean:
	rm -f $(TARGETS)

TARGETS := zone_infos.py zone_policies.py

all: $(TARGETS)

.PHONY: $(TARGETS)

zone_infos.py:
	../tzcompiler.sh --tag 2019b --action zonedb --language python --scope extended

zone_policies.py: zone_infos.py

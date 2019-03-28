TARGETS := zone_infos.py zone_policies.py validation_data.py

all: $(TARGETS)

.PHONY: $(TARGETS)

zone_infos.py:
	../tzcompiler.sh --tag 2018i --zonedb --python

zone_policies.py: zone_infos.py

validation_data.py:
	../tzcompiler.sh --tag 2018i --unittest --python

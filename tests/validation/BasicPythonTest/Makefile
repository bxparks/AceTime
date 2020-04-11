# See https://github.com/bxparks/UnixHostDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

DB_NAMESPACE = zonedbpytz
GENERATED := validation_data.cpp \
	validation_data.h \
	validation_tests.cpp \
	$(DB_NAMESPACE)/zone_infos.h \
	$(DB_NAMESPACE)/zone_infos.cpp \
	$(DB_NAMESPACE)/zone_policies.h \
	$(DB_NAMESPACE)/zone_policies.cpp \
	$(DB_NAMESPACE)/zone_registry.h \
	$(DB_NAMESPACE)/zone_registry.cpp \
	zones.txt
OBJS = validation_data.o \
	validation_tests.o \
	$(DB_NAMESPACE)/zone_infos.o \
	$(DB_NAMESPACE)/zone_policies.o \
	$(DB_NAMESPACE)/zone_registry.o
APP_NAME := BasicPythonTest
ARDUINO_LIBS := AUnit AceTime
include ../../../../UnixHostDuino/UnixHostDuino.mk

runtests:
	./$(APP_NAME).out

.PHONY: $(GENERATED)

# pytz (2019.3) uses 2019c
TZ_VERSION = 2019c
START_YEAR = 2000
UNTIL_YEAR = 2038

#validation_data.cpp:
#	 ../../../tools/tzcompiler.sh \
#	 	--tag $(TZ_VERSION) \
#	 	--action unittest \
#	 	--language arduino \
#	 	--scope basic \
#	 	--db_namespace $(DB_NAMESPACE) \
#	 	--start_year $(START_YEAR) \
#		--until_year $(UNTIL_YEAR)

validation_data.cpp: zones.txt
	../../../tools/compare_pytz/test_data_generator.py \
		--scope basic \
		--tz_version $(TZ_VERSION) \
		--db_namespace $(DB_NAMESPACE) \
		--output_dir $(CURDIR) \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR) \
		< zones.txt

zones.txt:
	../../../tools/tzcompiler.sh \
		--tag $(TZ_VERSION) \
		--action zonelist \
		--scope basic \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

# Make sure the zonedb files are generated before
# validation_data.cpp is compiled.
validation_data.o: $(DB_NAMESPACE)/zone_infos.h

# Generate the zonedb files under the $(DB_NAMESPACE) directory.
$(DB_NAMESPACE)/zone_infos.h:
	mkdir -p $(DB_NAMESPACE); \
	cd $(DB_NAMESPACE); \
	../../../../tools/tzcompiler.sh \
		--tag $(TZ_VERSION) \
		--action zonedb \
		--language arduino \
		--scope basic \
		--db_namespace $(DB_NAMESPACE) \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

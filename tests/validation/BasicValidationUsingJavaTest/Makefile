# See https://github.com/bxparks/UnixHostDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

DB_NAMESPACE = zonedb2018g
GENERATED := validation_data.cpp validation_data.h validation_tests.cpp \
	zones.txt \
	$(DB_NAMESPACE)/zone_infos.h $(DB_NAMESPACE)/zone_infos.cpp \
	$(DB_NAMESPACE)/zone_policies.h $(DB_NAMESPACE)/zone_policies.cpp \
	$(DB_NAMESPACE)/zone_registry.h $(DB_NAMESPACE)/zone_registry.cpp
OBJS := validation_data.o validation_tests.o $(DB_NAMESPACE)/zone_infos.o \
	$(DB_NAMESPACE)/zone_policies.o $(DB_NAMESPACE)/zone_registry.o
APP_NAME := BasicValidationUsingJavaTest
ARDUINO_LIBS := AUnit AceTime
include ../../../../UnixHostDuino/UnixHostDuino.mk

.PHONY: $(GENERATED)

runtests:
	./$(APP_NAME).out

TZ_VERSION = 2018g
START_YEAR = 2000
UNTIL_YEAR = 2050

validation_data.cpp: zones.txt ../../../tools/java/TestDataGenerator.class
	java -cp ../../../tools/java TestDataGenerator --scope basic \
		--db_namespace $(DB_NAMESPACE) \
		--start_year $(START_YEAR) --until_year $(UNTIL_YEAR) \
		< zones.txt

zones.txt:
	../../../tools/tzcompiler.sh --tag $(TZ_VERSION) --action zonelist \
		--scope basic \
		--start_year $(START_YEAR) --until_year $(UNTIL_YEAR) \

# Make sure the zonedb files are generated before
# validation_data.cpp is compiled.
validation_data.o: $(DB_NAMESPACE)/zone_infos.h

# Generate the zonedb files under the $(DB_NAMESPACE) directory.
$(DB_NAMESPACE)/zone_infos.h:
	mkdir -p $(DB_NAMESPACE); \
	cd $(DB_NAMESPACE); \
	../../../../tools/tzcompiler.sh --tag $(TZ_VERSION) --action zonedb \
		--language arduino --scope basic --db_namespace $(DB_NAMESPACE) \
		--start_year $(START_YEAR) --until_year $(UNTIL_YEAR)

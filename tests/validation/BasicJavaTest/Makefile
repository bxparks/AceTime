# See https://github.com/bxparks/UnixHostDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

DB_NAMESPACE = zonedbjava
GENERATED := validation_data.cpp \
	validation_data.h \
	validation_tests.cpp \
	validation_data.json \
	$(DB_NAMESPACE)/zone_infos.h \
	$(DB_NAMESPACE)/zone_infos.cpp \
	$(DB_NAMESPACE)/zone_policies.h \
	$(DB_NAMESPACE)/zone_policies.cpp \
	$(DB_NAMESPACE)/zone_registry.h \
	$(DB_NAMESPACE)/zone_registry.cpp \
	zones.txt
OBJS := validation_data.o \
	validation_tests.o \
	$(DB_NAMESPACE)/zone_infos.o \
	$(DB_NAMESPACE)/zone_policies.o \
	$(DB_NAMESPACE)/zone_registry.o
APP_NAME := BasicJavaTest
ARDUINO_LIBS := AUnit AceTime
include ../../../../UnixHostDuino/UnixHostDuino.mk

.PHONY: $(GENERATED)

runtests:
	./$(APP_NAME).out

# Define the scope of the test data.
# Validate using openjdk 11.0.6 2020-01-14 (which uses 2019c)
SCOPE = basic
TZ_VERSION = 2019c
START_YEAR = 2000
UNTIL_YEAR = 2050

validation_data.json: zones.txt \
../../../tools/compare_java/TestDataGenerator.class
	java -cp ../../../tools/compare_java TestDataGenerator \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR) \
		< zones.txt

validation_data.cpp: validation_data.json
	../../../tools/generate_validation.py \
		--tz_version $(TZ_VERSION) \
		--scope $(SCOPE) \
		--db_namespace $(DB_NAMESPACE) \
		< validation_data.json

../../../tools/compare_java/TestDataGenerator.class:
	make -C ../../../tools/compare_java

zones.txt:
	../../../tools/tzcompiler.sh \
		--tag $(TZ_VERSION) \
		--action zonelist \
		--scope $(SCOPE) \
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
		--scope $(SCOPE) \
		--db_namespace $(DB_NAMESPACE) \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

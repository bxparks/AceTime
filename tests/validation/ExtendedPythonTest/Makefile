# See https://github.com/bxparks/UnixHostDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

DB_NAMESPACE := zonedbxpytz
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
	$(DB_NAMESPACE)/zones.txt
OBJS := validation_data.o \
	validation_tests.o \
	$(DB_NAMESPACE)/zone_infos.o \
	$(DB_NAMESPACE)/zone_policies.o \
	$(DB_NAMESPACE)/zone_registry.o
APP_NAME := ExtendedPythonTest
ARDUINO_LIBS := AUnit AceTime
MORE_CLEAN := more_clean
include ../../../../UnixHostDuino/UnixHostDuino.mk

runtests:
	./$(APP_NAME).out

# Define the scope of the test data.
#	* pytz (2019.3) uses 2019c
#	* pytz (2020.1) uses 2020a
SCOPE = extended
TZ_VERSION = 2020a
START_YEAR = 2000
UNTIL_YEAR = 2038

validation_data.cpp: validation_data.h

validation_tests.cpp: validation_data.h

validation_data.h: validation_data.json
	../../../tools/generate_validation.py \
		--tz_version $(TZ_VERSION) \
		--scope $(SCOPE) \
		--db_namespace $(DB_NAMESPACE) \
		--blacklist ../../../tools/compare_pytz/blacklist.json \
		< validation_data.json

validation_data.json: $(DB_NAMESPACE)/zones.txt
	../../../tools/compare_pytz/test_data_generator.py \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR) \
		< $(DB_NAMESPACE)/zones.txt

$(DB_NAMESPACE)/%.o: $(DB_NAMESPACE)/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(DB_NAMESPACE)/zone_infos.cpp: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_policies.h: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_policies.cpp: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_registry.h: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_registry.cpp: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zones.txt: $(DB_NAMESPACE)/zone_infos.h

# Generate the zones.txt and zonedb files under the $(DB_NAMESPACE) directory.
$(DB_NAMESPACE)/zone_infos.h: tzfiles/$(TZ_VERSION)
	mkdir -p $(DB_NAMESPACE); \
	cd $(DB_NAMESPACE); \
	../../../../tools/tzcompiler.py \
		--input_dir ../tzfiles/$(TZ_VERSION) \
		--tz_version $(TZ_VERSION) \
		--action zonedb,zonelist \
		--language arduino \
		--scope $(SCOPE) \
		--db_namespace $(DB_NAMESPACE) \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

# Extract and copy the tzfiles into a local directory to allow parallel
# execution of validation/*Test makefiles.
tzfiles/$(TZ_VERSION):
	../../../tools/copytz.sh --output_dir tzfiles $(TZ_VERSION)

more_clean:
	rm -rf $(DB_NAMESPACE) tzfiles

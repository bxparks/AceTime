# See https://github.com/bxparks/UnixHostDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

DB_NAMESPACE := zonedbxhd
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
APP_NAME := ExtendedHinnantDateTest
ARDUINO_LIBS := AUnit AceCommon AceTime
MORE_CLEAN := more_clean
include ../../../../UnixHostDuino/UnixHostDuino.mk

#.PHONY: $(GENERATED)

runtests:
	./$(APP_NAME).out

# Define the scope of the test data.
# If START_YEAR < ~1974, then tzcompiler.py must be given the
# --ignore_buf_size_too_large flag to prevent an exception.
SCOPE = extended
TZ_VERSION = 2020f
START_YEAR = 1974
UNTIL_YEAR = 2050

validation_data.cpp: validation_data.h

validation_tests.cpp: validation_data.h

validation_data.h: validation_data.json
	../../../tools/generate_validation.py \
		--tz_version $(TZ_VERSION) \
		--scope $(SCOPE) \
		--db_namespace $(DB_NAMESPACE) \
		< validation_data.json

validation_data.json: $(DB_NAMESPACE)/zones.txt \
../../../tools/compare_cpp/generate_data.out
	../../../tools/compare_cpp/generate_data.out \
		--tz_version $(TZ_VERSION) \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR) \
		< $(DB_NAMESPACE)/zones.txt

../../../tools/compare_cpp/generate_data.out:
	make -C ../../../tools/compare_cpp/

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
		--ignore_buf_size_too_large \
		--input_dir ../tzfiles/$(TZ_VERSION) \
		--tz_version $(TZ_VERSION) \
		--language arduino,zonelist \
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

# See https://github.com/bxparks/EpoxyDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

# Define the scope of the test data.
SCOPE = extended
TZ_VERSION = 2021a
START_YEAR = 1974
UNTIL_YEAR = 2050

DB_NAMESPACE := zonedbxnoda
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
	$(DB_NAMESPACE)/zones.txt \
	tzdata$(TZ_VERSION).nzd
OBJS := validation_data.o \
	validation_tests.o \
	$(DB_NAMESPACE)/zone_infos.o \
	$(DB_NAMESPACE)/zone_policies.o \
	$(DB_NAMESPACE)/zone_registry.o
APP_NAME := ExtendedNodaTest
ARDUINO_LIBS := AUnit AceCommon AceTime
MORE_CLEAN := more_clean
include ../../../../EpoxyDuino/EpoxyDuino.mk

# Location of the https://github.com/nodatime/nodatime repo.
NODA_TIME_DIR := $(HOME)/dev/nodatime

runtests:
	./$(APP_NAME).out

validation_data.cpp: validation_data.h

validation_tests.cpp: validation_data.h

validation_data.h: validation_data.json
	../../../tools/generate_validation.py \
		--tz_version $(TZ_VERSION) \
		--scope $(SCOPE) \
		--db_namespace $(DB_NAMESPACE) \
		< validation_data.json

validation_data.json: $(DB_NAMESPACE)/zones.txt \
		../../../tools/compare_noda/Program.cs \
		tzdata$(TZ_VERSION).nzd
	dotnet run -p ../../../tools/compare_noda/compare_noda.csproj \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR) \
		--nzd_file tzdata$(TZ_VERSION).nzd \
		< $(DB_NAMESPACE)/zones.txt \
		> $@

# Generate the custom tzdata{xxx}.nzd file, depending on TZ_VERSION.
tzdata$(TZ_VERSION).nzd:
	dotnet run \
		-p $(NODA_TIME_DIR)/src/NodaTime.TzdbCompiler/NodaTime.TzdbCompiler.csproj \
		-- \
		-s https://data.iana.org/time-zones/releases/tzdata$(TZ_VERSION).tar.gz \
		-w $(NODA_TIME_DIR)/data/cldr/windowsZones-38-1.xml \
		-o $@

$(DB_NAMESPACE)/%.o: $(DB_NAMESPACE)/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(DB_NAMESPACE)/zone_infos.cpp: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_policies.h: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_policies.cpp: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_registry.h: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_registry.cpp: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zones.txt: $(DB_NAMESPACE)/zone_infos.h

# Generate the zones.txt and zonedbx files under the $(DB_NAMESPACE) directory.
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

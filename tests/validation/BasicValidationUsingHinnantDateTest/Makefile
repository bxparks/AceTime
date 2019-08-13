# See https://github.com/bxparks/UnixHostDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

GENERATED := zones.txt validation_data.cpp validation_data.h \
	validation_tests.cpp
OBJS := validation_data.o validation_tests.o
APP_NAME := BasicValidationUsingHinnantDateTest
ARDUINO_LIBS := AUnit AceTime
include ../../../../UnixHostDuino/UnixHostDuino.mk

.PHONY: $(GENERATED)

runtests:
	./$(APP_NAME).out

TZ_VERSION=2019a

validation_data.cpp: zones.txt
	../../../tools/cpp/test_data_generator.out --scope basic \
		--tz_version $(TZ_VERSION) \
		--start_year 2000 --until_year 2050 \
		< zones.txt

zones.txt:
	../../../tools/tzcompiler.sh --tag $(TZ_VERSION) --scope basic \
		--action zonedb --language java

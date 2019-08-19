# See https://github.com/bxparks/UnixHostDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

GENERATED := validation_data.cpp validation_data.h validation_tests.cpp \
	zones.txt
OBJS := validation_data.o validation_tests.o
APP_NAME := BasicValidationUsingHinnantDateTest
ARDUINO_LIBS := AUnit AceTime
include ../../../../UnixHostDuino/UnixHostDuino.mk

.PHONY: $(GENERATED)

runtests:
	./$(APP_NAME).out

TZ_VERSION=2019b
START_YEAR=2000
UNTIL_YEAR=2050

validation_data.cpp: zones.txt
	../../../tools/cpp/test_data_generator.out --scope basic \
		--tz_version $(TZ_VERSION) \
		--start_year $(START_YEAR) --until_year $(UNTIL_YEAR) \
		< zones.txt

zones.txt:
	../../../tools/tzcompiler.sh --tag $(TZ_VERSION) --action zonelist \
		--scope basic \
		--start_year $(START_YEAR) --until_year $(UNTIL_YEAR)

# See https://github.com/bxparks/UnixHostDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

GENERATED := validation_data.cpp validation_data.h validation_tests.cpp \
	zones.txt 
OBJS := validation_data.o validation_tests.o
APP_NAME := BasicValidationUsingJavaTest
ARDUINO_LIBS := AUnit AceTime
include ../../../../UnixHostDuino/UnixHostDuino.mk

.PHONY: $(GENERATED)

runtests:
	./$(APP_NAME).out

START_YEAR=2000
UNTIL_YEAR=2050

validation_data.cpp: zones.txt ../../../tools/java/TestDataGenerator.class
	java -cp ../../../tools/java TestDataGenerator --scope basic \
		--db_namespace zonedb2018g \
		--start_year $(START_YEAR) --until_year $(UNTIL_YEAR) \
		< zones.txt

zones.txt:
	../../../tools/tzcompiler.sh --tag 2018g --action zonelist \
		--scope basic \
		--start_year $(START_YEAR) --until_year $(UNTIL_YEAR) \
		


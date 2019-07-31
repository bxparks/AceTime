# See https://github.com/bxparks/UnixHostDuino for documentation
# about using UnixHostDuino to compile and run AUnit tests natively on Linux or
# MacOS.

GENERATED := zones.txt validation_data.cpp validation_data.h validation_tests.cpp
OBJS := validation_data.o validation_tests.o
APP_NAME := BasicValidationUsingJavaTest
ARDUINO_LIBS := AUnit AceTime
include ../../../../UnixHostDuino/UnixHostDuino.mk

.PHONY: $(GENERATED)

runtests:
	./$(APP_NAME).out

validation_data.cpp: zones.txt ../../../tools/java/TestDataGenerator.class
	java -cp ../../../tools/java TestDataGenerator --scope basic --db_namespace zonedb2018g --start_year 2000 --until_year 2050 < zones.txt

zones.txt:
	../../../tools/tzcompiler.sh --tag 2018g --scope basic --action zonedb --language java

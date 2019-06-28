# See https://github.com/bxparks/AUnit/tree/develop/unitduino for documentation
# about using unitduino to compile and run AUnit tests natively on Linux or
# MacOS.

GENERATED := zones.txt validation_data.cpp validation_data.h validation_tests.cpp
OBJS = validation_data.o validation_tests.o
APP_NAME := ExtendedValidationUsingJavaTest
ARDUINO_LIBS := AceTime
include ../../../../AUnit/unitduino/unitduino.mk

.PHONY: $(GENERATED)

runtests:
	./$(APP_NAME).out

validation_data.cpp: zones.txt ../../../tools/java/TestDataGenerator.class
	java -cp ../../../tools/java TestDataGenerator --scope extended --db_namespace zonedbx2018g --start_year 2000 --until_year 2050 < zones.txt

zones.txt:
	../../../tools/tzcompiler.sh --tag 2018g --scope extended --action zonedb --language java

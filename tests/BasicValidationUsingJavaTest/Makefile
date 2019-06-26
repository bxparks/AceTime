# See https://github.com/bxparks/AUnit/tree/develop/unitduino for documentation
# about using unitduino to compile and run AUnit tests natively on Linux or
# MacOS.

APP_NAME := BasicValidationUsingJavaTest
ARDUINO_LIBS := AceTime
include ../../../AUnit/unitduino/unitduino.mk

validation_data: zones.txt
	java -cp ../../tools/java TestDataGenerator --scope basic --db_namespace zonedb2018g --start_year 2000 --until_year 2050 < zones.txt

zones.txt:
	../../tools/tzcompiler.sh --tag 2018g --scope basic --action zonedb --language java

.PHONY: zones.txt

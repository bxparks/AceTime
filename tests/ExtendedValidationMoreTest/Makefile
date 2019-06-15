# See https://github.com/bxparks/AUnit/tree/develop/unitduino for documentation
# about using unitduino to compile and run AUnit tests natively on Linux or
# MacOS.

APP_NAME := ExtendedValidationMoreTest
ARDUINO_LIBS := AceTime
include ../../../AUnit/unitduino/unitduino.mk

validation_data: zones.txt
	java -cp ../../tools/java TestDataGenerator --scope extended --startYear 2000 --untilYear 2050 < zones.txt

zones.txt:
	../../tools/tzcompiler.sh --tag 2019a --scope extended --action zonedb --language java

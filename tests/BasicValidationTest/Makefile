# See https://github.com/bxparks/AUnit/tree/develop/unitduino for documentation
# about using unitduino to compile and run AUnit tests natively on Linux or
# MacOS.

APP_NAME := BasicValidationTest
ARDUINO_LIBS := AceTime
include ../../../AUnit/unitduino/unitduino.mk

validation_data:
	../../tools/tzcompiler.sh --tag 2019a --action unittest --language arduino --scope basic

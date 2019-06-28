# See https://github.com/bxparks/AUnit/tree/develop/unitduino for documentation
# about using unitduino to compile and run AUnit tests natively on Linux or
# MacOS.

GENERATED := validation_data.cpp validation_data.h validation_tests.cpp
OBJS = validation_data.o validation_tests.o
APP_NAME := BasicValidationUsingPythonTest
ARDUINO_LIBS := AceTime
include ../../../../AUnit/unitduino/unitduino.mk

runtests:
	./$(APP_NAME).out

.PHONY: $(GENERATED)

validation_data.cpp:
	../../../tools/tzcompiler.sh --tag 2019a --action unittest --language arduino --scope basic --start_year 2000 --until_year 2038

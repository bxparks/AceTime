# See https://github.com/bxparks/UnixHostDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

GENERATED := zones.txt validation_data.cpp validation_data.h \
	validation_tests.cpp
OBJS := validation_data.o validation_tests.o test_data_generator.o
APP_NAME := BasicValidationUsingHinnantDateTest
ARDUINO_LIBS := AUnit AceTime
include ../../../../UnixHostDuino/UnixHostDuino.mk

.PHONY: $(GENERATED)

runtests:
	./$(APP_NAME).out

validation_data.cpp: zones.txt test_data_generator.out
	./test_data_generator.out --scope basic --db_namespace zonedb \
		--start_year 2000 --until_year 2050 \
		< zones.txt

zones.txt:
	../../../tools/tzcompiler.sh --tag 2019a --scope basic --action zonedb \
		--language java

HINNANT_DATE_DIR := ../../../../date

test_data_generator.out: test_data_generator.cpp
	$(CXX) -I $(HINNANT_DATE_DIR)/include -Wall -std=gnu++11 -o $@ $^ \
		$(HINNANT_DATE_DIR)/src/*.cpp -lcurl

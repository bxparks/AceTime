# See https://github.com/bxparks/EpoxyDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

# Location of https://github.com/HowardHinnant/date, assumed to be sibling to
# this project.
HINNANT_DATE_DIR := $(abspath ../../../date)

# Various compiler and linker flags borrowed from
# AceTimeTools/compare_cpp/Makefile.
APP_NAME := CompareAceTimeToHinnantDate
ARDUINO_LIBS := AceCommon AceSorting AceTime
DATE_SRC := $(wildcard $(HINNANT_DATE_DIR)/src/*.cpp)
OBJS := $(DATE_SRC:%.cpp=%.o)
CPPFLAGS := -DHAS_REMOTE_API=1 -DAUTO_DOWNLOAD=0
CXXFLAGS := -I $(HINNANT_DATE_DIR)/include
LDFLAGS := -lcurl
include ../../../EpoxyDuino/EpoxyDuino.mk

TZ_VERSION := 2022g
START_YEAR := 2000
UNTIL_YEAR := 2100

result.txt: CompareAceTimeToHinnantDate.out Makefile
	./CompareAceTimeToHinnantDate.out \
		--tz_version $(TZ_VERSION) \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)\
		< zones.txt \
		> $@

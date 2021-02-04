# See https://github.com/bxparks/EpoxyDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

APP_NAME := AutoBenchmark
ARDUINO_LIBS := AceCommon AceTime AceRoutine
MORE_CLEAN := more_clean
include ../../../EpoxyDuino/EpoxyDuino.mk

.PHONY: benchmarks

AUNITER_DIR := ../../../AUniter/tools

TARGETS := nano.txt micro.txt samd.txt stm32.txt esp8266.txt esp32.txt \
teensy32.txt

# It is not possible to auto-generate the $(TARGETS) because the USB ports are
# dynamic.
README.md: generate_readme.py generate_table.awk $(TARGETS)
	./generate_readme.py > $@

benchmarks: $(TARGETS)

# These targets cannot be automated because the tty ports (USB0, ACM0) can
# change dynamically. I activate one microcontroller at a time, verify its USB
# port (when only a single microcontroller is active, it becomes one of the
# USB0 or ACM0 ports listed below), then run 'make xxx.txt' manually the
# command line.
nano.txt:
	$(AUNITER_DIR)/auniter.sh --cli upmon -o $@ --eof END nano:USB0

micro.txt:
	$(AUNITER_DIR)/auniter.sh --ide upmon -o $@ --eof END micro:ACM0

samd.txt:
	$(AUNITER_DIR)/auniter.sh --ide upmon -o $@ --eof END samd:ACM0

stm32.txt:
	$(AUNITER_DIR)/auniter.sh --cli upmon -o $@ --eof END stm32:ACM0

esp8266.txt:
	$(AUNITER_DIR)/auniter.sh --cli upmon -o $@ --eof END nodemcu:USB0

esp32.txt:
	$(AUNITER_DIR)/auniter.sh --cli upmon -o $@ --eof END esp32:USB0

# Teensy cannot upload a Arduino sketch and capture its Serial output
# programmatically using a script, at least, I don't know how. To generate
# teesy32.txt, I have to manually upload the program using the IDE, then fire
# up the Serial Monitor, then copy and paste the Serial output manually into
# teensy.txt. This Makefile rule is an aspirational hope that Teensy might
# allow automation some day in the future.
teensy32.txt:
	$(AUNITER_DIR)/auniter.sh --cli upmon -o $@ --eof END teensy32:ACM0

more_clean:
	echo "Use 'make clean_benchmarks' to remove *.txt files"

clean_benchmarks:
	rm -f $(TARGETS)

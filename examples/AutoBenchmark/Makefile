# See https://github.com/bxparks/EpoxyDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

APP_NAME := AutoBenchmark
ARDUINO_LIBS := AceCommon AceRoutine AceSorting AceTime
DEPS := Benchmark.o basic_registry.o extended_registry.o complete_registry.o
MORE_CLEAN := more_clean
include ../../../EpoxyDuino/EpoxyDuino.mk

.PHONY: benchmarks

AUNITER_DIR := ../../../AUniter/tools

TARGETS := nano.txt micro.txt samd21.txt stm32.txt samd51.txt \
	esp8266.txt esp32.txt

README.md: $(TARGETS) generate_readme.py generate_table.awk
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
	$(AUNITER_DIR)/auniter.sh --cli upmon -o $@ --eof END micro:ACM0

# Seeeduino XIAO M0 seems to need a short delay after flashing to detect its
# serial port.
samd21.txt:
	$(AUNITER_DIR)/auniter.sh --cli upmon --delay 1 -o $@ --eof END xiao:ACM0

stm32.txt:
	$(AUNITER_DIR)/auniter.sh --cli upmon -o $@ --eof END stm32:ACM0

# ItsyBitsy M4 appears on ACM0 when powered on, then disappears after flashing,
# and reappears on ACM1s. Specifying the ports as "ACM*" tells auniter.sh to
# use any port satisfying the glob pattern.
samd51.txt:
	$(AUNITER_DIR)/auniter.sh --cli upmon --delay 2 -o $@ --eof END itsym4:ACM*

esp8266.txt:
	$(AUNITER_DIR)/auniter.sh --cli upmon -o $@ --eof END nodemcu:USB0

esp32.txt:
	$(AUNITER_DIR)/auniter.sh --cli upmon -o $@ --eof END esp32:USB0

more_clean:
	echo "Use 'make clean_benchmarks' to remove *.txt files"

clean_benchmarks:
	rm -f $(TARGETS)

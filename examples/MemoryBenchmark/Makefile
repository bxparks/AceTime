TARGETS := nano.txt micro.txt samd.txt esp8266.txt esp32.txt teensy.txt

# Running 'make' does nothing to prevent interference with continuous build.
all:

# Type 'make benchmarks' to regenerate the benchmark files.
benchmarks: $(TARGETS)

.PHONY: $(TARGETS)

nano.txt:
	./collect.sh nano $@

micro.txt:
	./collect.sh micro $@

samd.txt:
	./collect.sh mkrzero $@

esp8266.txt:
	./collect.sh esp8266 $@

esp32.txt:
	./collect.sh esp32 $@

teensy.txt:
	@echo 'Must collect teensy.txt manually'

# Don't use 'clean' because we want to preserve the '*.txt' files when
# uber 'make clean' is called from the parent directory.
clean-benchmarks:
	rm -f $(TARGETS)

# Define a 'clean' target that does nothing.
clean:
	echo "Use 'make clean-benchmarks' to remove the *.txt files"

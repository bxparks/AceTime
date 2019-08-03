TARGETS := nano.txt micro.txt samd.txt esp8266.txt esp32.txt teensy.txt

all: $(TARGETS)

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

clean:
	rm -f $(TARGETS)

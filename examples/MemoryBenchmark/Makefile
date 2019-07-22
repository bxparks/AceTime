TARGETS := nano.txt micro.txt esp8266.txt esp32.txt

nano.txt:
	./collect.sh nano $@

micro.txt:
	./collect.sh micro $@

esp8266.txt:
	./collect.sh esp8266 $@

esp32.txt:
	./collect.sh esp32 $@

clean:
	rm -f $(TARGETS)

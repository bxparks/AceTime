GENERATED := validation_data.json

all:
	@echo 'No compilation necessary'

validation_data.json: small.txt generate_data.py tdgenerator.py
	./generate_data.py < small.txt > $@

clean:
	rm -f $(GENERATED)

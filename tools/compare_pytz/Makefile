GENERATED := validation_data.json

all:
	@echo 'No compilation necessary'

validation_data.json:
	./generate_data.py < small.txt > $@

clean:
	rm -f $(GENERATED)

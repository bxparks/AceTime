GENERATED := validation_data.h validation_data.cpp validation_tests.cpp \
	validation_data.json

mypy:
	mypy --strict .

small:
	./test_data_generator.py \
		--scope basic \
		--tz_version 2019c \
		--db_namespace zonedb \
		< small.txt

data:
	./test_data_generator.py \
		--scope basic \
		--tz_version 2019c \
		--db_namespace zonedb \
		< zones.txt

json:
	./test_data_generator.py \
		--scope basic \
		--tz_version 2019c \
		--db_namespace zonedb \
		--format json \
		< small.txt

clean:
	rm -f $(GENERATED)

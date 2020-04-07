GENERATED := validation_data.h validation_data.cpp validation_tests.cpp

mypy:
	mypy --strict .

small:
	./test_data_generator.py \
		--scope basic \
		--tz_version 2019c \
		--db_namespace zonedb \
		--output_dir $(PWD) \
		< small.txt

data:
	./test_data_generator.py \
		--scope basic \
		--tz_version 2019c \
		--db_namespace zonedb \
		--output_dir $(PWD) \
		< zones.txt

clean:
	rm -f $(GENERATED)

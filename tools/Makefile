TZ_VERSION := 2019c

# Files which pass 'mypy --strict'.
SRC := \
argenerator.py \
bufestimator.py \
compare_pytz \
extractor.py \
generate_validation.py \
ingenerator.py \
tzdbgenerator.py \
pygenerator.py \
tests/test_extractor.py \
tests/test_transformer.py \
transformer.py \
tzcompiler.py \
validate.py \
validation \
zone_specifier.py \
zonelistgenerator.py

# Files without Python typing.
SRC_UNTYPED := \
tests/test_zone_specifier.py

.PHONY: all mypy flake8 tests

all: mypy flake8 tests

mypy:
	mypy --strict $(SRC)

tests:
	python3 -m unittest

flake8:
	flake8 . \
		--exclude=archive,zonedb \
		--count \
		--ignore W503 \
		--show-source \
		--statistics \
		--max-line-length=80

# Copy the TZ DB files into this directory for testing purposes.
$(TZ_VERSION):
	./copytz.sh $(TZ_VERSION)

# Run the Validator using validate.py.
validate: $(TZ_VERSION)
	./validate.py --input_dir $(TZ_VERSION) --scope basic

# Generate tzdb.json for testing purposes.
tzdb.json: $(SRC) $(TZ_VERSION)
	./tzcompiler.py --tz_version $(TZ_VERSION) --input_dir $(TZ_VERSION) \
	--scope basic --action tzdb

# Generate the zones.txt file for testing purposes.
zones.txt: $(SRC) $(TZ_VERSION)
	./tzcompiler.py --tz_version $(TZ_VERSION) --input_dir $(TZ_VERSION) \
	--scope basic --action zonelist

# Generate the validation_data.json for testing purposes
validation_data.json: zones.txt
	./compare_pytz/test_data_generator.py --tz_version $(TZ_VERSION) \
	--scope basic --format json < $<

# Generate the validation_data.{h,cpp}, validation_tests.cpp
validation_data.h: validation_data.json
	./generate_validation.py --tz_version $(TZ_VERSION) \
	--scope basic --db_namespace zonedb < $<

validation_data.cpp: validation_data.h
	@true

validation_tests.cpp: validation_data.h
	@true

clean:
	rm -f zones.txt tzdb.json validation_data.json validation_data.h \
		validation_data.cpp validation_tests.cpp
	rm -rf $(TZ_VERSION)
	make -C compare_pytz clean
	make -C compare_java clean
	make -C compare_cpp clean

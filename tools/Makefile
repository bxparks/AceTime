TZ_VERSION := 2020d

# Files which pass 'mypy --strict'.
SRC := \
compare_dateutil \
compare_pytz \
data_types \
extractor \
generate_validation.py \
generator \
tests/test_extractor.py \
tests/test_transformer.py \
tests/test_zone_specifier.py \
tzcompiler.py \
transformer \
validate.py \
validation \
validator \
zinfo.py \
zone_processor

# Files without Python typing.
SRC_UNTYPED := \
tests/test_zone_specifier.py

.PHONY: all mypy flake8 tests

all: mypy flake8 tests

mypy:
	mypy --strict $(SRC)

tests:
	python3 -m unittest

# W503 and W504 are both enabled by default and are mutual
# contradictory, so we have to suppress one of them.
# E501 uses 79 columns by default, but 80 is the default line wrap in
# vim, so change the line-length.
flake8:
	flake8 . \
		--exclude=archive,zonedbpy \
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
	./validate.py --input_dir $(TZ_VERSION) --scope extended

# Generate zonedb.json for testing purposes.
zonedb.json: $(SRC) $(TZ_VERSION)
	./tzcompiler.py \
		--tz_version \
		$(TZ_VERSION) \
		--input_dir $(TZ_VERSION) \
		--scope basic \
		--language json \
		--json_file $@ \
		--start_year 2000 \
		--until_year 2050

# Generate zonedbx.json for testing purposes.
zonedbx.json: $(SRC) $(TZ_VERSION)
	./tzcompiler.py \
		--tz_version \
		$(TZ_VERSION) \
		--input_dir $(TZ_VERSION) \
		--scope extended \
		--language json \
		--json_file $@ \
		--start_year 2000 \
		--until_year 2050

# Generate the zones.txt file for testing purposes.
zones.txt: $(SRC) $(TZ_VERSION)
	./tzcompiler.py \
		--tz_version $(TZ_VERSION)
		--input_dir $(TZ_VERSION) \
		--scope basic \
		--language zonelist

# Generate the validation_data.json for testing purposes
validation_data.json: zones.txt
	./compare_pytz/test_data_generator.py < $<

# Generate the validation_data.{h,cpp}, validation_tests.cpp
validation_data.h: validation_data.json
	./generate_validation.py --tz_version $(TZ_VERSION) \
	--scope basic --db_namespace zonedb < $<

validation_data.cpp: validation_data.h
	@true

validation_tests.cpp: validation_data.h
	@true

clean:
	rm -f zones.txt zonedb.json zonedbx.json validation_data.json \
		validation_data.h validation_data.cpp validation_tests.cpp
	rm -rf $(TZ_VERSION)
	make -C compare_pytz clean
	make -C compare_dateutil clean
	make -C compare_java clean
	make -C compare_cpp clean

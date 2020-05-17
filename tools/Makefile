TZ_VERSION := 2019c

# Files which pass 'mypy --strict'.
SRC := \
acetz.py \
compare_pytz \
compare_dateutil \
generate_validation.py \
tests/test_extractor.py \
tests/test_transformer.py \
tzcompiler.py \
tzdb/extractor.py \
tzdb/transformer.py \
tzdb/tzdbcollector.py \
validate.py \
validation \
validator \
zinfo.py \
zonedb/argenerator.py \
zonedb/bufestimator.py \
zonedb/ingenerator.py \
zonedb/pygenerator.py \
zonedb/zone_specifier.py \
zonedb/zonelistgenerator.py

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
	rm -f zones.txt tzdb.json validation_data.json validation_data.h \
		validation_data.cpp validation_tests.cpp
	rm -rf $(TZ_VERSION)
	make -C compare_pytz clean
	make -C compare_java clean
	make -C compare_cpp clean

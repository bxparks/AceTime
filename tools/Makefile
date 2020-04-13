TZ_VERSION := 2019c

# Files which pass 'mypy --strict'.
SRC := \
argenerator.py \
bufestimator.py \
extractor.py \
ingenerator.py \
jsongenerator.py \
pygenerator.py \
tests/test_extractor.py \
tests/test_transformer.py \
transformer.py \
tzcompiler.py \
validate.py \
validation/tdgenerator.py \
validation/validator.py \
zone_specifier.py \
zonelistgenerator.py

# Files without Python typing.
SRC_UNTYPED := \
test_zone_specifier.py

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

clean:
	rm -f zones.txt tzdb.json
	rm -rf $(TZ_VERSION)

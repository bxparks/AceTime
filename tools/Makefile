# Full typing added to pass --strict mode.
SRC = \
argenerator.py \
bufestimator.py \
extractor.py \
ingenerator.py \
jsongenerator.py \
pygenerator.py \
tdgenerator.py \
test_extractor.py \
test_transformer.py \
transformer.py \
tzcompiler.py \
validate.py \
validator.py \
zone_specifier.py \
zonelistgenerator.py

# Files without Python typing.
SRC_UNTYPED = \
test_zone_specifier.py # Takes too long, imports zonedb/* files

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

# Run the Validator using validate.sh which runs validate.py.
validate:
	./validate.sh --tag 2019c --scope basic

# Generate tzdb.json for testing purposes.
tzdb.json: $(SRC)
	./tzcompiler.sh --tag 2019c --scope basic --action tzdb

# Generate the zones.txt file for testing purposes.
zones.txt: $(SRC)
	./tzcompiler.sh --tag 2019c --scope basic --action zonelist

clean:
	rm -f zones.txt tzdb.json

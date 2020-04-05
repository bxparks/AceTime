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

# Run the Validator using --action validate, for testing purposes.
validate:
	./tzcompiler.sh \
		--tag 2019c \
		--action validate \
		--language python \
		--scope basic

# Generate zonedb.json for testing purposes.
zonedb.json: extractor.py transformer.py jsongenerator.py \
tzcompiler.py tzcompiler.sh
	./tzcompiler.sh \
		--tag 2019c \
		--action zonedb \
		--language json \
		--scope basic

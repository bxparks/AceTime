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

# Run --validate_buffer_size and --validate_test_data.
validate-basic:
	./tzcompiler.sh \
		--tag 2019c \
		--action validate \
		--language python \
		--scope basic

validate-extended:
	./tzcompiler.sh \
		--tag 2019c \
		--action validate \
		--language python \
		--scope extended

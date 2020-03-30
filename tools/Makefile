# Full typing added to pass --strict mode.
SRC = \
argenerator.py \
bufestimator.py \
extractor.py \
ingenerator.py \
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

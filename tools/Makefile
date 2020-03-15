# Initial pass at adding typing info.
SRC = \
argenerator.py \
arvalgenerator.py \
bufestimator.py \
extractor.py \
ingenerator.py \
pygenerator.py \
pyvalgenerator.py \
tdgenerator.py \
test_extractor.py \
test_transformer.py \
transformer.py \
validator.py \
zone_specifier.py \
zonelistgenerator.py

# Full typing added to pass --strict mode.
SRC_STRICT = \
extractor.py \
ingenerator.py \
transformer.py

# Files without Python typing.
SRC_UNTYPED = \
test_zone_specifier.py \
tzcompiler.py

mypy:
	mypy $(SRC)

strict:
	mypy --strict $(SRC_STRICT)

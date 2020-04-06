# Archived Code

Deprecated code, but might be useful later.

## Test Data Generator

The `arvalgenerator.py`, `pyvalgenerator.py`, and `tdgenerator.py` modules
use the `zone_specifier.py` and `pytz` to generate the `validation_*`
unit test files, for both Arduino and Python.

For the Arduino vaidation files, this pipeline has been replaced with
`compare_pytz` module.

For the Python test files, we no longer do unit testing for the `ZoneSpecifier`
class.

```
        ingenerator.py
               |
               v
            zone_specifier.py
                    |
                    |     pytz
                    |     /
                    v    v
            tdgenerator.py
                /       \
                v         v
    arvalgenerator.py   pyvalgenerator.py
        |                    |
        v                    v
validation_data.{h,cpp}   validation_data.py
validation_tests.cpp




```

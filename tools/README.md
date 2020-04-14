# Tools

These are the Python and Java programs which generate the various zoneinfo files
and validation datasets from the [IANA TZ
database](https://www.iana.org/time-zones).

The main driver is the `tzcompiler.sh`, which is a thin shell wrapper
around the `tzcompiler.py` script, which invokes an ETL data
processing pipeline that converts the various TZ Database files (with `Zone`,
`Link` and `Rule` entries) into generated code fragments (`zone_infos.h`,
`zone_infos.cpp`, etc).

## TZ Compiler

The TZ Database processing pipeline for `tzcompiler.py` looks something like
this:

```
                    TZDB files
                        |
                        v
                   extractor.py
                        |
                        v
                  transformer.py
                        |
                        v          [tzdb]
      .-------- jsongenerator.py ----------> tzdb.json
     /             /    |    \
    /             /     |     \
   /             /      |      \ [zonedb/python]
  /             /       |       v
 /             /        |     pygenerator.py
/             /         |           \
|  [zonedb/  /  ingenerator.py       v
|  arduino] /          /        zone_infos.py
|          /          /         zone_policies.py
|         /          /          zone_strings.py
|        /          v                |
|       / zone_specifier.py          v
|      /           /            zone_specifier.py
|     /           v
|     |   bufestimator.py
|     |      /
|     |     /
|     v    v
|  argenerator.py
|        |
|        v
|   zone_infos.{h,cpp}
|   zone_policies.{h,cpp}
|   zone_registry.{h,cpp}
|   zone_strings.{h,cpp} (optional)
 \
  \
   v
  zonelistgenerator.py
      |
      v
    zones.txt
```

The `[zonedb/*]` and `[zonelist]` annotations in the diagram correspond to the
value of the `--action` and `--language` flags on the `tzcompiler.py` script.

## Validation Data Generation

These programs generate a list of `TestItem` which contains the `epoch_seconds`
and the date/time components from various 3rd party date/time libraries. The
AceTime data/time libraries (specifically the `ZoneProcessor` classes) will be
tested against the expected results from these 3rd party libraries. Three
libraries are supported:

* `compare_pytz` - Python `pytz` library
* `compare_java` - Java JDK11 `java.time` library
* `compare_cpp` - C++ Hinnant Date library

The `zones.txt` from the `tzcompiler.py` determines the time zones which
should be processed by the various `compare_xxx` scripts:

```
   zones.txt [from tzcompiler.py]
      |
      |  (compare_java/):
      |
      |         java.time
      |            |
      |            v
      +--> TestDataGenerator.java
      |           /             \
      |          v               v
      |  validation_data.{h,cpp} validation_data.json
      |  validation_tests.cpp
      |
      |
      |  (compare_cpp/):
      |
      |       Hinnant date
      |         |
      |         v
      +--> test_data_generator.cpp
      |          \
      |           v
      |          validation_data.{h,cpp}
      |          validation_tests.cpp
      |
      |
      |  (compare_pytz/):
      |
      |               pytz
      |                |
      |                v
      |         tdgenerator.py
      |             /     \
      |            v       v
      | arvalgenerator.py  jsonvalgenerator.py
      |          |             |
      |          |             |    validation/data.py
      |          |             |     /
      |          v             v    v
      +----->  test_data_generator.py
                /               \
               v                 v
      validation_data.{h,cpp}  validation_data.json
      validation_tests.cpp
```

## Interactive Validation

The `validate.py` script allows us to validate the processing of the
TZ Database through the Python implementation of the AceTime `ZoneProcessor`
classes. The Python implementation is called `ZoneSpecifier` (a previous naming
convention used in the C++ version which did not make it over the Python world).

The data processing pipeline for the `validation.py` script looks like this
(this used to be handled by `tzcompiler.py` itself before it was extracted out
into `validate.py`):

```
     TZDB files
         |
         v
    extractor.py
         |
         v
    transformer.py
         |
         v
   ingenerator.py
       /  \
      /    v             pytz
     | zone_specifier.py /
     |      \           /
     |       v         v
     |   validation/tdgenerator.py
     .                \
      \                \
       v                |
     zone_specifier.py  |
          |            /
          |           /
          v          v
     validation/validator.py
```

## Dependencies

The `tzcompiler.sh` script assumes that the [TZ Database from
GitHub](https://github.com/eggert/tz) is located as a sibling directory to
`./AceTime`. If `$ACE_TIME` is the directory of the AceTime library, then
we can run the following commands:
```
$ cd $ACE_TIME/..
$ git clone https://github.com/eggert/tz
```

## Usage

### Generating ZoneDB Files

To generate the `zonedb::` files in `src/ace_time/zonedb`, the `--action zonedb`
option is used:

```
$ cd $ACE_TIME/src/ace_time/zonedb
$ ../../../tools/tzcompiler.sh --tag 2019a --action zonedb --language arduino
--scope basic --start_year 2000 --until_year 2050
```

This has been captured in the `Makefile`, so you can also just type:
```
$ cd $ACE_TIME/src/ace_time/zonedb
$ make
```

To generate the `zonedbx::` files in `src/ace_time/zonedbx`, run the following
commands:

```
$ cd $ACE_TIME/src/ace_time/zonedbx
$ ../../../tools/tzcompiler.sh --tag 2019a --action zonedb \
--language arduino --scope extended --start_year 2000 --until_year 2050
```

This has been captured in the `Makefile`, so you can also just type:
```
$ cd $ACE_TIME/src/ace_time/zonedbx
$ make
```


### Generating Validation Files

To generate the various `validation_data.*` files for the validation tests under
`tests/validation/`, use the `--action zonelist` to generate the `zones.txt`
file. Then feed this file into the `tools/compare_java/TestDataGenerator.java`
or the `tools/compare_cpp/test_data_generator.cpp` programs, which then generate
the various `validation_data.*` files.

For validation against `pytz`, the 2 steps were combined into a single step
within `tzcompiler.py` itself, using the `--action unittest` flag. (For
symmetry, consider extracting the code that generates the `validation_data.*`
files out from `tzcompiler.py` into a separate Python program.)

First, compile the Java and C++ programs:
```
$ cd $ACE_TIME/tools/compare_java
$ make clean
$ make

$ cd $ACE_TIME/tools/compare_cpp
$ make clean
$ make
```

Then run the `make` command under each test directory:

```
$ cd $ACE_TIME/tests/validation/BasicHinnantDateTest/
$ make validation_data.cpp
```
and
```
$ cd $ACE_TIME/tests/validation/BasicJavaTest/
$ make validation_data.cpp
```
and
```
$ cd $ACE_TIME/tests/validation/BasicPythonTest/
$ make validation_data.cpp
```

and so on.

There is an uber `tests/validation/Makefile` which can generate
the `validation_data.*` files for all subdirectories:
```
$ cd $ACE_TIME/tests/validation
$ make tests
```

### Type Checking

The scripts should pass `mypy` type checking in `strict` mode:
```
$ make mypy
```

### Unit Testing

The unit tests can be run with:
```
$ make tests
```

# Tools

These are the Python and Java programs which generate the various zoneinfo files
and validation datasets from the [IANA TZ
database](https://www.iana.org/time-zones).

The main driver is the `tzcompiler.py` script, which invokes an ETL data
processing pipeline that converts the various TZ Database files (with `Zone`,
`Link` and `Rule` entries) into generated code fragments (`zone_infos.h`,
`zone_infos.cpp`, etc). The data processing pipeline looks something like this:

```
                TZDB files
                    |
                    v
               extractor.py
                    |
                    v
             transformer.py---------------------.
              /     |    \                       \
             /      |     v                       v
            /       |     pygenerator.py       zonelistgenerator.py
           /        |           \                    |
          / ingenerator.py       v                   v
         /          |      zone_infos.py        zones.txt       java.time
        /           |      zone_policies.py       /   \          /
       /           / \     zone_strings.py       /     v        v
      /           /   \                         /  TestDataGenerator.java
     /           v     \                       /         |
    /  bufestimator.py |\                     |          v
   /     /             | \          pytz      |  validation_data.{h,cpp}
  v     v              |  \        /  |       |  validation_tests.cpp
argenerator.py         |   v      v   |       |
     |                 | validator.py |        \           Hinnant date
     v                 |              /         \              /
zone_infos.{h,cpp}     \             /           v            v
zone_policies.{h,cpp}   \           /      test_data_generator.cpp
zone_registry.{h,cpp}    \         /                |
zone_strings.{h,cpp}      \       /                 v
                           v     v          validation_data.{h,cpp}
                       tdgenerator.py       validation_tests.cpp
                          /       \
                         v         v
             arvalgenerator.py   pyvalgenerator.py
                |                    |
                v                    v
       validation_data.{h,cpp}   validation_data.py
       validation_tests.cpp
```

The `tzcompiler.sh` is a thin shell wrapper that makes it slightly easier to
remember certain flags.

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
file. Then feed this file into the `tools/java/TestDataGenerator.java` or the
`tools/cpp/test_data_generator.cpp` programs, which then generate the various
`validation_data.*` files.

For validation against `pytz`, the 2 steps were combined into a single step
within `tzcompiler.py` itself, using the `--action unittest` flag. (For
symmetry, consider extracting the code that generates the `validation_data.*`
files out from `tzcompiler.py` into a separate Python program.)

First, compile the Java and C++ programs:
```
$ cd $ACE_TIME/tools/java
$ make clean
$ make

$ cd $ACE_TIME/tools/cpp
$ make clean
$ make
```

Then run the `make` command under each test directory:

```
$ cd $ACE_TIME/tests/validation/BasicValidationUsingHinnantDateTest/
$ make validation_data.cpp
```
and
```
$ cd $ACE_TIME/tests/validation/BasicValidationUsingJavaTest/
$ make validation_data.cpp
```
and
```
$ cd $ACE_TIME/tests/validation/BasicValidationUsingPythonTest/
$ make validation_data.cpp
```

and so on.

There is an uber `tests/validation/Makefile` which can generate
the `validation_data.*` files for all subdirectories:
```
$ cd $ACE_TIME/tests/validation
$ make tests
```

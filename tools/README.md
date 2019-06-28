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
                /       |     pygenerator.py     javagenerator.py
               /        |           \                    \
              / ingenerator.py       v                    v
             /          |         zone_infos.py        zones.txt    java.time
            /           |         zone_policies.py         |          /
           /           / \        zone_strings.py          |         /
          /           /   \                                v        v
         /           v     \                         TestDataGenerator.java
        /  bufestimator.py |\                                |
       /     /             | \          pytz                 |
      v     v              |  \        /  |                  v
    argenerator.py         |   v      v   |          validation_data{h,cpp}
       |                   | validator.py |          validation_tests.cpp
       v                   |              /
zone_infos.{h,cpp}         \             /
zone_policies.{h,cpp}       \           /
zone_strings.{h,cpp}         \         /
                              \       /
                               v     v
                           tdgenerator.py
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
`./AceTime`. Let `$ACE_TIME` be the directory of the AceTime library:
```
$ cd $ACE_TIME/..
$ git clone https://github.com/eggert/tz
```

## Usage

### ZoneDB Files

Generating `zonedb::` files:

```
$ cd $ACE_TIME/src/ace_time/zonedb
$ ../../../tools/tzcompiler.sh --tag 2019a --action zonedb --language arduino
--scope basic --start_year 2000 --until_year 2050
```

Generating `zonedbx::` files:

```
$ cd $ACE_TIME/src/ace_time/zonedbx
$ ../../../tools/tzcompiler.sh --tag 2019a --action zonedb --language arduino
--scope extended --start_year 2000 --until_year 2050
```

### Validation Files

Generating `validation_data.*` files using Java 11 java.time library:

```
$ cd $ACE_TIME/tools/java
$ make

$ cd $ACE_TIME/tests/validation/BasicValidationUsingJavaTest/
$ ../../../tools/tzcompiler.sh --tag 2018g --scope basic --action zonedb
--language java
$ java -cp ../../../tools/java TestDataGenerator --scope basic --db_namespace
zonedb2018g --start_year 2000 --until_year 2050 < zones.txt
```

OR, just use the `Makefile`:
```
$ cd $ACE_TIME/tests/validation/BasicValidationUsingJavaTest/
$ make clean
$ make
```

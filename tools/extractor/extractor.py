# Copyright 2018 Brian T. Park
#
# MIT License.

"""
Parses the zone info files in the TZ Database into Python data structures which
can be processed by subsequent Python scripts. The zone files used by this
script are:

    africa
    antarctica
    asia
    australasia
    backward
    etcetera
    europe
    northamerica
    southamerica

The following zone files are not used:

    backzone - contains zones differing before 1970
    systemv - 'SystemV' zones

There are 3 types of entries in these files: 'Rule', 'Zone' and 'Link' entries.

1) 'Rule' entries look like the following:

# Rule  NAME    FROM    TO    TYPE IN   ON      AT      SAVE    LETTER
Rule    US      2007    max   -    Mar  Sun>=8  2:00    1:00    D
Rule    US      2007    max   -    Nov  Sun>=1  2:00    0       S

Each 'Rule' entry is mapped to a ZoneRule class and a collection of Zone Rules
with the same name is called a "Zone Policy".

2) 'Zone' entries look like this:

# Zone  NAME                STDOFF      RULES   FORMAT  [UNTIL]
Zone    America/Chicago     -5:50:36    -       LMT     1883 Nov 18 12:09:24
                            -6:00       US      C%sT    1920
                            ...
                            -6:00       US      C%sT

The UNTIL column should be monotonically increasing and the last Zone era line
has an empty UNTIL field.

Each 'Zone' entry is mapped to a ZoneEra class and a collection of ZoneEras with
the same name is called a "Zone Info".

3) The 'backward' file and other files contain 'Link' entries which are synonyms
for other 'Zone' entries. The format is:

Link {target_zone} {linked_zone}

For example:

Link    America/Los_Angeles    US/Pacific

(The order of the 2 arguments is the reverse of what I would consider natural.
Maybe it helps to think of the 'Link' command similar to the 'ln' link command
in Unix, which has the same order of arguments as the 'cp' command.)
"""

from typing import Any
from typing import Dict
from typing import List
from typing import Optional
from typing import TextIO
from typing import Tuple
import logging
import os
from zonedb.data_types import (
    ZoneRuleRaw,
    ZoneEraRaw,
    PoliciesMap,
    ZonesMap,
    LinksMap,
    MAX_UNTIL_YEAR,
    MAX_YEAR,
)


class Extractor:
    """Reads each test data section from the given file-like object (e.g.
    sys.stdin).

    Usage:

        extractor = Extractor(input_dir)
        extractor.parse()
        extractor.print_summary()
        extractor.zones_map
        extractor.policies_map
        ...
    """

    ZONE_FILES: List[str] = [
        'africa',
        'antarctica',
        'asia',
        'australasia',
        'backward',
        'etcetera',
        'europe',
        'northamerica',
        'southamerica',
    ]

    def __init__(self, input_dir: str):
        self.input_dir: str = input_dir

        self.next_line: Optional[str] = None
        self.rule_lines: Dict[str, List[str]] = {}  # ruleName to lines[]
        self.zone_lines: Dict[str, List[str]] = {}  # zoneName to lines[]
        self.link_lines: Dict[str, List[str]] = {}  # linkName to zoneName[]
        self.policies_map: PoliciesMap = {}
        self.zones_map: ZonesMap = {}
        self.links_map: LinksMap = {}
        self.ignored_rule_lines: int = 0
        self.ignored_zone_lines: int = 0
        self.ignored_link_lines: int = 0
        self.invalid_rule_lines: int = 0
        self.invalid_zone_lines: int = 0
        self.invalid_link_lines: int = 0

    def parse(self) -> None:
        """Read the zoneinfo files from TZ Database and create the 'zones_map'
        and 'policies_map'.
        * zones_map contains a map of (zone_name -> ZoneEraRaw[]).
        * rules contains a map of (policy_name -> ZoneRuleRaw[]).
        """
        self._parse_zone_files()
        self._process_rules()
        self._process_zones()
        self._process_links()

    def get_data(self) -> Tuple[PoliciesMap, ZonesMap, LinksMap]:
        """Return the extracted data maps."""
        return self.policies_map, self.zones_map, self.links_map

    def _parse_zone_files(self) -> None:
        logging.basicConfig(level=logging.INFO)
        for file_name in self.ZONE_FILES:
            full_filename = os.path.join(self.input_dir, file_name)
            logging.info('Processing %s', full_filename)
            with open(full_filename, 'r', encoding='utf-8') as f:
                self._parse_zone_file(f)

    def _parse_zone_file(self, input: TextIO) -> None:
        """Read the 'input' file and collect all 'Rule' lines into
        self.rule_lines and all 'Zone' lines into self.zone_lines.
        """
        in_zone_mode: bool = False
        # prev_tag: str = ''
        prev_name: str = ''
        while True:
            line: Optional[str] = self._read_line(input)
            if line is None:
                break

            tag: str = line[:4]
            if tag == 'Rule':
                tokens: List[str] = line.split()
                policy_name: str = tokens[1]
                _add_item(self.rule_lines, policy_name, line)
                in_zone_mode = False
            elif tag == 'Link':
                tokens = line.split()
                link_name: str = tokens[2]
                _add_item(self.link_lines, link_name, tokens[1])
                in_zone_mode = False
            elif tag == 'Zone':
                tokens = line.split()
                zone_name: str = tokens[1]
                _add_item(self.zone_lines, zone_name, ' '.join(tokens[2:]))
                in_zone_mode = True
                # prev_tag = tag
                prev_name = zone_name
            elif tag[0] == '\t' and in_zone_mode:
                # Collect subsequent lines that begin with a TAB character into
                # the current 'Zone' entry.
                _add_item(self.zone_lines, prev_name, line)

    def _process_rules(self) -> None:
        name: str
        lines: List[str]
        for name, lines in self.rule_lines.items():
            line: str
            for line in lines:
                try:
                    rule_entry: ZoneRuleRaw = _process_rule_line(line)
                    if rule_entry:
                        _add_item(self.policies_map, name, rule_entry)
                    else:
                        self.ignored_rule_lines += 1
                except Exception as e:
                    logging.exception(f'Exception {e}: {line}')
                    self.invalid_rule_lines += 1

    def _process_zones(self) -> None:
        name: str
        lines: List[str]
        for name, lines in self.zone_lines.items():
            line: str
            for line in lines:
                try:
                    zone_era: ZoneEraRaw = _process_zone_line(line)
                    if zone_era:
                        _add_item(self.zones_map, name, zone_era)
                    else:
                        self.ignored_zone_lines += 1
                except Exception as e:
                    logging.exception(f'Exception {e}: {line}')
                    self.invalid_zone_lines += 1

    def _process_links(self) -> None:
        link_name: str
        lines: List[str]
        for link_name, lines in self.link_lines.items():
            if len(lines) > 1:
                self.invalid_link_lines += len(lines)
            else:
                self.links_map[link_name] = lines[0]

    def _read_line(self, input: TextIO) -> Optional[str]:
        """Return the next line, while supporting a one-line push_back().
        Comment lines begin with a '#' character and are skipped.
        Blank lines are skipped.
        Prepending and trailing whitespaces are stripped.
        Return 'None' if EOF is reached.
        """
        if self.next_line:
            line: str = self.next_line
            self.next_line = None
            return line

        while True:
            line = input.readline()

            # EOF
            if line == '':
                return None

            # remove trailing comments
            i: int = line.find('#')
            if i >= 0:
                line = line[:i]

            # strip any trailing whitespaces
            line = line.rstrip()

            # skip any blank lines after stripping
            if not line:
                continue

            return line

    def print_summary(self) -> None:
        rule_entry_count: int = 0

        name: str
        lines: List[str]
        for name, rules in self.policies_map.items():
            rule: ZoneRuleRaw
            for rule in rules:
                rule_entry_count += 1

        zone_entry_count: int = 0
        eras: List[ZoneEraRaw]
        for name, eras in self.zones_map.items():
            era: ZoneEraRaw
            for era in eras:
                zone_entry_count += 1

        logging.info(
            'Summary: Line count (Rule, Zone, Link): ('
            f'{len(self.rule_lines)}, '
            f'{len(self.zone_lines)}, '
            f'{len(self.link_lines)})')
        logging.info(
            'Summary: Name count (Rule, Zone, Link): ('
            f'{len(self.policies_map)}, '
            f'{len(self.zones_map)}, '
            f'{len(self.links_map)})')
        logging.info(f'Summary: Rule entry count: {rule_entry_count}')
        logging.info(f'Summary: Zone entry count: {zone_entry_count}')
        logging.info(
            'Summary: Ignored lines (Rule, Zone, Link): ('
            f'{self.ignored_rule_lines}, '
            f'{self.ignored_zone_lines}, '
            f'{self.ignored_link_lines})')
        logging.info(
            'Summary: Invalid lines: (Rule, Zone, Link): ('
            f'{self.invalid_rule_lines}, '
            f'{self.invalid_zone_lines}, '
            f'{self.invalid_link_lines})')


def _add_item(table: Dict[str, List[Any]], name: str, line: Any) -> None:
    array: Optional[List[Any]] = table.get(name)
    if not array:
        array = []
        table[name] = array
    array.append(line)


MONTH_TO_MONTH_INDEX: Dict[str, int] = {
    'Jan': 1,
    'Feb': 2,
    'Mar': 3,
    'Apr': 4,
    'May': 5,
    'Jun': 6,
    'Jul': 7,
    'Aug': 8,
    'Sep': 9,
    'Oct': 10,
    'Nov': 11,
    'Dec': 12
}


def _process_rule_line(line: str) -> ZoneRuleRaw:
    """Normalize a dictionary that represents a 'Rule' line from the TZ
    database. Contains the following fields:
    Rule NAME FROM TO TYPE IN ON AT SAVE LETTER
    0    1    2    3  4    5  6  7  8    9

    These represent transitions from Daylight to/from Standard.
    """
    tokens: List[str] = line.split()

    # Check for valid year.
    from_year: int = int(tokens[2])
    to_year_string: str = tokens[3]
    if to_year_string == 'only':
        to_year: int = from_year
    elif to_year_string == 'max':
        to_year = MAX_YEAR
    else:
        to_year = int(to_year_string)

    in_month: int = MONTH_TO_MONTH_INDEX[tokens[5]]
    on_day: str = tokens[6]
    at_time, at_time_suffix = parse_at_time_string(tokens[7])
    delta_offset = tokens[8]

    # Return map corresponding to a ZoneRule instance
    return {
        'fromYear': from_year,
        'toYear': to_year,
        'inMonth': in_month,
        'onDay': on_day,
        'atTime': at_time,
        'atTimeSuffix': at_time_suffix,
        'deltaOffset': delta_offset,
        'letter': tokens[9],
        'rawLine': line,
    }


def parse_at_time_string(at_string: str) -> Tuple[str, str]:
    """Parses the '2:00s' string into '2:00' and 's'. If there is no suffix,
    returns a '' as the suffix.
    """
    suffix: str = at_string[-1:]
    if suffix.isdigit():
        suffix = ''
        at_time: str = at_string
    else:
        at_time = at_string[:-1]
    if suffix not in ['', 'w', 's', 'u', 'g', 'z']:
        raise Exception(f'Invalid AT suffix ({suffix})')
    return (at_time, suffix)


def _process_zone_line(line: str) -> ZoneEraRaw:
    """Normalize an zone era from dictionary that represents one line of
    a 'Zone' record. The columns are:
    STDOFF   RULES  FORMAT  [UNTIL]
    0        1      2       3
    -5:50:36 -      LMT     1883 Nov 18 12:09:24
    -6:00    US     C%sT    1920
    """
    tokens: List[str] = line.split()

    # STDOFF
    offset_string: str = tokens[0]

    # 'RULES' field can be:
    rules_string: str = tokens[1]

    # check 'until' year
    if len(tokens) >= 4:
        until_year: int = int(tokens[3])
    else:
        until_year = MAX_UNTIL_YEAR

    # check for additional components of 'UNTIL' field
    if len(tokens) >= 5:
        until_year_only: bool = False
        until_month: int = MONTH_TO_MONTH_INDEX[tokens[4]]
    else:
        until_year_only = True
        until_month = 1

    if len(tokens) >= 6:
        until_day: str = tokens[5]
    else:
        until_day = '1'

    if len(tokens) >= 7:
        (until_time, until_time_suffix) = parse_at_time_string(tokens[6])
    else:
        until_time = '00:00'
        until_time_suffix = 'w'

    # FORMAT
    format: str = tokens[2]

    # Return map corresponding to a ZoneEra instance
    return {
        'offsetString': offset_string,
        'rules': rules_string,
        'format': format,
        'untilYear': until_year,
        'untilYearOnly': until_year_only,
        'untilMonth': until_month,
        'untilDayString': until_day,
        'untilTime': until_time,
        'untilTimeSuffix': until_time_suffix,
        'rawLine': line,
    }

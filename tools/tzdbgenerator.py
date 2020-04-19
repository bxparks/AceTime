# Copyright 2020 Brian T. Park
#
# MIT License

import os
import logging
import json
from extractor import ZonesMap
from extractor import RulesMap
from extractor import LinksMap
from transformer import CommentsMap
from transformer import StringCollection
from typing import List
from typing_extensions import TypedDict


# The internal representation of the TZ Database zone files.
class TzDb(TypedDict):
    # Context data.
    tz_version: str
    tz_files: List[str]
    scope: str
    start_year: int
    until_year: int
    until_at_granularity: int
    offset_granularity: int
    strict: bool

    # Data from Extractor filtered through Transformer
    zones_map: ZonesMap
    links_map: LinksMap
    rules_map: RulesMap

    # Data from Transformer
    removed_zones: CommentsMap
    removed_links: CommentsMap
    removed_policies: CommentsMap
    notable_zones: CommentsMap
    notable_links: CommentsMap
    notable_policies: CommentsMap
    format_strings: StringCollection
    zone_strings: StringCollection


class TzDbGenerator:
    """Collect the various data structures from parsing the TZ Database files
    into a single TzDb data structure which can easily be exported as a JSON
    file. Downstream processors (e.g. pygenerator.py, zonelistgenerator.py,
    argenerator.py) can consume this single data structure to generate ZoneInfo
    files for any other languages or targets.
    """

    # The output of this is called "tzdb.json" because it is the JSON
    # representation of the original TZ Database files. I don't want to call it
    # "zonedb.json" because 'zonedb' refers to the generated 'zone_infos.*' and
    # 'zone_polcies.*' files relevant to a specific target language, (e.g.
    # Arduino, Python).
    _OUTPUT_FILE = 'tzdb.json'

    def __init__(
        self,
        tz_version: str,
        tz_files: List[str],
        scope: str,
        start_year: int,
        until_year: int,
        until_at_granularity: int,
        offset_granularity: int,
        strict: bool,
        zones_map: ZonesMap,
        links_map: LinksMap,
        rules_map: RulesMap,
        removed_zones: CommentsMap,
        removed_links: CommentsMap,
        removed_policies: CommentsMap,
        notable_zones: CommentsMap,
        notable_links: CommentsMap,
        notable_policies: CommentsMap,
        format_strings: StringCollection,
        zone_strings: StringCollection,
    ):
        self.tzdb: TzDb = {
            # Context data.
            'tz_version': tz_version,
            'tz_files': tz_files,
            'scope': scope,
            'start_year': start_year,
            'until_year': until_year,
            'until_at_granularity': until_at_granularity,
            'offset_granularity': offset_granularity,
            'strict': strict,

            # Data from Extractor filtered through Transformer.
            'zones_map': zones_map,
            'links_map': links_map,
            'rules_map': rules_map,

            # Data from Transformer.
            'removed_zones': removed_zones,
            'removed_links': removed_links,
            'removed_policies': removed_policies,

            'notable_zones': notable_zones,
            'notable_links': notable_links,
            'notable_policies': notable_policies,

            'format_strings': format_strings,
            'zone_strings': zone_strings,
        }

    def get_data(self) -> TzDb:
        return self.tzdb

    def generate_files(self, output_dir: str) -> None:
        full_filename = os.path.join(output_dir, self._OUTPUT_FILE)
        with open(full_filename, 'w', encoding='utf-8') as output_file:
            json.dump(self.tzdb, output_file, indent=2)
            print(file=output_file)  # add terminating newline
        logging.info("Created %s", full_filename)

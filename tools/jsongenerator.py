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


class ZoneDb(TypedDict):
    # Context data.
    invocation: str
    tz_version: str
    tz_files: List[str]
    scope: str
    start_year: int
    until_year: int

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


class JsonGenerator:
    """Generate a JSON output to the STDOUT that contains the result of the
    Transformer and other meta information, which will allow downstream
    processors to generate ZoneInfo files for any other languages or targets.
    Existing generators (e.g. pygenerator.py, zonelistgenerator.py, even
    argenerator.py) could potentially be rewritten to consume the JSON file
    instead of the inlined Pythnon data structures.
    """

    # The output of this is called "tzdb.json" because it is the JSON
    # representation of that closely matches the original TZ Database files. I
    # don't want to call it "zonedb.json" because 'zonedb' refers to the
    # 'zone_infos.*' and 'zone_polcies.*' files which are the processed files
    # relevant to a specific target language, (e.g. Arduino, Python).
    _OUTPUT_FILE = 'tzdb.json'

    def __init__(
        self,
        invocation: str,
        tz_version: str,
        tz_files: List[str],
        scope: str,
        start_year: int,
        until_year: int,
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
        self.zonedb: ZoneDb = {
            # Context data.
            'invocation': invocation,
            'tz_version': tz_version,
            'tz_files': tz_files,
            'scope': scope,
            'start_year': start_year,
            'until_year': until_year,

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

    def generate_files(self, output_dir: str) -> None:
        full_filename = os.path.join(output_dir, self._OUTPUT_FILE)
        with open(full_filename, 'w', encoding='utf-8') as output_file:
            json.dump(self.zonedb, output_file, indent=2)
        logging.info("Created %s", full_filename)

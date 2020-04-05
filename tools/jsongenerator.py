# Copyright 2020 Brian T. Park
#
# MIT License

import os
import logging
import json
from collections import OrderedDict
from extractor import ZonesMap
from extractor import RulesMap
from extractor import LinksMap
from transformer import CommentsMap
from transformer import StringCollection
from typing import Any
from typing import Dict
from typing import List


class JsonGenerator:
    """Generate a JSON output to the STDOUT that contains the result of the
    Transformer and other meta information, which will allow downstream
    processors to generate ZoneInfo files for any other languages or targets.
    Existing generators (e.g. pygenerator.py, zonelistgenerator.py, even
    argenerator.py) could potentially be rewritten to consume the JSON file
    instead of the inlined Pythnon data structures.
    """

    _OUTPUT_FILE = 'zonedb.json'

    def __init__(
        self,
        invocation: str,
        tz_version: str,
        tz_files: List[str],
        scope: str,
        db_namespace: str,
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
        o: 'OrderedDict[str, Any]' = OrderedDict()

        # Metadata
        o['invocation'] = invocation
        o['tz_version'] = tz_version
        o['tz_files'] = tz_files
        o['scope'] = scope
        o['db_namespace'] = db_namespace
        o['start_year'] = start_year
        o['until_year'] = until_year

        # Data from Extractor filtered through Transformer
        o['rules_map'] = rules_map
        o['zones_map'] = zones_map
        o['links_map'] = links_map

        # Added data from Transformer
        o['removed_policies'] = self.convert_comment_map(removed_policies)
        o['removed_zones'] = self.convert_comment_map(removed_zones)
        o['removed_links'] = self.convert_comment_map(removed_links)
        o['format_strings'] = format_strings
        o['zone_strings'] = zone_strings

        self.out = o

    @staticmethod
    def convert_comment_map(m: CommentsMap) -> Dict[str, List[str]]:
        """Convert a CommentMap into a Dict which can be serialized to JSON."""
        d: Dict[str, List[str]] = {}
        for k, vals in m.items():
            d[k] = list(vals)
        return d

    def generate_files(self, output_dir: str) -> None:
        full_filename = os.path.join(output_dir, self._OUTPUT_FILE)
        with open(full_filename, 'w', encoding='utf-8') as output_file:
            json.dump(self.out, output_file, indent=2)
        logging.info("Created %s", full_filename)

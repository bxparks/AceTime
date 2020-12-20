# Copyright 2020 Brian T. Park
#
# MIT License

import os
import logging
import json
from zonedb.data_types import ZoneInfoDatabase
from zonedb.data_types import CommentsMap


class JsonGenerator:
    """Generate the JSON representation of the ZoneInfoDatabase named
    'zonedb.json'.
    """
    _OUTPUT_FILE = 'zonedb.json'

    def __init__(self, zidb: ZoneInfoDatabase):
        self.zidb = zidb

        # Convert Set[str] to List[] so that it can be converted into JSON.
        self._convert_to_list('removed_zones')
        self._convert_to_list('removed_policies')
        self._convert_to_list('removed_links')
        self._convert_to_list('notable_zones')
        self._convert_to_list('notable_policies')
        self._convert_to_list('notable_links')

    def generate_files(self, output_dir: str) -> None:
        """Serialize ZoneInfoDatabase to a zonedb.json file."""
        full_filename = os.path.join(output_dir, self._OUTPUT_FILE)
        with open(full_filename, 'w', encoding='utf-8') as output_file:
            json.dump(self.zidb, output_file, indent=2)
            print(file=output_file)  # add terminating newline
        logging.info("Created %s", full_filename)

    def _convert_to_list(self, param: str) -> None:
        # Mypy does not support a variable key, so bypass the type check.
        # See https://github.com/python/mypy/issues/7178
        collection: CommentsMap = self.zidb[param]  # type: ignore
        self.zidb[param] = {  # type: ignore
            k: list(v)
            for k, v in collection.items()
        }

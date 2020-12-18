# Copyright 2020 Brian T. Park
#
# MIT License

import os
import logging
import json
from zonedb.data_types import ZoneInfoDatabase


class JsonGenerator:
    """Generate the JSON representation of the ZoneInfoDatabase named
    'zonedb.json'.
    """
    _OUTPUT_FILE = 'zonedb.json'

    def __init__(
        self,
        zidb: ZoneInfoDatabase,
    ):
        self.zidb = zidb

    def generate_files(self, output_dir: str) -> None:
        full_filename = os.path.join(output_dir, self._OUTPUT_FILE)
        with open(full_filename, 'w', encoding='utf-8') as output_file:
            json.dump(self.zidb, output_file, indent=2)
            print(file=output_file)  # add terminating newline
        logging.info("Created %s", full_filename)

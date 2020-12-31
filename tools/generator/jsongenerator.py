# Copyright 2020 Brian T. Park
#
# MIT License

import os
import logging
import json
from data_types.at_types import ZoneInfoDatabase


class JsonGenerator:
    """Generate the JSON representation of the ZoneInfoDatabase to the given
    'json_file'.
    """
    def __init__(
        self,
        zidb: ZoneInfoDatabase,
        json_file: str
    ):
        self.zidb = zidb
        self.json_file = json_file

    def generate_files(self, output_dir: str) -> None:
        """Serialize ZoneInfoDatabase to the specified file."""
        full_filename = os.path.join(output_dir, self.json_file)
        with open(full_filename, 'w', encoding='utf-8') as output_file:
            json.dump(self.zidb, output_file, indent=2)
            print(file=output_file)  # add terminating newline
        logging.info("Created %s", full_filename)

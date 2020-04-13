# Copyright 2020 Brian T. Park
#
# MIT License
"""
Generate the Arduino validation data for tests/validation/* unit tests as a JSON
file. The file format is:
{
  'start_year': int,
  'until_year': int,
  'source': str,
  'version': str,
  'test_data': [
    {
      'epoch': int,
      'total_offset': int,
      'dst_offset': int,
      'y': int,
      'M': int,
      'd': int,
      'h': int,
      'm': int,
      's': int,
      'type', str,
    }
  ]
}
"""

import logging
import json
import os
from validation.data import ValidationData


class JsonValidationGenerator:
    def __init__(
        self,
        invocation: str,
        tz_version: str,
        validation_data: ValidationData,
    ):
        self.invocation = invocation
        self.tz_version = tz_version
        self.validation_data = validation_data

        self.test_data = validation_data['test_data']
        self.filename = 'validation_data.json'

    def generate_files(self, output_dir: str) -> None:
        full_filename = os.path.join(output_dir, self.filename)
        with open(full_filename, 'w', encoding='utf-8') as output_file:
            json.dump(self.validation_data, output_file, indent=2)
            print(file=output_file)  # add terminating newline
        logging.info("Created %s", full_filename)

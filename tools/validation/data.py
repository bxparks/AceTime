# Copyright 2020 Brian T. Park
#
# MIT License
#
"""
Data types for representing the time zone validation data. The ValidationData
type can be serialized to JSON directly. The JSON looks like:
{
  'start_year': int,
  'until_year': int,
  'source': str,
  'version': str,
  'has_abbrev': bool,
  'has_valid_dst': bool,
  'test_data': {
    '{zone_name}: [
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
      },
      [...]
    ],
    [...]
  }
}
"""

from typing import List
from typing import Dict
from typing import Optional
from typing_extensions import TypedDict

# An entry in the test data set.
# Each TestData is annotated with a 'type' as:
# * 'A': pre-transition
# * 'B': post-transition
# * 'S': a monthly test sample
# * 'Y': end of year test sample
TestItem = TypedDict("TestItem", {
    'epoch': int,  # seconds from AceTime epoch of 2000-01-01T00:00:00Z
    'total_offset': int,  # total UTC offset in seconds
    'dst_offset': int,  # DST offset in seconds
    'y': int,
    'M': int,
    'd': int,
    'h': int,
    'm': int,
    's': int,
    'abbrev': Optional[str],
    'type': str,
})

# The test data set (epoch -> timezone info)
TestData = Dict[str, List[TestItem]]

# The top-level validation data collection. This can be serialized to JSON.
ValidationData = TypedDict('ValidationData', {
    'start_year': int,
    'until_year': int,
    'source': str,
    'version': str,
    'has_abbrev': bool,  # 'abbrev' values are defined
    'has_valid_dst': bool,  # DST offsets are reliable
    'test_data': TestData,
})

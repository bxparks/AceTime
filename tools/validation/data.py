# Copyright 2020 Brian T. Park
#
# MIT License
#
# Data types for representing the time zone validation data.

from typing import List
from typing import Dict
from typing_extensions import TypedDict

# An entry in the test data set.
# Each TestData is annotated with a 'type' as:
# * 'A': pre-transition
# * 'B': post-transition
# * 'S': a monthly test sample
# * 'Y': end of year test sample
TestItem = TypedDict("TestItem", {
    'epoch': int,
    'total_offset': int,
    'dst_offset': int,
    'y': int,
    'M': int,
    'd': int,
    'h': int,
    'm': int,
    's': int,
    'type': str,
})

# The test data set (epoch -> timezone info)
TestData = Dict[str, List[TestItem]]

ValidationData = TypedDict('ValidationData', {
    'start_year': int,
    'until_year': int,
    'source': str,
    'version': str,
    'test_data': TestData,
})

# Copyright 2020 Brian T. Park
#
# MIT License
#
# Generate validation TestData using pytz.

import logging
from datetime import datetime
from datetime import timedelta
from datetime import tzinfo
import pytz
from typing import List
from typing import NamedTuple
from typing import Tuple
from typing import List
from typing import Dict

# Number of seconds from Unix Epoch (1970-01-01 00:00:00) to AceTime Epoch
# (2000-01-01 00:00:00)
SECONDS_SINCE_UNIX_EPOCH = 946684800

# An entry in the test data set.
TestItem = NamedTuple("TestItem", [
    ('epoch', int),
    ('total_offset', int),
    ('dst_offset', int),
    ('y', int),
    ('M', int),
    ('d', int),
    ('h', int),
    ('m', int),
    ('s', int),
    ('type', str),
])

# The test data set (epoch -> timezone info)
TestData = Dict[str, List[TestItem]]

TransitionTimes = Tuple[datetime, datetime]

class TestDataGenerator():
    # Look for a UTC offset transition every 12 hours
    _SAMPLING_INTERVAL = timedelta(hours=12)

    def __init__(
        self,
        start_year: int,
        until_year: int,
    ):
        self.start_year = start_year
        self.until_year = until_year

    def create_test_data(self, zones: List[str]) -> TestData:
        test_data: TestData = {}
        for zone_name in zones:
            test_items = self._create_test_items(zone_name)
            test_data[zone_name] = test_items
        return test_data

    def _create_test_items(self, zone_name: str) -> List[TestItem]:
        logging.info(f"_create_test_items(): {zone_name}")
        test_items: List[TestItem] = []
        transitions = self._find_transitions(zone_name)
        for (left, right) in transitions:
            left_item = self._create_test_item(left, 'A')
            right_item = self._create_test_item(right, 'B')
            test_items.append(left_item)
            test_items.append(right_item)

        return test_items

    def _find_transitions(self, zone_name: str,) -> List[TransitionTimes]:
        """Find the DST transition using pytz by sampling the time period from
        [start_year, until_year] in 12-hour samples.
        """
        tz = pytz.timezone(zone_name)
        dt = datetime(self.start_year, 1, 1, 0, 0, 0, tzinfo=pytz.utc)
        dt_local = dt.astimezone(tz)

        # Check every 12 hours for a transition
        transitions: List[TransitionTimes] = []
        while True:
            next_dt = dt + self._SAMPLING_INTERVAL
            next_dt_local = next_dt.astimezone(tz)
            if next_dt.year > self.until_year: break

            # Check for a change in UTC offset
            if dt_local.utcoffset() != next_dt_local.utcoffset():
                #print(f'Transition between {dt_local} and {next_dt_local}')
                dt_left, dt_right = self.binary_search_transition(
                    tz, dt, next_dt)
                dt_left_local = dt_left.astimezone(tz)
                dt_right_local = dt_right.astimezone(tz)
                transitions.append((dt_left_local, dt_right_local))

            dt = next_dt
            dt_local = next_dt_local

        return transitions

    @staticmethod
    def binary_search_transition(
        tz: tzinfo,
        dt_left: datetime,
        dt_right: datetime,
    ) -> Tuple[datetime, datetime]:
        """Do a binary search to find the exact transition times, to within 1
        minute accuracy. The dt_left and dt_right are 12 hours (720 minutes)
        apart. So the binary search should take a maximum of 10 iterations to
        find the DST transition within one adjacent minute.
        """
        dt_left_local = dt_left.astimezone(tz)
        dt_right_local = dt_right.astimezone(tz)
        while True:
            delta_minutes = int((dt_right - dt_left) / timedelta(minutes=1))
            delta_minutes //= 2
            #print(f' delta_minutes={delta_minutes}')
            if delta_minutes == 0: break

            dt_mid = dt_left + timedelta(minutes=delta_minutes)
            mid_dt_local = dt_mid.astimezone(tz)
            if dt_left_local.utcoffset() == mid_dt_local.utcoffset():
                #print(' right half')
                dt_left = dt_mid
                dt_left_local = mid_dt_local
            else:
                #print(' left half')
                dt_right = dt_mid
                dt_right_local = mid_dt_local

        return dt_left, dt_right

    def _create_test_item(self, dt: datetime, type: str) -> TestItem:
        unix_seconds = int(dt.timestamp())
        epoch_seconds = unix_seconds - SECONDS_SINCE_UNIX_EPOCH
        total_offset = int(dt.utcoffset().total_seconds()) # type: ignore
        dst_offset = int(dt.dst().total_seconds()) # type: ignore

        return TestItem(
            epoch=epoch_seconds,
            total_offset=total_offset,
            dst_offset=dst_offset,
            y=dt.year,
            M=dt.month,
            d=dt.day,
            h=dt.hour,
            m=dt.minute,
            s=dt.second,
            type=type)
            


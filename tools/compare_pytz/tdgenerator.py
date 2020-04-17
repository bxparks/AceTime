# Copyright 2020 Brian T. Park
#
# MIT License
#
# Generate validation TestData using pytz.

import logging
from datetime import datetime
from datetime import timedelta
import pytz
from typing import Any
from typing import Tuple
from typing import List
from typing import Dict
from typing import Optional
from validation.data import (TestItem, TestData, ValidationData)

# Number of seconds from Unix Epoch (1970-01-01 00:00:00) to AceTime Epoch
# (2000-01-01 00:00:00)
SECONDS_SINCE_UNIX_EPOCH = 946684800

# The [start, until) time interval used to search for DST transitions.
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

    def create_test_data(self, zones: List[str]) -> None:
        test_data: TestData = {}
        for zone_name in zones:
            test_items = self._create_test_items_for_zone(zone_name)
            if test_items:
                test_data[zone_name] = test_items
        self.test_data = test_data

    def get_validation_data(self) -> ValidationData:
        return {
            'start_year': self.start_year,
            'until_year': self.until_year,
            'source': 'pytz',
            'version': str(pytz.__version__),  # type: ignore
            'has_abbrev': False,
            'test_data': self.test_data,
        }

    def _create_test_items_for_zone(
        self,
        zone_name: str,
    ) -> Optional[List[TestItem]]:
        logging.info(f"_create_test_items(): {zone_name}")
        try:
            tz = pytz.timezone(zone_name)
        except pytz.UnknownTimeZoneError:
            logging.error(f"Zone '{zone_name}' not found in pytz package")
            return None

        items_map: Dict[int, TestItem] = {}
        self._add_test_items_for_transitions(items_map, tz)
        self._add_test_items_for_samples(items_map, tz)

        # Return the TestItems ordered by epoch
        return [items_map[x] for x in sorted(items_map)]

    def _add_test_items_for_samples(
        self,
        items_map: Dict[int, TestItem],
        tz: Any,
    ) -> None:
        """Add a TestItem for each month from start_year to until_year."""

        for year in range(self.start_year, self.until_year):
            for month in range(1, 13):
                # Add a sample test point on the first of each month
                dt_wall = datetime(year, month, 1, 0, 0, 0)
                dt_local = tz.localize(dt_wall)
                dt_local = tz.normalize(dt_local)
                item = self._create_test_item(dt_local, 'S')
                self._add_test_item(items_map, item)

            # Add a sample test point at the end of the year.
            dt_wall = datetime(year, 12, 31, 23, 59, 0)
            dt_local = tz.localize(dt_wall)
            dt_local = tz.normalize(dt_local)
            item = self._create_test_item(dt_local, 'Y')
            self._add_test_item(items_map, item)

    def _add_test_items_for_transitions(
        self,
        items_map: Dict[int, TestItem],
        tz: Any,
    ) -> None:
        """Add DST transitions, using 'A' and 'B' designators"""

        transitions = self._find_transitions(tz)
        for (left, right) in transitions:
            left_item = self._create_test_item(left, 'A')
            self._add_test_item(items_map, left_item)

            right_item = self._create_test_item(right, 'B')
            self._add_test_item(items_map, right_item)

    def _find_transitions(self, tz: Any) -> List[TransitionTimes]:
        """Find the DST transition using pytz by sampling the time period from
        [start_year, until_year] in 12-hour samples.
        """
        # TODO: Do I need to start 1 day before Jan 1 UTC, in case the
        # local time is ahead of UTC?
        dt = datetime(self.start_year, 1, 1, 0, 0, 0, tzinfo=pytz.utc)
        dt_local = dt.astimezone(tz)

        # Check every 12 hours for a transition
        transitions: List[TransitionTimes] = []
        while True:
            next_dt = dt + self._SAMPLING_INTERVAL
            next_dt_local = next_dt.astimezone(tz)
            if next_dt.year > self.until_year:
                break

            # Check for a change in UTC offset
            if dt_local.utcoffset() != next_dt_local.utcoffset():
                # print(f'Transition between {dt_local} and {next_dt_local}')
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
        tz: Any,
        dt_left: datetime,
        dt_right: datetime,
    ) -> Tuple[datetime, datetime]:
        """Do a binary search to find the exact transition times, to within 1
        minute accuracy. The dt_left and dt_right are 12 hours (720 minutes)
        apart. So the binary search should take a maximum of 10 iterations to
        find the DST transition within one adjacent minute.
        """
        dt_left_local = dt_left.astimezone(tz)
        while True:
            delta_minutes = int((dt_right - dt_left) / timedelta(minutes=1))
            delta_minutes //= 2
            if delta_minutes == 0:
                break

            dt_mid = dt_left + timedelta(minutes=delta_minutes)
            mid_dt_local = dt_mid.astimezone(tz)
            if dt_left_local.utcoffset() == mid_dt_local.utcoffset():
                dt_left = dt_mid
                dt_left_local = mid_dt_local
            else:
                dt_right = dt_mid

        return dt_left, dt_right

    @staticmethod
    def _create_test_item(dt: datetime, tag: str) -> TestItem:
        """Create a TestItem from a datetime."""
        unix_seconds = int(dt.timestamp())
        epoch_seconds = unix_seconds - SECONDS_SINCE_UNIX_EPOCH
        total_offset = int(dt.utcoffset().total_seconds())  # type: ignore
        dst_offset = int(dt.dst().total_seconds())  # type: ignore

        return {
            'epoch': epoch_seconds,
            'total_offset': total_offset,
            'dst_offset': dst_offset,
            'y': dt.year,
            'M': dt.month,
            'd': dt.day,
            'h': dt.hour,
            'm': dt.minute,
            's': dt.second,
            'type': tag,
        }

    @staticmethod
    def _add_test_item(items_map: Dict[int, TestItem], item: TestItem) -> None:
        current = items_map.get(item['epoch'])
        if current:
            # If a duplicate TestItem exists for epoch, then check that the
            # data is exactly the same.
            if (current['total_offset'] != item['total_offset']
                    or current['dst_offset'] != item['dst_offset']
                    or current['y'] != item['y'] or current['M'] != item['M']
                    or current['d'] != item['d'] or current['h'] != item['h']
                    or current['m'] != item['m'] or current['s'] != item['s']):
                raise Exception('Item %s does not match item %s' % (
                    current, item))
            # 'A' and 'B' takes precedence over 'S' or 'Y'
            if item['type'] in ['A', 'B']:
                items_map[item['epoch']] = item
        else:
            items_map[item['epoch']] = item

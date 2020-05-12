# Copyright 2020 Brian T. Park
#
# MIT License

"""
Generate validation TestData using 'dateutil' package. This version was derived
from the 'validation.tdgenerator' module with the critical difference that it
does not pull in the ZoneSpecifier to determine the DST transition times. This
means that this module also avoids pulling in the ZoneInfo, ZonePolicy and other
related classes from 'extractor' and 'transformer' processing pipeline.
Therefore, this module is truly dependent on only the 'dateutil' package.
"""

import logging
from datetime import datetime, timedelta
import dateutil
from dateutil.tz import gettz, resolve_imaginary, UTC
from typing import Any, Tuple, List, Dict, Optional
from validation.data import TestItem, TestData, ValidationData

# Number of seconds from Unix Epoch (1970-01-01 00:00:00) to AceTime Epoch
# (2000-01-01 00:00:00)
SECONDS_SINCE_UNIX_EPOCH = 946684800

# The [start, until) time interval used to search for DST transitions,
# and flag that is True if ONLY the DST changed.
TransitionTimes = Tuple[datetime, datetime, bool]


class TestDataGenerator():
    def __init__(
        self,
        start_year: int,
        until_year: int,
        sampling_interval: int,
        detect_dst_transition: bool = True,
    ):
        """If detect_dst_transition is set to True, changes in the DST offset
        will be considered to be a time offset transition. Enabling this will
        cause additional test data points to be generated, but often they will
        conflict with the DST offsets calculated by AceTime or HinnantDate
        library. In other words, I think dateutil is incorrect for those DST
        transitions.
        """
        self.start_year = start_year
        self.until_year = until_year
        self.sampling_interval = timedelta(hours=sampling_interval)
        self.detect_dst_transition = detect_dst_transition

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
            'source': 'dateutil',
            'version': str(dateutil.__version__),  # type: ignore
            'has_valid_abbrev': True,
            'has_valid_dst': True,
            'test_data': self.test_data,
        }

    def _create_test_items_for_zone(
        self,
        zone_name: str,
    ) -> Optional[List[TestItem]]:
        logging.info(f"_create_test_items(): {zone_name}")
        tz = gettz(zone_name)
        if not tz:
            logging.error(f"Zone '{zone_name}' not found in dateutil package")
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
                dt_local = resolve_imaginary(
                    datetime(year, month, 1, 0, 0, 0, tzinfo=tz)
                )
                item = self._create_test_item(dt_local, 'S')
                self._add_test_item(items_map, item)

            # Add a sample test point at the end of the year.
            dt_local = resolve_imaginary(
                datetime(year, 12, 31, 23, 59, 0, tzinfo=tz)
            )
            item = self._create_test_item(dt_local, 'Y')
            self._add_test_item(items_map, item)

    def _add_test_items_for_transitions(
        self,
        items_map: Dict[int, TestItem],
        tz: Any,
    ) -> None:
        """Add DST transitions, using 'A' and 'B' designators"""

        transitions = self._find_transitions(tz)
        for (left, right, only_dst) in transitions:
            left_item = self._create_test_item(
                left, 'a' if only_dst else 'A')
            self._add_test_item(items_map, left_item)

            right_item = self._create_test_item(
                right, 'b' if only_dst else 'B')
            self._add_test_item(items_map, right_item)

    def _find_transitions(self, tz: Any) -> List[TransitionTimes]:
        """Find the DST transition using dateutil by sampling the time period
        from [start_year, until_year].
        """
        # TODO: Do I need to start 1 day before Jan 1 UTC, in case the
        # local time is ahead of UTC?
        dt = datetime(self.start_year, 1, 1, 0, 0, 0, tzinfo=UTC)
        dt_local = dt.astimezone(tz)

        # Check every 'sampling_interval' hours for a transition
        transitions: List[TransitionTimes] = []
        while True:
            next_dt = dt + self.sampling_interval
            next_dt_local = next_dt.astimezone(tz)
            if next_dt.year >= self.until_year:
                break

            # Look for a UTC or DST transition.
            if self.is_transition(dt_local, next_dt_local):
                # print(f'Transition between {dt_local} and {next_dt_local}')
                dt_left, dt_right = self.binary_search_transition(
                    tz, dt, next_dt)
                dt_left_local = dt_left.astimezone(tz)
                dt_right_local = dt_right.astimezone(tz)
                only_dst = self.only_dst(dt_left_local, dt_right_local)
                transitions.append((dt_left_local, dt_right_local, only_dst))

            dt = next_dt
            dt_local = next_dt_local

        return transitions

    def is_transition(self, dt1: datetime, dt2: datetime) -> bool:
        """Determine if dt1 -> dt2 is a UTC offset transition. If
        detect_dst_transition is True, then also detect DST offset transition.
        """
        if dt1.utcoffset() != dt2.utcoffset():
            return True
        if self.detect_dst_transition:
            return dt1.dst() != dt2.dst()
        return False

    def only_dst(self, dt1: datetime, dt2: datetime) -> bool:
        """Determine if dt1 -> dt2 is only a DST transition."""
        if not self.detect_dst_transition:
            return False
        return dt1.utcoffset() == dt2.utcoffset() and dt1.dst() != dt2.dst()

    def binary_search_transition(
        self,
        tz: Any,
        dt_left: datetime,
        dt_right: datetime,
    ) -> Tuple[datetime, datetime]:
        """Do a binary search to find the exact transition times, to within 1
        minute accuracy. The dt_left and dt_right are 22 hours (1320 minutes)
        apart. So the binary search should take a maximum of 11 iterations to
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
            if self.is_transition(dt_left_local, mid_dt_local):
                dt_right = dt_mid
            else:
                dt_left = dt_mid
                dt_left_local = mid_dt_local

        return dt_left, dt_right

    @staticmethod
    def _create_test_item(dt: datetime, tag: str) -> TestItem:
        """Create a TestItem from a datetime."""
        unix_seconds = int(dt.timestamp())
        epoch_seconds = unix_seconds - SECONDS_SINCE_UNIX_EPOCH
        total_offset = int(dt.utcoffset().total_seconds())  # type: ignore
        dst_offset = int(dt.dst().total_seconds())  # type: ignore

        # See https://stackoverflow.com/questions/5946499 for more info on how
        # to extract the abbreviation. dt.tzinfo will never be None because the
        # timezone will always be defined.
        assert dt.tzinfo is not None
        abbrev = dt.tzinfo.tzname(dt)

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
            'abbrev': abbrev,
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

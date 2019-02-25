# Copyright 2019 Brian T. Park
#
# MIT License

import logging
import datetime
import collections
import pytz
from zone_specifier import ZoneSpecifier
from zone_specifier import SECONDS_SINCE_UNIX_EPOCH

# An entry in the test data set.
TestItem = collections.namedtuple(
    "TestItem", "epoch utc_offset dst_offset y M d h m s type")


class TestDataGenerator:
    """Generate the validation test data using the Transitions determined by
    ZoneSpecifier and the UTC offsets determined by pytz. This gives us
    stability which we can use to test other versions of ZoneSpecifier.
    """

    def __init__(self, zone_infos, zone_policies):
        """
        Args:
            zone_infos: (dict) of {name -> zone_info{} }
            zone_policies: (dict) of {name ->zone_policy{} }
        """
        self.zone_infos = zone_infos
        self.zone_policies = zone_policies
        self.zone_name = ''
        self.viewing_months = 14
        self.start_year = 2000
        self.end_year = 2019

    def create_test_data(self):
        """Create a map of {
            zone_short_name: [ TestItem() ]
        }
        Return (test_data, num_items).
        """
        test_data = {}
        num_items = 0
        for zone_short_name, zone_info in sorted(self.zone_infos.items()):
            if self.zone_name != '' and zone_short_name != self.zone_name:
                continue
            test_items = self.create_test_data_for_zone(
                zone_short_name, zone_info)
            if test_items:
                test_data[zone_short_name] = test_items
                num_items += len(test_items)
        return (test_data, num_items)

    def create_test_data_for_zone(self, zone_short_name, zone_info):
        """Create the TestItems for a specific zone.
        """
        zone_specifier = ZoneSpecifier(zone_info)
        zone_full_name = zone_info['name']
        try:
            tz = pytz.timezone(zone_full_name)
        except:
            logging.error("Zone '%s' not found in Python pytz package",
                          zone_full_name)
            return None

        return self.create_transition_test_items(tz, zone_specifier)

    def create_transition_test_items(self, tz, zone_specifier):
        """Create a TestItem for the tz for each Transition instance found by
        the ZoneSpecifier, for each year from start_year to end_year, inclusive.

        If the ZoneSpecifier is created with viewing_months=13, the first
        Transition occurs at the year boundary. The ZoneSpecifier has no prior
        Transition for 'epoch_seconds - 1', so a call to ZoneSpecifier for
        this will probably fail.

        If the ZoneSpecifier is created with viewing_months=14, then a
        Transition is created for Dec 1 of the prior year, so all of the test
        data points below will probably work.

        Some zones do not use DST, so we generate a test data point for
        Jan 1 and Dec 31 of every year to make sure that every year has
        something.
        """
        items = []
        for year in range(self.start_year, self.end_year + 1):
            # Skip start_year when viewing months is 36, because it needs data
            # for (start_year - 3), but ZoneSpecifier won't generate data for
            # years that old.
            if self.viewing_months == 36 and year == self_start_year: continue

            # Check the transition at the beginning of year.
            test_item = self.create_test_item_from_datetime(
                tz, year, month=1, day=1, hour=0, type='Y')
            items.append(test_item)

            # Check the transition at the end of the year.
            test_item = self.create_test_item_from_datetime(
                tz, year, month=12, day=31, hour=23, type='Y')
            items.append(test_item)

            # Add samples just before and just after the DST transition.
            zone_specifier.init_for_year(year)
            transition_found = False
            for transition in zone_specifier.transitions:
                # Some Transitions from ZoneSpecifier are in previous or post
                # years (e.g. viewing_months = [14, 36]), so skip those.
                start = transition.startDateTime
                transition_year = start.y
                if transition_year != year: continue

                # If viewing_months== (13 or 36), don't look at Transitions at
                # the beginning of the year since those have been already added.
                if self.viewing_months in [13, 36]:
                    if start.M == 1 and start.d == 1 and start.ss == 0:
                        continue

                epoch_seconds = transition.startEpochSecond
                items.append(
                    self.create_test_item_from_epoch_seconds(
                        tz, epoch_seconds - 1, 'A'))
                items.append(
                    self.create_test_item_from_epoch_seconds(
                        tz, epoch_seconds, 'B'))
                transition_found = True

            # If no transition found within the year, add a test sample
            # so that there's at least one sample per year.
            if not transition_found:
                items.append(
                    self.create_test_item_from_datetime(
                        tz, year, month=3, day=10, hour=2, type='S'))

        return items

    def create_test_item_from_datetime(self, tz, year, month, day, hour, type):
        """Create a TestItem for the given year, month, day, hour in local
        time zone.
        """
        # Can't use the normal datetime constructor for pytz. Must use
        # timezone.localize(). See https://stackoverflow.com/questions/18541051
        dt = tz.localize(datetime.datetime(year, month, day, hour))
        unix_seconds = int(dt.timestamp())
        epoch_seconds = unix_seconds - SECONDS_SINCE_UNIX_EPOCH
        return self.create_test_item_from_epoch_seconds(
            tz, epoch_seconds, type)

    def create_test_item_from_epoch_seconds(self, tz, epoch_seconds, type):
        """Return the TestItem fro the epoch_seconds.
            utc_offset: the total UTC offset
            dst_offset: the DST offset
        The base offset is (utc_offset - dst_offset).
        """
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH
        utc_dt = datetime.datetime.fromtimestamp(
            unix_seconds, tz=datetime.timezone.utc)
        dt = utc_dt.astimezone(tz)
        utc_offset = int(dt.utcoffset().total_seconds())
        dst_offset = int(dt.dst().total_seconds())

        return TestItem(
            epoch=epoch_seconds,
            utc_offset=utc_offset,
            dst_offset=dst_offset,
            y=dt.year,
            M=dt.month,
            d=dt.day,
            h=dt.hour,
            m=dt.minute,
            s=dt.second,
            type=type)



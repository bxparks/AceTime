#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License

import unittest
import zonedb.zone_infos
import zonedb.zone_policies
from zone_specifier import DateTuple
from zone_specifier import YearMonthTuple
from zone_specifier import ZoneSpecifier
from zone_specifier import get_candidate_years
from zone_specifier import expand_date_tuple
from zone_specifier import normalize_date_tuple


class TestZoneSpecifierHelperMethods(unittest.TestCase):
    def test_get_candidate_years(self):
        self.assertEqual({1, 2, 3}, get_candidate_years(1, 4, 2, 3))
        self.assertEqual({1, 2, 3}, get_candidate_years(0, 4, 2, 3))
        self.assertEqual(set(), get_candidate_years(4, 5, 2, 3))
        self.assertEqual({2}, get_candidate_years(0, 2, 5, 6))
        self.assertEqual({4, 5}, get_candidate_years(0, 5, 5, 6))
        self.assertEqual({0, 1, 2}, get_candidate_years(0, 2, 0, 2))
        self.assertEqual({1, 2, 3, 4}, get_candidate_years(0, 4, 2, 4))

    def test_expand_date_tuple(self):
        self.assertEqual((DateTuple(2000, 1, 30, 10800, 'w'),
                          DateTuple(2000, 1, 30, 7200, 's'),
                          DateTuple(2000, 1, 30, 0, 'u')),
                         expand_date_tuple(
                             DateTuple(2000, 1, 30, 10800, 'w'),
                             offset_seconds=7200,
                             delta_seconds=3600))

        self.assertEqual((DateTuple(2000, 1, 30, 10800, 'w'),
                          DateTuple(2000, 1, 30, 7200, 's'),
                          DateTuple(2000, 1, 30, 0, 'u')),
                         expand_date_tuple(
                             DateTuple(2000, 1, 30, 7200, 's'),
                             offset_seconds=7200,
                             delta_seconds=3600))

        self.assertEqual((DateTuple(2000, 1, 30, 10800, 'w'),
                          DateTuple(2000, 1, 30, 7200, 's'),
                          DateTuple(2000, 1, 30, 0, 'u')),
                         expand_date_tuple(
                             DateTuple(2000, 1, 30, 0, 'u'),
                             offset_seconds=7200,
                             delta_seconds=3600))

    def test_normalize_date_tuple(self):
        self.assertEqual(
            DateTuple(2000, 2, 1, 0, 'w'),
            normalize_date_tuple(DateTuple(2000, 2, 1, 0, 'w')))

        self.assertEqual(
            DateTuple(2000, 2, 1, 0, 's'),
            normalize_date_tuple(DateTuple(2000, 1, 31, 24 * 3600, 's')))

        self.assertEqual(
            DateTuple(2000, 2, 29, 23 * 3600, 'u'),
            normalize_date_tuple(DateTuple(2000, 3, 1, -3600, 'u')))


class TestZoneSpecifierFindMatches(unittest.TestCase):
    def test_los_angeles(self):
        """Only one Zone Era for all years (at least in zonedb).
        """
        zone_specifier = ZoneSpecifier(zonedb.zone_infos.ZONE_INFO_Los_Angeles,
            viewing_months=13)
        year = 2000
        start_ym = YearMonthTuple(year, 1)
        until_ym = YearMonthTuple(year + 1, 2)
        matches = zone_specifier.find_matches(start_ym, until_ym)

        self.assertEqual(1, len(matches))
        self.assertEqual(DateTuple(0, 1, 1, 0, 'w'), matches[0].startDateTime)
        self.assertEqual(DateTuple(10000, 1, 1, 0, 'w'),
            matches[0].untilDateTime)
        self.assertEqual('US', matches[0].zoneEra.policyName)

    def test_santo_domingo(self):
        """Two ZoneEra changes in year 2000.
        """
        zone_specifier = ZoneSpecifier(
            zonedb.zone_infos.ZONE_INFO_Santo_Domingo,
            viewing_months=13)
        zone_specifier.init_for_year(2000)
        matches = zone_specifier.matches

        self.assertEqual(3, len(matches))

        self.assertEqual(DateTuple(0, 1, 1, 0, 'w'), matches[0].startDateTime)
        self.assertEqual(DateTuple(2000, 10, 29, 2*3600, 'w'),
            matches[0].untilDateTime)
        self.assertEqual('-', matches[0].zoneEra.policyName)

        self.assertEqual(DateTuple(2000, 10, 29, 2*3600, 'w'),
            matches[1].startDateTime)
        self.assertEqual(DateTuple(2000, 12, 3, 1*3600, 'w'),
            matches[1].untilDateTime)
        self.assertEqual('US', matches[1].zoneEra.policyName)

        self.assertEqual(DateTuple(2000, 12, 3, 1*3600, 'w'),
            matches[2].startDateTime)
        self.assertEqual(DateTuple(10000, 1, 1, 0, 'w'),
            matches[2].untilDateTime)
        self.assertEqual('US', matches[1].zoneEra.policyName)


class TestZoneSpecifierInitForYear(unittest.TestCase):
    def test_los_angeles(self):
        zone_specifier = ZoneSpecifier(zonedb.zone_infos.ZONE_INFO_Los_Angeles,
            viewing_months=13)
        zone_specifier.init_for_year(2000)
        transitions = zone_specifier.transitions
        self.assertEqual(3, len(transitions))

        self.assertEqual(DateTuple(2000, 1, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2000, 4, 2, 2*3600, 'w'),
            transitions[0].untilDateTime)

        self.assertEqual(DateTuple(2000, 4, 2, 3*3600, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2000, 10, 29, 2*3600, 'w'),
            transitions[1].untilDateTime)

        self.assertEqual(DateTuple(2000, 10, 29, 1*3600, 'w'),
            transitions[2].startDateTime)
        self.assertEqual(DateTuple(2001, 2, 1, 0, 'w'),
            transitions[2].untilDateTime)

if __name__ == '__main__':
    unittest.main()

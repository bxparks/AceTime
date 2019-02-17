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
        self.assertEqual([1, 2, 3], sorted(get_candidate_years(1, 4, 2, 3)))
        self.assertEqual([1, 2, 3], sorted(get_candidate_years(0, 4, 2, 3)))
        self.assertEqual([], sorted(get_candidate_years(4, 5, 2, 3)))
        self.assertEqual([2], sorted(get_candidate_years(0, 2, 5, 6)))
        self.assertEqual([4, 5], sorted(get_candidate_years(0, 5, 5, 6)))
        self.assertEqual([0, 1, 2], sorted(get_candidate_years(0, 2, 0, 2)))
        self.assertEqual([1, 2, 3, 4], sorted(get_candidate_years(0, 4, 2, 4)))

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


class TestZoneSpecifierMatchesAndTransitions(unittest.TestCase):
    def test_Los_Angeles(self):
        """America/Los_Angela uses a simple US rule.
        """
        zone_specifier = ZoneSpecifier(zonedb.zone_infos.ZONE_INFO_Los_Angeles,
            viewing_months=14)
        zone_specifier.init_for_year(2000)

        matches = zone_specifier.matches
        self.assertEqual(1, len(matches))

        self.assertEqual(DateTuple(1999, 12, 1, 0, 'w'),
            matches[0].startDateTime)
        self.assertEqual(DateTuple(2001, 2, 1, 0, 'w'),
            matches[0].untilDateTime)
        self.assertEqual('US', matches[0].zoneEra.policyName)

        transitions = zone_specifier.transitions
        self.assertEqual(3, len(transitions))

        self.assertEqual(DateTuple(1999, 12, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2000, 4, 2, 2*3600, 'w'),
            transitions[0].untilDateTime)
        self.assertEqual(-8*3600, transitions[0].offsetSeconds)
        self.assertEqual(0, transitions[0].deltaSeconds)

        self.assertEqual(DateTuple(2000, 4, 2, 3*3600, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2000, 10, 29, 2*3600, 'w'),
            transitions[1].untilDateTime)
        self.assertEqual(-8*3600, transitions[1].offsetSeconds)
        self.assertEqual(1*3600, transitions[1].deltaSeconds)

        self.assertEqual(DateTuple(2000, 10, 29, 1*3600, 'w'),
            transitions[2].startDateTime)
        self.assertEqual(DateTuple(2001, 2, 1, 0, 'w'),
            transitions[2].untilDateTime)
        self.assertEqual(-8*3600, transitions[2].offsetSeconds)
        self.assertEqual(0*3600, transitions[2].deltaSeconds)

    def test_Petersburg(self):
        """America/Indianapolis/Petersbug moved from central to eastern time in
        1977, then switched back in 2006, then switched back again in 2007.
        """
        zone_specifier = ZoneSpecifier(
            zonedb.zone_infos.ZONE_INFO_Petersburg,
            viewing_months=14)
        zone_specifier.init_for_year(2006)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(DateTuple(2005, 12, 1, 0, 'w'),
            matches[0].startDateTime)
        self.assertEqual(DateTuple(2006, 4, 2, 2*3600, 'w'),
            matches[0].untilDateTime)
        self.assertEqual('-', matches[0].zoneEra.policyName)

        self.assertEqual(DateTuple(2006, 4, 2, 2*3600, 'w'),
            matches[1].startDateTime)
        self.assertEqual(DateTuple(2007, 2, 1, 0, 'w'),
            matches[1].untilDateTime)
        self.assertEqual('US', matches[1].zoneEra.policyName)

        transitions = zone_specifier.transitions
        self.assertEqual(3, len(transitions))

        self.assertEqual(DateTuple(2005, 12, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2006, 4, 2, 2*3600, 'w'),
            transitions[0].untilDateTime)
        self.assertEqual(-5*3600, transitions[0].offsetSeconds)
        self.assertEqual(0*3600, transitions[0].deltaSeconds)

        self.assertEqual(DateTuple(2006, 4, 2, 2*3600, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2006, 10, 29, 2*3600, 'w'),
            transitions[1].untilDateTime)
        self.assertEqual(-6*3600, transitions[1].offsetSeconds)
        self.assertEqual(1*3600, transitions[1].deltaSeconds)

        self.assertEqual(DateTuple(2006, 10, 29, 1*3600, 'w'),
            transitions[2].startDateTime)
        self.assertEqual(DateTuple(2007, 2, 1, 0, 'w'),
            transitions[2].untilDateTime)
        self.assertEqual(-6*3600, transitions[2].offsetSeconds)
        self.assertEqual(0*3600, transitions[2].deltaSeconds)

    def test_London(self):
        """Europe/London uses a EU which has a 'u' in the AT field.
        """
        zone_specifier = ZoneSpecifier(zonedb.zone_infos.ZONE_INFO_London,
            viewing_months=14)
        zone_specifier.init_for_year(2000)

        matches = zone_specifier.matches
        self.assertEqual(1, len(matches))

        self.assertEqual(DateTuple(1999, 12, 1, 0, 'w'),
            matches[0].startDateTime)
        self.assertEqual(DateTuple(2001, 2, 1, 0, 'w'),
            matches[0].untilDateTime)
        self.assertEqual('EU', matches[0].zoneEra.policyName)

        transitions = zone_specifier.transitions
        self.assertEqual(3, len(transitions))

        self.assertEqual(DateTuple(1999, 12, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2000, 3, 26, 1*3600, 'w'),
            transitions[0].untilDateTime)
        self.assertEqual(0*3600, transitions[0].offsetSeconds)
        self.assertEqual(0*3600, transitions[0].deltaSeconds)

        self.assertEqual(DateTuple(2000, 3, 26, 2*3600, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2000, 10, 29, 2*3600, 'w'),
            transitions[1].untilDateTime)
        self.assertEqual(0*3600, transitions[1].offsetSeconds)
        self.assertEqual(1*3600, transitions[1].deltaSeconds)

        self.assertEqual(DateTuple(2000, 10, 29, 1*3600, 'w'),
            transitions[2].startDateTime)
        self.assertEqual(DateTuple(2001, 2, 1, 0, 'w'),
            transitions[2].untilDateTime)
        self.assertEqual(0*3600, transitions[2].offsetSeconds)
        self.assertEqual(0*3600, transitions[2].deltaSeconds)

    def test_Winnipeg(self):
        """America/Winnipeg uses 'Rule Winn' until 2006 which has an 's' suffix
        in the Rule.AT field.
        """
        zone_specifier = ZoneSpecifier(
            zonedb.zone_infos.ZONE_INFO_Winnipeg,
            viewing_months=14)
        zone_specifier.init_for_year(2005)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(DateTuple(2004, 12, 1, 0, 'w'),
            matches[0].startDateTime)
        self.assertEqual(DateTuple(2006, 1, 1, 0*3600, 'w'),
            matches[0].untilDateTime)
        self.assertEqual('Winn', matches[0].zoneEra.policyName)

        self.assertEqual(DateTuple(2006, 1, 1, 0*3600, 'w'),
            matches[1].startDateTime)
        self.assertEqual(DateTuple(2006, 2, 1, 0*3600, 'w'),
            matches[1].untilDateTime)
        self.assertEqual('Canada', matches[1].zoneEra.policyName)

        transitions = zone_specifier.transitions
        self.assertEqual(4, len(transitions))

        self.assertEqual(DateTuple(2004, 12, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2005, 4, 3, 2*3600, 'w'),
            transitions[0].untilDateTime)
        self.assertEqual(-6*3600, transitions[0].offsetSeconds)
        self.assertEqual(0*3600, transitions[0].deltaSeconds)

        self.assertEqual(DateTuple(2005, 4, 3, 3*3600, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2005, 10, 30, 3*3600, 'w'),
            transitions[1].untilDateTime)
        self.assertEqual(-6*3600, transitions[1].offsetSeconds)
        self.assertEqual(1*3600, transitions[1].deltaSeconds)

        self.assertEqual(DateTuple(2005, 10, 30, 2*3600, 'w'),
            transitions[2].startDateTime)
        self.assertEqual(DateTuple(2006, 1, 1, 0, 'w'),
            transitions[2].untilDateTime)
        self.assertEqual(-6*3600, transitions[2].offsetSeconds)
        self.assertEqual(0*3600, transitions[2].deltaSeconds)

        self.assertEqual(DateTuple(2006, 1, 1, 0*3600, 'w'),
            transitions[3].startDateTime)
        self.assertEqual(DateTuple(2006, 2, 1, 0, 'w'),
            transitions[3].untilDateTime)
        self.assertEqual(-6*3600, transitions[3].offsetSeconds)
        self.assertEqual(0*3600, transitions[3].deltaSeconds)

    def test_Moscow(self):
        """Europe/Moscow uses 's' in the Zone UNTIL field.
        """
        zone_specifier = ZoneSpecifier(
            zonedb.zone_infos.ZONE_INFO_Moscow,
            viewing_months=14)
        zone_specifier.init_for_year(2011)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(DateTuple(2010, 12, 1, 0, 'w'),
            matches[0].startDateTime)
        self.assertEqual(DateTuple(2011, 3, 27, 2*3600, 's'),
            matches[0].untilDateTime)
        self.assertEqual('Russia', matches[0].zoneEra.policyName)

        self.assertEqual(DateTuple(2011, 3, 27, 2*3600, 's'),
            matches[1].startDateTime)
        self.assertEqual(DateTuple(2012, 2, 1, 0, 'w'),
            matches[1].untilDateTime)
        self.assertEqual('-', matches[1].zoneEra.policyName)

        transitions = zone_specifier.transitions
        self.assertEqual(2, len(transitions))

        self.assertEqual(DateTuple(2010, 12, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2011, 3, 27, 2*3600, 'w'),
            transitions[0].untilDateTime)
        self.assertEqual(3*3600, transitions[0].offsetSeconds)
        self.assertEqual(0*3600, transitions[0].deltaSeconds)

        self.assertEqual(DateTuple(2011, 3, 27, 3*3600, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2012, 2, 1, 0*3600, 'w'),
            transitions[1].untilDateTime)
        self.assertEqual(4*3600, transitions[1].offsetSeconds)
        self.assertEqual(0*3600, transitions[1].deltaSeconds)

    def test_Famagusta(self):
        """Asia/Famagusta uses 'u' in the Zone UNTIL field.
        """
        zone_specifier = ZoneSpecifier(
            zonedb.zone_infos.ZONE_INFO_Famagusta,
            viewing_months=14)
        zone_specifier.init_for_year(2017)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(DateTuple(2016, 12, 1, 0, 'w'),
            matches[0].startDateTime)
        self.assertEqual(DateTuple(2017, 10, 29, 1*3600, 'u'),
            matches[0].untilDateTime)
        self.assertEqual('-', matches[0].zoneEra.policyName)

        self.assertEqual(DateTuple(2017, 10, 29, 1*3600, 'u'),
            matches[1].startDateTime)
        self.assertEqual(DateTuple(2018, 2, 1, 0, 'w'),
            matches[1].untilDateTime)
        self.assertEqual('EUAsia', matches[1].zoneEra.policyName)

        transitions = zone_specifier.transitions
        self.assertEqual(2, len(transitions))

        self.assertEqual(DateTuple(2016, 12, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2017, 10, 29, 4*3600, 'w'),
            transitions[0].untilDateTime)
        self.assertEqual(3*3600, transitions[0].offsetSeconds)
        self.assertEqual(0*3600, transitions[0].deltaSeconds)

        self.assertEqual(DateTuple(2017, 10, 29, 3*3600, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2018, 2, 1, 0*3600, 'w'),
            transitions[1].untilDateTime)
        self.assertEqual(2*3600, transitions[1].offsetSeconds)
        self.assertEqual(0*3600, transitions[1].deltaSeconds)

    def test_Santo_Domingo(self):
        """America/Santo_Domingo uses 2 ZoneEra changes in year 2000.
        """
        zone_specifier = ZoneSpecifier(
            zonedb.zone_infos.ZONE_INFO_Santo_Domingo,
            viewing_months=14)
        zone_specifier.init_for_year(2000)

        matches = zone_specifier.matches
        self.assertEqual(3, len(matches))

        self.assertEqual(DateTuple(1999, 12, 1, 0, 'w'),
            matches[0].startDateTime)
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
        self.assertEqual(DateTuple(2001, 2, 1, 0, 'w'),
            matches[2].untilDateTime)
        self.assertEqual('-', matches[2].zoneEra.policyName)

        transitions = zone_specifier.transitions
        self.assertEqual(3, len(transitions))

        self.assertEqual(DateTuple(1999, 12, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2000, 10, 29, 2*3600, 'w'),
            transitions[0].untilDateTime)
        self.assertEqual(-4*3600, transitions[0].offsetSeconds)
        self.assertEqual(0*3600, transitions[0].deltaSeconds)

        self.assertEqual(DateTuple(2000, 10, 29, 1*3600, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2000, 12, 3, 1*3600, 'w'),
            transitions[1].untilDateTime)
        self.assertEqual(-5*3600, transitions[1].offsetSeconds)
        self.assertEqual(0*3600, transitions[1].deltaSeconds)

        self.assertEqual(DateTuple(2000, 12, 3, 2*3600, 'w'),
            transitions[2].startDateTime)
        self.assertEqual(DateTuple(2001, 2, 1, 0, 'w'),
            transitions[2].untilDateTime)
        self.assertEqual(-4*3600, transitions[2].offsetSeconds)
        self.assertEqual(0*3600, transitions[2].deltaSeconds)

    def test_Moncton(self):
        """America/Moncton transitioned DST at 00:01 through 2006.
        """
        zone_specifier = ZoneSpecifier(
            zonedb.zone_infos.ZONE_INFO_Moncton,
            viewing_months=14)
        zone_specifier.init_for_year(2006)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(DateTuple(2005, 12, 1, 0, 'w'),
            matches[0].startDateTime)
        self.assertEqual(DateTuple(2007, 1, 1, 0*3600, 'w'),
            matches[0].untilDateTime)
        self.assertEqual('Moncton', matches[0].zoneEra.policyName)

        self.assertEqual(DateTuple(2007, 1, 1, 0*3600, 'w'),
            matches[1].startDateTime)
        self.assertEqual(DateTuple(2007, 2, 1, 0, 'w'),
            matches[1].untilDateTime)
        self.assertEqual('Canada', matches[1].zoneEra.policyName)

        transitions = zone_specifier.transitions
        self.assertEqual(4, len(transitions))

        self.assertEqual(DateTuple(2005, 12, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2006, 4, 2, 0*3600+60, 'w'),
            transitions[0].untilDateTime)
        self.assertEqual(-4*3600, transitions[0].offsetSeconds)
        self.assertEqual(0*3600, transitions[0].deltaSeconds)

        self.assertEqual(DateTuple(2006, 4, 2, 1*3600+60, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2006, 10, 29, 0*3600+60, 'w'),
            transitions[1].untilDateTime)
        self.assertEqual(-4*3600, transitions[1].offsetSeconds)
        self.assertEqual(1*3600, transitions[1].deltaSeconds)

        self.assertEqual(DateTuple(2006, 10, 28, 23*3600+60, 'w'),
            transitions[2].startDateTime)
        self.assertEqual(DateTuple(2007, 1, 1, 0, 'w'),
            transitions[2].untilDateTime)
        self.assertEqual(-4*3600, transitions[2].offsetSeconds)
        self.assertEqual(0*3600, transitions[2].deltaSeconds)

        self.assertEqual(DateTuple(2007, 1, 1, 0*3600, 'w'),
            transitions[3].startDateTime)
        self.assertEqual(DateTuple(2007, 2, 1, 0, 'w'),
            transitions[3].untilDateTime)
        self.assertEqual(-4*3600, transitions[3].offsetSeconds)
        self.assertEqual(0*3600, transitions[3].deltaSeconds)

    def test_Istanbul(self):
        """Europe/Istanbul uses an 'hh:mm' offset in the RULES field in 2015.
        """
        zone_specifier = ZoneSpecifier(
            zonedb.zone_infos.ZONE_INFO_Istanbul,
            viewing_months=14)
        zone_specifier.init_for_year(2015)

        matches = zone_specifier.matches
        self.assertEqual(3, len(matches))

        self.assertEqual(DateTuple(2014, 12, 1, 0, 'w'),
            matches[0].startDateTime)
        self.assertEqual(DateTuple(2015, 10, 25, 1*3600, 'u'),
            matches[0].untilDateTime)
        self.assertEqual('EU', matches[0].zoneEra.policyName)

        self.assertEqual(DateTuple(2015, 10, 25, 1*3600, 'u'),
            matches[1].startDateTime)
        self.assertEqual(DateTuple(2015, 11, 8, 1*3600, 'u'),
            matches[1].untilDateTime)
        self.assertEqual(':', matches[1].zoneEra.policyName)

        self.assertEqual(DateTuple(2015, 11, 8, 1*3600, 'u'),
            matches[2].startDateTime)
        self.assertEqual(DateTuple(2016, 2, 1, 0, 'w'),
            matches[2].untilDateTime)
        self.assertEqual('EU', matches[2].zoneEra.policyName)

        transitions = zone_specifier.transitions
        self.assertEqual(4, len(transitions))

        self.assertEqual(DateTuple(2014, 12, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2015, 3, 29, 3*3600, 'w'),
            transitions[0].untilDateTime)
        self.assertEqual(2*3600, transitions[0].offsetSeconds)
        self.assertEqual(0*3600, transitions[0].deltaSeconds)

        self.assertEqual(DateTuple(2015, 3, 29, 4*3600, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2015, 10, 25, 4*3600, 'w'),
            transitions[1].untilDateTime)
        self.assertEqual(2*3600, transitions[1].offsetSeconds)
        self.assertEqual(1*3600, transitions[1].deltaSeconds)

        self.assertEqual(DateTuple(2015, 10, 25, 4*3600, 'w'),
            transitions[2].startDateTime)
        self.assertEqual(DateTuple(2015, 11, 8, 4*3600, 'w'),
            transitions[2].untilDateTime)
        self.assertEqual(2*3600, transitions[2].offsetSeconds)
        self.assertEqual(1*3600, transitions[2].deltaSeconds)

        self.assertEqual(DateTuple(2015, 11, 8, 3*3600, 'w'),
            transitions[3].startDateTime)
        self.assertEqual(DateTuple(2016, 2, 1, 0, 'w'),
            transitions[3].untilDateTime)
        self.assertEqual(2*3600, transitions[3].offsetSeconds)
        self.assertEqual(0*3600, transitions[3].deltaSeconds)

    def test_Dublin(self):
        """Europe/Dublin uses negative DST during Winter.
        """
        zone_specifier = ZoneSpecifier(zonedb.zone_infos.ZONE_INFO_Dublin,
            viewing_months=14)
        zone_specifier.init_for_year(2000)

        matches = zone_specifier.matches
        self.assertEqual(1, len(matches))

        self.assertEqual(DateTuple(1999, 12, 1, 0, 'w'),
            matches[0].startDateTime)
        self.assertEqual(DateTuple(2001, 2, 1, 0, 'w'),
            matches[0].untilDateTime)
        self.assertEqual('Eire', matches[0].zoneEra.policyName)

        transitions = zone_specifier.transitions
        self.assertEqual(3, len(transitions))

        self.assertEqual(DateTuple(1999, 12, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2000, 3, 26, 1*3600, 'w'),
            transitions[0].untilDateTime)
        self.assertEqual(1*3600, transitions[0].offsetSeconds)
        self.assertEqual(-1*3600, transitions[0].deltaSeconds)

        self.assertEqual(DateTuple(2000, 3, 26, 2*3600, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2000, 10, 29, 2*3600, 'w'),
            transitions[1].untilDateTime)
        self.assertEqual(1*3600, transitions[1].offsetSeconds)
        self.assertEqual(0*3600, transitions[1].deltaSeconds)

        self.assertEqual(DateTuple(2000, 10, 29, 1*3600, 'w'),
            transitions[2].startDateTime)
        self.assertEqual(DateTuple(2001, 2, 1, 0, 'w'),
            transitions[2].untilDateTime)
        self.assertEqual(1*3600, transitions[2].offsetSeconds)
        self.assertEqual(-1*3600, transitions[2].deltaSeconds)

    def test_Apia(self):
        """Pacific/Apia uses a transition time of 24:00 on Dec 29, 2011,
        going from Thursday 29th December 2011 23:59:59 Hours to Saturday 31st
        December 2011 00:00:00 Hours.
        """
        zone_specifier = ZoneSpecifier(zonedb.zone_infos.ZONE_INFO_Apia,
            viewing_months=14)
        zone_specifier.init_for_year(2011)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(DateTuple(2010, 12, 1, 0, 'w'),
            matches[0].startDateTime)
        self.assertEqual(DateTuple(2011, 12, 29, 24*3600, 'w'),
            matches[0].untilDateTime)
        self.assertEqual('WS', matches[0].zoneEra.policyName)

        self.assertEqual(DateTuple(2011, 12, 29, 24*3600, 'w'),
            matches[1].startDateTime)
        self.assertEqual(DateTuple(2012, 2, 1, 0, 'w'),
            matches[1].untilDateTime)
        self.assertEqual('WS', matches[1].zoneEra.policyName)

        transitions = zone_specifier.transitions
        self.assertEqual(4, len(transitions))

        self.assertEqual(DateTuple(2010, 12, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2011, 4, 2, 4*3600, 'w'),
            transitions[0].untilDateTime)
        self.assertEqual(-11*3600, transitions[0].offsetSeconds)
        self.assertEqual(1*3600, transitions[0].deltaSeconds)

        self.assertEqual(DateTuple(2011, 4, 2, 3*3600, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2011, 9, 24, 3*3600, 'w'),
            transitions[1].untilDateTime)
        self.assertEqual(-11*3600, transitions[1].offsetSeconds)
        self.assertEqual(0*3600, transitions[1].deltaSeconds)

        self.assertEqual(DateTuple(2011, 9, 24, 4*3600, 'w'),
            transitions[2].startDateTime)
        self.assertEqual(DateTuple(2011, 12, 30, 0, 'w'),
            transitions[2].untilDateTime)
        self.assertEqual(-11*3600, transitions[2].offsetSeconds)
        self.assertEqual(1*3600, transitions[2].deltaSeconds)

        self.assertEqual(DateTuple(2011, 12, 31, 0*3600, 'w'),
            transitions[3].startDateTime)
        self.assertEqual(DateTuple(2012, 2, 1, 0, 'w'),
            transitions[3].untilDateTime)
        self.assertEqual(13*3600, transitions[3].offsetSeconds)
        self.assertEqual(1*3600, transitions[3].deltaSeconds)

    def test_Macquarie(self):
        """Antarctica/Macquarie changes ZoneEra in 2011 using a 'w' time, but
        the ZoneRule transitions use an 's' time, which happens to coincide with
        the change in ZoneEra. The code must treat those 2 transition times as
        the same point in time.
        """
        zone_specifier = ZoneSpecifier(
            zonedb.zone_infos.ZONE_INFO_Macquarie,
            viewing_months=14)
        zone_specifier.init_for_year(2010)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(DateTuple(2009, 12, 1, 0, 'w'),
            matches[0].startDateTime)
        self.assertEqual(DateTuple(2010, 4, 4, 3*3600, 'w'),
            matches[0].untilDateTime)
        self.assertEqual('AT', matches[0].zoneEra.policyName)

        self.assertEqual(DateTuple(2010, 4, 4, 3*3600, 'w'),
            matches[1].startDateTime)
        self.assertEqual(DateTuple(2011, 2, 1, 0, 'w'),
            matches[1].untilDateTime)
        self.assertEqual('-', matches[1].zoneEra.policyName)

        transitions = zone_specifier.transitions
        self.assertEqual(2, len(transitions))

        self.assertEqual(DateTuple(2009, 12, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2010, 4, 4, 3*3600, 'w'),
            transitions[0].untilDateTime)
        self.assertEqual(10*3600, transitions[0].offsetSeconds)
        self.assertEqual(1*3600, transitions[0].deltaSeconds)

        self.assertEqual(DateTuple(2010, 4, 4, 3*3600, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2011, 2, 1, 0*3600, 'w'),
            transitions[1].untilDateTime)
        self.assertEqual(11*3600, transitions[1].offsetSeconds)
        self.assertEqual(0*3600, transitions[1].deltaSeconds)

    def test_Simferopol(self):
        """Asia/Simferopol in 2014 uses a bizarre mixture of 'w' when using EU
        rules (which itself uses 'u' in the UNTIL fields), then uses 's' time to
        switch to Moscow time.
        """
        zone_specifier = ZoneSpecifier(
            zonedb.zone_infos.ZONE_INFO_Simferopol,
            viewing_months=14)
        zone_specifier.init_for_year(2014)

        matches = zone_specifier.matches
        self.assertEqual(3, len(matches))

        self.assertEqual(DateTuple(2013, 12, 1, 0*3600, 'w'),
            matches[0].startDateTime)
        self.assertEqual(DateTuple(2014, 3, 30, 2*3600, 'w'),
            matches[0].untilDateTime)
        self.assertEqual('EU', matches[0].zoneEra.policyName)

        self.assertEqual(DateTuple(2014, 3, 30, 2*3600, 'w'),
            matches[1].startDateTime)
        self.assertEqual(DateTuple(2014, 10, 26, 2*3600, 's'),
            matches[1].untilDateTime)
        self.assertEqual('-', matches[1].zoneEra.policyName)

        self.assertEqual(DateTuple(2014, 10, 26, 2*3600, 's'),
            matches[2].startDateTime)
        self.assertEqual(DateTuple(2015, 2, 1, 0*3600, 'w'),
            matches[2].untilDateTime)
        self.assertEqual('-', matches[2].zoneEra.policyName)

        transitions = zone_specifier.transitions
        self.assertEqual(3, len(transitions))

        self.assertEqual(DateTuple(2013, 12, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2014, 3, 30, 2*3600, 'w'),
            transitions[0].untilDateTime)
        self.assertEqual(2*3600, transitions[0].offsetSeconds)
        self.assertEqual(0*3600, transitions[0].deltaSeconds)

        self.assertEqual(DateTuple(2014, 3, 30, 4*3600, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2014, 10, 26, 2*3600, 'w'),
            transitions[1].untilDateTime)
        self.assertEqual(4*3600, transitions[1].offsetSeconds)
        self.assertEqual(0*3600, transitions[1].deltaSeconds)

        self.assertEqual(DateTuple(2014, 10, 26, 1*3600, 'w'),
            transitions[2].startDateTime)
        self.assertEqual(DateTuple(2015, 2, 1, 0*3600, 'w'),
            transitions[2].untilDateTime)
        self.assertEqual(3*3600, transitions[2].offsetSeconds)
        self.assertEqual(0*3600, transitions[2].deltaSeconds)

    def test_Kamchatka(self):
        """Asia/Kamchatka uses 's' in the Zone UNTIL and Rule AT fields.
        """
        zone_specifier = ZoneSpecifier(
            zonedb.zone_infos.ZONE_INFO_Kamchatka,
            viewing_months=14)
        zone_specifier.init_for_year(2011)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(DateTuple(2010, 12, 1, 0*3600, 'w'),
            matches[0].startDateTime)
        self.assertEqual(DateTuple(2011, 3, 27, 2*3600, 's'),
            matches[0].untilDateTime)
        self.assertEqual('Russia', matches[0].zoneEra.policyName)

        self.assertEqual(DateTuple(2011, 3, 27, 2*3600, 's'),
            matches[1].startDateTime)
        self.assertEqual(DateTuple(2012, 2, 1, 0*3600, 'w'),
            matches[1].untilDateTime)
        self.assertEqual('-', matches[1].zoneEra.policyName)

        transitions = zone_specifier.transitions
        self.assertEqual(2, len(transitions))

        self.assertEqual(DateTuple(2010, 12, 1, 0, 'w'),
            transitions[0].startDateTime)
        self.assertEqual(DateTuple(2011, 3, 27, 2*3600, 'w'),
            transitions[0].untilDateTime)
        self.assertEqual(11*3600, transitions[0].offsetSeconds)
        self.assertEqual(0*3600, transitions[0].deltaSeconds)

        self.assertEqual(DateTuple(2011, 3, 27, 3*3600, 'w'),
            transitions[1].startDateTime)
        self.assertEqual(DateTuple(2012, 2, 1, 0*3600, 'w'),
            transitions[1].untilDateTime)
        self.assertEqual(12*3600, transitions[1].offsetSeconds)
        self.assertEqual(0*3600, transitions[1].deltaSeconds)

if __name__ == '__main__':
    unittest.main()

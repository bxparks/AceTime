# Copyright 2018 Brian T. Park
#
# MIT License

from typing import cast
import unittest
from datetime import datetime
from zonedbpy import zone_infos
# from zonedbpy import validation_data # reenable using zoneinfo.json?
# from validation.tdgenerator import TestItem
from zone_processor.zone_specifier import DateTuple
from zone_processor.zone_specifier import Transition
from zone_processor.zone_specifier import ZoneMatch
from zone_processor.zone_specifier import ZoneSpecifier
from zone_processor.zone_specifier import CandidateFinderBasic
from zone_processor.zone_specifier import _compare_transition_to_match
from zone_processor.zone_specifier import _compare_transition_to_match_fuzzy
from zone_processor.zone_specifier import _subtract_date_tuple
from zone_processor.inline_zone_info import ZoneInfo


# class TestValidationData(unittest.TestCase):
#     @unittest.skip("zonedb/validation_data.py not generated")
#     def test_validation_data(self) -> None:
#         test_data = validation_data.VALIDATION_DATA[
#             'America/Los_Angeles']
#         self.assertTrue(isinstance(test_data[0], TestItem))
#
#     @unittest.skip("zonedb/validation_data.py not generated")
#     def test_zone_specifier_using_validation_data(self) -> None:
#         for name, items in validation_data.VALIDATION_DATA.items():
#             zone_info = zone_infos.ZONE_INFO_MAP[name]
#             zone_specifier = ZoneSpecifier(zone_info, viewing_months=14)
#             for item in items:
#                 info = zone_specifier.get_timezone_info_for_seconds(
#                       item.epoch)
#                 self.assertEqual(
#                     item.total_offset * 60, info.total_offset,
#                     ('Zone %s; epoch:%s; %04d-%02d-%02d %02d:%02d:%02d' %
#                      (name, item.epoch, item.y, item.M, item.d, item.h,
#                       item.m, item.s)))


class TestZoneSpecifierHelperMethods(unittest.TestCase):
    def test_get_candidate_years(self) -> None:
        self.assertEqual([1, 2, 3],
                         sorted(
                             CandidateFinderBasic.get_candidate_years(
                                 1, 4, 2, 3)))
        self.assertEqual([1, 2, 3],
                         sorted(
                             CandidateFinderBasic.get_candidate_years(
                                 0, 4, 2, 3)))
        self.assertEqual([],
                         sorted(
                             CandidateFinderBasic.get_candidate_years(
                                 4, 5, 2, 3)))
        self.assertEqual([2],
                         sorted(
                             CandidateFinderBasic.get_candidate_years(
                                 0, 2, 5, 6)))
        self.assertEqual([4, 5],
                         sorted(
                             CandidateFinderBasic.get_candidate_years(
                                 0, 5, 5, 6)))
        self.assertEqual([0, 1, 2],
                         sorted(
                             CandidateFinderBasic.get_candidate_years(
                                 0, 2, 0, 2)))
        self.assertEqual([1, 2, 3, 4],
                         sorted(
                             CandidateFinderBasic.get_candidate_years(
                                 0, 4, 2, 4)))

    def test_expand_date_tuple(self) -> None:
        self.assertEqual((DateTuple(2000, 1, 30, 10800, 'w'),
                          DateTuple(2000, 1, 30, 7200, 's'),
                          DateTuple(2000, 1, 30, 0, 'u')),
                         ZoneSpecifier._expand_date_tuple(
                             DateTuple(2000, 1, 30, 10800, 'w'),
                             offset_seconds=7200,
                             delta_seconds=3600))

        self.assertEqual((DateTuple(2000, 1, 30, 10800, 'w'),
                          DateTuple(2000, 1, 30, 7200, 's'),
                          DateTuple(2000, 1, 30, 0, 'u')),
                         ZoneSpecifier._expand_date_tuple(
                             DateTuple(2000, 1, 30, 7200, 's'),
                             offset_seconds=7200,
                             delta_seconds=3600))

        self.assertEqual((DateTuple(2000, 1, 30, 10800, 'w'),
                          DateTuple(2000, 1, 30, 7200, 's'),
                          DateTuple(2000, 1, 30, 0, 'u')),
                         ZoneSpecifier._expand_date_tuple(
                             DateTuple(2000, 1, 30, 0, 'u'),
                             offset_seconds=7200,
                             delta_seconds=3600))

    def test_normalize_date_tuple(self) -> None:
        self.assertEqual(
            DateTuple(2000, 2, 1, 0, 'w'),
            ZoneSpecifier._normalize_date_tuple(DateTuple(2000, 2, 1, 0, 'w')))

        self.assertEqual(
            DateTuple(2000, 2, 1, 0, 's'),
            ZoneSpecifier._normalize_date_tuple(
                DateTuple(2000, 1, 31, 24 * 3600, 's')))

        self.assertEqual(
            DateTuple(2000, 2, 29, 23 * 3600, 'u'),
            ZoneSpecifier._normalize_date_tuple(
                DateTuple(2000, 3, 1, -3600, 'u')))

    def test_subtract_date_tuple(self) -> None:
        self.assertEqual(
            -1,
            _subtract_date_tuple(
                DateTuple(2000, 1, 1, 43, 'w'),
                DateTuple(2000, 1, 1, 44, 'w'),
            )
        )

        self.assertEqual(
            24 * 3600 - 1,
            _subtract_date_tuple(
                DateTuple(2000, 1, 2, 43, 'w'),
                DateTuple(2000, 1, 1, 44, 'w'),
            )
        )

        self.assertEqual(
            -31 * 24 * 3600 + 24 * 3600 - 1,
            _subtract_date_tuple(
                DateTuple(2000, 1, 2, 43, 'w'),
                DateTuple(2000, 2, 1, 44, 'w'),
            )
        )


class TestCompareTransitionToMatch(unittest.TestCase):
    def test_compare_exact(self) -> None:
        match = ZoneMatch({
            'start_date_time': DateTuple(2000, 1, 1, 0, 'w'),
            'until_date_time': DateTuple(2001, 1, 1, 0, 'w')
        })

        transition = Transition({
            'transition_time':
            DateTuple(1999, 12, 31, 0, 'w')
        })
        self.assertEqual(-1, _compare_transition_to_match(transition, match))

        transition = Transition({
            'transition_time': DateTuple(2000, 1, 1, 0, 'w')
        })
        self.assertEqual(0, _compare_transition_to_match(transition, match))

        transition = Transition({
            'transition_time': DateTuple(2000, 1, 2, 0, 'w')
        })
        self.assertEqual(1, _compare_transition_to_match(transition, match))

        transition = Transition({
            'transition_time': DateTuple(2001, 1, 2, 0, 'w')
        })
        self.assertEqual(2, _compare_transition_to_match(transition, match))

    def test_compare_fuzzy(self) -> None:
        match = ZoneMatch({
            'start_date_time': DateTuple(2000, 1, 1, 0, 'w'),
            'until_date_time': DateTuple(2001, 1, 1, 0, 'w')
        })

        transition = Transition({
            'transition_time':
            DateTuple(1999, 11, 1, 0, 'w')
        })
        self.assertEqual(-1,
                         _compare_transition_to_match_fuzzy(transition, match))

        transition = Transition({
            'transition_time':
            DateTuple(1999, 12, 1, 0, 'w')
        })
        self.assertEqual(1,
                         _compare_transition_to_match_fuzzy(transition, match))

        transition = Transition({
            'transition_time': DateTuple(2000, 1, 1, 0, 'w')
        })
        self.assertEqual(1,
                         _compare_transition_to_match_fuzzy(transition, match))

        transition = Transition({
            'transition_time': DateTuple(2001, 1, 1, 0, 'w')
        })
        self.assertEqual(1,
                         _compare_transition_to_match_fuzzy(transition, match))

        transition = Transition({
            'transition_time': DateTuple(2001, 2, 1, 0, 'w')
        })
        self.assertEqual(1,
                         _compare_transition_to_match_fuzzy(transition, match))

        transition = Transition({
            'transition_time': DateTuple(2001, 3, 1, 0, 'w')
        })
        self.assertEqual(2,
                         _compare_transition_to_match_fuzzy(transition, match))


class TestZoneSpecifierMatchesAndTransitions(unittest.TestCase):
    def test_Los_Angeles(self) -> None:
        """America/Los_Angela uses a simple US rule.
        """
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_America_Los_Angeles),
            viewing_months=14,
        )
        zone_specifier.init_for_year(2000)

        matches = zone_specifier.matches
        self.assertEqual(1, len(matches))

        self.assertEqual(
            DateTuple(1999, 12, 1, 0, 'w'), matches[0].start_date_time)
        self.assertEqual(
            DateTuple(2001, 2, 1, 0, 'w'), matches[0].until_date_time)
        self.assertEqual('US', matches[0].zone_era.policy_name)

        transitions = zone_specifier.transitions
        self.assertEqual(3, len(transitions))

        self.assertEqual(
            DateTuple(1999, 12, 1, 0, 'w'), transitions[0].start_date_time)
        self.assertEqual(
            DateTuple(2000, 4, 2, 2 * 3600, 'w'),
            transitions[0].until_date_time)
        self.assertEqual(-8 * 3600, transitions[0].offset_seconds)
        self.assertEqual(0, transitions[0].delta_seconds)

        self.assertEqual(
            DateTuple(2000, 4, 2, 3 * 3600, 'w'),
            transitions[1].start_date_time)
        self.assertEqual(
            DateTuple(2000, 10, 29, 2 * 3600, 'w'),
            transitions[1].until_date_time)
        self.assertEqual(-8 * 3600, transitions[1].offset_seconds)
        self.assertEqual(1 * 3600, transitions[1].delta_seconds)

        self.assertEqual(
            DateTuple(2000, 10, 29, 1 * 3600, 'w'),
            transitions[2].start_date_time)
        self.assertEqual(
            DateTuple(2001, 2, 1, 0, 'w'), transitions[2].until_date_time)
        self.assertEqual(-8 * 3600, transitions[2].offset_seconds)
        self.assertEqual(0 * 3600, transitions[2].delta_seconds)

    def test_Petersburg(self) -> None:
        """America/Indianapolis/Petersbug moved from central to eastern time in
        1977, then switched back in 2006, then switched back again in 2007.
        """
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_America_Indiana_Petersburg),
            viewing_months=14,
        )
        zone_specifier.init_for_year(2006)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(
            DateTuple(2005, 12, 1, 0, 'w'), matches[0].start_date_time)
        self.assertEqual(
            DateTuple(2006, 4, 2, 2 * 3600, 'w'), matches[0].until_date_time)
        self.assertEqual('-', matches[0].zone_era.policy_name)

        self.assertEqual(
            DateTuple(2006, 4, 2, 2 * 3600, 'w'), matches[1].start_date_time)
        self.assertEqual(
            DateTuple(2007, 2, 1, 0, 'w'), matches[1].until_date_time)
        self.assertEqual('US', matches[1].zone_era.policy_name)

        transitions = zone_specifier.transitions
        self.assertEqual(3, len(transitions))

        self.assertEqual(
            DateTuple(2005, 12, 1, 0, 'w'), transitions[0].start_date_time)
        self.assertEqual(
            DateTuple(2006, 4, 2, 2 * 3600, 'w'),
            transitions[0].until_date_time)
        self.assertEqual(-5 * 3600, transitions[0].offset_seconds)
        self.assertEqual(0 * 3600, transitions[0].delta_seconds)

        self.assertEqual(
            DateTuple(2006, 4, 2, 2 * 3600, 'w'),
            transitions[1].start_date_time)
        self.assertEqual(
            DateTuple(2006, 10, 29, 2 * 3600, 'w'),
            transitions[1].until_date_time)
        self.assertEqual(-6 * 3600, transitions[1].offset_seconds)
        self.assertEqual(1 * 3600, transitions[1].delta_seconds)

        self.assertEqual(
            DateTuple(2006, 10, 29, 1 * 3600, 'w'),
            transitions[2].start_date_time)
        self.assertEqual(
            DateTuple(2007, 2, 1, 0, 'w'), transitions[2].until_date_time)
        self.assertEqual(-6 * 3600, transitions[2].offset_seconds)
        self.assertEqual(0 * 3600, transitions[2].delta_seconds)

    def test_London(self) -> None:
        """Europe/London uses a EU which has a 'u' in the AT field.
        """
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_Europe_London),
            viewing_months=14,
        )
        zone_specifier.init_for_year(2000)

        matches = zone_specifier.matches
        self.assertEqual(1, len(matches))

        self.assertEqual(
            DateTuple(1999, 12, 1, 0, 'w'), matches[0].start_date_time)
        self.assertEqual(
            DateTuple(2001, 2, 1, 0, 'w'), matches[0].until_date_time)
        self.assertEqual('EU', matches[0].zone_era.policy_name)

        transitions = zone_specifier.transitions
        self.assertEqual(3, len(transitions))

        self.assertEqual(
            DateTuple(1999, 12, 1, 0, 'w'), transitions[0].start_date_time)
        self.assertEqual(
            DateTuple(2000, 3, 26, 1 * 3600, 'w'),
            transitions[0].until_date_time)
        self.assertEqual(0 * 3600, transitions[0].offset_seconds)
        self.assertEqual(0 * 3600, transitions[0].delta_seconds)

        self.assertEqual(
            DateTuple(2000, 3, 26, 2 * 3600, 'w'),
            transitions[1].start_date_time)
        self.assertEqual(
            DateTuple(2000, 10, 29, 2 * 3600, 'w'),
            transitions[1].until_date_time)
        self.assertEqual(0 * 3600, transitions[1].offset_seconds)
        self.assertEqual(1 * 3600, transitions[1].delta_seconds)

        self.assertEqual(
            DateTuple(2000, 10, 29, 1 * 3600, 'w'),
            transitions[2].start_date_time)
        self.assertEqual(
            DateTuple(2001, 2, 1, 0, 'w'), transitions[2].until_date_time)
        self.assertEqual(0 * 3600, transitions[2].offset_seconds)
        self.assertEqual(0 * 3600, transitions[2].delta_seconds)

    def test_Winnipeg(self) -> None:
        """America/Winnipeg uses 'Rule Winn' until 2006 which has an 's' suffix
        in the Rule.AT field.
        """
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_America_Winnipeg),
            viewing_months=14,
        )
        zone_specifier.init_for_year(2005)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(
            DateTuple(2004, 12, 1, 0, 'w'), matches[0].start_date_time)
        self.assertEqual(
            DateTuple(2006, 1, 1, 0 * 3600, 'w'), matches[0].until_date_time)
        self.assertEqual('Winn', matches[0].zone_era.policy_name)

        self.assertEqual(
            DateTuple(2006, 1, 1, 0 * 3600, 'w'), matches[1].start_date_time)
        self.assertEqual(
            DateTuple(2006, 2, 1, 0 * 3600, 'w'), matches[1].until_date_time)
        self.assertEqual('Canada', matches[1].zone_era.policy_name)

        transitions = zone_specifier.transitions
        self.assertEqual(4, len(transitions))

        self.assertEqual(
            DateTuple(2004, 12, 1, 0, 'w'), transitions[0].start_date_time)
        self.assertEqual(
            DateTuple(2005, 4, 3, 2 * 3600, 'w'),
            transitions[0].until_date_time)
        self.assertEqual(-6 * 3600, transitions[0].offset_seconds)
        self.assertEqual(0 * 3600, transitions[0].delta_seconds)

        self.assertEqual(
            DateTuple(2005, 4, 3, 3 * 3600, 'w'),
            transitions[1].start_date_time)
        self.assertEqual(
            DateTuple(2005, 10, 30, 3 * 3600, 'w'),
            transitions[1].until_date_time)
        self.assertEqual(-6 * 3600, transitions[1].offset_seconds)
        self.assertEqual(1 * 3600, transitions[1].delta_seconds)

        self.assertEqual(
            DateTuple(2005, 10, 30, 2 * 3600, 'w'),
            transitions[2].start_date_time)
        self.assertEqual(
            DateTuple(2006, 1, 1, 0, 'w'), transitions[2].until_date_time)
        self.assertEqual(-6 * 3600, transitions[2].offset_seconds)
        self.assertEqual(0 * 3600, transitions[2].delta_seconds)

        self.assertEqual(
            DateTuple(2006, 1, 1, 0 * 3600, 'w'),
            transitions[3].start_date_time)
        self.assertEqual(
            DateTuple(2006, 2, 1, 0, 'w'), transitions[3].until_date_time)
        self.assertEqual(-6 * 3600, transitions[3].offset_seconds)
        self.assertEqual(0 * 3600, transitions[3].delta_seconds)

    def test_Moscow(self) -> None:
        """Europe/Moscow uses 's' in the Zone UNTIL field.
        """
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_Europe_Moscow),
            viewing_months=14,
        )
        zone_specifier.init_for_year(2011)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(
            DateTuple(2010, 12, 1, 0, 'w'), matches[0].start_date_time)
        self.assertEqual(
            DateTuple(2011, 3, 27, 2 * 3600, 's'), matches[0].until_date_time)
        self.assertEqual('Russia', matches[0].zone_era.policy_name)

        self.assertEqual(
            DateTuple(2011, 3, 27, 2 * 3600, 's'), matches[1].start_date_time)
        self.assertEqual(
            DateTuple(2012, 2, 1, 0, 'w'), matches[1].until_date_time)
        self.assertEqual('-', matches[1].zone_era.policy_name)

        transitions = zone_specifier.transitions
        self.assertEqual(2, len(transitions))

        self.assertEqual(
            DateTuple(2010, 12, 1, 0, 'w'), transitions[0].start_date_time)
        self.assertEqual(
            DateTuple(2011, 3, 27, 2 * 3600, 'w'),
            transitions[0].until_date_time)
        self.assertEqual(3 * 3600, transitions[0].offset_seconds)
        self.assertEqual(0 * 3600, transitions[0].delta_seconds)

        self.assertEqual(
            DateTuple(2011, 3, 27, 3 * 3600, 'w'),
            transitions[1].start_date_time)
        self.assertEqual(
            DateTuple(2012, 2, 1, 0 * 3600, 'w'),
            transitions[1].until_date_time)
        self.assertEqual(4 * 3600, transitions[1].offset_seconds)
        self.assertEqual(0 * 3600, transitions[1].delta_seconds)

    def test_Famagusta(self) -> None:
        """Asia/Famagusta uses 'u' in the Zone UNTIL field.
        """
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_Asia_Famagusta),
            viewing_months=14,
        )
        zone_specifier.init_for_year(2017)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(
            DateTuple(2016, 12, 1, 0, 'w'), matches[0].start_date_time)
        self.assertEqual(
            DateTuple(2017, 10, 29, 1 * 3600, 'u'), matches[0].until_date_time)
        self.assertEqual('-', matches[0].zone_era.policy_name)

        self.assertEqual(
            DateTuple(2017, 10, 29, 1 * 3600, 'u'), matches[1].start_date_time)
        self.assertEqual(
            DateTuple(2018, 2, 1, 0, 'w'), matches[1].until_date_time)
        self.assertEqual('EUAsia', matches[1].zone_era.policy_name)

        transitions = zone_specifier.transitions
        self.assertEqual(2, len(transitions))

        self.assertEqual(
            DateTuple(2016, 12, 1, 0, 'w'), transitions[0].start_date_time)
        self.assertEqual(
            DateTuple(2017, 10, 29, 4 * 3600, 'w'),
            transitions[0].until_date_time)
        self.assertEqual(3 * 3600, transitions[0].offset_seconds)
        self.assertEqual(0 * 3600, transitions[0].delta_seconds)

        self.assertEqual(
            DateTuple(2017, 10, 29, 3 * 3600, 'w'),
            transitions[1].start_date_time)
        self.assertEqual(
            DateTuple(2018, 2, 1, 0 * 3600, 'w'),
            transitions[1].until_date_time)
        self.assertEqual(2 * 3600, transitions[1].offset_seconds)
        self.assertEqual(0 * 3600, transitions[1].delta_seconds)

    def test_Santo_Domingo(self) -> None:
        """America/Santo_Domingo uses 2 ZoneEra changes in year 2000.
        """
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_America_Santo_Domingo),
            viewing_months=14,
        )
        zone_specifier.init_for_year(2000)

        matches = zone_specifier.matches
        self.assertEqual(3, len(matches))

        self.assertEqual(
            DateTuple(1999, 12, 1, 0, 'w'), matches[0].start_date_time)
        self.assertEqual(
            DateTuple(2000, 10, 29, 2 * 3600, 'w'), matches[0].until_date_time)
        self.assertEqual('-', matches[0].zone_era.policy_name)

        self.assertEqual(
            DateTuple(2000, 10, 29, 2 * 3600, 'w'), matches[1].start_date_time)
        self.assertEqual(
            DateTuple(2000, 12, 3, 1 * 3600, 'w'), matches[1].until_date_time)
        self.assertEqual('US', matches[1].zone_era.policy_name)

        self.assertEqual(
            DateTuple(2000, 12, 3, 1 * 3600, 'w'), matches[2].start_date_time)
        self.assertEqual(
            DateTuple(2001, 2, 1, 0, 'w'), matches[2].until_date_time)
        self.assertEqual('-', matches[2].zone_era.policy_name)

        transitions = zone_specifier.transitions
        self.assertEqual(3, len(transitions))

        self.assertEqual(
            DateTuple(1999, 12, 1, 0, 'w'), transitions[0].start_date_time)
        self.assertEqual(
            DateTuple(2000, 10, 29, 2 * 3600, 'w'),
            transitions[0].until_date_time)
        self.assertEqual(-4 * 3600, transitions[0].offset_seconds)
        self.assertEqual(0 * 3600, transitions[0].delta_seconds)

        self.assertEqual(
            DateTuple(2000, 10, 29, 1 * 3600, 'w'),
            transitions[1].start_date_time)
        self.assertEqual(
            DateTuple(2000, 12, 3, 1 * 3600, 'w'),
            transitions[1].until_date_time)
        self.assertEqual(-5 * 3600, transitions[1].offset_seconds)
        self.assertEqual(0 * 3600, transitions[1].delta_seconds)

        self.assertEqual(
            DateTuple(2000, 12, 3, 2 * 3600, 'w'),
            transitions[2].start_date_time)
        self.assertEqual(
            DateTuple(2001, 2, 1, 0, 'w'), transitions[2].until_date_time)
        self.assertEqual(-4 * 3600, transitions[2].offset_seconds)
        self.assertEqual(0 * 3600, transitions[2].delta_seconds)

    def test_Moncton(self) -> None:
        """America/Moncton transitioned DST at 00:01 through 2006.
        """
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_America_Moncton),
            viewing_months=14,
        )
        zone_specifier.init_for_year(2006)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(
            DateTuple(2005, 12, 1, 0, 'w'), matches[0].start_date_time)
        self.assertEqual(
            DateTuple(2007, 1, 1, 0 * 3600, 'w'), matches[0].until_date_time)
        self.assertEqual('Moncton', matches[0].zone_era.policy_name)

        self.assertEqual(
            DateTuple(2007, 1, 1, 0 * 3600, 'w'), matches[1].start_date_time)
        self.assertEqual(
            DateTuple(2007, 2, 1, 0, 'w'), matches[1].until_date_time)
        self.assertEqual('Canada', matches[1].zone_era.policy_name)

        transitions = zone_specifier.transitions
        self.assertEqual(4, len(transitions))

        self.assertEqual(
            DateTuple(2005, 12, 1, 0, 'w'), transitions[0].start_date_time)
        self.assertEqual(
            DateTuple(2006, 4, 2, 0 * 3600 + 60, 'w'),
            transitions[0].until_date_time)
        self.assertEqual(-4 * 3600, transitions[0].offset_seconds)
        self.assertEqual(0 * 3600, transitions[0].delta_seconds)

        self.assertEqual(
            DateTuple(2006, 4, 2, 1 * 3600 + 60, 'w'),
            transitions[1].start_date_time)
        self.assertEqual(
            DateTuple(2006, 10, 29, 0 * 3600 + 60, 'w'),
            transitions[1].until_date_time)
        self.assertEqual(-4 * 3600, transitions[1].offset_seconds)
        self.assertEqual(1 * 3600, transitions[1].delta_seconds)

        self.assertEqual(
            DateTuple(2006, 10, 28, 23 * 3600 + 60, 'w'),
            transitions[2].start_date_time)
        self.assertEqual(
            DateTuple(2007, 1, 1, 0, 'w'), transitions[2].until_date_time)
        self.assertEqual(-4 * 3600, transitions[2].offset_seconds)
        self.assertEqual(0 * 3600, transitions[2].delta_seconds)

        self.assertEqual(
            DateTuple(2007, 1, 1, 0 * 3600, 'w'),
            transitions[3].start_date_time)
        self.assertEqual(
            DateTuple(2007, 2, 1, 0, 'w'), transitions[3].until_date_time)
        self.assertEqual(-4 * 3600, transitions[3].offset_seconds)
        self.assertEqual(0 * 3600, transitions[3].delta_seconds)

    def test_Istanbul(self) -> None:
        """Europe/Istanbul uses an 'hh:mm' offset in the RULES field in 2015.
        """
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_Europe_Istanbul),
            viewing_months=14,
        )
        zone_specifier.init_for_year(2015)

        matches = zone_specifier.matches
        self.assertEqual(3, len(matches))

        self.assertEqual(
            DateTuple(2014, 12, 1, 0, 'w'), matches[0].start_date_time)
        self.assertEqual(
            DateTuple(2015, 10, 25, 1 * 3600, 'u'), matches[0].until_date_time)
        self.assertEqual('EU', matches[0].zone_era.policy_name)

        self.assertEqual(
            DateTuple(2015, 10, 25, 1 * 3600, 'u'), matches[1].start_date_time)
        self.assertEqual(
            DateTuple(2015, 11, 8, 1 * 3600, 'u'), matches[1].until_date_time)
        self.assertEqual(':', matches[1].zone_era.policy_name)

        self.assertEqual(
            DateTuple(2015, 11, 8, 1 * 3600, 'u'), matches[2].start_date_time)
        self.assertEqual(
            DateTuple(2016, 2, 1, 0, 'w'), matches[2].until_date_time)
        self.assertEqual('EU', matches[2].zone_era.policy_name)

        transitions = zone_specifier.transitions
        self.assertEqual(4, len(transitions))

        self.assertEqual(
            DateTuple(2014, 12, 1, 0, 'w'), transitions[0].start_date_time)
        self.assertEqual(
            DateTuple(2015, 3, 29, 3 * 3600, 'w'),
            transitions[0].until_date_time)
        self.assertEqual(2 * 3600, transitions[0].offset_seconds)
        self.assertEqual(0 * 3600, transitions[0].delta_seconds)

        self.assertEqual(
            DateTuple(2015, 3, 29, 4 * 3600, 'w'),
            transitions[1].start_date_time)
        self.assertEqual(
            DateTuple(2015, 10, 25, 4 * 3600, 'w'),
            transitions[1].until_date_time)
        self.assertEqual(2 * 3600, transitions[1].offset_seconds)
        self.assertEqual(1 * 3600, transitions[1].delta_seconds)

        self.assertEqual(
            DateTuple(2015, 10, 25, 4 * 3600, 'w'),
            transitions[2].start_date_time)
        self.assertEqual(
            DateTuple(2015, 11, 8, 4 * 3600, 'w'),
            transitions[2].until_date_time)
        self.assertEqual(2 * 3600, transitions[2].offset_seconds)
        self.assertEqual(1 * 3600, transitions[2].delta_seconds)

        self.assertEqual(
            DateTuple(2015, 11, 8, 3 * 3600, 'w'),
            transitions[3].start_date_time)
        self.assertEqual(
            DateTuple(2016, 2, 1, 0, 'w'), transitions[3].until_date_time)
        self.assertEqual(2 * 3600, transitions[3].offset_seconds)
        self.assertEqual(0 * 3600, transitions[3].delta_seconds)

    def test_Dublin(self) -> None:
        """Europe/Dublin uses negative DST during Winter.
        """
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_Europe_Dublin),
            viewing_months=14,
        )
        zone_specifier.init_for_year(2000)

        matches = zone_specifier.matches
        self.assertEqual(1, len(matches))

        self.assertEqual(
            DateTuple(1999, 12, 1, 0, 'w'), matches[0].start_date_time)
        self.assertEqual(
            DateTuple(2001, 2, 1, 0, 'w'), matches[0].until_date_time)
        self.assertEqual('Eire', matches[0].zone_era.policy_name)

        transitions = zone_specifier.transitions
        self.assertEqual(3, len(transitions))

        self.assertEqual(
            DateTuple(1999, 12, 1, 0, 'w'), transitions[0].start_date_time)
        self.assertEqual(
            DateTuple(2000, 3, 26, 1 * 3600, 'w'),
            transitions[0].until_date_time)
        self.assertEqual(1 * 3600, transitions[0].offset_seconds)
        self.assertEqual(-1 * 3600, transitions[0].delta_seconds)

        self.assertEqual(
            DateTuple(2000, 3, 26, 2 * 3600, 'w'),
            transitions[1].start_date_time)
        self.assertEqual(
            DateTuple(2000, 10, 29, 2 * 3600, 'w'),
            transitions[1].until_date_time)
        self.assertEqual(1 * 3600, transitions[1].offset_seconds)
        self.assertEqual(0 * 3600, transitions[1].delta_seconds)

        self.assertEqual(
            DateTuple(2000, 10, 29, 1 * 3600, 'w'),
            transitions[2].start_date_time)
        self.assertEqual(
            DateTuple(2001, 2, 1, 0, 'w'), transitions[2].until_date_time)
        self.assertEqual(1 * 3600, transitions[2].offset_seconds)
        self.assertEqual(-1 * 3600, transitions[2].delta_seconds)

    def test_Apia(self) -> None:
        """Pacific/Apia uses a transition time of 24:00 on Dec 29, 2011,
        going from Thursday 29th December 2011 23:59:59 Hours to Saturday 31st
        December 2011 00:00:00 Hours.
        """
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_Pacific_Apia),
            viewing_months=14,
        )
        zone_specifier.init_for_year(2011)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(
            DateTuple(2010, 12, 1, 0, 'w'), matches[0].start_date_time)
        self.assertEqual(
            DateTuple(2011, 12, 29, 24 * 3600, 'w'), matches[0].until_date_time)
        self.assertEqual('WS', matches[0].zone_era.policy_name)

        self.assertEqual(
            DateTuple(2011, 12, 29, 24 * 3600, 'w'), matches[1].start_date_time)
        self.assertEqual(
            DateTuple(2012, 2, 1, 0, 'w'), matches[1].until_date_time)
        self.assertEqual('WS', matches[1].zone_era.policy_name)

        transitions = zone_specifier.transitions
        self.assertEqual(4, len(transitions))

        self.assertEqual(
            DateTuple(2010, 12, 1, 0, 'w'), transitions[0].start_date_time)
        self.assertEqual(
            DateTuple(2011, 4, 2, 4 * 3600, 'w'),
            transitions[0].until_date_time)
        self.assertEqual(-11 * 3600, transitions[0].offset_seconds)
        self.assertEqual(1 * 3600, transitions[0].delta_seconds)

        self.assertEqual(
            DateTuple(2011, 4, 2, 3 * 3600, 'w'),
            transitions[1].start_date_time)
        self.assertEqual(
            DateTuple(2011, 9, 24, 3 * 3600, 'w'),
            transitions[1].until_date_time)
        self.assertEqual(-11 * 3600, transitions[1].offset_seconds)
        self.assertEqual(0 * 3600, transitions[1].delta_seconds)

        self.assertEqual(
            DateTuple(2011, 9, 24, 4 * 3600, 'w'),
            transitions[2].start_date_time)
        self.assertEqual(
            DateTuple(2011, 12, 30, 0, 'w'), transitions[2].until_date_time)
        self.assertEqual(-11 * 3600, transitions[2].offset_seconds)
        self.assertEqual(1 * 3600, transitions[2].delta_seconds)

        self.assertEqual(
            DateTuple(2011, 12, 31, 0 * 3600, 'w'),
            transitions[3].start_date_time)
        self.assertEqual(
            DateTuple(2012, 2, 1, 0, 'w'), transitions[3].until_date_time)
        self.assertEqual(13 * 3600, transitions[3].offset_seconds)
        self.assertEqual(1 * 3600, transitions[3].delta_seconds)

    def test_Macquarie(self) -> None:
        """Antarctica/Macquarie changes ZoneEra in 2011 using a 'w' time, but
        the ZoneRule transitions use an 's' time, which happens to coincide with
        the change in ZoneEra. The code must treat those 2 transition times as
        the same point in time.

        In TZ version 2020b (specifically commit
        6427fe6c0cca1dc0f8580f8b96348911ad051570 for github.com/eggert/tz on Thu
        Oct 1 23:59:18 2020) adds an additional ZoneEra line for 2010, changing
        this from 2 to 3. Antarctica/Macquarie stays on AEDT all year in 2010.
        """
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_Antarctica_Macquarie),
            viewing_months=14,
        )
        zone_specifier.init_for_year(2010)

        matches = zone_specifier.matches
        self.assertEqual(3, len(matches))

        # Match 0
        self.assertEqual(
            DateTuple(2009, 12, 1, 0, 'w'), matches[0].start_date_time)
        self.assertEqual(
            DateTuple(2010, 1, 1, 0, 'w'), matches[0].until_date_time)
        self.assertEqual('AT', matches[0].zone_era.policy_name)

        # Match 1
        self.assertEqual(
            DateTuple(2010, 1, 1, 0, 'w'), matches[1].start_date_time)
        self.assertEqual(
            DateTuple(2011, 1, 1, 0, 'w'), matches[1].until_date_time)
        self.assertEqual(':', matches[1].zone_era.policy_name)

        # Match 2
        self.assertEqual(
            DateTuple(2011, 1, 1, 0, 'w'), matches[2].start_date_time)
        self.assertEqual(
            DateTuple(2011, 2, 1, 0, 'w'), matches[2].until_date_time)
        self.assertEqual('AT', matches[2].zone_era.policy_name)

        transitions = zone_specifier.transitions
        self.assertEqual(3, len(transitions))

        # Transition 0
        self.assertEqual(
            DateTuple(2009, 12, 1, 0, 'w'), transitions[0].start_date_time)
        self.assertEqual(
            DateTuple(2010, 1, 1, 0, 'w'), transitions[0].until_date_time)
        self.assertEqual(10 * 3600, transitions[0].offset_seconds)
        self.assertEqual(1 * 3600, transitions[0].delta_seconds)

        # Transition 1
        self.assertEqual(
            DateTuple(2010, 1, 1, 0, 'w'), transitions[1].start_date_time)
        self.assertEqual(
            DateTuple(2011, 1, 1, 0, 'w'), transitions[1].until_date_time)
        self.assertEqual(10 * 3600, transitions[1].offset_seconds)
        self.assertEqual(1 * 3600, transitions[1].delta_seconds)

        # Transition 2
        self.assertEqual(
            DateTuple(2011, 1, 1, 0, 'w'), transitions[2].start_date_time)
        self.assertEqual(
            DateTuple(2011, 2, 1, 0, 'w'), transitions[2].until_date_time)
        self.assertEqual(10 * 3600, transitions[2].offset_seconds)
        self.assertEqual(1 * 3600, transitions[2].delta_seconds)

    def test_Simferopol(self) -> None:
        """Asia/Simferopol in 2014 uses a bizarre mixture of 'w' when using EU
        rules (which itself uses 'u' in the UNTIL fields), then uses 's' time to
        switch to Moscow time.
        """
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_Europe_Simferopol),
            viewing_months=14,
        )
        zone_specifier.init_for_year(2014)

        matches = zone_specifier.matches
        self.assertEqual(3, len(matches))

        self.assertEqual(
            DateTuple(2013, 12, 1, 0 * 3600, 'w'), matches[0].start_date_time)
        self.assertEqual(
            DateTuple(2014, 3, 30, 2 * 3600, 'w'), matches[0].until_date_time)
        self.assertEqual('EU', matches[0].zone_era.policy_name)

        self.assertEqual(
            DateTuple(2014, 3, 30, 2 * 3600, 'w'), matches[1].start_date_time)
        self.assertEqual(
            DateTuple(2014, 10, 26, 2 * 3600, 's'), matches[1].until_date_time)
        self.assertEqual('-', matches[1].zone_era.policy_name)

        self.assertEqual(
            DateTuple(2014, 10, 26, 2 * 3600, 's'), matches[2].start_date_time)
        self.assertEqual(
            DateTuple(2015, 2, 1, 0 * 3600, 'w'), matches[2].until_date_time)
        self.assertEqual('-', matches[2].zone_era.policy_name)

        transitions = zone_specifier.transitions
        self.assertEqual(3, len(transitions))

        self.assertEqual(
            DateTuple(2013, 12, 1, 0, 'w'), transitions[0].start_date_time)
        self.assertEqual(
            DateTuple(2014, 3, 30, 2 * 3600, 'w'),
            transitions[0].until_date_time)
        self.assertEqual(2 * 3600, transitions[0].offset_seconds)
        self.assertEqual(0 * 3600, transitions[0].delta_seconds)

        self.assertEqual(
            DateTuple(2014, 3, 30, 4 * 3600, 'w'),
            transitions[1].start_date_time)
        self.assertEqual(
            DateTuple(2014, 10, 26, 2 * 3600, 'w'),
            transitions[1].until_date_time)
        self.assertEqual(4 * 3600, transitions[1].offset_seconds)
        self.assertEqual(0 * 3600, transitions[1].delta_seconds)

        self.assertEqual(
            DateTuple(2014, 10, 26, 1 * 3600, 'w'),
            transitions[2].start_date_time)
        self.assertEqual(
            DateTuple(2015, 2, 1, 0 * 3600, 'w'),
            transitions[2].until_date_time)
        self.assertEqual(3 * 3600, transitions[2].offset_seconds)
        self.assertEqual(0 * 3600, transitions[2].delta_seconds)

    def test_Kamchatka(self) -> None:
        """Asia/Kamchatka uses 's' in the Zone UNTIL and Rule AT fields.
        """
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_Asia_Kamchatka),
            viewing_months=14,
        )
        zone_specifier.init_for_year(2011)

        matches = zone_specifier.matches
        self.assertEqual(2, len(matches))

        self.assertEqual(
            DateTuple(2010, 12, 1, 0 * 3600, 'w'), matches[0].start_date_time)
        self.assertEqual(
            DateTuple(2011, 3, 27, 2 * 3600, 's'), matches[0].until_date_time)
        self.assertEqual('Russia', matches[0].zone_era.policy_name)

        self.assertEqual(
            DateTuple(2011, 3, 27, 2 * 3600, 's'), matches[1].start_date_time)
        self.assertEqual(
            DateTuple(2012, 2, 1, 0 * 3600, 'w'), matches[1].until_date_time)
        self.assertEqual('-', matches[1].zone_era.policy_name)

        transitions = zone_specifier.transitions
        self.assertEqual(2, len(transitions))

        self.assertEqual(
            DateTuple(2010, 12, 1, 0, 'w'), transitions[0].start_date_time)
        self.assertEqual(
            DateTuple(2011, 3, 27, 2 * 3600, 'w'),
            transitions[0].until_date_time)
        self.assertEqual(11 * 3600, transitions[0].offset_seconds)
        self.assertEqual(0 * 3600, transitions[0].delta_seconds)

        self.assertEqual(
            DateTuple(2011, 3, 27, 3 * 3600, 'w'),
            transitions[1].start_date_time)
        self.assertEqual(
            DateTuple(2012, 2, 1, 0 * 3600, 'w'),
            transitions[1].until_date_time)
        self.assertEqual(12 * 3600, transitions[1].offset_seconds)
        self.assertEqual(0 * 3600, transitions[1].delta_seconds)


class TestZoneSpecifierGetTransition(unittest.TestCase):
    def test_get_transition_for_datetime(self) -> None:
        zone_specifier = ZoneSpecifier(
            cast(ZoneInfo, zone_infos.ZONE_INFO_America_Los_Angeles),
            viewing_months=14,
        )

        # Just after a DST transition
        dt = datetime(2000, 4, 2, 3, 0, 0)
        transition = zone_specifier.get_transition_for_datetime(dt)
        self.assertIsNotNone(transition)

        # DST gap does not exist, but a transition should be returned.
        dt = datetime(2000, 4, 2, 2, 59, 59)
        transition = zone_specifier.get_transition_for_datetime(dt)
        self.assertIsNotNone(transition)

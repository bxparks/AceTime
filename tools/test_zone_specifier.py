#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License

import unittest
from zone_specifier import get_candidate_years
from zone_specifier import DateTuple
from zone_specifier import expand_date_tuple
from zone_specifier import normalize_date_tuple


class TestZoneSpecifier(unittest.TestCase):
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


if __name__ == '__main__':
    unittest.main()

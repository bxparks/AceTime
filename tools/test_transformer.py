#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License

import unittest
from transformer import parse_on_day_string
from transformer import hour_string_to_minute


class TestParseOnDayString(unittest.TestCase):
    def test_parse_transition_day(self):
        self.assertEqual((0, 20), parse_on_day_string('20'))
        self.assertEqual((7, 10), parse_on_day_string('Sun>=10'))
        self.assertEqual((5, 0), parse_on_day_string('lastFri'))

    def test_parse_transition_day_fails(self):
        self.assertEqual((0, 0), parse_on_day_string('20ab'))
        self.assertEqual((0, 0), parse_on_day_string('Sun<=10'))
        self.assertEqual((0, 0), parse_on_day_string('lastFriday'))


class TestHourStringToOffsetCode(unittest.TestCase):
    def test_hour_string_to_offset_minutes(self):
        self.assertEqual(0, hour_string_to_minute('0'))
        self.assertEqual(0, hour_string_to_minute('0:00'))
        self.assertEqual(0, hour_string_to_minute('00:00'))
        self.assertEqual(60, hour_string_to_minute('1:00'))
        self.assertEqual(60, hour_string_to_minute('01:00'))
        self.assertEqual(-60, hour_string_to_minute('-1:00'))
        self.assertEqual(-60, hour_string_to_minute('-01:00'))
        self.assertEqual(75, hour_string_to_minute('1:15'))
        self.assertEqual(90, hour_string_to_minute('1:30'))
        self.assertEqual(105, hour_string_to_minute('1:45'))
        self.assertEqual(106, hour_string_to_minute('1:46'))
        self.assertEqual(1500, hour_string_to_minute('25:00'))

    def test_hour_string_to_offset_code_fails(self):
        self.assertEqual(9999, hour_string_to_minute('26:00'))
        self.assertEqual(9999, hour_string_to_minute('1:60'))
        self.assertEqual(9999, hour_string_to_minute('abc'))


if __name__ == '__main__':
    unittest.main()

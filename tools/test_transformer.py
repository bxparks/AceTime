#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License

import unittest
from transformer import parse_on_day_string
from transformer import time_string_to_seconds
from transformer import time_string_to_seconds
from transformer import seconds_to_hms
from transformer import INVALID_SECONDS


class TestParseOnDayString(unittest.TestCase):
    def test_parse_transition_day(self):
        self.assertEqual((0, 20), parse_on_day_string('20'))
        self.assertEqual((7, 10), parse_on_day_string('Sun>=10'))
        self.assertEqual((5, 0), parse_on_day_string('lastFri'))

    def test_parse_transition_day_fails(self):
        self.assertEqual((0, 0), parse_on_day_string('20ab'))
        self.assertEqual((0, 0), parse_on_day_string('Sun<=10'))
        self.assertEqual((0, 0), parse_on_day_string('lastFriday'))


class TestTimeStringToSeconds(unittest.TestCase):
    def test_time_string_to_seconds(self):
        self.assertEqual(0, time_string_to_seconds('0'))
        self.assertEqual(0, time_string_to_seconds('0:00'))
        self.assertEqual(0, time_string_to_seconds('00:00'))
        self.assertEqual(0, time_string_to_seconds('00:00:00'))

        self.assertEqual(3600, time_string_to_seconds('1'))
        self.assertEqual(3720, time_string_to_seconds('1:02'))
        self.assertEqual(3723, time_string_to_seconds('1:02:03'))

        self.assertEqual(-3600, time_string_to_seconds('-1'))
        self.assertEqual(-3720, time_string_to_seconds('-1:02'))
        self.assertEqual(-3723, time_string_to_seconds('-1:02:03'))

    def test_hour_string_to_offset_code_fails(self):
        self.assertEqual(INVALID_SECONDS, time_string_to_seconds('26:00'))
        self.assertEqual(INVALID_SECONDS, time_string_to_seconds('+26:00'))
        self.assertEqual(INVALID_SECONDS, time_string_to_seconds('1:60'))
        self.assertEqual(INVALID_SECONDS, time_string_to_seconds('1:02:60'))
        self.assertEqual(INVALID_SECONDS, time_string_to_seconds('1:02:03:04'))
        self.assertEqual(INVALID_SECONDS, time_string_to_seconds('abc'))

class TestSecondsToHms(unittest.TestCase):
    def test_seconds_to_hms(self):
        self.assertEqual((0, 0, 0), seconds_to_hms(0))
        self.assertEqual((0, 0, 1), seconds_to_hms(1))
        self.assertEqual((0, 1, 1), seconds_to_hms(61))
        self.assertEqual((1, 1, 1), seconds_to_hms(3661))

if __name__ == '__main__':
    unittest.main()

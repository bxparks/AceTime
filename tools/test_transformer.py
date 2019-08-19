#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License

import unittest
import transformer
from collections import OrderedDict
from transformer import _parse_on_day_string
from transformer import _days_in_month
from transformer import calc_day_of_month
from transformer import time_string_to_seconds
from transformer import time_string_to_seconds
from transformer import seconds_to_hms
from transformer import hms_to_seconds
from transformer import div_to_zero
from transformer import truncate_to_granularity
from transformer import INVALID_SECONDS
from transformer import hash_name


class TestParseOnDayString(unittest.TestCase):
    def test_parse_transition_day(self):
        self.assertEqual((0, 20), _parse_on_day_string('20'))
        self.assertEqual((7, 10), _parse_on_day_string('Sun>=10'))
        self.assertEqual((7, -10), _parse_on_day_string('Sun<=10'))
        self.assertEqual((5, 0), _parse_on_day_string('lastFri'))

    def test_parse_transition_day_fails(self):
        self.assertEqual((0, 0), _parse_on_day_string('20ab'))
        self.assertEqual((0, 0), _parse_on_day_string('lastFriday'))


class TestCalcDayOfMonth(unittest.TestCase):
    def test_calc_day_of_month(self):
        # 2013 Mar Fri>=23
        self.assertEqual((3, 29), calc_day_of_month(2013, 3, 5, 23))
        # 2013 Mar Fri>=30, shifts into April
        self.assertEqual((4, 5), calc_day_of_month(2013, 3, 5, 30))
        # 2002 Sep lastFri
        self.assertEqual((9, 27), calc_day_of_month(2002, 9, 5, 0))
        # 2005 Apr Fri<=7
        self.assertEqual((4, 1), calc_day_of_month(2005, 4, 5, -7))
        # 2005 Apr Fri<=1
        self.assertEqual((4, 1), calc_day_of_month(2005, 4, 5, -1))
        # 2006 Apr Fri<=1, shifts into March
        self.assertEqual((3, 31), calc_day_of_month(2006, 4, 5, -1))


class TestDaysInMonth(unittest.TestCase):
    def test_days_in_month(self):
        self.assertEqual(30, _days_in_month(2002, 9)) # Sep
        self.assertEqual(31, _days_in_month(2002, 0)) # Dec of prev year
        self.assertEqual(31, _days_in_month(2002, 13)) # Jan of following year


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

    def test_hms_to_seconds(self):
        self.assertEqual(0, hms_to_seconds(0, 0, 0))
        self.assertEqual(1, hms_to_seconds(0, 0, 1))
        self.assertEqual(61, hms_to_seconds(0, 1, 1))
        self.assertEqual(3661, hms_to_seconds(1, 1, 1))


class TestIntegerDivision(unittest.TestCase):
    def test_div_to_zero(self):
        self.assertEqual(1, div_to_zero(3, 3))
        self.assertEqual(0, div_to_zero(2, 3))
        self.assertEqual(0, div_to_zero(1, 3))
        self.assertEqual(0, div_to_zero(0, 3))
        self.assertEqual(0, div_to_zero(-1, 3))
        self.assertEqual(0, div_to_zero(-2, 3))
        self.assertEqual(-1, div_to_zero(-3, 3))
        self.assertEqual(-1, div_to_zero(-4, 3))
        self.assertEqual(-1, div_to_zero(-5, 3))
        self.assertEqual(-2, div_to_zero(-6, 3))

    def test_truncate_to_granularity(self):
        self.assertEqual(0, truncate_to_granularity(0, 15))
        self.assertEqual(0, truncate_to_granularity(14, 15))
        self.assertEqual(15, truncate_to_granularity(15, 15))
        self.assertEqual(15, truncate_to_granularity(16, 15))
        self.assertEqual(0, truncate_to_granularity(-1, 15))
        self.assertEqual(-15, truncate_to_granularity(-15, 15))
        self.assertEqual(-15, truncate_to_granularity(-29, 15))
        self.assertEqual(-30, truncate_to_granularity(-31, 15))


class TestAddString(unittest.TestCase):
    def test_add_string(self):
        strings = OrderedDict()
        self.assertEqual(0, transformer.add_string(strings, 'a'))
        self.assertEqual(0, transformer.add_string(strings, 'a'))
        self.assertEqual(1, transformer.add_string(strings, 'b'))
        self.assertEqual(2, transformer.add_string(strings, 'd'))
        self.assertEqual(3, transformer.add_string(strings, 'c'))
        self.assertEqual(2, transformer.add_string(strings, 'd'))

class TestHash(unittest.TestCase):
    def test_hash(self):
        self.assertEqual(5381, hash_name(''))
        self.assertEqual(177670, hash_name('a'));
        self.assertEqual(177671, hash_name('b'));
        self.assertEqual(5863208, hash_name('ab'));
        self.assertEqual(193485963, hash_name('abc'));
        self.assertEqual(2090069583, hash_name('abcd'));
        self.assertEqual(252819604, hash_name('abcde'));

if __name__ == '__main__':
    unittest.main()

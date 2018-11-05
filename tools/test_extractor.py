#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License

import unittest
from extractor import parse_on_day_string
from extractor import parse_at_hour_string
from extractor import hour_string_to_offset_code


class TestParseOnDayString(unittest.TestCase):
    def test_parse_transition_day(self):
        self.assertEqual((0, 20), parse_on_day_string('20'))
        self.assertEqual((7, 10), parse_on_day_string('Sun>=10'))
        self.assertEqual((5, 0), parse_on_day_string('lastFri'))

    def test_parse_transition_day_fails(self):
        self.assertRaises(Exception, parse_on_day_string, '20ab')
        self.assertRaises(Exception, parse_on_day_string, 'Sun<=10')
        self.assertRaises(Exception, parse_on_day_string, 'lastFriday')

class TestParseAtHourString(unittest.TestCase):
    def test_parse_at_hour_string(self):
        self.assertEqual(('2:00', 'w'), parse_at_hour_string('2:00'))
        self.assertEqual(('2:00', 'w'), parse_at_hour_string('2:00w'))
        self.assertEqual(('12:00', 's'), parse_at_hour_string('12:00s'))
        self.assertEqual(('12:00', 'g'), parse_at_hour_string('12:00g'))
        self.assertEqual(('12:00', 'u'), parse_at_hour_string('12:00u'))

    def test_pase_at_hour_string_fails(self):
        self.assertRaises(Exception, parse_at_hour_string, '2:00p')

class TestHourStringToOffsetCode(unittest.TestCase):
    def test_hour_string_to_offset_code(self):
        self.assertEqual(0, hour_string_to_offset_code('0'))
        self.assertEqual(0, hour_string_to_offset_code('0:00'))
        self.assertEqual(0, hour_string_to_offset_code('00:00'))
        self.assertEqual(4, hour_string_to_offset_code('1:00'))
        self.assertEqual(4, hour_string_to_offset_code('01:00'))
        self.assertEqual(-4, hour_string_to_offset_code('-1:00'))
        self.assertEqual(-4, hour_string_to_offset_code('-01:00'))
        self.assertEqual(5, hour_string_to_offset_code('1:15'))
        self.assertEqual(6, hour_string_to_offset_code('1:30'))
        self.assertEqual(7, hour_string_to_offset_code('1:45'))

    def test_hour_string_to_offset_code_fails(self):
        self.assertRaises(Exception, hour_string_to_offset_code, '1:01')

if __name__ == '__main__':
    unittest.main()

#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License

import unittest
from extractor import parse_at_hour_string
from extractor import hour_string_to_offset_minutes


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
    def test_hour_string_to_offset_minutes(self):
        self.assertEqual(0, hour_string_to_offset_minutes('0'))
        self.assertEqual(0, hour_string_to_offset_minutes('0:00'))
        self.assertEqual(0, hour_string_to_offset_minutes('00:00'))
        self.assertEqual(60, hour_string_to_offset_minutes('1:00'))
        self.assertEqual(60, hour_string_to_offset_minutes('01:00'))
        self.assertEqual(-60, hour_string_to_offset_minutes('-1:00'))
        self.assertEqual(-60, hour_string_to_offset_minutes('-01:00'))
        self.assertEqual(75, hour_string_to_offset_minutes('1:15'))
        self.assertEqual(90, hour_string_to_offset_minutes('1:30'))
        self.assertEqual(105, hour_string_to_offset_minutes('1:45'))
        self.assertEqual(106, hour_string_to_offset_minutes('1:46'))

    def test_hour_string_to_offset_code_fails(self):
        self.assertRaises(Exception, hour_string_to_offset_minutes, 'abc')


if __name__ == '__main__':
    unittest.main()

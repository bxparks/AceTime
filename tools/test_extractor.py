#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License

import unittest
from extractor import parse_at_time_string


class TestParseAtHourString(unittest.TestCase):
    def test_parse_at_time_string(self):
        self.assertEqual(('2:00', ''), parse_at_time_string('2:00'))
        self.assertEqual(('2:00', 'w'), parse_at_time_string('2:00w'))
        self.assertEqual(('12:00', 's'), parse_at_time_string('12:00s'))
        self.assertEqual(('12:00', 'g'), parse_at_time_string('12:00g'))
        self.assertEqual(('12:00', 'u'), parse_at_time_string('12:00u'))

    def test_pase_at_time_string_fails(self):
        self.assertRaises(Exception, parse_at_time_string, '2:00p')


if __name__ == '__main__':
    unittest.main()

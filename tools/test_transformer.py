#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License

import unittest
from transformer import parse_on_day_string


class TestParseOnDayString(unittest.TestCase):
    def test_parse_transition_day(self):
        self.assertEqual((0, 20), parse_on_day_string('20'))
        self.assertEqual((7, 10), parse_on_day_string('Sun>=10'))
        self.assertEqual((5, 0), parse_on_day_string('lastFri'))

    def test_parse_transition_day_fails(self):
        self.assertEqual((0, 0), parse_on_day_string('20ab'))
        self.assertEqual((0, 0), parse_on_day_string('Sun<=10'))
        self.assertEqual((0, 0), parse_on_day_string('lastFriday'))


if __name__ == '__main__':
    unittest.main()

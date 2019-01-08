#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License

import unittest
from zone_agent import get_candidate_years


class TestGetCandidateYears(unittest.TestCase):
    def test_get_candidate_years(self):
        self.assertEqual({1, 2, 3}, get_candidate_years(1, 4, 2, 3))
        self.assertEqual({1, 2, 3}, get_candidate_years(0, 4, 2, 3))
        self.assertEqual({4}, get_candidate_years(4, 5, 2, 3))
        self.assertEqual({2}, get_candidate_years(0, 2, 5, 6))
        self.assertEqual({4, 5}, get_candidate_years(0, 5, 5, 6))
        self.assertEqual({0, 1, 2}, get_candidate_years(0, 2, 0, 2))
        self.assertEqual({1, 2, 3, 4}, get_candidate_years(0, 4, 2, 4))


if __name__ == '__main__':
    unittest.main()

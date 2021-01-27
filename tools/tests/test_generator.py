# Copyright 2021 Brian T. Park
#
# MIT License

import unittest
from generator.argenerator import _compressed_name_to_c_string


class TestArduinoGenerator(unittest.TestCase):
    def test_compressed_name_to_c_string(self) -> None:
        self.assertEqual('"hello"', _compressed_name_to_c_string('hello'))
        self.assertEqual('"\\x01"', _compressed_name_to_c_string('\u0001'))
        self.assertEqual(
            '"\\x01" "hello"',
            _compressed_name_to_c_string('\u0001hello')
        )
        self.assertEqual(
            '"hello" "\\x02"',
            _compressed_name_to_c_string('hello\u0002')
        )
        self.assertEqual(
            '"\\x01" "hello" "\\x02"',
            _compressed_name_to_c_string('\u0001hello\u0002')
        )
        self.assertEqual(
            '"\\x01" "\\x02"',
            _compressed_name_to_c_string('\u0001\u0002')
        )

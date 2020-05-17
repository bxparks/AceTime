import unittest
from datetime import datetime
from acetz import gettz as agettz
from dateutil.tz import gettz


class TestAceTz(unittest.TestCase):

    def test(self) -> None:
        dtz = gettz('America/Los_Angeles')
        ddt = datetime(2000, 1, 2, 3, 4, 5, tzinfo=dtz)

        atz = agettz('America/Los_Angeles')
        adt = datetime(2000, 1, 2, 3, 4, 5, tzinfo=atz)

        self.assertEqual(ddt.year, adt.year)
        self.assertEqual(ddt.month, adt.month)
        self.assertEqual(ddt.day, adt.day)
        self.assertEqual(ddt.hour, adt.hour)
        self.assertEqual(ddt.minute, adt.minute)
        self.assertEqual(ddt.second, adt.second)
        self.assertEqual(ddt.year, adt.year)

        self.assertEqual(ddt.timestamp(), adt.timestamp())
        self.assertEqual(
            ddt.utcoffset().total_seconds(),
            adt.utcoffset().total_seconds(),
        )
        self.assertEqual(ddt.tzinfo.tzname(ddt), adt.tzinfo.tzname(adt))

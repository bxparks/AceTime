import unittest
from datetime import datetime, timedelta
from acetz import gettz as agettz, SECONDS_SINCE_UNIX_EPOCH
from dateutil.tz import gettz


class TestAceTz(unittest.TestCase):

    def test_constructor(self) -> None:
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

    def test_add_subtract_around_dst_shift(self) -> None:
        atz = agettz('America/Los_Angeles')
        adt = datetime(2000, 4, 2, 3, 0, 0, tzinfo=atz)
        self.assertEqual(
            7984800 + SECONDS_SINCE_UNIX_EPOCH,
            adt.timestamp(),
        )

        # Not normalized
        adt_minus = adt - timedelta(seconds=1)
        expected = datetime(2000, 4, 2, 2, 59, 59, tzinfo=atz)
        self.assertEqual(expected, adt_minus)

        # Calculate fro epoch seconds.
        adt_minus = datetime.fromtimestamp(
            7984799 + SECONDS_SINCE_UNIX_EPOCH,
            tz=atz,
        )
        expected = datetime(2000, 4, 2, 1, 59, 59, tzinfo=atz)
        self.assertEqual(expected, adt_minus)

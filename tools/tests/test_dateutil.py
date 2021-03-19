import unittest
from datetime import datetime, timedelta, timezone
from dateutil.tz import gettz

from data_types.at_types import SECONDS_SINCE_UNIX_EPOCH


class TestDateUtil(unittest.TestCase):

    def test_constructor(self) -> None:
        dtz = gettz('America/Los_Angeles')
        ddt = datetime(2000, 1, 2, 3, 4, 5, tzinfo=dtz)

        self.assertEqual(2000, ddt.year)
        self.assertEqual(1, ddt.month)
        self.assertEqual(2, ddt.day)
        self.assertEqual(3, ddt.hour)
        self.assertEqual(4, ddt.minute)
        self.assertEqual(5, ddt.second)

        # date +%s -d '2000-01-02T03:04:05-08:00'
        self.assertEqual(946811045, int(ddt.timestamp()))

        ddt_utcoffset = ddt.utcoffset()
        assert(ddt_utcoffset is not None)
        self.assertEqual(-8 * 3600, ddt_utcoffset.total_seconds())

        assert(ddt.tzinfo is not None)
        self.assertEqual("PST", ddt.tzinfo.tzname(ddt))

    def test_before_fall_back(self) -> None:
        tz = gettz('America/Los_Angeles')
        assert(tz is not None)

        # One second before DST shift, 01:59:59 UTC-7
        epoch_seconds = 26125199
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH
        dtu = datetime.fromtimestamp(unix_seconds, tz=timezone.utc)

        # Date from epoch seconds.
        dtt = dtu.astimezone(tz)
        self.assertEqual(unix_seconds, int(dtt.timestamp()))
        self.assertEqual(2000, dtt.year)
        self.assertEqual(10, dtt.month)
        self.assertEqual(29, dtt.day)
        self.assertEqual(1, dtt.hour)
        self.assertEqual(59, dtt.minute)
        self.assertEqual(59, dtt.second)
        self.assertEqual(0, dtt.fold)
        self.assertEqual("PDT", dtt.tzname())
        self.assertEqual(timedelta(hours=-7), dtt.utcoffset())
        self.assertEqual(timedelta(hours=1), dtt.dst())

        # Date from component
        dtc = datetime(2000, 10, 29, 1, 59, 59, fold=0, tzinfo=tz)
        self.assertEqual(unix_seconds, int(dtc.timestamp()))
        self.assertEqual(2000, dtc.year)
        self.assertEqual(10, dtc.month)
        self.assertEqual(29, dtc.day)
        self.assertEqual(1, dtc.hour)
        self.assertEqual(59, dtc.minute)
        self.assertEqual(59, dtc.second)
        self.assertEqual(0, dtc.fold)
        self.assertEqual("PDT", dtc.tzname())
        self.assertEqual(timedelta(hours=-7), dtc.utcoffset())
        self.assertEqual(timedelta(hours=1), dtc.dst())

        self.assertEqual(dtc, dtt)

    def test_after_fall_back(self) -> None:
        tz = gettz('America/Los_Angeles')
        assert(tz is not None)

        # Just after DST fall back 01:00:00 UTC-8
        epoch_seconds = 26125200
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH
        dtu = datetime.fromtimestamp(unix_seconds, tz=timezone.utc)

        # Date from epoch seconds.
        dtt = dtu.astimezone(tz)
        self.assertEqual(unix_seconds, int(dtt.timestamp()))
        self.assertEqual(2000, dtt.year)
        self.assertEqual(10, dtt.month)
        self.assertEqual(29, dtt.day)
        self.assertEqual(1, dtt.hour)
        self.assertEqual(0, dtt.minute)
        self.assertEqual(0, dtt.second)
        self.assertEqual(1, dtt.fold)
        self.assertEqual("PST", dtt.tzname())
        self.assertEqual(timedelta(hours=-8), dtt.utcoffset())
        self.assertEqual(timedelta(hours=0), dtt.dst())

        # Date from component
        dtc = datetime(2000, 10, 29, 1, 0, 0, fold=1, tzinfo=tz)
        self.assertEqual(unix_seconds, int(dtc.timestamp()))
        self.assertEqual(2000, dtc.year)
        self.assertEqual(10, dtc.month)
        self.assertEqual(29, dtc.day)
        self.assertEqual(1, dtc.hour)
        self.assertEqual(0, dtc.minute)
        self.assertEqual(0, dtc.second)
        self.assertEqual(1, dtc.fold)
        self.assertEqual("PST", dtc.tzname())
        self.assertEqual(timedelta(hours=-8), dtc.utcoffset())
        self.assertEqual(timedelta(hours=0), dtc.dst())

        self.assertEqual(dtc, dtt)

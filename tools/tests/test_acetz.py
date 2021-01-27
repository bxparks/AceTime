import unittest
from datetime import datetime, timedelta, timezone
from dateutil.tz import gettz
from data_types.at_types import SECONDS_SINCE_UNIX_EPOCH
from acetz import gettz as agettz


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
        ddt_utcoffset = ddt.utcoffset()
        assert(ddt_utcoffset is not None)
        adt_utcoffset = adt.utcoffset()
        assert(adt_utcoffset is not None)
        self.assertEqual(
            ddt_utcoffset.total_seconds(),
            adt_utcoffset.total_seconds(),
        )
        assert(ddt.tzinfo is not None)
        assert(adt.tzinfo is not None)
        self.assertEqual(ddt.tzinfo.tzname(ddt), adt.tzinfo.tzname(adt))

    def test_before_spring_forward(self) -> None:
        tz = agettz('America/Los_Angeles')
        # zs = tz.zone_specifier()

        # One second before DST shift, 01:59:59 UTC-8
        epoch_seconds = 7984799
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH
        dtu = datetime.fromtimestamp(unix_seconds, tz=timezone.utc)

        # Date from epoch seconds.
        dtt = dtu.astimezone(tz)
        self.assertEqual(unix_seconds, dtt.timestamp())
        self.assertEqual(2000, dtt.year)
        self.assertEqual(4, dtt.month)
        self.assertEqual(2, dtt.day)
        self.assertEqual(1, dtt.hour)
        self.assertEqual(59, dtt.minute)
        self.assertEqual(59, dtt.second)

        # Date from component
        dtc = datetime(2000, 4, 2, 1, 59, 59, tzinfo=tz)
        self.assertEqual(unix_seconds, dtc.timestamp())
        self.assertEqual(2000, dtc.year)
        self.assertEqual(4, dtc.month)
        self.assertEqual(2, dtc.day)
        self.assertEqual(1, dtc.hour)
        self.assertEqual(59, dtc.minute)
        self.assertEqual(59, dtc.second)
        self.assertEqual(timedelta(hours=-8), tz.utcoffset(dtc))
        self.assertEqual(timedelta(hours=0), tz.dst(dtc))

        self.assertEqual(dtc, dtt)

    def test_after_spring_forward(self) -> None:
        tz = agettz('America/Los_Angeles')
        # zs = tz.zone_specifier()

        # Right after DST forward shift, 03:00:00 UTC-7
        epoch_seconds = 7984800
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH
        dtu = datetime.fromtimestamp(unix_seconds, tz=timezone.utc)

        # Date from epoch seconds
        dtt = dtu.astimezone(tz)
        self.assertEqual(unix_seconds, dtt.timestamp())
        self.assertEqual(2000, dtt.year)
        self.assertEqual(4, dtt.month)
        self.assertEqual(2, dtt.day)
        self.assertEqual(3, dtt.hour)
        self.assertEqual(0, dtt.minute)
        self.assertEqual(0, dtt.second)

        # Date from component
        dtc = datetime(2000, 4, 2, 3, 0, 0, tzinfo=tz)
        self.assertEqual(unix_seconds, int(dtc.timestamp()))
        self.assertEqual(2000, dtc.year)
        self.assertEqual(4, dtc.month)
        self.assertEqual(2, dtc.day)
        self.assertEqual(3, dtc.hour)
        self.assertEqual(0, dtc.minute)
        self.assertEqual(0, dtc.second)
        self.assertEqual(timedelta(hours=-7), tz.utcoffset(dtc))
        self.assertEqual(timedelta(hours=1), tz.dst(dtc))

        self.assertEqual(dtc, dtt)

    def test_before_fall_back(self) -> None:
        tz = agettz('America/Los_Angeles')
        # zs = tz.zone_specifier()

        # One second before DST shift, 01:59:59 UTC-7
        epoch_seconds = 26125199
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH
        dtu = datetime.fromtimestamp(unix_seconds, tz=timezone.utc)

        # Date from epoch seconds.
        dtt = dtu.astimezone(tz)
        # Round-trip does not work.
        # self.assertEqual(unix_seconds, dtt.timestamp())
        self.assertEqual(2000, dtt.year)
        self.assertEqual(10, dtt.month)
        self.assertEqual(29, dtt.day)
        self.assertEqual(1, dtt.hour)
        self.assertEqual(59, dtt.minute)
        self.assertEqual(59, dtt.second)

        # Date from component
        # dtc = datetime(2000, 10, 29, 1, 59, 59, tzinfo=tz)
        # self.assertEqual(unix_seconds, dtc.timestamp())
        # self.assertEqual(2000, dtc.year)
        # self.assertEqual(10, dtc.month)
        # self.assertEqual(29, dtc.day)
        # self.assertEqual(1, dtc.hour)
        # self.assertEqual(59, dtc.minute)
        # self.assertEqual(59, dtc.second)
        # self.assertEqual(timedelta(hours=-7), tz.utcoffset(dtc))
        # self.assertEqual(timedelta(hours=1), tz.dst(dtc))

        # self.assertEqual(dtc, dtt)

    def test_after_fall_back(self) -> None:
        tz = agettz('America/Los_Angeles')
        # zs = tz.zone_specifier()

        # Just after DST fall back 01:00:00 UTC-8
        epoch_seconds = 26125200
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH
        dtu = datetime.fromtimestamp(unix_seconds, tz=timezone.utc)

        # Date from epoch seconds.
        dtt = dtu.astimezone(tz)
        self.assertEqual(unix_seconds, dtt.timestamp())
        self.assertEqual(2000, dtt.year)
        self.assertEqual(10, dtt.month)
        self.assertEqual(29, dtt.day)
        self.assertEqual(1, dtt.hour)
        self.assertEqual(0, dtt.minute)
        self.assertEqual(0, dtt.second)

        # Date from component
        dtc = datetime(2000, 10, 29, 1, 0, 0, tzinfo=tz)
        self.assertEqual(unix_seconds, dtc.timestamp())
        self.assertEqual(2000, dtc.year)
        self.assertEqual(10, dtc.month)
        self.assertEqual(29, dtc.day)
        self.assertEqual(1, dtc.hour)
        self.assertEqual(0, dtc.minute)
        self.assertEqual(0, dtc.second)
        self.assertEqual(timedelta(hours=-8), tz.utcoffset(dtc))
        self.assertEqual(timedelta(hours=0), tz.dst(dtc))

        self.assertEqual(dtc, dtt)

    def test_way_after_fall_back(self) -> None:
        tz = agettz('America/Los_Angeles')
        # zs = tz.zone_specifier()

        # Just after DST fall back 02:00:00 UTC-8
        epoch_seconds = 26125200 + 3600
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH
        dtu = datetime.fromtimestamp(unix_seconds, tz=timezone.utc)

        # Date from epoch seconds.
        dtt = dtu.astimezone(tz)
        self.assertEqual(unix_seconds, dtt.timestamp())
        self.assertEqual(2000, dtt.year)
        self.assertEqual(10, dtt.month)
        self.assertEqual(29, dtt.day)
        self.assertEqual(2, dtt.hour)
        self.assertEqual(0, dtt.minute)
        self.assertEqual(0, dtt.second)

        # Date from component
        dtc = datetime(2000, 10, 29, 2, 0, 0, tzinfo=tz)
        self.assertEqual(unix_seconds, dtc.timestamp())
        self.assertEqual(2000, dtc.year)
        self.assertEqual(10, dtc.month)
        self.assertEqual(29, dtc.day)
        self.assertEqual(2, dtc.hour)
        self.assertEqual(0, dtc.minute)
        self.assertEqual(0, dtc.second)
        self.assertEqual(timedelta(hours=-8), tz.utcoffset(dtc))
        self.assertEqual(timedelta(hours=0), tz.dst(dtc))

        self.assertEqual(dtc, dtt)


class TestDateUtil(unittest.TestCase):

    def test_before_fall_back(self) -> None:
        tz = gettz('America/Los_Angeles')
        assert(tz is not None)

        # One second before DST shift, 01:59:59 UTC-7
        epoch_seconds = 26125199
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH
        dtu = datetime.fromtimestamp(unix_seconds, tz=timezone.utc)

        # Date from epoch seconds.
        dtt = dtu.astimezone(tz)
        self.assertEqual(unix_seconds, dtt.timestamp())
        self.assertEqual(2000, dtt.year)
        self.assertEqual(10, dtt.month)
        self.assertEqual(29, dtt.day)
        self.assertEqual(1, dtt.hour)
        self.assertEqual(59, dtt.minute)
        self.assertEqual(59, dtt.second)
        self.assertEqual(0, dtt.fold)

        # Date from component
        dtc = datetime(2000, 10, 29, 1, 59, 59, fold=0, tzinfo=tz)
        self.assertEqual(unix_seconds, dtc.timestamp())
        self.assertEqual(2000, dtc.year)
        self.assertEqual(10, dtc.month)
        self.assertEqual(29, dtc.day)
        self.assertEqual(1, dtc.hour)
        self.assertEqual(59, dtc.minute)
        self.assertEqual(59, dtc.second)
        self.assertEqual(0, dtt.fold)
        self.assertEqual(timedelta(hours=-7), tz.utcoffset(dtc))
        self.assertEqual(timedelta(hours=1), tz.dst(dtc))

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
        self.assertEqual(unix_seconds, dtt.timestamp())
        self.assertEqual(2000, dtt.year)
        self.assertEqual(10, dtt.month)
        self.assertEqual(29, dtt.day)
        self.assertEqual(1, dtt.hour)
        self.assertEqual(0, dtt.minute)
        self.assertEqual(0, dtt.second)
        self.assertEqual(1, dtt.fold)

        # Date from component
        dtc = datetime(2000, 10, 29, 1, 0, 0, fold=1, tzinfo=tz)
        self.assertEqual(unix_seconds, dtc.timestamp())
        self.assertEqual(2000, dtc.year)
        self.assertEqual(10, dtc.month)
        self.assertEqual(29, dtc.day)
        self.assertEqual(1, dtc.hour)
        self.assertEqual(0, dtc.minute)
        self.assertEqual(0, dtc.second)
        self.assertEqual(1, dtt.fold)
        self.assertEqual(timedelta(hours=-8), tz.utcoffset(dtc))
        self.assertEqual(timedelta(hours=0), tz.dst(dtc))

        self.assertEqual(dtc, dtt)

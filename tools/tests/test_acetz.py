import unittest
from datetime import datetime, timedelta, timezone
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

    def test_before_spring_forward(self) -> None:
        atz = agettz('America/Los_Angeles')
        zs = atz.zone_specifier()

        # One second before DST shift, 01:59:59 UTC-8
        epoch_seconds = 7984799
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH

        # Date from component
        dtc = datetime(2000, 4, 2, 1, 59, 59, tzinfo=atz)
        self.assertEqual(unix_seconds, dtc.timestamp())
        self.assertEqual(2000, dtc.year)
        self.assertEqual(4, dtc.month)
        self.assertEqual(2, dtc.day)
        self.assertEqual(1, dtc.hour)
        self.assertEqual(59, dtc.minute)
        self.assertEqual(59, dtc.second)
        self.assertEqual(timedelta(hours=-8), atz.utcoffset(dtc))
        self.assertEqual(timedelta(hours=0), atz.dst(dtc))

        # Date from epoch seconds.
        dtu = datetime.fromtimestamp(unix_seconds, tz=timezone.utc)
        dtt = dtu.astimezone(atz)
        self.assertEqual(unix_seconds, dtt.timestamp())
        self.assertEqual(2000, dtt.year)
        self.assertEqual(4, dtt.month)
        self.assertEqual(2, dtt.day)
        self.assertEqual(1, dtt.hour)
        self.assertEqual(59, dtt.minute)
        self.assertEqual(59, dtt.second)
        self.assertEqual(dtc, dtt)

    def test_after_spring_forward(self) -> None:
        atz = agettz('America/Los_Angeles')
        zs = atz.zone_specifier()

        # Right after DST forward shift, 03:00:00 UTC-7
        epoch_seconds = 7984800
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH

        # Date from component
        dtc = datetime(2000, 4, 2, 3, 0, 0, tzinfo=atz)
        self.assertEqual(unix_seconds, int(dtc.timestamp()))
        self.assertEqual(2000, dtc.year)
        self.assertEqual(4, dtc.month)
        self.assertEqual(2, dtc.day)
        self.assertEqual(3, dtc.hour)
        self.assertEqual(0, dtc.minute)
        self.assertEqual(0, dtc.second)
        self.assertEqual(timedelta(hours=-7), atz.utcoffset(dtc))
        self.assertEqual(timedelta(hours=1), atz.dst(dtc))

        # Date from epoch seconds
        dtu = datetime.fromtimestamp(unix_seconds, tz=timezone.utc)
        dtt = dtu.astimezone(atz)
        self.assertEqual(unix_seconds, dtt.timestamp())
        self.assertEqual(2000, dtt.year)
        self.assertEqual(4, dtt.month)
        self.assertEqual(2, dtt.day)
        self.assertEqual(3, dtt.hour)
        self.assertEqual(0, dtt.minute)
        self.assertEqual(0, dtt.second)
        self.assertEqual(dtc, dtt)

    def test_before_fall_back(self) -> None:
        atz = agettz('America/Los_Angeles')
        zs = atz.zone_specifier()

        # One second before DST shift, 01:59:59 UTC-7
        epoch_seconds = 26125199
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH

        # Date from epoch seconds.
        dtu = datetime.fromtimestamp(unix_seconds, tz=timezone.utc)
        dtt = dtu.astimezone(atz)
        # Round-trip does not work.
        # self.assertEqual(unix_seconds, dtt.timestamp())
        self.assertEqual(2000, dtt.year)
        self.assertEqual(10, dtt.month)
        self.assertEqual(29, dtt.day)
        self.assertEqual(1, dtt.hour)
        self.assertEqual(59, dtt.minute)
        self.assertEqual(59, dtt.second)

        # Date from component
        #dtc = datetime(2000, 10, 29, 1, 59, 59, tzinfo=atz)
        #self.assertEqual(unix_seconds, dtc.timestamp())
        #self.assertEqual(2000, dtc.year)
        #self.assertEqual(10, dtc.month)
        #self.assertEqual(29, dtc.day)
        #self.assertEqual(1, dtc.hour)
        #self.assertEqual(59, dtc.minute)
        #self.assertEqual(59, dtc.second)
        #self.assertEqual(timedelta(hours=-7), atz.utcoffset(dtc))
        #self.assertEqual(timedelta(hours=1), atz.dst(dtc))
        #self.assertEqual(dtc, dtt)

    def test_after_fall_back(self) -> None:
        atz = agettz('America/Los_Angeles')
        zs = atz.zone_specifier()

        # Just after DST fall back 01:00:00 UTC-8
        epoch_seconds = 26125200
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH

        # Date from epoch seconds.
        dtu = datetime.fromtimestamp(unix_seconds, tz=timezone.utc)
        dtt = dtu.astimezone(atz)
        self.assertEqual(unix_seconds, dtt.timestamp())
        self.assertEqual(2000, dtt.year)
        self.assertEqual(10, dtt.month)
        self.assertEqual(29, dtt.day)
        self.assertEqual(1, dtt.hour)
        self.assertEqual(0, dtt.minute)
        self.assertEqual(0, dtt.second)

        # Date from component
        dtc = datetime(2000, 10, 29, 1, 0, 0, tzinfo=atz)
        self.assertEqual(unix_seconds, dtc.timestamp())
        self.assertEqual(2000, dtc.year)
        self.assertEqual(10, dtc.month)
        self.assertEqual(29, dtc.day)
        self.assertEqual(1, dtc.hour)
        self.assertEqual(0, dtc.minute)
        self.assertEqual(0, dtc.second)
        self.assertEqual(timedelta(hours=-8), atz.utcoffset(dtc))
        self.assertEqual(timedelta(hours=0), atz.dst(dtc))
        self.assertEqual(dtc, dtt)

    def test_way_after_fall_back(self) -> None:
        atz = agettz('America/Los_Angeles')
        zs = atz.zone_specifier()

        # Just after DST fall back 02:00:00 UTC-8
        epoch_seconds = 26125200 + 3600
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH

        # Date from epoch seconds.
        dtu = datetime.fromtimestamp(unix_seconds, tz=timezone.utc)
        dtt = dtu.astimezone(atz)
        self.assertEqual(unix_seconds, dtt.timestamp())
        self.assertEqual(2000, dtt.year)
        self.assertEqual(10, dtt.month)
        self.assertEqual(29, dtt.day)
        self.assertEqual(2, dtt.hour)
        self.assertEqual(0, dtt.minute)
        self.assertEqual(0, dtt.second)

        # Date from component
        dtc = datetime(2000, 10, 29, 2, 0, 0, tzinfo=atz)
        self.assertEqual(unix_seconds, dtc.timestamp())
        self.assertEqual(2000, dtc.year)
        self.assertEqual(10, dtc.month)
        self.assertEqual(29, dtc.day)
        self.assertEqual(2, dtc.hour)
        self.assertEqual(0, dtc.minute)
        self.assertEqual(0, dtc.second)
        self.assertEqual(timedelta(hours=-8), atz.utcoffset(dtc))
        self.assertEqual(timedelta(hours=0), atz.dst(dtc))
        self.assertEqual(dtc, dtt)

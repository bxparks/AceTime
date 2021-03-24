# Copyright 2020 Brian T. Park
#
# MIT License

# import sys, traceback
from typing import cast, Optional
from datetime import datetime, tzinfo, timedelta, timezone
from zonedbpy import zone_infos
from data_types.at_types import SECONDS_SINCE_UNIX_EPOCH
from zone_processor.zone_specifier import ZoneSpecifier
from zone_processor.inline_zone_info import ZoneInfo

__version__ = '0.1'


class acetz(tzinfo):
    """An implementation of datetime.tzinfo using the ZoneSpecifier class
    from AceTime/tools.
    """

    def __init__(self, zone_info: ZoneInfo):
        self.zone_info = zone_info
        self.zs = ZoneSpecifier(zone_info, use_python_transition=True)

    def utcoffset(self, dt: Optional[datetime]) -> timedelta:
        assert dt
        info = self.zs.get_timezone_info_for_datetime(dt)
        if not info:
            raise Exception(
                f'Unknown timezone info for '
                f'{dt.year:04}-{dt.month:02}-{dt.day:02} '
                f'{dt.hour:02}:{dt.minute:02}:{dt.second:02}'
            )

        return timedelta(seconds=info.total_offset)

    def dst(self, dt: Optional[datetime]) -> timedelta:
        assert dt
        offset_info = self.zs.get_timezone_info_for_datetime(dt)
        if not offset_info:
            raise Exception(
                f'Unknown timezone info for '
                f'{dt.year:04}-{dt.month:02}-{dt.day:02} '
                f'{dt.hour:02}:{dt.minute:02}:{dt.second:02}'
            )
        return timedelta(seconds=offset_info.dst_offset)

    def tzname(self, dt: Optional[datetime]) -> str:
        assert dt
        offset_info = self.zs.get_timezone_info_for_datetime(dt)
        if not offset_info:
            raise Exception(
                f'Unknown timezone info for '
                f'{dt.year:04}-{dt.month:02}-{dt.day:02} '
                f'{dt.hour:02}:{dt.minute:02}:{dt.second:02}'
            )
        return offset_info.abbrev

    def fromutc(self, dt: Optional[datetime]) -> datetime:
        """Override the default implementation in tzinfo which does not make
        sense for acetz.

        The 'dt' passed into this function from datetime.astimezone() is a weird
        one: the components are in UTC time, but the timezone is the target
        tzinfo, in other words, the same acetz as self.

        Warning: Do NOT call dt.isoformat() from this method, because it causes
        an infinite recursion as it tries to figure out the UTC offset. Use
        {dt.date()} and {dt.time()} instead.
        """
        if not isinstance(dt, datetime):
            raise TypeError("fromutc() requires a datetime argument")
        if dt.tzinfo is not self:
            raise ValueError("dt.tzinfo is not self")

        # Extract the epoch_seconds of the source 'dt'
        assert dt is not None
        utcdt = dt.replace(tzinfo=timezone.utc)
        unix_seconds = int(utcdt.timestamp())
        epoch_seconds = unix_seconds - SECONDS_SINCE_UNIX_EPOCH

        # Search the transitions for the matching Transition
        offset_info = self.zs.get_timezone_info_for_seconds(epoch_seconds)
        if not offset_info:
            raise ValueError(f"transition not found for {epoch_seconds}")

        # Convert the date/time fields into local date/time and attach
        # the current acetz object.
        newutcdt = utcdt + timedelta(seconds=offset_info.total_offset)
        newdt = newutcdt.replace(tzinfo=self, fold=offset_info.fold)

        return newdt

    def zone_specifier(self) -> ZoneSpecifier:
        return self.zs


def gettz(zone_name: str) -> acetz:
    zone_info = cast(ZoneInfo, zone_infos.ZONE_INFO_MAP.get(zone_name))
    if not zone_info:
        raise Exception(f"Zone '{zone_name}' not found")
    return acetz(zone_info)

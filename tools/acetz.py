from typing import cast, Optional
from datetime import datetime, tzinfo, timedelta
from zonedbpy import zone_infos
from zonedb.zone_specifier import (  # noqa
    ZoneSpecifier,
    SECONDS_SINCE_UNIX_EPOCH,
)
from zonedb.ingenerator import ZoneInfo

__version__ = '1.1'


class acetz(tzinfo):
    """An implementation of datetime.tzinfo using the ZoneSpecifier class
    from AceTime/tools.
    """

    def __init__(self, zone_info: ZoneInfo):
        self.zone_info = zone_info
        self.zs = ZoneSpecifier(zone_info, use_python_transition=True)

    def utcoffset(self, dt: Optional[datetime]) -> timedelta:
        assert dt
        self.zs.init_for_year(dt.year)
        offset_info = self.zs.get_timezone_info_for_datetime(dt)
        if not offset_info:
            raise Exception(
                f'Unknown timezone info for '
                f'{dt.year:04}-{dt.month:02}-{dt.day:02} '
                f'{dt.hour:02}:{dt.minute:02}:{dt.second:02}'
            )
        return timedelta(seconds=offset_info.total_offset)

    def dst(self, dt: Optional[datetime]) -> timedelta:
        assert dt
        self.zs.init_for_year(dt.year)
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
        self.zs.init_for_year(dt.year)
        offset_info = self.zs.get_timezone_info_for_datetime(dt)
        if not offset_info:
            raise Exception(
                f'Unknown timezone info for '
                f'{dt.year:04}-{dt.month:02}-{dt.day:02} '
                f'{dt.hour:02}:{dt.minute:02}:{dt.second:02}'
            )
        return offset_info.abbrev

    def zone_specifier(self) -> ZoneSpecifier:
        return self.zs


def gettz(zone_name: str) -> acetz:
    zone_info = cast(ZoneInfo, zone_infos.ZONE_INFO_MAP.get(zone_name))
    if not zone_info:
        raise Exception(f"Zone '{zone_name}' not found")
    return acetz(zone_info)

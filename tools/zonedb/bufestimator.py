# Copyright 2019 Brian T. Park
#
# MIT License

from typing import Dict
from typing_extensions import TypedDict
from .zone_specifier import ZoneSpecifier
from .ingenerator import ZoneInfoMap
from .ingenerator import ZonePolicyMap


# zoneName -> bufSize
BufSizeMap = Dict[str, int]


class BufSizeInfo(TypedDict):
    """Return type of BufSizeEstimator.estimate()."""
    buf_sizes: BufSizeMap
    max_buf_size: int  # maximum of all bufSize


class BufSizeEstimator:
    """Estimate the ExtendedZoneSpecifier::TransitionStorage buffer size for
    each zone.
    """

    def __init__(
        self,
        zone_infos: ZoneInfoMap,
        zone_policies: ZonePolicyMap,
        start_year: int,
        until_year: int,
    ):
        """
        Args:
            zone_infos: dict of ZoneInfos
            zone_policies dict of ZonePolicies
            start_year: start year
            until_year: until year
        """
        self.zone_infos = zone_infos
        self.zone_policies = zone_policies
        self.start_year = start_year
        self.until_year = until_year

    def estimate(self) -> BufSizeInfo:
        """Calculate the (dict) of {full_name -> buf_size} where buf_size is one
        more than the estimate from ZoneSpecifier.get_buffer_sizes(). Return
        the tuple of (buf_sizes, max_size).
        """
        buf_sizes: BufSizeMap = {}
        max_size = 0
        for zone_name, zone_info in self.zone_infos.items():
            zone_specifier = ZoneSpecifier(zone_info)

            # get_buffer_sizes() returns a tuple of
            # ((max_actives, year), (max_buffer_size, year)).
            (max_actives, max_buf_size) = zone_specifier.get_buffer_sizes(
                start_year=self.start_year,
                until_year=self.until_year,
            )

            # The TransitionStorage size should be one more than the estimate
            # because TransitionStorage.getFreeAgent() needs one slot even if
            # it's not used.
            buf_size = max_buf_size[0] + 1

            # The estimate is off for Asia/Atyrau. ZoneSpecifier returns
            # max_buf_size[0]==4 which means 5 should be enough, but
            # TransitionStorage.getHighWater() says that 6 is required. Not sure
            # why.
            if zone_name == 'Asia/Atyrau':
                buf_size += 1

            buf_sizes[zone_name] = buf_size
            if buf_size > max_size:
                max_size = buf_size

        return {
            'buf_sizes': buf_sizes,
            'max_buf_size': max_size,
        }

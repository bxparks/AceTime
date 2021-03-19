# Copyright 2019 Brian T. Park
#
# MIT License

import logging
from collections import OrderedDict
from data_types.at_types import ZonesMap, PoliciesMap
from data_types.at_types import BufSizeInfo, BufSizeMap
from .zone_specifier import ZoneSpecifier
from .inline_zone_info import ZoneInfoMap
from .inline_zone_info import ZonePolicyMap
from .inline_zone_info import InlineZoneInfo


class BufSizeEstimator:
    """Estimate the ExtendedZoneSpecifier::TransitionStorage buffer size for
    each zone.
    """

    def __init__(
        self,
        zones_map: ZonesMap,
        policies_map: PoliciesMap,
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
        self.zones_map = zones_map
        self.policies_map = policies_map
        self.start_year = start_year
        self.until_year = until_year

    def estimate(self) -> BufSizeInfo:
        """Calculate the (dict) of {full_name -> buf_size} where buf_size is one
        more than the estimate from ZoneSpecifier.get_buffer_sizes(). Also
        return the maximum.
        """
        # Generate internal zone_infos and zone_policies to be used by
        # ZoneSpecifier.
        inline_zone_info = InlineZoneInfo(self.zones_map, self.policies_map)
        zone_infos, zone_policies = inline_zone_info.generate_zonedb()
        logging.info(
            'InlinedZoneInfo: Zones %d; Policies %d',
            len(zone_infos), len(zone_policies))

        # Calculate buffer sizes using a ZoneSpecifier.
        buf_sizes = self.calculate_buf_sizes(zone_infos, zone_policies)
        max_buf_size = max(buf_sizes.values())
        logging.info('Found max_buffer_size=%d', max_buf_size)

        # Sort by zone_name
        buf_sizes = OrderedDict(sorted(buf_sizes.items()))

        return {
            'buf_sizes': buf_sizes,
            'max_buf_size': max_buf_size,
        }

    def calculate_buf_sizes(
        self,
        zone_infos: ZoneInfoMap,
        zone_policies: ZonePolicyMap,
    ) -> BufSizeMap:
        buf_sizes: BufSizeMap = {}
        for zone_name, zone_info in zone_infos.items():
            zone_specifier = ZoneSpecifier(zone_info)

            # get_buffer_sizes() returns a BufferSizeInfo(NamedTuple) composed
            # of max_actives(count, year) and max_buffer_size(count, year).
            buffer_size_info = zone_specifier.get_buffer_sizes(
                start_year=self.start_year,
                until_year=self.until_year,
            )

            # The TransitionStorage size should be one more than the estimate
            # because TransitionStorage.getFreeAgent() needs one slot even if
            # it's not used.
            buf_size = buffer_size_info.max_buffer_size.number + 1

            # The estimate is off for Asia/Atyrau. ZoneSpecifier returns
            # max_buf_size[0]==4 which means 5 should be enough, but
            # TransitionStorage.getHighWater() says that 6 is required. Not sure
            # why.
            if zone_name == 'Asia/Atyrau':
                buf_size += 1

            buf_sizes[zone_name] = buf_size

        return buf_sizes

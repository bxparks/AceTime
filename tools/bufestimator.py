# Copyright 2019 Brian T. Park
#
# MIT License

import logging
from zone_specifier import ZoneSpecifier

class BufSizeEstimator:
    """Estimate the Transition buffer size for each zone.
    """

    def __init__(self, zone_infos, zone_policies):
        """
        Args:
            zone_infos (dict): {full_name -> zone_info{} }
            zone_policies (dict): {policy_name ->zone_policy{} }
        """
        self.zone_infos = zone_infos
        self.zone_policies = zone_policies

    def estimate(self):
        """Calculate the (dict) of {full_name -> buf_size} where buf_size is one
        more than the estimate from ZoneSpecifier.get_buffer_sizes().
        """
        buf_sizes = {}
        for zone_short_name, zone_info in self.zone_infos.items():
            zone_full_name = zone_info['name']
            zone_specifier = ZoneSpecifier(zone_info)
            (max_actives, max_buffer_size) = zone_specifier.get_buffer_sizes()
            buf_sizes[zone_full_name] = max_buffer_size[0] + 1
        return buf_sizes

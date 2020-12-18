# Copyright 2020 Brian T. Park
#
# MIT License

from typing import List, Dict
from typing_extensions import TypedDict
from zone_processor.bufestimator import BufSizeInfo, BufSizeMap
from tzdb.data_types import ZonesMap
from tzdb.data_types import RulesMap
from tzdb.data_types import LinksMap
from tzdb.data_types import CommentsMap


class ZoneInfoDatabase(TypedDict):
    """The complete internal representation of the TZ Database files after
    processing them for the AceTime library.
    """

    # Context data.
    tz_version: str
    tz_files: List[str]
    scope: str
    start_year: int
    until_year: int
    until_at_granularity: int
    offset_granularity: int
    strict: bool

    # Data from Extractor filtered through Transformer
    zones_map: ZonesMap
    links_map: LinksMap
    rules_map: RulesMap

    # Data from Transformer
    removed_zones: CommentsMap
    removed_links: CommentsMap
    removed_policies: CommentsMap
    notable_zones: CommentsMap
    notable_links: CommentsMap
    notable_policies: CommentsMap

    # Data from BufSizeEstimator
    buf_sizes: BufSizeMap
    max_buf_size: int

    # ZoneIds
    zone_ids: Dict[str, int]


def create_zone_info_database(
    tz_version: str,
    tz_files: List[str],
    scope: str,
    start_year: int,
    until_year: int,
    until_at_granularity: int,
    offset_granularity: int,
    strict: bool,
    zones_map: ZonesMap,
    links_map: LinksMap,
    rules_map: RulesMap,
    removed_zones: CommentsMap,
    removed_links: CommentsMap,
    removed_policies: CommentsMap,
    notable_zones: CommentsMap,
    notable_links: CommentsMap,
    notable_policies: CommentsMap,
    buf_size_info: BufSizeInfo,
    zone_ids: Dict[str, int],
) -> ZoneInfoDatabase:
    """Return an instance of ZoneInfoDatabase from the various ingrediants."""
    return {
        # Context data.
        'tz_version': tz_version,
        'tz_files': tz_files,
        'scope': scope,
        'start_year': start_year,
        'until_year': until_year,
        'until_at_granularity': until_at_granularity,
        'offset_granularity': offset_granularity,
        'strict': strict,

        # Data from Extractor filtered through Transformer.
        'zones_map': zones_map,
        'links_map': links_map,
        'rules_map': rules_map,

        # Data from Transformer.
        'removed_zones': removed_zones,
        'removed_links': removed_links,
        'removed_policies': removed_policies,
        'notable_zones': notable_zones,
        'notable_links': notable_links,
        'notable_policies': notable_policies,

        # Data from BufSizeEstimator
        'buf_sizes': buf_size_info['buf_sizes'],
        'max_buf_size': buf_size_info['max_buf_size'],

        # ZoneIds
        'zone_ids': zone_ids,
    }

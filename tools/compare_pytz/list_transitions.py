#!/usr/bin/env python3

import argparse
import logging
from datetime import datetime
from datetime import timedelta
import pytz
from typing import Any
from typing import Tuple


def find_transitions(zone_name: str, start_year: int, until_year: int) -> None:
    """Find a DST transition by sampling the time period from [start_year, 
    until_year] in 12-hour samples.
    """
    twelve_hours = timedelta(hours=12)
    tz = pytz.timezone(zone_name)
    dt = datetime(start_year, 1, 1, 0, 0, 0, tzinfo=pytz.utc)
    loc_dt = dt.astimezone(tz)

    # Check every 12 hours for a transition
    while True:
        new_dt = dt + twelve_hours
        new_loc_dt = new_dt.astimezone(tz)
        if new_dt.year > until_year:
            break
        if loc_dt.utcoffset() != new_loc_dt.utcoffset():
            #print(f'Transition between {loc_dt} and {new_loc_dt}')
            start_dt, end_dt = binary_search_transition(tz, dt, new_dt)
            start_loc_dt = start_dt.astimezone(tz)
            end_loc_dt = end_dt.astimezone(tz)
            print(f'{start_loc_dt} -> {end_loc_dt}')

        dt = new_dt
        loc_dt = new_loc_dt

def binary_search_transition(
    tz: Any,
    start_dt: datetime,
    end_dt: datetime,
) -> Tuple[datetime, datetime]:
    """Do a binary search to find the exact transition times, to within 1 minute
    accuracy. The start_dt and end_dt are 12 hours (720 minutes) apart. So this
    binary search should take a maximum of 10 iterations to find the DST
    transition.
    """
    start_loc_dt = start_dt.astimezone(tz)
    end_loc_dt = end_dt.astimezone(tz)
    while True:
        delta_minutes = int((end_dt - start_dt) / timedelta(minutes=1))
        delta_minutes //= 2
        #print(f' delta_minutes={delta_minutes}')
        if delta_minutes == 0: break

        mid_dt = start_dt + timedelta(minutes=delta_minutes)
        mid_loc_dt = mid_dt.astimezone(tz)
        if start_loc_dt.utcoffset() == mid_loc_dt.utcoffset():
            #print(' right half')
            start_dt = mid_dt
            start_loc_dt = mid_loc_dt
        else:
            #print(' left half')
            end_dt = mid_dt
            end_loc_dt = mid_loc_dt

    return start_dt, end_dt


def main(zone_name: str, start_year: int, until_year: int) -> None:
    print(f'zone={zone_name}, start_year={start_year}, until_year={until_year}')
    if until_year == 0:
        until_year = start_year
    find_transitions(zone_name, start_year, until_year)


if __name__ == '__main__':
    # Configure command line flags.
    parser = argparse.ArgumentParser(description='Find the DST transitions')
    parser.add_argument(
        'zone',
        help='Zone name (e.g. America/Los_Angeles',
    )
    parser.add_argument(
        'start_year',
        type=int,
        help='Year to list the transitions',
    )
    parser.add_argument(
        'until_year',
        nargs='?',
        type=int,
        default=0,
        help='Year to list the transitions',
    )
    args = parser.parse_args()

    # Configure logging
    logging.basicConfig(level=logging.INFO)

    main(args.zone, args.start_year, args.until_year)

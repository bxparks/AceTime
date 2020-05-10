# Compare DateUtil

Generate the test data points using the Python `pytz` package.

## Blacklist

With `detect_dst_transition = True`, 6 zones listed in `blacklist.json` show
incorrect DST offsets compared to AceTime and Hinannt libraries.

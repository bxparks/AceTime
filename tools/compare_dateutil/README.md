# Compare DateUtil

Generate the test data points using the Python `dateutil` package.

## Blacklist

If `detect_dst_transition = False`, then dateutil has one zone with problems,
`America/Argentina/Mendoza`. The DST offset is returned as 120 minutes for the
first 3 months of 2000, but all other libraries show a DST offset of 60 minutes,
and a transition at 2000-03-03 00:00 that changed the DST offset, but not the
UTC offset.

If `detect_dst_transition = True`, then 37 additional zones fail the validation
due to incorrect DST offsets. TODO: Add those zones into `blacklist.json`.

# ZoneInfo Data Files

These are internal representations of the zoneinfo  data from the TZ Database.
Library users should treat these data objects as *opaque*. They should be
referenced only by the specific `basic::kZone*` or `extended::kZone*`
identifiers defined in the `src/zonedb` and `src/zonedbx` directories. It is
entirely possible that the internal structure of these data types may change
without warning when the internal implementations change.

There are 6 core data types: `ZoneContext`, `ZoneRule`, `ZonePolicy`, `ZoneEra`
and `ZoneInfo`. Currently, each data type (except `ZoneContext` which is
in the `internal::` namespace) is duplicated into 2 namespaces (`basic::` and
`extended::`) for use with `BasicZoneProcessor` and `ExtendedZoneProcessor`
respectively.

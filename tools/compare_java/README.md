# Compare Java

Generate the test data points using the `java.time` package in Java JDK 11.

## Blacklist

The `blacklist.json` file contains zones whose DST offsets do not match AceTime
library or Hinnant date libary. I believe these are errors in the java.time
library due to the way it handles negative DST offsets.

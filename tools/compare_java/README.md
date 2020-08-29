# Compare Java

Generate the test data points using the `java.time` package in Java JDK 11.

## Requirements

You need to install the Java 11 JDK to get the `javac` compiler.

**Ubuntu 18.04 and 20.04**

```
$ sudo apt install openjdk-11-jdk
```

## Blacklist

The `blacklist.json` file contains zones whose DST offsets do not match AceTime
library or Hinnant date libary. I believe these are errors in the java.time
library due to the way it handles negative DST offsets.

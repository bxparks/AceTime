# CommandLine Clock

A clock that provides access to basic functionality through a command
line interface. The following functions are supported:

```
help [command]
    Print the list of supported commands.
list
    List the AceRoutine coroutines.
date [dateString]
    Print or set the date.
timezone [manual {offset} | dst (on | off)] |
    Print or set the current TimeZone.
basic [list] | extended [list] ]
    Print or set the currently active TimeZone.
sync [status]
    Sync the SystemClock from its external source, or print its sync
    status.
wifi (status | config [{ssid} {password}] | connect)
    Print the ESP8266 or ESP32 wifi connection info.
    Connect to the wifi network.
    Print or set the wifi ssid and password.
```

## Installation

Compatible with [UnixHostDuino](https://github.com/bxparks/UnixHostDuino).
On Ubuntu Linux 18.04, you must install the [zlib](https://www.zlib.net/)
ackage to resolve the dependency to the CRC32 routine:
```
$ sudo apt install zlib1g-dev
```

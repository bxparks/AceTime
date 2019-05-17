# LedClock

This program was split off from the original `example/Clock` directory
which contained code that supported 3 different types of displays:

    * 128x64 OLED display
    * 128x32 OLED display
    * 4 segment LED clock display

It became too difficult to maintain all 3 display types within the same code
base. So I split the code into:
    * `examples/FullOledClock`
    * `examples/LedClock`

This folder has **not** been refactored to support only the LED display.

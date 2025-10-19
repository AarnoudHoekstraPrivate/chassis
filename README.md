# chassis
<<<<<<< HEAD
A library with functions for managing a motor chassis (4 wheels) with lights and commands, arduino mega based.<br>

# Details

The library depends on having a ArduinoMega compatible device available<br>
The library depends on having the SD library available to read an SD card with predefined commands<br>
The BLE is optional and isrequired if one wants to send commands over BT to the Arduino<br>
The wheel counter are catered for but not required unless one needs to travel a distance rather than duration<br>
<br>
Further assumptions:<br>
  - A motor driver is used to drive the chassis motors (forward, backward and speed)<br>
  - 4 LED lights are used<br>
  - Wheel counters are used<br>
<br>
Chassis example:
https://www.pngwing.com/en/free-png-xoejj (off the shelf product) <br>
<br>

# The structure for the SD-CARD

"root" folder contains the CONF.TXT for configuration purposes<br>
"commands" folder contains the GUIDE.TXT for the actual commands<br>
<br> 
 At this moment there are 7 commands available
# chassis

Chassis is an Arduino library to control a 4-wheel motor chassis with optional lights, wheel counters and SD-command support. Target platform: Arduino Mega / ATmega2560.

[![CI](https://github.com/AarnoudHoekstraPrivate/chassis/actions/workflows/ci.yml/badge.svg)](https://github.com/AarnoudHoekstraPrivate/chassis/actions)
[![License](https://img.shields.io/badge/license-AFL--3.0-blue.svg)](LICENSE)

## Quickstart

Requirements

- PlatformIO or Arduino IDE
- Arduino Mega (ATmega2560) or compatible board

Install (PlatformIO)

```bash
platformio lib install "Chassis"
```

Example (SimpleDrive - see `examples/SimpleDrive`)

```cpp
#include <Chassis.h>
Chassis chassis;
void setup() {
  chassis.begin();
}
void loop() {
  chassis.driveForward(150);
  delay(2000);
  chassis.stop();
  delay(2000);
}
```

## SD Card structure

- `CONF.TXT` in root — configuration
- `commands/GUIDE.TXT` — commands to execute

## Development

Run the example or build with PlatformIO using the `platformio.ini` provided in the repository.

## License

This library is released under the AFL-3.0 license. See `LICENSE` for details.

>>>>>>> db774c7 (Initial import: CI, LICENSE, examples, tests, metadata v1.2.0)

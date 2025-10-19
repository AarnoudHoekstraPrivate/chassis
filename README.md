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
  # chassis

  [![CI](https://github.com/AarnoudHoekstraPrivate/chassis/actions/workflows/ci.yml/badge.svg)](https://github.com/AarnoudHoekstraPrivate/chassis/actions)
  [![License](https://img.shields.io/badge/license-AFL--3.0-blue.svg)](LICENSE)

  Chassis is an Arduino library to control a 4-wheel motor chassis with optional lights, wheel counters and SD-command support.

  Key points

  - Target platform: Arduino Mega / ATmega2560
  # chassis

  [![CI](https://github.com/AarnoudHoekstraPrivate/chassis/actions/workflows/ci.yml/badge.svg)](https://github.com/AarnoudHoekstraPrivate/chassis/actions)
  [![License](https://img.shields.io/badge/license-AFL--3.0-blue.svg)](LICENSE)

  Chassis is an Arduino library to control a 4-wheel motor chassis with optional lights, wheel counters and SD-command support.

  Key points

  - Target platform: Arduino Mega / ATmega2560
  - Depends on Arduino `SD` library for SD-command support
  - Optional BLE support for remote commands

  ## Quickstart

  Requirements

  - PlatformIO or Arduino IDE
  - Arduino Mega (ATmega2560) or compatible board

  Install via PlatformIO

  ```bash
  platformio lib install "Chassis"
  ```

  Build locally (PlatformIO)

  ```bash
  # build
  platformio run

  # build and upload using the project platformio.ini
  platformio run -e ATmega2560 -t upload
  ```

  ## Example (SimpleDrive)

  See `examples/SimpleDrive/SimpleDrive.ino` for a minimal example that drives forward for 2s and stops.

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

  ## Development & Testing

  - A minimal PlatformIO `platformio.ini` is included for building locally.
  - Unit tests are under `test/` and use the Unity framework (suitable for non-hardware logic).

  ## Contributing

  If you'd like to contribute:

  - Fork the repo and open a pull request against `main`.
  - Add unit tests for new logic where possible.

  ## License

  This library is released under the AFL-3.0 license. See `LICENSE` for details.

  ## API Reference (selected)

  These are the main public methods available on the `Chassis` class (see `include/Chassis.h` for details):

  - Constructor
    - `Chassis()` — create an instance

  - Configuration
    - `initialiseWheels(int wheelPinSettings[NUM_WHEELS][NUM_WHEEL_PINS])` — set custom wheel pin mapping
    - `initialiseLights(int lightPinSettings[NUM_LIGHT_PINS])` — set light pins
    - `initialiseBLE(int blePinSettings[NUM_BLE_PINS])` — set BLE pins
    - `initialiseFromFile(String fileName)` — read configuration from SD card
    - `setCommandFile(String commandFileName)` — set the SD command file path
    - `dumpSettings()` — print current settings to output

  - Movement
    - `moveWheels(int movements[NUM_WHEELS])` — low-level wheel control
    - `moveForward(int speed)` — move forward at speed (0..255)
    - `moveBackwards(int speed)` — move backwards
    - `doFullStop()` — stop all wheels
    - `doRotate(int angle)` — rotate by degrees
    - `getWheelSpeedStatus()` — returns a string describing current wheel speeds

  - Lights
    - `switchLightsOn(bool lights[NUM_LIGHT_PINS])` — set individual lights
    - `setLightsOverride(bool setting)` — override automatic light handling
    - `setLights(bool setting)` — turn all lights on/off
    - `areLightsEnabled()` / `isLightsOverrideEnabled()` — check status
    - `getLightsStatus()` — status string

  - Misc
    - `setManualMode(bool mode)` / `getManualMode()`
    - `setSerial(bool setting)` / `setWire(bool setting)` — enable serial/I2C output
    - `writeToOutput(String outputText)` — write to configured outputs

  ## Default wiring / pin map

  The library defines default pin mapping values which can be overridden during initialization. Defaults (from `include/Chassis.h`):

  - Wheel pins (each row: ENx, control pin 2, control pin 3)

    - Front-left:  {4, 31, 32}
    - Front-right: {5, 24, 30}
    - Rear-left:   {6, 38, 39}
    - Rear-right:  {7, 27, 28}

  - Lights (4 pins): {42, 43, 44, 45} (lfl, rfl, rll, rrl)
  - BLE pins: {10, 11, 9} (RX, TX, KEY)

  Wiring notes

  - Use an appropriate motor driver for the voltage/current of your motors. The EN and control pins are the logic pins driven by the Arduino; make sure the motor driver has a common ground with the Arduino.
  - If using wheel counters, wire the pulse outputs to interrupt-capable pins and configure them as per the library examples.


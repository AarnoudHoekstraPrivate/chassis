# chassis
A library with functions for managing a motor chassis (4 wheels) with lights and commands, arduino mega based.

# More details to follow

The library depends on having a ArduinoMega compatible device available
The library depends on having the SD library available to read an SD card with predefined commands
The BLE is optional and required if one wants to send commands over BT to the Arduino
Further assumptions:
  - A motor driver is used to drive the chassis motors (forward, backward and speed)

The structure for the SD-CARD:

<root>
  |
  |- CONF.TXT
  |
  |-<commands>
        |
        |- GUIDE.TXT
  
  At this moment there are 7 commands available

# chassis
A library with functions for managing a motor chassis (4 wheels) with lights and commands, arduino mega based.

# More details to follow

The library depends on having a ArduinoMega compatible device available
The library depends on having the SD library available to read an SD card with predefined commands
The BLE is optional and isrequired if one wants to send commands over BT to the Arduino
The wheel counter are catered for but not required unless one needs to travel a distance rather than duration

Further assumptions:
  - A motor driver is used to drive the chassis motors (forward, backward and speed)
  - 4 LED lights are used
  - Wheel counters are used

Chassis example:
https://www.pngwing.com/en/free-png-xoejj (not bought but just an illustration of the base)

The structure for the SD-CARD:

"root" folder contains the CONF.TXT for configuration purposes
"commands" folder contains the GUIDE.TXT for the actual commands
  
 At this moment there are 7 commands available

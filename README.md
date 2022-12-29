# chassis
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

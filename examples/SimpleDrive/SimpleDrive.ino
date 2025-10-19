/*
  SimpleDrive example for Chassis library
  Moves forward for 2 seconds, then stops.
*/

#include <Chassis.h>

Chassis chassis;

void setup() {
  Serial.begin(9600);
  chassis.begin();
}

void loop() {
  Serial.println("Driving forward");
  chassis.driveForward(150); // speed 0-255
  delay(2000);
  chassis.stop();
  delay(2000);
}

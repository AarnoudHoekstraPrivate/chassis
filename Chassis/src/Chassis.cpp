//
//  Chassis.cpp
//  
//
//  Created by Aarnoud Hoekstra on 29/12/2020.
//

#include "Chassis.h"

//
// Constructor with defaults
//
Chassis::Chassis()
{
    lightsEnabled = false;
    lightsOverride = false;
    runCycles = 0;
    configFile = DEFAULT_CONF_FILE;
    commandFile = DEFAULT_COMMAND_FILE;
    cumulativeDistance = 0;
    pinMode(chassisBLE[2], OUTPUT);
}

// chassis definition
//
// wheelPinSettings is a 4 by 3 array
//   rows of the array are the wheels in order flw, frw, rlw, rrw
//   columns of the array are the pin definitions for the wheels being analog (enA/enB), digital (in1/in3), digital (in2/in4)
//
// the definitions below are standard for a arduino mega should be properly set up during initialization phase of the chassis
//
// wheels naming convention:
//  flw = front left wheel   {wheel = 0}
//  frw = front right wheel  {wheel = 1}
//  rlw = rear left wheel    {wheel = 2}
//  rrw = rear right wheel   {wheel = 3}
//
//
// initialize the chassis by setting the right pins to the right pinMode
//
// returns success = true for success
// returns success = false for failure
//
boolean Chassis::initialiseWheels(int wheelPinSettings[NUM_WHEELS][NUM_WHEEL_PINS])
{
  boolean success = false;
 
  if (wheelPinSettings != NULL)
  {
   //
   // set the pinNumbers for the chassis motors
   //
   for (int wheel=0; wheel < NUM_WHEELS; wheel++)
   {
       for (int pinNumber=0; pinNumber < NUM_WHEEL_PINS; pinNumber++)
       {
         chassisWheels[wheel][pinNumber] = wheelPinSettings[wheel][pinNumber];
         pinMode(chassisWheels[wheel][pinNumber], OUTPUT);
       }
       
       wheelSpeedStatus[wheel] = 0;
   }

   success = true;
  }

  return success;
}

//
// initialize the lights of the chassis. Assumption two front lights and two rear ligths
// array is in the order of {lfl, rfl, rll, rrl}
//
// lfl = Left Front Light  associated with lfw
// rfl = Right Front Light associated with rfw
// rll = Rear Left Light   associated with lrw
// rrl = Rear Rigth Light  associated with rrw
//
// returns false for failure
// returns true for success and sets lightsEnable private variable to true
//
boolean Chassis::initialiseLights(int lightPinSettings[NUM_LIGHT_PINS])
{
 boolean success = false;

 lightsEnabled = false;
 lightsOverride = false;
    
 if (lightPinSettings != NULL)
 {
  //
  // set the pin numbers for the lights
  //
     for (int wheel=0; wheel < NUM_WHEELS; wheel++)
     {
       pinMode(lightPinSettings[wheel], OUTPUT);
       chassisLights[wheel] = lightPinSettings[wheel];
       lightStatus[wheel] = false;
     }
     
     lightsEnabled = true;
     success = true;
 }

 return success;
}

//
// initialize the BLE pins RX, TX, Key
//
// returns false for failure
// returns true for success and sets the BLE pins private variable to true
//
boolean Chassis::initialiseBLE(int blePinSettings[NUM_BLE_PINS])
{
 boolean success = true;

 for (int i=0; i < NUM_BLE_PINS; i++)
    chassisBLE[i] = blePinSettings[i];

 pinMode(chassisBLE[2], OUTPUT);
    
 return success;
}

//
// Initialise from file allows for the reading of configuration settings through a config file
// rather than setting each of the individual items in an init loop.
//
// Returns true upon successful parsing of the file
//         false otherwise
//
boolean Chassis::initialiseFromFile(String fileName)
{
    boolean success = true;
    
    if (fileName == NULL)
    {
        Serial.println("Chassis::initialiseFromFile ERROR fileName is NULL");
        
        success = false;
    }
    
    success = success && SD.begin();
    
    if (success)
    {
        File confFile;
            
        confFile = SD.open(fileName);
        if (confFile)
        {
            int numConfItems = 0;
            String lineItem = "";
            String confItems[NUM_CONFIG_ITEMS][2];
                
            lineItem[0] = '\0';
                
            while (confFile.available() && (numConfItems < NUM_CONFIG_ITEMS))
            {
                lineItem = confFile.readStringUntil(';');
                lineItem [lineItem.length() + 1 ] = '\0';
                lineItem.trim();
                lineItem.replace(" ", "");
                
                if (DEBUG) Serial.println(lineItem);
                
                int pos = lineItem.indexOf("=");
                confItems[numConfItems][0] = lineItem.substring(0, pos);
                confItems[numConfItems][1] = lineItem.substring(pos+1, lineItem.length());
                        
                numConfItems++;
            }
                      
            confFile.close();
                
            //
            // run through all of the gathered conf items in the CONF.txt file
            //
            for (int i=0; i < numConfItems; i++)
            {
                //
                // for each found item, check if we can find a corresponding item in official list
                //
                bool found = false;
                int  pos = 0;
                while (!found && (pos < NUM_CONFIG_ITEMS))
                {
                    if (confItems[i][0].equals(configItemList[pos][0]))
                    {
                        configItemList[pos][1] = confItems[i][1];
                            
                        found = true;
                        pos = 0;
                    }

                    pos++;
                }

                if (!found)
                {
                    Serial.print("Chassis::initialiseFromFile ERROR CONFIG ITEM ");
                    Serial.print(confItems[i][0]);
                    Serial.println(" not found in the conf items list");
                    
                    success = success && false;
                }
                else
                {
                    success = success && true;
                }
            }
                
            if (success)
                setConfValues();
        }
        else
        {
            Serial.print("Chassis::initialiseFromFile ERROR cannot open file: ");
            Serial.println(fileName);
            
            success = success && false;
        }
    }
    else
        Serial.println("Chassis::initialiseFromFile ERROR cannot initialise SD card");
        
    return success;
 }

//
// set Manual (blootooth/other) controlled mode or automated (reading the guidance file)
//
void Chassis::setManualMode(boolean mode)
{
    manualMode = mode;
}

//
// return the current setting
//
boolean Chassis::getManualMode()
{
    return manualMode;
}

//
// moveWheels is to move the wheels on the chassis
//
// direction definitions:
// >0 = forward
// 0  = stop/neutral
// <0 = backward
//
// speed definitions:
//  integer between 0 - 255
// input:
//  integer array movements; < 0, 0, > 0; one for each wheel
//
void Chassis::moveWheels(int movements[NUM_WHEELS])
{
  int wheelSpeed = 0;
  int sumOfWheelSpeed = 0;
  boolean lights[NUM_LIGHT_PINS] = {false, false, false, false};

  for (int wheel=0; wheel < NUM_WHEELS; wheel++)
  {
      
    wheelSpeed = abs(movements[wheel]);
      if (wheelSpeed > MAX_WHEEL_SPEED) {wheelSpeed = MAX_WHEEL_SPEED;}     // set maximum wheel speed
      
    analogWrite(chassisWheels[wheel][0], wheelSpeed);
    digitalWrite(chassisWheels[wheel][1], (movements[wheel] > 0) && HIGH);
    digitalWrite(chassisWheels[wheel][2], (movements[wheel] < 0) && HIGH);
    wheelSpeedStatus[wheel] = wheelSpeed;
    
    sumOfWheelSpeed += movements[wheel];
  }
    
  if (!lightsOverride)
  {
     //
     // a bit dodgy but for now we look at the two front wheels
     //
     if ((movements[0] > 0) && (movements[1] > 0)) {lights[0] = true; lights[1] = true;} // moving forward
     if ((movements[0] < 0) && (movements[1] < 0)) {lights[2] = true; lights[3] = true;} // moving back
     if ((movements[0] < 0) && (movements[1] > 0)) {lights[0] = true;}
     if ((movements[0] > 0) && (movements[1] < 0)) {lights[1] = true;}
        
     switchLightsOn(lights);
   }
}

//
// return the speed status
//
String Chassis::getWheelSpeedStatus()
{
    String wheelSpeeds = "(";
    
    for (int i=0; i < NUM_WHEELS; i++)
    {
        wheelSpeeds += String(wheelSpeedStatus[i]);
        if (i < (NUM_WHEELS - 1)) wheelSpeeds += ",";
    }
    wheelSpeeds += ")";
    
    return wheelSpeeds;
}

//
// move the chassis forward
//
void Chassis::moveForward(int speed)
{
  int directions[NUM_WHEELS] = {0, 0, 0, 0};
    
  speed = abs(speed);
  if (speed > 255) {speed = 255;}
    
  for (int i=0; i < NUM_WHEELS; i++)
    directions[i] = speed;

  moveWheels(directions);
}

//
// move the chassis back
//
void Chassis::moveBackwards(int speed)
{
  int directions[NUM_WHEELS] = {0, 0, 0, 0};

  speed = abs(speed);
  if (speed > 255) {speed = 255;}
 
  for (int i=0; i < NUM_WHEELS; i++)
    directions[i] = -speed;
    
  moveWheels(directions);
}

//
// fullStop
//
void Chassis::doFullStop()
{
  int directions[NUM_WHEELS] = {0, 0, 0, 0};
    
  moveWheels(directions);
}

//
// rotate around the cental axis. As we have no angle detection yet, the only thing we can do is left or right rotation
//
void Chassis::doRotate(int angle)
{
    bool rotateLeft = false;
    int rotation = 0;
    
    rotateLeft = (angle < 0);
    rotation = abs(angle);
    
    if (rotation > MAX_ROTATION_ANGLE) {rotation = MAX_ROTATION_ANGLE;}
    
    if (rotateLeft)
    {
        // we need to rotate left ward
        int directions[NUM_WHEELS] = {-255, 255, -255, 255};
        moveWheels(directions);
    }
    else
    {
        // need to rotate right ward
        int directions[NUM_WHEELS] = {255, -255, 255, -255};
        moveWheels(directions);
    }
}

//
// have we enabled the lights?
//
boolean Chassis::areLightsEnabled()
{
    return lightsEnabled;
}

boolean Chassis::isLightsOverrideEnabled()
{
    return lightsOverride;
}

void Chassis::setLightsOverride(boolean setting)
{
    lightsOverride = setting;
}

//
// switch on the lights
//
void Chassis::switchLightsOn(boolean lights[NUM_LIGHT_PINS])
{
    if (lightsEnabled)
        for (int light=0; light < NUM_LIGHT_PINS; light++)
        {
            digitalWrite(chassisLights[light], lights[light] && HIGH);
            lightStatus[light] = lights[light];
        }
}

//
// enable Lights (default = true)
//
void Chassis::setLights(boolean setting)
{
    lightsEnabled = setting;
}

//
// return status of the lights (0000 - 1111 => 0 - 15)
//
String Chassis::getLightsStatus()
{
    String returnVal;
    
    for (int i=0; i < NUM_LIGHT_PINS; i++)
        returnVal += (lightStatus[i]?"1":"0");
    
    return returnVal;
}

//
// set the number of run cycles
//
void Chassis::setRunCycles(int setting)
{
    // ensure it is between 0 and MAX_RUN_CYCLES
    if (setting < 0) {runCycles = 0;}
    if (setting > MAX_RUN_CYCLES) {runCycles = MAX_RUN_CYCLES;}
    runCycles = setting;
}

void Chassis::setCommandFile(String commandFileName)
{
    if (commandFileName == NULL) {commandFileName = DEFAULT_COMMAND_FILE;}
    commandFile = commandFileName;
}

boolean Chassis::setConfValues()
{
  boolean success = true;

  Serial.println("");
    
  for (int i=0; i < NUM_CONFIG_ITEMS; i++)
  {
    if (configItemList[i][0].equals("LIGHTS"))
    {
      if (DEBUG) Serial.print("Chassis::setConfValue Config LIGHTS set to: ");
      if (DEBUG) Serial.println(configItemList[i][1] == "ON");
      lightsEnabled = configItemList[i][1] == "ON";
      success = success && true;
    }

    if (configItemList[i][0].equals("LIGHTS_OVERRIDE"))
    {
      if (DEBUG) Serial.print("Chassis::setConfValue Config LIGHTS_OVERRIDE set to: ");
      if (DEBUG) Serial.println(configItemList[i][1] == "ON");
      lightsOverride = configItemList[i][1] == "ON";
      success = success && true;
    }

    if (configItemList[i][0].equals("CYCLE"))
    {
      if (DEBUG) Serial.print("Chassis::setConfValue Config CYCLE set to: ");
      if (DEBUG) Serial.println(configItemList[i][1].toInt());
      runCycles = configItemList[i][1].toInt();
      success = success && true;
    }

    if (configItemList[i][0].equals("MOVEMENTS"))
    {
      if (DEBUG) Serial.print("Chassis::setConfValue Config MOVEMENTS set to: ");
      if (DEBUG) Serial.println(configItemList[i][1]);
      commandFile = configItemList[i][1];
      success = success && true;
    }

    if (configItemList[i][0].equals("BLE_PINS"))
    {
      if (DEBUG) Serial.print("Chassis::setConfValue Config BLE_PINS set to: ");

      String value = configItemList[i][1];
      if ((value[0] != '{') && (value[value.length()] != '}'))
      {
        Serial.print("Chassis::setConfValue BLE_PINS: ERROR value ");
        Serial.print(value);
        Serial.println(" is not a valid array");
        
        success = success && false;
      }
      else
      {
        value = value.substring(1, value.length()-1);

        // we expect 3 elements in the array
        unsigned int posStart = 0;
        unsigned int posEnd = 0;
        // boolean found = false;
        int valueFound = 0;

        while ((posStart < value.length()) && (valueFound < NUM_BLE_PINS))
        {
          posEnd = value.indexOf(",", posStart);
          if (posEnd < 0) posEnd = value.length();

          chassisBLE[valueFound] = value.substring(posStart,posEnd).toInt();
          
          posStart = posEnd;
          posStart++;

          if (DEBUG) Serial.print(chassisBLE[valueFound]); 
          if (DEBUG) Serial.print(" ");
          
          valueFound++;
        }

        success = success && true;
          
        if (DEBUG) Serial.println("");
      }
    }

    if (configItemList[i][0].equals("LIGHT_PINS"))
    {
      if (DEBUG) Serial.print("Chassis::setConfValue Config LIGHT_PINS set to: ");

      String value = configItemList[i][1];
      if ((value[0] != '{') && (value[value.length()] != '}'))
      {
        Serial.print("Chassis::setConfValue LIGHT_PINS ERROR value ");
        Serial.print(value);
        Serial.println(" is not a valid array");
          
        success = success && false;
      }
      else
      {
        value = value.substring(1, value.length()-1);

        // we expect 4 elements in the array
        unsigned int posStart = 0;
        unsigned int posEnd = 0;
        // boolean found = false;
        int valueFound = 0;

        while ((posStart < value.length()) && (valueFound < NUM_LIGHT_PINS))
        {
          posEnd = value.indexOf(",", posStart);
          if (posEnd < 0) posEnd = value.length();

          chassisLights[valueFound] = value.substring(posStart,posEnd).toInt();
          
          posStart = posEnd;
          posStart++;
            
          if (DEBUG) Serial.print(chassisLights[valueFound]); 
          if (DEBUG) Serial.print(" ");
          
          valueFound++;
        }
          
        if (success) initialiseLights(chassisLights);
          
        success = success && true;
        
        if (DEBUG) Serial.println("");
      }
    }
  }

  return success;
}

//
// dumps all of the settings to Serial. used for debugging purposes
//
void Chassis::dumpSettings()
{
    Serial.println("Dumping current configured settings");
    
    // dumping wheelpin settings
    Serial.print("Dumping wheel pin settings ");
    for (int i=0; i < NUM_WHEELS; i++)
    {
        Serial.print("{");
        for (int j=0; j < NUM_WHEEL_PINS; j++)
        {
            Serial.print(chassisWheels[i][j]);
            if (j != NUM_WHEEL_PINS-1) Serial.print(",");
        }
        Serial.print("} ");
    }
    Serial.println("");
    
    // dumping the light settings
    Serial.println("Dumping light settings");
    Serial.print("   Light pin settings {");
    for (int i=0; i < NUM_LIGHT_PINS; i++)
    {
        Serial.print(chassisLights[i]);
        if (i != NUM_LIGHT_PINS-1) Serial.print(",");
    }
    Serial.println("}");
    Serial.print("   Lights enabled ");
    if (lightsEnabled) Serial.println("YES");
    if (!lightsEnabled) Serial.println("NO");
    Serial.print("   Lights override ");
    if (lightsOverride) Serial.println("YES");
    if (!lightsOverride) Serial.println("NO");
    
    // dumping the BLE settings
    Serial.print("Dumping BLE pin settings {");
    for (int i=0; i < NUM_BLE_PINS; i++)
    {
        Serial.print(chassisBLE[i]);
        if (i != NUM_BLE_PINS-1) Serial.print(",");
    }
    Serial.println("}");
    
    // dumping the generic settings
    Serial.println("Dumping generic settings");
    Serial.print("   ConfigFile ");  Serial.println(configFile);
    Serial.print("   CommandFile "); Serial.println(commandFile);
    Serial.print("   Run cycles ");  Serial.println(runCycles);
}

//
// checks for a valid command
//
// returns true  when found
// returns false when not found
//
boolean Chassis::validateCommand(String cmdString)
{
    boolean found = false;
    int pos = 0;
    
    while ((!found) && (pos < NUM_OF_COMMANDS))
    {
        found = commandsAvailable[pos][0].equals(cmdString);
        pos++;
    }
    
    return found;
}

//
// return the number of runCycles
//
int Chassis::getRunCycles()
{
    return runCycles;
}


//
// PULSE COUNTER SECTION
//
// THEY RESIDE OUTSIDE THE CHASSIS CLASS
//
volatile byte numPulses[NUM_WHEELS];
unsigned int cumulativeDistances[NUM_WHEELS];
int pulseCounters[NUM_WHEELS] = {18, 19, 2, 3};

boolean initialisePulseCounters()
{
    attachInterrupt(digitalPinToInterrupt(pulseCounters[0]), pulseCounterFLW, PULSE_DETECTION);  // FLW sits on INT 2
    attachInterrupt(digitalPinToInterrupt(pulseCounters[1]), pulseCounterFRW, PULSE_DETECTION);  // FRW sits on INT 3
    attachInterrupt(digitalPinToInterrupt(pulseCounters[2]), pulseCounterRLW, PULSE_DETECTION);  // RLW sits on INT 0
    attachInterrupt(digitalPinToInterrupt(pulseCounters[3]), pulseCounterRRW, PULSE_DETECTION);  // RRW sits on INT 1
     
    for (int i=0; i < NUM_WHEELS; i++)
    {
        numPulses[i] = 0;
        cumulativeDistances[i] = 0;
    }
    
    return true;
}

void doPulseCalculation()
{
    //Don't process interrupts during calculations
    
    noInterrupts();
    cli();
    
    //
    // assume at least a 1/2 turn for proper calculation
    //
    if (
        (numPulses[0] >= (PULSES_PER_TURN / 2)) ||
        (numPulses[1] >= (PULSES_PER_TURN / 2)) ||
        (numPulses[2] >= (PULSES_PER_TURN / 2)) ||
        (numPulses[3] >= (PULSES_PER_TURN / 2))
       )
    {
   
        if (DEBUG) Serial.println("START doPulseCalculation");

        for (int i=0; i < NUM_WHEELS; i++)
            detachInterrupt(i);

        if (DEBUG)
        {
             Serial.print("CUMU DIST ....");
             for (int i=0; i < NUM_WHEELS; i++)
             {
               Serial.print(cumulativeDistances[i]);
               Serial.print(" ");
             }
              Serial.println("");
        }
        
        cumulativeDistances[0] += (WHEEL_CIRCUM_FLW * numPulses[0]) / PULSES_PER_TURN;
        cumulativeDistances[1] += (WHEEL_CIRCUM_FRW * numPulses[1]) / PULSES_PER_TURN;
        cumulativeDistances[2] += (WHEEL_CIRCUM_RLW * numPulses[2]) / PULSES_PER_TURN;
        cumulativeDistances[3] += (WHEEL_CIRCUM_RRW * numPulses[3]) / PULSES_PER_TURN;
   
        if (DEBUG)
        {
            Serial.print("NUM PULSES ....");
            for (int i=0; i < NUM_WHEELS; i++)
            {
                Serial.print(numPulses[i]);
                Serial.print(" ");
            }
            Serial.println("");
        }

        for (int i=0; i < NUM_WHEELS; i++)
            numPulses[i] = 0;
        
        if (DEBUG)
        {
            Serial.print("CUMU DIST ....");
            for (int i=0; i < NUM_WHEELS; i++)
            {
                Serial.print(cumulativeDistances[i]);
                Serial.print(" ");
            }
            Serial.println("");
        }
    
        //Restart the interrupt processing
        attachInterrupt(digitalPinToInterrupt(2),  pulseCounterRLW, PULSE_DETECTION);  // RLW sits on INT 0
        attachInterrupt(digitalPinToInterrupt(3),  pulseCounterRRW, PULSE_DETECTION);  // RRW sits on INT 1
        attachInterrupt(digitalPinToInterrupt(21), pulseCounterFLW, PULSE_DETECTION);  // FLW sits on INT 2
        attachInterrupt(digitalPinToInterrupt(20), pulseCounterFRW, PULSE_DETECTION);  // FRW sits on INT 3

        if (DEBUG) Serial.println("END doPulseCalculation");
    }
    
    sei();
    interrupts();
}


//
// pulse counting
//
void pulseCounterFLW()
{
    numPulses[0]++;
}

void pulseCounterFRW()
{
    numPulses[1]++;
}

void pulseCounterRLW()
{
    numPulses[2]++;
}

void pulseCounterRRW()
{
    numPulses[3]++;
}

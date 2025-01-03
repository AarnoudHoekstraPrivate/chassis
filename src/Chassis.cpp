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
    lightsEnabled      = false;
    lightsOverride     = false;
    haveSerial         = false;  // switch off Serial by default
    haveWire           = false;  // switch off Wire by default
    receivingEnd       = 0x08;   // receiving end is default 0x08
    runCycles          = MAX_RUN_CYCLES;
    configFile         = DEFAULT_CONF_FILE;
    commandFile        = DEFAULT_COMMAND_FILE;
    cumulativeDistance = 0;
    pinMode(chassisBLE[2], OUTPUT);
}

// chassis definition
//
// wheelPinSettings is a 4 by 3 array
//   rows of the array are the wheels in order flw, frw, rlw, rrw
//   columns of the array are the pin definitions for the wheels being analog (enA/enB), digital (in2/in4), digital (in1/in3)
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
bool Chassis::initialiseWheels(int wheelPinSettings[NUM_WHEELS][NUM_WHEEL_PINS])
{
  bool success = false;
 
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
bool Chassis::initialiseLights(int lightPinSettings[NUM_LIGHT_PINS])
{
 bool success = false;

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
bool Chassis::initialiseBLE(int blePinSettings[NUM_BLE_PINS])
{
 bool success = true;

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
bool Chassis::initialiseFromFile(String fileName)
{
    bool success = true;
    
    if (fileName == NULL)
    {
        writeToOutput("Chassis::initialiseFromFile ERROR fileName is NULL");
        
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
                    writeToOutput("Chassis::initialiseFromFile ERROR CONFIG ITEM " + String(confItems[i][0]) + " not found in the conf items list");
                    
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
            writeToOutput("Chassis::initialiseFromFile ERROR cannot open file: " + fileName);
            
            success = success && false;
        }
    }
    else
        writeToOutput("Chassis::initialiseFromFile ERROR cannot initialise SD card");
        
    return success;
 }

//
// set Manual (blootooth/other) controlled mode or automated (reading the guidance file)
//
void Chassis::setManualMode(bool mode)
{
    manualMode = mode;
}

//
// return the current setting
bool Chassis::getManualMode()
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
  bool lights[NUM_LIGHT_PINS] = {false, false, false, false};

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
bool Chassis::areLightsEnabled()
{
    return lightsEnabled;
}

bool Chassis::isLightsOverrideEnabled()
{
    return lightsOverride;
}

void Chassis::setLightsOverride(bool setting)
{
    lightsOverride = setting;
}

//
// switch on the lights
//
void Chassis::switchLightsOn(bool lights[NUM_LIGHT_PINS])
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
void Chassis::setLights(bool setting)
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

bool Chassis::setConfValues()
{
  bool success = true;

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
        writeToOutput("Chassis::setConfValue BLE_PINS: ERROR value " + String(value) + " is not a valid array");
        
        success = success && false;
      }
      else
      {
        value = value.substring(1, value.length()-1);

        // we expect 3 elements in the array
        unsigned int posStart = 0;
        unsigned int posEnd = 0;
        // bool found = false;
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
        writeToOutput("Chassis::setConfValue LIGHT_PINS ERROR value " + String(value) + " is not a valid array");
          
        success = success && false;
      }
      else
      {
        value = value.substring(1, value.length()-1);

        // we expect 4 elements in the array
        unsigned int posStart = 0;
        unsigned int posEnd = 0;
        // bool found = false;
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
    String sendText = "";

    writeToOutput("Dumping current configured settings");
    
    // dumping wheelpin settings
    writeToOutput("Dumping wheel pin settings ");
    for (int i=0; i < NUM_WHEELS; i++)
    {
       sendText = "   {";
        for (int j=0; j < NUM_WHEEL_PINS; j++)
        {
            sendText += String(chassisWheels[i][j]);
            if (j != NUM_WHEEL_PINS-1) sendText += ",";
        }
        sendText += "}";
        writeToOutput(sendText);
    }
    
    
    // dumping the light settings
    writeToOutput("Dumping light settings");

    sendText = "   Light pin settings {";
    for (int i=0; i < NUM_LIGHT_PINS; i++)
    {
        sendText += String(chassisLights[i]);
        if (i != NUM_LIGHT_PINS-1) sendText += (",");
    }
    sendText += "}";
    writeToOutput(sendText);

    sendText = "   Lights enabled ";
    if (lightsEnabled) writeToOutput(sendText + "YES");
    if (!lightsEnabled) writeToOutput(sendText + "NO");

    sendText = "   Lights override ";
    if (lightsOverride) writeToOutput(sendText + "YES");
    if (!lightsOverride) writeToOutput(sendText + "NO");
    
    // dumping the BLE settings
    sendText = "Dumping BLE pin settings {";
    for (int i=0; i < NUM_BLE_PINS; i++)
    {
        sendText +=  String(chassisBLE[i]);
        if (i != NUM_BLE_PINS-1) sendText += ",";
    }
    sendText += "}";
    writeToOutput(sendText);
    
    // dumping the generic settings
    writeToOutput("Dumping generic settings");
    writeToOutput("   ConfigFile "  + configFile); 
    writeToOutput("   CommandFile " + commandFile);
    writeToOutput("   Run cycles "  + String(runCycles));
}

//
// checks for a valid command
//
// returns true  when found
// returns false when not found
bool Chassis::validateCommand(String cmdString)
{
    bool found = false;
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
bool initialisePulseCounters()
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

//
// writeToOuput can be used to write to both serial and wire.
// for wire we have start/stop bytes (first 2 bytes) of a frame containing max of 30 bytes of info
//
// A set of frames has two reserved bytes. The first two bytes are:
//
//  start = '06'
//  continuation = '07'
//  stop = '08'
//
// '06' '08' = 1 frame
// '06' '07' followed by '07' '08' = 2 frames
// '07' '07' for more than 2 frames. '06' '07' start frame; '07' '07' (one of more times); '07' '08' stop frame
// 
// Depending on the variable setting in myChassis it will output to Serial and/or Wire
//
const int frameSize = 30;   

void Chassis::writeToOutput(String outputText)
{
  if (haveSerial)
  {
    Serial.println(outputText);
  }

  if (haveWire)
  {
    //
    // Wire can only send 32 bytes max at the time so we need to send in chunks
    //
    int lenText = outputText.length();
    int numChunks = ceil(lenText / 30);

    int i = 0;
    int endPos = 0;

    while (i <= numChunks)
    {
      endPos = (i+1)*frameSize > lenText?lenText:(i+1)*frameSize;

      Wire.beginTransmission(receivingEnd); // assume receivi

      if (i == 0)
      {
        // start frame
        Wire.write(6);
        Wire.write((numChunks > 0)?7:8);
        Wire.print(outputText.substring(i*frameSize, endPos));

        // Serial.print("Writing  6+"); Serial.println((numChunks > 1)?7:8);
      }
      else if (i == numChunks)
      {
        // end frame
        Wire.write((numChunks > 0)?7:6);
        Wire.write(8);
        Wire.print(outputText.substring(i*frameSize, endPos));

        // Serial.print("Writing ");Serial.print((numChunks > 0)?7:6);Serial.println("+8");
      }
      else
      {
        // middle frame
        Wire.write(7);
        Wire.write(7);
        Wire.print(outputText.substring(i*30, endPos));

        // Serial.println("Writing 7+7");
      }

      Wire.endTransmission();
      i++;
    }
  }
}                                              

//
// setting serial
bool Chassis::setSerial(bool setting)
{
    haveSerial = setting;
    
    return true;
}

//
// setting Wire
bool Chassis::setWire(bool setting)
{
    haveWire = setting;

    return true;
}

//
// set receivingEnd
bool Chassis::setReceivingEnd(uint8_t receiver)
{
  receivingEnd = receiver;

  return true;
}
//
//  Chassis.h
//  
//
//  Created by Aarnoud Hoekstra on 29/12/2020.
//

#ifndef Chassis_h
#define Chassis_h

#include <Arduino.h>
#include <SD.h>
#include <Wire.h>

// debug define
#define DEBUG                     false

// Definitions used
#define MAX_RUN_CYCLES            1000
#define MAX_WHEEL_SPEED            255
#define MIN_WHEEL_SPEED           -255
#define DEFAULT_CONF_FILE         "CONF.TXT"
#define DEFAULT_COMMAND_FILE      "COMMANDS/GUIDE.TXT"
#define MAX_ROTATION_ANGLE        360
#define NUM_CONFIG_ITEMS          7
#define NUM_BLE_PINS              3
#define NUM_LIGHT_PINS            4
#define NUM_WHEEL_PINS            3
#define NUM_WHEELS                4
#define START_BLOCK_IDENTIFIER    "<MOVEMENT>"
#define END_BLOCK_IDENTIFIER      "</MOVEMENT>"
#define NUM_OF_COMMANDS           8
#define WHEEL_CIRCUM_FLW          212       // mm's
#define WHEEL_CIRCUM_FRW          212       // mm's
#define WHEEL_CIRCUM_RLW          211       // mm's
#define WHEEL_CIRCUM_RRW          211       // mm's
#define PULSES_PER_TURN           20        // how many pulses for a single turn of a wheel
#define PULSE_DETECTION           RISING    // detect HIGH to LOW

//
// Chassis class defintion
//
class Chassis {
  public:
    Chassis(void);
    
    // Configuration
    bool initialiseWheels(int wheelPinSettings[NUM_WHEELS][NUM_WHEEL_PINS]);
    bool initialiseLights(int lightPinSettings[NUM_LIGHT_PINS]);
    bool initialiseBLE(int blePinSettings[NUM_BLE_PINS]);
    
    // Configuration from file and associated functions
    bool initialiseFromFile(String fileName);
    void setCommandFile(String commandFileName);
    void dumpSettings();
    void setRunCycles(int setting);
    int getRunCycles();

    // movement functions
    void moveWheels(int movements[NUM_WHEELS]);
    void moveBackwards(int speed);
    void moveForward(int speed);
    void doFullStop();
    void doRotate(int angle);
    String getWheelSpeedStatus();

    // light functions
    void switchLightsOn(bool lights[NUM_LIGHT_PINS]);
    void setLightsOverride(bool setting);
    void setLights(bool setting);
    bool areLightsEnabled();
    bool isLightsOverrideEnabled();
    String  getLightsStatus();

    // generic items
 
    void setManualMode(bool mode);
    bool getManualMode();
    
    unsigned int cumulativeDistance;
    
    // outputting through various channels
    bool setSerial(bool setting);
    bool setWire(bool setting);
    void writeToOutput(String outputText);
    bool setReceivingEnd(uint8_t receiver);

private:
    // default settings, can be changed during init phase
    int chassisWheels[NUM_WHEELS][NUM_WHEEL_PINS] = {
                                                        {4, 31, 32},  // lfw  ENA, EN2, EN1
                                                        {5, 24, 30},  // rfw  ENB, EN4, EN3
                                                        {6, 38, 39},  // lrw  ENB, EN4, EN3
                                                        {7, 27, 28}   // rrw  ENA, EN2, EN1
                                                    };
    int chassisLights[NUM_LIGHT_PINS] = {42, 43, 44, 45};  // lfl, rfl, rll, rrl
    int chassisBLE[NUM_BLE_PINS]      = {10, 11, 9};       // RX pin Arduino -> TX on Module, TX pin Arduino -> RX on Module, Key pin in case of BLE module
     
    int runCycles = MAX_RUN_CYCLES;  // number of cycles to run
    String configFile  = DEFAULT_CONF_FILE;
    String commandFile = DEFAULT_COMMAND_FILE;
    
    bool lightsEnabled  = false;
    bool lightsOverride = false;
    bool lightStatus[NUM_LIGHT_PINS] = {false, false, false, false};
 
    int  wheelSpeedStatus[NUM_WHEELS] = {0, 0, 0, 0};
 
    bool manualMode = true;
    
    // configuration items
    String configItemList[NUM_CONFIG_ITEMS][2] = {
                                   {"LIGHTS", ""},
                                   {"LIGHTS_OVERRIDE", ""},
                                   {"LIGHT_PINS", ""},
                                   {"WHEEL_PINS", ""},
                                   {"BLE_PINS", ""},
                                   {"CYCLE", ""},
                                   {"MOVEMENTS", ""}
                                                  };
    
    String commandsAvailable[NUM_OF_COMMANDS][2] = {
                                    {"WHEELS",   "1"},
                                    {"FORWARD",  "0"},
                                    {"BACKWARD", "0"},
                                    {"FULLSTOP", "0"},
                                    {"ROTATE",   "1"},
                                    {"LIGHTS",   "1"},
                                    {"DURATION", "1"},
                                    {"DISTANCE", "1"}
                                                };
    bool setConfValues();
    bool validateCommand(String cmdString);

    // outputStreams
    bool haveSerial   = false;
    bool haveWire     = false;  
    uint8_t receivingEnd = 0x08;
};

//
// interrupts cannot be part of a class :(
//
extern volatile byte numPulses[];
extern unsigned int cumulativeDistances[];
extern int pulseCounters[];

extern bool initialisePulseCounters();
extern void doPulseCalculation();
extern void pulseCounterFLW();
extern void pulseCounterFRW();
extern void pulseCounterRLW();
extern void pulseCounterRRW();

#endif /* Chassis_h */

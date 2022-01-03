#include <Arduino.h>

#include <SoftwareSerial.h>  // for the BLE module

#include <Chassis.h> // my own library for managing the chassis wheels#include <CommandParser.h>

#include <SPI.h>      // SD card
#include <SD.h>

// function declarations
boolean checkCommand(String);
void doReadCommandsFromFile();
void executeCommand(String, String);
void doRunsBasedOnDistance(unsigned int);
boolean specificSerialCommand(String, String);
void doSerialCommandProcessing();


// chassis variables
Chassis myChassis;
const int numCommands = 8;
int numRuns;

// Serial stuff BLE/BT module
const int keyPin = 9;  // in case have a BT/BLE module as serial
const int rxPin = 10;  // RX pin Arduino -> TX on Module
const int txPin = 11;  // TX pin Arduino -> RX on Module

SoftwareSerial mySerial(rxPin, txPin);


String commandsAvailable[8][2] = {
                                    {"WHEELS",   "1"},
                                    {"FORWARD",  "0"},
                                    {"BACKWARD", "0"},
                                    {"FULLSTOP", "0"},
                                    {"ROTATE",   "1"},
                                    {"LIGHTS",   "1"},
                                    {"DURATION", "1"},
                                    {"DISTANCE", "1"}
                                                };

void doReadCommandsFromFile()
{
  File cmdFile;
  int numRuns = myChassis.getRunCycles();
  int runsDone = 0;
  
  // how many times should we run the file and is it in AutoMode?
  while (
         runsDone < numRuns &&
         !myChassis.getManualMode()
        )
  {
    cmdFile = SD.open("commands/GUIDE.TXT");

    //Serial.println(myChassis.getManualMode());
  
   Serial.print("START CYCLE ");
   Serial.println(runsDone);

   while (
           cmdFile.available() &&
           !myChassis.getManualMode()
          )
    {
      String lineItem = cmdFile.readStringUntil('\n');
      boolean foundStartBlock = false;
      boolean foundEndBlock = false;
      int blockElements = 0;
      String block[10];
    

      foundStartBlock = lineItem.equals("<MOVEMENT>");
      foundEndBlock = lineItem.equals("</MOVEMENT>");
 
 
      while (foundStartBlock && !foundEndBlock && cmdFile.available() && blockElements < 10)
      {
        block[blockElements] = cmdFile.readStringUntil('\n');
        block[blockElements].replace(" ", "");
      
        foundEndBlock = block[blockElements].equals("</MOVEMENT>");

        blockElements++;
      } // end while for reading a block of commands
    
      if (foundEndBlock && blockElements < 10)
      {
        // apparently we have found a decent block
        blockElements--;

        // dump tha stuff!!!
        for (int i=0; i < blockElements; i++)
        {
          String cmdItem;
        
          if (block[i].indexOf("=") < 0)
          {
            // it might be a command without arguments
            if (!checkCommand(block[i]))
            {
               Serial.print("Invalid command string ");
               Serial.println(block[i]);
            }

            cmdItem = block[i];
            // Serial.println(cmdItem);

            executeCommand(cmdItem, "");
          }
          else
          {
              cmdItem = block[i].substring(block[i].indexOf("="), 0);
              if (!checkCommand(cmdItem))
              {
                Serial.print("Invalid command string ");
                Serial.println(block[i]);
              }         
              else
              {
                String cmdArguments = block[i].substring(block[i].indexOf("=")+1);
                // Serial.println(cmdItem);
                // Serial.println(cmdArguments);

                executeCommand(cmdItem, cmdArguments);
              }
           }
        } // end for loop processing the block commands
        
      } 
    } // end while loop reading the command file and checking automated mode  

   Serial.println("END CYCLE");

   if (cmdFile)
      cmdFile.close();

    runsDone++;  
  } // end while loop for number of cycles to execute and checking automated mode
}

void executeCommand(String cmdItem, String cmdArgs)
{
  cmdItem.toUpperCase();
  cmdArgs.toUpperCase();
  
  if (cmdItem.equals("WHEELS"))
  {
    Serial.print("Executing WHEELS command: ");
  
    if ((cmdArgs[0] == '(') && (cmdArgs[cmdArgs.length()-1] == ')'))
    {
        String value = cmdArgs.substring(1, cmdArgs.length()-1);
        int movements[4] = {0,0,0,0};

        // Serial.println(value);
        
        // we expect 4 elements in the array
        unsigned int posStart = 0;
        unsigned int posEnd = 0;
        // boolean found = false;
        int valueFound = 0;

        while ((posStart < value.length()) && (valueFound < 4))
        {
          posEnd = value.indexOf(",", posStart);
          if (posEnd < 0) posEnd = value.length();

          movements[valueFound] = value.substring(posStart,posEnd).toInt();
          
          posStart = posEnd;
          posStart++;

          Serial.print(movements[valueFound]); Serial.print(" ");
          
          valueFound++;
        } 

        myChassis.moveWheels(movements);
    }
    Serial.println("");

  }

  if (cmdItem.equals("LIGHTS"))
  {
    Serial.print("Executing LIGHTS command: ");
    
    boolean currentStatus = myChassis.isLightsOverrideEnabled();   // required for rollback

    // override on and set lights
    myChassis.setLightsOverride(true); 
    
    // switch on the lights
    if ((cmdArgs[0] == '(') && (cmdArgs[cmdArgs.length()-1] == ')'))
    {
        String value = cmdArgs.substring(1, cmdArgs.length()-1);
        boolean lights[4] = {0,0,0,0};

        // Serial.print(value);
        
        // we expect 4 elements in the array
        unsigned int posStart = 0;
        unsigned int posEnd = 0;
        // boolean found = false;
        int valueFound = 0;

        while ((posStart < value.length()) && (valueFound < 4))
        {
          posEnd = value.indexOf(",", posStart);
          if (posEnd < 0) posEnd = value.length();

          lights[valueFound] = (value.substring(posStart,posEnd) == "ON");
          
          posStart = posEnd;
          posStart++;

          Serial.print(lights[valueFound]); Serial.print(" ");
          
          valueFound++;
        }

        myChassis.switchLightsOn(lights);
    }
    
    Serial.println("");

    myChassis.setLightsOverride(currentStatus);
  }
  
  if (cmdItem.equals("FORWARD"))
  {
    Serial.print("Executing FORWARD command ");
    Serial.println(cmdArgs.toInt());
    
    myChassis.moveForward(cmdArgs.toInt());
  }

  if (cmdItem.equals("BACKWARD"))
  {
    Serial.print("Executing BACKWARD command ");
    Serial.println(cmdArgs.toInt());
 
    myChassis.moveBackwards(cmdArgs.toInt());
  }

  if (cmdItem.equals("FULLSTOP"))
  {
    Serial.println("Executing FULLSTOP command");

    myChassis.doFullStop();
  }

  if (cmdItem.equals("DURATION")) 
  {
    Serial.print("Executing DURATION command: ");
 
    bool lights[4] = {false,false, false, false};

    Serial.println(cmdArgs.toInt());
 
    delay(cmdArgs.toInt());

    
    myChassis.doFullStop();            // always a fullStop after a delay
    myChassis.switchLightsOn(lights);  // switch off the lights
    // myChassis.setLightsOverride(false);  // only reset the override when it was set during command execution
  }

  if (cmdItem.equals("ROTATE"))
  {
    // do nothing ... not implemented yet
    Serial.print("Executing ROTATE command: ");
    Serial.println(cmdArgs.toInt());
    
    myChassis.doFullStop();
  }

  if (cmdItem.equals("DISTANCE"))
  {
    Serial.print("Executing DISTANCE command: ");
    Serial.println(cmdArgs.toInt());

    unsigned int targetDistance = cmdArgs.toInt() * 10;  // input is in cm -> target in mm
    
    while (
           (cumulativeDistances[0] < targetDistance) &&
           (cumulativeDistances[1] < targetDistance) &&
           (cumulativeDistances[2] < targetDistance) &&
           (cumulativeDistances[3] < targetDistance)
          )
           
    {
      // myChassis.moveWheels(direction);
    }
    
    Serial.print("----------- "); Serial.print(" "); Serial.println(" ---------------");
    
    // myChassis.doFullStop();
    cumulativeDistances[0] = 0;
    cumulativeDistances[1] = 0;
    cumulativeDistances[2] = 0;
    cumulativeDistances[3] = 0;
    
    myChassis.doFullStop();
  }

 // for (int i=0; i < 4; i++)
 //   cumulativeDistances[i] = 0;
}

boolean checkCommand(String cmdItem)
{
  boolean found = false;
  int pos = 0;

  while ((!found) && (pos < numCommands))
  {
    found = commandsAvailable[pos][0].equals(cmdItem);
    pos++;
  }

  return found;
}


unsigned int targetDistance;

int sensorValue;
int outputValue;
int direction[4] = {-190, -190, -190, -190};

void doRunsBasedOnDistance(unsigned int targetDistance)
{
  Serial.println("Executing DISTANCE command: ");  

   while (
           (cumulativeDistances[0] < targetDistance) &&
           (cumulativeDistances[1] < targetDistance) &&
           (cumulativeDistances[2] < targetDistance) &&
           (cumulativeDistances[3] < targetDistance)
          )
    {
      myChassis.moveWheels(direction);
    }
    
     // myChassis.doFullStop();
    cumulativeDistances[0] = 0;
    cumulativeDistances[1] = 0;
    cumulativeDistances[2] = 0;
    cumulativeDistances[3] = 0;
    
    myChassis.doFullStop();delay(5000);//
}

boolean specificSerialCommand(String cmdItem, String cmdArgs)
{
  boolean returnValue = false;

  cmdItem.toUpperCase();
 
  if (cmdItem.equals("LED"))
  {
    Serial.println("JOEPIEDEPOEPIE");
    returnValue = true;
   }

  if (cmdItem.equals("LIGHTSSTATUS"))
  {
    Serial.println("Executing LIGHT STATUS command");
    Serial.println(myChassis.getLightsStatus());
    
    mySerial.println(myChassis.getLightsStatus());
    returnValue = true;
  }
 
  if (cmdItem.equals("SPEEDSTATUS"))
  {
    Serial.println("Executing SPEED STATUS command");
    Serial.println(myChassis.getWheelSpeedStatus());
    
    mySerial.println(myChassis.getWheelSpeedStatus());
    returnValue = true;
  }
 
  if (cmdItem.equals("AUTO"))
  {
    // switch to the guiding file stored on the SD card
    myChassis.setManualMode(false);
  
    Serial.println("Switching to AUTO mode");
  
    returnValue = true;
  }

  if (cmdItem.equals("MANUAL"))
  {
    // return to app / BT controlled mode
    myChassis.setManualMode(true);
  
    Serial.println("Switching to MANUAL mode");
  
    returnValue = true;
  }
 
  return returnValue;
}

String commandBySerial;

void doSerialCommandProcessing()
{ 
  // Feed any data from bluetooth to Terminal.
  // int pos = 0;
  // char readBySerial = '\0';
  int numChars = 0;
 
  numChars = mySerial.available();
  if (numChars > 0)
  {
    commandBySerial = mySerial.readString();
    Serial.println(commandBySerial);

    commandBySerial.replace(" ","");
    commandBySerial.toUpperCase();
           
    String cmdItem      = commandBySerial.substring(commandBySerial.indexOf("="), 0);
    String cmdArguments = commandBySerial.substring(commandBySerial.indexOf("=")+1);

    if (!specificSerialCommand(cmdItem, cmdArguments))
      executeCommand(cmdItem, cmdArguments);
   }

  // Feed all data from termial to bluetooth
  if (Serial.available())
    mySerial.write(Serial.read());
}

void setup()
{
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // chassis inits
  myChassis.initialiseFromFile("CONF.TXT");
  myChassis.dumpSettings();
  myChassis.setLights(true);
  myChassis.setLightsOverride(false);
  myChassis.setManualMode(false);
  
  numRuns = myChassis.getRunCycles();
  
  initialisePulseCounters();

  noInterrupts();
  cli();

    //set timer1 pind 2, 3 and 5 interrupt at 10 Hz (10 interrupts / sec)
    TCCR1A = 0;// set entire TCCR1A register to 0
    TCCR1B = 0;// same for TCCR1B
    TCNT1  = 0;//initialize counter value to 0
    
    // set compare match register for 10 hz increments
    OCR1A = 1562;// = (16*10^6) / (10 * 1024) - 1 (must be <65536)
    
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    
    // Set CS12 and CS10 bits for 1024 prescaler
    TCCR1B |= (1 << CS12) | (1 << CS10);  
  
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);

    //set timer4 pins 6, 7 and 8 interrupt at 10 Hz (10 interrupts / sec)
    //TCCR4A = 0;// set entire TCCR1A register to 0
    //TCCR4B = 0;// same for TCCR1B
    //TCNT4  = 0;//initialize counter value to 0
    
    // set compare match register for 10 hz increments
    //OCR4A = 15625;// = (16*10^6) / (1 * 1024) - 1 (must be <65536)
    
    // turn on CTC mode
    //TCCR4B |= (1 << WGM12);
    
    // Set CS12 and CS10 bits for 1024 prescaler
    //TCCR4B |= (1 << CS12) | (1 << CS10);  
  
    // enable timer compare interrupt
    //TIMSK4 |= (1 << OCIE4A);
    
  interrupts();
  sei();

  // BLE inits
  pinMode(keyPin, OUTPUT);  // this pin will pull the HC-42 pin 34 (key pin) HIGH to switch module to AT mode
  digitalWrite(keyPin, HIGH);

  mySerial.begin(9600);  //Default Baud for comm, it may be different for your Module. 
  while (mySerial.available()) {;}
  
  Serial.println("The bluetooth gates are open.\n Connect to HC-42 from any other bluetooth device");
} 

ISR(TIMER1_COMPA_vect)
{
  //timer1 interrupt 1Hz toggles pin 13
  //generates pulse wave of frequency 1Hz/2 = 0.5kHz (takes two cycles for full wave- toggle high then toggle low)
  doPulseCalculation();
  doSerialCommandProcessing();
}

//ISR(TIMER4_COMPA_vect)
//{
  //timer4 interrupt 1Hz toggles pin 13
  //generates pulse wave of frequency 1Hz/2 = 0.5kHz (takes two cycles for full wave- toggle high then toggle low)
  //doBLECommandProcessing();Serial.println("dhkfjsdhdfkas");
//}

void loop()
{
 //if (myChassisBLE.available())
 //   Serial.write(myChassisBLE.read());

 // if (Serial.available())
 //   myChassisBLE.write(Serial.read());

    
 //doBLECommandProcessing();
 doReadCommandsFromFile();
}
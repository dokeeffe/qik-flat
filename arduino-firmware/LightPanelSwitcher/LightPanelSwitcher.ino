/*
 * Very basic firmware to control a relay switch for a light panel
 */
#include <Firmata.h>

/**
 * Interpret the command string and set the pin state.
 */
void stringCallback(char *myString)
{
  String commandString = String(myString);
  if (commandString.equals("ON")) {
    digitalWrite(13, HIGH);
  } else if (commandString.equals("OFF")) {
    digitalWrite(13, LOW);
  } 
  Firmata.sendString(myString);
}

void setup()
{
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Firmata.setFirmwareVersion(FIRMATA_MAJOR_VERSION, FIRMATA_MINOR_VERSION);
  Firmata.attach(STRING_DATA, stringCallback);
  Firmata.begin(57600);
  
}

void loop()
{
  while (Firmata.available()) {
    Firmata.processInput();
  }
}



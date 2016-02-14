/*
 * Very basic firmware to control a relay switch for a light panel.
 * Relay connected to pin 14
 */
#include <Firmata.h>

const int RELAY_PIN =  12;

/**
 * Interpret the command string and set the pin state.
 */
void stringCallback(char *myString)
{
  String commandString = String(myString);
  if (commandString.equals("ON")) {
    digitalWrite(RELAY_PIN, HIGH);
  } else if (commandString.equals("OFF")) {
    digitalWrite(RELAY_PIN, LOW);
  } 
  Firmata.sendString(myString);
}

void setup()
{
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
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



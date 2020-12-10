/*
  sketch for state machine example
    we implement the reception of a modbus frame
    it is derived from the official Serial Event example, see :
    http://www.arduino.cc/en/Tutorial/SerialEvent


  NOTE: The serialEvent() feature is not available on the Leonardo, Micro, or
  other ATmega32U4 based boards.

  This example code is in the public domain.

*/
#include "Arduino.h"

void serialEvent();


String inputString = "";     // a String to hold incoming data
bool stringComplete = false; // whether the string is complete

// RX_STATE_IDLE
// RX_STATE_RECEIVED

void setup()
{
    // initialize serial:
    Serial.begin(9600);
    // reserve 200 bytes for the inputString:
    inputString.reserve(200);

    Serial.println(String("sketch: ")+ __FILE__ + ", built on " + __DATE__);
}

void loop()
{
    serialEvent();

    if (stringComplete)
    {
        Serial.println(); // faire un saut de ligne entre les réponses
        Serial.print("Le message recu est : ");
        Serial.println(inputString);

        // test de la réponse du joueur
        if (inputString == ":LED2ON\r\n")
        {
            Serial.println("Vous voulez allumer la LED 2");
        }
        else
        {
            Serial.println("Erreur, commande invalide...");
        }

        // clear the string:
        inputString = "";
        stringComplete = false;
    }
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent()
{
    if (Serial.available() > 0)
    {
        // get the new byte:
        char inChar = (char)Serial.read();
        // add it to the inputString:
        inputString += inChar; // ":LED2ON\r\n"
        // if the incoming character is a newline, set a flag so the main loop can
        // do something about it:
        if (inChar == '\n')
        {
            stringComplete = true;
        }
    }
}

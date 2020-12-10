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

//void serialEvent();
void readSerial();
void gereLED();

const int maxBufferModbus = 200;    // max size for modbus buffer
String inputString = "";     // a String to hold incoming data
//bool isStringComplete = false; // whether the string is complete

// we implement a state machine, we declare all possible sate values
enum class StateM { idle, received, rx_waiting_end_lf, emission, emission_start, emission_end };
StateM stateOfMachine = StateM::idle;

// the programme will be split in functions, each correspond to a state value
void stateIdle();
void stateReceived();
void stateRXWaitingEndLF();
void stateEmission();
void stateEmissionStart();
void stateEmissionEnd();


void setup()
{
    // initialize serial:
    Serial.begin(9600);
    // reserve maxBufferModbus bytes for the inputString
    //   if you dont reserve memory, it will start with 10 bytes, when it will need 20,
    //   the string will be moved in memory to find 20 bytes and it will leave a hole of 10 bytes,
    //   then when it will need 50 bytes, it will be moved elsewhere leaving another hole of
    //   20 bytes ...
    inputString.reserve(maxBufferModbus);

    Serial.println(String("sketch: ")+ __FILE__ + ", built on " + __DATE__);
}

void loop()
{
    // le comportement depend de l etat de la machine
    if (stateOfMachine == StateM::idle)   stateIdle();

    else if (stateOfMachine == StateM::received)   stateReceived();

    else if (stateOfMachine == StateM::rx_waiting_end_lf)    stateRXWaitingEndLF();

    else   {
        Serial.println(String("state of machine inconnu : "+
                       String(static_cast<int>(stateOfMachine)) +
                       ", retour a l etat idle"));
        stateOfMachine = StateM::idle;
    }

    // autres actions, en parallele de l etat  stateOfMachine
    gereLED();

}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void readSerial()
{
    if (Serial.available() > 0)
    {
        // get the new byte:
        char inChar = (char)Serial.read();
        // add it to the inputString:
        inputString += inChar; // ":LED2ON\r\n"

        // on traite differement certains caracteres particluiers

        // debut de chaine
        if (inChar == ':')   {
            // clear the string
            inputString = "";
            // repassage a l etat debut de chaine
            stateOfMachine = StateM::received;
        }
        // if the incoming character is a newline, set a flag so the main loop can
        // do something about it:
        if (inChar == '\n')
        {
//            isStringComplete = true;
        }
    }
}

void gereLED()
{
    // pas code
}

void stateIdle()
{
    // gere l arrivee de message sur le port serie
    while (Serial.available() > 0)
    {
        // get the new byte:
        char inChar = (char)Serial.read();

        // debut de chaine
        if (inChar == ':')   {
            // clear the string
            inputString = "";
            // repassage a l etat debut de chaine
            stateOfMachine = StateM::received;
            return;         //   <----   sortie de fonction   ---->
        }
    }

    // rien d autre a faire de prevu pour l instant
}

void stateReceived()
{
    // gere l arrivee de message sur le port serie
    while (Serial.available() > 0)
    {
        // get the new byte:
        char inChar = (char)Serial.read();
        // add it to the inputString:
        inputString += inChar; // ":LED2ON\r\n"

        // si la chaine est trop longue, on annule le message
        if (inputString.length() > maxBufferModbus - 3)   {
            // clear the string
            inputString = "";
            // repassage a l etat debut de chaine
            stateOfMachine = StateM::idle;
            return;     //   <----   sortie de fonction   ---->
        }

        // on traite differement certains caracteres particuliers

        // debut de chaine
        if (inChar == ':')   {
            // clear the string
            inputString = "";
            // repassage a l etat debut de chaine
            stateOfMachine = StateM::received;
            return;     //   <----   sortie de fonction   ---->
        }

        // if the incoming character is a CR, go to state waiting end of frame
        // ie: waiting \n character
        if (inChar == '\r')
        {
            stateOfMachine = StateM::rx_waiting_end_lf;
            return;     //   <----   sortie de fonction   ---->
        }
    }
}

void stateRXWaitingEndLF()
{
    while (Serial.available() > 0)
    {
        // get the new byte:
        char inChar = (char)Serial.read();
        // add it to the inputString:
        inputString += inChar;

        // debut de chaine
        if (inChar == ':')   {
            // clear the string
            inputString = "";
            // repassage a l etat debut de chaine
            stateOfMachine = StateM::received;
            return;     //   <----   sortie de fonction   ---->
        }
        // caractere different de \n,  on ne respecte pas l enchainement \r\n
        //   ce n est donc pas la fin de chaine, on est en milieu de chaine,
        //   on repasse a l etat received
        if (inChar != '\n')   {
            // we dont clear the string
            //inputString = "";
            // repassage a l etat reception de la chaine
            stateOfMachine = StateM::received;
            return;     //   <----   sortie de fonction   ---->
        }

        // ici on considere que la chaine est complete et on la traite

        Serial.println(); // faire un saut de ligne entre les réponses
        Serial.print("Le message recu est : ");
        Serial.println(inputString);

        // test de la réponse du joueur
        if (inputString == "LED2ON\r\n")
        {
            Serial.println("Vous voulez allumer la LED 2");
        }
        else
        {
            Serial.println("Erreur, commande invalide...");
        }

        // clear the string
        inputString = "";
        // repassage a l etat idle
        stateOfMachine = StateM::idle;
    }
}

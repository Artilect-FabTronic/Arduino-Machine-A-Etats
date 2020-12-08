/*
  Serial Event example

  When new serial data arrives, this sketch adds it to a String.
  When a newline is received, the loop prints the string and clears it.

  A good test for this is to try it with a GPS receiver that sends out
  NMEA 0183 sentences.

  NOTE: The serialEvent() feature is not available on the Leonardo, Micro, or
  other ATmega32U4 based boards.

  created 9 May 2011
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/SerialEvent

  Voir Communication > Serial
  https://www.arduino.cc/reference/en/language/functions/communication/serial/

  Reference > Language > Variables > Data types > String
    https://www.arduino.cc/reference/en/language/variables/data-types/string/

  Reference > Language > Variables > Data types > Stringobject
    https://www.arduino.cc/reference/en/language/variables/data-types/stringobject/

  Finite State Machine Programming Basics – Part 1
    https://arduinoplusplus.wordpress.com/2019/07/06/finite-state-machine-programming-basics-part-1/

  State Machine Design in C
    https://www.codeproject.com/Articles/1275479/State-Machine-Design-in-C
  
  FSM : Mode d'emploi
    http://www.bien-programmer.fr/fsm_um.htm

*/

/*****************************************************************************/
// Import des fonctions externes au fichier principal
#include "tempo.h"

/*****************************************************************************/
// Definition of hardware I/O (GPIO) / Définition des E/S matérielles
#define LED_STATUS_PIN 32 // the number of the LED pin (LED_BUILTIN on MKR ZERO = pin 32)
#define LED_RED_PIN 0     // the number of the LED pin

/*****************************************************************************/
// Création de l'objet "temporisationLedStatus" depuis la classe Tempo
Tempo temporisationLedStatus; // tempo pour le clignotement de la LED status
Tempo periodeEchantillonnage; // tempo entre deux valeurs analogiques

String inputString = "";     // a String to hold incoming data
bool stringComplete = false; // whether the string is complete

// RX_STATE_IDLE
// RX_STATE_RECEIVED

/*****************************************************************************/
// Définition pour la reception d'une trame
#define MAX_FRAME_LENGTH 30 // longueur maximal de la trame
/* Variable globales                                                         */
char buffer[8]; // Buffer qui stocke les 8 bits en ascii
byte index = 0; // Itérateur qui permet d'indexer le buffer
char input_data_frame[MAX_FRAME_LENGTH]; // a String to hold incoming data
bool data_frame_complete;                // whether the frame is complete
char command_received;                   // NUL équivaut à '\0' ou encore 0 (https://fr.wikipedia.org/wiki/Null)
uint8_t number_byte_to_read;             // cette valeur est déterminer en fonction du type de commande reçu
                                         // 'A' : commande "ALARM", nombre d'octets à recevoir = 18 (le premier octet de cmd n'est pas compté)
uint8_t byteReceivedInFrame;             // count number of received byte in frame
bool time_update_request;                // demande de mise à jour de l'heure
/*****************************************************************************/

#define DEBUG 1
#if DEBUG
#define FSM_STATE(s)           \
    {                          \
        Serial.print(F("n:")); \
        Serial.print(F(s));    \
    }

#define FSM_ERROR(s)           \
    {                          \
        Serial.print(F("n:")); \
        Serial.print(F(s));    \
    }
#else
#define FSM_STATE(s)
#define FSM_ERROR(s)
#endif

/*****************************************************************************/
// Fonction d'initialisation au démarrage du programme
void setup()
{
    // Initialiser les broches d'entrées/sorties

    // Initialiser la communication série
    Serial.begin(9600);

    // reserve 200 bytes for the inputString:
    inputString.reserve(200);

    Serial.println("Demo MAE Version 0.0.1\n");

    // Initialize and configure all timeout:
    temporisationLedStatus.begin(500); // start tempo for 500 ms
    periodeEchantillonnage.begin(10);  // sampling ADC every 10 ms

    // Clear "input_data_frame" Tab:
    memset(input_data_frame, 0, MAX_FRAME_LENGTH);
}

void loop()
{
    serialEvent();
    blink();

    // Main APP FSM, Step 1: check I/O update
    // aucunes mise à jours necessaire pour le moment
    // read the input on analog pin 0:
    int sensorValue = analogRead(A0);

    // Main APP FSM, Step 2: tempo processing
    // traitement des tempos
    if (temporisationLedStatus.isTimeEnding())
    {
        GPIO_ToggleOutput(LED_RED_PIN); // blink the LED
    }

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
  Nettoyage du buffer de reception
  (RAZ du compteur)
*/
void serialEvent()
{
    while (Serial.available() > 0)
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

/*****************************************************************************/
// Fonction permettant le changement d'état d'une sortie
void GPIO_ToggleOutput(int output_pin)
{
    digitalWrite(output_pin, !digitalRead(output_pin)); // change the output pin state
}

void blink(bool reset = false)
{
    const uint32_t LED_DELAY = 1000;
    static enum { LED_TOGGLE,
                  WAIT_DELAY } state = LED_TOGGLE;
    static uint32_t timeLastTransition = 0;

    if (reset)
    {
        state = LED_TOGGLE;
        digitalWrite(LED_BUILTIN, LOW);
    }

    switch (state)
    {
    case LED_TOGGLE: // toggle the LED
        FSM_STATE("LED_TOGGLE");
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        timeLastTransition = millis();
        state = WAIT_DELAY;
        break;

    case WAIT_DELAY: // wait for the delay period
        if (millis() - timeLastTransition >= LED_DELAY)
            state = LED_TOGGLE;
        break;

    default:
        state = LED_TOGGLE;
        break
    }
}

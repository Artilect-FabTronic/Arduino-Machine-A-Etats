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

  Finite _fms_rx_status Machine Programming Basics – Part 1
    https://arduinoplusplus.wordpress.com/2019/07/06/finite-_fms_rx_status-machine-programming-basics-part-1/

  _fms_rx_status Machine Design in C
    https://www.codeproject.com/Articles/1275479/_fms_rx_status-Machine-Design-in-C
  
  FSM : Mode d'emploi
    http://www.bien-programmer.fr/fsm_um.htm

*/

/*****************************************************************************/
// Import des fonctions externes au fichier principal
#include <Arduino.h>
#include "tempo.h"

/*****************************************************************************/
// Definition of hardware I/O (GPIO) / Définition des E/S matérielles
// Target Preprocessor Macro List -- needs to be found easily! (AVR, SAM, STM32...)
// https://forum.arduino.cc/index.php?topic=454617.0
// Uniquement pour la version Arduino CLI en ligne de commande <https://arduino.github.io/arduino-cli/latest/platform-specification/>
// #if defined(ARDUINO_ARCH_AVR)
// #define LED_STATUS_PIN 13 // the number of the LED pin (LED_BUILTIN on Uno = pin 13)
// #elif defined(ARDUINO_ARCH_SAMD)
// #define LED_STATUS_PIN 32 // the number of the LED pin (LED_BUILTIN on MKR ZERO = pin 32)
// #else
// // generic, non-platform specific code
// #endif

// http://electronics4dogs.blogspot.com/2011/01/arduino-predefined-constants.html
// https://gist.github.com/ah01/762576
#if defined(__AVR_ATmega328P__)
#define LED_STATUS_PIN 13 // the number of the LED pin (LED_BUILTIN on Uno = pin 13)
#elif defined(__AVR_ATmega32U4__)
#define LED_STATUS_PIN 32 // the number of the LED pin (LED_BUILTIN on MKR ZERO = pin 32)
#else
// generic, non-platform specific code
#define LED_STATUS_PIN LED_BUILTIN // the number of the LED pin for unknown board type
#endif

#define LED_RED_PIN 0 // the number of the LED pin

/*****************************************************************************/
// Création de l'objet "temporisationLedStatus" depuis la classe Tempo
Tempo temporisationLedStatus; // tempo pour le clignotement de la LED status
Tempo periodeEchantillonnage; // tempo entre deux valeurs analogiques

//String inputString = "";     // a String to hold incoming data
bool stringComplete = false; // whether the string is complete

// RX__fms_rx_status_IDLE
// RX__fms_rx_status_RECEIVED

/*****************************************************************************/
// Définition pour la reception d'une trame
#define MAX_FRAME_LENGTH 30 // longueur maximal de la trame
/* Variable globales                                                         */
char message_recu[MAX_FRAME_LENGTH]; // Buffer qui stocke les caractères ASCII reçu
uint8_t compteur_msg_perdu = 0;      // permet de savoir si l'on a louper la lecture d'un nouveau message

//byte index = 0;                      // Itérateur qui permet d'indexer le buffer
//char input_data_frame[MAX_FRAME_LENGTH]; // a String to hold incoming data
bool data_frame_complete;    // whether the frame is complete
char command_received;       // NUL équivaut à '\0' ou encore 0 (https://fr.wikipedia.org/wiki/Null)
uint8_t number_byte_to_read; // cette valeur est déterminer en fonction du type de commande reçu
                             // 'A' : commande "ALARM", nombre d'octets à recevoir = 18 (le premier octet de cmd n'est pas compté)
uint8_t byteReceivedInFrame; // count number of received byte in frame
bool time_update_request;    // demande de mise à jour de l'heure
/*****************************************************************************/

#define DEBUG 1
#if DEBUG
#define FSM_STATE(s)           \
    {                          \
        Serial.print(F("n:")); \
        Serial.println(F(s));  \
    }

#define FSM_ERROR(s)           \
    {                          \
        Serial.print(F("n:")); \
        Serial.println(F(s));  \
    }
#else
#define FSM__fms_rx_status(s)
#define FSM_ERROR(s)
#endif

/*****************************************************************************/
//void blink();
//void blink(bool reset = false);
void blink(bool reset);

/*****************************************************************************/
// Fonction d'initialisation au démarrage du programme
void setup()
{
    // Initialiser les broches d'entrées/sorties
    pinMode(LED_STATUS_PIN, OUTPUT);
    blink(0);

    // Initialiser la communication série
    Serial.begin(9600);

    // reserve 200 bytes for the inputString:
    //inputString.reserve(200);

    Serial.println("Demo MAE/FSM Version 0.0.1\n");

    // Initialize and configure all timeout:
    // temporisationLedStatus.begin(500); // start tempo for 500 ms
    // periodeEchantillonnage.begin(10);  // sampling ADC every 10 ms

    // Clear "input_data_frame" Tab:
    //memset(input_data_frame, 0, MAX_FRAME_LENGTH);
}

void loop()
{
    serialEvent(); // facultatif avec carte UNO ; n'est pas facultatif avec d'autres cartes
    blink(1);

    // Main APP FSM, Step 1: check I/O update
    // aucunes mise à jours necessaire pour le moment
    // read the input on analog pin 0:
    //int sensorValue = analogRead(A0);

    // Main APP FSM, Step 2: tempo processing
    // traitement des tempos
    if (temporisationLedStatus.isTimeEnding())
    {
        GPIO_ToggleOutput(LED_RED_PIN); // blink the LED
    }

    if (compteur_msg_perdu > 0)
    {
        Serial.println(); // faire un saut de ligne entre les réponses
        Serial.print("Erreur message, le nbr de msg perdu est : ");
        Serial.println(compteur_msg_perdu);
        compteur_msg_perdu = 0; // RAZ du nombre de message perdu
    }

    if (stringComplete)
    {
        Serial.println(); // faire un saut de ligne entre les réponses
        Serial.print("Le message recu est : ");
        Serial.println(message_recu);

        // test de la réponse du joueur, "equal" si strcmp retourne zero
        if (strcmp(message_recu, "LED2ON") == 0)
        {
            Serial.println("Vous voulez allumer la LED 2");
        }
        else
        {
            Serial.println("Erreur, commande invalide...");
        }

        // clear the string:
        //inputString = "";
        message_recu[0] = '\0'; //
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
typedef enum
{
    RX_STATE_IDLE,
    RX_STATE_CLEAR_BUFFER,
    RX_STATE_RECEIVED,
    RX_STATE_WAIT_EOF
} fsm_rx_state_typedef;

void serialEvent()
{
    static char _input_data_frame[MAX_FRAME_LENGTH]; // une chaîne pour contenir toutes les données entrantes
    static uint8_t _index_rx_data = 0;               // itérateur qui permet d'indexer le tableau "_input_data_frame"

    static fsm_rx_state_typedef _fsm_rx_status = RX_STATE_IDLE;

    while (Serial.available() > 0)
    {
        // get the new byte:
        char inChar = (char)Serial.read();

        switch (_fsm_rx_status)
        {
        case RX_STATE_IDLE:
            if (inChar == ':')
            {
                //_fsm_rx_status = RX_STATE_CLEAR_BUFFER;
                _index_rx_data = 0; // RAZ de l'index d'écriture dans le tableau
                //_input_data_frame[0] = 0; // juste pour le premier élément
                memset(_input_data_frame, 0, MAX_FRAME_LENGTH); // mettre 0 dans toutes les cellules du tableau "input_data_frame"
                _fsm_rx_status = RX_STATE_RECEIVED;
            }
            break;

            // case RX_STATE_CLEAR_BUFFER:
            //     _index_rx_data = 0; // RAZ de l'index d'écriture dans le tableau
            //     //_input_data_frame[0] = 0; // juste pour le premier élément
            //     memset(_input_data_frame, 0, MAX_FRAME_LENGTH); // mettre 0 dans toutes les cellules du tableau "input_data_frame"
            //     _fsm_rx_status = RX_STATE_RECEIVED;
            //     break;

        case RX_STATE_RECEIVED:
            if (inChar == '\r')
            {
                _fsm_rx_status = RX_STATE_WAIT_EOF;
            }
            else if (inChar == ':')
            {
                //_fsm_rx_status = RX_STATE_CLEAR_BUFFER;
                _index_rx_data = 0;                             // RAZ de l'index d'écriture dans le tableau
                memset(_input_data_frame, 0, MAX_FRAME_LENGTH); // mettre 0 dans toutes les cellules du tableau "input_data_frame"
                _fsm_rx_status = RX_STATE_RECEIVED;
            }
            else if (inChar == '\n')
            {
               _fsm_rx_status = RX_STATE_IDLE;
            }
            else
            {
                // ajoutez chaque nouveau caractère au buffer local dans tableau "_input_data_frame"
                _input_data_frame[_index_rx_data] = inChar; // ":LED2ON\r\n"
                _index_rx_data++;                           // on incrémente l'index du tableau
            }
            break;

        case RX_STATE_WAIT_EOF:
            // if the incoming character is a newline, set a flag so the main loop can
            // do something about it:
            if (inChar == '\n')
            {
                // Si le "message_recu" a été lu alors le premier élément doit être le caractère null
                if (message_recu[0] == '\0')
                {
                    // Copie du buffer en reception dans le tableau "message_recu"
                    uint8_t i;
                    for (i = 0; i < _index_rx_data; i++)
                    {
                        message_recu[i] = _input_data_frame[i]; // copier un par un chaque donnée reçue
                    }
                    message_recu[i] = '\0'; // ajouter le caractère null en fin de chaine
                    stringComplete = true;
                }
                else
                {
                    compteur_msg_perdu++;
                }

                _fsm_rx_status = RX_STATE_IDLE; // on ré-initialise pour le prochain message
            }
            else if (inChar == ':')
            {
                _fsm_rx_status = RX_STATE_CLEAR_BUFFER;
            }
            else
            {
                _fsm_rx_status = RX_STATE_IDLE;
            }
            break;
        }
    }
}

/*****************************************************************************/
// Fonction permettant le changement d'état d'une sortie
void GPIO_ToggleOutput(int output_pin)
{
    digitalWrite(output_pin, !digitalRead(output_pin)); // change the output pin _fms_rx_status
}

void blink(bool reset = false)
{
    const uint32_t LED_DELAY = 1000;
    static enum { LED_TOGGLE,
                  WAIT_DELAY } _fms_rx_status = LED_TOGGLE;
    static uint32_t timeLastTransition = 0;

    if (reset)
    {
        _fms_rx_status = LED_TOGGLE;
        digitalWrite(LED_BUILTIN, LOW);
    }

    switch (_fms_rx_status)
    {
    case LED_TOGGLE: // toggle the LED
        //FSM_STATE("LED_TOGGLE");
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        timeLastTransition = millis();
        _fms_rx_status = WAIT_DELAY;
        break;

    case WAIT_DELAY: // wait for the delay period
        if (millis() - timeLastTransition >= LED_DELAY)
            _fms_rx_status = LED_TOGGLE;
        break;

    default:
        _fms_rx_status = LED_TOGGLE;
    }
}

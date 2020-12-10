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

  Automate fini
    https://fr.wikipedia.org/wiki/Automate_fini

  Finite _fms_rx_status Machine Programming Basics – Part 1
    https://arduinoplusplus.wordpress.com/2019/07/06/finite-_fms_rx_status-machine-programming-basics-part-1/

  State Machine Design in C
    https://www.codeproject.com/Articles/1275479/_fms_rx_status-Machine-Design-in-C
  
  Automate fini ou Machine à états
    http://tvaira.free.fr/dev/qt/qt-statemachine.pdf

  FSM : Mode d'emploi
    http://www.bien-programmer.fr/fsm_um.htm

*/

/*****************************************************************************/
// Import des fonctions externes au fichier principal
//#include <Arduino.h>
#include "tempo.h"

/*****************************************************************************/
// Définition des E/S matérielles - Definition of hardware I/O (GPIO)
#define LED_STATUS_PIN LED_BUILTIN // the number of the LED pin for unknown board type
#define LED_RX_DATA_PIN 8          // the number of the LED pin for unknown board type

/*****************************************************************************/
// Fonction permettant le changement d'état d'une sortie
#define GPIO_ToggleOutput(output_pin)                                                                  \
    {                                                                                                  \
        digitalWrite(output_pin, !digitalRead(output_pin)); /* change the output pin _fms_rx_status */ \
    }

#define LED_RED_PIN 0 // the number of the LED pin

/*****************************************************************************/
// Création de l'objet "temporisationLedStatus" depuis la classe Tempo
Tempo temporisationLedStatus; // tempo pour le clignotement de la LED status

/*****************************************************************************/
// Définition pour la reception d'une trame
#define MAX_FRAME_LENGTH 30 // longueur maximal de la trame
/* Variable globales                                                         */
char message_recu[MAX_FRAME_LENGTH]; // Buffer qui stocke les caractères ASCII reçu
uint8_t compteur_msg_perdu = 0;      // permet de savoir si l'on a loupé la lecture d'un ou plusieurs nouveaux messages
bool nouveau_message_a_lire = false; // indique qu'un nouveau message est disponible

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
// Fonction d'initialisation au démarrage du programme
void setup()
{
    // Initialiser les broches d'entrées/sorties
    pinMode(LED_STATUS_PIN, OUTPUT);
    pinMode(LED_RX_DATA_PIN, OUTPUT);

    // Initialiser la communication série
    Serial.begin(9600);

    // reserve 200 bytes for the inputString:
    //inputString.reserve(200);

    Serial.println("Demo MAE/FSM Version 0.0.1\n");

    // Initialize and configure all timeout:
    temporisationLedStatus.begin(500); // start tempo for 500 ms
}

/*****************************************************************************/
// Fonction principale du programme
void loop()
{
    serialEvent(); // facultatif avec carte UNO ; n'est pas facultatif avec d'autres cartes

    // Main APP FSM, Step 1: check I/O update
    // aucunes mise à jours necessaire pour le moment

    // Main APP FSM, Step 2: tempo processing
    // traitement des tempos
    if (temporisationLedStatus.isTimeEnding())
    {
        GPIO_ToggleOutput(LED_STATUS_PIN); // blink the LED
    }

    if (compteur_msg_perdu > 0)
    {
        Serial.println(); // faire un saut de ligne entre les réponses
        Serial.print("Erreur message, le nbr de msg perdu est : ");
        Serial.println(compteur_msg_perdu);
        compteur_msg_perdu = 0; // RAZ du nombre de message perdu
    }

    if (nouveau_message_a_lire == true)
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

        message_recu[0] = '\0'; // effacer la chaîne reçu
        nouveau_message_a_lire = false;
    }
}

/*
  SerialEvent est exécutée à chaque fois qu'une nouvelle donnée arrive sur la broche RX du port série.
*/
typedef enum
{
    RX_STATE_IDLE,
    RX_STATE_RECEIVED,
    RX_STATE_WAIT_EOF
} fsm_rx_state_typedef;

void serialEvent()
{
    static char _buffer_des_donnees_recues[MAX_FRAME_LENGTH]; // une chaîne pour contenir toutes les données entrantes
    static uint8_t _index_du_buffer = 0;                      // itérateur qui permet d'indexer le tableau "_buffer_des_donnees_recues"

    static fsm_rx_state_typedef _mae_rx_etat_en_cours = RX_STATE_IDLE;

    while (Serial.available() > 0)
    {
        char la_donnee_recu = (char)Serial.read(); // lecture d'un nouveau octet reçu
        GPIO_ToggleOutput(LED_RX_DATA_PIN);

        switch (_mae_rx_etat_en_cours)
        {
        case RX_STATE_IDLE:
            if (la_donnee_recu == ':')
            {
                _index_du_buffer = 0;              // RAZ de l'index d'écriture dans le tableau
                _buffer_des_donnees_recues[0] = 0; // juste pour le premier élément
                //memset(_buffer_des_donnees_recues, 0, MAX_FRAME_LENGTH); // mettre 0 dans toutes les cellules du tableau "input_data_frame"
                _mae_rx_etat_en_cours = RX_STATE_RECEIVED;
            }
            break;

        case RX_STATE_RECEIVED:
            if (la_donnee_recu == '\r')
            {
                _mae_rx_etat_en_cours = RX_STATE_WAIT_EOF;
            }
            else if (la_donnee_recu == ':')
            {
                _index_du_buffer = 0;              // RAZ de l'index d'écriture dans le tableau
                _buffer_des_donnees_recues[0] = 0; // juste pour le premier élément
                //memset(_buffer_des_donnees_recues, 0, MAX_FRAME_LENGTH); // mettre 0 dans toutes les cellules du tableau "input_data_frame"
            }
            else if (la_donnee_recu == '\n')
            {
                _mae_rx_etat_en_cours = RX_STATE_IDLE;
            }
            else
            {
                // ajoutez chaque nouveau caractère au buffer local dans tableau "_buffer_des_donnees_recues"
                _buffer_des_donnees_recues[_index_du_buffer] = la_donnee_recu; // ajout de la donnee reçu dans le buffer
                _index_du_buffer++;                                            // on incrémente l'index du tableau
            }
            break;

        case RX_STATE_WAIT_EOF:
            // Si le caractère entrant est 'LF' (caractère nouvelle ligne),
            // mettre à vrai le drapeau "nouveau_message_a_lire" pour que la boucle principale
            // puisse lire le message fraîchement reçu
            if (la_donnee_recu == '\n')
            {
                // Si le "message_recu" précédament a été lu, alors "nouveau_message_a_lire" dois être égale à false
                if (nouveau_message_a_lire == false)
                {
                    // Copie du buffer de message en reception dans le tableau "message_recu"
                    uint8_t i;
                    for (i = 0; i < _index_du_buffer; i++)
                    {
                        message_recu[i] = _buffer_des_donnees_recues[i]; // copier un par un chaque donnée reçue
                    }
                    message_recu[i] = '\0';        // ajouter le caractère null en fin de chaine
                    nouveau_message_a_lire = true; // signaler qu'un nouveau message est arrivée
                }
                else
                {
                    compteur_msg_perdu++;
                }

                _mae_rx_etat_en_cours = RX_STATE_IDLE; // on ré-initialise pour le prochain message à recevoir
            }
            else if (la_donnee_recu == ':')
            {
                _index_du_buffer = 0;              // RAZ de l'index d'écriture dans le tableau
                _buffer_des_donnees_recues[0] = 0; // juste pour le premier élément
                //memset(_buffer_des_donnees_recues, 0, MAX_FRAME_LENGTH); // mettre 0 dans toutes les cellules du tableau "input_data_frame"
                _mae_rx_etat_en_cours = RX_STATE_RECEIVED;
            }
            else
            {
                _mae_rx_etat_en_cours = RX_STATE_IDLE;
            }
            break;
        }
    }
}

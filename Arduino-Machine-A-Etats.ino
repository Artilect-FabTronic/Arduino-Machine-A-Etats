/*
  Démo de réception de données gérées par une machine à états (événement série "Serial Event")
  
  Lorsque de nouvelles données série arrivent, la machine à états les ajoute à une chaîne de réception,
  on parle alors d'un buffer de réception (voir le tableau g_message_recu[])
  
  Lorsqu'un message est reçu en totalité, c'est à dire que le message commande par le caractère ':' et
  qu'il se termine par les caractères de fin de chaîne '\r' et '\n' (CR, LF), alors la machine à états
  averti le programme principale (la fonction loop()) en mettant la variable g_nouveau_message_a_lire à la valeur vrai (1)
  le message est alors renvoyé sur le port série (on parle alors d'un echo) et si une commande est reconnue alors
  le programme réalise l'action demandée.

  REMARQUE: la fonction serialEvent() n'est pas disponible sur les cartes
            Leonardo, Micro ou autres cartes ATmega32U4 ou version MKR
            il convient donc de l'appeler dans la fonction loop() afin
            de garantir la compatibilité du code sur différentes cartes.

  Ce programme est basé la version d'origine de la démo SerialEvent.ino,
  qui a été créé par Tom Igoe le 9 mai 2011
  
  Par convention :
  
    - les mots définis avec #define sont écrit en majuscule
    - toutes les variables sont écrites en minuscule en utilisant du snake case (exemple : ma_variable)
      https://fr.wikipedia.org/wiki/Snake_case
    - les variables globales commence par "g_", le 'g' signifiant que c'est une variable globale
    - les variables locales déclarées en static commence par '_',
      elles ont la particularité de conserver leur valeur dans la fonction entre deux appels.
  
  Cet exemple de code est dans le domaine public.

  Voici une liste de lien vers des références permettant d'approfondir ce sujet :
  
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
// Semantic Versioning Specification (SemVer)
// http://semver.org/

// VERSION = MAJOR.MINOR.PATCH
//   - MAJOR: version when you make incompatible API changes,
//   - MINOR: version when you add functionality in a backwards-compatible manner, and
//   - PATCH: version when you make backwards-compatible bug fixes.
#define MAJOR "0"
#define MINOR "2"
#define PATCH "0"
#define VERSION MAJOR + "." + MINOR + "." + PATCH
//#define VERSION "0.1.0"

/*****************************************************************************/
// Import des fonctions externes au fichier principal
//#include <Arduino.h>
#include "serialEvent.h"
#include "tempo.h"

/*****************************************************************************/
// Définition des E/S matérielles - Definition of hardware I/O (GPIO)
#define LED_PIN LED_BUILTIN // le numéro de la broche utilisé pour le pilotage de la LED
#define LED_RX_DATA_PIN 8   // the number of the LED pin for unknown board type

/*****************************************************************************/
// Fonction permettant le changement d'état d'une sortie
#define GPIO_ToggleOutput(output_pin)                       \
    {                                                       \
        digitalWrite(output_pin, !digitalRead(output_pin)); \
    }

/*****************************************************************************/
// Création de l'objet "temporisationLedStatus" depuis la classe Tempo
Tempo temporisationLed; // tempo pour le clignotement de la LED

/* Variable globales                                                         */

/*****************************************************************************/
// Fonction d'initialisation au démarrage du programme
void setup()
{
    // Initialiser les broches d'entrées/sorties
    pinMode(LED_PIN, OUTPUT);
    pinMode(LED_RX_DATA_PIN, OUTPUT);

    // Initialiser la communication série
    Serial.begin(9600);

    Serial.println(); // faire un saut de ligne entre les réponses
    Serial.println("==========================");
    Serial.println(String("Demo MAE/FSM Version ") + VERSION);
    Serial.println("==========================\n");
    Serial.println(String("sketch: ") + __FILE__ + ", built on " + __DATE__);

    // Initialize and configure all timeout:
    temporisationLed.begin(500); // start tempo for 500 ms
}

/*****************************************************************************/
// Fonction principale du programme
void loop()
{
    // Step 1: Vérifier l'évolution du système
    // aucune mise à jour des entrées/sorties ou des variables n'est nécessaire pour le moment...

    // Step 2: Traitement des tempos
    // aucune tempo à mettre à jour

    // Step 3: Traitement des tâches et des actions à réaliser par le programme principale

    // Step 3-1: Appel de la routine de réception de données depuis le port série
    // facultatif avec carte UNO, mais indispensable avec d'autres cartes
    // donc mettre cette fonction dans la loop() n'est pas un problème, bien au contraire
    serialEvent();

    // Step 3-2: Gestion des erreurs lors de la réception de données depuis le port série
    if (g_compteur_msg_perdu != 0)
    {
        Serial.println(); // faire un saut de ligne entre les réponses
        Serial.print("Erreur message, le nbr de msg perdu est : ");
        Serial.println(g_compteur_msg_perdu); // afficher le nombre de message perdu
        g_compteur_msg_perdu = 0;             // RAZ du nombre de message perdu
    }

    // Step 3-3: Gestion de l'arrivée d'un nouveau message depuis le port série
    if (g_nouveau_message_a_lire == true)
    {
        // Afficher le nouveau message
        Serial.print("Message recu : ");
        Serial.println(g_message_recu); // faire un echo du message reçu

        // Comparaison avec les différentes commandes
        // lorsque la fonction strcmp retourne zéro, c'est qu'il y a égalité entre les deux chaînes comparées
        if ((strcmp(g_message_recu, "AIDE") == 0) || (strcmp(g_message_recu, "aide") == 0))
        {
            // Si l'on reçoit "AIDE" ou "aide" alors on affiche l'aide
            Serial.println("Voici la liste des commandes pour changer l'etat de la LED :");
            Serial.println("  - \":LED ON<CR><LF>\"");
            Serial.println("  - \":LED OFF<CR><LF>\"");
            Serial.println("  - \":LED TOGGLE<CR><LF>\"");
        }
        else if (strcmp(g_message_recu, "LED ON") == 0)
        {
            // Si l'on reçoit "LED ON" alors on allume la LED
            Serial.println("Allumage de la LED.");
            digitalWrite(LED_PIN, HIGH);
        }
        else if (strcmp(g_message_recu, "LED OFF") == 0)
        {
            // Si l'on reçoit "LED OFF" alors on éteint la LED
            Serial.println("Extinction de la LED.");
            digitalWrite(LED_PIN, LOW);
        }
        else if (strcmp(g_message_recu, "LED TOGGLE") == 0)
        {
            // Si l'on reçoit "LED TOGGLE" alors on change l'état de la LED
            Serial.println("Inverser l'etat de la LED.");
            GPIO_ToggleOutput(LED_PIN);
        }
        else
        {
            // Si aucune commande ne correspond alors on l'indique à l'utilisateur
            Serial.println("Commande invalide!");
            Serial.println("Afficher la liste des commandes en envoyant \":AIDE<CR><LF>\".");
        }

        // RAZ du drapeau indiquant l'arrivée d'un nouveau message
        g_nouveau_message_a_lire = false; // la remise à zéro permet de recevoir un nouveau message
    }
}

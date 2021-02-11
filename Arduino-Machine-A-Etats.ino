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

*/

#define VERSION "0.0.1"

// Définition des E/S matérielles - Definition of hardware I/O (GPIO)
#define LED_PIN LED_BUILTIN // le numéro de la broche utilisé pour le pilotage de la LED

// Variable globales
String inputString = "";     // a String to hold incoming data
bool stringComplete = false; // whether the string is complete

// Fonction permettant le changement d'état d'une sortie
void GPIO_ToggleOutput(uint8_t output_pin)
{
    digitalWrite(output_pin, !digitalRead(output_pin));
}

void setup()
{
    // Initialiser les broches d'entrées/sorties
    pinMode(LED_PIN, OUTPUT);

    // initialize serial:
    Serial.begin(9600);

    // reserve 200 bytes for the inputString:
    inputString.reserve(200);

    Serial.println(String("Demo MAE/FSM Version ") + VERSION);
    Serial.println(String("sketch: ") + __FILE__ + ", built on " + __DATE__);
}

void loop()
{
    // Appel de la routine de réception des données du port série
    serialEvent();

    // Gestion de l'arrivée d'un nouveau message depuis le port série
    if (stringComplete)
    {
        Serial.println(); // faire un saut de ligne entre les réponses
        Serial.print("Le message recu est : ");
        Serial.println(inputString);

        // Comparaison avec les différentes commandes
        // lorsque la fonction strcmp retourne zéro, c'est qu'il y a égalité entre les deux chaînes comparées
        if ((inputString == ":AIDE\r\n") || (inputString == ":aide\r\n"))
        {
            // Si l'on reçoit "AIDE" ou "aide" alors on affiche l'aide
            Serial.println("Voici la liste des commandes pour changer l'etat de la LED :");
            Serial.println("  - \":LED ON<CR><LF>\"");
            Serial.println("  - \":LED OFF<CR><LF>\"");
            Serial.println("  - \":LED TOGGLE<CR><LF>\"");
        }
        else if (inputString == ":LED ON\r\n")
        {
            // Si l'on reçoit "LED ON" alors on allume la LED
            Serial.println("Allumage de la LED.");
            digitalWrite(LED_PIN, HIGH);
        }
        else if (inputString == ":LED OFF\r\n")
        {
            // Si l'on reçoit "LED OFF" alors on éteint la LED
            Serial.println("Extinction de la LED.");
            digitalWrite(LED_PIN, LOW);
        }
        else if (inputString == ":LED TOGGLE\r\n")
        {
            // Si l'on reçoit "LED TOGGLE" alors on change l'état de la LED
            Serial.println("Inverser l'etat de la LED.");
            GPIO_ToggleOutput(LED_PIN);
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
        inputString += inChar; // ":LED ON\r\n"

        // if the incoming character is a newline, set a flag so the main loop can
        // do something about it:
        if (inChar == '\n')
        {
            stringComplete = true;
        }
    }
}

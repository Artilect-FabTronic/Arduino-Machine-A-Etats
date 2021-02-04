/*
  SerialEvent est exécutée à chaque fois qu'une nouvelle donnée arrive sur la broche RX du port série.
  
  Les différents états pour la réception d'un message série sont :
  
  - RX_STATE_IDLE       : Attente du caractère ':' déclenchant l'acquisition d'un nouveau message. 
  - RX_STATE_RECEIVED   : Accumulation des caractères du message jusqu'à la détection du caractère '\r'.
                          Durant cet état, si un caractère '\n' ou un caractère non ASCII est reçu,
                          alors on retourne à l'état RX_STATE_IDLE.
                          Si c'est le caractère ':' qui est à nouveau reçu, alors on efface les données
                          précédemment reçues et on relance RX_STATE_RECEIVED.
  - RX_STATE_WAIT_EOF   : Validation de la réception du message avec le caractère '\n'.
                          (EOF signifie "End Of Frame", soit "fin du message")
*/

/* Variable globales                                                         */
char g_message_recu[MAX_FRAME_LENGTH]; // buffer qui stocke les caractères ASCII reçu
uint8_t g_compteur_msg_perdu = 0;      // permet de savoir si l'on a loupé la lecture d'un ou plusieurs messages
bool g_nouveau_message_a_lire = false; // si vrai, indique qu'un nouveau message est disponible

typedef enum
{
    RX_STATE_IDLE,
    RX_STATE_RECEIVED,
    RX_STATE_WAIT_EOF
} fsm_rx_state_typedef;

void serialEvent()
{
    static uint8_t _index_du_buffer = 0;                               // itérateur qui permet d'indexer le buffer des données reçues
    static fsm_rx_state_typedef _mae_rx_etat_en_cours = RX_STATE_IDLE; // stock l'état courant de la réception d'un message

    while (Serial.available() > 0)
    {
        //digitalWrite(LED_RX_DATA_PIN, HIGH);
        char _la_donnee_recu = (char)Serial.read(); // lecture d'un nouveau octet reçu

        switch (_mae_rx_etat_en_cours)
        {
        case RX_STATE_IDLE:
            if (_la_donnee_recu == ':')
            {
                // Si le message précédent a été lu,
                // alors "g_nouveau_message_a_lire" doit être mis à false par le programme principale
                // dans ce cas on accepte la lecture du nouveau message entrant
                if (g_nouveau_message_a_lire == false)
                {
                    _index_du_buffer = 0; // RAZ de l'index d'écriture dans le tableau
                    _mae_rx_etat_en_cours = RX_STATE_RECEIVED;
                }
                else
                {
                    // Erreur : message précédent non lu !
                    // En cas de message non lu, on incrémente le compteur des messages perdus
                    // et on attend le prochain message...
                    g_compteur_msg_perdu++;
                }
            }
            break;

        case RX_STATE_RECEIVED:
            if (_la_donnee_recu == '\r')
            {
                // Si l'on reçoit le caractère CR ('\r' ou encore 0x0D)
                // c'est la fin du message, ne reste plus que le dernier caractère de fin de chaîne '\n' à recevoir.
                _mae_rx_etat_en_cours = RX_STATE_WAIT_EOF;
            }
            else if (_la_donnee_recu == '\n')
            {
                // Erreur : ce n'est pas le bon caractère de fin de chaîne !
                // Si la séquence de fin de chaîne n'est pas correcte,
                // on abandonne le message en cours de réception,
                // on incrémente le compteur des messages perdus,
                // et on attend le prochain message.
                g_compteur_msg_perdu++;
                _mae_rx_etat_en_cours = RX_STATE_IDLE;
            }
            else if (_la_donnee_recu == ':')
            {
                // Erreur : réception du caractère de début de chaîne !
                // Si on reçoit le caractère de début de chaîne,
                // on abandonne le message en cours de réception
                // et on lit le nouveau message entrant.
                g_compteur_msg_perdu++;
                _index_du_buffer = 0; // RAZ de l'index d'écriture dans le tableau
            }
            else
            {
                // Si nous recevons juste un nouveau caractère,
                // ajoutez chaque caractère dans le buffer de réception.
                g_message_recu[_index_du_buffer] = _la_donnee_recu; // ajout de la donnée reçu dans le buffer
                _index_du_buffer++;                                 // on incrémente l'index du tableau

                if (_index_du_buffer > MAX_FRAME_LENGTH - 1)
                {
                    // Erreur : si la valeur de l'index indique que nous allons dépasser la taille du buffer de réception
                    // avant d'avoir reçu le caractère de fin du message, alors on abandonne le message en cours de réception
                    // et on attend le prochain message.
                    g_compteur_msg_perdu++;
                    _mae_rx_etat_en_cours = RX_STATE_IDLE;
                }
            }
            break;

        case RX_STATE_WAIT_EOF:
            // Si le caractère entrant est 'LF' (caractère nouvelle ligne = 0x0A),
            // mettre à vrai le drapeau "g_nouveau_message_a_lire" pour que la boucle principale
            // puisse lire le nouveau message reçu
            if (_la_donnee_recu == '\n')
            {
                g_message_recu[_index_du_buffer] = '\0'; // ajouter le caractère null en fin de chaîne
                g_nouveau_message_a_lire = true;         // signaler qu'un nouveau message est arrivée
                _mae_rx_etat_en_cours = RX_STATE_IDLE;   // on ré-initialise pour le prochain message à recevoir
            }
            else if (_la_donnee_recu == ':')
            {
                // Erreur : réception du caractère de début de chaîne !
                // Si on reçoit le caractère de début de chaîne,
                // on abandonne le message en cours de réception
                // et on lit le nouveau message entrant.
                g_compteur_msg_perdu++;
                _index_du_buffer = 0; // RAZ de l'index d'écriture dans le tableau
                _mae_rx_etat_en_cours = RX_STATE_RECEIVED;
            }
            else
            {
                // Erreur : reception d'un caractère différent de celui attendu pour valider la réception du message !
                // Si la séquence de fin de chaîne n'est pas correcte,
                // on abandonne le message en cours de reception
                // et on attend le prochain message.
                g_compteur_msg_perdu++;
                _mae_rx_etat_en_cours = RX_STATE_IDLE;
            }
            break;

        default:
            // En cas de dernier recours si _mae_rx_etat_en_cours a perdu les pédales
            // on ré-initialise la machine à états
            _mae_rx_etat_en_cours = RX_STATE_IDLE;
        }

        //digitalWrite(LED_RX_DATA_PIN, LOW);
    }
}

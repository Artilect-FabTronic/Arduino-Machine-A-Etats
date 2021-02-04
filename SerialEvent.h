
/*****************************************************************************/
// Définition pour la réception d'une trame, d'un message
// Il est nécessaire d'avoir une place supplémentaire dans le tableau de réception afin de pouvoir ajouter le caractère de fin de chaîne '\0'
#define MAX_FRAME_LENGTH 11 // le nombre de caractères maximum est MAX_FRAME_LENGTH-1, donc ici nous pouvons recevoir jusqu'à 10 caractères

/* Variable globales */
extern char g_message_recu[]; // buffer qui stocke les caractères ASCII reçu
extern uint8_t g_compteur_msg_perdu;      // permet de savoir si l'on a loupé la lecture d'un ou plusieurs messages
extern bool g_nouveau_message_a_lire; // si vrai, indique qu'un nouveau message est disponible

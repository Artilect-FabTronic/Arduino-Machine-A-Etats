# Arduino-Machine-A-Etats

Ce projet collaboratif a pour but de comprendre le fonctionnement et l'implementation des machines à états pour structurer un programme embarqué.

Nous allons pour cela mettre en oeuvre une partie du [protocol Modbus](https://www.modbustools.com/modbus.html) qui peut être composé soit de données brut "RTU", soit de données textes "ASCII".

[Format de message Modbus ASCII](https://www.virtual-serial-port.org/fr/articles/modbus-ascii-guide/)

Le format des messages Modbus ASCII comprend un caractère de début qui est un deux-points ':' et la fin du message est définie par la combinaison d'un retour à la ligne et d'un saut de ligne (CR LF). Ce type de protocole permet de faire varier le temps de transmission de chaque donnée du message, comme lorsque celui-ci est entrée à la main dans un terminal.

Voici une description complète d'un message Modbus ASCII :

| Start | Address | Function | Data    |   LRC   |  End  |
| :---: | :-----: | :------: | :------ | :-----: | :---: |
|  ':'  | 2 Chars | 2 Chars  | N Chars | 2 Chars | CR LF |

Dans un premier temps, nous allons implémenter une version simple de ce protocol, sans ajouter à la trame l'adresse d'un destinataire, ni la valeur numérique permettant de vérifier l'intégrité des données de la trame. La gestion "Error Checking" du protocole (vérification d'erreur) ne sera donc pas implémentée.

Le but est simplement de pouvoir capturer une trame au format [ASCII](https://en.wikipedia.org/wiki/ASCII) de manière autonome, en créant une machine à états.

| Start | Data    |  End  |
| :---: | :------ | :---: |
|  ':'  | N Chars | CR LF |

Pour la réception de données à travers le port COM principale de la carte Arduino, nous utiliserons la fonction serialEvent() et nous prendrons soin de ne pas écraser un message par un nouveau avant d'avoir pu le lire.

Pour cela vous pouvez vous inspirez du diagramme d'états ci-dessous.

---

## Algorithme d'émission/réception d'un message Modbus ASCII

![modbus-ascii-fsm-message](Images/modbus-ascii-fsm-message.png)

Source : <http://www.ozeki.hu/p_5855-ozeki-modbus-ascii.html>

---

## Test de reception d'une trame

Exemple de messages que doit recevoir le microcontrôleur pour changer l'état de notre LED :

```serial
":LED ON\r\n"
":LED OFF\r\n"
":LED TOGGLE\r\n"
```

Mais si vous essayez d'écrire une commande en minuscule, ça ne fonctionne pas :

":led toggle\r\n"

La raison est que ce qui est transmis du PC au microcontrôleur ne sont que des nombres :

| Numéro TXD  |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9   |  10   |  11   |  12   |  13   |
| :---------: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
|  Type char  |  ':'  |  'L'  |  'E'  |  'D'  |  ' '  |  'T'  |  'O'  |  'G'  |  'G'  |  'L'  |  'E'  | '\r'  | '\n'  |
| Valeur Hexa | 0x3A  | 0x4C  | 0x45  | 0x44  | 0x20  | 0x54  | 0x4F  | 0x47  | 0x47  | 0x4C  | 0x45  | 0x0D  | 0x0A  |

| Numéro TXD  |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9   |  10   |  11   |  12   |  13   |
| :---------: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
|  Type char  |  ':'  |  'l'  |  'e'  |  'd'  |  ' '  |  't'  |  'o'  |  'g'  |  'g'  |  'l'  |  'e'  | '\r'  | '\n'  |
| Valeur Hexa | 0x3A  | 0x6C  | 0x65  | 0x64  | 0x20  | 0x74  | 0x6F  | 0x67  | 0x67  | 0x6C  | 0x65  | 0x0D  | 0x0A  |

On peut remarquer que les caractères en minuscule ont une valeur de codage supérieure de + 0x20 par rapport aux même lettres en majuscule.
Ce codage est défini par [le code ASCII](https://www.commentcamarche.net/contents/93-code-ascii), voir aussi la [table ASCII](https://fr.wikibooks.org/wiki/Les_ASCII_de_0_%C3%A0_127/La_table_ASCII)

---

### Codage réalisé

A partir de l'exemple [Serial Event](https://www.arduino.cc/en/Tutorial/BuiltInExamples/SerialEvent) d'Arduino, nous avons commencé à modifier la manière de recevoir les données pour créer une machine à états.

#### Code de la branche **main**

Utilisation direct de l'exemple [Serial Event](https://www.arduino.cc/en/Tutorial/BuiltInExamples/SerialEvent) d'Arduino.

#### Code de la branche **arnaud** de CrazyFraug

Découpage en différentes fonctions, une par action à réaliser.

Le traitement du message est separe de la lecture buffer

La machine à états principale pour la reception est dans la fonction :

`void readSerialWithStateMachine(String& amsgModbus)`

#### Code de la branche **Arnauld** de ArnauldDev

Décomposé en plusieurs branches, celle-ci met en oeuvre l'implementation d'une machine à états pour être identique à la branche **main**.

La branche **Arnauld-Trame-Avec-CRC16** permet de rajouter le calcul et l'integration du CRC16 pour la reception d'un message.

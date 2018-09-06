# Etude technique du Minitel

Le but de cette projet est de présenter les résultats de l'étude technique du Minitel que j'ai réalisé il y a quelques années.
Mais pourquoi faire le reverse engineering du Minitel, objet complètement démodé, aux capacités apparemment très limitées ?
En fait cela vient d'un pari technique : Remplacer le logiciel interne du Minitel pour en démontrer les réelles capacités graphiques.
Beaucoup d'objet informatique des années 80/90 ont leurs démos, mais sur Minitel cela n'a jamais été fait auparavant.
La grande diffusion de ce terminal en France mérite que l'on s'intéresse un peu à lui ;-)

Le sujet d'étude est un Minitel 2, appareil à l'origine destiné à la déchetterie.

![Minitel 2](https://raw.githubusercontent.com/jfdelnero/minitel/master/doc/img/minitel2.jpg)

![Minitel 2 Model Number](https://raw.githubusercontent.com/jfdelnero/minitel/master/doc/img/minitel2_modelnumber.jpg)

En retirant le capot arrière du minitel voici ce qu'il y a en interne:

![Minitel 2 Internal](https://raw.githubusercontent.com/jfdelnero/minitel/master/doc/img/minitel2_interne.jpg)

Il y a principalement 2 cartes : La carte latérale est la carte du moniteur CRT du minitel. Cette dernière contient également la partie alimentation de l'ensemble.
La carte en position horizontale est la carte que j'appellerai carte CPU du minitel qui est en fait le coeur du minitel.

![Minitel 2 Motherboard](https://raw.githubusercontent.com/jfdelnero/minitel/master/doc/img/minitel2_motherboard.jpg)

Ci-dessus la carte mère du Minitel 2. On peut y repérer les composants principaux:

### L'unitée centrale

![Minitel 2 CPU](https://raw.githubusercontent.com/jfdelnero/minitel/master/doc/img/minitel2_cpu.jpg)

L'unité centrale du Minitel 2: Un microcontroleur *80C32*. Il s'agit en fait de la version ROMless du microcontroleur 80C51. Ce microcontroleur est cadencé par la partie vidéo à *14,31818 Mhz*. En terme de puissance de calcul sachant qu'il faut au minimum 12 cycles d'horloges pour une instruction, cela tourne autour de 1 MIPS.

Ce microcontroleur contient 256 octets de RAM en interne.

Le petit circuit en dessous est une EEPROM I2C de 256 octets (24C02) contenant l'annuaire de l'utilisateur ainsi que le mot de passe de protection du minitel.

![Minitel 2 EPROM](https://raw.githubusercontent.com/jfdelnero/minitel/master/doc/img/minitel2_eprom.jpg)

L'EPROM contenant le logiciel du minitel. Ce chip est sur support pour une eventuelle mise à jour. Il s'agit d'une 87C257, une EPROM de 256Kbits (32Ko). La différence majeure avec une classique 27C256 est qu'elle est capable de se connecter directement au bus multiplexé du 80C32 sans latch externe (le latch LS373 est intégré).

### La partie Video

![Minitel 2 Video](https://raw.githubusercontent.com/jfdelnero/minitel/master/doc/img/minitel2_video.jpg)

La partie vidéo du minitel, peut être la partie la plus intéréssante. Le *TS9347* est un composant vidéo capable d'afficher du texte Alpha-Numérique.
Le composant en question gêre directement *8KB de DRAM* (théoriquement extensible jusqu'à 32Ko).
Il est possible de définir ses propres tables de caractères, d'ou le nom "Semi-Graphic Display Processor".

Le microcontroleur n'a pas d'accès direct à cette mémoire mais peut envoyer des commandes de lectures/écriture via le processeur graphique.

L'horloge issue de l'oscillateur du processeur graphique cadencé à *14,31818 Mhz* est également utilisée par le microcontroleur 80C32.

### Le Modem

![Minitel 2 Modem](https://raw.githubusercontent.com/jfdelnero/minitel/master/doc/img/minitel2_modem.jpg)

La partie modem, composée d'un *TS7514CP*, un modem V.23 (modulation / démodulation en FSK a 1200/75 baud ou 75/1200 baud) .
Ce dernier est également connecté a un haut parleur interne au minitel et est capable de générer les fréquences DTMF 697Hz, 770Hz,852Hz, 941Hz, 1209Hz, 1336Hz, 1477Hz, 1633Hz ainsi que les fréquences de modulation 390Hz, 450Hz, 1300Hz et 2100Hz.

Il ne reste plus qu'a composer quelque chose avec ça ;).

## Architecture du Minitel 2

Ci-dessous un draft du schema du minitel 2. A noter qu'il y a uniquement les interconnections "numériques". La partie video et modem est à ce jour manquante.

![Minitel 2 Schema draft](https://raw.githubusercontent.com/jfdelnero/minitel/master/doc/img/UC_Decodage_Clavier_small.jpg)

![Minitel 2 Design](https://raw.githubusercontent.com/jfdelnero/minitel/master/doc/img/Architecture_Minitel_2.gif)

## Le clavier

Le clavier est une classique matrice de contacts. Voici l'interconnection interne :

![Minitel 2 Keyboard connection](https://raw.githubusercontent.com/jfdelnero/minitel/master/doc/img/minitel2_clavier.jpg)

Et voici la matrice du clavier :

![Minitel 2 Keyboard Matrix](https://raw.githubusercontent.com/jfdelnero/minitel/master/doc/img/matrice_clavier_minitel.gif)

### IO & Mem Mapping :

*0x2000 : Registre de contrôle modem / CRT / Relais :*

#define HW_CTRL_MCBC 0x01
#define HW_CTRL_MODDTMF 0x02
#define HW_CTRL_CTRON 0x08
#define HW_CTRL_COILON 0x20

*0x4000 :* Chip video TS9347.

*Connexion modem:*

#define RXD_MODEM P3_3 // Modem -> CPU
#define RTS_MODEM P1_4 // CPU -> Modem
#define TXD_MODEM P1_3 // CPU -> Modem
#define PRD_MODEM P1_2 // CPU -> Modem
#define DCD_MODEM P1_1 // Modem -> CPU
#define ZCO_MODEM P3_2 // Modem -> CPU

### Bon et après ? Que peut on faire avec cela ?

Une fois que les composants sont identifiés, et l'architecture générale du minitel est bien comprise, il est maintenant possible de le programmer.
Le microprocesseur étant basé sur un coeur de 80C51, j'ai simplement utilisé SDCC :

http://sdcc.sourceforge.net/

A l'aide de ce compilateur j'ai pu réaliser différents test de différentes routines d'affichage (2D et 3D) afin de determiner les capacités de la machine. En voici quelques exemples en images :

https://www.youtube.com/watch?v=a2HD6OzNoEo

https://www.youtube.com/watch?v=ba_51zGY1cQ

Pas mal pour un minitel ;)

Au passage voici le dump de l'EPROM d'origine contenant le firmware de cette machine : MINITEL2_BV4.BIN

Bientôt plus de détails !

### 30 Avril 2017 :

J'ai ajouté le support de l'émulation hardware du Minitel 2 dans Mame :

http://hxc2001.free.fr/minitel/mame_minitel.zip

L'archive contient Mame ainsi que la ROM d'origine et celle de la démo avec différents effets graphiques 2D et 3D.
Les sources du driver Minitel 2 sont sur le dépot officiel de Mame :

https://github.com/mamedev/mame/blob/master/src/mame/drivers/minitel_2_rpic.cpp

![Mame Minitel 2](https://raw.githubusercontent.com/jfdelnero/minitel/master/doc/img/mame_minitel.png)

Donc maintenant les ROMs alternatives peuvent être écrite et testé sans forcement avoir le matériel.

A noter que les sources de la démo sont disponibles sur ce dépot.

L'ensemble se compile avec SDCC.

( Page du projet : http://hxc2001.free.fr/minitel/index.html )

Jean-François DEL NERO
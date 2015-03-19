# A commander : 
* Sample TI :
  * porte xor : http://www.ti.com/product/SN74LVC1G86/samplebuy
  * Régulateur 5V SOT23 http://www.ti.com/product/LM1117-N/samplebuy
  * demux http://www.ti.com/product/SN74LV4051A/samplebuy et http://www.ti.com/product/SN74LV4051A-EP/samplebuy
  
* ENSEA
  * TIP122 : transistor NPN boitier TO220
  * potentiomettre 3Khom
  * crystal 16Mhz
  * Régulateur 12V 7812


# Exemples de balises IR
* http://poivron-robotique.fr/Balises-2012.html
* http://www.totofweb.net/robots-projet-67.html
* http://simon.chaudeurge.free.fr/DIY/index.php/post/Fabriquer-une-balise-InfraRouge
* http://fribotte.free.fr/bdtech/BaliseIR/baliseIR.html
* http://www.pobot.org/Capteur-IR-circulaire.html?lang=fr


# Fonctionnement 

## Principe 1 (le plus simple logiciellement)
Balise emmetrice robot adverse
balise receptrice notre robot = permet d'éviter le robot adverse


## Principe 2
Balise reception sur notre robot, et balise emmetrices fixe.
Envoi de message synchronisé pour être décallé. Permet de se trianguler et donc de se positionner sur la table.
La même opération effectué par le robot adverse qui nous envoie en plus sa position.
décodage en // avec un mbed nucléo (32IR au moins)

# Spec
## Balise emmetrice
* 32 LED IR modulées 38Khz
* Commandé par le microP (ATMEGA328p) qui envoie une trame de 12 bits en UART
Synchro sans fil avec un nrf24l01 ou xbee (pins dispoavec sortie UART et SPI).
Potentiomettre permettant de réguler la puissance d'emmission


## Balise receptrice
recepteur angle détection de 45 degré 360/45 = 8 donc en théorie 8 suffises
Si on en met 15, cela fait comme s'ils avaient un angle de 24 degré
pas de demux : 18 entrée + rx/tx





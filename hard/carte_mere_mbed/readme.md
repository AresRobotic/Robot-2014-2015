# Ligne directrice de cette carte
* Pouvoir faire le maximum de choses simples à elle seule (avec l'aide d'une carte moteur et d'une carte alim)
* Pouvoir augmenter ses IO pour rajouter autant de fonctions que necessaires en la laissant être le seul cerveau (vive l'I2C)
* Pouvoir eventuellement connecter d'autres microcontrolleur (avec BUS CAN)

## Aquis : 
### Necessaire à l'homologation
* Carte moteur (avec un vrai optocoupleur)
* Carte bouton 2013-2014

### Optionnel
* Carte alim 2012-2013 optionelle si pas d'ax12 et à condition de changer la carte de moteur (rajout d'un petit traco pour fournir le 5V isolé)
* Carte commande MOS qui permet de controller un moteur en Tout Ou Rien avec une PWM (ou sortie Digital) 2013-2014
* Carte couleur

# Fonctionnalités 
## Detection
* 4 ou 6 sharp header 3p : Data_sharp,GND,5V
* 4 ou 6 ultrasons 
 
## Commmande actionneurs
* 3 "connecteurs odométriques" pin 31 - 36 :   (2 roues robot + 1 actionneur)
* commande moteur en i2c (carte similaire à la shield motor v2 de Adafruit) qui permet de commander 4 moteur DC et 1 pas à pas
** Permet de contôler les deux moteurs du la transmission du robot

* 2 connecteurs AX12, isolés de la commande par un adum1250 (rf coupleur symétrique)
* x Connecteurs Servo : Data VCC GND alimentés par le 5V propre 
** Sert pour les pour des petits servo 9g
** Sert pour contrôler des moteurs avec la carte de commande MOS si besoin
** Sert à lire un fréquence pour interfacer avec capteur couleur par exemple
** sert à contrôler des gros servo en ajoutant carte d'adptation avec alim de moteur et si possible opto 


## Communication
* Commande carte bouton : micromatch 4 pin pour couleur et tirette
* i2C avec connecteur micromatch 8 points (même que le CAN, il faudra mettre des nappes de couleur différentes par contre)
** carte d'extension de PWM, expanseur de bus
** écran LCD
** capteurs de température (just for fun)
** Gyroscope (pour l'asservissement en angle)
* Bus CAN avec micromatch qui fait l'alim pour communication entre carte s'il y a besoin
* 3 UART
** AX12
** Autre : bluetooth, communication entre carte de dernière minute, ...


## Protection
Fusible +  Diode Transil (~Zener) sur l'alim
Sérigraphie bien faite


##Ref utile
* isolateur bus I2C : http://www.ti.com/product/iso1541
* expenseur de bus I2C : http://www.ti.com/lit/ds/symlink/pcf8574.pdf
* Capteur températue i2C (soudable) : https://mbed.org/components/STTS751-Temperature-Sensor/
* Capteur températue i2C (soudable) : http://www.ti.com/general/docs/lit/getliterature.tsp?literatureNumber=snis153b
* ADC 12 bits 8 chanels (carte d'extension sharp ?) https://mbed.org/components/ADC128D818/
* Capteur couleur i2C : BM017 ou ADJD-S371-CR999 ou ISL29125

# Carte à faire :
* Carte moteur, améliorer et mettre petit traco pour alimenter la carte mere, permet de faire un robot avec OU sans carte d'alim (pratique pour le petit robot)
* Carte alim plus petite (5V isolé, 5V dirty, 12Vdirty) 

# Questions en suspend 
Prevois t on de faire une boucle d'asservissement en courant (limitation automatique du couple) ?

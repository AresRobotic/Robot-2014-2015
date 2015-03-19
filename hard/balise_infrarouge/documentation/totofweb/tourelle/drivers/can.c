/*
 * Carte de réception de balise
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
 */

/*
 * A chaque fois que l'on manipule le registre CANPAGE, il ne faut
 * pas oublier de sauvegarder sa valeur initiale et de la restaurer
 * ensuite pour ne pas risquer de corrompre le fonctionnement du
 * programme, en particulier dans des interruptions.
 *
 * Lorsqu'une erreur de transmission se produit, les messages sont
 * automatiquement réémis jusqu'à transmettre avec succès (géré
 * par le hardware tant qu'on ne donne pas de contre-ordre).
 */

//-----------------------------
// Librairies
//-----------------------------

#include <avr/io.h>				// Mapping des registres   - http://www.nongnu.org/avr-libc/user-manual/group__avr__io.html
#include <avr/interrupt.h>		// Vecteurs d'interruption - http://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html
#include <compat/deprecated.h>	// Macros sbi et cbi       - http://www.nongnu.org/avr-libc/user-manual/group__deprecated__items.html

//-----------------------------
// Librairies persos
//-----------------------------

#include "can.h"

//-----------------------------
// Variables globales
//-----------------------------

volatile unsigned char can_timerovf;			// Compte le nombre d'overflow du timer CAN depuis la dernière mise à zéro
can_irq_boff_t can_irq_boff;					// Interruption déclenchée en cas de Bus Off
can_irq_mob_t can_irq_mob;						// Interruption déclenchée sur TXOK/RXOK

//-----------------------------
// Fonctions
//-----------------------------

// Gestion des Message Objects
void can_mob_clear(void) {						// Remise à zéro du MOB courant
	//unsigned char i;
	CANSTMOB = 0;
	CANCDMOB = 0;
	CANIDT1 = 0;
	CANIDT2 = 0;
	CANIDT3 = 0;
	CANIDT4 = 0;
	CANIDM1 = 0;
	CANIDM2 = 0;
	CANIDM3 = 0;
	CANIDM4 = 0;
	can_mob_irqoff(can_getmobid());

	// Effacement du FIFO interne :
	// Ici, une répétition est plus optimale qu'une boucle for
	//for (i = 0; i < 8; i++) CANMSG = 0;
	CANMSG = 0;
	CANMSG = 0;
	CANMSG = 0;
	CANMSG = 0;
	CANMSG = 0;
	CANMSG = 0;
	CANMSG = 0;
	CANMSG = 0;
}
void can_mob_setmode(can_mob_mode mode) {		// Définition de l'état du MOb
	CANCDMOB &= (unsigned char)~((1<<CONMOB1)|(1<<CONMOB0)|(1<<RPLV));
	cbi(CANIDT4, RTRTAG);
	switch (mode) {
		case CAN_MOB_MODE_TX_REMOTE:
			sbi(CANIDT4, RTRTAG);
		case CAN_MOB_MODE_TX_DATA:
			sbi(CANCDMOB, CONMOB0);
			break;

		case CAN_MOB_MODE_RX_AUTOREPLY:
			sbi(CANCDMOB, RPLV);
		case CAN_MOB_MODE_RX_REMOTE:
			sbi(CANIDT4, RTRTAG);
		case CAN_MOB_MODE_RX_DATA:
			sbi(CANCDMOB, CONMOB1);
			break;

		/*case CAN_MOB_MODE_FRAMEBUFFERRECEPTION:
			CANCDMOB |= (1<<CONMOB1)|(1<<CONMOB0);
			break;*/

		case CAN_MOB_MODE_DISABLED:
		default:
			break;
	}
}
void can_mob_irqon(unsigned char mob) {			// Activation des interruptions pour le mob
	if (mob < 8)
		sbi(CANIE2, mob);
	else
		sbi(CANIE1, mob-8);
}
void can_mob_irqoff(unsigned char mob) {		// Désactivation des interruptions pour le mob
	if (mob < 8)
		cbi(CANIE2, mob);
	else
		cbi(CANIE1, mob-8);
}
unsigned char can_mob_state_idle(unsigned char mob) {	// Indique si le MOb est inactif
	if (mob < 8)
		return bit_is_clear(CANEN2, mob);
	else
		return bit_is_clear(CANEN1, mob-8);
}

// Gestion générale du CAN
void can_setmode(can_mode mode) {				// Sélection du mode de fonctionnement du contrôleur CAN
	while (!bit_is_clear(CANGSTA, RXBSY));		// Pour éviter tout état imprévu (cf datasheet)
	switch (mode) {
		case CAN_MODE_LISTENING:
			sbi(CANGCON, LISTEN);
			sbi(CANGCON, ENASTB);
			break;
		case CAN_MODE_ENABLED:
			cbi(CANGCON, LISTEN);
			sbi(CANGCON, ENASTB);
			break;
		case CAN_MODE_STANDBY:
		default:
			cbi(CANGCON, LISTEN);
			cbi(CANGCON, ENASTB);
			break;
	}
}
void can_setbaudrate(unsigned int kbps) {		// Changement du baudrate
	unsigned char TQ;

	/*
	 * On divise chaque période de base du CAN en 16 TQ (Time Quantum)
	 *  - Propagation Time Segment = 7 TQ
	 *  - Phase Segment 1 = 4 TQ
	 *  - Phase Segment 2 = 4 TQ
	 *  - Resynchronization Jump Width = 1 TQ
	 */

	if (kbps > CAN_MAXBAUD) kbps = CAN_MAXBAUD;

	TQ = (unsigned char)(((F_CPU_DIV16000)/kbps)) - 1;

	CANBT1 = TQ << BRP0;
	CANBT2 = (0 << SJW0) | (6 << PRS0);
	CANBT3 = (3 << PHS20) | (3 << PHS10) | (1<<SMP);
}
void can_init(unsigned int kbps, can_mode mode, can_interrupts_t interrupts) {	// Configuration initiale du contrôleur CAN
	unsigned char mob;

	// Configuration des IO (en principe c'est fait tout seul par le hardware, mais c'est une précaution de plus)
	sbi(CAN_TX_DDR, CAN_TX_PIN);
	cbi(CAN_RX_DDR, CAN_RX_PIN);
	sbi(CAN_LED_DDR, CAN_LED_PIN);

	// Reset du contrôleur
	can_reset();

	// Configuration du baudrate
	can_setbaudrate(kbps);

	// Effacement de tous les mobs
	for (mob = 0; mob < CAN_NB_MOB; mob++) {
		can_mob_select(mob);
		can_mob_clear();
		can_mob_setmode(CAN_MOB_MODE_DISABLED);
	}

	// Configuration du prescaler du timer CAN pour avoir un overflow toutes les 100ms
	CANTCON = 2;
	can_cleartimerovf();

	// Désactivation des interruptions
	can_irq_boff.en = 0;
	can_irq_mob.en = 0;

	CANGIE = 0;
	if (interrupts != NO_INTERRUPTS) {
		sbi(CANGIE, ENIT);		// Activation globale des interruption du CAN
		sbi(CANGIE, ENTX);		// Activation de l'interruption sur émission accomplie
		sbi(CANGIE, ENRX);		// Activation de l'interruption sur réception valide
		//sbi(CANGIE, ENERR);	// Activation de l'interruption sur erreur de transmission
	}
	sbi(CANGIE, ENOVRT);		// Activation de l'interruption sur overflow de timer can

	// Passage du contrôleur en mode enabled
	can_setmode(mode);
}

//-----------------------------
// Interruptions
//-----------------------------

ISR(OVRIT_vect) {								// Interruption sur overflow du timer CAN (toutes les ~100ms)
	can_timerovf++;

	// Actualisation de la LED d'état de bus
	if (can_state_buserror())
		sbi(CAN_LED_PORT, CAN_LED_PIN);
	else
		cbi(CAN_LED_PORT, CAN_LED_PIN);
}
#ifdef CAN_NOBLOCK
// TODO : corriger le cas NOBLOCK
// NOBLOCK pas possible directement, il faut ruser
	#error "CAN_NOBLOCK pas encore géré"
#else
ISR(CANIT_vect) {								// Interruption sur un évènement a priori quelconque (succès/erreur)
#endif
	unsigned char canpage_copy = CANPAGE;		// Sauvegarde du mob actuellement sélectionné
	unsigned char mob;							// MOb ayant généré l'interruption ?

	if (can_state_buserror()) {
		// Contrôleur en mode Bus Off
		if (can_irq_boff.en)
			(*(can_irq_boff.func))();
	} else if (can_irq_mob.en) {
		/*
		 * Une transmission s'est achevée avec succès ou bien une trame a été matchée
		 * en réception.
		 * Il est toutefois possible que quelques erreurs soient survenues (stuff
		 * error, crc, etc...), mais l'essentiel est que le message ait finalement pu
		 * être reçu/envoyé correctement (sans garantie d'exactitude de DLC).
		 */
		mob = can_int_getmob();					// Quel mob a généré l'interruption ?
		can_mob_select(mob);

		(*(can_irq_mob.func))(mob);

		CANSTMOB = 0; 							// Reset des flags d'interruption du MOb (notamment RXOK et TXOK)
	}
	CANGIT = 0xFF;								// Reset des flags d'interruption généraux
	CANPAGE = canpage_copy;						// Restauration de la situation initiale
}

/*
 * Carte de réception de balise
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
 */

#ifndef DRIVER_CAN_H
#define DRIVER_CAN_H

//-----------------------------
// Utilisation des ports
//-----------------------------

#define CAN_TX_DDR				DDRD
#define CAN_TX_PIN				5
#define CAN_RX_DDR				DDRD
#define CAN_RX_PIN				6

#include "can_config.h"

//-----------------------------
// Defines divers
//-----------------------------

#define CAN_NB_MOB				15
#define CAN_NO_MOB				0xFF
#define CAN_MSK_11B				0x7FF		//(unsigned int)((1<<11)-1)
#define CAN_MSK_29B				0x1FFFFFFF	//(unsigned long)((1<<29)-1)

#define F_CPU_DIV16000			((unsigned int)(F_CPU/16000))
#define CAN_MAXBAUD				((unsigned int)(F_CPU/16000))

//-----------------------------
// Types spéciaux
//-----------------------------

typedef enum {						// Mode du contrôleur CAN
	CAN_MODE_ENABLED,
	CAN_MODE_STANDBY,
	CAN_MODE_LISTENING
} can_mode;
typedef enum {						// Mode d'un MOb
	CAN_MOB_MODE_DISABLED,
	CAN_MOB_MODE_TX_DATA,
	CAN_MOB_MODE_TX_REMOTE,
	CAN_MOB_MODE_RX_DATA,
	CAN_MOB_MODE_RX_REMOTE,
	CAN_MOB_MODE_RX_AUTOREPLY,
	//CAN_MOB_MODE_FRAMEBUFFERRECEPTION
} can_mob_mode;
typedef enum {
	NO_INTERRUPTS,
	INTERRUPTS
} can_interrupts_t;					// Paramètre pour can_init
typedef struct {
	unsigned char en;				// Interruption activée
	void (*func)(void);				// Fonction d'interruption
} can_irq_boff_t;
typedef struct {
	unsigned char en;				// Interruption activée
	void (*func)(unsigned char);	// Fonction d'interruption
} can_irq_mob_t;

//-----------------------------
// Prototypes
//-----------------------------

// Gestion des Message Objects
static inline unsigned char can_getmobid(void) __attribute__((always_inline));
static inline void can_mob_select(unsigned char mob) __attribute__((always_inline));
static inline void can_mob_setid_std(unsigned int id) __attribute__((always_inline));
static inline void can_mob_setid_ext(unsigned long id) __attribute__((always_inline));
static inline unsigned int can_mob_getid_std(void) __attribute__((always_inline));
static inline unsigned long can_mob_getid_ext(void) __attribute__((always_inline));
static inline void can_mob_setmsk_std(unsigned int msk) __attribute__((always_inline));
static inline void can_mob_setmsk_ext(unsigned long msk) __attribute__((always_inline));
static inline void can_mob_setidemsk(void) __attribute__((always_inline));
static inline void can_mob_clearidemsk(void) __attribute__((always_inline));
static inline unsigned char can_mob_getide(void) __attribute__((always_inline));
static inline void can_mob_setrtr(void) __attribute__((always_inline));
static inline void can_mob_clearrtr(void) __attribute__((always_inline));
static inline void can_mob_setrtrmsk(void) __attribute__((always_inline));
static inline void can_mob_clearrtrmsk(void) __attribute__((always_inline));
static inline unsigned char can_mob_getrtr(void) __attribute__((always_inline));
static inline void can_mob_setdlc(unsigned char length) __attribute__((always_inline));
static inline unsigned char can_mob_getdlc(void) __attribute__((always_inline));
void can_mob_clear(void);
void can_mob_setmode(can_mob_mode mode);
void can_mob_irqon(unsigned char mob);
void can_mob_irqoff(unsigned char mob);
static inline void can_mob_setdata(unsigned char * in, unsigned char dlc);
static inline unsigned char can_mob_getdata(unsigned char * out, unsigned char dlcmax);
static inline unsigned char can_mob_state_txcompleted(void) __attribute__((always_inline));
static inline unsigned char can_mob_state_rxcompleted(void) __attribute__((always_inline));
static inline unsigned char can_mob_state_notcompleted(void) __attribute__((always_inline));
unsigned char can_mob_state_idle(unsigned char mob);
static inline unsigned char can_mob_state_error(void) __attribute__((always_inline));

// Gestion générale du CAN
void can_setmode(can_mode mode);
static inline void can_reset(void) __attribute__((always_inline));
void can_setbaudrate(unsigned int kbps);
void can_init(unsigned int kbps, can_mode mode, can_interrupts_t interrupts);
static inline unsigned char can_state_buserror(void) __attribute__((always_inline));
static inline unsigned char can_state_buserrorpassive(void) __attribute__((always_inline));
static inline unsigned char can_gettimerovf(void) __attribute__((always_inline));
static inline void can_cleartimerovf(void) __attribute__((always_inline));

// Interruptions
static inline unsigned char can_int_getmob(void);
static inline void can_int_mob_enable(void (*func)(unsigned char)) __attribute__((always_inline));
static inline void can_int_boff_enable(void (*func)(void)) __attribute__((always_inline));
static inline void can_int_mob_disable(void) __attribute__((always_inline));
static inline void can_int_boff_disable(void) __attribute__((always_inline));

//-----------------------------
// Fonctions inlinées
//-----------------------------

// Librairies
#include <avr/io.h>				// Mapping des registres   - http://www.nongnu.org/avr-libc/user-manual/group__avr__io.html
#include <compat/deprecated.h>	// Macros sbi et cbi       - http://www.nongnu.org/avr-libc/user-manual/group__deprecated__items.html

// Variables globales
extern volatile unsigned char can_timerovf;
extern can_irq_boff_t can_irq_boff;
extern can_irq_mob_t can_irq_mob;

unsigned char can_getmobid(void) {						// Récupération du numéro du MOb sélectionné
	return (CANPAGE >> 4);
}
void can_mob_select(unsigned char mob) {		// Sélection du MOB accédé
	// Au passage, on se place aussi en début de buffer
	// et on active l'autoincrémentation d'index
	CANPAGE = (unsigned char)(mob << 4);
}
void can_mob_setid_std(unsigned int id) {		// Definition d'un identifier sur 11 bits
	CANIDT1 = (unsigned char)(id >> 3);
	CANIDT2 = (unsigned char)(id << 5);
	cbi(CANCDMOB, IDE);
}
void can_mob_setid_ext(unsigned long id) {		// Définition d'un identifier sur 29 bits (attention, efface le bit RTRTAG !)
	CANIDT1 = (unsigned char)(id >> 21);
	CANIDT2 = (unsigned char)(id >> 13);
	CANIDT3 = (unsigned char)(id >> 5);
	CANIDT4 = (unsigned char)(id << 3);
	sbi(CANCDMOB, IDE);
}
unsigned int can_mob_getid_std(void) {			// Récupération d'un identifier sur 11 bits
	return ((unsigned int)CANIDT1<<3)+((unsigned int)CANIDT2>>5);
}
unsigned long can_mob_getid_ext(void) {			// Récupération d'un identifier sur 29 bits
	return ((unsigned long)CANIDT1<<21)+((unsigned long)CANIDT2<<13)+((unsigned long)CANIDT3<<5)+((unsigned long)CANIDT4>>3);
}
void can_mob_setmsk_std(unsigned int msk) {		// Définition d'un masque de réception sur 11 bits
	// Le bit IDE est défini au moment de l'écriture de l'identifier
	CANIDM1 = (unsigned char)(msk >> 3);
	CANIDM2 = (unsigned char)(msk << 5);
}
void can_mob_setmsk_ext(unsigned long msk) {	// Définition d'un masque de réception sur 29 bits (attention, efface le bit RTRMSK !)
	// Le bit IDE est défini au moment de l'écriture de l'identifier
	CANIDM1 = (unsigned char)(msk >> 21);
	CANIDM2 = (unsigned char)(msk >> 13);
	CANIDM3 = (unsigned char)(msk >> 5);
	CANIDM4 = (unsigned char)(msk << 3);
}
void can_mob_setidemsk(void) {					// Matcher aussi l'IDE en réception
	sbi(CANIDM4, IDEMSK);	// Activation du masque
}
void can_mob_clearidemsk(void) {				// Ignorer l'IDE en réception
	cbi(CANIDM4, IDEMSK);	// Désactivation du masque
}
unsigned char can_mob_getide(void) {			// Lecture du bit IDE
	return bit_is_set(CANCDMOB, IDE);
}
void can_mob_setrtr(void) {						// Type Remote
	sbi(CANIDT4, RTRTAG);
}
void can_mob_clearrtr(void) {					// Type Data
	cbi(CANIDT4, RTRTAG);
}
void can_mob_setrtrmsk(void) {					// Matcher aussi le RTR en réception
	sbi(CANIDM4, RTRMSK);	// Activation du masque
}
void can_mob_clearrtrmsk(void) {				// Ignorer le RTR en réception
	cbi(CANIDM4, RTRMSK);	// Désactivation du masque
}
unsigned char can_mob_getrtr(void) {			// Lecture du bit RTR
	return bit_is_set(CANIDT4, RTRTAG);
}
void can_mob_setdlc(unsigned char length) {		// Définition du nombre d'octets à envoyer/recevoir
	CANCDMOB &= ~((1<<DLC3)|(1<<DLC2)|(1<<DLC1)|(1<<DLC0));
	CANCDMOB |= (length & ((1<<DLC3)|(1<<DLC2)|(1<<DLC1)|(1<<DLC0)));
}
unsigned char can_mob_getdlc(void) {			// Récupération du nombre d'octets reçus
	return (CANCDMOB & ((1<<DLC3)|(1<<DLC2)|(1<<DLC1)|(1<<DLC0)));
}
void can_mob_setdata(unsigned char * in, unsigned char dlc) {	// Chargement de données dans le buffer du mob courant.
	unsigned char i;
	for (i = 0; i < dlc; i++)
		CANMSG = *(in+i);
	can_mob_setdlc(dlc);
}
unsigned char can_mob_getdata(unsigned char * out, unsigned char dlcmax) {	// Récupération du contenu du buffer FIFO du mob courant. Renvoie le nombre de bits contenus dans le DLC
	unsigned char i;
	if (can_mob_getdlc() < dlcmax)
		dlcmax = can_mob_getdlc();
	for (i = 0; i < dlcmax; i++)
		*(out+i) = CANMSG;
	return dlcmax;
}
unsigned char can_mob_state_txcompleted(void) {	// Indique si le MOb a fini une réception valide
	return bit_is_set(CANSTMOB, TXOK);
}
unsigned char can_mob_state_rxcompleted(void) {	// Indique si le MOb a fini une émission valide
	return bit_is_set(CANSTMOB, RXOK);
}
unsigned char can_mob_state_notcompleted(void) {	// Indique si le MOb est encore en attente de l'action qui lui a été assignée
	return ~(CANSTMOB & ((1<<TXOK)|(1<<RXOK)));
}
unsigned char can_mob_state_error(void) {		// Indique si le MOb a connu des erreurs depuis sa réinitialisation
	return (CANSTMOB & ((1<<AERR)|(1<<FERR)|(1<<CERR)|(1<<SERR)|(1<<BERR)));
}

void can_reset(void) {							// Reset du contrôleur CAN
	sbi(CANGCON, SWRES);	// Auto-resettable bit
}
unsigned char can_state_buserror(void) {
	return bit_is_set(CANGSTA, BOFF);
}
unsigned char can_state_buserrorpassive(void) {
	return bit_is_set(CANGSTA, ERRP);
}
unsigned char can_gettimerovf(void) {			// Récupération du nombre d'overflows du timer CAN
	return can_timerovf;
}
void can_cleartimerovf(void) {					// Redéfinition du nombre d'overflows du timer CAN
	can_timerovf = 0;
}

unsigned char can_int_getmob(void) {			// Détermination du MOb responsable de l'interruption déclenchée
	unsigned char mob = (CANHPMOB >> HPMOB0);
	if (mob < CAN_NB_MOB)
		return mob;
	else
		return CAN_NO_MOB;
}
void can_int_mob_enable(void (*func)(unsigned char)) {
	can_irq_mob.func = func;
	can_irq_mob.en = 1;
}
void can_int_boff_enable(void (*func)(void)) {
	can_irq_boff.func = func;
	can_irq_boff.en = 1;
	sbi(CANGIE, ENBOFF);	// Activation de l'interruption sur mode Bus Off
}
void can_int_mob_disable(void) {
	can_irq_mob.en = 0;
}
void can_int_boff_disable(void) {
	cbi(CANGIE, ENBOFF);	// Activation de l'interruption sur mode Bus Off
	can_irq_boff.en = 0;
}

#endif

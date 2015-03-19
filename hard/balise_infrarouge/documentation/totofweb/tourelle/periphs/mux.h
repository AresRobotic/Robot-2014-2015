/*
 * Carte de réception de balise
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
 */

#ifndef PERIPH_MUX_H
#define PERIPH_MUX_H

//-----------------------------
// Utilisation des ports
//-----------------------------

#define MUX0_INH_DDR		DDRE
#define MUX0_INH_PORT		PORTE
#define MUX0_INH_PIN		2
#define MUX0_A_DDR			DDRE
#define MUX0_A_PORT			PORTE
#define MUX0_A_PIN			3
#define MUX0_B_DDR			DDRE
#define MUX0_B_PORT			PORTE
#define MUX0_B_PIN			4
#define MUX0_C_DDR			DDRE
#define MUX0_C_PORT			PORTE
#define MUX0_C_PIN			5
#define MUX1_INH_DDR		DDRA
#define MUX1_INH_PORT		PORTA
#define MUX1_INH_PIN		6
#define MUX1_A_DDR			DDRA
#define MUX1_A_PORT			PORTA
#define MUX1_A_PIN			3
#define MUX1_B_DDR			DDRA
#define MUX1_B_PORT			PORTA
#define MUX1_B_PIN			4
#define MUX1_C_DDR			DDRA
#define MUX1_C_PORT			PORTA
#define MUX1_C_PIN			5

//-----------------------------
// Prototypes
//-----------------------------

void mux_init(void);

// Fonctions inlinées
static inline void mux_enable(void) __attribute__((always_inline));
static inline void mux_disable(void) __attribute__((always_inline));
static inline void mux_set(unsigned char num) __attribute__((always_inline));

//-----------------------------
// Fonctions inlinées
//-----------------------------

#include <avr/io.h>				// Mapping des registres   - http://www.nongnu.org/avr-libc/user-manual/group__avr__io.html
#include <compat/deprecated.h>	// Macros sbi et cbi       - http://www.nongnu.org/avr-libc/user-manual/group__deprecated__items.html

void mux_enable(void) {
	sbi(MUX0_INH_DDR, MUX0_INH_PIN);
	sbi(MUX1_INH_DDR, MUX1_INH_PIN);
}
void mux_disable(void) {
	cbi(MUX0_INH_DDR, MUX0_INH_PIN);
	cbi(MUX1_INH_DDR, MUX1_INH_PIN);
}
void mux_set(unsigned char num) {
	MUX0_A_PORT &= ~(7 << MUX0_A_PIN);
	MUX1_A_PORT &= ~(7 << MUX1_A_PIN);
	MUX0_A_PORT |= (num << MUX0_A_PIN);
	MUX1_A_PORT |= (num << MUX1_A_PIN);
}

#endif

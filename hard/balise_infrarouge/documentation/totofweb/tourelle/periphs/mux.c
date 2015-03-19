/*
 * Carte de réception de balise
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
 */

//-----------------------------
// Librairies
//-----------------------------

#include <avr/io.h>				// Mapping des registres   - http://www.nongnu.org/avr-libc/user-manual/group__avr__io.html
#include <compat/deprecated.h>	// Macros sbi et cbi       - http://www.nongnu.org/avr-libc/user-manual/group__deprecated__items.html

//-----------------------------
// Librairies persos
//-----------------------------

#include "mux.h"

//-----------------------------
// Fonctions
//-----------------------------

void mux_init(void) {
	// Par défaut, les entrées INH sont en haute impédance
	cbi(MUX0_INH_DDR, MUX0_INH_PIN);
	cbi(MUX1_INH_DDR, MUX1_INH_PIN);
	cbi(MUX0_INH_PORT, MUX0_INH_PIN);
	cbi(MUX1_INH_PORT, MUX1_INH_PIN);

	sbi(MUX0_A_DDR, MUX0_A_PIN);
	sbi(MUX1_A_DDR, MUX1_A_PIN);
	sbi(MUX0_B_DDR, MUX0_B_PIN);
	sbi(MUX1_B_DDR, MUX1_B_PIN);
	sbi(MUX0_C_DDR, MUX0_C_PIN);
	sbi(MUX1_C_DDR, MUX1_C_PIN);
}

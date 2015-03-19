/*
 * Carte de réception de balise
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
 */

//-----------------------------
// Librairies
//-----------------------------

#include <util/delay.h>			// Attente bloquante       - http://www.nongnu.org/avr-libc/user-manual/group__util__delay.html

//-----------------------------
// Librairies persos
//-----------------------------

#include "delais.h"

//-----------------------------
// Macros
//-----------------------------

#define DELAI_MAX (262140000UL/F_CPU)

//-----------------------------
// Fonctions
//-----------------------------

void delay_ms(unsigned int duree) {
	// Pour la fonction _delay_ms, le delai maximal possible est
	// 262.14 ms / F_CPU(MHz) = 16.38ms
	for (; duree >= DELAI_MAX; duree -= DELAI_MAX)
		_delay_ms(DELAI_MAX);
	if (duree)
		_delay_ms(duree);
}

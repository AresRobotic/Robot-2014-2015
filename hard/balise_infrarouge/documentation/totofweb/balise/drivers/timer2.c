/*
 * Balise d'émission IR doublement codée
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
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

#include "timer2.h"

//-----------------------------
// Variables globales
//-----------------------------

void (*timer2_isr[2])(void) = {T2_ISR_NULLFUNC, T2_ISR_NULLFUNC};
volatile unsigned char timer2_isr_cnt[2] = {0, 0};

//-----------------------------
// Fonctions
//-----------------------------

// Fonctions spécifiques au fonctionnement en mode PWM
void timer2pwm_init(T2_presc prescaler) {		// Configuration initiale en mode PWM

	// Configuration en entrée des ports (PWM désactivée par défaut)
	cbi(OC2A_DDR, OC2A_PIN);

	// Configuration à l'état bas par défaut
	cbi(OC2A_PORT, OC2A_PIN);

	timer2_setpresc(prescaler);
	timer2_setmode(T2_MODE_PWM_FAST);
	timer2_compare_setmode(T2_OC_MODE_CLEAR_ON_COMPARE);
}

// Fonctions spécifiques au fonctionnement en mode CTC
void timer2ctc_init(unsigned long freq) {		// Configuration initiale en mode CTC (Clear on Timer Compare)
	timer2ctc_setfreq(freq);
	timer2_setmode(T2_MODE_CTC);
}
void timer2ctc_setfreq(unsigned long freq) {	// Réglage sur une fréquence précise

	// On convertit la fréquence en Hz en nombre de cycles d'horloge par période
	unsigned long ticks = ((unsigned long)F_CPU)/(freq);

	// On adapte le prescaler en conséquence, de manière à avoir l'erreur la plus faible possible
	if (ticks < 256) {
		timer2_setpresc(T2_CLK_DIV1);
		OCR2 = (unsigned char)(ticks-1);
	} else {
		ticks >>= 3;
		if (ticks < 256) {
			timer2_setpresc(T2_CLK_DIV8);
			OCR2 = (unsigned char)(ticks-1);
		} else {
			ticks >>= 2;
			if (ticks < 256) {
				timer2_setpresc(T2_CLK_DIV32);
				OCR2 = (unsigned char)(ticks-1);
			} else {
				ticks >>= 1;
				if (ticks < 256) {
					timer2_setpresc(T2_CLK_DIV64);
					OCR2 = (unsigned char)(ticks-1);
				} else {
					ticks >>= 1;
					if (ticks < 256) {
						timer2_setpresc(T2_CLK_DIV128);
						OCR2 = (unsigned char)(ticks-1);
					} else {
						ticks >>= 1;
						if (ticks < 256) {
							timer2_setpresc(T2_CLK_DIV256);
							OCR2 = (unsigned char)(ticks-1);
						} else {
							ticks >>= 2;
							if (ticks > 256)
								ticks = 256;
							timer2_setpresc(T2_CLK_DIV1024);
							OCR2 = (unsigned char)(ticks-1);
						}
					}
				}
			}
		}
	}
}

//-----------------------------
// Interruptions
//-----------------------------

ISR(TIMER2_OVF_vect) {							// Interruption sur overflow de TCNT2
	timer2_isr_cnt[(unsigned char)T2_ISR_OVERFLOW]++;
	if (timer2_isr[(unsigned char)T2_ISR_OVERFLOW])
		(*timer2_isr[(unsigned char)T2_ISR_OVERFLOW])();
}
ISR(TIMER2_COMP_vect) {							// Interruption sur Compare Match (TCNT2 = OCR2)
	timer2_isr_cnt[(unsigned char)T2_ISR_COMP_A]++;
	if (timer2_isr[(unsigned char)T2_ISR_COMP_A])
		(*timer2_isr[(unsigned char)T2_ISR_COMP_A])();
}

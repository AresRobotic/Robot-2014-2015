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

#include "timer1.h"

//-----------------------------
// Variables globales
//-----------------------------

void (*timer1_isr[3])(void) = {T1_ISR_NULLFUNC, T1_ISR_NULLFUNC, T1_ISR_NULLFUNC};
volatile unsigned char timer1_isr_cnt[3] = {0, 0, 0};

//-----------------------------
// Fonctions
//-----------------------------

// Fonctions spécifiques au fonctionnement en mode PWM
void timer1pwm_init(T1_presc prescaler, T1_mode_t mode) {		// Configuration initiale en mode PWM

	// Configuration en entrée des ports (PWM désactivée par défaut)
	cbi(OC1A_DDR, OC1A_PIN);
	cbi(OC1B_DDR, OC1B_PIN);

	// Configuration à l'état bas par défaut
	cbi(OC1A_PORT, OC1A_PIN);
	cbi(OC1B_PORT, OC1B_PIN);

	timer1_setpresc(prescaler);
	timer1_setmode(mode);
	timer1_compare_setmodeA(T1_OC_MODE_CLEAR_ON_COMPARE);
	timer1_compare_setmodeB(T1_OC_MODE_CLEAR_ON_COMPARE);
}

// Fonctions spécifiques au fonctionnement en mode CTC
void timer1ctc_init(unsigned long freq) {		// Configuration initiale en mode CTC (Clear on Timer Compare)
	timer1ctc_setfreq(freq);
	timer1_setmode(T1_MODE_CTC);
}
void timer1ctc_setfreq(unsigned long freq) {	// Réglage sur une fréquence précise

	// On convertit la fréquence en Hz en nombre de cycles d'horloge par période
	unsigned long ticks = ((unsigned long)F_CPU)/(freq);

	// On adapte le prescaler en conséquence, de manière à avoir l'erreur la plus faible possible
	if (ticks < 65536) {
		timer1_setpresc(T1_CLK_DIV1);
		OCR1A = (unsigned int)(ticks-1);
	} else {
		ticks >>= 3;
		if (ticks < 65536) {
			timer1_setpresc(T1_CLK_DIV8);
			OCR1A = (unsigned int)(ticks-1);
		} else {
			ticks >>= 3;
			if (ticks < 65536) {
				timer1_setpresc(T1_CLK_DIV64);
				OCR1A = (unsigned int)(ticks-1);
			} else {
				ticks >>= 2;
				if (ticks < 65536) {
					timer1_setpresc(T1_CLK_DIV256);
					OCR1A = (unsigned int)(ticks-1);
				} else {
					ticks >>= 2;
					if (ticks > 65536)
						ticks = 65536;
					timer1_setpresc(T1_CLK_DIV1024);
					OCR1A = (unsigned int)(ticks-1);
				}
			}
		}
	}
}

//-----------------------------
// Interruptions
//-----------------------------

ISR(TIMER1_OVF_vect) {							// Interruption sur overflow de TCNT0
	timer1_isr_cnt[(unsigned char)T1_ISR_OVERFLOW]++;
	if (timer1_isr[(unsigned char)T1_ISR_OVERFLOW])
		(*timer1_isr[(unsigned char)T1_ISR_OVERFLOW])();
}
ISR(TIMER1_COMPA_vect) {						// Interruption sur Compare Match (TCNT1 = OCR1A)
	timer1_isr_cnt[(unsigned char)T1_ISR_COMP_A]++;
	if (timer1_isr[(unsigned char)T1_ISR_COMP_A])
		(*timer1_isr[(unsigned char)T1_ISR_COMP_A])();
}
ISR(TIMER1_COMPB_vect) {						// Interruption sur Compare Match (TCNT1 = OCR1B)
	timer1_isr_cnt[(unsigned char)T1_ISR_COMP_B]++;
	if (timer1_isr[(unsigned char)T1_ISR_COMP_B])
		(*timer1_isr[(unsigned char)T1_ISR_COMP_B])();
}

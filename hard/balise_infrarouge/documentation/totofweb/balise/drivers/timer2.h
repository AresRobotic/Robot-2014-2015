/*
 * Balise d'émission IR doublement codée
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
 */

#ifndef DRIVER_TIMER2_H
#define DRIVER_TIMER2_H

//-----------------------------
// Defines divers
//-----------------------------

#define T2_ISR_NULLFUNC			0		//((void)(*0)())

//-----------------------------
// Attribution des IO
//-----------------------------

#define OC2A_PORT				PORTB
#define OC2A_DDR				DDRB
#define OC2A_PIN				3

//-----------------------------
// Librairies
//-----------------------------

#include <avr/io.h>				// Mapping des registres   - http://www.nongnu.org/avr-libc/user-manual/group__avr__io.html
#include <compat/deprecated.h>	// Macros sbi et cbi       - http://www.nongnu.org/avr-libc/user-manual/group__deprecated__items.html

//-----------------------------
// Types personnalisés
//-----------------------------

typedef enum {					// Interruptions disponibles
	T2_ISR_OVERFLOW					= 0,
	T2_ISR_COMP_A					= 1
} T2_isr;
typedef enum {					// Mode d'opération du timer
	T2_MODE_NORMAL					= ((0<<WGM21)|(0<<WGM20)),
	T2_MODE_PWM_PHASECORRECT		= ((0<<WGM21)|(1<<WGM20)),
	T2_MODE_CTC						= ((1<<WGM21)|(0<<WGM20)),
	T2_MODE_PWM_FAST				= ((1<<WGM21)|(1<<WGM20)),
} T2_mode_t;
typedef enum {					// Sélection de la source d'horloge
	T2_CLK_STOP						= ((0<<CS22)|(0<<CS21)|(0<<CS20)),
	T2_CLK_DIV1						= ((0<<CS22)|(0<<CS21)|(1<<CS20)),
	T2_CLK_DIV8						= ((0<<CS22)|(1<<CS21)|(0<<CS20)),
	T2_CLK_DIV32					= ((0<<CS22)|(1<<CS21)|(1<<CS20)),
	T2_CLK_DIV64					= ((1<<CS22)|(0<<CS21)|(0<<CS20)),
	T2_CLK_DIV128					= ((1<<CS22)|(0<<CS21)|(1<<CS20)),
	T2_CLK_DIV256					= ((1<<CS22)|(1<<CS21)|(0<<CS20)),
	T2_CLK_DIV1024					= ((1<<CS22)|(1<<CS21)|(1<<CS20)),
} T2_presc;
typedef enum {					// Mode d'opération des Output Compare channels
	T2_OC_MODE_NORMAL				= 0,
	T2_OC_MODE_TOGGLE_ON_COMPARE	= 1,
	T2_OC_MODE_CLEAR_ON_COMPARE		= 2,
	T2_OC_MODE_SET_ON_COMPARE		= 3,
} T2_OC_mode_t;

//-----------------------------
// Prototypes
//-----------------------------

// Fonctions générales de paramétrage du timer
static inline void timer2_setmode(T2_mode_t mode) __attribute__((always_inline));
static inline void timer2_setpresc(T2_presc prescaler) __attribute__((always_inline));
static inline void timer2_compare_setmode(T2_OC_mode_t mode) __attribute__((always_inline));
static inline void timer2_compare_setcomp(unsigned char cmp) __attribute__((always_inline));
static inline void timer2_cleartcnt(void) __attribute__((always_inline));
static inline unsigned char timer2_gettcnt(void) __attribute__((always_inline));

// Fonctions spécifiques au fonctionnement en mode PWM
void timer2pwm_init(T2_presc prescaler);
static inline void timer2pwm_enable(void) __attribute__((always_inline));
static inline void timer2pwm_disable(void) __attribute__((always_inline));
static inline void timer2pwm_setduty(unsigned char duty) __attribute__((always_inline));

// Fonctions spécifiques au fonctionnement en mode CTC
void timer2ctc_init(unsigned long freq);
static inline void timer2ctc_enable(void) __attribute__((always_inline));
static inline void timer2ctc_disable(void) __attribute__((always_inline));
void timer2ctc_setfreq(unsigned long freq);
static inline void timer2ctc_isr_enable(void (*func)(void)) __attribute__((always_inline));
static inline void timer2ctc_isr_disable(void) __attribute__((always_inline));
static inline unsigned char timer2ctc_isr_getcnt(void) __attribute__((always_inline));
static inline void timer2ctc_isr_clearcnt(void) __attribute__((always_inline));

//-----------------------------
// Fonctions inlinées
//-----------------------------

extern void (*timer2_isr[2])(void);
extern volatile unsigned char timer2_isr_cnt[2];

// Fonctions générales de paramétrage du timer
void timer2_setmode(T2_mode_t mode) {
	TCCR2 &= ~((1<<WGM21)|(1<<WGM20));
	TCCR2 |= (unsigned char)mode;
}
void timer2_setpresc(T2_presc prescaler) {
	TCCR2 &= ~((1<<CS22)|(1<<CS21)|(1<<CS20));
	TCCR2 |= (unsigned char)prescaler;
}
void timer2_compare_setmode(T2_OC_mode_t mode) {
	TCCR2 &= ~((1<<COM21)|(1<<COM20));
	TCCR2 |= (unsigned char)mode;
}
void timer2_compare_setcomp(unsigned char cmp) {
	OCR2 = cmp;
}
void timer2_cleartcnt(void) {
	TCNT2 = 0;
}
unsigned char timer2_gettcnt(void) {
	return TCNT2;
}

// Fonctions spécifiques au fonctionnement en mode PWM
void timer2pwm_enable(void) {
	sbi(OC2A_DDR, OC2A_PIN);
}
void timer2pwm_disable(void) {
	// La sortie revient d'elle-même à l'état bas grâce
	// à la configuration faite dans timer2pwm_init()
	cbi(OC2A_DDR, OC2A_PIN);
}
void timer2pwm_setduty(unsigned char duty) {
	OCR2 = duty;
}

// Fonctions spécifiques au fonctionnement en mode CTC
void timer2ctc_enable(void) {
	sbi(OC2A_DDR, OC2A_PIN);
}
void timer2ctc_disable(void) {
	// La sortie revient d'elle-même à l'état bas grâce
	// à la configuration faite dans timer2ctc_init()
	cbi(OC2A_DDR, OC2A_PIN);
}
void timer2ctc_isr_enable(void (*func)(void)) {
	timer2_isr[(unsigned char)T2_ISR_COMP_A] = func;
	sbi(TIFR, OCF2);			// Reset du flag d'interruption
	sbi(TIMSK, OCIE2);
}
void timer2ctc_isr_disable(void) {
	cbi(TIMSK, OCIE2);
}
unsigned char timer2ctc_isr_getcnt(void) {
	return timer2_isr_cnt[(unsigned char)T2_ISR_COMP_A];
}
void timer2ctc_isr_clearcnt(void) {
	timer2_isr_cnt[(unsigned char)T2_ISR_COMP_A] = 0;
}

#endif

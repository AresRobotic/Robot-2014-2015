/*
 * Carte de réception de balise
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
 */

#ifndef DRIVER_TIMER1_H
#define DRIVER_TIMER1_H

//-----------------------------
// Defines divers
//-----------------------------

#define T1_ISR_NULLFUNC			0		//((void)(*0)())

//-----------------------------
// Attribution des IO
//-----------------------------

#define OC1A_PORT				PORTB
#define OC1A_DDR				DDRB
#define OC1A_PIN				5
#define OC1B_PORT				PORTB
#define OC1B_DDR				DDRB
#define OC1B_PIN				6
#define OC1C_PORT				PORTB
#define OC1C_DDR				DDRB
#define OC1C_PIN				7

//-----------------------------
// Librairies
//-----------------------------

#include <avr/io.h>				// Mapping des registres   - http://www.nongnu.org/avr-libc/user-manual/group__avr__io.html
#include <compat/deprecated.h>	// Macros sbi et cbi       - http://www.nongnu.org/avr-libc/user-manual/group__deprecated__items.html

//-----------------------------
// Types personnalisés
//-----------------------------

typedef enum {					// Interruptions disponibles
	T1_ISR_OVERFLOW					= 0,
	T1_ISR_COMP_A					= 1,
	T1_ISR_COMP_B					= 2,
	T1_ISR_COMP_C					= 3
} T1_isr;
typedef enum {					// Mode d'opération du timer
	T1_MODE_NORMAL					= ((0<<WGM13)|(0<<WGM12)|(0<<WGM11)|(0<<WGM10)),
	T1_MODE_PWM_PHASECORR_8B		= ((0<<WGM13)|(0<<WGM12)|(0<<WGM11)|(1<<WGM10)),
	T1_MODE_PWM_PHASECORR_9B		= ((0<<WGM13)|(0<<WGM12)|(1<<WGM11)|(0<<WGM10)),
	T1_MODE_PWM_PHASECORR_10B		= ((0<<WGM13)|(0<<WGM12)|(1<<WGM11)|(1<<WGM10)),
	T1_MODE_CTC						= ((0<<WGM13)|(1<<WGM12)|(0<<WGM11)|(0<<WGM10)),
	T1_MODE_PWM_FAST_8B				= ((0<<WGM13)|(1<<WGM12)|(0<<WGM11)|(1<<WGM10)),
	T1_MODE_PWM_FAST_9B				= ((0<<WGM13)|(1<<WGM12)|(1<<WGM11)|(0<<WGM10)),
	T1_MODE_PWM_FAST_10B			= ((0<<WGM13)|(1<<WGM12)|(1<<WGM11)|(1<<WGM10)),
	//T1_MODE_PWM_PHASEFREQCORR		= ((1<<WGM13)|(0<<WGM12)|(0<<WGM11)|(0<<WGM10)),
	T1_MODE_PWM_PHASEFREQCORR		= ((1<<WGM13)|(0<<WGM12)|(0<<WGM11)|(1<<WGM10)),
	//T1_MODE_PWM_PHASECORR			= ((1<<WGM13)|(0<<WGM12)|(1<<WGM11)|(0<<WGM10)),
	T1_MODE_PWM_PHASECORR			= ((1<<WGM13)|(0<<WGM12)|(1<<WGM11)|(1<<WGM10)),
	//T1_MODE_CTC					= ((1<<WGM13)|(1<<WGM12)|(0<<WGM11)|(0<<WGM10)),
	//T1_MODE_RESERVED				= ((1<<WGM13)|(1<<WGM12)|(0<<WGM11)|(1<<WGM10)),
	T1_MODE_PWM_FAST_ICR			= ((1<<WGM13)|(1<<WGM12)|(1<<WGM11)|(0<<WGM10)),
	T1_MODE_PWM_FAST				= ((1<<WGM13)|(1<<WGM12)|(1<<WGM11)|(1<<WGM10)),
} T1_mode_t;
typedef enum {					// Sélection de la source d'horloge
	T1_CLK_STOP						= ((0<<CS12)|(0<<CS11)|(0<<CS10)),
	T1_CLK_DIV1						= ((0<<CS12)|(0<<CS11)|(1<<CS10)),
	T1_CLK_DIV8						= ((0<<CS12)|(1<<CS11)|(0<<CS10)),
	T1_CLK_DIV64					= ((0<<CS12)|(1<<CS11)|(1<<CS10)),
	T1_CLK_DIV256					= ((1<<CS12)|(0<<CS11)|(0<<CS10)),
	T1_CLK_DIV1024					= ((1<<CS12)|(0<<CS11)|(1<<CS10)),
	T1_CLK_EXT_FALLING				= ((1<<CS12)|(1<<CS11)|(0<<CS10)),
	T1_CLK_EXT_RISING				= ((1<<CS12)|(1<<CS11)|(1<<CS10)),
} T1_presc;
typedef enum {					// Mode d'opération des Output Compare channels
	T1_OC_MODE_NORMAL				= 0,
	T1_OC_MODE_TOGGLE_ON_COMPARE	= 1,
	T1_OC_MODE_CLEAR_ON_COMPARE		= 2,
	T1_OC_MODE_SET_ON_COMPARE		= 3,
} T1_OC_mode_t;

//-----------------------------
// Prototypes
//-----------------------------

// Fonctions spécifiques au fonctionnement en mode PWM
void timer1pwm_init(T1_presc prescaler, T1_mode_t mode);

// Fonctions spécifiques au fonctionnement en mode CTC
void timer1ctc_init(unsigned long freq);
void timer1ctc_setfreq(unsigned long freq);

// Fonctions inlinées
// - Fonctions générales de paramétrage du timer
static inline void timer1_setmode(T1_mode_t mode) __attribute__((always_inline));
static inline void timer1_setpresc(T1_presc prescaler) __attribute__((always_inline));
static inline void timer1_compare_setmodeA(T1_OC_mode_t mode) __attribute__((always_inline));
static inline void timer1_compare_setmodeB(T1_OC_mode_t mode) __attribute__((always_inline));
static inline void timer1_compare_setmodeC(T1_OC_mode_t mode) __attribute__((always_inline));
static inline void timer1_compare_setcompA(unsigned int cmp) __attribute__((always_inline));
static inline void timer1_compare_setcompB(unsigned int cmp) __attribute__((always_inline));
static inline void timer1_compare_setcompC(unsigned int cmp) __attribute__((always_inline));
static inline void timer1_cleartcnt(void) __attribute__((always_inline));
static inline unsigned int timer1_gettcnt(void) __attribute__((always_inline));
// - Fonctions spécifiques au fonctionnement en mode PWM
static inline void timer1pwm_setperiod(unsigned int period) __attribute__((always_inline));
static inline void timer1pwm_enableA(void) __attribute__((always_inline));
static inline void timer1pwm_enableB(void) __attribute__((always_inline));
static inline void timer1pwm_enableC(void) __attribute__((always_inline));
static inline void timer1pwm_disableA(void) __attribute__((always_inline));
static inline void timer1pwm_disableB(void) __attribute__((always_inline));
static inline void timer1pwm_disableC(void) __attribute__((always_inline));
static inline void timer1pwm_setdutyA(unsigned int duty) __attribute__((always_inline));
static inline void timer1pwm_setdutyB(unsigned int duty) __attribute__((always_inline));
static inline void timer1pwm_setdutyC(unsigned int duty) __attribute__((always_inline));
// - Fonctions spécifiques au fonctionnement en mode CTC
static inline void timer1ctc_enableA(void) __attribute__((always_inline));
static inline void timer1ctc_enableB(void) __attribute__((always_inline));
static inline void timer1ctc_enableC(void) __attribute__((always_inline));
static inline void timer1ctc_disableA(void) __attribute__((always_inline));
static inline void timer1ctc_disableB(void) __attribute__((always_inline));
static inline void timer1ctc_disableC(void) __attribute__((always_inline));
static inline void timer1ctc_isr_enable(void (*func)(void)) __attribute__((always_inline));
static inline void timer1ctc_isr_disable(void) __attribute__((always_inline));
static inline unsigned char timer1ctc_isr_getcntA(void) __attribute__((always_inline));
static inline unsigned char timer1ctc_isr_getcntB(void) __attribute__((always_inline));
static inline unsigned char timer1ctc_isr_getcntC(void) __attribute__((always_inline));
static inline void timer1ctc_isr_clearcntA(void) __attribute__((always_inline));
static inline void timer1ctc_isr_clearcntB(void) __attribute__((always_inline));
static inline void timer1ctc_isr_clearcntC(void) __attribute__((always_inline));

//-----------------------------
// Fonctions inlinées
//-----------------------------

extern void (*timer1_isr[4])(void);
extern volatile unsigned char timer1_isr_cnt[4];

// Fonctions générales de paramétrage du timer
void timer1_setmode(T1_mode_t mode) {
	TCCR1A &= ~((1<<WGM11)|(1<<WGM10));
	TCCR1A |= ((unsigned char)mode & ((1<<WGM11)|(1<<WGM10)));
	TCCR1B &= ~((1<<WGM13)|(1<<WGM12));
	TCCR1B |= ((unsigned char)mode & ((1<<WGM13)|(1<<WGM12)));
}
void timer1_setpresc(T1_presc prescaler) {
	TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10));
	TCCR1B |= (unsigned char)prescaler;
}
void timer1_compare_setmodeA(T1_OC_mode_t mode) {
	TCCR1A &= ~((1<<COM1A1)|(1<<COM1A0));
	TCCR1A |= ((unsigned char)mode << COM1A0);
}
void timer1_compare_setmodeB(T1_OC_mode_t mode) {
	TCCR1A &= ~((1<<COM1B1)|(1<<COM1B0));
	TCCR1A |= ((unsigned char)mode << COM1B0);
}
void timer1_compare_setmodeC(T1_OC_mode_t mode) {
	TCCR1A &= ~((1<<COM1C1)|(1<<COM1C0));
	TCCR1A |= ((unsigned char)mode << COM1C0);
}
void timer1_compare_setcompA(unsigned int cmp) {
	OCR1A = cmp;
}
void timer1_compare_setcompB(unsigned int cmp) {
	OCR1B = cmp;
}
void timer1_compare_setcompC(unsigned int cmp) {
	OCR1C = cmp;
}
void timer1_cleartcnt(void) {
	TCNT1 = 0;
}
unsigned int timer1_gettcnt(void) {
	return TCNT1;
}

// Fonctions spécifiques au fonctionnement en mode PWM
void timer1pwm_setperiod(unsigned int period) {
	ICR1 = (period - 1);
}
void timer1pwm_enableA(void) {
	sbi(OC1A_DDR, OC1A_PIN);
}
void timer1pwm_enableB(void) {
	sbi(OC1B_DDR, OC1B_PIN);
}
void timer1pwm_enableC(void) {
	sbi(OC1C_DDR, OC1C_PIN);
}
void timer1pwm_disableA(void) {
	// La sortie revient d'elle-même à l'état bas grâce
	// à la configuration faite dans timer1pwm_init()
	cbi(OC1A_DDR, OC1A_PIN);
}
void timer1pwm_disableB(void) {
	// La sortie revient d'elle-même à l'état bas grâce
	// à la configuration faite dans timer1pwm_init()
	cbi(OC1B_DDR, OC1B_PIN);
}
void timer1pwm_disableC(void) {
	// La sortie revient d'elle-même à l'état bas grâce
	// à la configuration faite dans timer1pwm_init()
	cbi(OC1C_DDR, OC1C_PIN);
}
void timer1pwm_setdutyA(unsigned int duty) {
	OCR1A = duty;
}
void timer1pwm_setdutyB(unsigned int duty) {
	OCR1B = duty;
}
void timer1pwm_setdutyC(unsigned int duty) {
	OCR1C = duty;
}

// Fonctions spécifiques au fonctionnement en mode CTC
void timer1ctc_enableA(void) {
	sbi(OC1A_DDR, OC1A_PIN);
}
void timer1ctc_enableB(void) {
	sbi(OC1B_DDR, OC1B_PIN);
}
void timer1ctc_enableC(void) {
	sbi(OC1C_DDR, OC1C_PIN);
}
void timer1ctc_disableA(void) {
	// La sortie revient d'elle-même à l'état bas grâce
	// à la configuration faite dans timer1ctc_init()
	cbi(OC1A_DDR, OC1A_PIN);
}
void timer1ctc_disableB(void) {
	// La sortie revient d'elle-même à l'état bas grâce
	// à la configuration faite dans timer1ctc_init()
	cbi(OC1B_DDR, OC1B_PIN);
}
void timer1ctc_disableC(void) {
	// La sortie revient d'elle-même à l'état bas grâce
	// à la configuration faite dans timer1ctc_init()
	cbi(OC1C_DDR, OC1C_PIN);
}
void timer1ctc_isr_enable(void (*func)(void)) {
	timer1_isr[(unsigned char)T1_ISR_COMP_A] = func;
	sbi(TIFR1, OCF1A);			// Reset du flag d'interruption
	sbi(TIMSK1, OCIE1A);
}
void timer1ctc_isr_disable(void) {
	cbi(TIMSK1, OCIE1A);
}
unsigned char timer1ctc_isr_getcntA(void) {
	return timer1_isr_cnt[(unsigned char)T1_ISR_COMP_A];
}
unsigned char timer1ctc_isr_getcntB(void) {
	return timer1_isr_cnt[(unsigned char)T1_ISR_COMP_B];
}
unsigned char timer1ctc_isr_getcntC(void) {
	return timer1_isr_cnt[(unsigned char)T1_ISR_COMP_C];
}
void timer1ctc_isr_clearcntA(void) {
	timer1_isr_cnt[(unsigned char)T1_ISR_COMP_A] = 0;
}
void timer1ctc_isr_clearcntB(void) {
	timer1_isr_cnt[(unsigned char)T1_ISR_COMP_B] = 0;
}
void timer1ctc_isr_clearcntC(void) {
	timer1_isr_cnt[(unsigned char)T1_ISR_COMP_C] = 0;
}

#endif

/*
 * Balise d'émission IR doublement codée
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
 *
 * Le Timer 1 en mode Fast PWM crée une modulation à 455kHz (~presque)
 * L'UART crée une sur-modulation à 19200 baud avec toujours le même caractère
 * Le Timer 2 en mode CTC génère des interruptions toutes les 1.5ms pour envoyer
 * un nouveau caractère (on envoie un caractère, on attend, on renvoie, etc...
 * de sorte qu'il y ait une pause équivalente à la taille d'un caractère entre
 * chaque caractère, c'est important pour fiabiliser la démodulation multiplexée).
 *
 * Pour la porteuse à 455kHz, on utilise le mode Fast PWM :
 * - OC1A est mis à l'état haut quand TCNT1=0
 * - OC1A est mis à l'état bas quand TCNT1=OCR1
 * - TCNT1 est remis à zéro quand TCNT1=ICR1
 * => ICR1 = 35, OCR1 = 17
 * ==> 457.1kHz, 48.5%
 *
 * État analogique (rapport 1//2) de la batterie sur l'ADC0.
 * Comme c'est une LiPo, on coupe tout si on atteint le seuil critique,
 * soit environ 3V/élément LiPo
 *
 */

//-----------------------------
// Paramétrages divers
//-----------------------------

#include "../common/balise_protocol.h"

#define UART_TXBUF_SIZE			1
#define UART_RXBUF_SIZE			32

#define TIMER1_PWM_PERIOD		35
#define TIMER1_PWM_DUTY			17
#define TIMER2_CTC_FREQ			700		// 700 octets envoyés par seconde, soit un rapport émission/total = 44%

// Demi-périodes de clignotement de la led, unité = 1/700s
#define LED_NBMS_NORMAL			350		// 1Hz
#define LED_NBMS_WARNING		50		// 7Hz

// Seuils d'alerte de la batterie LiPo (rapport 1/2 : 1024 <=> 10V)
#define BATT_WARNING			666		// 6.5V
#define BATT_CRITICAL			614		// 6V

//-----------------------------
// Attribution des IO
//-----------------------------

#define LED_DDR					DDRD
#define LED_PORT				PORTD
#define LED_PIN					3

//-----------------------------
// Librairies
//-----------------------------

#include <avr/io.h>				// Mapping des registres   - http://www.nongnu.org/avr-libc/user-manual/group__avr__io.html
#include <avr/sleep.h>			// Mise en veille          - http://www.nongnu.org/avr-libc/user-manual/group__avr__sleep.html
#include <compat/deprecated.h>	// Macros sbi et cbi       - http://www.nongnu.org/avr-libc/user-manual/group__deprecated__items.html

//-----------------------------
// Librairies persos
//-----------------------------

#include "drivers/uart.h"
#include "drivers/timer1.h"
#include "drivers/timer2.h"
#include "drivers/adc.h"

//-----------------------------
// Variables globales
//-----------------------------

unsigned char uart_RxBuf_tab[UART_RXBUF_SIZE];
unsigned char uart_TxBuf_tab[UART_TXBUF_SIZE];

volatile unsigned char emission_flag = 0;
volatile unsigned int led_top = LED_NBMS_NORMAL;

//-----------------------------
// Prototypes
//-----------------------------

// Interruptions
void timer2_int_emission(void);
void timer0_int_10Hz(void);
void adc_batt_int(void);

//-----------------------------
// Interruptions
//-----------------------------

void timer2_int_emission(void) {
	static unsigned int led_count = 0;		// Demi-période de clignotement de la led

	// Envoi d'un caractère
	uart_putc_buffered(BALISE_CODE);

	// Gestion du clignotement plus ou moins rapide de la led
	led_count++;
	if (led_count >= led_top) {
		LED_PORT ^= (1 << LED_PIN);
		led_count = 0;
	}
}
void timer0_int_10Hz(void) {
	// Langement d'une nouvelle acquisition analogique de l'état de la batterie
	adc_run();
}
void adc_batt_int(void) {
	unsigned int meas = adc_getmeas16();
	if (meas < BATT_CRITICAL) {
		// Extinction globale, on ne plaisante pas avec les LiPo !
		cbi(LED_DDR, LED_PIN);
		timer1pwm_disableA();
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_mode();
	} else if (meas < BATT_WARNING) {
		// Clignotement accéléré de la led
		led_top = LED_NBMS_WARNING;
	} else {
		// Clignotement normal de la led
		led_top = LED_NBMS_NORMAL;
	}
}

//-----------------------------
// Fonction principale
//-----------------------------

int main (void) {

	cli();

	// Configuration de la led
	sbi(LED_DDR, LED_PIN);

	// Configuration de l'acquisition analogique-numérique
	adc_init(ADC_REF_AVCC, ADC_RIGHT_ADJUST, ADC_DIV128);
	adc_selectchan(ADC_CHAN_0);
	adc_isr_enable(adc_batt_int);

	// Configuration du port série
	uart_init(BALISE_BAUDRATE, BALISE_NBBITS, BALISE_PARITY, BALISE_NBSTOP, uart_RxBuf_tab, UART_RXBUF_SIZE, uart_TxBuf_tab, UART_TXBUF_SIZE);

	// Configuration du Timer 1 en mode Fast PWM pour une émission à 455kHz sur OC1A
	timer1pwm_init(T1_CLK_DIV1, T1_MODE_PWM_FAST_TOPICR);
	timer1pwm_setdutyA(TIMER1_PWM_DUTY);
	timer1pwm_setperiod(TIMER1_PWM_PERIOD);
	timer1pwm_enableA();

	// Configuration du Timer 2 en mode CTC pour générer des interruptions à 1kHz
	timer2ctc_init(TIMER2_CTC_FREQ);
	timer2ctc_isr_enable(timer2_int_emission);

	sei();

	while (1);

	return 0;
}

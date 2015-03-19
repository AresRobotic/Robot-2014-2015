/*
 * Balise d'émission IR doublement codée
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
 */

#ifndef DRIVER_ADC_H
#define DRIVER_ADC_H

//-----------------------------
// Types spéciaux
//-----------------------------

typedef enum {
	ADC_REF_AREF		= (0<<REFS1)|(0<<REFS0),
	ADC_REF_AVCC		= (0<<REFS1)|(1<<REFS0),
	ADC_REF_2V56		= (1<<REFS1)|(1<<REFS0)
} adc_ref_t;
typedef enum {
	ADC_RIGHT_ADJUST	= (0<<ADLAR),
	ADC_LEFT_ADJUST		= (1<<ADLAR)
} adc_mode_t;
typedef enum {
	//ADC_DIV1			= (0<<ADPS2)|(0<<ADPS1)|(0<<ADPS0),
	ADC_DIV2			= (0<<ADPS2)|(0<<ADPS1)|(1<<ADPS0),
	ADC_DIV4			= (0<<ADPS2)|(1<<ADPS1)|(0<<ADPS0),
	ADC_DIV8			= (0<<ADPS2)|(1<<ADPS1)|(1<<ADPS0),
	ADC_DIV16			= (1<<ADPS2)|(0<<ADPS1)|(0<<ADPS0),
	ADC_DIV32			= (1<<ADPS2)|(0<<ADPS1)|(1<<ADPS0),
	ADC_DIV64			= (1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0),
	ADC_DIV128			= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)
} adc_presc_t;
typedef enum {
	ADC_CHAN_0			= (0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0),
	ADC_CHAN_1			= (0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(1<<MUX0),
	ADC_CHAN_2			= (0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(0<<MUX0),
	ADC_CHAN_3			= (0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(1<<MUX0),
	ADC_CHAN_4			= (0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(0<<MUX0),
	ADC_CHAN_5			= (0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(1<<MUX0),
	ADC_CHAN_6			= (0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(0<<MUX0),
	ADC_CHAN_7			= (0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0),
	ADC_123V			= (1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(0<<MUX0),
	ADC_GND				= (1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)
} adc_chan_t;

//-----------------------------
// Prototypes
//-----------------------------

static inline void adc_init(adc_ref_t reference, adc_mode_t mode, adc_presc_t prescaler);
static inline void adc_selectchan(adc_chan_t channel);
static inline void adc_run(void) __attribute__((always_inline));
static inline void adc_isr_enable(void (*func)(void));
static inline void adc_isr_disable(void) __attribute__((always_inline));
static inline void adc_sleepmode(unsigned char mode);
static inline unsigned char adc_finished(void) __attribute__((always_inline));
static inline unsigned char adc_getmeas8(void) __attribute__((always_inline));
static inline unsigned int adc_getmeas16(void) __attribute__((always_inline));

//-----------------------------
// Fonctions inlinées
//-----------------------------

extern void (*adc_isr_func)(void);

void adc_init(adc_ref_t reference, adc_mode_t mode, adc_presc_t prescaler) {
	ADMUX = (unsigned char)reference | (unsigned char)mode;
	ADCSRA |= (1<<ADEN) | (unsigned char)prescaler;
}
void adc_selectchan(adc_chan_t channel) {
	ADMUX &= ~((1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0));
	ADMUX |= (unsigned char)channel;
}
void adc_run(void) {
	sbi(ADCSRA, ADSC);
}
void adc_isr_enable(void (*func)(void)) {
	adc_isr_func = func;
	sbi(ADCSRA, ADIF);		// Remise à zéro du flag d'interruption
	sbi(ADCSRA, ADIE);
}
void adc_isr_disable(void) {
	cbi(ADCSRA, ADIE);
}
void adc_sleepmode(unsigned char mode) {
	if (mode == 0)
		sbi(ADCSRA, ADEN);
	else
		cbi(ADCSRA, ADEN);
}
unsigned char adc_finished(void) {
	return bit_is_clear(ADCSRA, ADSC);
}
unsigned char adc_getmeas8(void) {
	return ADCH;
}
unsigned int adc_getmeas16(void) {
	return ADC;
}

#endif

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
#include <avr/interrupt.h>		// Vecteurs d'interruption - http://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html
#include <util/atomic.h>		// Garantie d'atomicité    - http://www.nongnu.org/avr-libc/user-manual/group__util__atomic.html
#include <compat/deprecated.h>	// Macros sbi et cbi       - http://www.nongnu.org/avr-libc/user-manual/group__deprecated__items.html

//-----------------------------
// Librairies persos
//-----------------------------

#include "../libs/buffer.h"		// Gestion générique d'un buffer FIFO circulaire
#include "uart0.h"

//-----------------------------
// Variables globales
//-----------------------------

volatile unsigned char uart0_RxOverflow;
volatile unsigned char uart0_TxReady;
buffer uart0_TxBuf;
buffer uart0_RxBuf;

//-----------------------------
// Fonctions
//-----------------------------

// Configuration initiale de l'UART et des buffers
void uart0_init(unsigned long baudrate, unsigned char nbbits, unsigned char parity, unsigned char stop, unsigned char * rxbuf_tab, unsigned char rxbuf_size, unsigned char * txbuf_tab, unsigned char txbuf_size) {

	unsigned char nbbits1, nbbits2;

	// Calcul de la valeur du registre de baudrate
	// Attention : la formule diffère si on utilise U2X=0, il faut remplacer 8 par 16
	unsigned long baudreg = (F_CPU/(baudrate*8L))-1;

	// On adapte les données de configuration pour obtenir les registres adaptés
	switch (parity) {
		case 'E': parity = (1<<UPM01)|(0<<UPM00); break;
		case 'O': parity = (1<<UPM01)|(1<<UPM00); break;
		case 'N':
		default:  parity = (0<<UPM01)|(0<<UPM00); break;
	}
	switch (stop) {
		case 2:  stop = (1<<USBS0); break;
		case 1:
		default: stop = (0<<USBS0); break;
	}
	switch (nbbits) {
		case 5:  nbbits1 = (0<<UCSZ01)|(0<<UCSZ00); nbbits2 = (0<<UCSZ02); break;
		case 6:  nbbits1 = (0<<UCSZ01)|(1<<UCSZ00); nbbits2 = (0<<UCSZ02); break;
		case 7:  nbbits1 = (1<<UCSZ01)|(0<<UCSZ00); nbbits2 = (0<<UCSZ02); break;
		case 9:  nbbits1 = (1<<UCSZ01)|(1<<UCSZ00); nbbits2 = (1<<UCSZ02); break;
		case 8:
		default: nbbits1 = (1<<UCSZ01)|(1<<UCSZ00); nbbits2 = (0<<UCSZ02); break;
	}

	// Configuration des IO (en principe c'est fait tout seul par le hardware, mais c'est une précaution de plus)
	// C'est aussi ce qui permet de ne pas causer de frame error pendant une mise en veille (garder TX à 5V)
	sbi(UART0_TX_DDR,  UART0_TX_PIN);
	cbi(UART0_RX_DDR,  UART0_RX_PIN);
	sbi(UART0_TX_PORT, UART0_TX_PIN);

	// Configuration des registres
	UCSR0A = (1<<U2X0);												// Doublement de la vitesse de transmission
	UCSR0B = (1<<RXCIE0)|(1<<TXCIE0)|(0<<UDRIE0)|(1<<RXEN0)|(1<<TXEN0)|nbbits2;	// Activation des interruptions sur TX, RX et UDRE
	UCSR0C = parity|stop|nbbits1;									// Transferts asymchrones
	UBRR0  = (unsigned int)baudreg;									// Configuration du baudrate

	// Configuration des buffers
	buffer_init(&uart0_RxBuf, rxbuf_tab, rxbuf_size);
	buffer_init(&uart0_TxBuf, txbuf_tab, txbuf_size);

	// Initialisation des flags
	uart0_RxOverflow = 0;
	uart0_TxReady = 1;
}
// Envoi de données, mais avec attente
void uart0_putc(unsigned char c) {
	while (!uart0_TxReady);
	if (c == '\n') uart0_putc('\r');
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		while (!(UCSR0A & (1<<UDRE0)));
		UDR0 = c;
		uart0_TxReady = 0;

		// On active l'interruption sur UDRE, qui se déclenchera lorsque le
		// buffer sera prêt à accepter une nouvelle donnée
		sbi(UCSR0B, UDRIE0);
	}
}
void uart0_puts(char * s) {
	while (*s) {
		uart0_putc((unsigned char)(*s));
		s++;
	}
}
// Envoi de données, mais avec gestion du buffer d'émission
void uart0_putc_buffered(unsigned char c) {
	while (!buffer_IsNotFull(&uart0_TxBuf));
	if (c == '\n') uart0_putc_buffered('\r');
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		buffer_AddToEnd(&uart0_TxBuf, c);
		uart0_TxReady = 0;
		// On active l'interruption sur UDRE, qui se déclenchera lorsque le
		// buffer sera prêt à accepter une nouvelle donnée
		sbi(UCSR0B, UDRIE0);
	}
}
void uart0_puts_buffered(char * s) {
	while (*s) {
		uart0_putc_buffered((unsigned char)(*s));
		s++;
	}
}

//-----------------------------
// Interruptions
//-----------------------------

ISR(USART0_RX_vect) {	// Interruption déclenchée sur réception d'un caractère
	unsigned char c = UDR0;
	if (!buffer_AddToEnd(&uart0_RxBuf, c))
		uart0_RxOverflow++;
}
ISR(USART0_UDRE_vect) {	// Interruption déclenchée lorsque le registre UDR peut accepter une nouvelle donnée (alors que celle d'avant est toujours en cours de transmission)
	if (buffer_GetLength(&uart0_TxBuf)) {
		// S'il reste des données dans le buffer, on relance une émission
		UDR0 = buffer_GetFromFront(&uart0_TxBuf);
	} else {
		// Sinon on désactive cette interruption, sinon on bouclera dessus !
		cbi(UCSR0B, UDRIE0);
	}
}
ISR(USART0_TX_vect) {	// Interruption déclenchée lorsque le buffer hardware (UDR) est vide et que la dernière transmission est achevée
	uart0_TxReady = 1;
}

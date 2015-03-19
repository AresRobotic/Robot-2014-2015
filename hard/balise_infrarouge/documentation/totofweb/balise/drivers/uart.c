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
#include <util/atomic.h>		// Garantie d'atomicité    - http://www.nongnu.org/avr-libc/user-manual/group__util__atomic.html
#include <compat/deprecated.h>	// Macros sbi et cbi       - http://www.nongnu.org/avr-libc/user-manual/group__deprecated__items.html

//-----------------------------
// Librairies persos
//-----------------------------

#include "../libs/buffer.h"		// Gestion générique d'un buffer FIFO circulaire
#include "uart.h"

//-----------------------------
// Variables globales
//-----------------------------

volatile unsigned char uart_RxOverflow;
volatile unsigned char uart_TxReady;
buffer uart_TxBuf;
buffer uart_RxBuf;

//-----------------------------
// Fonctions
//-----------------------------

// Configuration initiale de l'UART et des buffers
void uart_init(unsigned long baudrate, unsigned char nbbits, unsigned char parity, unsigned char stop, unsigned char * rxbuf_tab, unsigned char rxbuf_size, unsigned char * txbuf_tab, unsigned char txbuf_size) {

	unsigned char nbbits1, nbbits2;

	// Calcul de la valeur du registre de baudrate
	// Attention : la formule diffère si on utilise U2X=0, il faut remplacer 8 par 16
	unsigned long baudreg = (F_CPU/(baudrate*8L))-1;

	// On adapte les données de configuration pour obtenir les registres adaptés
	switch (parity) {
		case 'E': parity = (1<<UPM1)|(0<<UPM0); break;
		case 'O': parity = (1<<UPM1)|(1<<UPM0); break;
		case 'N':
		default:  parity = (0<<UPM1)|(0<<UPM0); break;
	}
	switch (stop) {
		case 2:  stop = (1<<USBS); break;
		case 1:
		default: stop = (0<<USBS); break;
	}
	switch (nbbits) {
		case 5:  nbbits1 = (0<<UCSZ1)|(0<<UCSZ0); nbbits2 = (0<<UCSZ2); break;
		case 6:  nbbits1 = (0<<UCSZ1)|(1<<UCSZ0); nbbits2 = (0<<UCSZ2); break;
		case 7:  nbbits1 = (1<<UCSZ1)|(0<<UCSZ0); nbbits2 = (0<<UCSZ2); break;
		case 9:  nbbits1 = (1<<UCSZ1)|(1<<UCSZ0); nbbits2 = (1<<UCSZ2); break;
		case 8:
		default: nbbits1 = (1<<UCSZ1)|(1<<UCSZ0); nbbits2 = (0<<UCSZ2); break;
	}

	// Configuration des IO (en principe c'est fait tout seul par le hardware, mais c'est une précaution de plus)
	// C'est aussi ce qui permet de ne pas causer de frame error pendant une mise en veille (garder TX à 5V)
	sbi(UART_TX_DDR,  UART_TX_PIN);
	cbi(UART_RX_DDR,  UART_RX_PIN);
	sbi(UART_TX_PORT, UART_TX_PIN);

	// Configuration des registres
	UCSRA = (1<<U2X);												// Doublement de la vitesse de transmission
	UCSRB = (1<<RXCIE)|(1<<TXCIE)|(0<<UDRIE)|(1<<RXEN)|(1<<TXEN)|nbbits2;	// Activation des interruptions sur TX, RX et UDRE
	UCSRC = (1<<URSEL)|parity|stop|nbbits1;							// Transferts asymchrones
	UBRRH = (unsigned char)(baudreg >> 8);							// Configuration du baudrate
	UBRRL = (unsigned char)baudreg;									// Configuration du baudrate

	// Configuration des buffers
	buffer_init(&uart_RxBuf, rxbuf_tab, rxbuf_size);
	buffer_init(&uart_TxBuf, txbuf_tab, txbuf_size);

	// Initialisation des flags
	uart_RxOverflow = 0;
	uart_TxReady = 1;
}
// Envoi de données, mais avec attente
void uart_putc(unsigned char c) {
	while (!uart_TxReady);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		while (!(UCSRA & (1<<UDRE)));
		UDR = c;
		uart_TxReady = 0;

		// On active l'interruption sur UDRE, qui se déclenchera lorsque le
		// buffer sera prêt à accepter une nouvelle donnée
		sbi(UCSRB, UDRIE);
	}
}
void uart_puts(char * s) {
	while (*s) {
		uart_putc((unsigned char)(*s));
		s++;
	}
}
// Envoi de données, mais avec gestion du buffer d'émission
void uart_putc_buffered(unsigned char c) {
	while (!buffer_IsNotFull(&uart_TxBuf));
	if (c == '\n') uart_putc_buffered('\r');
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		buffer_AddToEnd(&uart_TxBuf, c);
		uart_TxReady = 0;
		// On active l'interruption sur UDRE, qui se déclenchera lorsque le
		// buffer sera prêt à accepter une nouvelle donnée
		sbi(UCSRB, UDRIE);
	}
}
void uart_puts_buffered(char * s) {
	while (*s) {
		uart_putc_buffered((unsigned char)(*s));
		s++;
	}
}

//-----------------------------
// Interruptions
//-----------------------------

ISR(USART_RXC_vect) {	// Interruption déclenchée sur réception d'un caractère
	unsigned char c = UDR;
	if (!buffer_AddToEnd(&uart_RxBuf, c))
		uart_RxOverflow++;
}
ISR(USART_UDRE_vect) {	// Interruption déclenchée lorsque le registre UDR peut accepter une nouvelle donnée (alors que celle d'avant est toujours en cours de transmission)
	if (buffer_GetLength(&uart_TxBuf)) {
		// S'il reste des données dans le buffer, on relance une émission
		UDR = buffer_GetFromFront(&uart_TxBuf);
	} else {
		// Sinon on désactive cette interruption, sinon on bouclera dessus !
		cbi(UCSRB, UDRIE);
	}
}
ISR(USART_TXC_vect) {	// Interruption déclenchée lorsque le buffer hardware (UDR) est vide et que la dernière transmission est achevée
	uart_TxReady = 1;
}

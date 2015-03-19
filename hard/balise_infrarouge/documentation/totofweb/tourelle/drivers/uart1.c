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
#include "uart1.h"

//-----------------------------
// Variables globales
//-----------------------------

volatile unsigned char uart1_RxOverflow;
volatile unsigned char uart1_TxReady;
buffer uart1_TxBuf;
buffer uart1_RxBuf;

//-----------------------------
// Fonctions
//-----------------------------

// Configuration initiale de l'UART et des buffers
void uart1_init(unsigned long baudrate, unsigned char nbbits, unsigned char parity, unsigned char stop, unsigned char * rxbuf_tab, unsigned char rxbuf_size, unsigned char * txbuf_tab, unsigned char txbuf_size) {

	unsigned char nbbits1, nbbits2;

	// Calcul de la valeur du registre de baudrate
	// Attention : la formule diffère si on utilise U2X=0, il faut remplacer 8 par 16
	unsigned long baudreg = (F_CPU/(baudrate*8L))-1;

	// On adapte les données de configuration pour obtenir les registres adaptés
	switch (parity) {
		case 'E': parity = (1<<UPM11)|(0<<UPM10); break;
		case 'O': parity = (1<<UPM11)|(1<<UPM10); break;
		case 'N':
		default:  parity = (0<<UPM11)|(0<<UPM10); break;
	}
	switch (stop) {
		case 2:  stop = (1<<USBS1); break;
		case 1:
		default: stop = (0<<USBS1); break;
	}
	switch (nbbits) {
		case 5:  nbbits1 = (0<<UCSZ11)|(0<<UCSZ10); nbbits2 = (0<<UCSZ12); break;
		case 6:  nbbits1 = (0<<UCSZ11)|(1<<UCSZ10); nbbits2 = (0<<UCSZ12); break;
		case 7:  nbbits1 = (1<<UCSZ11)|(0<<UCSZ10); nbbits2 = (0<<UCSZ12); break;
		case 9:  nbbits1 = (1<<UCSZ11)|(1<<UCSZ10); nbbits2 = (1<<UCSZ12); break;
		case 8:
		default: nbbits1 = (1<<UCSZ11)|(1<<UCSZ10); nbbits2 = (0<<UCSZ12); break;
	}

	// Configuration des IO (en principe c'est fait tout seul par le hardware, mais c'est une précaution de plus)
	// C'est aussi ce qui permet de ne pas causer de frame error pendant une mise en veille (garder TX à 5V)
	sbi(UART1_TX_DDR,  UART1_TX_PIN);
	cbi(UART1_RX_DDR,  UART1_RX_PIN);
	sbi(UART1_TX_PORT, UART1_TX_PIN);

	// Configuration des registres
	UCSR1A = (1<<U2X1);												// Doublement de la vitesse de transmission
	UCSR1B = (1<<RXCIE1)|(1<<TXCIE1)|(0<<UDRIE1)|(1<<RXEN1)|(1<<TXEN1)|nbbits2;	// Activation des interruptions sur TX, RX et UDRE
	UCSR1C = parity|stop|nbbits1;									// Transferts asymchrones
	UBRR1  = (unsigned int)baudreg;									// Configuration du baudrate

	// Configuration des buffers
	buffer_init(&uart1_RxBuf, rxbuf_tab, rxbuf_size);
	buffer_init(&uart1_TxBuf, txbuf_tab, txbuf_size);

	// Initialisation des flags
	uart1_RxOverflow = 0;
	uart1_TxReady = 1;
}
// Envoi de données, mais avec attente
void uart1_putc(unsigned char c) {
	while (!uart1_TxReady);
	if (c == '\n') uart1_putc('\r');
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		while (!(UCSR1A & (1<<UDRE1)));
		UDR1 = c;
		uart1_TxReady = 0;

		// On active l'interruption sur UDRE, qui se déclenchera lorsque le
		// buffer sera prêt à accepter une nouvelle donnée
		sbi(UCSR1B, UDRIE1);
	}
}
void uart1_puts(char * s) {
	while (*s) {
		uart1_putc((unsigned char)(*s));
		s++;
	}
}
// Envoi de données, mais avec gestion du buffer d'émission
void uart1_putc_buffered(unsigned char c) {
	while (!buffer_IsNotFull(&uart1_TxBuf));
	if (c == '\n') uart1_putc_buffered('\r');
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		buffer_AddToEnd(&uart1_TxBuf, c);
		uart1_TxReady = 0;
		// On active l'interruption sur UDRE, qui se déclenchera lorsque le
		// buffer sera prêt à accepter une nouvelle donnée
		sbi(UCSR1B, UDRIE1);
	}
}
void uart1_puts_buffered(char * s) {
	while (*s) {
		uart1_putc_buffered((unsigned char)(*s));
		s++;
	}
}

//-----------------------------
// Interruptions
//-----------------------------

ISR(USART1_RX_vect) {	// Interruption déclenchée sur réception d'un caractère
	unsigned char c = UDR1;
	if (!buffer_AddToEnd(&uart1_RxBuf, c))
		uart1_RxOverflow++;
}
ISR(USART1_UDRE_vect) {	// Interruption déclenchée lorsque le registre UDR peut accepter une nouvelle donnée (alors que celle d'avant est toujours en cours de transmission)
	if (buffer_GetLength(&uart1_TxBuf)) {
		// S'il reste des données dans le buffer, on relance une émission
		UDR1 = buffer_GetFromFront(&uart1_TxBuf);
	} else {
		// Sinon on désactive cette interruption, sinon on bouclera dessus !
		cbi(UCSR1B, UDRIE1);
	}
}
ISR(USART1_TX_vect) {	// Interruption déclenchée lorsque le buffer hardware (UDR) est vide et que la dernière transmission est achevée
	uart1_TxReady = 1;
}

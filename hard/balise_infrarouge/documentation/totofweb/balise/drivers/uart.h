/*
 * Balise d'émission IR doublement codée
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
 */

#ifndef DRIVER_UART_H
#define DRIVER_UART_H

//-----------------------------
// Utilisation des ports
//-----------------------------

#define UART_TX_DDR				DDRD
#define UART_TX_PORT			PORTD
#define UART_TX_PIN				1
#define UART_RX_DDR				DDRD
#define UART_RX_PIN				0

//-----------------------------
// Prototypes
//-----------------------------

void uart_init(unsigned long baudrate, unsigned char nbbits, unsigned char parity, unsigned char stop, unsigned char * rxbuf_tab, unsigned char rxbuf_size, unsigned char * txbuf_tab, unsigned char txbuf_size);
void uart_putc(unsigned char c);
void uart_puts(char * s);
void uart_putc_buffered(unsigned char c);
void uart_puts_buffered(char * s);
static inline unsigned char uart_RxBuf_GetFromFront(void) __attribute__((always_inline));
static inline void uart_RxBuf_DumpFromFront(unsigned char nb) __attribute__((always_inline));
static inline unsigned char uart_RxBuf_GetAtIndex(unsigned char index) __attribute__((always_inline));
static inline unsigned char uart_RxBuf_GetLength(void) __attribute__((always_inline));
static inline void uart_RxBuf_Flush(void) __attribute__((always_inline));
static inline unsigned char uart_TxBusy(void) __attribute__((always_inline));

//-----------------------------
// Fonctions inlinées
//-----------------------------

#include "../libs/buffer.h"		// Gestion générique d'un buffer FIFO circulaire
extern buffer uart_RxBuf;
extern volatile unsigned char uart_TxReady;

// On laisse l'accès au buffer de réception, mais seulement à certaines fonctions
unsigned char uart_RxBuf_GetFromFront(void) { return buffer_GetFromFront(&uart_RxBuf); }
void uart_RxBuf_DumpFromFront(unsigned char nb) { buffer_DumpFromFront(&uart_RxBuf, nb); }
unsigned char uart_RxBuf_GetAtIndex(unsigned char index) { return buffer_GetAtIndex(&uart_RxBuf, index); }
unsigned char uart_RxBuf_GetLength(void) { return buffer_GetLength(&uart_RxBuf); }
void uart_RxBuf_Flush(void) { buffer_Flush(&uart_RxBuf); }

unsigned char uart_TxBusy(void) { return (!uart_TxReady); }

#endif

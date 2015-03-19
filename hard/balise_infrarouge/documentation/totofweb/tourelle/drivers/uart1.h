/*
 * Carte de réception de balise
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
 */

#ifndef DRIVER_UART1_H
#define DRIVER_UART1_H

//-----------------------------
// Utilisation des ports
//-----------------------------

#define UART1_TX_DDR			DDRD
#define UART1_TX_PORT			PORTD
#define UART1_TX_PIN			3
#define UART1_RX_DDR			DDRD
#define UART1_RX_PIN			2

//-----------------------------
// Prototypes
//-----------------------------

void uart1_init(unsigned long baudrate, unsigned char nbbits, unsigned char parity, unsigned char stop, unsigned char * rxbuf_tab, unsigned char rxbuf_size, unsigned char * txbuf_tab, unsigned char txbuf_size);
void uart1_putc(unsigned char c);
void uart1_puts(char * s);
void uart1_putc_buffered(unsigned char c);
void uart1_puts_buffered(char * s);
static inline unsigned char uart1_RxBuf_GetFromFront(void) __attribute__((always_inline));
static inline void uart1_RxBuf_DumpFromFront(unsigned char nb) __attribute__((always_inline));
static inline unsigned char uart1_RxBuf_GetAtIndex(unsigned char index) __attribute__((always_inline));
static inline unsigned char uart1_RxBuf_GetLength(void) __attribute__((always_inline));
static inline void uart1_RxBuf_Flush(void) __attribute__((always_inline));
static inline unsigned char uart1_TxBusy(void) __attribute__((always_inline));

//-----------------------------
// Fonctions inlinées
//-----------------------------

#include "../libs/buffer.h"		// Gestion générique d'un buffer FIFO circulaire
extern buffer uart1_RxBuf;
extern volatile unsigned char uart1_TxReady;

// On laisse l'accès au buffer de réception, mais seulement à certaines fonctions
unsigned char uart1_RxBuf_GetFromFront(void) { return buffer_GetFromFront(&uart1_RxBuf); }
void uart1_RxBuf_DumpFromFront(unsigned char nb) { buffer_DumpFromFront(&uart1_RxBuf, nb); }
unsigned char uart1_RxBuf_GetAtIndex(unsigned char index) { return buffer_GetAtIndex(&uart1_RxBuf, index); }
unsigned char uart1_RxBuf_GetLength(void) { return buffer_GetLength(&uart1_RxBuf); }
void uart1_RxBuf_Flush(void) { buffer_Flush(&uart1_RxBuf); }

unsigned char uart1_TxBusy(void) { return (!uart1_TxReady); }

#endif

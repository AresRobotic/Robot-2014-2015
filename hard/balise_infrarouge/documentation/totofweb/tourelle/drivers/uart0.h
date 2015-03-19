/*
 * Carte de réception de balise
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
 */

#ifndef DRIVER_UART0_H
#define DRIVER_UART0_H

//-----------------------------
// Utilisation des ports
//-----------------------------

#define UART0_TX_DDR			DDRE
#define UART0_TX_PORT			PORTE
#define UART0_TX_PIN			1
#define UART0_RX_DDR			DDRE
#define UART0_RX_PIN			0

//-----------------------------
// Prototypes
//-----------------------------

void uart0_init(unsigned long baudrate, unsigned char nbbits, unsigned char parity, unsigned char stop, unsigned char * rxbuf_tab, unsigned char rxbuf_size, unsigned char * txbuf_tab, unsigned char txbuf_size);
void uart0_putc(unsigned char c);
void uart0_puts(char * s);
void uart0_putc_buffered(unsigned char c);
void uart0_puts_buffered(char * s);
static inline unsigned char uart0_RxBuf_GetFromFront(void) __attribute__((always_inline));
static inline void uart0_RxBuf_DumpFromFront(unsigned char nb) __attribute__((always_inline));
static inline unsigned char uart0_RxBuf_GetAtIndex(unsigned char index) __attribute__((always_inline));
static inline unsigned char uart0_RxBuf_GetLength(void) __attribute__((always_inline));
static inline void uart0_RxBuf_Flush(void) __attribute__((always_inline));
static inline unsigned char uart0_TxBusy(void) __attribute__((always_inline));

//-----------------------------
// Fonctions inlinées
//-----------------------------

#include "../libs/buffer.h"		// Gestion générique d'un buffer FIFO circulaire
extern buffer uart0_RxBuf;
extern volatile unsigned char uart0_TxReady;

// On laisse l'accès au buffer de réception, mais seulement à certaines fonctions
unsigned char uart0_RxBuf_GetFromFront(void) { return buffer_GetFromFront(&uart0_RxBuf); }
void uart0_RxBuf_DumpFromFront(unsigned char nb) { buffer_DumpFromFront(&uart0_RxBuf, nb); }
unsigned char uart0_RxBuf_GetAtIndex(unsigned char index) { return buffer_GetAtIndex(&uart0_RxBuf, index); }
unsigned char uart0_RxBuf_GetLength(void) { return buffer_GetLength(&uart0_RxBuf); }
void uart0_RxBuf_Flush(void) { buffer_Flush(&uart0_RxBuf); }

unsigned char uart0_TxBusy(void) { return (!uart0_TxReady); }

#endif

/*
 * Balise d'émission IR doublement codée
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
 */

#ifndef LIB_BUFFER_H
#define LIB_BUFFER_H

//-----------------------------
// Librairies
//-----------------------------

#include <util/atomic.h>			// Garantie d'atomicité    - http://www.nongnu.org/avr-libc/user-manual/group__util__atomic.html

//-----------------------------
// Types spéciaux
//-----------------------------

// Définition du type de données "buffer"
typedef struct {
	unsigned char * dataptr;
	unsigned char size;
	volatile unsigned char datalength;
	volatile unsigned char dataindex;
} buffer;

//-----------------------------
// Prototypes
//-----------------------------

void buffer_init(buffer * buf, unsigned char * start, unsigned char size);
unsigned char buffer_GetFromFront(buffer * buf);
void buffer_DumpFromFront(buffer * buf, unsigned char nb);
static inline unsigned char buffer_GetAtIndex(buffer * buf, unsigned char index) __attribute__((always_inline));
unsigned char buffer_AddToEnd(buffer * buf, unsigned char data);
static inline unsigned char buffer_GetLength(buffer * buf) __attribute__((always_inline));
static inline unsigned char buffer_IsNotFull(buffer * buf) __attribute__((always_inline));
static inline void buffer_Flush(buffer * buf) __attribute__((always_inline));

//-----------------------------
// Fonctions inlinées
//-----------------------------

// Lecture (sans suppression) à un index quelconque
unsigned char buffer_GetAtIndex(buffer * buf, unsigned char index) {
	unsigned char data;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		data = buf->dataptr[(buf->dataindex+index)%(buf->size)];
	}
	return data;
}
// Quelle est la quantité de données dans le buffer ?
unsigned char buffer_GetLength(buffer * buf) {
	// Variable 8 bits, pas besoin de bloquer les interruptions
	//unsigned char length = buf->datalength;
	//return length;
	unsigned char length;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		length = buf->datalength;
	}
	return length;
}
// Le buffer est-il plein (renvoie la place disponible)
unsigned char buffer_IsNotFull(buffer * buf) {
	unsigned char freespace;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		freespace = (buf->size - buf->datalength);
	}
	return freespace;
}
// Suppression du contenu du buffer
void buffer_Flush(buffer * buf) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		buf->datalength = 0;
	}
}

#endif

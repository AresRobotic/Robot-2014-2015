/*
 * Balise d'émission IR doublement codée
 *
 * Christophe Le Lann <contact@totofweb.net>
 * http://www.totofweb.net/robots-projet-67.html
 */

/*
 *
 * Il est nécessaire de garantir l'atomicité du code dans
 * quelques parties critiques, pour éviter qu'une interruption
 * vienne corrompre les données alors que l'on est déjà en
 * train de modifier le buffer.
 *
 */

//-----------------------------
// Librairies
//-----------------------------

#include <util/atomic.h>			// Garantie d'atomicité    - http://www.nongnu.org/avr-libc/user-manual/group__util__atomic.html

//-----------------------------
// Librairies persos
//-----------------------------

#include "buffer.h"

//-----------------------------
// Fonctions
//-----------------------------

// Initialisation d'un buffer en spécifiant une zone de mémoire dédiée
void buffer_init(buffer * buf, unsigned char * start, unsigned char size) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		buf->dataptr = start;
		buf->size = size;
		buf->dataindex = 0;
		buf->datalength = 0;
	}
}
// Extraction du prochain octet, 0 si pas d'octet
unsigned char buffer_GetFromFront(buffer * buf) {
	unsigned char data = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (buf->datalength > 0) {
			data = buf->dataptr[buf->dataindex];
			buf->dataindex++;
			if (buf->dataindex >= buf->size)
				buf->dataindex -= buf->size;
			buf->datalength--;
		}
	}
	return data;
}
// Suppression des n prochains octets
void buffer_DumpFromFront(buffer * buf, unsigned char nb) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (nb < buf->datalength) {
			buf->dataindex += nb;
			if (buf->dataindex >= buf->size)
				buf->dataindex -= buf->size;
			buf->datalength -= nb;
		} else {
			buf->datalength = 0;
		}
	}
}
// Ajout d'un octet en queue de buffer
unsigned char buffer_AddToEnd(buffer * buf, unsigned char data) {
	if (buf->datalength < buf->size) {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			buf->dataptr[(buf->dataindex+buf->datalength)%(buf->size)] = data;
			buf->datalength++;
		}
		return 1;
	}
	// Echec, buffer plein
	return 0;
}

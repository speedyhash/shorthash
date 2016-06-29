
#ifndef SHORTHASH_TABULATED_H
#define SHORTHASH_TABULATED_H

#include <stdint.h>
#include <limits.h>
#include "util.h"

typedef struct zobrist_s {
  uint64_t hashtab[sizeof(uint64_t)][1 << CHAR_BIT] ;
} zobrist_t;

void zobrist_init(zobrist_t * k) {
	for ( uint32_t i = 0 ; i < sizeof(uint64_t) ; i++ ) {
		for ( uint32_t j = 0 ; j < ( 1 << CHAR_BIT) ; j++ ) {
			k->hashtab [i][j]  = get64rand();
		}
  }
}


uint64_t zobrist(uint64_t val, zobrist_t * k) {
  uint64_t h = 0;
  const unsigned char *s = (const unsigned char *) &val;
  h ^= k->hashtab [ 0 ] [s[0]];
  h ^= k->hashtab [ 1 ] [s[1]];
  h ^= k->hashtab [ 2 ] [s[2]];
  h ^= k->hashtab [ 3 ] [s[3]];
  h ^= k->hashtab [ 4 ] [s[4]];
  h ^= k->hashtab [ 5 ] [s[5]];
  h ^= k->hashtab [ 6 ] [s[6]];
  h ^= k->hashtab [ 7 ] [s[7]];
  return h;
}


#endif

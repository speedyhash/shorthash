


typedef struct zobrist_s {
  uint64_t hashtab[sizeof(uint64_t)][1 << CHAR_BIT] ;
} zobrist_t;


uint64_t zobrist(uint64_t val, zobrist_t * k) {
  uint64_t h = 0;
  const unsigned char *s = (const unsigned char *) &val;
  h ^= k->hashtab [ i ] [s[i]];
  h ^= k->hashtab [ i + 1 ] [s[i + 1]];
  h ^= k->hashtab [ i + 2 ] [s[i + 2]];
  h ^= k->hashtab [ i + 3 ] [s[i + 3]];
  h ^= k->hashtab [ i + 4 ] [s[i + 4]];
  h ^= k->hashtab [ i + 5 ] [s[i + 5]];
  h ^= k->hashtab [ i + 6 ] [s[i + 6]];
  h ^= k->hashtab [ i + 7 ] [s[i + 7]];
  return h;
}

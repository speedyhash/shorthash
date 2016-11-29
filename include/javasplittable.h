#ifndef JAVASPLIT_H
#define JAVASPLIT_H

/**
* Using Java Splittable counter-based RNG, see Vigna's
* http://xoroshiro.di.unimi.it/splitmix64.c
*
* If you use this to hash 1,2,3... you get pseudo-random numbers
* that pass the BigCrush test.
*
*  References:
* Parallel Random Numbers: As Easy as 1, 2, 3
* www.thesalmons.org/john/random123/papers/random123sc11.pdf
*
* Fast splittable pseudorandom number generators
* http://dx.doi.org/10.1145/2714064.2660195
*/
typedef struct {
  int shift1;
  int shift2;
  int shift3;
  uint64_t multiplier1;
  uint64_t multiplier2;
  uint64_t multiplier3;

} javasplit64_t;

void javasplit64_init(javasplit64_t *key) {
  key->shift1 = 30;
  key->shift2 = 27;
  key->shift3 = 31;
  key->multiplier1 = UINT64_C(0x9E3779B97F4A7C15);
  key->multiplier2 = UINT64_C(0xBF58476D1CE4E5B9);
  key->multiplier3 = UINT64_C(0x94D049BB133111EB);
}

__attribute__((always_inline))
inline uint64_t javasplit64(uint64_t h, const javasplit64_t * key) {
	uint64_t z = (key->multiplier1 * h);
	z = (z ^ (z >> key->shift1)) * key->multiplier2;
	z = (z ^ (z >> key->shift2)) * key->multiplier3;
	return z ^ (z >> key->shift3);
}
#endif // JAVASPLIT

#include "cmwc.hh"
#include <stdlib.h>

static
uint32_t rand32(void) {
   uint32_t result = rand();
   return result << 16 | rand();
}

void cmwc::seed(unsigned int seed) {
   srand(seed);        
   for (int i = 0; i < CMWC_CYCLE; i++)
      this->Q[i] = rand32();
   do
      this->c = rand32();
   while (this->c >= CMWC_C_MAX);
   this->i = CMWC_CYCLE - 1;
}

uint32_t cmwc::next() {
   uint64_t const a = 18782;	  // as Marsaglia recommends
   uint32_t const m = 0xfffffffe; // as Marsaglia recommends
   uint64_t t;
   uint32_t x;

   this->i = (this->i + 1) & (CMWC_CYCLE - 1);
   t = a * this->Q[this->i] + this->c;
   /* Let c = t / 0xfffffff, x = t mod 0xffffffff */
   this->c = t >> 32;
   x = t + this->c;
   if (x < this->c) {
      x++;
      this->c++;
   }
   return this->Q[this->i] = m - x;
}

#pragma once
#include <cstdint>
#include "common.hpp"


namespace rack {
namespace random {


/** Seeds the RNG with the current time */
void init();
/** Returns a uniform random uint32_t from 0 to UINT32_MAX */
uint32_t u32();
uint64_t u64();
/** Returns a uniform random float in the interval [0.0, 1.0) */
float uniform();
/** Returns a normal random number with mean 0 and standard deviation 1 */
float normal();


} // namespace random


DEPRECATED inline float randomu32() {return random::u32();}
DEPRECATED inline float randomu64() {return random::u64();}
DEPRECATED inline float randomUniform() {return random::uniform();}
DEPRECATED inline float randomNormal() {return random::normal();}
DEPRECATED inline float randomf() {return random::uniform();}


} // namespace rack

#pragma once
#include "common.hpp"
#include <cstdint>


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
} // namespace rack

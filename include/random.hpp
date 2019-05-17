#pragma once
#include "common.hpp"
#include <cstdint>


namespace rack {


/** Random number generator
*/
namespace random {


/** Initializes the thread-local RNG state.
Must call per-thread, otherwise the RNG will always return 0.
*/
void init();
/** Returns a uniform random uint32_t from 0 to UINT32_MAX */
uint32_t u32();
/** Returns a uniform random uint64_t from 0 to UINT64_MAX */
uint64_t u64();
/** Returns a uniform random float in the interval [0.0, 1.0) */
float uniform();
/** Returns a normal random number with mean 0 and standard deviation 1 */
float normal();


} // namespace random
} // namespace rack

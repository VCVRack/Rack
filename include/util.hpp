#pragma once

// Include most of the C standard library for convenience
// (C++ programmers will hate me)
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <string>


namespace rack {


////////////////////
// RNG
////////////////////

uint32_t randomu32();
/** Returns a uniform random float in the interval [0.0, 1.0) */
float randomf();
/** Returns a normal random number with mean 0 and std dev 1 */
float randomNormal();

////////////////////
// Helper functions
////////////////////

/** Converts a printf format string and optional arguments into a std::string */
std::string stringf(const char *format, ...);

/** Truncates and adds "..." to a string, not exceeding `len` characters */
std::string ellipsize(std::string s, size_t len);

} // namespace rack

#pragma once

#include "AudioMath.h"

#include <assert.h>
#include <iostream>

/**
 * Our own little assert library, loosely inspired by Chai Assert.
 *
 * Will print information on failure, then generate a "real" assertion
 */


#define assertEQEx(actual, expected, msg) if (actual != expected) { \
    std::cout << "assertEq failed " << msg << " actual value =" << \
    actual << " expected=" << expected << std::endl; \
    assert(false); }

#define assertEQ(actual, expected) assertEQEx(actual, expected, "")

#define assertNEEx(actual, expected, msg) if (actual == expected) { \
    std::cout << "assertNE failed " << msg << " did not expect " << \
    actual << " to be == to " << expected << std::endl; \
    assert(false); }

#define assertNE(actual, expected) assertNEEx(actual, expected, "")

#define assertClose(actual, expected, diff) if (!AudioMath::closeTo(actual, expected, diff)) { \
    std::cout << "assertClose failed actual value =" << \
    actual << " expected=" << expected << std::endl << std::flush; \
    assert(false); }


// assert less than
#define assertLT(actual, expected) if ( actual >= expected) { \
    std::cout << "assertLt " << expected << " actual value = " << \
    actual << std::endl ; \
    assert(false); }

// assert less than or equal to
#define assertLE(actual, expected) if ( actual > expected) { \
    std::cout << "assertLE " << expected << " actual value = " << \
    actual << std::endl ; \
    assert(false); }

// assert greater than 
#define assertGT(actual, expected) if ( actual <= expected) { \
    std::cout << "assertGT " << expected << " actual value = " << \
    actual << std::endl ; \
    assert(false); }
// assert greater than or equal to
#define assertGE(actual, expected) if ( actual < expected) { \
    std::cout << "assertGE " << expected << " actual value = " << \
    actual << std::endl ; \
    assert(false); }
// leave space after macro
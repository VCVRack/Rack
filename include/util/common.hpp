#pragma once

// Include most of the C++ standard library for convenience
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <climits>

#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>

#include "macros.hpp"
#include "math.hpp"
#include "string.hpp"
#include "logger.hpp"
#include "system.hpp"


namespace rack {

////////////////////
// Template hacks
////////////////////

/** C#-style property constructor
Example:
	Foo *foo = construct<Foo>(&Foo::greeting, "Hello world");
*/
template<typename T>
T *construct() {
	return new T();
}

template<typename T, typename F, typename V, typename... Args>
T *construct(F f, V v, Args... args) {
	T *o = construct<T>(args...);
	o->*f = v;
	return o;
}

/** Defers code until the scope is destructed
From http://www.gingerbill.org/article/defer-in-cpp.html

Example:
	file = fopen(...);
	defer({
		fclose(file);
	});
*/
template<typename F>
struct DeferWrapper {
	F f;
	DeferWrapper(F f) : f(f) {}
	~DeferWrapper() { f(); }
};

template<typename F>
DeferWrapper<F> deferWrapper(F f) {
	return DeferWrapper<F>(f);
}

#define defer(code) auto CONCAT(_defer_, __COUNTER__) = deferWrapper([&]() code)

////////////////////
// Random number generator
// random.cpp
////////////////////

/** Seeds the RNG with the current time */
void randomInit();
/** Returns a uniform random uint32_t from 0 to UINT32_MAX */
uint32_t randomu32();
uint64_t randomu64();
/** Returns a uniform random float in the interval [0.0, 1.0) */
float randomUniform();
/** Returns a normal random number with mean 0 and standard deviation 1 */
float randomNormal();

DEPRECATED inline float randomf() {return randomUniform();}


} // namespace rack

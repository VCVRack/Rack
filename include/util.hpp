#pragma once

// Include most of the C standard library for convenience
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <string>
#include <condition_variable>
#include <mutex>


/** Surrounds raw text with quotes
Example:
	printf("Hello " STRINGIFY(world))
will expand to
	printf("Hello " "world")
and of course the C++ lexer/parser will then concatenate the string literals
*/
#define STRINGIFY(x) #x
/** Converts a macro to a string literal
Example:
	#define NAME "world"
	printf("Hello " TOSTRING(NAME))
will expand to
	printf("Hello " "world")
*/
#define TOSTRING(x) STRINGIFY(x)


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


/** Threads which obtain a VIPLock will cause wait() to block for other less important threads.
This does not provide the VIPs with an exclusive lock. That should be left up to another mutex shared between the less important thread.
*/
struct VIPMutex {
	int count = 0;
	std::condition_variable cv;
	std::mutex countMutex;

	/** Blocks until there are no remaining VIPLocks */
	void wait() {
		std::unique_lock<std::mutex> lock(countMutex);
		while (count > 0)
			cv.wait(lock);
	}
};

struct VIPLock {
	VIPMutex &m;
	VIPLock(VIPMutex &m) : m(m) {
		std::unique_lock<std::mutex> lock(m.countMutex);
		m.count++;
	}
	~VIPLock() {
		std::unique_lock<std::mutex> lock(m.countMutex);
		m.count--;
		lock.unlock();
		m.cv.notify_all();
	}
};


} // namespace rack

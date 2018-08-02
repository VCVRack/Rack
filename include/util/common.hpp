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

////////////////////
// Operating-system specific utilities
// system.cpp
////////////////////

std::vector<std::string> systemListEntries(std::string path);
bool systemIsFile(std::string path);
bool systemIsDirectory(std::string path);
void systemCopy(std::string srcPath, std::string destPath);
void systemCreateDirectory(std::string path);

/** Opens a URL, also happens to work with PDFs and folders.
Shell injection is possible, so make sure the URL is trusted or hard coded.
May block, so open in a new thread.
*/
void systemOpenBrowser(std::string url);

////////////////////
// Debug logger
// logger.cpp
////////////////////


////////////////////
// Thread functions
////////////////////

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

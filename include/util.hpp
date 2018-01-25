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

#define LENGTHOF(arr) (sizeof(arr) / sizeof((arr)[0]))

/** Reserve space for _count enums starting with _name.
Example:
	enum Foo {
		ENUMS(BAR, 14)
	};

	BAR + 0 to BAR + 11 is reserved
*/
#define ENUMS(_name, _count) _name, _name ## _LAST = _name + (_count) - 1

/** Deprecation notice for GCC */
#define DEPRECATED __attribute__ ((deprecated))


namespace rack {


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


////////////////////
// RNG
////////////////////

/** Seeds the RNG with the current time */
void randomInit();
uint32_t randomu32();
uint64_t randomu64();
/** Returns a uniform random float in the interval [0.0, 1.0) */
float randomf();
/** Returns a normal random number with mean 0 and std dev 1 */
float randomNormal();

////////////////////
// String functions
////////////////////

/** Converts a printf format string and optional arguments into a std::string */
std::string stringf(const char *format, ...);
std::string lowercase(std::string s);
std::string uppercase(std::string s);

/** Truncates and adds "..." to a string, not exceeding `len` characters */
std::string ellipsize(std::string s, size_t len);
bool startsWith(std::string str, std::string prefix);

std::string extractDirectory(std::string path);
std::string extractFilename(std::string path);
std::string extractExtension(std::string path);

////////////////////
// Operating system functions
////////////////////

/** Opens a URL, also happens to work with PDFs and folders.
Shell injection is possible, so make sure the URL is trusted or hard coded.
May block, so open in a new thread.
*/
void openBrowser(std::string url);

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

////////////////////
// logger
////////////////////

extern FILE *gLogFile;
void debug(const char *format, ...);
void info(const char *format, ...);
void warn(const char *format, ...);
void fatal(const char *format, ...);


} // namespace rack

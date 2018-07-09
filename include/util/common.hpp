#pragma once

// Include most of the C standard library for convenience
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>

////////////////////
// Handy macros
////////////////////

/** Concatenates two literals or two macros
Example:
	#define COUNT 42
	CONCAT(myVariable, COUNT)
expands to
	myVariable42
*/
#define CONCAT_LITERAL(x, y) x ## y
#define CONCAT(x, y) CONCAT_LITERAL(x, y)

/** Surrounds raw text with quotes
Example:
	#define NAME "world"
	printf("Hello " TOSTRING(NAME))
expands to
	printf("Hello " "world")
and of course the C++ lexer/parser then concatenates the string literals.
*/
#define TOSTRING_LITERAL(x) #x
#define TOSTRING(x) TOSTRING_LITERAL(x)

/** Produces the length of a static array in number of elements */
#define LENGTHOF(arr) (sizeof(arr) / sizeof((arr)[0]))

/** Reserve space for `count` enums starting with `name`.
Example:
	enum Foo {
		ENUMS(BAR, 14),
		BAZ
	};

	BAR + 0 to BAR + 13 is reserved. BAZ has a value of 14.
*/
#define ENUMS(name, count) name, name ## _LAST = name + (count) - 1

/** Deprecation notice for GCC */
#define DEPRECATED __attribute__ ((deprecated))


/** References binary files compiled into the program.
For example, to include a file "Test.dat" directly into your program binary, add
	BINARIES += Test.dat
to your Makefile and declare
	BINARY(Test_dat);
at the root of a .c or .cpp source file. Note that special characters are replaced with "_". Then use
	BINARY_START(Test_dat)
	BINARY_END(Test_dat)
to reference the data beginning and end as a void* array, and
	BINARY_SIZE(Test_dat)
to get its size in bytes.
*/
#ifdef ARCH_MAC
	// Use output from `xxd -i`
	#define BINARY(sym) extern unsigned char sym[]; extern unsigned int sym##_len
	#define BINARY_START(sym) ((const void*) sym)
	#define BINARY_END(sym) ((const void*) sym + sym##_len)
	#define BINARY_SIZE(sym) (sym##_len)
#else
	#define BINARY(sym) extern char _binary_##sym##_start, _binary_##sym##_end, _binary_##sym##_size
	#define BINARY_START(sym) ((const void*) &_binary_##sym##_start)
	#define BINARY_END(sym) ((const void*) &_binary_##sym##_end)
	// The symbol "_binary_##sym##_size" doesn't seem to be valid after a plugin is dynamically loaded, so simply take the difference between the two addresses.
	#define BINARY_SIZE(sym) ((size_t) (&_binary_##sym##_end - &_binary_##sym##_start))
#endif


#include "util/math.hpp"


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
// String utilities
// string.cpp
////////////////////

/** Converts a printf format string and optional arguments into a std::string */
std::string stringf(const char *format, ...);
std::string stringLowercase(std::string s);
std::string stringUppercase(std::string s);

/** Truncates and adds "..." to a string, not exceeding `len` characters */
std::string stringEllipsize(std::string s, size_t len);
bool stringStartsWith(std::string str, std::string prefix);
bool stringEndsWith(std::string str, std::string suffix);

/** Extracts portions of a path */
std::string stringDirectory(std::string path);
std::string stringFilename(std::string path);
std::string stringExtension(std::string path);

struct StringCaseInsensitiveCompare {
	bool operator()(const std::string &a, const std::string &b) const {
		return stringLowercase(a) < stringLowercase(b);
	}
};

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

enum LoggerLevel {
	DEBUG_LEVEL = 0,
	INFO_LEVEL,
	WARN_LEVEL,
	FATAL_LEVEL
};

void loggerInit(bool devMode);
void loggerDestroy();
/** Do not use this function directly. Use the macros below. */
void loggerLog(LoggerLevel level, const char *file, int line, const char *format, ...);
/** Example usage:
	debug("error: %d", errno);
will print something like
	[0.123 debug myfile.cpp:45] error: 67
*/
#define debug(format, ...) loggerLog(DEBUG_LEVEL, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define info(format, ...) loggerLog(INFO_LEVEL, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define warn(format, ...) loggerLog(WARN_LEVEL, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define fatal(format, ...) loggerLog(FATAL_LEVEL, __FILE__, __LINE__, format, ##__VA_ARGS__)

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

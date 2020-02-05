#pragma once
// Include most of the C stdlib for convenience
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstdarg>
#include <climits>
#include <cmath>
#include <cstring>
#include <cassert>

// Include some of the C++ stdlib for convenience
#include <string>
#include <stdexcept>

#include <logger.hpp>


namespace rack {


/** Deprecation notice for functions
E.g.

	DEPRECATED void foo();
*/
#if defined(__GNUC__) || defined(__clang__)
	#define DEPRECATED __attribute__ ((deprecated))
#elif defined(_MSC_VER)
	#define DEPRECATED __declspec(deprecated)
#endif



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

`BAR + 0` to `BAR + 13` is reserved. `BAZ` has a value of 14.
*/
#define ENUMS(name, count) name, name ## _LAST = name + (count) - 1


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
#if defined ARCH_MAC
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


/** C#-style property constructor
Example:

	Foo *foo = construct<Foo>(&Foo::greeting, "Hello world", &Foo::legs, 2);
*/
template <typename T>
T* construct() {
	return new T;
}

template <typename T, typename F, typename V, typename... Args>
T* construct(F f, V v, Args... args) {
	T* o = construct<T>(args...);
	o->*f = v;
	return o;
}

/** Defers code until the scope is destructed
From http://www.gingerbill.org/article/defer-in-cpp.html
Example:

	file = fopen(...);
	DEFER({
		fclose(file);
	});
*/
template <typename F>
struct DeferWrapper {
	F f;
	DeferWrapper(F f) : f(f) {}
	~DeferWrapper() {
		f();
	}
};

template <typename F>
DeferWrapper<F> deferWrapper(F f) {
	return DeferWrapper<F>(f);
}

#define DEFER(code) auto CONCAT(_defer_, __COUNTER__) = rack::deferWrapper([&]() code)


/** An exception explicitly thrown by Rack. */
struct Exception : std::runtime_error {
	Exception(const std::string& msg) : std::runtime_error(msg) {}
};


// config

extern const std::string APP_NAME;
extern const std::string APP_VERSION;
extern const std::string APP_ARCH;

extern const std::string ABI_VERSION;

extern const std::string API_URL;
extern const std::string API_VERSION;


} // namespace rack


#if defined ARCH_WIN
// Windows C standard functions are ASCII-8 instead of UTF-8, so redirect these functions to wrappers which convert to UTF-8
#define fopen fopen_utf8
#define remove remove_utf8
#define rename rename_utf8

extern "C" {
FILE* fopen_utf8(const char* filename, const char* mode);
int remove_utf8(const char* path);
int rename_utf8(const char* oldname, const char* newname);
}

namespace std {
	using ::fopen_utf8;
	using ::remove_utf8;
	using ::rename_utf8;
}
#endif

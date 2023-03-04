#pragma once
// Include most of the C stdlib for convenience
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cinttypes>
#include <cstdarg>
#include <climits>
#include <cmath>
#include <cstring>
#include <cassert>

// Include some of the C++ stdlib for convenience
#include <string>
#include <stdexcept>


/** Attribute for deprecated functions and symbols.
E.g.

	DEPRECATED void foo();
*/
#define DEPRECATED __attribute__((deprecated))

/** Attribute for private functions not intended to be called by plugins.
When #including rack.hpp, attempting to call PRIVATE functions will result in a compile-time error.
*/
#ifndef PRIVATE
#define PRIVATE
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


/** Helpful user-defined literals for specifying exact integer and float types.
Usage examples:
	42_i8
	-4242_u16
	0x4a2b_i32
	0b11010111000000_u64
	42_f32
	4.2e-4_f64
*/
inline int8_t operator"" _i8(unsigned long long x) {return x;}
inline int16_t operator"" _i16(unsigned long long x) {return x;}
inline int32_t operator"" _i32(unsigned long long x) {return x;}
inline int64_t operator"" _i64(unsigned long long x) {return x;}
inline uint8_t operator"" _u8(unsigned long long x) {return x;}
inline uint16_t operator"" _u16(unsigned long long x) {return x;}
inline uint32_t operator"" _u32(unsigned long long x) {return x;}
inline uint64_t operator"" _u64(unsigned long long x) {return x;}
inline float operator"" _f32(long double x) {return x;}
inline float operator"" _f32(unsigned long long x) {return x;}
inline double operator"" _f64(long double x) {return x;}
inline double operator"" _f64(unsigned long long x) {return x;}


#if defined ARCH_WIN
// wchar_t on Windows should be 2 bytes
static_assert(sizeof(wchar_t) == 2);

// Windows C standard functions are ASCII-8 instead of UTF-8, so redirect these functions to wrappers which convert to UTF-8
#define fopen fopen_u8

extern "C" {
FILE* fopen_u8(const char* filename, const char* mode);
}

namespace std {
	using ::fopen_u8;
}
#endif


/** Root namespace for the Rack API */
namespace rack {


/** Casts a primitive, preserving its bits instead of converting. */
template <typename To, typename From>
To bitCast(From from) {
	static_assert(sizeof(From) == sizeof(To), "Types must be the same size");
	To to;
	// This is optimized out
	std::memcpy(&to, &from, sizeof(From));
	return to;
}


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


/** Defers running code until the scope is destructed

From http://www.gingerbill.org/article/defer-in-cpp.html.
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


/** An exception explicitly thrown by Rack or a Rack plugin.
Can be subclassed to throw/catch specific custom exceptions.
*/
struct Exception : std::exception {
	std::string msg;

	// Attribute index 1 refers to `Exception*` argument so use 2.
	__attribute__((format(printf, 2, 3)))
	Exception(const char* format, ...);
	Exception(const std::string& msg) : msg(msg) {}
	const char* what() const noexcept override {
		return msg.c_str();
	}
};


/** Given a std::map `c`, returns the value of the given key, or returns `def` if the key doesn't exist.
Does *not* add the default value to the map.

Posted to https://stackoverflow.com/a/63683271/272642.
Example:

	std::map<std::string, int> m;
	int v = get(m, "a", 3);
	// v is 3 because the key "a" does not exist

	int w = get(m, "a");
	// w is 0 because no default value is given, so it assumes the default int.
*/
template <typename C>
const typename C::mapped_type& get(const C& c, const typename C::key_type& key, const typename C::mapped_type& def = typename C::mapped_type()) {
	typename C::const_iterator it = c.find(key);
	if (it == c.end())
		return def;
	return it->second;
}


/** Given a std::vector `c`, returns the value of the given index `i`, or returns `def` if the index is out of bounds.
*/
template <typename C>
const typename C::value_type& get(const C& c, typename C::size_type i, const typename C::value_type& def = typename C::value_type()) {
	if (i >= c.size())
		return def;
	return c[i];
}


// config

extern const std::string APP_NAME;
extern const std::string APP_EDITION;
extern const std::string APP_EDITION_NAME;
extern const std::string APP_VERSION_MAJOR;
extern const std::string APP_VERSION;
extern const std::string APP_OS;
extern const std::string APP_OS_NAME;
extern const std::string APP_CPU;
extern const std::string APP_CPU_NAME;
extern const std::string API_URL;


} // namespace rack


// Logger depends on common.hpp, but it is handy to include it along with common.hpp.
#include <logger.hpp>

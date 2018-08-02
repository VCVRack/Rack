#pragma once


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

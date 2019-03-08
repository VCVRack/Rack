// Public Domain. See "unlicense" statement at the end of this file.

// USAGE
//
// This is a single-file library. To use it, do something like the following in one .c file.
//   #define DR_IMPLEMENTATION
//   #include "dr.h"
//
// You can then #include dr.h in other parts of the program as you would with any other header file.
//
//
//
// OPTIONS
//
// #define DR_UTIL_WIN32_USE_CRITICAL_SECTION_MUTEX
//   - Use the Win32 CRITICAL_SECTION API for mutex objects.

#ifndef dr_util_h
#define dr_util_h

#ifdef __cplusplus
extern "C" {
#endif


// Disable MSVC compatibility if we're compiling with it.
#if defined(_MSC_VER) || defined(__MINGW32__)
    #define DR_NO_MSVC_COMPAT
#endif

#if defined(_MSC_VER)
#define DR_INLINE static __inline
#else
#define DR_INLINE static inline
#endif


#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#ifndef DR_NO_MSVC_COMPAT
#include <errno.h>
#endif

#ifndef DR_SIZED_TYPES_DEFINED
#define DR_SIZED_TYPES_DEFINED
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef   signed char    dr_int8;
typedef unsigned char    dr_uint8;
typedef   signed short   dr_int16;
typedef unsigned short   dr_uint16;
typedef   signed int     dr_int32;
typedef unsigned int     dr_uint32;
typedef   signed __int64 dr_int64;
typedef unsigned __int64 dr_uint64;
#else
#include <stdint.h>
typedef int8_t           dr_int8;
typedef uint8_t          dr_uint8;
typedef int16_t          dr_int16;
typedef uint16_t         dr_uint16;
typedef int32_t          dr_int32;
typedef uint32_t         dr_uint32;
typedef int64_t          dr_int64;
typedef uint64_t         dr_uint64;
#endif
typedef dr_uint8         dr_bool8;
typedef dr_uint32        dr_bool32;
#define DR_TRUE          1
#define DR_FALSE         0
#endif


#define STRINGIFY(x)    #x
#define TOSTRING(x)     STRINGIFY(x)


/////////////////////////////////////////////////////////
// Annotations

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif


/////////////////////////////////////////////////////////
// min/max/clamp

#ifndef dr_min
#define dr_min(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef dr_max
#define dr_max(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef dr_clamp
#define dr_clamp(x, low, high) (dr_max(low, dr_min(x, high)))
#endif

#ifndef dr_round_up
#define dr_round_up(x, multiple) ((((x) + ((multiple) - 1)) / (multiple)) * (multiple))
#endif

#ifndef dr_round_up_signed
#define dr_round_up_signed(x, multiple) ((((x) + (((x) >= 0)*((multiple) - 1))) / (multiple)) * (multiple))
#endif

DR_INLINE dr_uint32 dr_next_power_of_2(dr_uint32 value)
{
    --value;
    value = (value >> 1)  | value;
    value = (value >> 2)  | value;
    value = (value >> 4)  | value;
    value = (value >> 8)  | value;
    value = (value >> 16) | value;
    return value + 1;
}


#define dr_abs(x) (((x) < 0) ? (-(x)) : (x))


/////////////////////////////////////////////////////////
// MSVC Compatibility

// A basic implementation of MSVC's strcpy_s().
int dr_strcpy_s(char* dst, size_t dstSizeInBytes, const char* src);

// A basic implementation of MSVC's strncpy_s().
int dr_strncpy_s(char* dst, size_t dstSizeInBytes, const char* src, size_t count);

// A basic implementation of MSVC's strcat_s().
int dr_strcat_s(char* dst, size_t dstSizeInBytes, const char* src);

// A basic implementation of MSVC's strncat_s()
int dr_strncat_s(char* dst, size_t dstSizeInBytes, const char* src, size_t count);

// A basic implementation of MSVC's _atoi_s()
int dr_itoa_s(int value, char* dst, size_t dstSizeInBytes, int radix);

#ifndef DR_NO_MSVC_COMPAT
#ifndef _TRUNCATE
#define _TRUNCATE ((size_t)-1)
#endif

DR_INLINE int strcpy_s(char* dst, size_t dstSizeInBytes, const char* src)
{
    return dr_strcpy_s(dst, dstSizeInBytes, src);
}

DR_INLINE int strncpy_s(char* dst, size_t dstSizeInBytes, const char* src, size_t count)
{
    return dr_strncpy_s(dst, dstSizeInBytes, src, count);
}

DR_INLINE int strcat_s(char* dst, size_t dstSizeInBytes, const char* src)
{
    return dr_strcat_s(dst, dstSizeInBytes, src);
}

DR_INLINE int strncat_s(char* dst, size_t dstSizeInBytes, const char* src, size_t count)
{
    return dr_strncat_s(dst, dstSizeInBytes, src, count);
}

#ifndef __MINGW32__
DR_INLINE int _stricmp(const char* string1, const char* string2)
{
    return strcasecmp(string1, string2);
}
#endif

DR_INLINE int _itoa_s(int value, char* dst, size_t dstSizeInBytes, int radix)
{
    return dr_itoa_s(value, dst, dstSizeInBytes, radix);
}
#endif


/////////////////////////////////////////////////////////
// String Helpers

// Determines if the given character is whitespace.
dr_bool32 dr_is_whitespace(dr_uint32 utf32);

/// Removes every occurance of the given character from the given string.
void dr_strrmchar(char* str, char c);

/// Finds the first non-whitespace character in the given string.
const char* dr_first_non_whitespace(const char* str);

static inline const char* dr_ltrim(const char* str) { return dr_first_non_whitespace(str); }
static const char* dr_rtrim(const char* str);

/// Trims both the leading and trailing whitespace from the given string.
void dr_trim(char* str);

/// Finds the first occurance of a whitespace character in the given string.
const char* dr_first_whitespace(const char* str);

/// Finds the beginning of the next line.
const char* dr_next_line(const char* str);

/// Makes a copy of the first line of the given string.
size_t dr_copy_line(const char* str, char* lineOut, size_t lineOutSize);

// A slow string replace function. Free the returned string with free().
char* dr_string_replace(const char* src, const char* query, const char* replacement);

// Replaces an ASCII character with another in the given string.
void dr_string_replace_ascii(char* src, char c, char replacement);


/////////////////////////////////////////////////////////
// Unicode Utilities

/// Converts a UTF-32 character to UTF-16.
///
/// @param utf16 [in] A pointer to an array of at least two 16-bit values that will receive the UTF-16 character.
///
/// @return 2 if the returned character is a surrogate pair, 1 if it's a simple UTF-16 code point, or 0 if it's an invalid character.
///
/// @remarks
///     It is assumed the <utf16> is large enough to hold at least 2 unsigned shorts. <utf16> will be padded with 0 for unused
///     components.
DR_INLINE int dr_utf32_to_utf16_ch(unsigned int utf32, unsigned short utf16[2])
{
    if (utf16 == NULL) {
        return 0;
    }

    if (utf32 < 0xD800 || (utf32 >= 0xE000 && utf32 <= 0xFFFF))
    {
        utf16[0] = (unsigned short)utf32;
        utf16[1] = 0;
        return 1;
    }
    else
    {
        if (utf32 >= 0x10000 && utf32 <= 0x10FFFF)
        {
            utf16[0] = (unsigned short)(0xD7C0 + (unsigned short)(utf32 >> 10));
            utf16[1] = (unsigned short)(0xDC00 + (unsigned short)(utf32 & 0x3FF));
            return 2;
        }
        else
        {
            // Invalid.
            utf16[0] = 0;
            utf16[1] = 0;
            return 0;
        }
    }
}

/// Converts a UTF-16 character to UTF-32.
DR_INLINE unsigned int dr_utf16_to_utf32_ch(unsigned short utf16[2])
{
    if (utf16 == NULL) {
        return 0;
    }

    if (utf16[0] < 0xD800 || utf16[0] > 0xDFFF)
    {
        return utf16[0];
    }
    else
    {
        if ((utf16[0] & 0xFC00) == 0xD800 && (utf16[1] & 0xFC00) == 0xDC00)
        {
            return ((unsigned int)utf16[0] << 10) + utf16[1] - 0x35FDC00;
        }
        else
        {
            // Invalid.
            return 0;
        }
    }
}

/// Converts a UTF-16 surrogate pair to UTF-32.
DR_INLINE unsigned int dr_utf16pair_to_utf32_ch(unsigned short utf160, unsigned short utf161)
{
    unsigned short utf16[2];
    utf16[0] = utf160;
    utf16[1] = utf161;
    return dr_utf16_to_utf32_ch(utf16);
}


/////////////////////////////////////////////////////////
// Aligned Allocations

#ifndef DRUTIL_NO_ALIGNED_MALLOC
DR_INLINE void* dr_aligned_malloc(size_t alignment, size_t size)
{
#if defined(_WIN32) || defined(_WIN64)
    return _aligned_malloc(size, alignment);
#else
    void* pResult;
    if (posix_memalign(&pResult, alignment, size) == 0) {
        return pResult;
    }

    return 0;
#endif
}

DR_INLINE void dr_aligned_free(void* ptr)
{
#if defined(_WIN32) || defined(_WIN64)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}
#endif  // !DRUTIL_NO_ALIGNED_MALLOC



/////////////////////////////////////////////////////////
// Key/Value Pair Parsing

typedef size_t (* dr_key_value_read_proc) (void* pUserData, void* pDataOut, size_t bytesToRead);
typedef void   (* dr_key_value_pair_proc) (void* pUserData, const char* key, const char* value);
typedef void   (* dr_key_value_error_proc)(void* pUserData, const char* message, unsigned int line);

/// Parses a series of simple Key/Value pairs.
///
/// @remarks
///     This function is suitable for parsing simple key/value config files.
///     @par
///     This function will never allocate memory on the heap. Because of this there is a minor restriction in the length of an individual
///     key/value pair which is 4KB.
///     @par
///     Formatting rules are as follows:
///      - The basic syntax for a key/value pair is [key][whitespace][value]. Example: MyProperty 1234
///      - All key/value pairs must be declared on a single line, and a single line cannot contain more than a single key/value pair.
///      - Comments begin with the '#' character and continue until the end of the line.
///      - A key cannot contain spaces but are permitted in values.
///      - The value will have any leading and trailing whitespace trimmed.
///     @par
///     If an error occurs, that line will be skipped and processing will continue.
void dr_parse_key_value_pairs(dr_key_value_read_proc onRead, dr_key_value_pair_proc onPair, dr_key_value_error_proc onError, void* pUserData);

// This will only return DR_FALSE if the file fails to open. It will still return DR_TRUE even if there are syntax error or whatnot.
dr_bool32 dr_parse_key_value_pairs_from_file(const char* filePath, dr_key_value_pair_proc onPair, dr_key_value_error_proc onError, void* pUserData);



/////////////////////////////////////////////////////////
// Basic Tokenizer

/// Retrieves the first token in the given string.
///
/// @remarks
///     This function is suitable for doing a simple whitespace tokenization of a null-terminated string.
///     @par
///     The return value is a pointer to one character past the last character of the next token. You can use the return value to execute
///     this function in a loop to parse an entire string.
///     @par
///     <tokenOut> can be null. If the buffer is too small to contain the entire token it will be set to an empty string. The original
///     input string combined with the return value can be used to reliably find the token.
///     @par
///     This will handle double-quoted strings, so a string such as "My \"Complex String\"" contains two tokens: "My" and "\"Complex String\"".
///     @par
///     This function has no dependencies.
const char* dr_next_token(const char* tokens, char* tokenOut, size_t tokenOutSize);



/////////////////////////////////////////////////////////
// Known Folders

/// Retrieves the path of the executable.
///
/// @remarks
///     Currently only works on Windows and Linux. Other platforms will be added as they're needed.
dr_bool32 dr_get_executable_path(char* pathOut, size_t pathOutSize);

/// Retrieves the path of the directory containing the executable.
///
/// @remarks
///     Currently only works on Windows and Linux. Other platforms will be added as they're needed.
dr_bool32 dr_get_executable_directory_path(char* pathOut, size_t pathOutSize);

/// Retrieves the path of the user's config directory.
///
/// @remarks
///     On Windows this will typically be %APPDATA% and on Linux it will usually be ~/.config
dr_bool32 dr_get_config_folder_path(char* pathOut, size_t pathOutSize);

/// Retrieves the path of the user's log directory.
///
/// @remarks
///     On Windows this will typically be %APPDATA% and on Linux it will usually be var/log
dr_bool32 dr_get_log_folder_path(char* pathOut, size_t pathOutSize);

/// Retrieves the current directory.
const char* dr_get_current_directory(char* pathOut, size_t pathOutSize);

/// Sets the current directory.
dr_bool32 dr_set_current_directory(const char* path);



/////////////////////////////////////////////////////////
// Basic File Management

// Callback function for file iteration.
typedef dr_bool32 (* dr_iterate_files_proc)(const char* filePath, void* pUserData);


// Helper for opening a stdio FILE.
FILE* dr_fopen(const char* fileName, const char* openMode);

// Helper for creating an empty file.
dr_bool32 dr_create_empty_file(const char* fileName, dr_bool32 failIfExists);

// Retrieves the file data of the given file. Free the returned pointer with dr_free_file_data().
void* dr_open_and_read_file(const char* filePath, size_t* pFileSizeOut);

// Retrieves the file data of the given file as a null terminated string. Free the returned pointer with dr_free_file_data(). The
// returned file size is the length of the string not including the null terminator.
char* dr_open_and_read_text_file(const char* filePath, size_t* pFileSizeOut);

// Creates a new file with the given data.
dr_bool32 dr_open_and_write_file(const char* filePath, const void* pData, size_t dataSize);

// Creates a new file with the given string.
dr_bool32 dr_open_and_write_text_file(const char* filePath, const char* text);

// Frees the file data returned by dr_open_and_read_file().
void dr_free_file_data(void* valueReturnedByOpenAndReadFile);

// Determines whether or not the given file path is to a file.
//
// This will return DR_FALSE if the path points to a directory.
dr_bool32 dr_file_exists(const char* filePath);

// Determines whether or not the given file path points to a directory.
//
// This will return DR_FALSE if the path points to a file.
dr_bool32 dr_directory_exists(const char* directoryPath);
static inline dr_bool32 dr_is_directory(const char* directoryPath) { return dr_directory_exists(directoryPath); }

// Moves a file.
//
// This uses rename() on POSIX platforms and MoveFileEx(oldPath, newPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH) on windows platforms.
dr_bool32 dr_move_file(const char* oldPath, const char* newPath);

// Copies a file.
dr_bool32 dr_copy_file(const char* srcPath, const char* dstPath, dr_bool32 failIfExists);

// Determines if the given file is read only.
dr_bool32 dr_is_file_read_only(const char* filePath);

// Retrieves the last modified time of the file at the given path.
dr_uint64 dr_get_file_modified_time(const char* filePath);

// Deletes the file at the given path.
//
// This uses remove() on POSIX platforms and DeleteFile() on Windows platforms.
dr_bool32 dr_delete_file(const char* filePath);

// Cross-platform wrapper for creating a directory.
dr_bool32 dr_mkdir(const char* directoryPath);

// Recursively creates a directory.
dr_bool32 dr_mkdir_recursive(const char* directoryPath);

// Iterates over every file and folder of the given directory.
dr_bool32 dr_iterate_files(const char* directory, dr_bool32 recursive, dr_iterate_files_proc proc, void* pUserData);


/////////////////////////////////////////////////////////
// DPI Awareness

#if defined(_WIN32)
/// Win32 Only: Makes the application DPI aware.
void dr_win32_make_dpi_aware();

/// Win32 Only: Retrieves the base DPI to use as a reference when calculating DPI scaling.
void dr_win32_get_base_dpi(int* pDPIXOut, int* pDPIYOut);

/// Win32 Only: Retrieves the system-wide DPI.
void dr_win32_get_system_dpi(int* pDPIXOut, int* pDPIYOut);

/// Win32 Only: Retrieves the actual DPI of the monitor at the given index.
///
/// @remarks
///     If per-monitor DPI is not supported, the system wide DPI settings will be used instead.
///     @par
///     This runs in linear time.
void dr_win32_get_monitor_dpi(int monitor, int* pDPIXOut, int* pDPIYOut);

/// Win32 Only: Retrieves the number of monitors active at the time of calling.
///
/// @remarks
///     This runs in linear time.
int dr_win32_get_monitor_count();
#endif



/////////////////////////////////////////////////////////
// Date / Time

/// Retrieves a time_t as of the time the function was called.
time_t dr_now();

/// Formats a data/time string.
void dr_datetime_short(time_t t, char* strOut, unsigned int strOutSize);

// Returns a date string in YYYYMMDD format.
void dr_date_YYYYMMDD(time_t t, char* strOut, unsigned int strOutSize);



/////////////////////////////////////////////////////////
// Command Line
//
// The command line functions below are just simple iteration functions. This command line system is good for
// simple command lines, but probably not the best for programs requiring complex command line work.
//
// For argv style command lines, parse_cmdline() will run without any heap allocations. With a Win32 style
// command line there will be one malloc() per call fo parse_cmdline(). This is the only function that will do
// a malloc().
//
// Below is an example:
//
// dr_cmdline cmdline;
// if (dr_init_cmdline(&cmdline, argc, argv)) {
//     dr_parse_cmdline(&cmdline, my_cmdline_handler, pMyUserData);
// }
//
// void my_cmdline_handler(const char* key, const char* value, void* pUserData)
// {
//     // Do something...
// }
//
//
// When parsing the command line, the first iteration will be the program path and the key will be "[path]".
//
// For segments such as "-abcd", the callback will be called for "a", "b", "c", "d" individually, with the
// value set to NULL.
//
// For segments such as "--server", the callback will be called for "server", with the value set to NULL.
//
// For segments such as "-f file.txt", the callback will be called with the key set to "f" and the value set
// to "file.txt".
//
// For segments such as "-f file1.txt file2.txt", the callback will be called twice, once for file1.txt and
// again for file2.txt, with with the key set to "f" in both cases.
//
// For segments where there is no leading key, the values will be posted as annonymous (key set to NULL). An example
// is "my_program.exe file1.txt file2.txt", in which case the first iteration will be the program path, the second iteration
// will be "file1.txt", with the key set to NULL. The third iteration will be "file2.txt" with the key set to NULL.
//
// For segments such as "-abcd file.txt", "a", "b", "c", "d" will be sent with NULL values, and "file.txt" will be
// posted with a NULL key.

typedef struct dr_cmdline dr_cmdline;
struct dr_cmdline
{
    // argv style.
    int argc;
    char** argv;

    // Win32 style
    const char* win32;
};

typedef dr_bool32 dr_cmdline_parse_proc(const char* key, const char* value, void* pUserData);


/// Initializes a command line object.
dr_bool32 dr_init_cmdline(dr_cmdline* pCmdLine, int argc, char** argv);

/// Initializes a command line object using a Win32 style command line.
dr_bool32 dr_init_cmdline_win32(dr_cmdline* pCmdLine, const char* args);

/// Parses the given command line.
void dr_parse_cmdline(dr_cmdline* pCmdLine, dr_cmdline_parse_proc callback, void* pUserData);

/// Helper for determining whether or not the given key exists.
dr_bool32 dr_cmdline_key_exists(dr_cmdline* pCmdLine, const char* key);

// Convers the given command line object to argc/argv style.
//
// Returns the argument count. Returns 0 if an error occurs. Free "argvOut" with dr_free_argv().
int dr_cmdline_to_argv(dr_cmdline* pCmdLine, char*** argvOut);

// Converts a WinMain style command line to argc/argv.
//
// Returns the argument count. Returns 0 if an error occurs. Free "argvOut" with dr_free_argv().
int dr_winmain_to_argv(const char* cmdlineWinMain, char*** argvOut);

// Frees the argc/argv command line that was generated by dr.h
void dr_free_argv(char** argv);




/////////////////////////////////////////////////////////
// Threading

/// Puts the calling thread to sleep for approximately the given number of milliseconds.
///
/// @remarks
///     This is not 100% accurate and should be considered an approximation.
void dr_sleep(unsigned int milliseconds);
void dr_yield();

/// Retrieves the number of logical cores on system.
unsigned int dr_get_logical_processor_count();


/// Thread.
typedef void* dr_thread;
typedef int (* dr_thread_entry_proc)(void* pData);

/// Creates and begins executing a new thread.
///
/// @remarks
///     This will not return until the thread has entered into it's entry point.
///     @par
///     Creating a thread should be considered an expensive operation. For high performance, you should create threads
///     at load time and cache them.
dr_thread dr_create_thread(dr_thread_entry_proc entryProc, void* pData);

/// Deletes the given thread.
///
/// @remarks
///     This does not actually exit the thread, but rather deletes the memory that was allocated for the thread
///     object returned by dr_create_thread().
///     @par
///     It is usually best to wait for the thread to terminate naturally with dr_wait_thread() before calling
///     this function, however it is still safe to do something like the following.
///     @code
///     dr_delete_thread(dr_create_thread(my_thread_proc, pData))
///     @endcode
void dr_delete_thread(dr_thread thread);

/// Waits for the given thread to terminate.
void dr_wait_thread(dr_thread thread);

/// Helper function for waiting for a thread and then deleting the handle after it has terminated.
void dr_wait_and_delete_thread(dr_thread thread);



/// Mutex
typedef void* dr_mutex;

/// Creates a mutex object.
///
/// @remarks
///     If an error occurs, 0 is returned. Otherwise a handle the size of a pointer is returned.
dr_mutex dr_create_mutex();

/// Deletes a mutex object.
void dr_delete_mutex(dr_mutex mutex);

/// Locks the given mutex.
void dr_lock_mutex(dr_mutex mutex);

/// Unlocks the given mutex.
void dr_unlock_mutex(dr_mutex mutex);



/// Semaphore
typedef void* dr_semaphore;

/// Creates a semaphore object.
///
/// @remarks
///     If an error occurs, 0 is returned. Otherwise a handle the size of a pointer is returned.
dr_semaphore dr_create_semaphore(int initialValue);

/// Deletes the given semaphore.
void dr_delete_semaphore(dr_semaphore semaphore);

/// Waits on the given semaphore object and decrements it's counter by one upon returning.
dr_bool32 dr_wait_semaphore(dr_semaphore semaphore);

/// Releases the given semaphore and increments it's counter by one upon returning.
dr_bool32 dr_release_semaphore(dr_semaphore semaphore);



/////////////////////////////////////////////////////////
// Timing

typedef struct
{
    dr_int64 counter;
} dr_timer;

// Initializes a high-resolution timer.
void dr_timer_init(dr_timer* pTimer);

// Ticks the timer and returns the number of seconds since the previous tick.
//
// The maximum return value is about 140 years or so.
double dr_timer_tick(dr_timer* pTimer);




/////////////////////////////////////////////////////////
// Random

// Generates a random double between 0 and 1. This is bassed of C standard rand().
double dr_randd();

// Generates a random float between 0 and 1. This is based off C standard rand().
float dr_randf();



/////////////////////////////////////////////////////////
// User Accounts and Process Management

// Retrieves the user name of the user running the application.
size_t dr_get_username(char* usernameOut, size_t usernameOutSize);

// Retrieves the ID of the current process.
unsigned int dr_get_process_id();




/////////////////////////////////////////////////////////
// Miscellaneous Stuff.

// Helper for clearing the given object to 0.
#define dr_zero_object(pObject) memset(pObject, 0, sizeof(*pObject));

// Converts an ASCII hex character to it's integral equivalent. Returns DR_FALSE if it's not a valid hex character.
dr_bool32 dr_hex_char_to_uint(char ascii, unsigned int* out);


/////////////////////////////////////////////////////////
// C++ Specific

#ifdef __cplusplus

// Use this to prevent objects of the given class or struct from being copied. This is also useful for eliminating some
// compiler warnings.
//
// Note for structs - this sets the access mode to private, so place this at the end of the declaration.
#define NO_COPY(classname) \
    private: \
        classname(const classname &); \
        classname & operator=(const classname &);


#ifndef DR_NO_MSVC_COMPAT
extern "C++"
{

template <size_t dstSizeInBytes>
int strcpy_s(char (&dst)[dstSizeInBytes], const char* src)
{
    return strcpy_s(dst, dstSizeInBytes, src);
}

template <size_t dstSizeInBytes>
int strncpy_s(char (&dst)[dstSizeInBytes], const char* src, size_t count)
{
    return strncpy_s(dst, dstSizeInBytes, src, count);
}

template <size_t dstSizeInBytes>
int strcat_s(char (&dst)[dstSizeInBytes], const char* src)
{
    return strcat_s(dst, dstSizeInBytes, src);
}

template <size_t dstSizeInBytes>
int strncat_s(char (&dst)[dstSizeInBytes], const char* src, size_t count)
{
    return strncat_s(dst, dstSizeInBytes, src, count);
}

}
#endif

#endif



#ifdef __cplusplus
}
#endif

#endif  //dr_util_h



///////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
///////////////////////////////////////////////////////////////////////////////

#ifdef DR_IMPLEMENTATION
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>     // For memmove()
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
#include <dirent.h>
#endif

int dr_strcpy_s(char* dst, size_t dstSizeInBytes, const char* src)
{
    if (dst == 0) {
        return EINVAL;
    }
    if (dstSizeInBytes == 0) {
        return ERANGE;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    size_t i;
    for (i = 0; i < dstSizeInBytes && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }

    if (i < dstSizeInBytes) {
        dst[i] = '\0';
        return 0;
    }

    dst[0] = '\0';
    return ERANGE;
}

int dr_strncpy_s(char* dst, size_t dstSizeInBytes, const char* src, size_t count)
{
    if (dst == 0) {
        return EINVAL;
    }
    if (dstSizeInBytes == 0) {
        return ERANGE;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    size_t maxcount = count;
    if (count == ((size_t)-1) || count >= dstSizeInBytes) {        // -1 = _TRUNCATE
        maxcount = dstSizeInBytes - 1;
    }

    size_t i;
    for (i = 0; i < maxcount && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }

    if (src[i] == '\0' || i == count || count == ((size_t)-1)) {
        dst[i] = '\0';
        return 0;
    }

    dst[0] = '\0';
    return ERANGE;
}

int dr_strcat_s(char* dst, size_t dstSizeInBytes, const char* src)
{
    if (dst == 0) {
        return EINVAL;
    }
    if (dstSizeInBytes == 0) {
        return ERANGE;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    char* dstorig = dst;

    while (dstSizeInBytes > 0 && dst[0] != '\0') {
        dst += 1;
        dstSizeInBytes -= 1;
    }

    if (dstSizeInBytes == 0) {
        return EINVAL;  // Unterminated.
    }


    while (dstSizeInBytes > 0 && src[0] != '\0') {
        *dst++ = *src++;
        dstSizeInBytes -= 1;
    }

    if (dstSizeInBytes > 0) {
        dst[0] = '\0';
    } else {
        dstorig[0] = '\0';
        return ERANGE;
    }

    return 0;
}

int dr_strncat_s(char* dst, size_t dstSizeInBytes, const char* src, size_t count)
{
    if (dst == 0) {
        return EINVAL;
    }
    if (dstSizeInBytes == 0) {
        return ERANGE;
    }
    if (src == 0) {
        return EINVAL;
    }

    char* dstorig = dst;

    while (dstSizeInBytes > 0 && dst[0] != '\0') {
        dst += 1;
        dstSizeInBytes -= 1;
    }

    if (dstSizeInBytes == 0) {
        return EINVAL;  // Unterminated.
    }


    if (count == ((size_t)-1)) {        // _TRUNCATE
        count = dstSizeInBytes - 1;
    }

    while (dstSizeInBytes > 0 && src[0] != '\0' && count > 0)
    {
        *dst++ = *src++;
        dstSizeInBytes -= 1;
        count -= 1;
    }

    if (dstSizeInBytes > 0) {
        dst[0] = '\0';
    } else {
        dstorig[0] = '\0';
        return ERANGE;
    }

    return 0;
}

int dr_itoa_s(int value, char* dst, size_t dstSizeInBytes, int radix)
{
    if (dst == NULL || dstSizeInBytes == 0) {
        return EINVAL;
    }
    if (radix < 2 || radix > 36) {
        dst[0] = '\0';
        return EINVAL;
    }

    int sign = (value < 0 && radix == 10) ? -1 : 1;     // The negative sign is only used when the base is 10.

    unsigned int valueU;
    if (value < 0) {
        valueU = -value;
    } else {
        valueU = value;
    }

    char* dstEnd = dst;
    do
    {
        int remainder = valueU % radix;
        if (remainder > 9) {
            *dstEnd = (char)((remainder - 10) + 'a');
        } else {
            *dstEnd = (char)(remainder + '0');
        }

        dstEnd += 1;
        dstSizeInBytes -= 1;
        valueU /= radix;
    } while (dstSizeInBytes > 0 && valueU > 0);

    if (dstSizeInBytes == 0) {
        dst[0] = '\0';
        return EINVAL;  // Ran out of room in the output buffer.
    }

    if (sign < 0) {
        *dstEnd++ = '-';
        dstSizeInBytes -= 1;
    }

    if (dstSizeInBytes == 0) {
        dst[0] = '\0';
        return EINVAL;  // Ran out of room in the output buffer.
    }

    *dstEnd = '\0';


    // At this point the string will be reversed.
    dstEnd -= 1;
    while (dst < dstEnd) {
        char temp = *dst;
        *dst = *dstEnd;
        *dstEnd = temp;

        dst += 1;
        dstEnd -= 1;
    }

    return 0;
}

/////////////////////////////////////////////////////////
// String Helpers

dr_bool32 dr_is_whitespace(dr_uint32 utf32)
{
    return utf32 == ' ' || utf32 == '\t' || utf32 == '\n' || utf32 == '\v' || utf32 == '\f' || utf32 == '\r';
}

void dr_strrmchar(char* str, char c)
{
    char* src = str;
    char* dst = str;

    while (src[0] != '\0')
    {
        dst[0] = src[0];

        if (dst[0] != c) {
            dst += 1;
        }

        src += 1;
    }

    dst[0] = '\0';
}

const char* dr_first_non_whitespace(const char* str)
{
    if (str == NULL) {
        return NULL;
    }

    while (str[0] != '\0' && !(str[0] != ' ' && str[0] != '\t' && str[0] != '\n' && str[0] != '\v' && str[0] != '\f' && str[0] != '\r')) {
        str += 1;
    }

    return str;
}

const char* dr_first_whitespace(const char* str)
{
    if (str == NULL) {
        return NULL;
    }

    while (str[0] != '\0' && (str[0] != ' ' && str[0] != '\t' && str[0] != '\n' && str[0] != '\v' && str[0] != '\f' && str[0] != '\r')) {
        str += 1;
    }

    return str;
}

const char* dr_rtrim(const char* str)
{
    if (str == NULL) {
        return NULL;
    }

    const char* rstr = str;
    while (str[0] != '\0') {
        if (dr_is_whitespace(str[0])) {
            str += 1;
            continue;
        }

        str += 1;
        rstr = str;
    }

    return rstr;
}

void dr_trim(char* str)
{
    if (str == NULL) {
        return;
    }

    const char* lstr = dr_ltrim(str);
    const char* rstr = dr_rtrim(lstr);

    if (lstr > str) {
        memmove(str, lstr, rstr-lstr);
    }

    str[rstr-lstr] = '\0';
}

const char* dr_next_line(const char* str)
{
    if (str == NULL) {
        return NULL;
    }

    while (str[0] != '\0' && (str[0] != '\n' && !(str[0] == '\r' && str[1] == '\n'))) {
        str += 1;
    }

    if (str[0] == '\0') {
        return NULL;
    }

    if (str[0] == '\r') {
        return str + 2;
    }

    return str + 1;
}

size_t dr_copy_line(const char* str, char* lineOut, size_t lineOutSize)
{
    if (str == NULL) {
        return 0;
    }

    if (str == NULL) {
        return 0;
    }

    size_t length = 0;
    while (lineOutSize > 0 && str[0] != '\0' && (str[0] != '\n' && !(str[0] == '\r' && str[1] == '\n'))) {
        *lineOut++ = *str++;
        lineOutSize -= 1;
        length += 1;
    }

    if (lineOutSize == 0) {
        return 0;
    }

    *lineOut = '\0';
    return length;
}

char* dr_string_replace(const char* src, const char* query, const char* replacement)
{
    // This function could be improved, but it's good enough for now.

    if (src == NULL || query == NULL) {
        return NULL;
    }

    if (replacement == NULL) {
        replacement = "";
    }

    int queryLen = (int)strlen(query);
    int replacementLen = (int)strlen(replacement);

    size_t replacementCount = 0;

    const char* temp = src;
    for (;;) {
        temp = strstr(temp, query);
        if (temp == NULL) {
            break;
        }

        temp += queryLen;
        replacementCount += 1;
    }


    char* result = (char*)malloc(strlen(src) + (replacementLen - queryLen)*replacementCount + 1);   // +1 for null terminator.
    if (result == NULL) {
        return NULL;
    }

    char* runningResult = result;
    for (size_t i = 0; i < replacementCount; ++i) {
        size_t len = strstr(src, query) - src;
        for (size_t j = 0; j < len; ++j) {
            runningResult[j] = src[j];
        }

        runningResult += len;
        for (int j = 0; j < replacementLen; ++j) {
            runningResult[j] = replacement[j];
        }

        runningResult += replacementLen;
        src += len + queryLen;
    }

    // The trailing part.
    strcpy_s(runningResult, strlen(src)+1, src);
    return result;
}

void dr_string_replace_ascii(char* src, char c, char replacement)
{
    for (;;) {
        if (*src == '\0') {
            break;
        }

        if (*src == c) {
            *src = replacement;
        }

        src += 1;
    }
}


/////////////////////////////////////////////////////////
// Key/Value Pair Parsing

void dr_parse_key_value_pairs(dr_key_value_read_proc onRead, dr_key_value_pair_proc onPair, dr_key_value_error_proc onError, void* pUserData)
{
    if (onRead == NULL) {
        return;
    }

    char pChunk[4096];
    size_t chunkSize = 0;

    unsigned int currentLine = 1;

    dr_bool32 moveToNextLineBeforeProcessing = DR_FALSE;
    dr_bool32 skipWhitespaceBeforeProcessing = DR_FALSE;

    // Just keep looping. We'll break from this loop when we have run out of data.
    for (;;)
    {
        // Start the iteration by reading as much data as we can.
        chunkSize = onRead(pUserData, pChunk, sizeof(pChunk));
        if (chunkSize == 0) {
            // No more data available.
            return;
        }

        char* pChunkEnd = pChunk + chunkSize;
        char* pC = pChunk;  // Chunk pointer. This is as the chunk is processed.

        if (moveToNextLineBeforeProcessing)
        {
            move_to_next_line:
            while (pC < pChunkEnd && pC[0] != '\n') {
                pC += 1;
            }

            if (pC == pChunkEnd) {
                // Ran out of data. Load the next chunk and keep going.
                moveToNextLineBeforeProcessing = DR_TRUE;
                continue;
            }

            pC += 1;     // pC[0] == '\n' - skip past the new line character.
            currentLine += 1;
            moveToNextLineBeforeProcessing = DR_FALSE;
        }

        if (skipWhitespaceBeforeProcessing)
        {
            while (pC < pChunkEnd && (pC[0] == ' ' || pC[0] == '\t' || pC[0] == '\r')) {
                pC += 1;
            }

            if (pC == pChunkEnd) {
                // Ran out of data.
                skipWhitespaceBeforeProcessing = DR_TRUE;
                continue;
            }

            skipWhitespaceBeforeProcessing = DR_FALSE;
        }


        // We loop character by character. When we run out of data, we start again.
        while (pC < pChunkEnd)
        {
            //// Key ////

            // Skip whitespace.
            while (pC < pChunkEnd && (pC[0] == ' ' || pC[0] == '\t' || pC[0] == '\r')) {
                pC += 1;
            }

            if (pC == pChunkEnd) {
                // Ran out of data.
                skipWhitespaceBeforeProcessing = DR_TRUE;
                continue;
            }

            if (pC[0] == '\n') {
                // Found the end of the line.
                pC += 1;
                currentLine += 1;
                continue;
            }

            if (pC[0] == '#') {
                // Found a comment. Move to the end of the line and continue.
                goto move_to_next_line;
            }

            char* pK = pC;
            while (pC < pChunkEnd && pC[0] != ' ' && pC[0] != '\t' && pC[0] != '\r' && pC[0] != '\n' && pC[0] != '#') {
                pC += 1;
            }

            if (pC == pChunkEnd)
            {
                // Ran out of data. We need to move what we have of the key to the start of the chunk buffer, and then read more data.
                if (chunkSize == sizeof(pChunk))
                {
                    size_t lineSizeSoFar = pC - pK;
                    memmove(pChunk, pK, lineSizeSoFar);

                    chunkSize = lineSizeSoFar + onRead(pUserData, pChunk + lineSizeSoFar, sizeof(pChunk) - lineSizeSoFar);
                    pChunkEnd = pChunk + chunkSize;

                    pK = pChunk;
                    pC = pChunk + lineSizeSoFar;
                    while (pC < pChunkEnd && pC[0] != ' ' && pC[0] != '\t' && pC[0] != '\r' && pC[0] != '\n' && pC[0] != '#') {
                        pC += 1;
                    }
                }

                if (pC == pChunkEnd) {
                    if (chunkSize == sizeof(pChunk)) {
                        if (onError) {
                            onError(pUserData, "Line is too long. A single line cannot exceed 4KB.", currentLine);
                        }

                        goto move_to_next_line;
                    } else {
                        // No more data. Just treat this one as a value-less key and return.
                        if (onPair) {
                            pC[0] = '\0';
                            onPair(pUserData, pK, NULL);
                        }

                        return;
                    }
                }
            }

            char* pKEnd = pC;

            //// Value ////

            // Skip whitespace.
            while (pC < pChunkEnd && (pC[0] == ' ' || pC[0] == '\t' || pC[0] == '\r')) {
                pC += 1;
            }

            if (pC == pChunkEnd)
            {
                // Ran out of data. We need to move what we have of the key to the start of the chunk buffer, and then read more data.
                if (chunkSize == sizeof(pChunk))
                {
                    size_t lineSizeSoFar = pC - pK;
                    memmove(pChunk, pK, lineSizeSoFar);

                    chunkSize = lineSizeSoFar + onRead(pUserData, pChunk + lineSizeSoFar, sizeof(pChunk) - lineSizeSoFar);
                    pChunkEnd = pChunk + chunkSize;

                    pKEnd = pChunk + (pKEnd - pK);
                    pK = pChunk;
                    pC = pChunk + lineSizeSoFar;
                    while (pC < pChunkEnd && (pC[0] == ' ' || pC[0] == '\t' || pC[0] == '\r')) {
                        pC += 1;
                    }
                }

                if (pC == pChunkEnd) {
                    if (chunkSize == sizeof(pChunk)) {
                        if (onError) {
                            onError(pUserData, "Line is too long. A single line cannot exceed 4KB.", currentLine);
                        }

                        goto move_to_next_line;
                    } else {
                        // No more data. Just treat this one as a value-less key and return.
                        if (onPair) {
                            pKEnd[0] = '\0';
                            onPair(pUserData, pK, NULL);
                        }

                        return;
                    }
                }
            }

            if (pC[0] == '\n') {
                // Found the end of the line. Treat it as a value-less key.
                pKEnd[0] = '\0';
                if (onPair) {
                    onPair(pUserData, pK, NULL);
                }

                pC += 1;
                currentLine += 1;
                continue;
            }

            if (pC[0] == '#') {
                // Found a comment. Treat is as a value-less key and move to the end of the line.
                pKEnd[0] = '\0';
                if (onPair) {
                    onPair(pUserData, pK, NULL);
                }

                goto move_to_next_line;
            }

            char* pV = pC;

            // Find the last non-whitespace character.
            char* pVEnd = pC;
            while (pC < pChunkEnd && pC[0] != '\n' && pC[0] != '#') {
                if (pC[0] != ' ' && pC[0] != '\t' && pC[0] != '\r') {
                    pVEnd = pC;
                }

                pC += 1;
            }

            if (pC == pChunkEnd)
            {
                // Ran out of data. We need to move what we have of the key to the start of the chunk buffer, and then read more data.
                if (chunkSize == sizeof(pChunk))
                {
                    size_t lineSizeSoFar = pC - pK;
                    memmove(pChunk, pK, lineSizeSoFar);

                    chunkSize = lineSizeSoFar + onRead(pUserData, pChunk + lineSizeSoFar, sizeof(pChunk) - lineSizeSoFar);
                    pChunkEnd = pChunk + chunkSize;

                    pVEnd = pChunk + (pVEnd - pK);
                    pKEnd = pChunk + (pKEnd - pK);
                    pV = pChunk + (pV - pK);
                    pK = pChunk;
                    pC = pChunk + lineSizeSoFar;
                    while (pC < pChunkEnd && pC[0] != '\n' && pC[0] != '#') {
                        if (pC[0] != ' ' && pC[0] != '\t' && pC[0] != '\r') {
                            pVEnd = pC;
                        }

                        pC += 1;
                    }
                }

                if (pC == pChunkEnd) {
                    if (chunkSize == sizeof(pChunk)) {
                        if (onError) {
                            onError(pUserData, "Line is too long. A single line cannot exceed 4KB.", currentLine);
                        }

                        goto move_to_next_line;
                    }
                }
            }


            // Before null-terminating the value we first need to determine how we'll proceed after posting onPair.
            dr_bool32 wasOnNL = pVEnd[1] == '\n';

            pKEnd[0] = '\0';
            pVEnd[1] = '\0';
            if (onPair) {
                onPair(pUserData, pK, pV);
            }

            if (wasOnNL)
            {
                // Was sitting on a new-line character.
                pC += 1;
                currentLine += 1;
                continue;
            }
            else
            {
                // Was sitting on a comment - just to the next line.
                goto move_to_next_line;
            }
        }
    }
}


typedef struct
{
    FILE* pFile;
    dr_key_value_pair_proc onPair;
    dr_key_value_error_proc onError;
    void* pOriginalUserData;
} dr_parse_key_value_pairs_from_file_data;

size_t dr_parse_key_value_pairs_from_file__on_read(void* pUserData, void* pDataOut, size_t bytesToRead)
{
    dr_parse_key_value_pairs_from_file_data* pData = (dr_parse_key_value_pairs_from_file_data*)pUserData;
    assert(pData != NULL);

    return fread(pDataOut, 1, bytesToRead, pData->pFile);
}

void dr_parse_key_value_pairs_from_file__on_pair(void* pUserData, const char* key, const char* value)
{
    dr_parse_key_value_pairs_from_file_data* pData = (dr_parse_key_value_pairs_from_file_data*)pUserData;
    assert(pData != NULL);

    pData->onPair(pData->pOriginalUserData, key, value);
}

void dr_parse_key_value_pairs_from_file__on_error(void* pUserData, const char* message, unsigned int line)
{
    dr_parse_key_value_pairs_from_file_data* pData = (dr_parse_key_value_pairs_from_file_data*)pUserData;
    assert(pData != NULL);

    pData->onError(pData->pOriginalUserData, message, line);
}

dr_bool32 dr_parse_key_value_pairs_from_file(const char* filePath, dr_key_value_pair_proc onPair, dr_key_value_error_proc onError, void* pUserData)
{
    dr_parse_key_value_pairs_from_file_data data;
    data.pFile = dr_fopen(filePath, "rb");
    if (data.pFile == NULL) {
        if (onError) onError(pUserData, "Could not open file.", 0);
        return DR_FALSE;
    }

    data.onPair = onPair;
    data.onError = onError;
    data.pOriginalUserData = pUserData;
    dr_parse_key_value_pairs(dr_parse_key_value_pairs_from_file__on_read, dr_parse_key_value_pairs_from_file__on_pair, dr_parse_key_value_pairs_from_file__on_error, &data);

    fclose(data.pFile);
    return DR_TRUE;
}


/////////////////////////////////////////////////////////
// Basic Tokenizer

const char* dr_next_token(const char* tokens, char* tokenOut, size_t tokenOutSize)
{
    if (tokenOut) tokenOut[0] = '\0';

    if (tokens == NULL) {
        return NULL;
    }

    // Skip past leading whitespace.
    while (tokens[0] != '\0' && !(tokens[0] != ' ' && tokens[0] != '\t' && tokens[0] != '\n' && tokens[0] != '\v' && tokens[0] != '\f' && tokens[0] != '\r')) {
        tokens += 1;
    }

    if (tokens[0] == '\0') {
        return NULL;
    }


    const char* strBeg = tokens;
    const char* strEnd = strBeg;

    if (strEnd[0] == '\"')
    {
        // It's double-quoted - loop until the next unescaped quote character.

        // Skip past the first double-quote character.
        strBeg += 1;
        strEnd += 1;

        // Keep looping until the next unescaped double-quote character.
        char prevChar = '\0';
        while (strEnd[0] != '\0' && (strEnd[0] != '\"' || prevChar == '\\'))
        {
            prevChar = strEnd[0];
            strEnd += 1;
        }
    }
    else
    {
        // It's not double-quoted - just loop until the first whitespace.
        while (strEnd[0] != '\0' && (strEnd[0] != ' ' && strEnd[0] != '\t' && strEnd[0] != '\n' && strEnd[0] != '\v' && strEnd[0] != '\f' && strEnd[0] != '\r')) {
            strEnd += 1;
        }
    }


    // If the output buffer is large enough to hold the token, copy the token into it. When we copy the token we need to
    // ensure we don't include the escape character.
    //assert(strEnd >= strBeg);

    while (tokenOutSize > 1 && strBeg < strEnd)
    {
        if (strBeg[0] == '\\' && strBeg[1] == '\"' && strBeg < strEnd) {
            strBeg += 1;
        }

        *tokenOut++ = *strBeg++;
        tokenOutSize -= 1;
    }

    // Null-terminate.
    if (tokenOutSize > 0) {
        *tokenOut = '\0';
    }


    // Skip past the double-quote character before returning.
    if (strEnd[0] == '\"') {
        strEnd += 1;
    }

    return strEnd;
}




/////////////////////////////////////////////////////////
// Known Folders

#if defined(_WIN32)
#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4091)   // 'typedef ': ignored on left of 'tagGPFIDL_FLAGS' when no variable is declared
#endif
#include <shlobj.h>
#if defined(_MSC_VER)
    #pragma warning(pop)
#endif


dr_bool32 dr_get_executable_path(char* pathOut, size_t pathOutSize)
{
    if (pathOut == NULL || pathOutSize == 0) {
        return 0;
    }

    DWORD length = GetModuleFileNameA(NULL, pathOut, (DWORD)pathOutSize);
    if (length == 0) {
        pathOut[0] = '\0';
        return DR_FALSE;
    }

    // Force null termination.
    if (length == pathOutSize) {
        pathOut[length - 1] = '\0';
    }

    // Back slashes need to be normalized to forward.
    while (pathOut[0] != '\0') {
        if (pathOut[0] == '\\') {
            pathOut[0] = '/';
        }

        pathOut += 1;
    }

    return DR_TRUE;
}

dr_bool32 dr_get_config_folder_path(char* pathOut, size_t pathOutSize)
{
    // The documentation for SHGetFolderPathA() says that the output path should be the size of MAX_PATH. We'll enforce
    // that just to be safe.
    if (pathOutSize >= MAX_PATH)
    {
        SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, pathOut);
    }
    else
    {
        char pathOutTemp[MAX_PATH];
        SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, pathOutTemp);

        if (strcpy_s(pathOut, pathOutSize, pathOutTemp) != 0) {
            return 0;
        }
    }


    // Back slashes need to be normalized to forward.
    while (pathOut[0] != '\0') {
        if (pathOut[0] == '\\') {
            pathOut[0] = '/';
        }

        pathOut += 1;
    }

    return 1;
}

dr_bool32 dr_get_log_folder_path(char* pathOut, size_t pathOutSize)
{
    return dr_get_config_folder_path(pathOut, pathOutSize);
}

const char* dr_get_current_directory(char* pathOut, size_t pathOutSize)
{
    DWORD result = GetCurrentDirectoryA((DWORD)pathOutSize, pathOut);
    if (result == 0) {
        return NULL;
    }

    return pathOut;
}

dr_bool32 dr_set_current_directory(const char* path)
{
    return SetCurrentDirectoryA(path) != 0;
}
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

dr_bool32 dr_get_executable_path(char* pathOut, size_t pathOutSize)
{
    if (pathOut == NULL || pathOutSize == 0) {
        return 0;
    }

    ssize_t length = readlink("/proc/self/exe", pathOut, pathOutSize);
    if (length == -1) {
        pathOut[0] = '\0';
        return DR_FALSE;
    }

    if ((size_t)length == pathOutSize) {
        pathOut[length - 1] = '\0';
    } else {
        pathOut[length] = '\0';
    }

    return DR_TRUE;
}

dr_bool32 dr_get_config_folder_path(char* pathOut, size_t pathOutSize)
{
    const char* configdir = getenv("XDG_CONFIG_HOME");
    if (configdir != NULL)
    {
        return strcpy_s(pathOut, pathOutSize, configdir) == 0;
    }
    else
    {
        const char* homedir = getenv("HOME");
        if (homedir == NULL) {
            homedir = getpwuid(getuid())->pw_dir;
        }

        if (homedir != NULL)
        {
            if (strcpy_s(pathOut, pathOutSize, homedir) == 0)
            {
                size_t homedirLength = strlen(homedir);
                pathOut     += homedirLength;
                pathOutSize -= homedirLength;

                if (pathOutSize > 0)
                {
                    pathOut[0] = '/';
                    pathOut     += 1;
                    pathOutSize -= 1;

                    return strcpy_s(pathOut, pathOutSize, ".config") == 0;
                }
            }
        }
    }

    return 0;
}

dr_bool32 dr_get_log_folder_path(char* pathOut, size_t pathOutSize)
{
    return strcpy_s(pathOut, pathOutSize, "var/log") == 0;
}

const char* dr_get_current_directory(char* pathOut, size_t pathOutSize)
{
    return getcwd(pathOut, pathOutSize);
}

dr_bool32 dr_set_current_directory(const char* path)
{
    return chdir(path) == 0;
}
#endif

dr_bool32 dr_get_executable_directory_path(char* pathOut, size_t pathOutSize)
{
    if (!dr_get_executable_path(pathOut, pathOutSize)) {
        return DR_FALSE;
    }

    // A null terminator needs to be placed at the last slash.
    char* lastSlash = pathOut;
    while (pathOut[0] != '\0') {
        if (pathOut[0] == '/' || pathOut[0] == '\\') {
            lastSlash = pathOut;
        }
        pathOut += 1;
    }

    lastSlash[0] = '\0';
    return DR_TRUE;
}



/////////////////////////////////////////////////////////
// Basic File Management

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

FILE* dr_fopen(const char* filePath, const char* openMode)
{
    FILE* pFile;
#ifdef _MSC_VER
    if (fopen_s(&pFile, filePath, openMode) != 0) {
        return NULL;
    }
#else
    pFile = fopen(filePath, openMode);
    if (pFile == NULL) {
        return NULL;
    }
#endif

    return pFile;
}

dr_bool32 dr_create_empty_file(const char* fileName, dr_bool32 failIfExists)
{
    if (fileName == NULL) {
        return DR_FALSE;
    }
#ifdef _WIN32
    DWORD dwCreationDisposition;
    if (failIfExists) {
        dwCreationDisposition = CREATE_NEW;
    } else {
        dwCreationDisposition = CREATE_ALWAYS;
    }

    HANDLE hFile = CreateFileA(fileName, FILE_GENERIC_WRITE, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return DR_FALSE;
    }

    CloseHandle(hFile);
    return DR_TRUE;
#else
    int flags = O_WRONLY | O_CREAT;
    if (failIfExists) {
        flags |= O_EXCL;
    } else {
        flags |= O_TRUNC;
    }
    int fd = open(fileName, flags, 0666);
    if (fd == -1) {
        return DR_FALSE;
    }

    close(fd);
    return DR_TRUE;
#endif
}

static void* dr_open_and_read_file_with_extra_data(const char* filePath, size_t* pFileSizeOut, size_t extraBytes)
{
    if (pFileSizeOut) *pFileSizeOut = 0;   // For safety.

    if (filePath == NULL) {
        return NULL;
    }

    // TODO: Use 64-bit versions of the FILE APIs.

    FILE* pFile = dr_fopen(filePath, "rb");
    if (pFile == NULL) {
        return NULL;
    }

    fseek(pFile, 0, SEEK_END);
    dr_uint64 fileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    if (fileSize + extraBytes > SIZE_MAX) {
        fclose(pFile);
        return NULL;    // File is too big.
    }

    void* pFileData = malloc((size_t)fileSize + extraBytes);    // <-- Safe cast due to the check above.
    if (pFileData == NULL) {
        fclose(pFile);
        return NULL;    // Failed to allocate memory for the file. Good chance the file is too big.
    }

    size_t bytesRead = fread(pFileData, 1, (size_t)fileSize, pFile);
    if (bytesRead != fileSize) {
        free(pFileData);
        fclose(pFile);
        return NULL;    // Failed to read every byte from the file.
    }

    fclose(pFile);

    if (pFileSizeOut) *pFileSizeOut = (size_t)fileSize;
    return pFileData;
}

void* dr_open_and_read_file(const char* filePath, size_t* pFileSizeOut)
{
    return dr_open_and_read_file_with_extra_data(filePath, pFileSizeOut, 0);
}

char* dr_open_and_read_text_file(const char* filePath, size_t* pFileSizeOut)
{
    if (pFileSizeOut) *pFileSizeOut = 0;   // For safety.

    size_t fileSize;
    char* pFileData = (char*)dr_open_and_read_file_with_extra_data(filePath, &fileSize, 1);     // <-- 1 extra byte for the null terminator.
    if (pFileData == NULL) {
        return NULL;
    }

    pFileData[fileSize] = '\0';

    if (pFileSizeOut) *pFileSizeOut = fileSize;
    return pFileData;
}

dr_bool32 dr_open_and_write_file(const char* filePath, const void* pData, size_t dataSize)
{
    if (filePath == NULL) {
        return DR_FALSE;
    }

    // TODO: Use 64-bit versions of the FILE APIs.

    FILE* pFile = dr_fopen(filePath, "wb");
    if (pFile == NULL) {
        return DR_FALSE;
    }

    if (pData != NULL && dataSize > 0) {
        fwrite(pData, 1, dataSize, pFile);
    }

    fclose(pFile);
    return DR_TRUE;
}

dr_bool32 dr_open_and_write_text_file(const char* filePath, const char* text)
{
    if (text == NULL) {
        text = "";
    }

    return dr_open_and_write_file(filePath, text, strlen(text));
}

void dr_free_file_data(void* valueReturnedByOpenAndReadFile)
{
    free(valueReturnedByOpenAndReadFile);
}

dr_bool32 dr_file_exists(const char* filePath)
{
    if (filePath == NULL) {
        return DR_FALSE;
    }

#if _WIN32
    DWORD attributes = GetFileAttributesA(filePath);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
    struct stat info;
    if (stat(filePath, &info) != 0) {
        return DR_FALSE;   // Likely the folder doesn't exist.
    }

    return (info.st_mode & S_IFDIR) == 0;
#endif
}

dr_bool32 dr_directory_exists(const char* directoryPath)
{
    if (directoryPath == NULL) {
        return DR_FALSE;
    }

#if _WIN32
    DWORD attributes = GetFileAttributesA(directoryPath);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    struct stat info;
    if (stat(directoryPath, &info) != 0) {
        return DR_FALSE;   // Likely the folder doesn't exist.
    }

    return (info.st_mode & S_IFDIR) != 0;
#endif
}

dr_bool32 dr_move_file(const char* oldPath, const char* newPath)
{
    if (oldPath == NULL || newPath == NULL) {
        return DR_FALSE;
    }

#if _WIN32
    return MoveFileExA(oldPath, newPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return rename(oldPath, newPath) == 0;
#endif
}

dr_bool32 dr_copy_file(const char* srcPath, const char* dstPath, dr_bool32 failIfExists)
{
    if (srcPath == NULL || dstPath == NULL) {
        return DR_FALSE;
    }

#if _WIN32
    return CopyFileA(srcPath, dstPath, failIfExists) != 0;
#else
    int fdSrc = open(srcPath, O_RDONLY, 0666);
    if (fdSrc == -1) {
        return DR_FALSE;
    }

    int fdDst = open(dstPath, O_WRONLY | O_TRUNC | O_CREAT | ((failIfExists) ? O_EXCL : 0), 0666);
    if (fdDst == -1) {
        close(fdSrc);
        return DR_FALSE;
    }

    dr_bool32 result = DR_TRUE;
    struct stat info;
    if (fstat(fdSrc, &info) == 0) {
        char buffer[BUFSIZ];
        int bytesRead;
        while ((bytesRead = read(fdSrc, buffer, sizeof(buffer))) > 0) {
            if (write(fdDst, buffer, bytesRead) != bytesRead) {
                result = DR_FALSE;
                break;
            }
        }
    } else {
        result = DR_FALSE;
    }

    close(fdDst);
    close(fdSrc);

    // Permissions.
    chmod(dstPath, info.st_mode & 07777);

    return result;
#endif
}

dr_bool32 dr_is_file_read_only(const char* filePath)
{
    if (filePath == NULL || filePath[0] == '\0') {
        return DR_FALSE;
    }

#if _WIN32
    DWORD attributes = GetFileAttributesA(filePath);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_READONLY) != 0;
#else
    return access(filePath, W_OK) == -1;
#endif
}

dr_uint64 dr_get_file_modified_time(const char* filePath)
{
    if (filePath == NULL || filePath[0] == '\0') {
        return 0;
    }

#if _WIN32
    HANDLE hFile = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return 0;
    }

    FILETIME fileTime;
    BOOL wasSuccessful = GetFileTime(hFile, NULL, NULL, &fileTime);
    CloseHandle(hFile);

    if (!wasSuccessful) {
        return 0;
    }

    ULARGE_INTEGER result;
    result.HighPart = fileTime.dwHighDateTime;
    result.LowPart = fileTime.dwLowDateTime;
    return result.QuadPart;
#else
    struct stat info;
    if (stat(filePath, &info) != 0) {
        return 0;
    }

    return info.st_mtime;
#endif
}

dr_bool32 dr_delete_file(const char* filePath)
{
    if (filePath == NULL) {
        return DR_FALSE;
    }

#if _WIN32
    return DeleteFileA(filePath) != 0;
#else
    return remove(filePath) == 0;
#endif
}

dr_bool32 dr_mkdir(const char* directoryPath)
{
    if (directoryPath == NULL) {
        return DR_FALSE;
    }

#if _WIN32
    return CreateDirectoryA(directoryPath, NULL) != 0;
#else
    return mkdir(directoryPath, 0777) == 0;
#endif
}

dr_bool32 dr_mkdir_recursive(const char* directoryPath)
{
    if (directoryPath == NULL || directoryPath[0] == '\0') {
        return DR_FALSE;
    }

    // All we need to do is iterate over every segment in the path and try creating the directory.
    char runningPath[4096];
    memset(runningPath, 0, sizeof(runningPath));

    size_t i = 0;
    for (;;) {
        if (i >= sizeof(runningPath)-1) {
            return DR_FALSE;   // Path is too long.
        }

        if (directoryPath[0] == '\0' || directoryPath[0] == '/' || directoryPath[0] == '\\') {
            if (runningPath[0] != '\0' && !(runningPath[1] == ':' && runningPath[2] == '\0')) {   // <-- If the running path is empty, it means we're trying to create the root directory.
                if (!dr_directory_exists(runningPath)) {
                    if (!dr_mkdir(runningPath)) {
                        return DR_FALSE;
                    }
                }
            }

            //printf("%s\n", runningPath);
            runningPath[i++] = '/';
            runningPath[i]   = '\0';

            if (directoryPath[0] == '\0') {
                break;
            }
        } else {
            runningPath[i++] = directoryPath[0];
        }

        directoryPath += 1;
    }

    return DR_TRUE;
}

dr_bool32 dr_iterate_files(const char* directory, dr_bool32 recursive, dr_iterate_files_proc proc, void* pUserData)
{
#ifdef _WIN32
    char searchQuery[MAX_PATH];
    strcpy_s(searchQuery, sizeof(searchQuery), directory);

    unsigned int searchQueryLength = (unsigned int)strlen(searchQuery);
    if (searchQueryLength >= MAX_PATH - 3) {
        return DR_FALSE;    // Path is too long.
    }

    searchQuery[searchQueryLength + 0] = '\\';
    searchQuery[searchQueryLength + 1] = '*';
    searchQuery[searchQueryLength + 2] = '\0';

    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA(searchQuery, &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        return DR_FALSE; // Failed to begin search.
    }

    do
    {
        // Skip past "." and ".." directories.
        if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0) {
            continue;
        }

        char filePath[MAX_PATH];
        strcpy_s(filePath, sizeof(filePath), directory);
        strcat_s(filePath, sizeof(filePath), "/");
        strcat_s(filePath, sizeof(filePath), ffd.cFileName);

        if (!proc(filePath, pUserData)) {
            return DR_FALSE;
        }

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (recursive) {
                if (!dr_iterate_files(filePath, recursive, proc, pUserData)) {
                    return DR_FALSE;
                }
            }
        }

    } while (FindNextFileA(hFind, &ffd));

    FindClose(hFind);
#else
    DIR* dir = opendir(directory);
    if (dir == NULL) {
        return DR_FALSE;
    }

    struct dirent* info = NULL;
    while ((info = readdir(dir)) != NULL)
    {
        // Skip past "." and ".." directories.
        if (strcmp(info->d_name, ".") == 0 || strcmp(info->d_name, "..") == 0) {
            continue;
        }

        char filePath[4096];
        strcpy_s(filePath, sizeof(filePath), directory);
        strcat_s(filePath, sizeof(filePath), "/");
        strcat_s(filePath, sizeof(filePath), info->d_name);

        struct stat fileinfo;
        if (stat(filePath, &fileinfo) != 0) {
            continue;
        }

        if (!proc(filePath, pUserData)) {
            return DR_FALSE;
        }

        if (fileinfo.st_mode & S_IFDIR) {
            if (recursive) {
                if (!dr_iterate_files(filePath, recursive, proc, pUserData)) {
                    return DR_FALSE;
                }
            }
        }
    }

    closedir(dir);
#endif

    return DR_TRUE;
}



/////////////////////////////////////////////////////////
// DPI Awareness

#if defined(_WIN32) || defined(_WIN64)

typedef enum PROCESS_DPI_AWARENESS {
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

typedef enum MONITOR_DPI_TYPE {
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI = 1,
    MDT_RAW_DPI = 2,
    MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;

typedef BOOL    (__stdcall * PFN_SetProcessDPIAware)     (void);
typedef HRESULT (__stdcall * PFN_SetProcessDpiAwareness) (PROCESS_DPI_AWARENESS);
typedef HRESULT (__stdcall * PFN_GetDpiForMonitor)       (HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);

void dr_win32_make_dpi_aware()
{
    dr_bool32 fallBackToDiscouragedAPI = DR_FALSE;

    // We can't call SetProcessDpiAwareness() directly because otherwise on versions of Windows < 8.1 we'll get an error at load time about
    // a missing DLL.
    HMODULE hSHCoreDLL = LoadLibraryW(L"shcore.dll");
    if (hSHCoreDLL != NULL)
    {
        PFN_SetProcessDpiAwareness _SetProcessDpiAwareness = (PFN_SetProcessDpiAwareness)GetProcAddress(hSHCoreDLL, "SetProcessDpiAwareness");
        if (_SetProcessDpiAwareness != NULL)
        {
            if (_SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE) != S_OK)
            {
                fallBackToDiscouragedAPI = DR_FALSE;
            }
        }
        else
        {
            fallBackToDiscouragedAPI = DR_FALSE;
        }

        FreeLibrary(hSHCoreDLL);
    }
    else
    {
        fallBackToDiscouragedAPI = DR_FALSE;
    }


    if (fallBackToDiscouragedAPI)
    {
        HMODULE hUser32DLL = LoadLibraryW(L"user32.dll");
        if (hUser32DLL != NULL)
        {
            PFN_SetProcessDPIAware _SetProcessDPIAware = (PFN_SetProcessDPIAware)GetProcAddress(hUser32DLL, "SetProcessDPIAware");
            if (_SetProcessDPIAware != NULL) {
                _SetProcessDPIAware();
            }

            FreeLibrary(hUser32DLL);
        }
    }
}

void dr_win32_get_base_dpi(int* pDPIXOut, int* pDPIYOut)
{
    if (pDPIXOut != NULL) {
        *pDPIXOut = 96;
    }

    if (pDPIYOut != NULL) {
        *pDPIYOut = 96;
    }
}

void dr_win32_get_system_dpi(int* pDPIXOut, int* pDPIYOut)
{
    if (pDPIXOut != NULL) {
        *pDPIXOut = GetDeviceCaps(GetDC(NULL), LOGPIXELSX);
    }

    if (pDPIYOut != NULL) {
        *pDPIYOut = GetDeviceCaps(GetDC(NULL), LOGPIXELSY);
    }
}


typedef struct
{
    int monitorIndex;
    int i;
    int dpiX;
    int dpiY;
    PFN_GetDpiForMonitor _GetDpiForMonitor;

} win32_get_monitor_dpi_data;

static BOOL CALLBACK win32_get_monitor_dpi_callback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    (void)hdcMonitor;
    (void)lprcMonitor;

    win32_get_monitor_dpi_data* pData = (win32_get_monitor_dpi_data*)dwData;
    if (pData->monitorIndex == pData->i)
    {
        UINT dpiX;
        UINT dpiY;
        if (pData->_GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY) == S_OK)
        {
            pData->dpiX = (int)dpiX;
            pData->dpiY = (int)dpiY;
        }
        else
        {
            dr_win32_get_system_dpi(&pData->dpiX, &pData->dpiY);
        }

        return FALSE;   // Return DR_FALSE to terminate the enumerator.
    }

    pData->i += 1;
    return TRUE;
}

void dr_win32_get_monitor_dpi(int monitor, int* pDPIXOut, int* pDPIYOut)
{
    // If multi-monitor DPI awareness is not supported we will need to fall back to system DPI.
    HMODULE hSHCoreDLL = LoadLibraryW(L"shcore.dll");
    if (hSHCoreDLL == NULL) {
        dr_win32_get_system_dpi(pDPIXOut, pDPIYOut);
        return;
    }

    PFN_GetDpiForMonitor _GetDpiForMonitor = (PFN_GetDpiForMonitor)GetProcAddress(hSHCoreDLL, "GetDpiForMonitor");
    if (_GetDpiForMonitor == NULL) {
        dr_win32_get_system_dpi(pDPIXOut, pDPIYOut);
        FreeLibrary(hSHCoreDLL);
        return;
    }


    win32_get_monitor_dpi_data data;
    data.monitorIndex = monitor;
    data.i = 0;
    data.dpiX = 0;
    data.dpiY = 0;
    data._GetDpiForMonitor = _GetDpiForMonitor;
    EnumDisplayMonitors(NULL, NULL, win32_get_monitor_dpi_callback, (LPARAM)&data);

    if (pDPIXOut) {
        *pDPIXOut = data.dpiX;
    }

    if (pDPIYOut) {
        *pDPIYOut = data.dpiY;
    }


    FreeLibrary(hSHCoreDLL);
}


static BOOL CALLBACK win32_get_monitor_count_callback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    (void)hMonitor;
    (void)hdcMonitor;
    (void)lprcMonitor;

    int *count = (int*)dwData;
    (*count)++;

    return TRUE;
}

int dr_win32_get_monitor_count()
{
    int count = 0;
    if (EnumDisplayMonitors(NULL, NULL, win32_get_monitor_count_callback, (LPARAM)&count)) {
        return count;
    }

    return 0;
}
#endif


/////////////////////////////////////////////////////////
// Date / Time

time_t dr_now()
{
    return time(NULL);
}

void dr_datetime_short(time_t t, char* strOut, unsigned int strOutSize)
{
#if defined(_MSC_VER)
	struct tm local;
	localtime_s(&local, &t);
    strftime(strOut, strOutSize, "%x %H:%M:%S", &local);
#else
	struct tm *local = localtime(&t);
	strftime(strOut, strOutSize, "%x %H:%M:%S", local);
#endif
}

void dr_date_YYYYMMDD(time_t t, char* strOut, unsigned int strOutSize)
{
#if defined(_MSC_VER)
	struct tm local;
	localtime_s(&local, &t);
    strftime(strOut, strOutSize, "%Y%m%d", &local);
#else
	struct tm *local = localtime(&t);
	strftime(strOut, strOutSize, "%Y%m%d", local);
#endif
}



/////////////////////////////////////////////////////////
// Command Line

typedef struct
{
    dr_cmdline* pCmdLine;
    char* value;

    // Win32 style data.
    char* win32_payload;
    char* valueEnd;

    // argv style data.
    int iarg;   // <-- This starts at -1 so that the first call to next() increments it to 0.

} dr_cmdline_iterator;

dr_cmdline_iterator dr_cmdline_begin(dr_cmdline* pCmdLine)
{
    dr_cmdline_iterator i;
    i.pCmdLine      = pCmdLine;
    i.value         = NULL;
    i.win32_payload = NULL;
    i.valueEnd      = NULL;
    i.iarg          = -1;

    if (pCmdLine != NULL && pCmdLine->win32 != NULL) {
        // Win32 style
        size_t length = strlen(pCmdLine->win32);
        i.win32_payload = (char*)malloc(length + 2);         // +2 for a double null terminator.
        strcpy_s(i.win32_payload, length + 2, pCmdLine->win32);
        i.win32_payload[length + 1] = '\0';

        i.valueEnd = i.win32_payload;
    }

    return i;
}

dr_bool32 dr_cmdline_next(dr_cmdline_iterator* i)
{
    if (i != NULL && i->pCmdLine != NULL)
    {
        if (i->pCmdLine->win32 != NULL)
        {
            // Win32 style
            if (i->value == NULL) {
                i->value    = i->win32_payload;
                i->valueEnd = i->value;
            } else {
                i->value = i->valueEnd + 1;
            }


            // Move to the start of the next argument.
            while (i->value[0] == ' ') {
                i->value += 1;
            }


            // If at this point we are sitting on the null terminator it means we have finished iterating.
            if (i->value[0] == '\0')
            {
                free(i->win32_payload);
                i->win32_payload = NULL;
                i->pCmdLine      = NULL;
                i->value         = NULL;
                i->valueEnd      = NULL;

                return DR_FALSE;
            }


            // Move to the end of the token. If the argument begins with a double quote, we iterate until we find
            // the next unescaped double-quote.
            if (i->value[0] == '\"')
            {
                // Go to the last unescaped double-quote.
                i->value += 1;
                i->valueEnd = i->value + 1;

                while (i->valueEnd[0] != '\0' && i->valueEnd[0] != '\"')
                {
                    if (i->valueEnd[0] == '\\') {
                        i->valueEnd += 1;

                        if (i->valueEnd[0] == '\0') {
                            break;
                        }
                    }

                    i->valueEnd += 1;
                }
                i->valueEnd[0] = '\0';
            }
            else
            {
                // Go to the next space.
                i->valueEnd = i->value + 1;

                while (i->valueEnd[0] != '\0' && i->valueEnd[0] != ' ')
                {
                    i->valueEnd += 1;
                }
                i->valueEnd[0] = '\0';
            }

            return DR_TRUE;
        }
        else
        {
            // argv style
            i->iarg += 1;
            if (i->iarg < i->pCmdLine->argc)
            {
                i->value = i->pCmdLine->argv[i->iarg];
                return DR_TRUE;
            }
            else
            {
                i->value = NULL;
                return DR_FALSE;
            }
        }
    }

    return DR_FALSE;
}


dr_bool32 dr_init_cmdline(dr_cmdline* pCmdLine, int argc, char** argv)
{
    if (pCmdLine == NULL) {
        return DR_FALSE;
    }

    pCmdLine->argc  = argc;
    pCmdLine->argv  = argv;
    pCmdLine->win32 = NULL;

    return DR_TRUE;
}

dr_bool32 dr_init_cmdline_win32(dr_cmdline* pCmdLine, const char* args)
{
    if (pCmdLine == NULL) {
        return DR_FALSE;
    }

    pCmdLine->argc  = 0;
    pCmdLine->argv  = NULL;
    pCmdLine->win32 = args;

    return DR_TRUE;
}

void dr_parse_cmdline(dr_cmdline* pCmdLine, dr_cmdline_parse_proc callback, void* pUserData)
{
    if (pCmdLine == NULL || callback == NULL) {
        return;
    }


    char pTemp[2] = {0};

    char* pKey = NULL;
    char* pVal = NULL;

    dr_cmdline_iterator arg = dr_cmdline_begin(pCmdLine);
    if (dr_cmdline_next(&arg))
    {
        if (!callback("[path]", arg.value, pUserData)) {
            return;
        }
    }

    while (dr_cmdline_next(&arg))
    {
        if (arg.value[0] == '-')
        {
            // key

            // If the key is non-null, but the value IS null, it means we hit a key with no value in which case it will not yet have been posted.
            if (pKey != NULL && pVal == NULL)
            {
                if (!callback(pKey, pVal, pUserData)) {
                    return;
                }

                pKey = NULL;
            }
            else
            {
                // Need to ensure the key and value are reset before doing any further processing.
                pKey = NULL;
                pVal = NULL;
            }



            if (arg.value[1] == '-')
            {
                // --argument style
                pKey = arg.value + 2;
            }
            else
            {
                // -a -b -c -d or -abcd style
                if (arg.value[1] != '\0')
                {
                    if (arg.value[2] == '\0')
                    {
                        // -a -b -c -d style
                        pTemp[0] = arg.value[1];
                        pKey = pTemp;
                        pVal = NULL;
                    }
                    else
                    {
                        // -abcd style.
                        int i = 1;
                        while (arg.value[i] != '\0')
                        {
                            pTemp[0] = arg.value[i];

                            if (!callback(pTemp, NULL, pUserData)) {
                                return;
                            }

                            pKey = NULL;
                            pVal = NULL;

                            i += 1;
                        }
                    }
                }
            }
        }
        else
        {
            // value

            pVal = arg.value;
            if (!callback(pKey, pVal, pUserData)) {
                return;
            }
        }
    }


    // There may be a key without a value that needs posting.
    if (pKey != NULL && pVal == NULL) {
        callback(pKey, pVal, pUserData);
    }
}

typedef struct
{
    dr_bool32 exists;
    const char* key;
} dr_cmdline_key_exists_data;

dr_bool32 dr_cmdline_key_exists_callback(const char* key, const char* value, void* pUserData)
{
    (void)value;

    dr_cmdline_key_exists_data* pData = (dr_cmdline_key_exists_data*)pUserData;
    assert(pData != NULL);

    if (key != NULL && strcmp(pData->key, key) == 0) {
        pData->exists = DR_TRUE;
        return DR_FALSE;
    }

    return DR_TRUE;
}

dr_bool32 dr_cmdline_key_exists(dr_cmdline* pCmdLine, const char* key)
{
    dr_cmdline_key_exists_data data;
    data.exists = DR_FALSE;
    data.key = key;
    dr_parse_cmdline(pCmdLine, dr_cmdline_key_exists_callback, &data);

    return data.exists;
}

int dr_cmdline_to_argv(dr_cmdline* pCmdLine, char*** argvOut)
{
    if (argvOut == NULL) return 0;
    *argvOut = NULL;    // Safety.

    int argc = 0;
    char** argv = NULL;
    size_t cmdlineLen = 0;

    // The command line is parsed in 2 passes. The first pass simple calculates the required sizes of each buffer. The second
    // pass fills those buffers with actual data.

    // First pass.
    dr_cmdline_iterator arg = dr_cmdline_begin(pCmdLine);
    while (dr_cmdline_next(&arg)) {
        cmdlineLen += strlen(arg.value) + 1;    // +1 for null terminator.
        argc += 1;
    }

    if (argc == 0) {
        return 0;
    }


    // The entire data for the command line is stored in a single buffer.
    char* data = (char*)malloc((argc * sizeof(char**)) + (cmdlineLen * sizeof(char)));
    if (data == NULL) {
        return 0;   // Ran out of memory.
    }

    argv = (char**)data;
    char* cmdlineStr = data + (argc * sizeof(char**));
    


    // Second pass.
    argc = 0;
    cmdlineLen = 0;

    arg = dr_cmdline_begin(pCmdLine);
    while (dr_cmdline_next(&arg)) {
        argv[argc] = cmdlineStr + cmdlineLen;
        
        int i = 0;
        while (arg.value[i] != '\0') {
            argv[argc][i] = arg.value[i];
            i += 1;
        }
        argv[argc][i] = '\0';


        cmdlineLen += strlen(arg.value) + 1;    // +1 for null terminator.
        argc += 1;
    }


    *argvOut = argv;
    return argc;
}

int dr_winmain_to_argv(const char* cmdlineWinMain, char*** argvOut)
{
    dr_cmdline cmdline;
    if (!dr_init_cmdline_win32(&cmdline, cmdlineWinMain)) {
        return 0;
    }

    return dr_cmdline_to_argv(&cmdline, argvOut);
}

void dr_free_argv(char** argv)
{
    if (argv == NULL) {
        return;
    }

    free(argv);
}



/////////////////////////////////////////////////////////
// Threading

#if defined(_WIN32)
void dr_sleep(unsigned int milliseconds)
{
    Sleep((DWORD)milliseconds);
}

void dr_yield()
{
    SwitchToThread();
}

unsigned int dr_get_logical_processor_count()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);

    return (unsigned int)sysinfo.dwNumberOfProcessors;
}


typedef struct
{
    /// The Win32 thread handle.
    HANDLE hThread;

    /// The entry point.
    dr_thread_entry_proc entryProc;

    /// The user data to pass to the thread's entry point.
    void* pData;

    /// Set to DR_TRUE by the entry function. We use this to wait for the entry function to start.
    dr_bool32 isInEntryProc;

} dr_thread_win32;

static DWORD WINAPI dr_thread_entry_proc_win32(LPVOID pUserData)
{
    dr_thread_win32* pThreadWin32 = (dr_thread_win32*)pUserData;
    assert(pThreadWin32 != NULL);

    void* pEntryProcData = pThreadWin32->pData;
    dr_thread_entry_proc entryProc = pThreadWin32->entryProc;
    assert(entryProc != NULL);

    pThreadWin32->isInEntryProc = DR_TRUE;

    return (DWORD)entryProc(pEntryProcData);
}

dr_thread dr_create_thread(dr_thread_entry_proc entryProc, void* pData)
{
    if (entryProc == NULL) {
        return NULL;
    }

    dr_thread_win32* pThreadWin32 = (dr_thread_win32*)malloc(sizeof(*pThreadWin32));
    if (pThreadWin32 != NULL)
    {
        pThreadWin32->entryProc     = entryProc;
        pThreadWin32->pData         = pData;
        pThreadWin32->isInEntryProc = DR_FALSE;

        pThreadWin32->hThread = CreateThread(NULL, 0, dr_thread_entry_proc_win32, pThreadWin32, 0, NULL);
        if (pThreadWin32 == NULL) {
            free(pThreadWin32);
            return NULL;
        }

        // Wait for the new thread to enter into it's entry point before returning. We need to do this so we can safely
        // support something like dr_delete_thread(dr_create_thread(my_thread_proc, pData)).
        //
        // On Win32 there are times when this can get stuck in an infinite loop - I expect it's something to do with some
        // bad scheduling by the OS. This can be "fixed" by sleeping for a bit.
        while (!pThreadWin32->isInEntryProc) { dr_sleep(0); }
    }

    return (dr_thread)pThreadWin32;
}

void dr_delete_thread(dr_thread thread)
{
    dr_thread_win32* pThreadWin32 = (dr_thread_win32*)thread;
    if (pThreadWin32 != NULL)
    {
        CloseHandle(pThreadWin32->hThread);
    }

    free(pThreadWin32);
}

void dr_wait_thread(dr_thread thread)
{
    dr_thread_win32* pThreadWin32 = (dr_thread_win32*)thread;
    if (pThreadWin32 != NULL)
    {
        WaitForSingleObject(pThreadWin32->hThread, INFINITE);
    }
}

void dr_wait_and_delete_thread(dr_thread thread)
{
    dr_wait_thread(thread);
    dr_delete_thread(thread);
}


#ifdef DR_UTIL_WIN32_USE_CRITICAL_SECTION_MUTEX
dr_mutex dr_create_mutex()
{
    dr_mutex mutex = malloc(sizeof(CRITICAL_SECTION));
    if (mutex != NULL)
    {
        InitializeCriticalSection(mutex);
    }

    return mutex;
}

void dr_delete_mutex(dr_mutex mutex)
{
    DeleteCriticalSection(mutex);
    free(mutex);
}

void dr_lock_mutex(dr_mutex mutex)
{
    EnterCriticalSection(mutex);
}

void dr_unlock_mutex(dr_mutex mutex)
{
    LeaveCriticalSection(mutex);
}
#else
dr_mutex dr_create_mutex()
{
    return CreateEventA(NULL, FALSE, TRUE, NULL);
}

void dr_delete_mutex(dr_mutex mutex)
{
    CloseHandle((HANDLE)mutex);
}

void dr_lock_mutex(dr_mutex mutex)
{
    WaitForSingleObject((HANDLE)mutex, INFINITE);
}

void dr_unlock_mutex(dr_mutex mutex)
{
    SetEvent((HANDLE)mutex);
}
#endif


dr_semaphore dr_create_semaphore(int initialValue)
{
    return (void*)CreateSemaphoreA(NULL, initialValue, LONG_MAX, NULL);
}

void dr_delete_semaphore(dr_semaphore semaphore)
{
    CloseHandle(semaphore);
}

dr_bool32 dr_wait_semaphore(dr_semaphore semaphore)
{
    return WaitForSingleObject((HANDLE)semaphore, INFINITE) == WAIT_OBJECT_0;
}

dr_bool32 dr_release_semaphore(dr_semaphore semaphore)
{
    return ReleaseSemaphore((HANDLE)semaphore, 1, NULL) != 0;
}
#else
void dr_sleep(unsigned int milliseconds)
{
    usleep(milliseconds * 1000);    // <-- usleep is in microseconds.
}

void dr_yield()
{
    sched_yield();
}

unsigned int dr_get_logical_processor_count()
{
    return (unsigned int)sysconf(_SC_NPROCESSORS_ONLN);
}


typedef struct
{
    /// The Win32 thread handle.
    pthread_t pthread;

    /// The entry point.
    dr_thread_entry_proc entryProc;

    /// The user data to pass to the thread's entry point.
    void* pData;

    /// Set to DR_TRUE by the entry function. We use this to wait for the entry function to start.
    dr_bool32 isInEntryProc;

} dr_thread_posix;

static void* dr_thread_entry_proc_posix(void* pDataIn)
{
    dr_thread_posix* pThreadPosix = (dr_thread_posix*)pDataIn;
    assert(pThreadPosix != NULL);

    void* pEntryProcData = pThreadPosix->pData;
    dr_thread_entry_proc entryProc = pThreadPosix->entryProc;
    assert(entryProc != NULL);

    pThreadPosix->isInEntryProc = DR_TRUE;

    return (void*)(size_t)entryProc(pEntryProcData);
}

dr_thread dr_create_thread(dr_thread_entry_proc entryProc, void* pData)
{
    if (entryProc == NULL) {
        return NULL;
    }

    dr_thread_posix* pThreadPosix = (dr_thread_posix*)malloc(sizeof(*pThreadPosix));
    if (pThreadPosix != NULL)
    {
        pThreadPosix->entryProc     = entryProc;
        pThreadPosix->pData         = pData;
        pThreadPosix->isInEntryProc = DR_FALSE;

        if (pthread_create(&pThreadPosix->pthread, NULL, dr_thread_entry_proc_posix, pThreadPosix) != 0) {
            free(pThreadPosix);
            return NULL;
        }

        // Wait for the new thread to enter into it's entry point before returning. We need to do this so we can safely
        // support something like dr_delete_thread(dr_create_thread(my_thread_proc, pData)).
        while (!pThreadPosix->isInEntryProc) {}
    }

    return (dr_thread)pThreadPosix;
}

void dr_delete_thread(dr_thread thread)
{
    free(thread);
}

void dr_wait_thread(dr_thread thread)
{
    dr_thread_posix* pThreadPosix = (dr_thread_posix*)thread;
    if (pThreadPosix != NULL)
    {
        pthread_join(pThreadPosix->pthread, NULL);
    }
}



dr_mutex dr_create_mutex()
{
    pthread_mutex_t* mutex = (pthread_mutex_t*)malloc(sizeof(*mutex));
    if (pthread_mutex_init(mutex, NULL) != 0) {
        free(mutex);
        mutex = NULL;
    }

    return mutex;
}

void dr_delete_mutex(dr_mutex mutex)
{
    pthread_mutex_destroy((pthread_mutex_t*)mutex);
}

void dr_lock_mutex(dr_mutex mutex)
{
    pthread_mutex_lock((pthread_mutex_t*)mutex);
}

void dr_unlock_mutex(dr_mutex mutex)
{
    pthread_mutex_unlock((pthread_mutex_t*)mutex);
}



dr_semaphore dr_create_semaphore(int initialValue)
{
    sem_t* semaphore = (sem_t*)malloc(sizeof(*semaphore));
    if (sem_init(semaphore, 0, (unsigned int)initialValue) == -1) {
        free(semaphore);
        semaphore = NULL;
    }

    return semaphore;
}

void dr_delete_semaphore(dr_semaphore semaphore)
{
    sem_close((sem_t*)semaphore);
}

dr_bool32 dr_wait_semaphore(dr_semaphore semaphore)
{
    return sem_wait((sem_t*)semaphore) != -1;
}

dr_bool32 dr_release_semaphore(dr_semaphore semaphore)
{
    return sem_post((sem_t*)semaphore) != -1;
}
#endif



/////////////////////////////////////////////////////////
// Timing

// macOS does not have clock_gettime on OS X < 10.12
#ifdef __MACH__
#include <AvailabilityMacros.h>
#ifndef MAC_OS_X_VERSION_10_12
#define MAC_OS_X_VERSION_10_12 101200
#endif
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_12
#include <mach/mach_time.h>
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 0
int clock_gettime(int clk_id, struct timespec* t)
{
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    dr_uint64 time;
    time = mach_absolute_time();
    double nseconds = ((double)time * (double)timebase.numer) / ((double)timebase.denom);
    double seconds = ((double)time * (double)timebase.numer) / ((double)timebase.denom * 1e9);
    t->tv_sec = seconds;
    t->tv_nsec = nseconds;
    return 0;
}
#endif
#endif

#ifdef _WIN32
static LARGE_INTEGER g_DRTimerFrequency = {{0}};
void dr_timer_init(dr_timer* pTimer)
{
    if (g_DRTimerFrequency.QuadPart == 0) {
        QueryPerformanceFrequency(&g_DRTimerFrequency);
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    pTimer->counter = (dr_uint64)counter.QuadPart;
}

double dr_timer_tick(dr_timer* pTimer)
{
    LARGE_INTEGER counter;
    if (!QueryPerformanceCounter(&counter)) {
        return 0;
    }

    dr_uint64 newTimeCounter = counter.QuadPart;
    dr_uint64 oldTimeCounter = pTimer->counter;

    pTimer->counter = newTimeCounter;

    return (newTimeCounter - oldTimeCounter) / (double)g_DRTimerFrequency.QuadPart;
}
#else
void dr_timer_init(dr_timer* pTimer)
{
    struct timespec newTime;
    clock_gettime(CLOCK_MONOTONIC, &newTime);

    pTimer->counter = (newTime.tv_sec * 1000000000LL) + newTime.tv_nsec;
}

double dr_timer_tick(dr_timer* pTimer)
{
    struct timespec newTime;
    clock_gettime(CLOCK_MONOTONIC, &newTime);

    dr_uint64 newTimeCounter = (newTime.tv_sec * 1000000000LL) + newTime.tv_nsec;
    dr_uint64 oldTimeCounter = pTimer->counter;

    pTimer->counter = newTimeCounter;

    return (newTimeCounter - oldTimeCounter) / 1000000000.0;
}
#endif


/////////////////////////////////////////////////////////
// Random

double dr_randd()
{
    return (double)rand() / (double)RAND_MAX;
}

float dr_randf()
{
    return (float)dr_randd();
}


/////////////////////////////////////////////////////////
// User Accounts and Process Management

size_t dr_get_username(char* usernameOut, size_t usernameOutSize)
{
    if (usernameOut != NULL && usernameOutSize > 0) {
        usernameOut[0] = '\0';
    }

#ifdef _WIN32
    DWORD dwSize = (DWORD)usernameOutSize;
    if (!GetUserNameA(usernameOut, &dwSize)) {
        return 0;
    }

    return dwSize;
#else
    struct passwd *pw = getpwuid(geteuid());
    if (pw == NULL) {
        return 0;
    }

    if (usernameOut != NULL) {
        strcpy_s(usernameOut, usernameOutSize, pw->pw_name);
    }

    return strlen(pw->pw_name);
#endif
}

unsigned int dr_get_process_id()
{
#ifdef _WIN32
    return GetProcessId(GetCurrentProcess());
#else
    return (unsigned int)getpid();
#endif
}



/////////////////////////////////////////////////////////
// Miscellaneous Stuff.

dr_bool32 dr_hex_char_to_uint(char ascii, unsigned int* out)
{
    if (ascii >= '0' && ascii <= '9') {
        if (out) *out = ascii - '0';
        return DR_TRUE;
    }

    if (ascii >= 'A' && ascii <= 'F') {
        if (out) *out = 10 + (ascii - 'A');
        return DR_TRUE;
    }

    if (ascii >= 'a' && ascii <= 'f') {
        if (out) *out = 10 + (ascii - 'a');
        return DR_TRUE;
    }

    if (out) *out = 0;
    return DR_FALSE;
}

#endif  //DR_IMPLEMENTATION




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// dr_path
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef dr_path_h
#define dr_path_h

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


// Structure representing a section of a path.
typedef struct
{
    size_t offset;
    size_t length;

} drpath_segment;

// Structure used for iterating over a path while at the same time providing useful and easy-to-use information about the iteration.
typedef struct drpath_iterator
{
    const char* path;
    drpath_segment segment;

} drpath_iterator;



/// Compares a section of two strings for equality.
///
/// @param s0Path [in] The first path.
/// @param s0     [in] The segment of the first path to compare.
/// @param s1Path [in] The second path.
/// @param s1     [in] The segment of the second path to compare.
///
/// @return DR_TRUE if the strings are equal; DR_FALSE otherwise.
dr_bool32 drpath_segments_equal(const char* s0Path, const drpath_segment s0, const char* s1Path, const drpath_segment s1);


/// Creates an iterator for iterating over each segment in a path.
///
/// @param path [in] The path whose segments are being iterated.
///
/// @return True if at least one segment is found; DR_FALSE if it's an empty path.
dr_bool32 drpath_first(const char* path, drpath_iterator* i);

/// Creates an iterator beginning at the last segment.
dr_bool32 drpath_last(const char* path, drpath_iterator* i);

/// Goes to the next segment in the path as per the given iterator.
///
/// @param i [in] A pointer to the iterator to increment.
///
/// @return True if the iterator contains a valid value. Use this to determine when to terminate iteration.
dr_bool32 drpath_next(drpath_iterator* i);

/// Goes to the previous segment in the path.
///
/// @param i [in] A pointer to the iterator to decrement.
///
/// @return DR_TRUE if the iterator contains a valid value. Use this to determine when to terminate iteration.
dr_bool32 drpath_prev(drpath_iterator* i);

/// Determines if the given iterator is at the end.
///
/// @param i [in] The iterator to check.
dr_bool32 drpath_at_end(drpath_iterator i);

/// Determines if the given iterator is at the start.
///
/// @param i [in] The iterator to check.
dr_bool32 drpath_at_start(drpath_iterator i);

/// Compares the string values of two iterators for equality.
///
/// @param i0 [in] The first iterator to compare.
/// @param i1 [in] The second iterator to compare.
///
/// @return DR_TRUE if the strings are equal; DR_FALSE otherwise.
dr_bool32 drpath_iterators_equal(const drpath_iterator i0, const drpath_iterator i1);


/// Determines whether or not the given iterator refers to the root segment of a path.
dr_bool32 drpath_is_root_segment(const drpath_iterator i);

/// Determines whether or not the given iterator refers to a Linux style root directory ("/")
dr_bool32 drpath_is_linux_style_root_segment(const drpath_iterator i);

/// Determines whether or not the given iterator refers to a Windows style root directory.
dr_bool32 drpath_is_win32_style_root_segment(const drpath_iterator i);


/// Converts the slashes in the given path to forward slashes.
///
/// @param path [in] The path whose slashes are being converted.
void drpath_to_forward_slashes(char* path);

/// Converts the slashes in the given path to back slashes.
///
/// @param path [in] The path whose slashes are being converted.
void drpath_to_backslashes(char* path);


/// Determines whether or not the given path is a decendant of another.
///
/// @param descendantAbsolutePath [in] The absolute path of the descendant.
/// @param parentAbsolutePath     [in] The absolute path of the parent.
///
/// @remarks
///     As an example, "C:/My/Folder" is a descendant of "C:/".
///     @par
///     If either path contains "." or "..", clean it with drpath_clean() before calling this.
dr_bool32 drpath_is_descendant(const char* descendantAbsolutePath, const char* parentAbsolutePath);

/// Determines whether or not the given path is a direct child of another.
///
/// @param childAbsolutePath [in] The absolute of the child.
/// @param parentAbsolutePath [in] The absolute path of the parent.
///
/// @remarks
///     As an example, "C:/My/Folder" is NOT a child of "C:/" - it is a descendant. Alternatively, "C:/My" IS a child of "C:/".
///     @par
///     If either path contains "." or "..", clean it with drpath_clean() before calling this.
dr_bool32 drpath_is_child(const char* childAbsolutePath, const char* parentAbsolutePath);


/// Modifies the given path by transforming it into it's base path.
///
/// Returns <path>, for convenience.
char* drpath_base_path(char* path);

/// Retrieves the base path from the given path, not including the trailing slash.
///
/// @param path            [in]  The full path.
/// @param baseOut         [out] A pointer to the buffer that will receive the base path.
/// @param baseSizeInBytes [in]  The size in bytes of the buffer that will receive the base directory.
///
/// @remarks
///     As an example, when "path" is "C:/MyFolder/MyFile", the output will be "C:/MyFolder". Note that there is no trailing slash.
///     @par
///     If "path" is something like "/MyFolder", the return value will be an empty string.
void drpath_copy_base_path(const char* path, char* baseOut, size_t baseSizeInBytes);

/// Finds the file name portion of the path.
///
/// @param path [in] The path to search.
///
/// @return A pointer to the beginning of the string containing the file name. If this is non-null, it will be an offset of "path".
///
/// @remarks
///     A path with a trailing slash will return an empty string.
///     @par
///     The return value is just an offset of "path".
const char* drpath_file_name(const char* path);

/// Copies the file name into the given buffer.
const char* drpath_copy_file_name(const char* path, char* fileNameOut, size_t fileNameSizeInBytes);

/// Finds the file extension of the given file path.
///
/// @param path [in] The path to search.
///
/// @return A pointer to the beginning of the string containing the file's extension.
///
/// @remarks
///     A path with a trailing slash will return an empty string.
///     @par
///     The return value is just an offset of "path".
///     @par
///     On a path such as "filename.ext1.ext2" the returned string will be "ext2".
const char* drpath_extension(const char* path);


/// Checks whether or not the two paths are equal.
///
/// @param path1 [in] The first path.
/// @param path2 [in] The second path.
///
/// @return DR_TRUE if the paths are equal, DR_FALSE otherwise.
///
/// @remarks
///     This is case-sensitive.
///     @par
///     This is more than just a string comparison. Rather, this splits the path and compares each segment. The path "C:/My/Folder" is considered
///     equal to to "C:\\My\\Folder".
dr_bool32 drpath_equal(const char* path1, const char* path2);

/// Checks if the extension of the given path is equal to the given extension.
///
/// @remarks
///     By default this is NOT case-sensitive, however if the standard library is disable, it is case-sensitive.
dr_bool32 drpath_extension_equal(const char* path, const char* extension);


/// Determines whether or not the given path is relative.
///
/// @param path [in] The path to check.
dr_bool32 drpath_is_relative(const char* path);

/// Determines whether or not the given path is absolute.
///
/// @param path [in] The path to check.
dr_bool32 drpath_is_absolute(const char* path);


/// Appends two paths together, ensuring there is not double up on the last slash.
///
/// @param base                  [in, out] The base path that is being appended to.
/// @param baseBufferSizeInBytes [in]      The size of the buffer pointed to by "base", in bytes.
/// @param other                 [in]      The other path segment.
///
/// @remarks
///     This assumes both paths are well formed and "other" is a relative path.
dr_bool32 drpath_append(char* base, size_t baseBufferSizeInBytes, const char* other);

/// Appends an iterator object to the given base path.
dr_bool32 drpath_append_iterator(char* base, size_t baseBufferSizeInBytes, drpath_iterator i);

/// Appends an extension to the given path.
dr_bool32 drpath_append_extension(char* base, size_t baseBufferSizeInBytes, const char* extension);

/// Appends two paths together, and copyies them to a separate buffer.
///
/// @param dst            [out] The destination buffer.
/// @param dstSizeInBytes [in]  The size of the buffer pointed to by "dst", in bytes.
/// @param base           [in]  The base directory.
/// @param other          [in]  The relative path to append to "base".
///
/// @return DR_TRUE if the paths were appended successfully; DR_FALSE otherwise.
///
/// @remarks
///     This assumes both paths are well formed and "other" is a relative path.
dr_bool32 drpath_copy_and_append(char* dst, size_t dstSizeInBytes, const char* base, const char* other);

/// Appends a base path and an iterator together, and copyies them to a separate buffer.
///
/// @param dst            [out] The destination buffer.
/// @param dstSizeInBytes [in]  The size of the buffer pointed to by "dst", in bytes.
/// @param base           [in]  The base directory.
/// @param i              [in]  The iterator to append.
///
/// @return DR_TRUE if the paths were appended successfully; DR_FALSE otherwise.
///
/// @remarks
///     This assumes both paths are well formed and "i" is a valid iterator.
dr_bool32 drpath_copy_and_append_iterator(char* dst, size_t dstSizeInBytes, const char* base, drpath_iterator i);

/// Appends an extension to the given base path and copies them to a separate buffer.
/// @param dst            [out] The destination buffer.
/// @param dstSizeInBytes [in]  The size of the buffer pointed to by "dst", in bytes.
/// @param base           [in]  The base directory.
/// @param extension      [in]  The relative path to append to "base".
///
/// @return DR_TRUE if the paths were appended successfully; DR_FALSE otherwise.
dr_bool32 drpath_copy_and_append_extension(char* dst, size_t dstSizeInBytes, const char* base, const char* extension);


/// Cleans the path and resolves the ".." and "." segments.
///
/// @param path               [in]  The path to clean.
/// @param pathOut            [out] A pointer to the buffer that will receive the path.
/// @param pathOutSizeInBytes [in]  The size of the buffer pointed to by pathOut, in bytes.
///
/// @return The number of bytes written to the output buffer, including the null terminator.
///
/// @remarks
///     The output path will never be longer than the input path.
///     @par
///     The output buffer should never overlap with the input path.
///     @par
///     As an example, the path "my/messy/../path" will result in "my/path"
///     @par
///     The path "my/messy/../../../path" (note how there are too many ".." segments) will return "path" (the extra ".." segments will be dropped.)
///     @par
///     If an error occurs, such as an invalid input path, 0 will be returned.
size_t drpath_clean(const char* path, char* pathOut, size_t pathOutSizeInBytes);

/// Appends one path to the other and then cleans it.
size_t drpath_append_and_clean(char* dst, size_t dstSizeInBytes, const char* base, const char* other);


/// Removes the extension from the given path.
///
/// @remarks
///     If the given path does not have an extension, 1 will be returned, but the string will be left unmodified.
dr_bool32 drpath_remove_extension(char* path);

/// Creates a copy of the given string and removes the extension.
dr_bool32 drpath_copy_and_remove_extension(char* dst, size_t dstSizeInBytes, const char* path);


/// Removes the last segment from the given path.
dr_bool32 drpath_remove_file_name(char* path);

/// Creates a copy of the given string and removes the extension.
dr_bool32 drpath_copy_and_remove_file_name(char* dst, size_t dstSizeInBytes, const char* path);


/// Converts an absolute path to a relative path.
///
/// @return DR_TRUE if the conversion was successful; DR_FALSE if there was an error.
///
/// @remarks
///     This will normalize every slash to forward slashes.
dr_bool32 drpath_to_relative(const char* absolutePathToMakeRelative, const char* absolutePathToMakeRelativeTo, char* relativePathOut, size_t relativePathOutSizeInBytes);

/// Converts a relative path to an absolute path based on a base path.
///
/// @return DR_TRUE if the conversion was successful; DR_FALSE if there was an error.
///
/// @remarks
///     This is equivalent to an append followed by a clean. Slashes will be normalized to forward slashes.
dr_bool32 drpath_to_absolute(const char* relativePathToMakeAbsolute, const char* basePath, char* absolutePathOut, size_t absolutePathOutSizeInBytes);


#ifdef __cplusplus
}
#endif
#endif  //dr_path_h



///////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DR_IMPLEMENTATION
#include <string.h>
#include <ctype.h>
#include <errno.h>

dr_bool32 drpath_first(const char* path, drpath_iterator* i)
{
    if (i == 0) return DR_FALSE;
    i->path = path;
    i->segment.offset = 0;
    i->segment.length = 0;

    if (path == 0 || path[0] == '\0') {
        return DR_FALSE;
    }

    while (i->path[i->segment.length] != '\0' && (i->path[i->segment.length] != '/' && i->path[i->segment.length] != '\\')) {
        i->segment.length += 1;
    }

    return DR_TRUE;
}

dr_bool32 drpath_last(const char* path, drpath_iterator* i)
{
    if (i == 0) return DR_FALSE;
    i->path = path;
    i->segment.offset = 0;
    i->segment.length = 0;

    if (path == 0 || path[0] == '\0') {
        return DR_FALSE;
    }

    i->path = path;
    i->segment.offset = strlen(path);
    i->segment.length = 0;

    return drpath_prev(i);
}

dr_bool32 drpath_next(drpath_iterator* i)
{
    if (i == NULL || i->path == NULL) {
        return DR_FALSE;
    }

    i->segment.offset = i->segment.offset + i->segment.length;
    i->segment.length = 0;

    while (i->path[i->segment.offset] != '\0' && (i->path[i->segment.offset] == '/' || i->path[i->segment.offset] == '\\')) {
        i->segment.offset += 1;
    }

    if (i->path[i->segment.offset] == '\0') {
        return DR_FALSE;
    }


    while (i->path[i->segment.offset + i->segment.length] != '\0' && (i->path[i->segment.offset + i->segment.length] != '/' && i->path[i->segment.offset + i->segment.length] != '\\')) {
        i->segment.length += 1;
    }

    return DR_TRUE;
}

dr_bool32 drpath_prev(drpath_iterator* i)
{
    if (i == NULL || i->path == NULL || i->segment.offset == 0) {
        return DR_FALSE;
    }

    i->segment.length = 0;

    do
    {
        i->segment.offset -= 1;
    } while (i->segment.offset > 0 && (i->path[i->segment.offset] == '/' || i->path[i->segment.offset] == '\\'));

    if (i->segment.offset == 0) {
        if (i->path[i->segment.offset] == '/' || i->path[i->segment.offset] == '\\') {
            i->segment.length = 0;
            return DR_TRUE;
        }

        return DR_FALSE;
    }


    size_t offsetEnd = i->segment.offset + 1;
    while (i->segment.offset > 0 && (i->path[i->segment.offset] != '/' && i->path[i->segment.offset] != '\\')) {
        i->segment.offset -= 1;
    }

    if (i->path[i->segment.offset] == '/' || i->path[i->segment.offset] == '\\') {
        i->segment.offset += 1;
    }


    i->segment.length = offsetEnd - i->segment.offset;

    return DR_TRUE;
}

dr_bool32 drpath_at_end(drpath_iterator i)
{
    return i.path == 0 || i.path[i.segment.offset] == '\0';
}

dr_bool32 drpath_at_start(drpath_iterator i)
{
    return i.path != 0 && i.segment.offset == 0;
}

dr_bool32 drpath_iterators_equal(const drpath_iterator i0, const drpath_iterator i1)
{
    return drpath_segments_equal(i0.path, i0.segment, i1.path, i1.segment);
}

dr_bool32 drpath_segments_equal(const char* s0Path, const drpath_segment s0, const char* s1Path, const drpath_segment s1)
{
    if (s0Path == NULL || s1Path == NULL) {
        return DR_FALSE;
    }

    if (s0.length != s1.length) {
        return DR_FALSE;
    }

    return strncmp(s0Path + s0.offset, s1Path + s1.offset, s0.length) == 0;
}


dr_bool32 drpath_is_root_segment(const drpath_iterator i)
{
    return drpath_is_linux_style_root_segment(i) || drpath_is_win32_style_root_segment(i);
}

dr_bool32 drpath_is_linux_style_root_segment(const drpath_iterator i)
{
    if (i.path == NULL) {
        return DR_FALSE;
    }

    if (i.segment.offset == 0 && i.segment.length == 0) {
        return DR_TRUE;    // "/" style root.
    }

    return DR_FALSE;
}

dr_bool32 drpath_is_win32_style_root_segment(const drpath_iterator i)
{
    if (i.path == NULL) {
        return DR_FALSE;
    }

    if (i.segment.offset == 0 && i.segment.length == 2) {
        if (((i.path[0] >= 'a' && i.path[0] <= 'z') || (i.path[0] >= 'A' && i.path[0] <= 'Z')) && i.path[1] == ':') {
            return DR_TRUE;
        }
    }

    return DR_FALSE;
}


void drpath_to_forward_slashes(char* path)
{
    if (path == NULL) {
        return;
    }

    while (path[0] != '\0') {
        if (path[0] == '\\') {
            path[0] = '/';
        }

        path += 1;
    }
}

void drpath_to_backslashes(char* path)
{
    if (path == NULL) {
        return;
    }

    while (path[0] != '\0') {
        if (path[0] == '/') {
            path[0] = '\\';
        }

        path += 1;
    }
}


dr_bool32 drpath_is_descendant(const char* descendantAbsolutePath, const char* parentAbsolutePath)
{
    drpath_iterator iChild;
    if (!drpath_first(descendantAbsolutePath, &iChild)) {
        return DR_FALSE;   // The descendant is an empty string which makes it impossible for it to be a descendant.
    }

    drpath_iterator iParent;
    if (drpath_first(parentAbsolutePath, &iParent))
    {
        do
        {
            // If the segment is different, the paths are different and thus it is not a descendant.
            if (!drpath_iterators_equal(iParent, iChild)) {
                return DR_FALSE;
            }

            if (!drpath_next(&iChild)) {
                return DR_FALSE;   // The descendant is shorter which means it's impossible for it to be a descendant.
            }

        } while (drpath_next(&iParent));
    }

    return DR_TRUE;
}

dr_bool32 drpath_is_child(const char* childAbsolutePath, const char* parentAbsolutePath)
{
    drpath_iterator iChild;
    if (!drpath_first(childAbsolutePath, &iChild)) {
        return DR_FALSE;   // The descendant is an empty string which makes it impossible for it to be a descendant.
    }

    drpath_iterator iParent;
    if (drpath_first(parentAbsolutePath, &iParent))
    {
        do
        {
            // If the segment is different, the paths are different and thus it is not a descendant.
            if (!drpath_iterators_equal(iParent, iChild)) {
                return DR_FALSE;
            }

            if (!drpath_next(&iChild)) {
                return DR_FALSE;   // The descendant is shorter which means it's impossible for it to be a descendant.
            }

        } while (drpath_next(&iParent));
    }

    // At this point we have finished iteration of the parent, which should be shorter one. We now do one more iterations of
    // the child to ensure it is indeed a direct child and not a deeper descendant.
    return !drpath_next(&iChild);
}

char* drpath_base_path(char* path)
{
    if (path == NULL) {
        return NULL;
    }

    char* pathorig = path;
    char* baseend = path;

    // We just loop through the path until we find the last slash.
    while (path[0] != '\0') {
        if (path[0] == '/' || path[0] == '\\') {
            baseend = path;
        }

        path += 1;
    }


    // Now we just loop backwards until we hit the first non-slash.
    while (baseend > path) {
        if (baseend[0] != '/' && baseend[0] != '\\') {
            break;
        }

        baseend -= 1;
    }


    // We just put a null terminator on the end.
    baseend[0] = '\0';

    return pathorig;
}

void drpath_copy_base_path(const char* path, char* baseOut, size_t baseSizeInBytes)
{
    if (path == NULL || baseOut == NULL || baseSizeInBytes == 0) {
        return;
    }

    strcpy_s(baseOut, baseSizeInBytes, path);
    drpath_base_path(baseOut);
}

const char* drpath_file_name(const char* path)
{
    if (path == NULL) {
        return NULL;
    }

    const char* fileName = path;

    // We just loop through the path until we find the last slash.
    while (path[0] != '\0') {
        if (path[0] == '/' || path[0] == '\\') {
            fileName = path;
        }

        path += 1;
    }

    // At this point the file name is sitting on a slash, so just move forward.
    while (fileName[0] != '\0' && (fileName[0] == '/' || fileName[0] == '\\')) {
        fileName += 1;
    }

    return fileName;
}

const char* drpath_copy_file_name(const char* path, char* fileNameOut, size_t fileNameSizeInBytes)
{
    const char* fileName = drpath_file_name(path);
    if (fileName != NULL) {
        strcpy_s(fileNameOut, fileNameSizeInBytes, fileName);
    }

    return fileName;
}

const char* drpath_extension(const char* path)
{
    if (path == NULL) {
        return NULL;
    }

    const char* extension     = drpath_file_name(path);
    const char* lastoccurance = 0;

    // Just find the last '.' and return.
    while (extension[0] != '\0')
    {
        if (extension[0] == '.') {
            extension    += 1;
            lastoccurance = extension;
        }

        extension += 1;
    }

    return (lastoccurance != 0) ? lastoccurance : extension;
}


dr_bool32 drpath_equal(const char* path1, const char* path2)
{
    if (path1 == NULL || path2 == NULL) {
        return DR_FALSE;
    }

    if (path1 == path2 || (path1[0] == '\0' && path2[0] == '\0')) {
        return DR_TRUE;    // Two empty paths are treated as the same.
    }

    drpath_iterator iPath1;
    drpath_iterator iPath2;
    if (drpath_first(path1, &iPath1) && drpath_first(path2, &iPath2))
    {
        dr_bool32 isPath1Valid;
        dr_bool32 isPath2Valid;

        do
        {
            if (!drpath_iterators_equal(iPath1, iPath2)) {
                return DR_FALSE;
            }

            isPath1Valid = drpath_next(&iPath1);
            isPath2Valid = drpath_next(&iPath2);

        } while (isPath1Valid && isPath2Valid);

        // At this point either iPath1 and/or iPath2 have finished iterating. If both of them are at the end, the two paths are equal.
        return isPath1Valid == isPath2Valid && iPath1.path[iPath1.segment.offset] == '\0' && iPath2.path[iPath2.segment.offset] == '\0';
    }

    return DR_FALSE;
}

dr_bool32 drpath_extension_equal(const char* path, const char* extension)
{
    if (path == NULL || extension == NULL) {
        return DR_FALSE;
    }

    const char* ext1 = extension;
    const char* ext2 = drpath_extension(path);

#ifdef _MSC_VER
    return _stricmp(ext1, ext2) == 0;
#else
    return strcasecmp(ext1, ext2) == 0;
#endif
}



dr_bool32 drpath_is_relative(const char* path)
{
    if (path == NULL) {
        return DR_FALSE;
    }

    drpath_iterator seg;
    if (drpath_first(path, &seg)) {
        return !drpath_is_root_segment(seg);
    }

    // We'll get here if the path is empty. We consider this to be a relative path.
    return DR_TRUE;
}

dr_bool32 drpath_is_absolute(const char* path)
{
    return !drpath_is_relative(path);
}


dr_bool32 drpath_append(char* base, size_t baseBufferSizeInBytes, const char* other)
{
    if (base == NULL || other == NULL) {
        return DR_FALSE;
    }

    size_t path1Length = strlen(base);
    size_t path2Length = strlen(other);

    if (path1Length >= baseBufferSizeInBytes) {
        return DR_FALSE;
    }


    // Slash.
    if (path1Length > 0 && base[path1Length - 1] != '/' && base[path1Length - 1] != '\\') {
        base[path1Length] = '/';
        path1Length += 1;
    }

    // Other part.
    if (path1Length + path2Length >= baseBufferSizeInBytes) {
        path2Length = baseBufferSizeInBytes - path1Length - 1;      // -1 for the null terminator.
    }

    strncpy_s(base + path1Length, baseBufferSizeInBytes - path1Length, other, path2Length);

    return DR_TRUE;
}

dr_bool32 drpath_append_iterator(char* base, size_t baseBufferSizeInBytes, drpath_iterator i)
{
    if (base == NULL) {
        return DR_FALSE;
    }

    size_t path1Length = strlen(base);
    size_t path2Length = i.segment.length;

    if (path1Length >= baseBufferSizeInBytes) {
        return DR_FALSE;
    }

    if (drpath_is_linux_style_root_segment(i)) {
        if (baseBufferSizeInBytes > 1) {
            base[0] = '/';
            base[1] = '\0';
            return DR_TRUE;
        }
    }

    // Slash.
    if (path1Length > 0 && base[path1Length - 1] != '/' && base[path1Length - 1] != '\\') {
        base[path1Length] = '/';
        path1Length += 1;
    }

    // Other part.
    if (path1Length + path2Length >= baseBufferSizeInBytes) {
        path2Length = baseBufferSizeInBytes - path1Length - 1;      // -1 for the null terminator.
    }

    strncpy_s(base + path1Length, baseBufferSizeInBytes - path1Length, i.path + i.segment.offset, path2Length);

    return DR_TRUE;
}

dr_bool32 drpath_append_extension(char* base, size_t baseBufferSizeInBytes, const char* extension)
{
    if (base == NULL || extension == NULL) {
        return DR_FALSE;
    }

    size_t baseLength = strlen(base);
    size_t extLength  = strlen(extension);

    if (baseLength >= baseBufferSizeInBytes) {
        return DR_FALSE;
    }

    base[baseLength] = '.';
    baseLength += 1;

    if (baseLength + extLength >= baseBufferSizeInBytes) {
        extLength = baseBufferSizeInBytes - baseLength - 1;      // -1 for the null terminator.
    }

    strncpy_s(base + baseLength, baseBufferSizeInBytes - baseLength, extension, extLength);

    return DR_TRUE;
}

dr_bool32 drpath_copy_and_append(char* dst, size_t dstSizeInBytes, const char* base, const char* other)
{
    if (dst == NULL || dstSizeInBytes == 0) {
        return DR_FALSE;
    }

    strcpy_s(dst, dstSizeInBytes, base);
    return drpath_append(dst, dstSizeInBytes, other);
}

dr_bool32 drpath_copy_and_append_iterator(char* dst, size_t dstSizeInBytes, const char* base, drpath_iterator i)
{
    if (dst == NULL || dstSizeInBytes == 0) {
        return DR_FALSE;
    }

    strcpy_s(dst, dstSizeInBytes, base);
    return drpath_append_iterator(dst, dstSizeInBytes, i);
}

dr_bool32 drpath_copy_and_append_extension(char* dst, size_t dstSizeInBytes, const char* base, const char* extension)
{
    if (dst == NULL || dstSizeInBytes == 0) {
        return DR_FALSE;
    }

    strcpy_s(dst, dstSizeInBytes, base);
    return drpath_append_extension(dst, dstSizeInBytes, extension);
}



// This function recursively cleans a path which is defined as a chain of iterators. This function works backwards, which means at the time of calling this
// function, each iterator in the chain should be placed at the end of the path.
//
// This does not write the null terminator, nor a leading slash for absolute paths.
size_t _drpath_clean_trywrite(drpath_iterator* iterators, unsigned int iteratorCount, char* pathOut, size_t pathOutSizeInBytes, unsigned int ignoreCounter)
{
    if (iteratorCount == 0) {
        return 0;
    }

    drpath_iterator isegment = iterators[iteratorCount - 1];


    // If this segment is a ".", we ignore it. If it is a ".." we ignore it and increment "ignoreCount".
    int ignoreThisSegment = ignoreCounter > 0 && isegment.segment.length > 0;

    if (isegment.segment.length == 1 && isegment.path[isegment.segment.offset] == '.')
    {
        // "."
        ignoreThisSegment = 1;
    }
    else
    {
        if (isegment.segment.length == 2 && isegment.path[isegment.segment.offset] == '.' && isegment.path[isegment.segment.offset + 1] == '.')
        {
            // ".."
            ignoreThisSegment = 1;
            ignoreCounter += 1;
        }
        else
        {
            // It's a regular segment, so decrement the ignore counter.
            if (ignoreCounter > 0) {
                ignoreCounter -= 1;
            }
        }
    }


    // The previous segment needs to be written before we can write this one.
    size_t bytesWritten = 0;

    drpath_iterator prev = isegment;
    if (!drpath_prev(&prev))
    {
        if (iteratorCount > 1)
        {
            iteratorCount -= 1;
            prev = iterators[iteratorCount - 1];
        }
        else
        {
            prev.path           = NULL;
            prev.segment.offset = 0;
            prev.segment.length = 0;
        }
    }

    if (prev.segment.length > 0)
    {
        iterators[iteratorCount - 1] = prev;
        bytesWritten = _drpath_clean_trywrite(iterators, iteratorCount, pathOut, pathOutSizeInBytes, ignoreCounter);
    }


    if (!ignoreThisSegment)
    {
        if (pathOutSizeInBytes > 0)
        {
            pathOut            += bytesWritten;
            pathOutSizeInBytes -= bytesWritten;

            if (bytesWritten > 0)
            {
                pathOut[0] = '/';
                bytesWritten += 1;

                pathOut            += 1;
                pathOutSizeInBytes -= 1;
            }

            if (pathOutSizeInBytes >= isegment.segment.length)
            {
                strncpy_s(pathOut, pathOutSizeInBytes, isegment.path + isegment.segment.offset, isegment.segment.length);
                bytesWritten += isegment.segment.length;
            }
            else
            {
                strncpy_s(pathOut, pathOutSizeInBytes, isegment.path + isegment.segment.offset, pathOutSizeInBytes);
                bytesWritten += pathOutSizeInBytes;
            }
        }
    }

    return bytesWritten;
}

size_t drpath_clean(const char* path, char* pathOut, size_t pathOutSizeInBytes)
{
    if (path == NULL) {
        return 0;
    }

    drpath_iterator last;
    if (drpath_last(path, &last))
    {
        size_t bytesWritten = 0;
        if (path[0] == '/') {
            if (pathOut != NULL && pathOutSizeInBytes > 1) {
                pathOut[0] = '/';
                bytesWritten = 1;
            }
        }

        bytesWritten += _drpath_clean_trywrite(&last, 1, pathOut + bytesWritten, pathOutSizeInBytes - bytesWritten - 1, 0);  // -1 to ensure there is enough room for a null terminator later on.
        if (pathOutSizeInBytes > bytesWritten) {
            pathOut[bytesWritten] = '\0';
        }

        return bytesWritten + 1;
    }

    return 0;
}

size_t drpath_append_and_clean(char* dst, size_t dstSizeInBytes, const char* base, const char* other)
{
    if (base == NULL || other == NULL) {
        return 0;
    }

    drpath_iterator last[2] = {
        {NULL, {0, 0}},
        {NULL, {0, 0}}
    };

    dr_bool32 isPathEmpty0 = !drpath_last(base,  last + 0);
    dr_bool32 isPathEmpty1 = !drpath_last(other, last + 1);

    int iteratorCount = !isPathEmpty0 + !isPathEmpty1;
    if (iteratorCount == 0) {
        return 0;   // Both input strings are empty.
    }

    size_t bytesWritten = 0;
    if (base[0] == '/') {
        if (dst != NULL && dstSizeInBytes > 1) {
            dst[0] = '/';
            bytesWritten = 1;
        }
    }

    bytesWritten += _drpath_clean_trywrite(last, 2, dst + bytesWritten, dstSizeInBytes - bytesWritten - 1, 0);  // -1 to ensure there is enough room for a null terminator later on.
    if (dstSizeInBytes > bytesWritten) {
        dst[bytesWritten] = '\0';
    }

    return bytesWritten + 1;
}


dr_bool32 drpath_remove_extension(char* path)
{
    if (path == NULL) {
        return DR_FALSE;
    }

    const char* extension = drpath_extension(path);
    if (extension != NULL) {
        extension -= 1;
        path[(extension - path)] = '\0';
    }

    return DR_TRUE;
}

dr_bool32 drpath_copy_and_remove_extension(char* dst, size_t dstSizeInBytes, const char* path)
{
    if (dst == NULL || dstSizeInBytes == 0 || path == NULL) {
        return DR_FALSE;
    }

    const char* extension = drpath_extension(path);
    if (extension != NULL) {
        extension -= 1;
    }

    strncpy_s(dst, dstSizeInBytes, path, (size_t)(extension - path));
    return DR_TRUE;
}


dr_bool32 drpath_remove_file_name(char* path)
{
    if (path == NULL) {
        return DR_FALSE;
    }


    // We just create an iterator that starts at the last segment. We then move back one and place a null terminator at the end of
    // that segment. That will ensure the resulting path is not left with a slash.
    drpath_iterator iLast;
    if (!drpath_last(path, &iLast)) {
        return DR_FALSE;   // The path is empty.
    }

    if (drpath_is_root_segment(iLast)) {
        return DR_FALSE;
    }


    drpath_iterator iSecondLast = iLast;
    if (drpath_prev(&iSecondLast))
    {
        // There is a segment before it so we just place a null terminator at the end, but only if it's not the root of a Linux-style absolute path.
        if (drpath_is_linux_style_root_segment(iSecondLast)) {
            path[iLast.segment.offset] = '\0';
        } else {
            path[iSecondLast.segment.offset + iSecondLast.segment.length] = '\0';
        }
    }
    else
    {
        // This is already the last segment, so just place a null terminator at the beginning of the string.
        path[0] = '\0';
    }

    return DR_TRUE;
}

dr_bool32 drpath_copy_and_remove_file_name(char* dst, size_t dstSizeInBytes, const char* path)
{
    if (dst == NULL) {
        return DR_FALSE;
    }
    if (dstSizeInBytes == 0) {
        return DR_FALSE;
    }
    if (path == NULL) {
        dst[0] = '\0';
        return DR_FALSE;
    }


    // We just create an iterator that starts at the last segment. We then move back one and place a null terminator at the end of
    // that segment. That will ensure the resulting path is not left with a slash.
    drpath_iterator iLast;
    if (!drpath_last(path, &iLast)) {
        dst[0] = '\0';
        return DR_FALSE;   // The path is empty.
    }

    if (drpath_is_linux_style_root_segment(iLast)) {
        if (dstSizeInBytes > 1) {
            dst[0] = '/'; dst[1] = '\0';
            return DR_FALSE;
        } else {
            dst[0] = '\0';
            return DR_FALSE;
        }
    }

    if (drpath_is_win32_style_root_segment(iLast)) {
        return strncpy_s(dst, dstSizeInBytes, path + iLast.segment.offset, iLast.segment.length) == 0;
    }


    drpath_iterator iSecondLast = iLast;
    if (drpath_prev(&iSecondLast))
    {
        // There is a segment before it so we just place a null terminator at the end, but only if it's not the root of a Linux-style absolute path.
        if (drpath_is_linux_style_root_segment(iSecondLast)) {
            return strcpy_s(dst, dstSizeInBytes, "/") == 0;
        } else {
            return strncpy_s(dst, dstSizeInBytes, path, iSecondLast.segment.offset + iSecondLast.segment.length) == 0;
        }
    }
    else
    {
        // This is already the last segment, so just place a null terminator at the beginning of the string.
        dst[0] = '\0';
        return DR_TRUE;
    }
}


dr_bool32 drpath_to_relative(const char* absolutePathToMakeRelative, const char* absolutePathToMakeRelativeTo, char* relativePathOut, size_t relativePathOutSizeInBytes)
{
    // We do this in two phases. The first phase just iterates past each segment of both the path to convert and the
    // base path until we find two that are not equal. The second phase just adds the appropriate ".." segments.

    if (relativePathOut == 0) {
        return DR_FALSE;
    }
    if (relativePathOutSizeInBytes == 0) {
        return DR_FALSE;
    }

    if (!drpath_is_absolute(absolutePathToMakeRelative) || !drpath_is_absolute(absolutePathToMakeRelativeTo)) {
        return DR_FALSE;
    }


    drpath_iterator iPath;
    drpath_iterator iBase;
    dr_bool32 isPathEmpty = !drpath_first(absolutePathToMakeRelative, &iPath);
    dr_bool32 isBaseEmpty = !drpath_first(absolutePathToMakeRelativeTo, &iBase);

    if (isPathEmpty && isBaseEmpty)
    {
        // Looks like both paths are empty.
        relativePathOut[0] = '\0';
        return DR_FALSE;
    }


    // Phase 1: Get past the common section.
    int isPathAtEnd = 0;
    int isBaseAtEnd = 0;
    while (!isPathAtEnd && !isBaseAtEnd && drpath_iterators_equal(iPath, iBase))
    {
        isPathAtEnd = !drpath_next(&iPath);
        isBaseAtEnd = !drpath_next(&iBase);
    }

    if (iPath.segment.offset == 0)
    {
        // The path is not relative to the base path.
        relativePathOut[0] = '\0';
        return DR_FALSE;
    }


    // Phase 2: Append ".." segments - one for each remaining segment in the base path.
    char* pDst = relativePathOut;
    size_t bytesAvailable = relativePathOutSizeInBytes;

    if (!drpath_at_end(iBase))
    {
        do
        {
            if (pDst != relativePathOut)
            {
                if (bytesAvailable <= 3)
                {
                    // Ran out of room.
                    relativePathOut[0] = '\0';
                    return DR_FALSE;
                }

                pDst[0] = '/';
                pDst[1] = '.';
                pDst[2] = '.';

                pDst += 3;
                bytesAvailable -= 3;
            }
            else
            {
                // It's the first segment, so we need to ensure we don't lead with a slash.
                if (bytesAvailable <= 2)
                {
                    // Ran out of room.
                    relativePathOut[0] = '\0';
                    return DR_FALSE;
                }

                pDst[0] = '.';
                pDst[1] = '.';

                pDst += 2;
                bytesAvailable -= 2;
            }
        } while (drpath_next(&iBase));
    }


    // Now we just append whatever is left of the main path. We want the path to be clean, so we append segment-by-segment.
    if (!drpath_at_end(iPath))
    {
        do
        {
            // Leading slash, if required.
            if (pDst != relativePathOut)
            {
                if (bytesAvailable <= 1)
                {
                    // Ran out of room.
                    relativePathOut[0] = '\0';
                    return DR_FALSE;
                }

                pDst[0] = '/';

                pDst += 1;
                bytesAvailable -= 1;
            }


            if (bytesAvailable <= iPath.segment.length)
            {
                // Ran out of room.
                relativePathOut[0] = '\0';
                return DR_FALSE;
            }

            if (strncpy_s(pDst, bytesAvailable, iPath.path + iPath.segment.offset, iPath.segment.length) != 0)
            {
                // Error copying the string. Probably ran out of room in the output buffer.
                relativePathOut[0] = '\0';
                return DR_FALSE;
            }

            pDst += iPath.segment.length;
            bytesAvailable -= iPath.segment.length;


        } while (drpath_next(&iPath));
    }


    // Always null terminate.
    //assert(bytesAvailable > 0);
    pDst[0] = '\0';

    return DR_TRUE;
}

dr_bool32 drpath_to_absolute(const char* relativePathToMakeAbsolute, const char* basePath, char* absolutePathOut, size_t absolutePathOutSizeInBytes)
{
    return drpath_append_and_clean(absolutePathOut, absolutePathOutSizeInBytes, basePath, relativePathToMakeAbsolute) != 0;
}
#endif  //DR_IMPLEMENTATION



/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

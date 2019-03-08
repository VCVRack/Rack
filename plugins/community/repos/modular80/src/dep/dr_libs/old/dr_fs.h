// Public Domain. See "unlicense" statement at the end of this file.
//
// Includes code from miniz.c which can be found here: https://github.com/richgel999/miniz

// NOTE: dr_fs is very early in development and should be considered unstable. Expect many APIs to change.

// ABOUT
//
// dr_fs is a simple library which abstracts file IO to allow one to open files from both the native file
// system and archive/package files such as Zip files using a common API.
//
// This file includes code from miniz.c which has been stripped down to include only what's needed to support
// Zip files at basic level. Every public API has been namespaced with "drfs_" to avoid naming conflicts.
//
// Some noteworthy features:
// - Supports verbose absolute paths to avoid ambiguity. For example you can specify a path
//   such as "my/package.zip/file.txt"
// - Supports shortened, transparent paths by automatically scanning for supported archives. The
//   path "my/package.zip/file.txt" can be shortened to "my/file.txt", for example. This does not
//   work for absolute paths, however. See notes below.
// - Fully recursive. A path such as "pack1.zip/pack2.zip/file.txt" should work just fine.
// - Easily supports custom package formats without the need to modify the original source code.
//   Look at drfs_register_archive_backend() and the implementation of Zip archives for an
//   example.
// - No dependencies except for the C standard library.
//
// Limitations:
// - When a file contained within a Zip file is opened, the entire uncompressed data is loaded
//   onto the heap. Keep this in mind when working with large files.
// - Zip, PAK and Wavefront MTL archives are read-only at the moment.
// - dr_fs is not fully thread-safe. See notes below.
// - Asynchronous IO is not supported.
//
//
//
// USAGE
//
// This is a single-file library. To use it, do something like the following in one .c file.
//   #define DR_FS_IMPLEMENTATION
//   #include "dr_fs.h"
//
// You can then #include dr_fs.h in other parts of the program as you would with any other header file.
//
// Example:
//      // Create a context.
//      drfs_context* pVFS = drfs_create_context();
//      if (pVFS == NULL) {
//          // There was an error creating the context.
//      }
//
//      // Add your base directories for loading from relative paths. If you do not specify at
//      // least one base directory you will need to load from absolute paths.
//      drfs_add_base_directory(pVFS, "C:/Users/Admin");
//      drfs_add_base_directory(pVFS, "C:/My/Folder");
//
//      ...
//
//      // Open a file. A relative path was specified which means it will first check it against
//      // "C:/Users/Admin". If it can't be found it will then check against "C:/My/Folder".
//      drfs_file* pFile;
//      drfs_result result = drfs_open(pVFS, "my/file.txt", DRFS_READ, &pFile);
//      if (result != drfs_success) {
//          // There was an error loading the file. It probably doesn't exist.
//      }
//
//      result = drfs_read(pFile, buffer, bufferSize, NULL);
//      if (result != drfs_success) {
//          // There was an error reading the file.
//      }
//
//      drfs_close(pFile);
//
//      ...
//
//      // Shutdown.
//      drfs_delete_context(pVFS);
//
//
//
// OPTIONS
//
// To use these options, just place the appropriate #define's in the .c file before including this file.
//
// #define DR_FS_NO_ZIP
//   Disable built-in support for Zip files.
//
// #define DR_FS_NO_PAK
//   Disable support for Quake 2 PAK files.
//
// #define DR_FS_NO_MTL
//   Disable support for Wavefront MTL files.
//
//
//
// THREAD SAFETY
//
// dr_fs is not fully thread safe. Known unsafe functionality includes:
// - Opening a file while adding or removing base directories and backends
// - Closing a file while doing anything on that file object
//   - drfs_open() will malloc() the drfs_file object, and drfs_close() will free() it with no garbage collection
//     nor reference counting.
//
// The issues mentioned above should not be an issue for the vast majority of cases. Base directories and backends
// will typically be registered once during initialization when the context is created, and it's unlikely an
// application will want to close a file while simultaneously trying to use it on another thread without doing it's
// own synchronization anyway.
//
// Thread-safety has not been completely ignored either. It is possible to read, write and seek on multiple threads.
// In this case it is a simple matter of first-in first-served. Also, APIs are in place to allow an application to
// do it's own synchronization. An application can use drfs_lock() and drfs_unlock() to lock and unlock a file using
// simple mutal exclusion. Inside the lock/unlock pair the application can then use the "_nolock" variation of the
// relevant APIs:
// - drfs_read_nolock()
// - drfs_write_nolock()
// - drfs_seek_nolock()
// - drfs_tell_nolock()
// - drfs_size_nolock()
//
// Opening two files that share the same archive should work fine across multiple threads. For example, if you have
// an archive called MyArchive.zip and then open two files within that archive, you can do work with each of those
// files independently on separate threads. This functionality depends on the implementation of the relevant backend,
// however.
//
// When implementing a backend, it is important to keep synchronization in mind when reading data from the host
// archive file. To help with this, use the drfs_lock() and drfs_unlock() combined with the "_nolock" variations
// of the APIs listed above.
//
//
//
// QUICK NOTES
//
// - The library works by using the notion of an "archive" to create an abstraction around the file system.
// - Conceptually, and archive is just a grouping of files and folders. An archive can be a directory on the native
//   file system or an actual archive file such as a .zip file.
// - When iterating over files and folder, the order is undefined. Do not assume alphabetical.
// - When a path includes the name of the package file, such as "my/package.zip/file.txt" (note how the .zip file is
//   included in the path), it is referred to as a verbose path.
// - When specifying an absolute path, it is assumed to be verbose. When specifying a relative path, it does not need
//   to be verbose, in which case the library will try to search for it. A path such as "my/package.zip/file.txt" is
//   equivalent to "my/file.txt".
// - Archive backends are selected based on their extension.
// - Archive backends cannot currently share the same extension. For example, many package file formats use the .pak
//   extension, however only one backend can use that .pak extension.
// - For safety, if you want to overwrite a file you must explicitly call drfs_open() with the DRFS_TRUNCATE flag.
// - Platforms other than Windows do not use buffering. That is, they use read() and write() instead of fread() and
//   fwrite().
// - On Linux platforms, if you are having issues with opening files larger than 2GB, make sure this file is the first
//   file included in the .c file. This ensures the _LARGEFILE64_SOURCE macro is defined before any other header file
//   as required for the use of 64-bit variants of the POSIX APIs.
// - Base paths must be absolute and verbose.
//
//
//
// TODO:
//
// - Test result code consistency.
// - Document performance issues.
// - Consider making it so persistent constant strings (such as base paths) use dynamically allocated strings rather
//   than fixed sized arrays of DRFS_MAX_PATH.
// - Replace the miniz reading functionality with a custom one:
//   - There is a sort-of-bug where miniz does not correctly enumerate directories in a zip file that was created with
//     the "Send to -> Compressed (zipped) folder" functionality in Windows Explorer. This is more of a thing with
//     Windows Explorer more than anything, but it'd be nice if it would Just Work.
//   - miniz does not support streamed reading yet. Instead, one must decompress the entire file onto the heap which
//     is a bit untidy and doesn't work well with very large files.
//   - ZIP64 is not supported.

#ifndef dr_fs_h
#define dr_fs_h

// These need to be defined before including any headers, but we don't want to expose it to the public header.
#if defined(DR_FS_IMPLEMENTATION) && !defined(_WIN32)
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#endif

#include <stddef.h>

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

#ifdef __cplusplus
extern "C" {
#endif


// The maximum length of a path in bytes, including the null terminator. If a path exceeds this amount, it will be set to an empty
// string. When this is changed the source file will need to be recompiled. Most of the time leaving this at 256 is fine, but it's
// not a problem to increase the size if you are encountering issues. Note that increasing this value will increase memory usage
// on both the heap and the stack.
#ifndef DRFS_MAX_PATH
//#define DRFS_MAX_PATH    256
#define DRFS_MAX_PATH    1024
//#define DRFS_MAX_PATH    4096
#endif

#define DRFS_READ        (1 << 0)
#define DRFS_WRITE       (1 << 1)
#define DRFS_EXISTING    (1 << 2)
#define DRFS_TRUNCATE    (1 << 3)
#define DRFS_CREATE_DIRS (1 << 4)    // Creates the directory structure if required.

#define DRFS_FILE_ATTRIBUTE_DIRECTORY    0x00000001
#define DRFS_FILE_ATTRIBUTE_READONLY     0x00000002

// Result codes.
typedef enum
{
    drfs_success                =  0,
    drfs_unknown_error          = -1,
    drfs_invalid_args           = -2,      // Bad input arguments like a file path string is equal to NULL or whatnot.
    drfs_does_not_exist         = -3,
    drfs_already_exists         = -4,
    drfs_permission_denied      = -5,
    drfs_too_many_open_files    = -6,
    drfs_no_backend             = -7,
    drfs_out_of_memory          = -8,
    drfs_not_in_write_directory = -9,     // A write operation is required, but the given path is not within the write directory or it's sub-directories.
    drfs_path_too_long          = -10,
    drfs_no_space               = -11,
    drfs_not_directory          = -12,
    drfs_too_large              = -13,
    drfs_at_end_of_file         = -14,
    drfs_invalid_archive        = -15,
    drfs_negative_seek          = -16
} drfs_result;

// The allowable seeking origins.
typedef enum
{
    drfs_origin_current,
    drfs_origin_start,
    drfs_origin_end
}drfs_seek_origin;


typedef void* drfs_handle;

typedef struct drfs_context   drfs_context;
typedef struct drfs_archive   drfs_archive;
typedef struct drfs_file      drfs_file;
typedef struct drfs_file_info drfs_file_info;
typedef struct drfs_iterator  drfs_iterator;

typedef dr_bool32   (* drfs_is_valid_extension_proc)(const char* extension);
typedef drfs_result (* drfs_open_archive_proc)      (drfs_file* pArchiveFile, unsigned int accessMode, drfs_handle* pHandleOut);
typedef void        (* drfs_close_archive_proc)     (drfs_handle archive);
typedef drfs_result (* drfs_get_file_info_proc)     (drfs_handle archive, const char* relativePath, drfs_file_info* fi);
typedef drfs_handle (* drfs_begin_iteration_proc)   (drfs_handle archive, const char* relativePath);
typedef void        (* drfs_end_iteration_proc)     (drfs_handle archive, drfs_handle iterator);
typedef dr_bool32   (* drfs_next_iteration_proc)    (drfs_handle archive, drfs_handle iterator, drfs_file_info* fi);
typedef drfs_result (* drfs_delete_file_proc)       (drfs_handle archive, const char* relativePath);
typedef drfs_result (* drfs_create_directory_proc)  (drfs_handle archive, const char* relativePath);
typedef drfs_result (* drfs_move_file_proc)         (drfs_handle archive, const char* relativePathOld, const char* relativePathNew);
typedef drfs_result (* drfs_copy_file_proc)         (drfs_handle archive, const char* relativePathSrc, const char* relativePathDst, dr_bool32 failIfExists);
typedef drfs_result (* drfs_open_file_proc)         (drfs_handle archive, const char* relativePath, unsigned int accessMode, drfs_handle* pHandleOut);
typedef void        (* drfs_close_file_proc)        (drfs_handle archive, drfs_handle file);
typedef drfs_result (* drfs_read_file_proc)         (drfs_handle archive, drfs_handle file, void* pDataOut, size_t bytesToRead, size_t* pBytesReadOut);
typedef drfs_result (* drfs_write_file_proc)        (drfs_handle archive, drfs_handle file, const void* pData, size_t bytesToWrite, size_t* pBytesWrittenOut);
typedef drfs_result (* drfs_seek_file_proc)         (drfs_handle archive, drfs_handle file, dr_int64 bytesToSeek, drfs_seek_origin origin);
typedef dr_uint64   (* drfs_tell_file_proc)         (drfs_handle archive, drfs_handle file);
typedef dr_uint64   (* drfs_file_size_proc)         (drfs_handle archive, drfs_handle file);
typedef void        (* drfs_flush_file_proc)        (drfs_handle archive, drfs_handle file);

typedef struct
{
    drfs_is_valid_extension_proc is_valid_extension;
    drfs_open_archive_proc       open_archive;
    drfs_close_archive_proc      close_archive;
    drfs_get_file_info_proc      get_file_info;
    drfs_begin_iteration_proc    begin_iteration;
    drfs_end_iteration_proc      end_iteration;
    drfs_next_iteration_proc     next_iteration;
    drfs_delete_file_proc        delete_file;
    drfs_create_directory_proc   create_directory;
    drfs_move_file_proc          move_file;
    drfs_copy_file_proc          copy_file;
    drfs_open_file_proc          open_file;
    drfs_close_file_proc         close_file;
    drfs_read_file_proc          read_file;
    drfs_write_file_proc         write_file;
    drfs_seek_file_proc          seek_file;
    drfs_tell_file_proc          tell_file;
    drfs_file_size_proc          file_size;
    drfs_flush_file_proc         flush_file;
} drfs_archive_callbacks;

struct drfs_file_info
{
    // The absolute path of the file.
    char absolutePath[DRFS_MAX_PATH];

    // The size of the file, in bytes.
    dr_uint64 sizeInBytes;

    // The time the file was last modified.
    dr_uint64 lastModifiedTime;

    // File attributes.
    unsigned int attributes;
};

struct drfs_iterator
{
    // A pointer to the archive that contains the folder being iterated.
    drfs_archive* pArchive;

    // A pointer to the iterator's internal handle that was returned with begin_iteration().
    drfs_handle internalIteratorHandle;

    // The file info.
    drfs_file_info info;
};


typedef struct
{
    drfs_archive_callbacks* pBuffer;
    unsigned int count;
} drfs_callbacklist;

typedef struct
{
    char absolutePath[DRFS_MAX_PATH];
} drfs_basepath;

typedef struct
{
    drfs_basepath* pBuffer;
    unsigned int capacity;
    unsigned int count;
} drfs_basedirs;

struct drfs_context
{
    // The list of archive callbacks which are used for loading non-native archives. This does not include the native callbacks.
    drfs_callbacklist archiveCallbacks;

    // The list of base directories.
    drfs_basedirs baseDirectories;

    // The write base directory.
    char writeBaseDirectory[DRFS_MAX_PATH];

    // Keeps track of whether or not write directory guard is enabled.
    dr_bool32 isWriteGuardEnabled;
};


// Initializes a pre-allocated context.
drfs_result drfs_init(drfs_context* pContext);

// Uninitializes a context.
drfs_result drfs_uninit(drfs_context* pContext);


// Creates an empty context.
drfs_context* drfs_create_context();

// Deletes the given context.
//
// This does not close any files or archives - it is up to the application to ensure those are tidied up.
void drfs_delete_context(drfs_context* pContext);


// Registers an archive back-end.
void drfs_register_archive_backend(drfs_context* pContext, drfs_archive_callbacks callbacks);


// Inserts a base directory at a specific priority position.
//
// A lower value index means a higher priority. This must be in the range of [0, drfs_get_base_directory_count()].
void drfs_insert_base_directory(drfs_context* pContext, const char* absolutePath, unsigned int index);

// Adds a base directory to the end of the list.
//
// The further down the list the base directory, the lower priority is will receive. This adds it to the end which
// means it it given a lower priority to those that are already in the list. Use drfs_insert_base_directory() to
// insert the base directory at a specific position.
//
// Base directories must be an absolute path to a real directory.
void drfs_add_base_directory(drfs_context* pContext, const char* absolutePath);

// Removes the given base directory.
void drfs_remove_base_directory(drfs_context* pContext, const char* absolutePath);

// Removes the directory at the given index.
//
// If you need to remove every base directory, use drfs_remove_all_base_directories() since that is more efficient.
void drfs_remove_base_directory_by_index(drfs_context* pContext, unsigned int index);

// Removes every base directory from the given context.
void drfs_remove_all_base_directories(drfs_context* pContext);

// Retrieves the number of base directories attached to the given context.
unsigned int drfs_get_base_directory_count(drfs_context* pContext);

// Retrieves the base directory at the given index.
const char* drfs_get_base_directory_by_index(drfs_context* pContext, unsigned int index);


// Sets the base directory for write operations (including delete).
//
// When doing a write operation using a relative path, the full path will be resolved using this directory as the base.
//
// If the base write directory is not set, and absolute path must be used for all write operations.
//
// If the write directory guard is enabled, all write operations that are attempted at a higher level than this directory
// will fail.
void drfs_set_base_write_directory(drfs_context* pContext, const char* absolutePath);

// Retrieves the base write directory.
dr_bool32 drfs_get_base_write_directory(drfs_context* pContext, char* absolutePathOut, unsigned int absolutePathOutSize);

// Enables the write directory guard.
void drfs_enable_write_directory_guard(drfs_context* pContext);

// Disables the write directory guard.
void drfs_disable_write_directory_guard(drfs_context* pContext);

// Determines whether or not the base directory guard is enabled.
dr_bool32 drfs_is_write_directory_guard_enabled(drfs_context* pContext);


// Opens an archive at the given path.
//
// If the given path points to a directory on the native file system an archive will be created at that
// directory. If the path points to an archive file such as a .zip file, dr_fs will hold a handle to
// that file until the archive is closed with drfs_close_archive(). Keep this in mind if you are keeping
// many archives open at a time on platforms that limit the number of files one can have open at any given
// time.
//
// The given path must be either absolute, or relative to one of the base directories.
//
// The path can be nested, such as "C:/my_zip_file.zip/my_inner_zip_file.zip".
drfs_result drfs_open_archive(drfs_context* pContext, const char* absoluteOrRelativePath, unsigned int accessMode, drfs_archive** ppArchiveOut);

// Opens the archive that owns the given file.
//
// This is different to drfs_open_archive() in that it can accept non-archive files. It will open the
// archive that directly owns the file. In addition, it will output the path of the file, relative to the
// archive.
//
// If the given file is an archive itself, the archive that owns that archive will be opened. If the file
// is a file on the native file system, the returned archive will represent the folder it's directly
// contained in.
drfs_result drfs_open_owner_archive(drfs_context* pContext, const char* absoluteOrRelativePath, unsigned int accessMode, char* relativePathOut, size_t relativePathOutSize, drfs_archive** ppArchiveOut);

// Closes the given archive.
void drfs_close_archive(drfs_archive* pArchive);

// Opens a file relative to the given archive.
drfs_result drfs_open_file_from_archive(drfs_archive* pArchive, const char* relativePath, unsigned int accessMode, drfs_file** ppFileOut);



// Opens a file.
//
// When opening the file in write mode, the write pointer will always be sitting at the start of the file.
drfs_result drfs_open(drfs_context* pContext, const char* absoluteOrRelativePath, unsigned int accessMode, drfs_file** ppFileOut);

// Closes the given file.
void drfs_close(drfs_file* pFile);

// Reads data from the given file.
//
// Returns DR_TRUE if successful; DR_FALSE otherwise. If the value output to <pBytesReadOut> is less than <bytesToRead> it means the file is at the end.
//
// Do not use the return value to check if the end of the file has been reached. Instead, compare <bytesRead> to the value returned to <pBytesReadOut>.
drfs_result drfs_read(drfs_file* pFile, void* pDataOut, size_t bytesToRead, size_t* pBytesReadOut);

// Writes data to the given file.
drfs_result drfs_write(drfs_file* pFile, const void* pData, size_t bytesToWrite, size_t* pBytesWrittenOut);

// Seeks the file pointer by the given number of bytes, relative to the specified origin.
drfs_result drfs_seek(drfs_file* pFile, dr_int64 bytesToSeek, drfs_seek_origin origin);

// Retrieves the current position of the file pointer.
dr_uint64 drfs_tell(drfs_file* pFile);

// Retrieves the size of the given file.
dr_uint64 drfs_size(drfs_file* pFile);

// Flushes the given file.
void drfs_flush(drfs_file* pFile);


// Locks the given file for simple mutal exclusion.
//
// If DR_FALSE is returned it means there was an error and the operation should be aborted.
dr_bool32 drfs_lock(drfs_file* pFile);

// Unlocks the given file for simple mutal exclusion.
void drfs_unlock(drfs_file* pFile);

// Unlocked drfs_read() - should only be called inside a drfs_lock()/drfs_unlock() pair.
drfs_result drfs_read_nolock(drfs_file* pFile, void* pDataOut, size_t bytesToRead, size_t* pBytesReadOut);

// Unlocked drfs_write() - should only be called inside a drfs_lock()/drfs_unlock() pair.
drfs_result drfs_write_nolock(drfs_file* pFile, const void* pData, size_t bytesToWrite, size_t* pBytesWrittenOut);

// Unlocked drfs_seek() - should only be called inside a drfs_lock()/drfs_unlock() pair.
drfs_result drfs_seek_nolock(drfs_file* pFile, dr_int64 bytesToSeek, drfs_seek_origin origin);

// Unlocked drfs_tell() - should only be called inside a drfs_lock()/drfs_unlock() pair.
dr_uint64 drfs_tell_nolock(drfs_file* pFile);

// Unlocked drfs_size() - should only be called inside a drfs_lock()/drfs_unlock() pair.
dr_uint64 drfs_size_nolock(drfs_file* pFile);


// Retrieves information about the file at the given path.
//
// <fi> is allowed to be null, in which case the call is equivalent to simply checking if the file exists.
drfs_result drfs_get_file_info(drfs_context* pContext, const char* absoluteOrRelativePath, drfs_file_info* fi);


// Creates an iterator for iterating over the files and folders in the given directory.
dr_bool32 drfs_begin(drfs_context* pContext, const char* absoluteOrRelativePath, drfs_iterator* pIteratorOut);

// Goes to the next file or folder based on the given iterator.
dr_bool32 drfs_next(drfs_context* pContext, drfs_iterator* pIterator);

// Closes the given iterator.
//
// This is not needed if drfs_next_iteration() returns DR_FALSE naturally. If iteration is terminated early, however, this
// needs to be called on the iterator to ensure internal resources are freed.
void drfs_end(drfs_context* pContext, drfs_iterator* pIterator);


// Deletes the file at the given path.
//
// The path must be a absolute, or relative to the write directory.
drfs_result drfs_delete_file(drfs_context* pContext, const char* path);

// Creates a directory.
//
// The path must be a absolute, or relative to the write directory.
drfs_result drfs_create_directory(drfs_context* pContext, const char* path);

// Moves or renames the given file.
//
// The path must be a absolute, or relative to the write directory. This will fail if:
//   - the file already exists
//   - the old and new paths are across different archives
//   - the archive containing both the old and new paths is read-only
//   - the destinations directory structure does not exist
//
// Consider using drfs_copy_file() for more flexibility with moving files to a different location.
drfs_result drfs_move_file(drfs_context* pContext, const char* pathOld, const char* pathNew);

// Copies a file.
//
// The destination path must be a absolute, or relative to the write directory.
drfs_result drfs_copy_file(drfs_context* pContext, const char* srcPath, const char* dstPath, dr_bool32 failIfExists);


// Determines whether or not the given path refers to an archive file.
//
// This does not validate that the archive file exists or is valid. This will also return DR_FALSE if the path refers
// to a folder on the normal file system.
//
// Use drfs_open_archive() to check that the archive file actually exists.
dr_bool32 drfs_is_archive_path(drfs_context* pContext, const char* path);



///////////////////////////////////////////////////////////////////////////////
//
// High Level API
//
///////////////////////////////////////////////////////////////////////////////

// Free's memory that was allocated internally by dr_fs. This is used when dr_fs allocates memory via a high-level helper API
// and the application is done with that memory.
void drfs_free(void* p);

// Finds the absolute, verbose path of the given path.
drfs_result drfs_find_absolute_path(drfs_context* pContext, const char* relativePath, char* absolutePathOut, size_t absolutePathOutSize);

// Finds the absolute, verbose path of the given path, using the given path as the higest priority base path.
drfs_result drfs_find_absolute_path_explicit_base(drfs_context* pContext, const char* relativePath, const char* highestPriorityBasePath, char* absolutePathOut, size_t absolutePathOutSize);

// Helper function for determining whether or not the given path refers to a base directory.
dr_bool32 drfs_is_base_directory(drfs_context* pContext, const char* baseDir);

// Helper function for writing a string.
drfs_result drfs_write_string(drfs_file* pFile, const char* str);

// Helper function for writing a string, and then inserting a new line right after it.
//
// The new line character is "\n" and NOT "\r\n".
drfs_result drfs_write_line(drfs_file* pFile, const char* str);


// Helper function for opening a binary file and retrieving it's data in one go.
//
// Free the returned pointer with drfs_free()
void* drfs_open_and_read_binary_file(drfs_context* pContext, const char* absoluteOrRelativePath, size_t* pSizeInBytesOut);

// Helper function for opening a text file and retrieving it's data in one go.
//
// Free the returned pointer with drfs_free().
//
// The returned string is null terminated. The size returned by pSizeInBytesOut does not include the null terminator.
char* drfs_open_and_read_text_file(drfs_context* pContext, const char* absoluteOrRelativePath, size_t* pSizeInBytesOut);

// Helper function for opening a file, writing the given data, and then closing it. This deletes the contents of the existing file, if any.
drfs_result drfs_open_and_write_binary_file(drfs_context* pContext, const char* absoluteOrRelativePath, const void* pData, size_t dataSize);

// Helper function for opening a file, writing the given textual data, and then closing it. This deletes the contents of the existing file, if any.
drfs_result drfs_open_and_write_text_file(drfs_context* pContext, const char* absoluteOrRelativePath, const char* pTextData);


// Helper function for determining whether or not the given path refers to an existing file or directory.
dr_bool32 drfs_exists(drfs_context* pContext, const char* absoluteOrRelativePath);

// Determines if the given path refers to an existing file (not a directory).
//
// This will return DR_FALSE for directories. Use drfs_exists() to check for either a file or directory.
dr_bool32 drfs_is_existing_file(drfs_context* pContext, const char* absoluteOrRelativePath);

// Determines if the given path refers to an existing directory.
dr_bool32 drfs_is_existing_directory(drfs_context* pContext, const char* absoluteOrRelativePath);

// Same as drfs_create_directory(), except creates the entire directory structure recursively.
drfs_result drfs_create_directory_recursive(drfs_context* pContext, const char* path);

// Determines whether or not the given file is at the end.
//
// This is just a high-level helper function equivalent to drfs_tell(pFile) == drfs_size(pFile).
dr_bool32 drfs_eof(drfs_file* pFile);



#ifdef __cplusplus
}
#endif

#endif  //dr_fs_h



///////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
///////////////////////////////////////////////////////////////////////////////

#ifdef DR_FS_IMPLEMENTATION
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <strings.h>
#include <errno.h>
#include <pthread.h>
#endif

// Whether or not the file owns the archive object it's part of.
#define DR_FS_OWNS_PARENT_ARCHIVE       0x00000001


static int drfs__strcpy_s(char* dst, size_t dstSizeInBytes, const char* src)
{
#ifdef _MSC_VER
    return strcpy_s(dst, dstSizeInBytes, src);
#else
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
#endif
}

static int drfs__strncpy_s(char* dst, size_t dstSizeInBytes, const char* src, size_t count)
{
#ifdef _MSC_VER
    return strncpy_s(dst, dstSizeInBytes, src, count);
#else
    if (dst == 0) {
        return EINVAL;
    }
    if (dstSizeInBytes == 0) {
        return EINVAL;
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
#endif
}

static int drfs__strcat_s(char* dst, size_t dstSizeInBytes, const char* src)
{
#ifdef _MSC_VER
    return strcat_s(dst, dstSizeInBytes, src);
#else
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
#endif
}

static int drfs__stricmp(const char* string1, const char* string2)
{
#ifdef _MSC_VER
    return _stricmp(string1, string2);
#else
    return strcasecmp(string1, string2);
#endif
}


static dr_bool32 drfs_basedirs_init(drfs_basedirs* pBasePaths)
{
    if (pBasePaths == NULL) {
        return DR_FALSE;
    }

    pBasePaths->pBuffer  = 0;
    pBasePaths->capacity = 0;
    pBasePaths->count    = 0;

    return DR_TRUE;
}

static void drfs_basedirs_uninit(drfs_basedirs* pBasePaths)
{
    if (pBasePaths == NULL) {
        return;
    }

    free(pBasePaths->pBuffer);
}

static dr_bool32 drfs_basedirs_inflateandinsert(drfs_basedirs* pBasePaths, const char* absolutePath, unsigned int index)
{
    if (pBasePaths == NULL) {
        return DR_FALSE;
    }

    unsigned int newBufferSize = (pBasePaths->capacity == 0) ? 2 : pBasePaths->capacity*2;

    drfs_basepath* pOldBuffer = pBasePaths->pBuffer;
    drfs_basepath* pNewBuffer = (drfs_basepath*)malloc(newBufferSize * sizeof(drfs_basepath));
    if (pNewBuffer == NULL) {
        return DR_FALSE;
    }

    for (unsigned int iDst = 0; iDst < index; ++iDst) {
        memcpy(pNewBuffer + iDst, pOldBuffer + iDst, sizeof(drfs_basepath));
    }

    drfs__strcpy_s((pNewBuffer + index)->absolutePath, DRFS_MAX_PATH, absolutePath);

    for (unsigned int iDst = index; iDst < pBasePaths->count; ++iDst) {
        memcpy(pNewBuffer + iDst + 1, pOldBuffer + iDst, sizeof(drfs_basepath));
    }


    pBasePaths->pBuffer  = pNewBuffer;
    pBasePaths->capacity = newBufferSize;
    pBasePaths->count   += 1;

    free(pOldBuffer);
    return DR_TRUE;
}

static dr_bool32 drfs_basedirs_movedown1slot(drfs_basedirs* pBasePaths, unsigned int index)
{
    if (pBasePaths == NULL || pBasePaths->count >= pBasePaths->capacity) {
        return DR_FALSE;
    }

    for (unsigned int iDst = pBasePaths->count; iDst > index; --iDst) {
        memcpy(pBasePaths->pBuffer + iDst, pBasePaths->pBuffer + iDst - 1, sizeof(drfs_basepath));
    }

    return DR_TRUE;
}

static dr_bool32 drfs_basedirs_insert(drfs_basedirs* pBasePaths, const char* absolutePath, unsigned int index)
{
    if (pBasePaths == NULL || index > pBasePaths->count) {
        return DR_FALSE;
    }

    if (pBasePaths->count == pBasePaths->capacity) {
        return drfs_basedirs_inflateandinsert(pBasePaths, absolutePath, index);
    } else {
        if (!drfs_basedirs_movedown1slot(pBasePaths, index)) {
            return DR_FALSE;
        }

        drfs__strcpy_s((pBasePaths->pBuffer + index)->absolutePath, DRFS_MAX_PATH, absolutePath);
        pBasePaths->count += 1;

        return DR_TRUE;
    }
}

static dr_bool32 drfs_basedirs_remove(drfs_basedirs* pBasePaths, unsigned int index)
{
    if (pBasePaths == NULL || index >= pBasePaths->count) {
        return DR_FALSE;
    }

    assert(pBasePaths->count > 0);

    for (unsigned int iDst = index; iDst < pBasePaths->count - 1; ++iDst) {
        memcpy(pBasePaths->pBuffer + iDst, pBasePaths->pBuffer + iDst + 1, sizeof(drfs_basepath));
    }

    pBasePaths->count -= 1;

    return DR_TRUE;
}

static void drfs_basedirs_clear(drfs_basedirs* pBasePaths)
{
    if (pBasePaths == NULL) {
        return;
    }

    drfs_basedirs_uninit(pBasePaths);
    drfs_basedirs_init(pBasePaths);
}



static dr_bool32 drfs_callbacklist_init(drfs_callbacklist* pList)
{
    if (pList == NULL) {
        return DR_FALSE;
    }

    pList->pBuffer = 0;
    pList->count   = 0;

    return DR_TRUE;
}

static void drfs_callbacklist_uninit(drfs_callbacklist* pList)
{
    if (pList == NULL) {
        return;
    }

    free(pList->pBuffer);
}

static dr_bool32 drfs_callbacklist_inflate(drfs_callbacklist* pList)
{
    if (pList == NULL) {
        return DR_FALSE;
    }

    drfs_archive_callbacks* pOldBuffer = pList->pBuffer;
    drfs_archive_callbacks* pNewBuffer = (drfs_archive_callbacks*)malloc((pList->count + 1) * sizeof(drfs_archive_callbacks));
    if (pNewBuffer == NULL) {
        return DR_FALSE;
    }

    for (unsigned int iDst = 0; iDst < pList->count; ++iDst) {
        memcpy(pNewBuffer + iDst, pOldBuffer + iDst, sizeof(drfs_archive_callbacks));
    }

    pList->pBuffer = pNewBuffer;

    free(pOldBuffer);
    return DR_TRUE;
}

static dr_bool32 drfs_callbacklist_pushback(drfs_callbacklist* pList, drfs_archive_callbacks callbacks)
{
    if (pList == NULL) {
        return DR_FALSE;
    }

    if (!drfs_callbacklist_inflate(pList)) {
        return DR_FALSE;
    }

    pList->pBuffer[pList->count] = callbacks;
    pList->count  += 1;

    return DR_TRUE;
}


struct drfs_archive
{
    // A pointer to the context that owns this archive.
    drfs_context* pContext;

    // A pointer to the archive that contains this archive. This can be null in which case it is the top level archive (which is always native).
    drfs_archive* pParentArchive;

    // A pointer to the file containing the data of the archive file.
    drfs_file* pFile;

    // The internal handle that was returned when the archive was opened by the archive definition.
    drfs_handle internalArchiveHandle;

    // Flags. Can be a combination of the following:
    //   DR_FS_OWNS_PARENT_ARCHIVE
    int flags;

    // The callbacks to use when working with on the archive. This contains all of the functions for opening files, reading
    // files, etc.
    drfs_archive_callbacks callbacks;

    // The absolute, verbose path of the archive. For native archives, this will be the name of the folder on the native file
    // system. For non-native archives (zip, etc.) this is the the path of the archive file.
    char absolutePath[DRFS_MAX_PATH];    // Change this to char absolutePath[1] and have it sized exactly as needed.
};

struct drfs_file
{
    // A pointer to the archive that contains the file. This should never be null. Retrieve a pointer to the contex from this
    // by doing pArchive->pContext. The file containing the archive's raw data can be retrieved with pArchive->pFile.
    drfs_archive* pArchive;

    // The internal file handle for use by the archive that owns it.
    drfs_handle internalFileHandle;


    // Flags. Can be a combination of the following:
    //   DR_FS_OWNS_PARENT_ARCHIVE
    int flags;


    // The critical section for locking and unlocking files.
#ifdef _WIN32
#ifdef DR_FS_WIN32_USE_EVENT_MUTEX
    HANDLE lock;
#else
    CRITICAL_SECTION lock;
#endif
#else
    pthread_mutex_t lock;
#endif
};


//// Path Manipulation ////
//
// Functionality below is taken straight from dr_path, but namespaced as "drfs" to avoid naming conflicts.

// Structure representing a section of a path.
typedef struct
{
    size_t offset;
    size_t length;
} drfs_drpath_segment;

// Structure used for iterating over a path while at the same time providing useful and easy-to-use information about the iteration.
typedef struct drfs_drpath_iterator
{
    const char* path;
    drfs_drpath_segment segment;
} drfs_drpath_iterator;

static dr_bool32 drfs_drpath_next(drfs_drpath_iterator* i)
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

static dr_bool32 drfs_drpath_prev(drfs_drpath_iterator* i)
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

static dr_bool32 drfs_drpath_first(const char* path, drfs_drpath_iterator* i)
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

static dr_bool32 drfs_drpath_last(const char* path, drfs_drpath_iterator* i)
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

    return drfs_drpath_prev(i);
}

static dr_bool32 drfs_drpath_segments_equal(const char* s0Path, const drfs_drpath_segment s0, const char* s1Path, const drfs_drpath_segment s1)
{
    if (s0Path == NULL || s1Path == NULL) {
        return DR_FALSE;
    }

    if (s0.length != s1.length) {
        return DR_FALSE;
    }

    return strncmp(s0Path + s0.offset, s1Path + s1.offset, s0.length) == 0;
}

static dr_bool32 drfs_drpath_iterators_equal(const drfs_drpath_iterator i0, const drfs_drpath_iterator i1)
{
    return drfs_drpath_segments_equal(i0.path, i0.segment, i1.path, i1.segment);
}

static dr_bool32 drfs_drpath_is_linux_style_root_segment(const drfs_drpath_iterator i)
{
    if (i.path == NULL) {
        return DR_FALSE;
    }

    if (i.segment.offset == 0 && i.segment.length == 0) {
        return DR_TRUE;    // "/" style root.
    }

    return DR_FALSE;
}

static dr_bool32 drfs_drpath_is_win32_style_root_segment(const drfs_drpath_iterator i)
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

static dr_bool32 drfs_drpath_is_root_segment(const drfs_drpath_iterator i)
{
    return drfs_drpath_is_linux_style_root_segment(i) || drfs_drpath_is_win32_style_root_segment(i);
}

static dr_bool32 drfs_drpath_append_iterator(char* base, size_t baseBufferSizeInBytes, drfs_drpath_iterator i)
{
    if (base == NULL) {
        return DR_FALSE;
    }

    size_t path1Length = strlen(base);
    size_t path2Length = i.segment.length;

    if (path1Length >= baseBufferSizeInBytes) {
        return DR_FALSE;
    }

    if (drfs_drpath_is_linux_style_root_segment(i)) {
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

    drfs__strncpy_s(base + path1Length, baseBufferSizeInBytes - path1Length, i.path + i.segment.offset, path2Length);

    return DR_TRUE;
}

static dr_bool32 drfs_drpath_is_descendant(const char* descendantAbsolutePath, const char* parentAbsolutePath)
{
    drfs_drpath_iterator iChild;
    if (!drfs_drpath_first(descendantAbsolutePath, &iChild)) {
        return DR_FALSE;   // The descendant is an empty string which makes it impossible for it to be a descendant.
    }

    drfs_drpath_iterator iParent;
    if (drfs_drpath_first(parentAbsolutePath, &iParent))
    {
        do
        {
            // If the segment is different, the paths are different and thus it is not a descendant.
            if (!drfs_drpath_iterators_equal(iParent, iChild)) {
                return DR_FALSE;
            }

            if (!drfs_drpath_next(&iChild)) {
                return DR_FALSE;   // The descendant is shorter which means it's impossible for it to be a descendant.
            }

        } while (drfs_drpath_next(&iParent));
    }

    return DR_TRUE;
}

static dr_bool32 drfs_drpath_is_child(const char* childAbsolutePath, const char* parentAbsolutePath)
{
    drfs_drpath_iterator iChild;
    if (!drfs_drpath_first(childAbsolutePath, &iChild)) {
        return DR_FALSE;   // The descendant is an empty string which makes it impossible for it to be a descendant.
    }

    drfs_drpath_iterator iParent;
    if (drfs_drpath_first(parentAbsolutePath, &iParent))
    {
        do
        {
            // If the segment is different, the paths are different and thus it is not a descendant.
            if (!drfs_drpath_iterators_equal(iParent, iChild)) {
                return DR_FALSE;
            }

            if (!drfs_drpath_next(&iChild)) {
                return DR_FALSE;   // The descendant is shorter which means it's impossible for it to be a descendant.
            }

        } while (drfs_drpath_next(&iParent));
    }

    // At this point we have finished iteration of the parent, which should be shorter one. We now do one more iterations of
    // the child to ensure it is indeed a direct child and not a deeper descendant.
    return !drfs_drpath_next(&iChild);
}

static void drfs_drpath_base_path(char* path)
{
    if (path == NULL) {
        return;
    }

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
}

static void drfs_drpath_copy_base_path(const char* path, char* baseOut, size_t baseSizeInBytes)
{
    if (path == NULL || baseOut == NULL || baseSizeInBytes == 0) {
        return;
    }

    drfs__strcpy_s(baseOut, baseSizeInBytes, path);
    drfs_drpath_base_path(baseOut);
}

static const char* drfs_drpath_file_name(const char* path)
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

static const char* drfs_drpath_extension(const char* path)
{
    if (path == NULL) {
        return NULL;
    }

    const char* extension     = drfs_drpath_file_name(path);
    const char* lastoccurance = 0;

    // Just find the last '.' and return.
    while (extension[0] != '\0')
    {
        extension += 1;

        if (extension[0] == '.') {
            extension    += 1;
            lastoccurance = extension;
        }
    }

    return (lastoccurance != 0) ? lastoccurance : extension;
}

static dr_bool32 drfs_drpath_equal(const char* path1, const char* path2)
{
    if (path1 == NULL || path2 == NULL) {
        return DR_FALSE;
    }

    if (path1 == path2 || (path1[0] == '\0' && path2[0] == '\0')) {
        return DR_TRUE;    // Two empty paths are treated as the same.
    }

    drfs_drpath_iterator iPath1;
    drfs_drpath_iterator iPath2;
    if (drfs_drpath_first(path1, &iPath1) && drfs_drpath_first(path2, &iPath2))
    {
        dr_bool32 isPath1Valid;
        dr_bool32 isPath2Valid;

        do
        {
            if (!drfs_drpath_iterators_equal(iPath1, iPath2)) {
                return DR_FALSE;
            }

            isPath1Valid = drfs_drpath_next(&iPath1);
            isPath2Valid = drfs_drpath_next(&iPath2);

        } while (isPath1Valid && isPath2Valid);

        // At this point either iPath1 and/or iPath2 have finished iterating. If both of them are at the end, the two paths are equal.
        return isPath1Valid == isPath2Valid && iPath1.path[iPath1.segment.offset] == '\0' && iPath2.path[iPath2.segment.offset] == '\0';
    }

    return DR_FALSE;
}

static dr_bool32 drfs_drpath_is_relative(const char* path)
{
    if (path == NULL) {
        return DR_FALSE;
    }

    drfs_drpath_iterator seg;
    if (drfs_drpath_first(path, &seg)) {
        return !drfs_drpath_is_root_segment(seg);
    }

    // We'll get here if the path is empty. We consider this to be a relative path.
    return DR_TRUE;
}

static dr_bool32 drfs_drpath_is_absolute(const char* path)
{
    return !drfs_drpath_is_relative(path);
}

static dr_bool32 drfs_drpath_append(char* base, size_t baseBufferSizeInBytes, const char* other)
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

    drfs__strncpy_s(base + path1Length, baseBufferSizeInBytes - path1Length, other, path2Length);

    return DR_TRUE;
}

static dr_bool32 drfs_drpath_copy_and_append(char* dst, size_t dstSizeInBytes, const char* base, const char* other)
{
    if (dst == NULL || dstSizeInBytes == 0) {
        return DR_FALSE;
    }

    drfs__strcpy_s(dst, dstSizeInBytes, base);
    return drfs_drpath_append(dst, dstSizeInBytes, other);
}

// This function recursively cleans a path which is defined as a chain of iterators. This function works backwards, which means at the time of calling this
// function, each iterator in the chain should be placed at the end of the path.
//
// This does not write the null terminator, nor a leading slash for absolute paths.
static size_t _drfs_drpath_clean_trywrite(drfs_drpath_iterator* iterators, unsigned int iteratorCount, char* pathOut, size_t pathOutSizeInBytes, unsigned int ignoreCounter)
{
    if (iteratorCount == 0) {
        return 0;
    }

    drfs_drpath_iterator isegment = iterators[iteratorCount - 1];


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

    drfs_drpath_iterator prev = isegment;
    if (!drfs_drpath_prev(&prev))
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
        bytesWritten = _drfs_drpath_clean_trywrite(iterators, iteratorCount, pathOut, pathOutSizeInBytes, ignoreCounter);
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
                drfs__strncpy_s(pathOut, pathOutSizeInBytes, isegment.path + isegment.segment.offset, isegment.segment.length);
                bytesWritten += isegment.segment.length;
            }
            else
            {
                drfs__strncpy_s(pathOut, pathOutSizeInBytes, isegment.path + isegment.segment.offset, pathOutSizeInBytes);
                bytesWritten += pathOutSizeInBytes;
            }
        }
    }

    return bytesWritten;
}

static size_t drfs_drpath_append_and_clean(char* dst, size_t dstSizeInBytes, const char* base, const char* other)
{
    if (base == NULL || other == NULL) {
        return 0;
    }

    drfs_drpath_iterator last[2];
    dr_bool32 isPathEmpty0 = !drfs_drpath_last(base,  last + 0);
    dr_bool32 isPathEmpty1 = !drfs_drpath_last(other, last + 1);

    if (isPathEmpty0 && isPathEmpty1) {
        return 0;   // Both input strings are empty.
    }

    size_t bytesWritten = 0;
    if (base[0] == '/') {
        if (dst != NULL && dstSizeInBytes > 1) {
            dst[0] = '/';
            bytesWritten = 1;
        }
    }

    bytesWritten += _drfs_drpath_clean_trywrite(last, 2, dst + bytesWritten, dstSizeInBytes - bytesWritten - 1, 0);  // -1 to ensure there is enough room for a null terminator later on.
    if (dstSizeInBytes > bytesWritten) {
        dst[bytesWritten] = '\0';
    }

    return bytesWritten + 1;
}


//// Private Utiltities ////

// Recursively creates the given directory structure on the native file system.
static dr_bool32 drfs_mkdir_recursive_native(const char* absolutePath);

// Determines whether or not the given path is valid for writing based on the base write path and whether or not
// the write guard is enabled.
static dr_bool32 drfs_validate_write_path(drfs_context* pContext, const char* absoluteOrRelativePath, char* absolutePathOut, unsigned int absolutePathOutSize)
{
    // If the path is relative, we need to convert to absolute. Then, if the write directory guard is enabled, we need to check that it's a descendant of the base path.
    if (drfs_drpath_is_relative(absoluteOrRelativePath)) {
        if (drfs_drpath_append_and_clean(absolutePathOut, absolutePathOutSize, pContext->writeBaseDirectory, absoluteOrRelativePath)) {
            absoluteOrRelativePath = absolutePathOut;
        } else {
            return DR_FALSE;
        }
    } else {
        if (drfs__strcpy_s(absolutePathOut, absolutePathOutSize, absoluteOrRelativePath) != 0) {
            return DR_FALSE;
        }
    }

    // If you trigger this assert it means you're trying to open a file for writing with a relative path but haven't yet
    // set the write directory. Either set the write directory with drfs_set_base_write_directory(), or use an absolute
    // path to open the file.
    assert(drfs_drpath_is_absolute(absoluteOrRelativePath));
    if (!drfs_drpath_is_absolute(absoluteOrRelativePath)) {
        return DR_FALSE;
    }


    if (drfs_is_write_directory_guard_enabled(pContext)) {
        if (drfs_drpath_is_descendant(absoluteOrRelativePath, pContext->writeBaseDirectory)) {
            return DR_TRUE;
        } else {
            return DR_FALSE;
        }
    } else {
        return DR_TRUE;
    }
}

// A simple helper function for determining the access mode to use for an archive file based on the access mode
// of a file within that archive.
static unsigned int drfs_archive_access_mode(unsigned int fileAccessMode)
{
    return (fileAccessMode == DRFS_READ) ? DRFS_READ : DRFS_READ | DRFS_WRITE | DRFS_EXISTING;
}



//// Platform-Specific Section ////

// The functions in this section implement a common abstraction for working with files on the native file system. When adding
// support for a new platform you'll probably want to either implement a platform-specific back-end or force stdio.

#if defined(_WIN32)
#define DR_FS_USE_WIN32
#else
#define DR_FS_USE_STDIO
#endif

// Low-level function for opening a file on the native file system.
//
// This will fail if attempting to open a file that's inside an archive such as a zip file. It can only open
// files that are sitting on the native file system.
//
// The given file path must be absolute.
static drfs_result drfs_open_native_file(const char* absolutePath, unsigned int accessMode, drfs_handle* pHandleOut);

// Closes the given native file.
static void drfs_close_native_file(drfs_handle file);

// Determines whether or not the given path refers to a directory on the native file system.
static dr_bool32 drfs_is_native_directory(const char* absolutePath);

// Determines whether or not the given path refers to a file on the native file system. This will return DR_FALSE for directories.
static dr_bool32 drfs_is_native_file(const char* absolutePath);

// Deletes a native file.
static drfs_result drfs_delete_native_file(const char* absolutePath);

// Creates a directory on the native file system.
static drfs_result drfs_mkdir_native(const char* absolutePath);

// Moves or renames a native file. Fails if the target already exists.
static drfs_result drfs_move_native_file(const char* absolutePathOld, const char* absolutePathNew);

// Creates a copy of a native file.
static drfs_result drfs_copy_native_file(const char* absolutePathSrc, const char* absolutePathDst, dr_bool32 failIfExists);

// Reads data from the given native file.
static drfs_result drfs_read_native_file(drfs_handle file, void* pDataOut, size_t bytesToRead, size_t* pBytesReadOut);

// Writes data to the given native file.
static drfs_result drfs_write_native_file(drfs_handle file, const void* pData, size_t bytesToWrite, size_t* pBytesWrittenOut);

// Seeks the given native file.
static drfs_result drfs_seek_native_file(drfs_handle file, dr_int64 bytesToSeek, drfs_seek_origin origin);

// Retrieves the read/write pointer of the given native file.
static dr_uint64 drfs_tell_native_file(drfs_handle file);

// Retrieves the size of the given native file.
static dr_uint64 drfs_get_native_file_size(drfs_handle file);

// Flushes the given native file.
static void drfs_flush_native_file(drfs_handle file);

// Retrieves information about the file OR DIRECTORY at the given path on the native file system.
//
// <fi> is allowed to be null, in which case the call is equivalent to simply checking if the file or directory exists.
static drfs_result drfs_get_native_file_info(const char* absolutePath, drfs_file_info* fi);

// Creates an iterator for iterating over the native files in the given directory.
//
// The iterator will be deleted with drfs_end_native_iteration().
static drfs_handle drfs_begin_native_iteration(const char* absolutePath);

// Deletes the iterator that was returned with drfs_end_native_iteration().
static void drfs_end_native_iteration(drfs_handle iterator);

// Retrieves information about the next native file based on the given iterator.
static dr_bool32 drfs_next_native_iteration(drfs_handle iterator, drfs_file_info* fi);



#ifdef DR_FS_USE_WIN32
static drfs_result drfs__GetLastError_to_result()
{
    switch (GetLastError())
    {
    case ERROR_SUCCESS:             return drfs_success;
    case ERROR_FILE_NOT_FOUND:      return drfs_does_not_exist;
    case ERROR_PATH_NOT_FOUND:      return drfs_does_not_exist;
    case ERROR_TOO_MANY_OPEN_FILES: return drfs_too_many_open_files;
    case ERROR_ACCESS_DENIED:       return drfs_permission_denied;
    case ERROR_NOT_ENOUGH_MEMORY:   return drfs_out_of_memory;
    case ERROR_DISK_FULL:           return drfs_no_space;
    case ERROR_HANDLE_EOF:          return drfs_at_end_of_file;
    case ERROR_NEGATIVE_SEEK:       return drfs_negative_seek;
    default: return drfs_unknown_error;
    }
}

static drfs_result drfs_open_native_file(const char* absolutePath, unsigned int accessMode, drfs_handle* pHandleOut)
{
    assert(absolutePath != NULL);
    assert(pHandleOut != NULL);

    DWORD dwDesiredAccess       = 0;
    DWORD dwShareMode           = 0;
    DWORD dwCreationDisposition = OPEN_EXISTING;

    if ((accessMode & DRFS_READ) != 0) {
        dwDesiredAccess |= FILE_GENERIC_READ;
        dwShareMode     |= FILE_SHARE_READ;
    }

    if ((accessMode & DRFS_WRITE) != 0) {
        dwDesiredAccess |= FILE_GENERIC_WRITE;

        if ((accessMode & DRFS_EXISTING) != 0) {
            if ((accessMode & DRFS_TRUNCATE) != 0) {
                dwCreationDisposition = TRUNCATE_EXISTING;
            } else {
                dwCreationDisposition = OPEN_EXISTING;
            }
        } else {
            if ((accessMode & DRFS_TRUNCATE) != 0) {
                dwCreationDisposition = CREATE_ALWAYS;
            } else {
                dwCreationDisposition = OPEN_ALWAYS;
            }
        }
    }


    HANDLE hFile = CreateFileA(absolutePath, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        // We failed to create the file, however it may be because we are trying to create the file and the directory structure does not exist yet. In this
        // case we want to try creating the directory structure and try again.
        if ((accessMode & DRFS_WRITE) != 0 && (accessMode & DRFS_CREATE_DIRS) != 0)
        {
            char dirAbsolutePath[DRFS_MAX_PATH];
            drfs_drpath_copy_base_path(absolutePath, dirAbsolutePath, sizeof(dirAbsolutePath));

            if (!drfs_is_native_directory(dirAbsolutePath) && drfs_mkdir_recursive_native(dirAbsolutePath)) {
                hFile = CreateFileA(absolutePath, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
            }
        }
    }

    if (hFile != INVALID_HANDLE_VALUE) {
        *pHandleOut = (drfs_handle)hFile;
        return drfs_success;
    }

    return drfs__GetLastError_to_result();
}

static void drfs_close_native_file(drfs_handle file)
{
    CloseHandle((HANDLE)file);
}

static dr_bool32 drfs_is_native_directory(const char* absolutePath)
{
    DWORD attributes = GetFileAttributesA(absolutePath);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

static dr_bool32 drfs_is_native_file(const char* absolutePath)
{
    DWORD attributes = GetFileAttributesA(absolutePath);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

static drfs_result drfs_delete_native_file(const char* absolutePath)
{
    BOOL wasSuccessful;

    DWORD attributes = GetFileAttributesA(absolutePath);
    if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        wasSuccessful = DeleteFileA(absolutePath);
    } else {
        wasSuccessful = RemoveDirectoryA(absolutePath);
    }

    if (wasSuccessful) {
        return drfs_success;
    }

    return drfs__GetLastError_to_result();
}

static drfs_result drfs_mkdir_native(const char* absolutePath)
{
    BOOL wasSuccessful = CreateDirectoryA(absolutePath, NULL);
    if (wasSuccessful) {
        return drfs_success;
    }

    return drfs__GetLastError_to_result();
}

static drfs_result drfs_move_native_file(const char* absolutePathOld, const char* absolutePathNew)
{
    BOOL wasSuccessful = MoveFileExA(absolutePathOld, absolutePathNew, 0);
    if (wasSuccessful) {
        return drfs_success;
    }

    return drfs__GetLastError_to_result();
}

static drfs_result drfs_copy_native_file(const char* absolutePathSrc, const char* absolutePathDst, dr_bool32 failIfExists)
{
    BOOL wasSuccessful = CopyFileA(absolutePathSrc, absolutePathDst, failIfExists);
    if (wasSuccessful) {
        return drfs_success;
    }

    return drfs__GetLastError_to_result();
}

static drfs_result drfs_read_native_file(drfs_handle file, void* pDataOut, size_t bytesToRead, size_t* pBytesReadOut)
{
    // Unfortunately Win32 expects a DWORD for the number of bytes to read, however we accept size_t. We need to loop to ensure
    // we handle data > 4GB correctly.

    size_t totalBytesRead = 0;

    char* pDst = (char*)pDataOut;
    dr_uint64 bytesRemaining = bytesToRead;
    while (bytesRemaining > 0)
    {
        DWORD bytesToProcess;
        if (bytesRemaining > UINT_MAX) {
            bytesToProcess = UINT_MAX;
        } else {
            bytesToProcess = (DWORD)bytesRemaining;
        }


        DWORD bytesRead;
        BOOL wasSuccessful = ReadFile((HANDLE)file, pDst, bytesToProcess, &bytesRead, NULL);
        if (!wasSuccessful || bytesRead != bytesToProcess)
        {
            // We failed to read the chunk.
            totalBytesRead += bytesRead;

            if (pBytesReadOut) {
                *pBytesReadOut = totalBytesRead;
            }

            return drfs__GetLastError_to_result();
        }

        pDst += bytesRead;
        bytesRemaining -= bytesRead;
        totalBytesRead += bytesRead;
    }


    if (pBytesReadOut != NULL) {
        *pBytesReadOut = totalBytesRead;
    }

    return drfs_success;
}

static drfs_result drfs_write_native_file(drfs_handle file, const void* pData, size_t bytesToWrite, size_t* pBytesWrittenOut)
{
    // Unfortunately Win32 expects a DWORD for the number of bytes to write, however we accept size_t. We need to loop to ensure
    // we handle data > 4GB correctly.

    size_t totalBytesWritten = 0;
    const char* pSrc = (const char*)pData;

    size_t bytesRemaining = bytesToWrite;
    while (bytesRemaining > 0)
    {
        DWORD bytesToProcess;
        if (bytesRemaining > UINT_MAX) {
            bytesToProcess = UINT_MAX;
        } else {
            bytesToProcess = (DWORD)bytesRemaining;
        }

        DWORD bytesWritten;
        BOOL wasSuccessful = WriteFile((HANDLE)file, pSrc, bytesToProcess, &bytesWritten, NULL);
        if (!wasSuccessful || bytesWritten != bytesToProcess)
        {
            // We failed to write the chunk.
            totalBytesWritten += bytesWritten;

            if (pBytesWrittenOut) {
                *pBytesWrittenOut = totalBytesWritten;
            }

            return drfs__GetLastError_to_result();
        }

        pSrc += bytesWritten;
        bytesRemaining -= bytesWritten;
        totalBytesWritten += bytesWritten;
    }

    if (pBytesWrittenOut) {
        *pBytesWrittenOut = totalBytesWritten;
    }

    return drfs_success;;
}

static drfs_result drfs_seek_native_file(drfs_handle file, dr_int64 bytesToSeek, drfs_seek_origin origin)
{
    LARGE_INTEGER lNewFilePointer;
    LARGE_INTEGER lDistanceToMove;
    lDistanceToMove.QuadPart = bytesToSeek;

    DWORD dwMoveMethod = FILE_CURRENT;
    if (origin == drfs_origin_start) {
        dwMoveMethod = FILE_BEGIN;
    } else if (origin == drfs_origin_end) {
        dwMoveMethod = FILE_END;
    }

    BOOL wasSuccessful = SetFilePointerEx((HANDLE)file, lDistanceToMove, &lNewFilePointer, dwMoveMethod);
    if (!wasSuccessful) {
        return drfs__GetLastError_to_result();
    }

    return drfs_success;
}

static dr_uint64 drfs_tell_native_file(drfs_handle file)
{
    LARGE_INTEGER lNewFilePointer;
    LARGE_INTEGER lDistanceToMove;
    lDistanceToMove.QuadPart = 0;

    if (SetFilePointerEx((HANDLE)file, lDistanceToMove, &lNewFilePointer, FILE_CURRENT)) {
        return (dr_uint64)lNewFilePointer.QuadPart;
    }

    return 0;
}

static dr_uint64 drfs_get_native_file_size(drfs_handle file)
{
    LARGE_INTEGER fileSize;
    if (GetFileSizeEx((HANDLE)file, &fileSize)) {
        return (dr_uint64)fileSize.QuadPart;
    }

    return 0;
}

static void drfs_flush_native_file(drfs_handle file)
{
    FlushFileBuffers((HANDLE)file);
}

static drfs_result drfs_get_native_file_info(const char* absolutePath, drfs_file_info* fi)
{
    assert(absolutePath != NULL);

    // <fi> is allowed to be null, in which case the call is equivalent to simply checking if the file exists.
    if (fi == NULL) {
        if (GetFileAttributesA(absolutePath) != INVALID_FILE_ATTRIBUTES) {
            return drfs_success;
        }

        return drfs_does_not_exist;
    }

    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (GetFileAttributesExA(absolutePath, GetFileExInfoStandard, &fad))
    {
        ULARGE_INTEGER liSize;
        liSize.LowPart  = fad.nFileSizeLow;
        liSize.HighPart = fad.nFileSizeHigh;

        ULARGE_INTEGER liTime;
        liTime.LowPart  = fad.ftLastWriteTime.dwLowDateTime;
        liTime.HighPart = fad.ftLastWriteTime.dwHighDateTime;

        strcpy_s(fi->absolutePath, sizeof(fi->absolutePath), absolutePath);
        fi->sizeInBytes      = liSize.QuadPart;
        fi->lastModifiedTime = liTime.QuadPart;

        fi->attributes = 0;
        if ((fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            fi->attributes |= DRFS_FILE_ATTRIBUTE_DIRECTORY;
        }
        if ((fad.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0) {
            fi->attributes |= DRFS_FILE_ATTRIBUTE_READONLY;
        }

        return drfs_success;
    }

    return drfs__GetLastError_to_result();
}

typedef struct
{
    HANDLE hFind;
    WIN32_FIND_DATAA ffd;
    char directoryPath[1];
} drfs_iterator_win32;

static drfs_handle drfs_begin_native_iteration(const char* absolutePath)
{
    assert(drfs_drpath_is_absolute(absolutePath));

    char searchQuery[DRFS_MAX_PATH];
    if (strcpy_s(searchQuery, sizeof(searchQuery), absolutePath) != 0) {
        return NULL;
    }

    unsigned int searchQueryLength = (unsigned int)strlen(searchQuery);
    if (searchQueryLength >= DRFS_MAX_PATH - 3) {
        return NULL;    // Path is too long.
    }

    searchQuery[searchQueryLength + 0] = '\\';
    searchQuery[searchQueryLength + 1] = '*';
    searchQuery[searchQueryLength + 2] = '\0';

    drfs_iterator_win32* pIterator = (drfs_iterator_win32*)malloc(sizeof(*pIterator) + searchQueryLength);
    if (pIterator == NULL) {
        return NULL;
    }

    ZeroMemory(pIterator, sizeof(*pIterator));

    pIterator->hFind = FindFirstFileA(searchQuery, &pIterator->ffd);
    if (pIterator->hFind == INVALID_HANDLE_VALUE) {
        free(pIterator);
        return NULL;    // Failed to begin search.
    }

    strcpy_s(pIterator->directoryPath, searchQueryLength + 1, absolutePath);     // This won't fail in practice.
    return (drfs_handle)pIterator;
}

static void drfs_end_native_iteration(drfs_handle iterator)
{
    drfs_iterator_win32* pIterator = (drfs_iterator_win32*)iterator;
    assert(pIterator != NULL);

    if (pIterator->hFind) {
        FindClose(pIterator->hFind);
    }

    free(pIterator);
}

static dr_bool32 drfs_next_native_iteration(drfs_handle iterator, drfs_file_info* fi)
{
    drfs_iterator_win32* pIterator = (drfs_iterator_win32*)iterator;
    assert(pIterator != NULL);

    if (pIterator->hFind != INVALID_HANDLE_VALUE && pIterator->hFind != NULL)
    {
        // Skip past "." and ".." directories.
        while (strcmp(pIterator->ffd.cFileName, ".") == 0 || strcmp(pIterator->ffd.cFileName, "..") == 0) {
            if (FindNextFileA(pIterator->hFind, &pIterator->ffd) == 0) {
                return DR_FALSE;
            }
        }

        if (fi != NULL)
        {
            // The absolute path actually needs to be set to the relative path. The higher level APIs are the once responsible for translating
            // it back to an absolute path.
            drfs__strcpy_s(fi->absolutePath, sizeof(fi->absolutePath), pIterator->ffd.cFileName);

            ULARGE_INTEGER liSize;
            liSize.LowPart  = pIterator->ffd.nFileSizeLow;
            liSize.HighPart = pIterator->ffd.nFileSizeHigh;
            fi->sizeInBytes = liSize.QuadPart;

            ULARGE_INTEGER liTime;
            liTime.LowPart  = pIterator->ffd.ftLastWriteTime.dwLowDateTime;
            liTime.HighPart = pIterator->ffd.ftLastWriteTime.dwHighDateTime;
            fi->lastModifiedTime = liTime.QuadPart;

            fi->attributes = 0;
            if ((pIterator->ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                fi->attributes |= DRFS_FILE_ATTRIBUTE_DIRECTORY;
            }
            if ((pIterator->ffd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0) {
                fi->attributes |= DRFS_FILE_ATTRIBUTE_READONLY;
            }
        }

        if (!FindNextFileA(pIterator->hFind, &pIterator->ffd)) {
            FindClose(pIterator->hFind);
            pIterator->hFind = NULL;
        }

        return DR_TRUE;
    }

    return DR_FALSE;
}
#endif //DR_FS_USE_WIN32


#ifdef DR_FS_USE_STDIO
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>

#ifdef __linux__
#include <sys/sendfile.h>
#endif

#define DRFS_HANDLE_TO_FD(file) ((int)((size_t)file) - 1)
#define DRFS_FD_TO_HANDLE(fd)   ((drfs_handle)(size_t)(fd + 1))

static drfs_result drfs__errno_to_result()
{
    switch (errno)
    {
    case EACCES:       return drfs_permission_denied;
    case EEXIST:       return drfs_already_exists;
    case EISDIR:       return drfs_permission_denied;
    case EMFILE:       return drfs_too_many_open_files;
    case ENFILE:       return drfs_too_many_open_files;
    case ENAMETOOLONG: return drfs_path_too_long;
    case ENOENT:       return drfs_does_not_exist;
    case ENOMEM:       return drfs_out_of_memory;
    case ENOSPC:       return drfs_no_space;
    case ENOTDIR:      return drfs_not_directory;
    case EOVERFLOW:    return drfs_too_large;
    case EROFS:        return drfs_permission_denied;
    case ETXTBSY:      return drfs_permission_denied;
    case EBADF:        return drfs_invalid_args;
    case EINVAL:       return drfs_invalid_args;
    default:           return drfs_unknown_error;
    }
}

static int drfs__open_fd(const char* absolutePath, int flags)
{
    return open64(absolutePath, flags, 0666);
}

static int drfs__stat64(const char* filename, struct stat64* st)
{
    return stat64(filename, st);
}

static int drfs__fstat64(int fd, struct stat64* st)
{
    return fstat64(fd, st);
}

static int drfs__mode_is_dir(int mode)
{
    return S_ISDIR(mode);
}

static int drfs__mode_has_write_permission(int mode)
{
    return (mode & S_IWUSR) != 0;
}


static drfs_result drfs_open_native_file(const char* absolutePath, unsigned int accessMode, drfs_handle* pHandleOut)
{
    assert(absolutePath != NULL);
    assert(pHandleOut != NULL);

    *pHandleOut = NULL;

    int flags = 0;
    if ((accessMode & DRFS_READ) != 0) {
        if ((accessMode & DRFS_WRITE) != 0) {
            flags = O_RDWR;
        } else {
            flags = O_RDONLY;
        }
    } else {
        if ((accessMode & DRFS_WRITE) != 0) {
            flags = O_WRONLY;
        } else {
            return drfs_permission_denied;    // Neither read nor write mode was specified.
        }
    }

    if ((accessMode & DRFS_TRUNCATE) != 0) {
        flags |= O_TRUNC;
    }

    if ((accessMode & DRFS_EXISTING) == 0) {
        flags |= O_CREAT;
    }

    int fd = drfs__open_fd(absolutePath, flags);
    if (fd == -1)
    {
        // We failed to open the file, however it could be because the directory structure is not in place. We need to check
        // the access mode flags for DRFS_CREATE_DIRS and try creating the directory structure.
        if ((accessMode & DRFS_WRITE) != 0 && (accessMode & DRFS_CREATE_DIRS) != 0)
        {
            char dirAbsolutePath[DRFS_MAX_PATH];
            drfs_drpath_copy_base_path(absolutePath, dirAbsolutePath, sizeof(dirAbsolutePath));

            if (!drfs_is_native_directory(dirAbsolutePath) && drfs_mkdir_recursive_native(dirAbsolutePath)) {
                fd = drfs__open_fd(absolutePath, flags);
            }
        }
    }

    if (fd == -1) {
        return drfs__errno_to_result();
    }

    *pHandleOut = DRFS_FD_TO_HANDLE(fd);
    return drfs_success;
}

static void drfs_close_native_file(drfs_handle file)
{
    close(DRFS_HANDLE_TO_FD(file));
}

static dr_bool32 drfs_is_native_directory(const char* absolutePath)
{
    struct stat64 info;
    if (drfs__stat64(absolutePath, &info) != 0) {
        return DR_FALSE;   // Likely the folder doesn't exist.
    }

    return drfs__mode_is_dir(info.st_mode);   // Only return DR_TRUE if it's a directory. Return DR_FALSE if it's a file.
}

static dr_bool32 drfs_is_native_file(const char* absolutePath)
{
    struct stat64 info;
    if (drfs__stat64(absolutePath, &info) != 0) {
        return DR_FALSE;   // Likely the folder doesn't exist.
    }

    return !drfs__mode_is_dir(info.st_mode);  // Only return DR_TRUE if it's a file. Return DR_FALSE if it's a directory.
}

static drfs_result drfs_delete_native_file(const char* absolutePath)
{
    if (remove(absolutePath) == 0) {
        return drfs_success;
    }

    return drfs__errno_to_result();
}

static drfs_result drfs_move_native_file(const char* absolutePathOld, const char* absolutePathNew)
{
    if (rename(absolutePathOld, absolutePathNew) == 0) {
        return drfs_success;
    }

    return drfs__errno_to_result();
}

static drfs_result drfs_mkdir_native(const char* absolutePath)
{
    if (mkdir(absolutePath, 0777) == 0) {
        return drfs_success;
    }

    return drfs__errno_to_result();
}

static drfs_result drfs_copy_native_file(const char* absolutePathSrc, const char* absolutePathDst, dr_bool32 failIfExists)
{
    if (drfs_drpath_equal(absolutePathSrc, absolutePathDst)) {
        if (!failIfExists) {
            return drfs_success;
        } else {
            return drfs_already_exists;
        }
    }

    int fdSrc = drfs__open_fd(absolutePathSrc, O_RDONLY);
    if (fdSrc == -1) {
        return drfs__errno_to_result();
    }

    int fdDst = drfs__open_fd(absolutePathDst, O_WRONLY | O_TRUNC | O_CREAT | ((failIfExists) ? O_EXCL : 0));
    if (fdDst == -1) {
        close(fdSrc);
        return drfs__errno_to_result();
    }

    drfs_result result = drfs_success;

    struct stat64 info;
    if (drfs__fstat64(fdSrc, &info) == 0)
    {
#ifdef __linux__
        ssize_t bytesRemaining = info.st_size;
        while (bytesRemaining > 0) {
            ssize_t bytesCopied = sendfile(fdDst, fdSrc, NULL, bytesRemaining);
            if (bytesCopied == -1) {
                result = drfs__errno_to_result();
                break;
            }

            bytesRemaining -= bytesCopied;
        }
#else
        char buffer[BUFSIZ];
        int bytesRead;
        while ((bytesRead = read(fdSrc, buffer, sizeof(buffer))) > 0) {
            if (write(fdDst, buffer, bytesRead) != bytesRead) {
                result = drfs__errno_to_result();
                break;
            }
        }
#endif
    }
    else
    {
        result = drfs__errno_to_result();
    }

    close(fdDst);
    close(fdSrc);

    // Permissions.
    chmod(absolutePathDst, info.st_mode & 07777);

    return result;
}

static drfs_result drfs_read_native_file(drfs_handle file, void* pDataOut, size_t bytesToRead, size_t* pBytesReadOut)
{
    // The documentation for read() states that if the number of bytes being read (bytesToRead) is larger than SSIZE_MAX,
    // the result is unspecified. We'll make things a bit more robust by explicitly checking for this and handling it.
    char* pDataOut8 = (char*)pDataOut;
    drfs_result result = drfs_success;

    size_t totalBytesRead = 0;
    while (bytesToRead > 0)
    {
        ssize_t bytesRead = read(DRFS_HANDLE_TO_FD(file), pDataOut8 + totalBytesRead, (bytesToRead > SSIZE_MAX) ? SSIZE_MAX : bytesToRead);
        if (bytesRead == -1) {
            result = drfs__errno_to_result();
            break;
        }

        if (bytesRead == 0) {
            break;  // Reached the end.
        }

        totalBytesRead += bytesRead;
        bytesToRead    -= bytesRead;
    }

    if (pBytesReadOut) {
        *pBytesReadOut = totalBytesRead;
    }

    return result;
}

static drfs_result drfs_write_native_file(drfs_handle file, const void* pData, size_t bytesToWrite, size_t* pBytesWrittenOut)
{
    // We want to handle writes in the same way as we do reads due to the return valid being signed.
    const char* pDataIn8 = (const char*)pData;
    drfs_result result = drfs_success;

    size_t totalBytesWritten = 0;
    while (bytesToWrite > 0)
    {
        ssize_t bytesWritten = write(DRFS_HANDLE_TO_FD(file), pDataIn8 + totalBytesWritten, (bytesToWrite > SSIZE_MAX) ? SSIZE_MAX : bytesToWrite);
        if (bytesWritten == -1) {
            result = drfs__errno_to_result();
            break;
        }

        if (bytesWritten == 0) {
            result = drfs__errno_to_result();
            break;
        }

        totalBytesWritten += bytesWritten;
        bytesToWrite      -= bytesWritten;
    }

    if (pBytesWrittenOut != NULL) {
        *pBytesWrittenOut = totalBytesWritten;
    }

    return result;
}

static drfs_result drfs_seek_native_file(drfs_handle file, dr_int64 bytesToSeek, drfs_seek_origin origin)
{
    int stdioOrigin = SEEK_CUR;
    if (origin == drfs_origin_start) {
        stdioOrigin = SEEK_SET;
    } else if (origin == drfs_origin_end) {
        stdioOrigin = SEEK_END;
    }

    if (lseek64(DRFS_HANDLE_TO_FD(file), bytesToSeek, stdioOrigin) == -1) {
        return drfs__errno_to_result();
    }

    return drfs_success;
}

static dr_uint64 drfs_tell_native_file(drfs_handle file)
{
    return lseek64(DRFS_HANDLE_TO_FD(file), 0, SEEK_CUR);
}

static dr_uint64 drfs_get_native_file_size(drfs_handle file)
{
    struct stat64 info;
    if (drfs__fstat64(DRFS_HANDLE_TO_FD(file), &info) != 0) {
        return 0;
    }

    return info.st_size;
}

static void drfs_flush_native_file(drfs_handle file)
{
    // The posix implementation does not require flushing.
    (void)file;
}

static drfs_result drfs_get_native_file_info(const char* absolutePath, drfs_file_info* fi)
{
    assert(absolutePath != NULL);

    struct stat64 info;
    if (stat64(absolutePath, &info) != 0) {
        return drfs__errno_to_result();
    }

    // <fi> is allowed to be null, in which case the call is equivalent to simply checking if the file exists.
    if (fi != NULL)
    {
        drfs__strcpy_s(fi->absolutePath, sizeof(fi->absolutePath), absolutePath);
        fi->sizeInBytes      = info.st_size;
        fi->lastModifiedTime = info.st_mtime;

        fi->attributes = 0;
        if (drfs__mode_is_dir(info.st_mode)) {
            fi->attributes |= DRFS_FILE_ATTRIBUTE_DIRECTORY;
        }
        if (drfs__mode_has_write_permission(info.st_mode) == DR_FALSE) {
            fi->attributes |= DRFS_FILE_ATTRIBUTE_READONLY;
        }
    }

    return drfs_success;
}

typedef struct
{
    DIR* dir;
    char directoryPath[1];
} drfs_iterator_posix;

static drfs_handle drfs_begin_native_iteration(const char* absolutePath)
{
    DIR* dir = opendir(absolutePath);
    if (dir == NULL) {
        return NULL;
    }

    drfs_iterator_posix* pIterator = (drfs_iterator_posix*)malloc(sizeof(drfs_iterator_posix) + strlen(absolutePath));
    if (pIterator == NULL) {
        return NULL;
    }

    pIterator->dir = dir;
    drfs__strcpy_s(pIterator->directoryPath, strlen(absolutePath) + 1, absolutePath);

    return pIterator;
}

static void drfs_end_native_iteration(drfs_handle iterator)
{
    drfs_iterator_posix* pIterator = (drfs_iterator_posix*)iterator;
    if (pIterator == NULL) {
        return;
    }

    closedir(pIterator->dir);
    free(pIterator);
}

static dr_bool32 drfs_next_native_iteration(drfs_handle iterator, drfs_file_info* fi)
{
    drfs_iterator_posix* pIterator = (drfs_iterator_posix*)iterator;
    if (pIterator == NULL || pIterator->dir == NULL) {
        return DR_FALSE;
    }

    struct dirent* info = readdir(pIterator->dir);
    if (info == NULL) {
        return DR_FALSE;
    }

    // Skip over "." and ".." folders.
    while (strcmp(info->d_name, ".") == 0 || strcmp(info->d_name, "..") == 0) {
        info = readdir(pIterator->dir);
        if (info == NULL) {
            return DR_FALSE;
        }
    }

    char fileAbsolutePath[DRFS_MAX_PATH];
    drfs_drpath_copy_and_append(fileAbsolutePath, sizeof(fileAbsolutePath), pIterator->directoryPath, info->d_name);

    if (drfs_get_native_file_info(fileAbsolutePath, fi)) {
        drfs__strcpy_s(fi->absolutePath, sizeof(fi->absolutePath), info->d_name);
    }

    return DR_TRUE;
}
#endif //DR_FS_USE_STDIO


static dr_bool32 drfs_mkdir_recursive_native(const char* absolutePath)
{
    char runningPath[DRFS_MAX_PATH];
    runningPath[0] = '\0';

    drfs_drpath_iterator iPathSeg;
    if (!drfs_drpath_first(absolutePath, &iPathSeg)) {
        return DR_FALSE;
    }

    // Never check the first segment because we can assume it always exists - it will always be the drive root.
    if (drfs_drpath_append_iterator(runningPath, sizeof(runningPath), iPathSeg))
    {
        // Loop over every directory until we find one that does not exist.
        while (drfs_drpath_next(&iPathSeg))
        {
            if (!drfs_drpath_append_iterator(runningPath, sizeof(runningPath), iPathSeg)) {
                return DR_FALSE;
            }

            if (!drfs_is_native_directory(runningPath)) {
                if (drfs_mkdir_native(runningPath) != drfs_success) {
                    return DR_FALSE;
                }

                break;
            }
        }


        // At this point all we need to do is create the remaining directories - we can assert that the directory does not exist
        // rather than actually checking it which should be a bit more efficient.
        while (drfs_drpath_next(&iPathSeg))
        {
            if (!drfs_drpath_append_iterator(runningPath, sizeof(runningPath), iPathSeg)) {
                return DR_FALSE;
            }

            assert(!drfs_is_native_directory(runningPath));

            if (drfs_mkdir_native(runningPath) != drfs_success) {
                return DR_FALSE;
            }
        }

        return DR_TRUE;
    }

    return DR_FALSE;
}



//// Native Archive Implementation ////

typedef struct
{
    // The access mode.
    unsigned int accessMode;

    // The absolute path of the directory.
    char absolutePath[1];

} drfs_archive_native;

static drfs_result drfs_open_archive__native(drfs_file* pArchiveFile, unsigned int accessMode, drfs_handle* pHandleOut)
{
    (void)pArchiveFile;
    (void)accessMode;

    // This function should never be called for native archives. Native archives are a special case because they are just directories
    // on the file system rather than actual files, and as such they are handled just slightly differently. This function is only
    // included here for this assert so we can ensure we don't incorrectly call this function.
    assert(DR_FALSE);

    *pHandleOut = NULL;
    return drfs_unknown_error;
}

static drfs_result drfs_open_archive__native__special(const char* absolutePath, unsigned int accessMode, drfs_handle* pHandleOut)
{
    assert(absolutePath != NULL);       // There is no notion of a file for native archives (which are just directories on the file system).
    assert(pHandleOut != NULL);

    *pHandleOut = NULL;

    size_t absolutePathLen = strlen(absolutePath);

    drfs_archive_native* pNativeArchive = (drfs_archive_native*)malloc(sizeof(*pNativeArchive) + absolutePathLen + 1);
    if (pNativeArchive == NULL) {
        return drfs_out_of_memory;
    }

    drfs__strcpy_s(pNativeArchive->absolutePath, absolutePathLen + 1, absolutePath);
    pNativeArchive->accessMode = accessMode;

    *pHandleOut = (drfs_handle)pNativeArchive;
    return drfs_success;
}

static void drfs_close_archive__native(drfs_handle archive)
{
    // Nothing to do except free the object.
    free(archive);
}

static drfs_result drfs_get_file_info__native(drfs_handle archive, const char* relativePath, drfs_file_info* fi)
{
    drfs_archive_native* pNativeArchive = (drfs_archive_native*)archive;
    assert(pNativeArchive != NULL);

    char absolutePath[DRFS_MAX_PATH];
    if (!drfs_drpath_copy_and_append(absolutePath, sizeof(absolutePath), pNativeArchive->absolutePath, relativePath)) {
        return drfs_path_too_long;
    }

    if (fi != NULL) {
        memset(fi, 0, sizeof(*fi));
    }

    return drfs_get_native_file_info(absolutePath, fi);
}

static drfs_handle drfs_begin_iteration__native(drfs_handle archive, const char* relativePath)
{
    drfs_archive_native* pNativeArchive = (drfs_archive_native*)archive;
    assert(pNativeArchive != NULL);

    char absolutePath[DRFS_MAX_PATH];
    if (!drfs_drpath_copy_and_append(absolutePath, sizeof(absolutePath), pNativeArchive->absolutePath, relativePath)) {
        return NULL;
    }

    return drfs_begin_native_iteration(absolutePath);
}

static void drfs_end_iteration__native(drfs_handle archive, drfs_handle iterator)
{
    (void)archive;
    assert(archive != NULL);
    assert(iterator != NULL);

    drfs_end_native_iteration(iterator);
}

static dr_bool32 drfs_next_iteration__native(drfs_handle archive, drfs_handle iterator, drfs_file_info* fi)
{
    (void)archive;
    assert(archive != NULL);
    assert(iterator != NULL);

    return drfs_next_native_iteration(iterator, fi);
}

static drfs_result drfs_delete_file__native(drfs_handle archive, const char* relativePath)
{
    assert(relativePath != NULL);

    drfs_archive_native* pNativeArchive = (drfs_archive_native*)archive;
    assert(pNativeArchive != NULL);

    char absolutePath[DRFS_MAX_PATH];
    if (!drfs_drpath_copy_and_append(absolutePath, sizeof(absolutePath), pNativeArchive->absolutePath, relativePath)) {
        return drfs_path_too_long;
    }

    return drfs_delete_native_file(absolutePath);
}

static drfs_result drfs_move_file__native(drfs_handle archive, const char* relativePathOld, const char* relativePathNew)
{
    assert(relativePathOld != NULL);
    assert(relativePathNew != NULL);

    drfs_archive_native* pNativeArchive = (drfs_archive_native*)archive;
    assert(pNativeArchive != NULL);

    char absolutePathOld[DRFS_MAX_PATH];
    if (!drfs_drpath_copy_and_append(absolutePathOld, sizeof(absolutePathOld), pNativeArchive->absolutePath, relativePathOld)) {
        return drfs_path_too_long;
    }

    char absolutePathNew[DRFS_MAX_PATH];
    if (!drfs_drpath_copy_and_append(absolutePathNew, sizeof(absolutePathNew), pNativeArchive->absolutePath, relativePathNew)) {
        return drfs_path_too_long;
    }

    return drfs_move_native_file(absolutePathOld, absolutePathNew);
}

static drfs_result drfs_create_directory__native(drfs_handle archive, const char* relativePath)
{
    assert(relativePath != NULL);

    drfs_archive_native* pNativeArchive = (drfs_archive_native*)archive;
    assert(pNativeArchive != NULL);

    char absolutePath[DRFS_MAX_PATH];
    if (!drfs_drpath_copy_and_append(absolutePath, sizeof(absolutePath), pNativeArchive->absolutePath, relativePath)) {
        return drfs_path_too_long;
    }

    return drfs_mkdir_native(absolutePath);
}

static drfs_result drfs_copy_file__native(drfs_handle archive, const char* relativePathSrc, const char* relativePathDst, dr_bool32 failIfExists)
{
    assert(relativePathSrc != NULL);
    assert(relativePathDst != NULL);

    drfs_archive_native* pNativeArchive = (drfs_archive_native*)archive;
    assert(pNativeArchive != NULL);

    char absolutePathSrc[DRFS_MAX_PATH];
    if (!drfs_drpath_copy_and_append(absolutePathSrc, sizeof(absolutePathSrc), pNativeArchive->absolutePath, relativePathSrc)) {
        return drfs_path_too_long;
    }

    char absolutePathDst[DRFS_MAX_PATH];
    if (!drfs_drpath_copy_and_append(absolutePathDst, sizeof(absolutePathDst), pNativeArchive->absolutePath, relativePathDst)) {
        return drfs_path_too_long;
    }

    return drfs_copy_native_file(absolutePathSrc, absolutePathDst, failIfExists);
}

static drfs_result drfs_open_file__native(drfs_handle archive, const char* relativePath, unsigned int accessMode, drfs_handle* pHandleOut)
{
    assert(archive != NULL);
    assert(relativePath != NULL);

    drfs_archive_native* pNativeArchive = (drfs_archive_native*)archive;
    assert(pNativeArchive != NULL);

    char absolutePath[DRFS_MAX_PATH];
    if (drfs_drpath_copy_and_append(absolutePath, sizeof(absolutePath), pNativeArchive->absolutePath, relativePath)) {
        return drfs_open_native_file(absolutePath, accessMode, pHandleOut);
    }

    return drfs_path_too_long;
}

static void drfs_close_file__native(drfs_handle archive, drfs_handle file)
{
    (void)archive;
    assert(archive != NULL);
    assert(file != NULL);

    drfs_close_native_file(file);
}

static drfs_result drfs_read_file__native(drfs_handle archive, drfs_handle file, void* pDataOut, size_t bytesToRead, size_t* pBytesReadOut)
{
    (void)archive;
    assert(archive != NULL);
    assert(file != NULL);

    return drfs_read_native_file(file, pDataOut, bytesToRead, pBytesReadOut);
}

static drfs_result drfs_write_file__native(drfs_handle archive, drfs_handle file, const void* pData, size_t bytesToWrite, size_t* pBytesWrittenOut)
{
    (void)archive;
    assert(archive != NULL);
    assert(file != NULL);

    return drfs_write_native_file(file, pData, bytesToWrite, pBytesWrittenOut);
}

static drfs_result drfs_seek_file__native(drfs_handle archive, drfs_handle file, dr_int64 bytesToSeek, drfs_seek_origin origin)
{
    (void)archive;
    assert(archive != NULL);
    assert(file != NULL);

    return drfs_seek_native_file(file, bytesToSeek, origin);
}

static dr_uint64 drfs_tell_file__native(drfs_handle archive, drfs_handle file)
{
    (void)archive;
    assert(archive != NULL);
    assert(file != NULL);

    return drfs_tell_native_file(file);
}

static dr_uint64 drfs_file_size__native(drfs_handle archive, drfs_handle file)
{
    (void)archive;
    assert(archive != NULL);
    assert(file != NULL);

    return drfs_get_native_file_size(file);
}

static void drfs_flush__native(drfs_handle archive, drfs_handle file)
{
    (void)archive;
    assert(archive != NULL);
    assert(file != NULL);

    drfs_flush_native_file(file);
}


// Finds the back-end callbacks by the given extension.
static dr_bool32 drfs_find_backend_by_extension(drfs_context* pContext, const char* extension, drfs_archive_callbacks* pCallbacksOut)
{
    if (pContext == NULL || extension == NULL || extension[0] == '\0') {
        return DR_FALSE;
    }

    for (unsigned int i = 0; i < pContext->archiveCallbacks.count; ++i)
    {
        if (pContext->archiveCallbacks.pBuffer[i].is_valid_extension) {
            if (pContext->archiveCallbacks.pBuffer[i].is_valid_extension(extension)) {
                if (pCallbacksOut) {
                    *pCallbacksOut = pContext->archiveCallbacks.pBuffer[i];
                }
                return DR_TRUE;
            }
        }
    }

    return DR_FALSE;
}

// Recursively claims ownership of parent archives so that when the child archive is deleted, so are it's parents.
static void drfs_recursively_claim_ownership_or_parent_archive(drfs_archive* pArchive)
{
    if (pArchive == NULL) {
        return;
    }

    pArchive->flags |= DR_FS_OWNS_PARENT_ARCHIVE;
    drfs_recursively_claim_ownership_or_parent_archive(pArchive->pParentArchive);
}

// Opens a native archive.
static drfs_result drfs_open_native_archive(drfs_context* pContext, const char* absolutePath, unsigned int accessMode, drfs_archive** ppArchive)
{
    assert(pContext != NULL);
    assert(ppArchive != NULL);
    assert(absolutePath != NULL);

    *ppArchive = NULL;

    drfs_handle internalArchiveHandle;
    drfs_result result = drfs_open_archive__native__special(absolutePath, accessMode, &internalArchiveHandle);
    if (result != drfs_success) {
        return result;
    }

    drfs_archive* pArchive = (drfs_archive*)malloc(sizeof(*pArchive));
    if (pArchive == NULL) {
        drfs_close_archive__native(internalArchiveHandle);
        return drfs_out_of_memory;
    }

    pArchive->pContext                     = pContext;
    pArchive->pParentArchive               = NULL;
    pArchive->pFile                        = NULL;
    pArchive->internalArchiveHandle        = internalArchiveHandle;
    pArchive->flags                        = 0;
    pArchive->callbacks.is_valid_extension = NULL;
    pArchive->callbacks.open_archive       = drfs_open_archive__native;
    pArchive->callbacks.close_archive      = drfs_close_archive__native;
    pArchive->callbacks.get_file_info      = drfs_get_file_info__native;
    pArchive->callbacks.begin_iteration    = drfs_begin_iteration__native;
    pArchive->callbacks.end_iteration      = drfs_end_iteration__native;
    pArchive->callbacks.next_iteration     = drfs_next_iteration__native;
    pArchive->callbacks.delete_file        = drfs_delete_file__native;
    pArchive->callbacks.create_directory   = drfs_create_directory__native;
    pArchive->callbacks.move_file          = drfs_move_file__native;
    pArchive->callbacks.copy_file          = drfs_copy_file__native;
    pArchive->callbacks.open_file          = drfs_open_file__native;
    pArchive->callbacks.close_file         = drfs_close_file__native;
    pArchive->callbacks.read_file          = drfs_read_file__native;
    pArchive->callbacks.write_file         = drfs_write_file__native;
    pArchive->callbacks.seek_file          = drfs_seek_file__native;
    pArchive->callbacks.tell_file          = drfs_tell_file__native;
    pArchive->callbacks.file_size          = drfs_file_size__native;
    pArchive->callbacks.flush_file         = drfs_flush__native;
    drfs__strcpy_s(pArchive->absolutePath, sizeof(pArchive->absolutePath), absolutePath);

    *ppArchive = pArchive;
    return drfs_success;
}

// Opens an archive from a file and callbacks.
static drfs_result drfs_open_non_native_archive(drfs_archive* pParentArchive, drfs_file* pArchiveFile, drfs_archive_callbacks* pBackEndCallbacks, const char* relativePath, unsigned int accessMode, drfs_archive** ppArchiveOut)
{
    assert(pParentArchive != NULL);
    assert(pArchiveFile != NULL);
    assert(pBackEndCallbacks != NULL);
    assert(ppArchiveOut != NULL);

    *ppArchiveOut = NULL;

    if (pBackEndCallbacks->open_archive == NULL) {
        return drfs_no_backend;
    }

    drfs_handle internalArchiveHandle;
    drfs_result result = pBackEndCallbacks->open_archive(pArchiveFile, accessMode, &internalArchiveHandle);
    if (result != drfs_success) {
        return result;
    }

    drfs_archive* pArchive = (drfs_archive*)malloc(sizeof(*pArchive));
    if (pArchive == NULL) {
        pBackEndCallbacks->close_archive(internalArchiveHandle);
        return drfs_out_of_memory;
    }

    pArchive->pContext              = pParentArchive->pContext;
    pArchive->pParentArchive        = pParentArchive;
    pArchive->pFile                 = pArchiveFile;
    pArchive->internalArchiveHandle = internalArchiveHandle;
    pArchive->flags                 = 0;
    pArchive->callbacks             = *pBackEndCallbacks;
    drfs_drpath_copy_and_append(pArchive->absolutePath, sizeof(pArchive->absolutePath), pParentArchive->absolutePath, relativePath);

    *ppArchiveOut = pArchive;
    return drfs_success;
}

// Attempts to open an archive from another archive.
static drfs_result drfs_open_non_native_archive_from_path(drfs_archive* pParentArchive, const char* relativePath, unsigned int accessMode, drfs_archive** ppArchiveOut)
{
    assert(pParentArchive != NULL);
    assert(ppArchiveOut != NULL);
    assert(relativePath != NULL);

    *ppArchiveOut = NULL;

    drfs_archive_callbacks backendCallbacks;
    if (!drfs_find_backend_by_extension(pParentArchive->pContext, drfs_drpath_extension(relativePath), &backendCallbacks)) {
        return drfs_no_backend;
    }

    drfs_file* pArchiveFile;
    drfs_result result = drfs_open_file_from_archive(pParentArchive, relativePath, accessMode, &pArchiveFile);
    if (result != drfs_success) {
        return result;
    }

    drfs_archive* pArchive;
    result = drfs_open_non_native_archive(pParentArchive, pArchiveFile, &backendCallbacks, relativePath, accessMode, &pArchive);
    if (pArchive == NULL) {
        return result;
    }

    *ppArchiveOut = pArchive;
    return drfs_success;
}


// Recursively opens the archive that owns the file at the given verbose path.
static drfs_result drfs_open_owner_archive_recursively_from_verbose_path(drfs_archive* pParentArchive, const char* relativePath, unsigned int accessMode, char* relativePathOut, size_t relativePathOutSize, drfs_archive** ppArchiveOut)
{
    assert(pParentArchive != NULL);
    assert(relativePath != NULL);
    assert(ppArchiveOut != NULL);

    *ppArchiveOut = NULL;

    if (pParentArchive->callbacks.get_file_info == NULL) {
        return drfs_no_backend;
    }

    drfs_file_info fi;
    if (pParentArchive->callbacks.get_file_info(pParentArchive->internalArchiveHandle, relativePath, &fi) == drfs_success)
    {
        // The file is in this archive.
        drfs__strcpy_s(relativePathOut, relativePathOutSize, relativePath);
        *ppArchiveOut = pParentArchive;
        return drfs_success;
    }
    else
    {
        char runningPath[DRFS_MAX_PATH];
        runningPath[0] = '\0';

        drfs_drpath_iterator segment;
        if (drfs_drpath_first(relativePath, &segment))
        {
            do
            {
                if (!drfs_drpath_append_iterator(runningPath, sizeof(runningPath), segment)) {
                    return drfs_path_too_long;
                }

                if (pParentArchive->callbacks.get_file_info(pParentArchive->internalArchiveHandle, runningPath, &fi) == drfs_success)
                {
                    if ((fi.attributes & DRFS_FILE_ATTRIBUTE_DIRECTORY) == 0)
                    {
                        // The running path points to an actual file. It could be a sub-archive.
                        drfs_archive_callbacks backendCallbacks;
                        if (drfs_find_backend_by_extension(pParentArchive->pContext, drfs_drpath_extension(runningPath), &backendCallbacks))
                        {
                            drfs_file* pNextArchiveFile;
                            drfs_open_file_from_archive(pParentArchive, runningPath, accessMode, &pNextArchiveFile);
                            if (pNextArchiveFile == NULL) {
                                break;    // Failed to open the archive file.
                            }

                            drfs_archive* pNextArchive;
                            drfs_open_non_native_archive(pParentArchive, pNextArchiveFile, &backendCallbacks, runningPath, accessMode, &pNextArchive);
                            if (pNextArchive == NULL) {
                                drfs_close(pNextArchiveFile);
                                break;
                            }

                            // At this point we should have an archive. We now need to call this function recursively if there are any segments left.
                            drfs_drpath_iterator nextsegment = segment;
                            if (drfs_drpath_next(&nextsegment))
                            {
                                drfs_archive* pOwnerArchive;
                                drfs_open_owner_archive_recursively_from_verbose_path(pNextArchive, nextsegment.path + nextsegment.segment.offset, accessMode, relativePathOut, relativePathOutSize, &pOwnerArchive);
                                if (pOwnerArchive == NULL) {
                                    drfs_close_archive(pNextArchive);
                                    break;
                                }

                                *ppArchiveOut = pOwnerArchive;
                                return drfs_success;
                            }
                            else
                            {
                                // We reached the end of the path. If we get here it means the file doesn't exist, because otherwise we would have caught it in the check right at the top.
                                drfs_close_archive(pNextArchive);
                                break;
                            }
                        }
                    }
                }
            } while (drfs_drpath_next(&segment));
        }

        // The running path is not part of this archive.
        // NOTE: Is this the correct return value we should be using? Should we be returning an error and setting the output archive to NULL?
        drfs__strcpy_s(relativePathOut, relativePathOutSize, relativePath);
        *ppArchiveOut = pParentArchive;
        return drfs_does_not_exist;
    }
}

// Opens the archive that owns the file at the given verbose path.
static drfs_result drfs_open_owner_archive_from_absolute_path(drfs_context* pContext, const char* absolutePath, unsigned int accessMode, char* relativePathOut, size_t relativePathOutSize, drfs_archive** ppArchiveOut)
{
    assert(pContext != NULL);
    assert(absolutePath != NULL);
    assert(ppArchiveOut != NULL);

    *ppArchiveOut = NULL;

    // We are currently assuming absolute path's are verbose. This means we don't need to do any searching on the file system. We just
    // move through the path and look for a segment with an extension matching one of the registered back-ends.

    char runningPath[DRFS_MAX_PATH];
    runningPath[0] = '\0';

    if (absolutePath[0] == '/') {
        runningPath[0] = '/';
        runningPath[1] = '\0';
    }

    drfs_drpath_iterator segment;
    if (drfs_drpath_first(absolutePath, &segment))
    {
        do
        {
            if (!drfs_drpath_append_iterator(runningPath, sizeof(runningPath), segment)) {
                return drfs_path_too_long;
            }

            if (!drfs_is_native_directory(runningPath))
            {
                char dirAbsolutePath[DRFS_MAX_PATH];
                drfs_drpath_copy_base_path(runningPath, dirAbsolutePath, sizeof(dirAbsolutePath));

                drfs_archive* pNativeArchive;
                drfs_result result = drfs_open_native_archive(pContext, dirAbsolutePath, accessMode, &pNativeArchive);
                if (result != drfs_success) {
                    return result;    // Failed to open the native archive.
                }


                // The running path is not a native directory. It could be an archive file.
                drfs_archive_callbacks backendCallbacks;
                if (drfs_find_backend_by_extension(pContext, drfs_drpath_extension(runningPath), &backendCallbacks))
                {
                    // The running path refers to an archive file. We need to try opening the archive here. If this fails, we
                    // need to return NULL.
                    drfs_archive* pArchive;
                    result = drfs_open_owner_archive_recursively_from_verbose_path(pNativeArchive, segment.path + segment.segment.offset, accessMode, relativePathOut, relativePathOutSize, &pArchive);
                    if (pArchive == NULL) {
                        drfs_close_archive(pNativeArchive);
                        return result;
                    }

                    *ppArchiveOut = pArchive;
                    return drfs_success;
                }
                else
                {
                    drfs__strcpy_s(relativePathOut, relativePathOutSize, segment.path + segment.segment.offset);
                    *ppArchiveOut = pNativeArchive;
                    return drfs_success;
                }
            }

        } while (drfs_drpath_next(&segment));
    }

    return drfs_does_not_exist;
}

// Recursively opens the archive that owns the file specified by the given relative path.
static drfs_result drfs_open_owner_archive_recursively_from_relative_path(drfs_archive* pParentArchive, const char* rootSearchPath, const char* relativePath, unsigned int accessMode, char* relativePathOut, size_t relativePathOutSize, drfs_archive** ppArchiveOut)
{
    assert(pParentArchive != NULL);
    assert(relativePath != NULL);
    assert(ppArchiveOut != NULL);

    *ppArchiveOut = NULL;

    if (pParentArchive->callbacks.get_file_info == NULL) {
        return drfs_no_backend;
    }

    // Always try the direct route by checking if the file exists within the archive first.
    drfs_file_info fi;
    if (pParentArchive->callbacks.get_file_info(pParentArchive->internalArchiveHandle, relativePath, &fi) == drfs_success)
    {
        // The file is in this archive.
        drfs__strcpy_s(relativePathOut, relativePathOutSize, relativePath);
        *ppArchiveOut = pParentArchive;
        return drfs_success;
    }
    else
    {
        // The file does not exist within this archive. We need to search the parent archive recursively. We never search above
        // the path specified by rootSearchPath.
        char runningPath[DRFS_MAX_PATH];
        drfs__strcpy_s(runningPath, sizeof(runningPath), rootSearchPath);

        // Part of rootSearchPath and relativePath will be overlapping. We want to begin searching at the non-overlapping section.
        drfs_drpath_iterator pathSeg0;
        drfs_drpath_iterator pathSeg1;
        dr_bool32 isSeg0Valid = drfs_drpath_first(rootSearchPath, &pathSeg0);
        dr_bool32 isSeg1Valid = drfs_drpath_first(relativePath,   &pathSeg1);
        while (isSeg0Valid && isSeg1Valid) {
            isSeg0Valid = drfs_drpath_next(&pathSeg0);
            isSeg1Valid = drfs_drpath_next(&pathSeg1);
        }

        relativePath = pathSeg1.path + pathSeg1.segment.offset;

        // Searching works as such:
        //   For each segment in "relativePath"
        //       If segment is archive then
        //           Open and search archive
        //       Else
        //           Search each archive file in this directory
        //       Endif
        drfs_drpath_iterator pathseg;
        if (drfs_drpath_first(relativePath, &pathseg))
        {
            do
            {
                char runningPathBase[DRFS_MAX_PATH];
                drfs__strcpy_s(runningPathBase, sizeof(runningPathBase), runningPath);

                if (!drfs_drpath_append_iterator(runningPath, sizeof(runningPath), pathseg)) {
                    return drfs_path_too_long;
                }

                drfs_archive* pNextArchive;
                drfs_open_non_native_archive_from_path(pParentArchive, runningPath, accessMode, &pNextArchive);
                if (pNextArchive != NULL)
                {
                    // It's an archive segment. We need to check this archive recursively, starting from the next segment.
                    drfs_drpath_iterator nextseg = pathseg;
                    if (!drfs_drpath_next(&nextseg)) {
                        // Should never actually get here, but if we do it means we've reached the end of the path. Assume the file could not be found.
                        drfs_close_archive(pNextArchive);
                        return drfs_unknown_error;
                    }

                    drfs_archive* pOwnerArchive;
                    drfs_open_owner_archive_recursively_from_relative_path(pNextArchive, "", nextseg.path + nextseg.segment.offset, accessMode, relativePathOut, relativePathOutSize, &pOwnerArchive);
                    if (pOwnerArchive == NULL) {
                        drfs_close_archive(pNextArchive);
                        return drfs_does_not_exist;
                    }

                    *ppArchiveOut = pOwnerArchive;
                    return drfs_success;
                }
                else
                {
                    // It's not an archive segment. We need to search.
                    if (pParentArchive->callbacks.begin_iteration == NULL || pParentArchive->callbacks.next_iteration == NULL || pParentArchive->callbacks.end_iteration == NULL) {
                        return drfs_no_backend;
                    }

                    drfs_handle iterator = pParentArchive->callbacks.begin_iteration(pParentArchive->internalArchiveHandle, runningPathBase);
                    if (iterator == NULL) {
                        return drfs_no_backend;
                    }

                    while (pParentArchive->callbacks.next_iteration(pParentArchive->internalArchiveHandle, iterator, &fi))
                    {
                        if ((fi.attributes & DRFS_FILE_ATTRIBUTE_DIRECTORY) == 0)
                        {
                            // It's a file which means it could be an archive. Note that fi.absolutePath is actually relative to the parent archive.
                            drfs_open_non_native_archive_from_path(pParentArchive, fi.absolutePath, accessMode, &pNextArchive);
                            if (pNextArchive != NULL)
                            {
                                // It's an archive, so check it.
                                drfs_archive* pOwnerArchive;
                                drfs_open_owner_archive_recursively_from_relative_path(pNextArchive, "", pathseg.path + pathseg.segment.offset, accessMode, relativePathOut, relativePathOutSize, &pOwnerArchive);
                                if (pOwnerArchive != NULL) {
                                    pParentArchive->callbacks.end_iteration(pParentArchive->internalArchiveHandle, iterator);
                                    *ppArchiveOut = pOwnerArchive;
                                    return drfs_success;
                                } else {
                                    drfs_close_archive(pNextArchive);
                                }
                            }
                        }
                    }

                    pParentArchive->callbacks.end_iteration(pParentArchive->internalArchiveHandle, iterator);
                }

            } while (drfs_drpath_next(&pathseg));
        }
    }

    return drfs_does_not_exist;
}

// Opens the archive that owns the file at the given path that's relative to the given base path. This supports non-verbose paths and will search
// the file system for the archive.
static drfs_result drfs_open_owner_archive_from_relative_path(drfs_context* pContext, const char* absoluteBasePath, const char* relativePath, unsigned int accessMode, char* relativePathOut, size_t relativePathOutSize, drfs_archive** ppArchiveOut)
{
    assert(pContext != NULL);
    assert(absoluteBasePath != NULL);
    assert(relativePath != NULL);
    assert(drfs_drpath_is_absolute(absoluteBasePath));
    assert(ppArchiveOut != NULL);

    *ppArchiveOut = NULL;

    char adjustedRelativePath[DRFS_MAX_PATH];
    char relativeBasePath[DRFS_MAX_PATH];
    relativeBasePath[0] = '\0';

    // The base path should be absolute and verbose. It does not need to be an actual directory.
    drfs_archive* pBaseArchive = NULL;
    if (drfs_is_native_directory(absoluteBasePath))
    {
        drfs_result result = drfs_open_native_archive(pContext, absoluteBasePath, accessMode, &pBaseArchive);
        if (result != drfs_success) {
            return result;
        }

        if (drfs__strcpy_s(adjustedRelativePath, sizeof(adjustedRelativePath), relativePath) != 0) {
            drfs_close_archive(pBaseArchive);
            return drfs_path_too_long;
        }
    }
    else
    {
        drfs_result result = drfs_open_owner_archive(pContext, absoluteBasePath, accessMode, relativeBasePath, sizeof(relativeBasePath), &pBaseArchive);
        if (result != drfs_success) {
            return result;
        }

        if (!drfs_drpath_copy_and_append(adjustedRelativePath, sizeof(adjustedRelativePath), relativeBasePath, relativePath)) {
            drfs_close_archive(pBaseArchive);
            return drfs_path_too_long;
        }
    }


    // We couldn't open the archive file from here, so we'll need to search for the owner archive recursively.
    drfs_archive* pOwnerArchive;
    drfs_result result = drfs_open_owner_archive_recursively_from_relative_path(pBaseArchive, relativeBasePath, adjustedRelativePath, accessMode, relativePathOut, relativePathOutSize, &pOwnerArchive);
    if (pOwnerArchive == NULL) {
        return result;
    }

    drfs_recursively_claim_ownership_or_parent_archive(pOwnerArchive);

    *ppArchiveOut = pOwnerArchive;
    return drfs_success;
}

// Opens an archive from a path that's relative to the given base path. This supports non-verbose paths and will search
// the file system for the archive.
static drfs_result drfs_open_archive_from_relative_path(drfs_context* pContext, const char* absoluteBasePath, const char* relativePath, unsigned int accessMode, drfs_archive** ppArchiveOut)
{
    assert(pContext != NULL);
    assert(absoluteBasePath != NULL);
    assert(relativePath != NULL);
    assert(drfs_drpath_is_absolute(absoluteBasePath));
    assert(ppArchiveOut != NULL);

    *ppArchiveOut = NULL;

    char adjustedRelativePath[DRFS_MAX_PATH];
    char relativeBasePath[DRFS_MAX_PATH];
    relativeBasePath[0] = '\0';

    // The base path should be absolute and verbose. It does not need to be an actual directory.
    drfs_archive* pBaseArchive = NULL;
    if (drfs_is_native_directory(absoluteBasePath))
    {
        drfs_result result = drfs_open_native_archive(pContext, absoluteBasePath, accessMode, &pBaseArchive);
        if (result != drfs_success) {
            return result;
        }

        if (drfs__strcpy_s(adjustedRelativePath, sizeof(adjustedRelativePath), relativePath) != 0) {
            drfs_close_archive(pBaseArchive);
            return drfs_path_too_long;
        }
    }
    else
    {
        drfs_result result = drfs_open_owner_archive(pContext, absoluteBasePath, accessMode, relativeBasePath, sizeof(relativeBasePath), &pBaseArchive);
        if (result != drfs_success) {
            return result;
        }

        if (!drfs_drpath_copy_and_append(adjustedRelativePath, sizeof(adjustedRelativePath), relativeBasePath, relativePath)) {
            drfs_close_archive(pBaseArchive);
            return drfs_path_too_long;
        }
    }


    // We can try opening the file directly from the base path. If this doesn't work, we can try recursively opening the owner archive
    // an open from there.
    drfs_archive* pArchive;
    drfs_result result = drfs_open_non_native_archive_from_path(pBaseArchive, adjustedRelativePath, accessMode, &pArchive);
    if (pArchive == NULL)
    {
        // We couldn't open the archive file from here, so we'll need to search for the owner archive recursively.
        char archiveRelativePath[DRFS_MAX_PATH];
        drfs_archive* pOwnerArchive;
        result = drfs_open_owner_archive_recursively_from_relative_path(pBaseArchive, relativeBasePath, adjustedRelativePath, accessMode, archiveRelativePath, sizeof(archiveRelativePath), &pOwnerArchive);
        if (pOwnerArchive != NULL)
        {
            drfs_recursively_claim_ownership_or_parent_archive(pOwnerArchive);

            result = drfs_open_non_native_archive_from_path(pOwnerArchive, archiveRelativePath, accessMode, &pArchive);
            if (pArchive == NULL) {
                drfs_close_archive(pOwnerArchive);
                return result;
            }
        }
    }

    if (pArchive == NULL) {
        drfs_close_archive(pBaseArchive);
        return drfs_does_not_exist;
    }

    *ppArchiveOut = pArchive;
    return drfs_success;
}

//// Back-End Registration ////

#ifndef DR_FS_NO_ZIP
// Registers the archive callbacks which enables support for Zip files.
static void drfs_register_zip_backend(drfs_context* pContext);
#endif

#ifndef DR_FS_NO_PAK
// Registers the archive callbacks which enables support for Quake 2 pak files.
static void drfs_register_pak_backend(drfs_context* pContext);
#endif

#ifndef DR_FS_NO_MTL
// Registers the archive callbacks which enables support for Quake 2 pak files.
static void drfs_register_mtl_backend(drfs_context* pContext);
#endif



//// Public API Implementation ////

drfs_result drfs_init(drfs_context* pContext)
{
    if (pContext == NULL) return drfs_invalid_args;
    memset(pContext, 0, sizeof(*pContext));

    if (!drfs_callbacklist_init(&pContext->archiveCallbacks) || !drfs_basedirs_init(&pContext->baseDirectories)) {
        return drfs_unknown_error;
    }

    pContext->isWriteGuardEnabled = 0;

#ifndef DR_FS_NO_ZIP
    drfs_register_zip_backend(pContext);
#endif

#ifndef DR_FS_NO_PAK
    drfs_register_pak_backend(pContext);
#endif

#ifndef DR_FS_NO_MTL
    drfs_register_mtl_backend(pContext);
#endif

    return drfs_success;
}

drfs_result drfs_uninit(drfs_context* pContext)
{
    if (pContext == NULL) return drfs_invalid_args;

    drfs_basedirs_uninit(&pContext->baseDirectories);
    drfs_callbacklist_uninit(&pContext->archiveCallbacks);
    return drfs_success;
}


drfs_context* drfs_create_context()
{
    drfs_context* pContext = (drfs_context*)malloc(sizeof(*pContext));
    if (pContext == NULL) {
        return NULL;
    }

    if (drfs_init(pContext) != drfs_success) {
        free(pContext);
        return NULL;
    }

    return pContext;
}

void drfs_delete_context(drfs_context* pContext)
{
    if (pContext == NULL) {
        return;
    }

    drfs_uninit(pContext);
    free(pContext);
}


void drfs_register_archive_backend(drfs_context* pContext, drfs_archive_callbacks callbacks)
{
    if (pContext == NULL) {
        return;
    }

    drfs_callbacklist_pushback(&pContext->archiveCallbacks, callbacks);
}


void drfs_insert_base_directory(drfs_context* pContext, const char* absolutePath, unsigned int index)
{
    if (pContext == NULL) {
        return;
    }

    drfs_basedirs_insert(&pContext->baseDirectories, absolutePath, index);
}

void drfs_add_base_directory(drfs_context* pContext, const char* absolutePath)
{
    if (pContext == NULL) {
        return;
    }

    drfs_insert_base_directory(pContext, absolutePath, drfs_get_base_directory_count(pContext));
}

void drfs_remove_base_directory(drfs_context* pContext, const char* absolutePath)
{
    if (pContext == NULL) {
        return;
    }

    for (unsigned int iPath = 0; iPath < pContext->baseDirectories.count; /*DO NOTHING*/) {
        if (drfs_drpath_equal(pContext->baseDirectories.pBuffer[iPath].absolutePath, absolutePath)) {
            drfs_basedirs_remove(&pContext->baseDirectories, iPath);
        } else {
            ++iPath;
        }
    }
}

void drfs_remove_base_directory_by_index(drfs_context* pContext, unsigned int index)
{
    if (pContext == NULL) {
        return;
    }

    drfs_basedirs_remove(&pContext->baseDirectories, index);
}

void drfs_remove_all_base_directories(drfs_context* pContext)
{
    if (pContext == NULL) {
        return;
    }

    drfs_basedirs_clear(&pContext->baseDirectories);
}

unsigned int drfs_get_base_directory_count(drfs_context* pContext)
{
    if (pContext == NULL) {
        return 0;
    }

    return pContext->baseDirectories.count;
}

const char* drfs_get_base_directory_by_index(drfs_context* pContext, unsigned int index)
{
    if (pContext == NULL || index >= pContext->baseDirectories.count) {
        return NULL;
    }

    return pContext->baseDirectories.pBuffer[index].absolutePath;
}


void drfs_set_base_write_directory(drfs_context* pContext, const char* absolutePath)
{
    if (pContext == NULL) {
        return;
    }

    if (absolutePath == NULL) {
        memset(pContext->writeBaseDirectory, 0, sizeof(pContext->writeBaseDirectory));
    } else {
        drfs__strcpy_s(pContext->writeBaseDirectory, sizeof(pContext->writeBaseDirectory), absolutePath);
    }
}

dr_bool32 drfs_get_base_write_directory(drfs_context* pContext, char* absolutePathOut, unsigned int absolutePathOutSize)
{
    if (pContext == NULL || absolutePathOut == NULL || absolutePathOutSize == 0) {
        return DR_FALSE;
    }

    return drfs__strcpy_s(absolutePathOut, absolutePathOutSize, pContext->writeBaseDirectory) == 0;
}

void drfs_enable_write_directory_guard(drfs_context* pContext)
{
    if (pContext == NULL) {
        return;
    }

    pContext->isWriteGuardEnabled = DR_TRUE;
}

void drfs_disable_write_directory_guard(drfs_context* pContext)
{
    if (pContext == NULL) {
        return;
    }

    pContext->isWriteGuardEnabled = DR_FALSE;
}

dr_bool32 drfs_is_write_directory_guard_enabled(drfs_context* pContext)
{
    if (pContext == NULL) {
        return DR_FALSE;
    }

    return pContext->isWriteGuardEnabled;
}




drfs_result drfs_open_archive(drfs_context* pContext, const char* absoluteOrRelativePath, unsigned int accessMode, drfs_archive** ppArchiveOut)
{
    if (ppArchiveOut == NULL) {
        return drfs_invalid_args;
    }

    *ppArchiveOut = NULL;

    if (pContext == NULL || absoluteOrRelativePath == NULL) {
        return drfs_invalid_args;
    }

    if (drfs_drpath_is_absolute(absoluteOrRelativePath))
    {
        if (drfs_is_native_directory(absoluteOrRelativePath))
        {
            // It's a native directory.
            return drfs_open_native_archive(pContext, absoluteOrRelativePath, accessMode, ppArchiveOut);
        }
        else
        {
            // It's not a native directory. We can just use drfs_open_owner_archive() in this case.
            char relativePath[DRFS_MAX_PATH];
            drfs_archive* pOwnerArchive;
            drfs_result result = drfs_open_owner_archive(pContext, absoluteOrRelativePath, accessMode, relativePath, sizeof(relativePath), &pOwnerArchive);
            if (result != drfs_success) {
                return result;
            }

            drfs_archive* pArchive;
            result = drfs_open_non_native_archive_from_path(pOwnerArchive, relativePath, accessMode, &pArchive);
            if (result != drfs_success) {
                drfs_close_archive(pOwnerArchive);
                return result;
            }

            drfs_recursively_claim_ownership_or_parent_archive(pArchive);

            *ppArchiveOut = pArchive;
            return drfs_success;
        }
    }
    else
    {
        // The input path is not absolute. We need to check each base directory.

        for (unsigned int iBaseDir = 0; iBaseDir < drfs_get_base_directory_count(pContext); ++iBaseDir)
        {
            drfs_archive* pArchive;
            drfs_result result = drfs_open_archive_from_relative_path(pContext, drfs_get_base_directory_by_index(pContext, iBaseDir), absoluteOrRelativePath, accessMode, &pArchive);
            if (result == drfs_success && pArchive != NULL) {
                drfs_recursively_claim_ownership_or_parent_archive(pArchive);

                *ppArchiveOut = pArchive;
                return drfs_success;
            }
        }
    }

    return drfs_does_not_exist;
}

drfs_result drfs_open_owner_archive(drfs_context* pContext, const char* absoluteOrRelativePath, unsigned int accessMode, char* relativePathOut, size_t relativePathOutSize, drfs_archive** ppArchiveOut)
{
    if (ppArchiveOut == NULL) {
        return drfs_invalid_args;
    }

    *ppArchiveOut = NULL;

    if (pContext == NULL || absoluteOrRelativePath == NULL) {
        return drfs_invalid_args;
    }

    if (drfs_drpath_is_absolute(absoluteOrRelativePath))
    {
        if (drfs_is_native_file(absoluteOrRelativePath) || drfs_is_native_directory(absoluteOrRelativePath))
        {
            // It's a native file. The owner archive is the directory it's directly sitting in.
            char dirAbsolutePath[DRFS_MAX_PATH];
            drfs_drpath_copy_base_path(absoluteOrRelativePath, dirAbsolutePath, sizeof(dirAbsolutePath));

            drfs_archive* pArchive;
            drfs_result result = drfs_open_archive(pContext, dirAbsolutePath, accessMode, &pArchive);
            if (result != drfs_success) {
                return result;
            }

            if (relativePathOut) {
                if (drfs__strcpy_s(relativePathOut, relativePathOutSize, drfs_drpath_file_name(absoluteOrRelativePath)) != 0) {
                    drfs_close_archive(pArchive);
                    return drfs_path_too_long;
                }
            }

            *ppArchiveOut = pArchive;
            return drfs_success;
        }
        else
        {
            // It's not a native file or directory.
            drfs_archive* pArchive;
            drfs_result result = drfs_open_owner_archive_from_absolute_path(pContext, absoluteOrRelativePath, accessMode, relativePathOut, relativePathOutSize, &pArchive);
            if (pArchive == NULL) {
                return result;
            }

            drfs_recursively_claim_ownership_or_parent_archive(pArchive);

            *ppArchiveOut = pArchive;
            return drfs_success;
        }
    }
    else
    {
        // The input path is not absolute. We need to loop through each base directory.

        for (unsigned int iBaseDir = 0; iBaseDir < drfs_get_base_directory_count(pContext); ++iBaseDir)
        {
            drfs_archive* pArchive;
            drfs_result result = drfs_open_owner_archive_from_relative_path(pContext, drfs_get_base_directory_by_index(pContext, iBaseDir), absoluteOrRelativePath, accessMode, relativePathOut, relativePathOutSize, &pArchive);
            if (result == drfs_success && pArchive != NULL) {
                drfs_recursively_claim_ownership_or_parent_archive(pArchive);

                *ppArchiveOut = pArchive;
                return drfs_success;
            }
        }
    }

    return drfs_does_not_exist;
}

void drfs_close_archive(drfs_archive* pArchive)
{
    if (pArchive == NULL) {
        return;
    }

    // The internal handle needs to be closed.
    if (pArchive->callbacks.close_archive) {
        pArchive->callbacks.close_archive(pArchive->internalArchiveHandle);
    }

    drfs_close(pArchive->pFile);


    if ((pArchive->flags & DR_FS_OWNS_PARENT_ARCHIVE) != 0) {
        drfs_close_archive(pArchive->pParentArchive);
    }

    free(pArchive);
}

drfs_result drfs_open_file_from_archive(drfs_archive* pArchive, const char* relativePath, unsigned int accessMode, drfs_file** ppFileOut)
{
    if (ppFileOut == NULL) {
        return drfs_invalid_args;
    }

    *ppFileOut = NULL;

    if (pArchive == NULL || relativePath == NULL || pArchive->callbacks.open_file == NULL) {
        return drfs_invalid_args;
    }

    drfs_handle internalFileHandle;
    drfs_result result = pArchive->callbacks.open_file(pArchive->internalArchiveHandle, relativePath, accessMode, &internalFileHandle);
    if (result != drfs_success) {
        return result;
    }

    // At this point the file is opened and we can create the file object.
    drfs_file* pFile = (drfs_file*)malloc(sizeof(*pFile));
    if (pFile == NULL) {
        pArchive->callbacks.close_file(pArchive->internalArchiveHandle, internalFileHandle);
        return drfs_out_of_memory;
    }

    pFile->pArchive           = pArchive;
    pFile->internalFileHandle = internalFileHandle;
    pFile->flags              = 0;

    // The lock.
#ifdef _WIN32
#ifdef DR_FS_WIN32_USE_EVENT_MUTEX
    pFile->lock = CreateEvent(NULL, FALSE, TRUE, NULL);
#else
    InitializeCriticalSection(&pFile->lock);
#endif
#else
    if (pthread_mutex_init(&pFile->lock, NULL) != 0) {
        drfs_close(pFile);
        return drfs_unknown_error;
    }
#endif

    *ppFileOut = pFile;
    return drfs_success;
}


drfs_result drfs_open(drfs_context* pContext, const char* absoluteOrRelativePath, unsigned int accessMode, drfs_file** ppFile)
{
    if (ppFile == NULL) {
        return drfs_invalid_args;
    }

    // Always set the output file to null at the start for safety.
    *ppFile = NULL;

    if (pContext == NULL || absoluteOrRelativePath == NULL) {
        return drfs_invalid_args;
    }

    char absolutePathForWriteMode[DRFS_MAX_PATH];
    if ((accessMode & DRFS_WRITE) != 0) {
        if (drfs_validate_write_path(pContext, absoluteOrRelativePath, absolutePathForWriteMode, sizeof(absolutePathForWriteMode))) {
            absoluteOrRelativePath = absolutePathForWriteMode;
        } else {
            return drfs_not_in_write_directory;
        }
    }

    char relativePath[DRFS_MAX_PATH];
    drfs_archive* pArchive;
    drfs_result result = drfs_open_owner_archive(pContext, absoluteOrRelativePath, drfs_archive_access_mode(accessMode), relativePath, sizeof(relativePath), &pArchive);
    if (result != drfs_success) {
        return result;
    }

    drfs_file* pFile;
    result = drfs_open_file_from_archive(pArchive, relativePath, accessMode, &pFile);
    if (result != drfs_success) {
        drfs_close_archive(pArchive);
        return result;
    }

    // When using this API, we want to claim ownership of the archive so that it's closed when we close this file.
    pFile->flags |= DR_FS_OWNS_PARENT_ARCHIVE;

    *ppFile = pFile;
    return drfs_success;
}

void drfs_close(drfs_file* pFile)
{
    if (!drfs_lock(pFile)) {
        return;
    }

    if (pFile->pArchive != NULL && pFile->pArchive->callbacks.close_file) {
        pFile->pArchive->callbacks.close_file(pFile->pArchive->internalArchiveHandle, pFile->internalFileHandle);
        pFile->internalFileHandle = NULL;
    }

    if ((pFile->flags & DR_FS_OWNS_PARENT_ARCHIVE) != 0) {
        drfs_close_archive(pFile->pArchive);
        pFile->pArchive = NULL;
    }

    drfs_unlock(pFile);


    // The lock.
#ifdef _WIN32
#ifdef DR_FS_WIN32_USE_EVENT_MUTEX
    CloseHandle(pFile->lock);
#else
    DeleteCriticalSection(&pFile->lock);
#endif
#else
    pthread_mutex_destroy(&pFile->lock);
#endif

    free(pFile);
}

drfs_result drfs_read_nolock(drfs_file* pFile, void* pDataOut, size_t bytesToRead, size_t* pBytesReadOut)
{
    if (pFile == NULL || pDataOut == NULL || pFile->pArchive == NULL || pFile->pArchive->callbacks.read_file == NULL) {
        return drfs_invalid_args;
    }

    return pFile->pArchive->callbacks.read_file(pFile->pArchive->internalArchiveHandle, pFile->internalFileHandle, pDataOut, bytesToRead, pBytesReadOut);
}

drfs_result drfs_read(drfs_file* pFile, void* pDataOut, size_t bytesToRead, size_t* pBytesReadOut)
{
    if (!drfs_lock(pFile)) {
        return drfs_unknown_error;
    }

    drfs_result result = drfs_read_nolock(pFile, pDataOut, bytesToRead, pBytesReadOut);

    drfs_unlock(pFile);
    return result;
}

drfs_result drfs_write_nolock(drfs_file* pFile, const void* pData, size_t bytesToWrite, size_t* pBytesWrittenOut)
{
    if (pFile == NULL || pData == NULL || pFile->pArchive == NULL || pFile->pArchive->callbacks.write_file == NULL) {
        return drfs_invalid_args;
    }

    return pFile->pArchive->callbacks.write_file(pFile->pArchive->internalArchiveHandle, pFile->internalFileHandle, pData, bytesToWrite, pBytesWrittenOut);
}

drfs_result drfs_write(drfs_file* pFile, const void* pData, size_t bytesToWrite, size_t* pBytesWrittenOut)
{
    if (!drfs_lock(pFile)) {
        return drfs_unknown_error;
    }

    drfs_result result = drfs_write_nolock(pFile, pData, bytesToWrite, pBytesWrittenOut);

    drfs_unlock(pFile);
    return result;
}

drfs_result drfs_seek_nolock(drfs_file* pFile, dr_int64 bytesToSeek, drfs_seek_origin origin)
{
    if (pFile == NULL || pFile->pArchive == NULL || pFile->pArchive->callbacks.seek_file == NULL) {
        return drfs_invalid_args;
    }

    return pFile->pArchive->callbacks.seek_file(pFile->pArchive->internalArchiveHandle, pFile->internalFileHandle, bytesToSeek, origin);
}

drfs_result drfs_seek(drfs_file* pFile, dr_int64 bytesToSeek, drfs_seek_origin origin)
{
    if (!drfs_lock(pFile)) {
        return drfs_unknown_error;
    }

    drfs_result result = drfs_seek_nolock(pFile, bytesToSeek, origin);

    drfs_unlock(pFile);
    return result;
}

dr_uint64 drfs_tell_nolock(drfs_file* pFile)
{
    if (pFile == NULL || pFile->pArchive == NULL || pFile->pArchive->callbacks.tell_file == NULL) {
        return DR_FALSE;
    }

    return pFile->pArchive->callbacks.tell_file(pFile->pArchive->internalArchiveHandle, pFile->internalFileHandle);
}

dr_uint64 drfs_tell(drfs_file* pFile)
{
    if (!drfs_lock(pFile)) {
        return 0;
    }

    dr_uint64 result = drfs_tell_nolock(pFile);

    drfs_unlock(pFile);
    return result;
}

dr_uint64 drfs_size_nolock(drfs_file* pFile)
{
    if (pFile == NULL || pFile->pArchive == NULL || pFile->pArchive->callbacks.file_size == NULL) {
        return 0;
    }

    return pFile->pArchive->callbacks.file_size(pFile->pArchive->internalArchiveHandle, pFile->internalFileHandle);
}

dr_uint64 drfs_size(drfs_file* pFile)
{
    if (!drfs_lock(pFile)) {
        return DR_FALSE;
    }

    dr_uint64 result = drfs_size_nolock(pFile);

    drfs_unlock(pFile);
    return result;
}

void drfs_flush(drfs_file* pFile)
{
    if (pFile == NULL || pFile->pArchive == NULL || pFile->pArchive->callbacks.flush_file == NULL) {
        return;
    }

    pFile->pArchive->callbacks.flush_file(pFile->pArchive->internalArchiveHandle, pFile->internalFileHandle);
}


dr_bool32 drfs_lock(drfs_file* pFile)
{
    if (pFile == NULL || pFile->internalFileHandle == NULL) {
        return DR_FALSE;
    }

#ifdef _WIN32
#ifdef DR_FS_WIN32_USE_EVENT_MUTEX
    WaitForSingleObject(pFile->lock, INFINITE);
#else
    EnterCriticalSection(&pFile->lock);
#endif
#else
    pthread_mutex_lock(&pFile->lock);
#endif

    return DR_TRUE;
}

void drfs_unlock(drfs_file* pFile)
{
    if (pFile == NULL || pFile->internalFileHandle == NULL) {
        return;
    }

#ifdef _WIN32
#ifdef DR_FS_WIN32_USE_EVENT_MUTEX
    SetEvent(pFile->lock);
#else
    LeaveCriticalSection(&pFile->lock);
#endif
#else
    pthread_mutex_unlock(&pFile->lock);
#endif
}


drfs_result drfs_get_file_info(drfs_context* pContext, const char* absoluteOrRelativePath, drfs_file_info* fi)
{
    if (pContext == NULL || absoluteOrRelativePath == NULL) {
        return drfs_invalid_args;
    }

    char relativePath[DRFS_MAX_PATH];
    drfs_archive* pOwnerArchive;
    drfs_result result = drfs_open_owner_archive(pContext, absoluteOrRelativePath, DRFS_READ, relativePath, sizeof(relativePath), &pOwnerArchive);
    if (result != drfs_success) {
        return result;
    }

    result = drfs_no_backend;
    if (pOwnerArchive->callbacks.get_file_info) {
        result = pOwnerArchive->callbacks.get_file_info(pOwnerArchive->internalArchiveHandle, relativePath, fi);
    }

    if (result == drfs_success && fi != NULL) {
        drfs_drpath_copy_and_append(fi->absolutePath, sizeof(fi->absolutePath), pOwnerArchive->absolutePath, relativePath);
    }

    drfs_close_archive(pOwnerArchive);
    return result;
}


dr_bool32 drfs_begin(drfs_context* pContext, const char* absoluteOrRelativePath, drfs_iterator* pIteratorOut)
{
    if (pIteratorOut == NULL) return DR_FALSE;
    memset(pIteratorOut, 0, sizeof(*pIteratorOut));

    if (pContext == NULL || absoluteOrRelativePath == NULL) {
        return DR_FALSE;
    }

    // We need to open the archive that the given folder is in. The path could, however, point to an actual archive which is allowed
    // in which case we just iterate over the root directory within that archive. What we do is first try using the path as an actual
    // archive. If it fails we assume the path is to a folder within an archive (such as a zip file) in which case we just try opening
    // the owner archive. If both fail, we return DR_FALSE.

    char relativePath[DRFS_MAX_PATH];
    drfs_result result = drfs_open_archive(pContext, absoluteOrRelativePath, DRFS_READ, &pIteratorOut->pArchive);
    if (result == drfs_success) {
        relativePath[0] = '\0';
    } else {
        result = drfs_open_owner_archive(pContext, absoluteOrRelativePath, DRFS_READ, relativePath, sizeof(relativePath), &pIteratorOut->pArchive);
        if (result != drfs_success) {
            return DR_FALSE;
        }
    }

    assert(pIteratorOut->pArchive != NULL);


    if (pIteratorOut->pArchive->callbacks.begin_iteration) {
        pIteratorOut->internalIteratorHandle = pIteratorOut->pArchive->callbacks.begin_iteration(pIteratorOut->pArchive->internalArchiveHandle, relativePath);
    }

    if (pIteratorOut->internalIteratorHandle == NULL) {
        drfs_close_archive(pIteratorOut->pArchive);
        pIteratorOut->pArchive = NULL;
        return DR_FALSE;
    }


    dr_bool32 foundFirst = drfs_next(pContext, pIteratorOut);
    if (!foundFirst) {
        drfs_end(pContext, pIteratorOut);
    }

    return foundFirst;
}

dr_bool32 drfs_next(drfs_context* pContext, drfs_iterator* pIterator)
{
    if (pIterator == NULL) {
        return DR_FALSE;
    }

    memset(&pIterator->info, 0, sizeof(pIterator->info));

    if (pContext == NULL || pIterator->pArchive == NULL) {
        return DR_FALSE;
    }

    dr_bool32 result = DR_FALSE;
    if (pIterator->pArchive->callbacks.next_iteration) {
        result = pIterator->pArchive->callbacks.next_iteration(pIterator->pArchive->internalArchiveHandle, pIterator->internalIteratorHandle, &pIterator->info);
    }

    // At this point the pIterator->info.absolutePath is actually referring to a relative path. We need to convert it to an absolute path.
    if (result) {
        char relativePath[DRFS_MAX_PATH];
        drfs__strcpy_s(relativePath, sizeof(relativePath), pIterator->info.absolutePath);
        drfs_drpath_copy_and_append(pIterator->info.absolutePath, sizeof(pIterator->info.absolutePath), pIterator->pArchive->absolutePath, relativePath);
    }

    return result;
}

void drfs_end(drfs_context* pContext, drfs_iterator* pIterator)
{
    if (pContext == NULL || pIterator == NULL) {
        return;
    }

    if (pIterator->pArchive != NULL && pIterator->pArchive->callbacks.end_iteration) {
        pIterator->pArchive->callbacks.end_iteration(pIterator->pArchive->internalArchiveHandle, pIterator->internalIteratorHandle);
    }

    drfs_close_archive(pIterator->pArchive);
    memset(pIterator, 0, sizeof(*pIterator));
}

drfs_result drfs_delete_file(drfs_context* pContext, const char* path)
{
    if (pContext == NULL || path == NULL) {
        return drfs_invalid_args;
    }

    char absolutePath[DRFS_MAX_PATH];
    if (!drfs_validate_write_path(pContext, path, absolutePath, sizeof(absolutePath))) {
        return drfs_not_in_write_directory;
    }


    char relativePath[DRFS_MAX_PATH];
    drfs_archive* pArchive;
    drfs_result result = drfs_open_owner_archive(pContext, absolutePath, drfs_archive_access_mode(DRFS_READ | DRFS_WRITE), relativePath, sizeof(relativePath), &pArchive);
    if (result != drfs_success) {
        return result;
    }

    result = drfs_no_backend;
    if (pArchive->callbacks.delete_file) {
        result = pArchive->callbacks.delete_file(pArchive->internalArchiveHandle, relativePath);
    }

    drfs_close_archive(pArchive);
    return result;
}

drfs_result drfs_create_directory(drfs_context* pContext, const char* path)
{
    if (pContext == NULL || path == NULL) {
        return drfs_invalid_args;
    }

    char absolutePath[DRFS_MAX_PATH];
    if (!drfs_validate_write_path(pContext, path, absolutePath, sizeof(absolutePath))) {
        return drfs_not_in_write_directory;
    }

    char relativePath[DRFS_MAX_PATH];
    drfs_archive* pArchive;
    drfs_result result = drfs_open_owner_archive(pContext, absolutePath, drfs_archive_access_mode(DRFS_READ | DRFS_WRITE), relativePath, sizeof(relativePath), &pArchive);
    if (result != drfs_success) {
        return result;
    }

    result = drfs_no_backend;
    if (pArchive->callbacks.create_directory) {
        result = pArchive->callbacks.create_directory(pArchive->internalArchiveHandle, relativePath);
    }

    drfs_close_archive(pArchive);
    return result;
}

drfs_result drfs_move_file(drfs_context* pContext, const char* pathOld, const char* pathNew)
{
    // Renaming/moving is not supported across different archives.

    if (pContext == NULL || pathOld == NULL || pathNew == NULL) {
        return drfs_invalid_args;
    }


    char absolutePathOld[DRFS_MAX_PATH];
    if (drfs_validate_write_path(pContext, pathOld, absolutePathOld, sizeof(absolutePathOld))) {
        pathOld = absolutePathOld;
    } else {
        return drfs_not_in_write_directory;
    }

    char absolutePathNew[DRFS_MAX_PATH];
    if (drfs_validate_write_path(pContext, pathNew, absolutePathNew, sizeof(absolutePathNew))) {
        pathNew = absolutePathNew;
    } else {
        return drfs_not_in_write_directory;
    }





    char relativePathOld[DRFS_MAX_PATH];
    drfs_archive* pArchiveOld;
    drfs_result result = drfs_open_owner_archive(pContext, pathOld, drfs_archive_access_mode(DRFS_READ | DRFS_WRITE), relativePathOld, sizeof(relativePathOld), &pArchiveOld);
    if (pArchiveOld != NULL)
    {
        char relativePathNew[DRFS_MAX_PATH];
        drfs_archive* pArchiveNew;
        result = drfs_open_owner_archive(pContext, pathNew, drfs_archive_access_mode(DRFS_READ | DRFS_WRITE), relativePathNew, sizeof(relativePathNew), &pArchiveNew);
        if (pArchiveNew != NULL)
        {
            dr_bool32 areBothArchivesNative = (pArchiveOld->callbacks.move_file == pArchiveNew->callbacks.move_file && pArchiveNew->callbacks.move_file == drfs_move_file__native);
            if (areBothArchivesNative)
            {
                result = drfs_move_native_file(absolutePathOld, absolutePathNew);
            }
            else
            {
                if (drfs_drpath_equal(pArchiveOld->absolutePath, pArchiveNew->absolutePath) && pArchiveOld->callbacks.move_file) {
                    result = pArchiveOld->callbacks.move_file(pArchiveOld, relativePathOld, relativePathNew);
                } else {
                    result = drfs_permission_denied; // Attempting to rename across different archives which is not supported.
                }
            }

            drfs_close_archive(pArchiveNew);

        }

        drfs_close_archive(pArchiveOld);
    }

    return result;
}

drfs_result drfs_copy_file(drfs_context* pContext, const char* srcPath, const char* dstPath, dr_bool32 failIfExists)
{
    if (pContext == NULL || srcPath == NULL || dstPath == NULL) {
        return drfs_invalid_args;
    }

    char dstPathAbsolute[DRFS_MAX_PATH];
    if (!drfs_validate_write_path(pContext, dstPath, dstPathAbsolute, sizeof(dstPathAbsolute))) {
        return drfs_not_in_write_directory;
    }


    // We want to open the archive of both the source and destination. If they are the same archive we'll do an intra-archive copy.
    char srcRelativePath[DRFS_MAX_PATH];
    drfs_archive* pSrcArchive;
    drfs_result result = drfs_open_owner_archive(pContext, srcPath, drfs_archive_access_mode(DRFS_READ), srcRelativePath, sizeof(srcRelativePath), &pSrcArchive);
    if (result != drfs_success) {
        return result;
    }

    char dstRelativePath[DRFS_MAX_PATH];
    drfs_archive* pDstArchive;
    result = drfs_open_owner_archive(pContext, dstPathAbsolute, drfs_archive_access_mode(DRFS_READ | DRFS_WRITE), dstRelativePath, sizeof(dstRelativePath), &pDstArchive);
    if (result != drfs_success) {
        drfs_close_archive(pSrcArchive);
        return result;
    }



    result = drfs_success;
    if (strcmp(pSrcArchive->absolutePath, pDstArchive->absolutePath) == 0 && pDstArchive->callbacks.copy_file)
    {
        // Intra-archive copy.
        result = pDstArchive->callbacks.copy_file(pDstArchive->internalArchiveHandle, srcRelativePath, dstRelativePath, failIfExists);
    }
    else
    {
        dr_bool32 areBothArchivesNative = (pSrcArchive->callbacks.copy_file == pDstArchive->callbacks.copy_file && pDstArchive->callbacks.copy_file == drfs_copy_file__native);
        if (areBothArchivesNative)
        {
            char srcPathAbsolute[DRFS_MAX_PATH];
            drfs_drpath_copy_and_append(srcPathAbsolute, sizeof(srcPathAbsolute), pSrcArchive->absolutePath, srcPath);

            result = drfs_copy_native_file(srcPathAbsolute, dstPathAbsolute, failIfExists);
        }
        else
        {
            // Inter-archive copy. This is a theoretically slower path because we do everything manually. Probably not much slower in practice, though.
            if (failIfExists && pDstArchive->callbacks.get_file_info(pDstArchive, dstRelativePath, NULL) == drfs_success)
            {
                result = drfs_already_exists;
            }
            else
            {
                drfs_file* pSrcFile;
                result = drfs_open_file_from_archive(pSrcArchive, srcRelativePath, DRFS_READ, &pSrcFile);
                if (result != drfs_success) {
                    return result;
                }

                drfs_file* pDstFile;
                result = drfs_open_file_from_archive(pDstArchive, dstRelativePath, DRFS_WRITE | DRFS_TRUNCATE, &pDstFile);
                if (result != drfs_success) {
                    drfs_close(pSrcFile);
                    return result;
                }

                assert(pSrcFile != NULL);
                assert(pDstFile != NULL);

                char chunk[4096];
                size_t bytesRead;
                while (drfs_read(pSrcFile, chunk, sizeof(chunk), &bytesRead) == drfs_success && bytesRead > 0)
                {
                    result = drfs_write(pDstFile, chunk, bytesRead, NULL);
                    if (result != drfs_success) {
                        break;
                    }
                }

                drfs_close(pSrcFile);
                drfs_close(pDstFile);
            }
        }
    }


    drfs_close_archive(pSrcArchive);
    drfs_close_archive(pDstArchive);

    return result;
}


dr_bool32 drfs_is_archive_path(drfs_context* pContext, const char* path)
{
    return drfs_find_backend_by_extension(pContext, drfs_drpath_extension(path), NULL);
}



///////////////////////////////////////////////////////////////////////////////
//
// High Level API
//
///////////////////////////////////////////////////////////////////////////////
void drfs_free(void* p)
{
    free(p);
}

drfs_result drfs_find_absolute_path(drfs_context* pContext, const char* relativePath, char* absolutePathOut, size_t absolutePathOutSize)
{
    if (absolutePathOut == NULL) return drfs_invalid_args;
    if (absolutePathOutSize > 0) absolutePathOut[0] = '\0';

    if (pContext == NULL || relativePath == NULL || absolutePathOutSize == 0) {
        return drfs_invalid_args;
    }

    drfs_file_info fi;
    if (drfs_get_file_info(pContext, relativePath, &fi) == drfs_success) {
        if (drfs__strcpy_s(absolutePathOut, absolutePathOutSize, fi.absolutePath) == 0) {
            return drfs_success;
        }
    }

    return drfs_does_not_exist;
}

drfs_result drfs_find_absolute_path_explicit_base(drfs_context* pContext, const char* relativePath, const char* highestPriorityBasePath, char* absolutePathOut, size_t absolutePathOutSize)
{
    if (absolutePathOut == NULL) return drfs_invalid_args;
    if (absolutePathOutSize > 0) absolutePathOut[0] = '\0';

    if (pContext == NULL || relativePath == NULL || highestPriorityBasePath == NULL || absolutePathOutSize == 0) {
        return drfs_invalid_args;
    }

    drfs_result result = drfs_unknown_error;
    drfs_insert_base_directory(pContext, highestPriorityBasePath, 0);
    {
        result = drfs_find_absolute_path(pContext, relativePath, absolutePathOut, absolutePathOutSize);
    }
    drfs_remove_base_directory_by_index(pContext, 0);

    return result;
}

dr_bool32 drfs_is_base_directory(drfs_context* pContext, const char* baseDir)
{
    if (pContext == NULL) {
        return DR_FALSE;
    }

    for (unsigned int i = 0; i < drfs_get_base_directory_count(pContext); ++i) {
        if (drfs_drpath_equal(drfs_get_base_directory_by_index(pContext, i), baseDir)) {
            return DR_TRUE;
        }
    }

    return DR_FALSE;
}

drfs_result drfs_write_string(drfs_file* pFile, const char* str)
{
    return drfs_write(pFile, str, (unsigned int)strlen(str), NULL);
}

drfs_result drfs_write_line(drfs_file* pFile, const char* str)
{
    drfs_result result = drfs_write_string(pFile, str);
    if (result != drfs_success) {
        return result;
    }

    return drfs_write_string(pFile, "\n");
}


void* drfs_open_and_read_binary_file(drfs_context* pContext, const char* absoluteOrRelativePath, size_t* pSizeInBytesOut)
{
    drfs_file* pFile;
    if (drfs_open(pContext, absoluteOrRelativePath, DRFS_READ, &pFile) != drfs_success) {
        return NULL;
    }

    dr_uint64 fileSize = drfs_size(pFile);
    if (fileSize > SIZE_MAX)
    {
        // File's too big.
        drfs_close(pFile);
        return NULL;
    }


    void* pData = malloc((size_t)fileSize);
    if (pData == NULL)
    {
        // Failed to allocate memory.
        drfs_close(pFile);
        return NULL;
    }


    if (drfs_read(pFile, pData, (size_t)fileSize, NULL) != drfs_success)
    {
        free(pData);
        if (pSizeInBytesOut != NULL) {
            *pSizeInBytesOut = 0;
        }

        drfs_close(pFile);
        return NULL;
    }


    if (pSizeInBytesOut != NULL) {
        *pSizeInBytesOut = (size_t)fileSize;
    }

    drfs_close(pFile);
    return pData;
}

char* drfs_open_and_read_text_file(drfs_context* pContext, const char* absoluteOrRelativePath, size_t* pSizeInBytesOut)
{
    drfs_file* pFile;
    if (drfs_open(pContext, absoluteOrRelativePath, DRFS_READ, &pFile) != drfs_success) {
        return NULL;
    }

    dr_uint64 fileSize = drfs_size(pFile);
    if (fileSize > SIZE_MAX)
    {
        // File's too big.
        drfs_close(pFile);
        return NULL;
    }


    void* pData = malloc((size_t)fileSize + 1);     // +1 for null terminator.
    if (pData == NULL)
    {
        // Failed to allocate memory.
        drfs_close(pFile);
        return NULL;
    }


    if (drfs_read(pFile, pData, (size_t)fileSize, NULL) != drfs_success)
    {
        free(pData);
        if (pSizeInBytesOut != NULL) {
            *pSizeInBytesOut = 0;
        }

        drfs_close(pFile);
        return NULL;
    }

    // Null terminator.
    ((char*)pData)[fileSize] = '\0';


    if (pSizeInBytesOut != NULL) {
        *pSizeInBytesOut = (size_t)fileSize;
    }

    drfs_close(pFile);
    return (char*)pData;
}

drfs_result drfs_open_and_write_binary_file(drfs_context* pContext, const char* absoluteOrRelativePath, const void* pData, size_t dataSize)
{
    drfs_result result = drfs_unknown_error;

    drfs_file* pFile;
    result = drfs_open(pContext, absoluteOrRelativePath, DRFS_WRITE | DRFS_TRUNCATE, &pFile);
    if (result != drfs_success) {
        return result;
    }

    result = drfs_write(pFile, pData, dataSize, NULL);

    drfs_close(pFile);
    return result;
}

drfs_result drfs_open_and_write_text_file(drfs_context* pContext, const char* absoluteOrRelativePath, const char* pTextData)
{
    return drfs_open_and_write_binary_file(pContext, absoluteOrRelativePath, pTextData, strlen(pTextData));
}


dr_bool32 drfs_exists(drfs_context* pContext, const char* absoluteOrRelativePath)
{
    drfs_file_info fi;
    return drfs_get_file_info(pContext, absoluteOrRelativePath, &fi) == drfs_success;
}

dr_bool32 drfs_is_existing_file(drfs_context* pContext, const char* absoluteOrRelativePath)
{
    drfs_file_info fi;
    if (drfs_get_file_info(pContext, absoluteOrRelativePath, &fi) == drfs_success)
    {
        return (fi.attributes & DRFS_FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    return DR_FALSE;
}

dr_bool32 drfs_is_existing_directory(drfs_context* pContext, const char* absoluteOrRelativePath)
{
    drfs_file_info fi;
    if (drfs_get_file_info(pContext, absoluteOrRelativePath, &fi) == drfs_success)
    {
        return (fi.attributes & DRFS_FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    return DR_FALSE;
}

drfs_result drfs_create_directory_recursive(drfs_context* pContext, const char* path)
{
    if (pContext == NULL || path == NULL) {
        return drfs_invalid_args;
    }

    // We just iterate over each segment and try creating each directory if it doesn't exist.
    char absolutePath[DRFS_MAX_PATH];
    if (drfs_validate_write_path(pContext, path, absolutePath, DRFS_MAX_PATH)) {
        path = absolutePath;
    } else {
        return drfs_not_in_write_directory;
    }


    char runningPath[DRFS_MAX_PATH];
    runningPath[0] = '\0';

    drfs_drpath_iterator iPathSeg;
    if (!drfs_drpath_first(absolutePath, &iPathSeg)) {
        return drfs_invalid_args;
    }

    // Never check the first segment because we can assume it always exists - it will always be the drive root.
    if (drfs_drpath_append_iterator(runningPath, sizeof(runningPath), iPathSeg))
    {
        // Loop over every directory until we find one that does not exist.
        while (drfs_drpath_next(&iPathSeg))
        {
            if (!drfs_drpath_append_iterator(runningPath, sizeof(runningPath), iPathSeg)) {
                return drfs_path_too_long;
            }

            if (!drfs_is_existing_directory(pContext, runningPath)) {
                drfs_result result = drfs_create_directory(pContext, runningPath);
                if (result != drfs_success) {
                    return result;
                }

                break;
            }
        }


        // At this point all we need to do is create the remaining directories - we can assert that the directory does not exist
        // rather than actually checking it which should be a bit more efficient.
        while (drfs_drpath_next(&iPathSeg))
        {
            if (!drfs_drpath_append_iterator(runningPath, sizeof(runningPath), iPathSeg)) {
                return drfs_path_too_long;
            }

            assert(!drfs_is_existing_directory(pContext, runningPath));

            drfs_result result = drfs_create_directory(pContext, runningPath);
            if (result != drfs_success) {
                return result;
            }
        }


        return drfs_success;
    }
    else
    {
        return drfs_invalid_args;
    }
}

dr_bool32 drfs_eof(drfs_file* pFile)
{
    return drfs_tell(pFile) == drfs_size(pFile);
}




///////////////////////////////////////////////////////////////////////////////
//
// ZIP
//
///////////////////////////////////////////////////////////////////////////////
#ifndef DR_FS_NO_ZIP

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4334)
#endif
#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-macros"
    #pragma GCC diagnostic ignored "-Wcast-align"
    #pragma GCC diagnostic ignored "-Wextra"

#if __GNUC__ >= 6 && !defined __clang__
    #pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif
#endif

#ifndef DRFS_MINIZ_HEADER_INCLUDED
#define DRFS_MINIZ_HEADER_INCLUDED

// If DRFS_MINIZ_NO_TIME is specified then the ZIP archive functions will not be able to get the current time, or
// get/set file times, and the C run-time funcs that get/set times won't be called.
// The current downside is the times written to your archives will be from 1979.
//#define DRFS_MINIZ_NO_TIME

// Define DRFS_MINIZ_NO_ARCHIVE_APIS to disable all ZIP archive API's.
//#define DRFS_MINIZ_NO_ARCHIVE_APIS

#if defined(__TINYC__) && (defined(__linux) || defined(__linux__))
  // TODO: Work around "error: include file 'sys\utime.h' when compiling with tcc on Linux
  #define DRFS_MINIZ_NO_TIME
#endif

#if !defined(DRFS_MINIZ_NO_TIME) && !defined(DRFS_MINIZ_NO_ARCHIVE_APIS)
  #include <time.h>
#endif

#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
// DRFS_MINIZ_X86_OR_X64_CPU is only used to help set the below macros.
#define DRFS_MINIZ_X86_OR_X64_CPU 1
#endif

#if (__BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__) || DRFS_MINIZ_X86_OR_X64_CPU
// Set DRFS_MINIZ_LITTLE_ENDIAN to 1 if the processor is little endian.
#define DRFS_MINIZ_LITTLE_ENDIAN 1
#endif

#if DRFS_MINIZ_X86_OR_X64_CPU
// Set DRFS_MINIZ_USE_UNALIGNED_LOADS_AND_STORES to 1 on CPU's that permit efficient integer loads and stores from unaligned addresses.
#define DRFS_MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1
#endif

#if defined(_M_X64) || defined(_WIN64) || defined(__MINGW64__) || defined(_LP64) || defined(__LP64__) || defined(__ia64__) || defined(__x86_64__)
// Set DRFS_MINIZ_HAS_64BIT_REGISTERS to 1 if operations on 64-bit integers are reasonably fast (and don't involve compiler generated calls to helper functions).
#define DRFS_MINIZ_HAS_64BIT_REGISTERS 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ------------------- zlib-style API Definitions.

// For more compatibility with zlib, miniz.c uses unsigned long for some parameters/struct members. Beware: drfs_mz_ulong can be either 32 or 64-bits!
typedef unsigned long drfs_mz_ulong;

// drfs_mz_free() internally uses the DRFS_MZ_FREE() macro (which by default calls free() unless you've modified the DRFS_MZ_MALLOC macro) to release a block allocated from the heap.
void drfs_mz_free(void *p);

#define DRFS_MZ_ADLER32_INIT (1)
// drfs_mz_adler32() returns the initial adler-32 value to use when called with ptr==NULL.
drfs_mz_ulong drfs_mz_adler32(drfs_mz_ulong adler, const unsigned char *ptr, size_t buf_len);

#define DRFS_MZ_CRC32_INIT (0)
// drfs_mz_crc32() returns the initial CRC-32 value to use when called with ptr==NULL.
drfs_mz_ulong drfs_mz_crc32(drfs_mz_ulong crc, const unsigned char *ptr, size_t buf_len);

// Compression strategies.
enum { DRFS_MZ_DEFAULT_STRATEGY = 0, DRFS_MZ_FILTERED = 1, DRFS_MZ_HUFFMAN_ONLY = 2, DRFS_MZ_RLE = 3, DRFS_MZ_FIXED = 4 };

// Method
#define DRFS_MZ_DEFLATED 8

// Heap allocation callbacks.
// Note that drfs_mz_alloc_func parameter types purpsosely differ from zlib's: items/size is size_t, not unsigned long.
typedef void *(*drfs_mz_alloc_func)(void *opaque, size_t items, size_t size);
typedef void (*drfs_mz_free_func)(void *opaque, void *address);
typedef void *(*drfs_mz_realloc_func)(void *opaque, void *address, size_t items, size_t size);

// ------------------- Types and macros

typedef unsigned char drfs_mz_uint8;
typedef signed short drfs_mz_int16;
typedef unsigned short drfs_drfs_mz_uint16;
typedef unsigned int drfs_drfs_mz_uint32;
typedef unsigned int drfs_mz_uint;
typedef long long drfs_mz_int64;
typedef unsigned long long drfs_mz_uint64;
typedef int drfs_mz_bool;

#define DRFS_MZ_FALSE (0)
#define DRFS_MZ_TRUE (1)

// An attempt to work around MSVC's spammy "warning C4127: conditional expression is constant" message.
#ifdef _MSC_VER
   #define DRFS_MZ_MACRO_END while (0, 0)
#else
   #define DRFS_MZ_MACRO_END while (0)
#endif

// ------------------- ZIP archive reading/writing

#ifndef DRFS_MINIZ_NO_ARCHIVE_APIS

enum
{
  DRFS_MZ_ZIP_MAX_IO_BUF_SIZE = 64*1024,
  DRFS_MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE = 260,
  DRFS_MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE = 256
};

typedef struct
{
  drfs_drfs_mz_uint32 m_file_index;
  drfs_drfs_mz_uint32 m_central_dir_ofs;
  drfs_drfs_mz_uint16 m_version_made_by;
  drfs_drfs_mz_uint16 m_version_needed;
  drfs_drfs_mz_uint16 m_bit_flag;
  drfs_drfs_mz_uint16 m_method;
#ifndef DRFS_MINIZ_NO_TIME
  time_t m_time;
#endif
  drfs_drfs_mz_uint32 m_crc32;
  drfs_mz_uint64 m_comp_size;
  drfs_mz_uint64 m_uncomp_size;
  drfs_drfs_mz_uint16 m_internal_attr;
  drfs_drfs_mz_uint32 m_external_attr;
  drfs_mz_uint64 m_local_header_ofs;
  drfs_drfs_mz_uint32 m_comment_size;
  char m_filename[DRFS_MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
  char m_comment[DRFS_MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE];
} drfs_drfs_mz_zip_archive_file_stat;

typedef size_t (*drfs_mz_file_read_func)(void *pOpaque, drfs_mz_uint64 file_ofs, void *pBuf, size_t n);
typedef size_t (*drfs_mz_file_write_func)(void *pOpaque, drfs_mz_uint64 file_ofs, const void *pBuf, size_t n);

struct drfs_mz_zip_internal_state_tag;
typedef struct drfs_mz_zip_internal_state_tag drfs_mz_zip_internal_state;

typedef enum
{
  DRFS_MZ_ZIP_MODE_INVALID = 0,
  DRFS_MZ_ZIP_MODE_READING = 1,
  DRFS_MZ_ZIP_MODE_WRITING = 2,
  DRFS_MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED = 3
} drfs_mz_zip_mode;

typedef struct drfs_mz_zip_archive_tag
{
  drfs_mz_uint64 m_archive_size;
  drfs_mz_uint64 m_central_directory_file_ofs;
  drfs_mz_uint m_total_files;
  drfs_mz_zip_mode m_zip_mode;

  drfs_mz_uint m_file_offset_alignment;

  drfs_mz_alloc_func m_pAlloc;
  drfs_mz_free_func m_pFree;
  drfs_mz_realloc_func m_pRealloc;
  void *m_pAlloc_opaque;

  drfs_mz_file_read_func m_pRead;
  drfs_mz_file_write_func m_pWrite;
  void *m_pIO_opaque;

  drfs_mz_zip_internal_state *m_pState;

} drfs_mz_zip_archive;

typedef enum
{
  DRFS_MZ_ZIP_FLAG_CASE_SENSITIVE                = 0x0100,
  DRFS_MZ_ZIP_FLAG_IGNORE_PATH                   = 0x0200,
  DRFS_MZ_ZIP_FLAG_COMPRESSED_DATA               = 0x0400,
  DRFS_MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY = 0x0800
} drfs_mz_zip_flags;

// ZIP archive reading

// Inits a ZIP archive reader.
// These functions read and validate the archive's central directory.
drfs_mz_bool drfs_mz_zip_reader_init(drfs_mz_zip_archive *pZip, drfs_mz_uint64 size, drfs_drfs_mz_uint32 flags);

// Returns the total number of files in the archive.
drfs_mz_uint drfs_mz_zip_reader_get_num_files(drfs_mz_zip_archive *pZip);

// Returns detailed information about an archive file entry.
drfs_mz_bool drfs_mz_zip_reader_file_stat(drfs_mz_zip_archive *pZip, drfs_mz_uint file_index, drfs_drfs_mz_zip_archive_file_stat *pStat);

// Determines if an archive file entry is a directory entry.
drfs_mz_bool drfs_mz_zip_reader_is_file_a_directory(drfs_mz_zip_archive *pZip, drfs_mz_uint file_index);

// Retrieves the filename of an archive file entry.
// Returns the number of bytes written to pFilename, or if filename_buf_size is 0 this function returns the number of bytes needed to fully store the filename.
drfs_mz_uint drfs_mz_zip_reader_get_filename(drfs_mz_zip_archive *pZip, drfs_mz_uint file_index, char *pFilename, drfs_mz_uint filename_buf_size);

// Attempts to locates a file in the archive's central directory.
// Valid flags: DRFS_MZ_ZIP_FLAG_CASE_SENSITIVE, DRFS_MZ_ZIP_FLAG_IGNORE_PATH
// Returns -1 if the file cannot be found.
int drfs_mz_zip_reader_locate_file(drfs_mz_zip_archive *pZip, const char *pName, const char *pComment, drfs_mz_uint flags);

// Extracts a archive file to a memory buffer using no memory allocation.
drfs_mz_bool drfs_mz_zip_reader_extract_to_mem_no_alloc(drfs_mz_zip_archive *pZip, drfs_mz_uint file_index, void *pBuf, size_t buf_size, drfs_mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size);
drfs_mz_bool drfs_mz_zip_reader_extract_file_to_mem_no_alloc(drfs_mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, drfs_mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size);

// Extracts a archive file to a memory buffer.
drfs_mz_bool drfs_mz_zip_reader_extract_to_mem(drfs_mz_zip_archive *pZip, drfs_mz_uint file_index, void *pBuf, size_t buf_size, drfs_mz_uint flags);
drfs_mz_bool drfs_mz_zip_reader_extract_file_to_mem(drfs_mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, drfs_mz_uint flags);

// Extracts a archive file to a dynamically allocated heap buffer.
void *drfs_mz_zip_reader_extract_to_heap(drfs_mz_zip_archive *pZip, drfs_mz_uint file_index, size_t *pSize, drfs_mz_uint flags);

// Ends archive reading, freeing all allocations, and closing the input archive file if mz_zip_reader_init_file() was used.
drfs_mz_bool drfs_mz_zip_reader_end(drfs_mz_zip_archive *pZip);

#endif // #ifndef DRFS_MINIZ_NO_ARCHIVE_APIS

// ------------------- Low-level Decompression API Definitions

// Decompression flags used by drfs_tinfl_decompress().
// DRFS_TINFL_FLAG_PARSE_ZLIB_HEADER: If set, the input has a valid zlib header and ends with an adler32 checksum (it's a valid zlib stream). Otherwise, the input is a raw deflate stream.
// DRFS_TINFL_FLAG_HAS_MORE_INPUT: If set, there are more input bytes available beyond the end of the supplied input buffer. If clear, the input buffer contains all remaining input.
// DRFS_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF: If set, the output buffer is large enough to hold the entire decompressed stream. If clear, the output buffer is at least the size of the dictionary (typically 32KB).
// DRFS_TINFL_FLAG_COMPUTE_ADLER32: Force adler-32 checksum computation of the decompressed bytes.
enum
{
  DRFS_TINFL_FLAG_PARSE_ZLIB_HEADER = 1,
  DRFS_TINFL_FLAG_HAS_MORE_INPUT = 2,
  DRFS_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF = 4,
  DRFS_TINFL_FLAG_COMPUTE_ADLER32 = 8
};

// High level decompression functions:
// drfs_tinfl_decompress_mem_to_heap() decompresses a block in memory to a heap block allocated via malloc().
// On entry:
//  pSrc_buf, src_buf_len: Pointer and size of the Deflate or zlib source data to decompress.
// On return:
//  Function returns a pointer to the decompressed data, or NULL on failure.
//  *pOut_len will be set to the decompressed data's size, which could be larger than src_buf_len on uncompressible data.
//  The caller must call drfs_mz_free() on the returned block when it's no longer needed.
void *drfs_tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags);

// drfs_tinfl_decompress_mem_to_mem() decompresses a block in memory to another block in memory.
// Returns DRFS_TINFL_DECOMPRESS_MEM_TO_MEM_FAILED on failure, or the number of bytes written on success.
#define DRFS_TINFL_DECOMPRESS_MEM_TO_MEM_FAILED ((size_t)(-1))
size_t drfs_tinfl_decompress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);

// drfs_tinfl_decompress_mem_to_callback() decompresses a block in memory to an internal 32KB buffer, and a user provided callback function will be called to flush the buffer.
// Returns 1 on success or 0 on failure.
typedef int (*tinfl_put_buf_func_ptr)(const void* pBuf, int len, void *pUser);
int drfs_tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size, tinfl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

struct drfs_tinfl_decompressor_tag; typedef struct drfs_tinfl_decompressor_tag drfs_tinfl_decompressor;

// Max size of LZ dictionary.
#define DRFS_TINFL_LZ_DICT_SIZE 32768

// Return status.
typedef enum
{
  DRFS_TINFL_STATUS_BAD_PARAM = -3,
  DRFS_TINFL_STATUS_ADLER32_MISMATCH = -2,
  DRFS_TINFL_STATUS_FAILED = -1,
  DRFS_TINFL_STATUS_DONE = 0,
  DRFS_TINFL_STATUS_NEEDS_MORE_INPUT = 1,
  DRFS_TINFL_STATUS_HAS_MORE_OUTPUT = 2
} drfs_tinfl_status;

// Initializes the decompressor to its initial state.
#define drfs_tinfl_init(r) do { (r)->m_state = 0; } DRFS_MZ_MACRO_END
#define drfs_tinfl_get_adler32(r) (r)->m_check_adler32

// Main low-level decompressor coroutine function. This is the only function actually needed for decompression. All the other functions are just high-level helpers for improved usability.
// This is a universal API, i.e. it can be used as a building block to build any desired higher level decompression API. In the limit case, it can be called once per every byte input or output.
drfs_tinfl_status drfs_tinfl_decompress(drfs_tinfl_decompressor *r, const drfs_mz_uint8 *pIn_buf_next, size_t *pIn_buf_size, drfs_mz_uint8 *pOut_buf_start, drfs_mz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const drfs_drfs_mz_uint32 decomp_flags);

// Internal/private bits follow.
enum
{
  DRFS_TINFL_MAX_HUFF_TABLES = 3, DRFS_TINFL_MAX_HUFF_SYMBOLS_0 = 288, DRFS_TINFL_MAX_HUFF_SYMBOLS_1 = 32, DRFS_TINFL_MAX_HUFF_SYMBOLS_2 = 19,
  DRFS_TINFL_FAST_LOOKUP_BITS = 10, DRFS_TINFL_FAST_LOOKUP_SIZE = 1 << DRFS_TINFL_FAST_LOOKUP_BITS
};

typedef struct
{
  drfs_mz_uint8 m_code_size[DRFS_TINFL_MAX_HUFF_SYMBOLS_0];
  drfs_mz_int16 m_look_up[DRFS_TINFL_FAST_LOOKUP_SIZE], m_tree[DRFS_TINFL_MAX_HUFF_SYMBOLS_0 * 2];
} drfs_tinfl_huff_table;

#if DRFS_MINIZ_HAS_64BIT_REGISTERS
  #define DRFS_TINFL_USE_64BIT_BITBUF 1
#endif

#if DRFS_TINFL_USE_64BIT_BITBUF
  typedef drfs_mz_uint64 drfs_tinfl_bit_buf_t;
  #define DRFS_TINFL_BITBUF_SIZE (64)
#else
  typedef drfs_drfs_mz_uint32 drfs_tinfl_bit_buf_t;
  #define DRFS_TINFL_BITBUF_SIZE (32)
#endif

struct drfs_tinfl_decompressor_tag
{
  drfs_drfs_mz_uint32 m_state, m_num_bits, m_zhdr0, m_zhdr1, m_z_adler32, m_final, m_type, m_check_adler32, m_dist, m_counter, m_num_extra, m_table_sizes[DRFS_TINFL_MAX_HUFF_TABLES];
  drfs_tinfl_bit_buf_t m_bit_buf;
  size_t m_dist_from_out_buf_start;
  drfs_tinfl_huff_table m_tables[DRFS_TINFL_MAX_HUFF_TABLES];
  drfs_mz_uint8 m_raw_header[4], m_len_codes[DRFS_TINFL_MAX_HUFF_SYMBOLS_0 + DRFS_TINFL_MAX_HUFF_SYMBOLS_1 + 137];
};

#ifdef __cplusplus
}
#endif

#endif // MINIZ_HEADER_INCLUDED

// ------------------- End of Header: Implementation follows. (If you only want the header, define DRFS_MINIZ_HEADER_FILE_ONLY.)

#ifndef DRFS_MINIZ_HEADER_FILE_ONLY

typedef unsigned char mz_validate_uint16[sizeof(drfs_drfs_mz_uint16)==2 ? 1 : -1];
typedef unsigned char mz_validate_uint32[sizeof(drfs_drfs_mz_uint32)==4 ? 1 : -1];
typedef unsigned char mz_validate_uint64[sizeof(drfs_mz_uint64)==8 ? 1 : -1];

#include <string.h>
#include <assert.h>

#define DRFS_MZ_ASSERT(x) assert(x)

#define DRFS_MZ_MALLOC(x) malloc(x)
#define DRFS_MZ_FREE(x) free(x)
#define DRFS_MZ_REALLOC(p, x) realloc(p, x)

#define DRFS_MZ_MAX(a,b) (((a)>(b))?(a):(b))
#define DRFS_MZ_MIN(a,b) (((a)<(b))?(a):(b))
#define DRFS_MZ_CLEAR_OBJ(obj) memset(&(obj), 0, sizeof(obj))

#if DRFS_MINIZ_USE_UNALIGNED_LOADS_AND_STORES && DRFS_MINIZ_LITTLE_ENDIAN
  #define DRFS_MZ_READ_LE16(p) *((const drfs_drfs_mz_uint16 *)(p))
  #define DRFS_MZ_READ_LE32(p) *((const drfs_drfs_mz_uint32 *)(p))
#else
  #define DRFS_MZ_READ_LE16(p) ((drfs_drfs_mz_uint32)(((const drfs_mz_uint8 *)(p))[0]) | ((drfs_drfs_mz_uint32)(((const drfs_mz_uint8 *)(p))[1]) << 8U))
  #define DRFS_MZ_READ_LE32(p) ((drfs_drfs_mz_uint32)(((const drfs_mz_uint8 *)(p))[0]) | ((drfs_drfs_mz_uint32)(((const drfs_mz_uint8 *)(p))[1]) << 8U) | ((drfs_drfs_mz_uint32)(((const drfs_mz_uint8 *)(p))[2]) << 16U) | ((drfs_drfs_mz_uint32)(((const drfs_mz_uint8 *)(p))[3]) << 24U))
#endif

#ifdef _MSC_VER
  #define DRFS_MZ_FORCEINLINE __forceinline
#elif defined(__GNUC__)
  #define DRFS_MZ_FORCEINLINE inline __attribute__((__always_inline__))
#else
  #define DRFS_MZ_FORCEINLINE inline
#endif

#ifdef __cplusplus
  extern "C" {
#endif

static void *drfs__def_alloc_func(void *opaque, size_t items, size_t size) { (void)opaque, (void)items, (void)size; return DRFS_MZ_MALLOC(items * size); }
static void drfs__def_free_func(void *opaque, void *address) { (void)opaque, (void)address; DRFS_MZ_FREE(address); }
static void *drfs__def_realloc_func(void *opaque, void *address, size_t items, size_t size) { (void)opaque, (void)address, (void)items, (void)size; return DRFS_MZ_REALLOC(address, items * size); }

// ------------------- zlib-style API's

drfs_mz_ulong drfs_mz_adler32(drfs_mz_ulong adler, const unsigned char *ptr, size_t buf_len)
{
  drfs_drfs_mz_uint32 i, s1 = (drfs_drfs_mz_uint32)(adler & 0xffff), s2 = (drfs_drfs_mz_uint32)(adler >> 16); size_t block_len = buf_len % 5552;
  if (!ptr) return DRFS_MZ_ADLER32_INIT;
  while (buf_len) {
    for (i = 0; i + 7 < block_len; i += 8, ptr += 8) {
      s1 += ptr[0], s2 += s1; s1 += ptr[1], s2 += s1; s1 += ptr[2], s2 += s1; s1 += ptr[3], s2 += s1;
      s1 += ptr[4], s2 += s1; s1 += ptr[5], s2 += s1; s1 += ptr[6], s2 += s1; s1 += ptr[7], s2 += s1;
    }
    for ( ; i < block_len; ++i) s1 += *ptr++, s2 += s1;
    s1 %= 65521U, s2 %= 65521U; buf_len -= block_len; block_len = 5552;
  }
  return (s2 << 16) + s1;
}

// Karl Malbrain's compact CRC-32. See "A compact CCITT crc16 and crc32 C implementation that balances processor cache usage against speed": http://www.geocities.com/malbrain/
drfs_mz_ulong drfs_mz_crc32(drfs_mz_ulong crc, const drfs_mz_uint8 *ptr, size_t buf_len)
{
  static const drfs_drfs_mz_uint32 s_crc32[16] = { 0, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
  drfs_drfs_mz_uint32 crcu32 = (drfs_drfs_mz_uint32)crc;
  if (!ptr) return DRFS_MZ_CRC32_INIT;
  crcu32 = ~crcu32; while (buf_len--) { drfs_mz_uint8 b = *ptr++; crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b & 0xF)]; crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b >> 4)]; }
  return ~crcu32;
}

void drfs_mz_free(void *p)
{
  DRFS_MZ_FREE(p);
}


// ------------------- Low-level Decompression (completely independent from all compression API's)

#define DRFS_TINFL_MEMCPY(d, s, l) memcpy(d, s, l)
#define DRFS_TINFL_MEMSET(p, c, l) memset(p, c, l)

#define DRFS_TINFL_CR_BEGIN switch(r->m_state) { case 0:
#define DRFS_TINFL_CR_RETURN(state_index, result) do { status = result; r->m_state = state_index; goto common_exit; case state_index:; } DRFS_MZ_MACRO_END
#define DRFS_TINFL_CR_RETURN_FOREVER(state_index, result) do { for ( ; ; ) { DRFS_TINFL_CR_RETURN(state_index, result); } } DRFS_MZ_MACRO_END
#define DRFS_TINFL_CR_FINISH }

// TODO: If the caller has indicated that there's no more input, and we attempt to read beyond the input buf, then something is wrong with the input because the inflator never
// reads ahead more than it needs to. Currently DRFS_TINFL_GET_BYTE() pads the end of the stream with 0's in this scenario.
#define DRFS_TINFL_GET_BYTE(state_index, c) do { \
  if (pIn_buf_cur >= pIn_buf_end) { \
    for ( ; ; ) { \
      if (decomp_flags & DRFS_TINFL_FLAG_HAS_MORE_INPUT) { \
        DRFS_TINFL_CR_RETURN(state_index, DRFS_TINFL_STATUS_NEEDS_MORE_INPUT); \
        if (pIn_buf_cur < pIn_buf_end) { \
          c = *pIn_buf_cur++; \
          break; \
        } \
      } else { \
        c = 0; \
        break; \
      } \
    } \
  } else c = *pIn_buf_cur++; } DRFS_MZ_MACRO_END

#define DRFS_TINFL_NEED_BITS(state_index, n) do { drfs_mz_uint c; DRFS_TINFL_GET_BYTE(state_index, c); bit_buf |= (((drfs_tinfl_bit_buf_t)c) << num_bits); num_bits += 8; } while (num_bits < (drfs_mz_uint)(n))
#define DRFS_TINFL_SKIP_BITS(state_index, n) do { if (num_bits < (drfs_mz_uint)(n)) { DRFS_TINFL_NEED_BITS(state_index, n); } bit_buf >>= (n); num_bits -= (n); } DRFS_MZ_MACRO_END
#define DRFS_TINFL_GET_BITS(state_index, b, n) do { if (num_bits < (drfs_mz_uint)(n)) { DRFS_TINFL_NEED_BITS(state_index, n); } b = bit_buf & ((1 << (n)) - 1); bit_buf >>= (n); num_bits -= (n); } DRFS_MZ_MACRO_END

// DRFS_TINFL_HUFF_BITBUF_FILL() is only used rarely, when the number of bytes remaining in the input buffer falls below 2.
// It reads just enough bytes from the input stream that are needed to decode the next Huffman code (and absolutely no more). It works by trying to fully decode a
// Huffman code by using whatever bits are currently present in the bit buffer. If this fails, it reads another byte, and tries again until it succeeds or until the
// bit buffer contains >=15 bits (deflate's max. Huffman code size).
#define DRFS_TINFL_HUFF_BITBUF_FILL(state_index, pHuff) \
  do { \
    temp = (pHuff)->m_look_up[bit_buf & (DRFS_TINFL_FAST_LOOKUP_SIZE - 1)]; \
    if (temp >= 0) { \
      code_len = temp >> 9; \
      if ((code_len) && (num_bits >= code_len)) \
      break; \
    } else if (num_bits > DRFS_TINFL_FAST_LOOKUP_BITS) { \
       code_len = DRFS_TINFL_FAST_LOOKUP_BITS; \
       do { \
          temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)]; \
       } while ((temp < 0) && (num_bits >= (code_len + 1))); if (temp >= 0) break; \
    } DRFS_TINFL_GET_BYTE(state_index, c); bit_buf |= (((drfs_tinfl_bit_buf_t)c) << num_bits); num_bits += 8; \
  } while (num_bits < 15);

// DRFS_TINFL_HUFF_DECODE() decodes the next Huffman coded symbol. It's more complex than you would initially expect because the zlib API expects the decompressor to never read
// beyond the final byte of the deflate stream. (In other words, when this macro wants to read another byte from the input, it REALLY needs another byte in order to fully
// decode the next Huffman code.) Handling this properly is particularly important on raw deflate (non-zlib) streams, which aren't followed by a byte aligned adler-32.
// The slow path is only executed at the very end of the input buffer.
#define DRFS_TINFL_HUFF_DECODE(state_index, sym, pHuff) do { \
  int temp; drfs_mz_uint code_len, c; \
  if (num_bits < 15) { \
    if ((pIn_buf_end - pIn_buf_cur) < 2) { \
       DRFS_TINFL_HUFF_BITBUF_FILL(state_index, pHuff); \
    } else { \
       bit_buf |= (((drfs_tinfl_bit_buf_t)pIn_buf_cur[0]) << num_bits) | (((drfs_tinfl_bit_buf_t)pIn_buf_cur[1]) << (num_bits + 8)); pIn_buf_cur += 2; num_bits += 16; \
    } \
  } \
  if ((temp = (pHuff)->m_look_up[bit_buf & (DRFS_TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0) \
    code_len = temp >> 9, temp &= 511; \
  else { \
    code_len = DRFS_TINFL_FAST_LOOKUP_BITS; do { temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)]; } while (temp < 0); \
  } sym = temp; bit_buf >>= code_len; num_bits -= code_len; } DRFS_MZ_MACRO_END

drfs_tinfl_status drfs_tinfl_decompress(drfs_tinfl_decompressor *r, const drfs_mz_uint8 *pIn_buf_next, size_t *pIn_buf_size, drfs_mz_uint8 *pOut_buf_start, drfs_mz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const drfs_drfs_mz_uint32 decomp_flags)
{
  static const int s_length_base[31] = { 3,4,5,6,7,8,9,10,11,13, 15,17,19,23,27,31,35,43,51,59, 67,83,99,115,131,163,195,227,258,0,0 };
  static const int s_length_extra[31]= { 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };
  static const int s_dist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193, 257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};
  static const int s_dist_extra[32] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
  static const drfs_mz_uint8 s_length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
  static const int s_min_table_sizes[3] = { 257, 1, 4 };

  drfs_tinfl_status status = DRFS_TINFL_STATUS_FAILED; drfs_drfs_mz_uint32 num_bits, dist, counter, num_extra; drfs_tinfl_bit_buf_t bit_buf;
  const drfs_mz_uint8 *pIn_buf_cur = pIn_buf_next, *const pIn_buf_end = pIn_buf_next + *pIn_buf_size;
  drfs_mz_uint8 *pOut_buf_cur = pOut_buf_next, *const pOut_buf_end = pOut_buf_next + *pOut_buf_size;
  size_t out_buf_size_mask = (decomp_flags & DRFS_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF) ? (size_t)-1 : ((pOut_buf_next - pOut_buf_start) + *pOut_buf_size) - 1, dist_from_out_buf_start;

  // Ensure the output buffer's size is a power of 2, unless the output buffer is large enough to hold the entire output file (in which case it doesn't matter).
  if (((out_buf_size_mask + 1) & out_buf_size_mask) || (pOut_buf_next < pOut_buf_start)) { *pIn_buf_size = *pOut_buf_size = 0; return DRFS_TINFL_STATUS_BAD_PARAM; }

  num_bits = r->m_num_bits; bit_buf = r->m_bit_buf; dist = r->m_dist; counter = r->m_counter; num_extra = r->m_num_extra; dist_from_out_buf_start = r->m_dist_from_out_buf_start;
  DRFS_TINFL_CR_BEGIN

  bit_buf = num_bits = dist = counter = num_extra = r->m_zhdr0 = r->m_zhdr1 = 0; r->m_z_adler32 = r->m_check_adler32 = 1;
  if (decomp_flags & DRFS_TINFL_FLAG_PARSE_ZLIB_HEADER)
  {
    DRFS_TINFL_GET_BYTE(1, r->m_zhdr0); DRFS_TINFL_GET_BYTE(2, r->m_zhdr1);
    counter = (((r->m_zhdr0 * 256 + r->m_zhdr1) % 31 != 0) || (r->m_zhdr1 & 32) || ((r->m_zhdr0 & 15) != 8));
    if (!(decomp_flags & DRFS_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)) counter |= (((1U << (8U + (r->m_zhdr0 >> 4))) > 32768U) || ((out_buf_size_mask + 1) < (size_t)(1U << (8U + (r->m_zhdr0 >> 4)))));
    if (counter) { DRFS_TINFL_CR_RETURN_FOREVER(36, DRFS_TINFL_STATUS_FAILED); }
  }

  do
  {
    DRFS_TINFL_GET_BITS(3, r->m_final, 3); r->m_type = r->m_final >> 1;
    if (r->m_type == 0)
    {
      DRFS_TINFL_SKIP_BITS(5, num_bits & 7);
      for (counter = 0; counter < 4; ++counter) { if (num_bits) DRFS_TINFL_GET_BITS(6, r->m_raw_header[counter], 8); else DRFS_TINFL_GET_BYTE(7, r->m_raw_header[counter]); }
      if ((counter = (r->m_raw_header[0] | (r->m_raw_header[1] << 8))) != (drfs_mz_uint)(0xFFFF ^ (r->m_raw_header[2] | (r->m_raw_header[3] << 8)))) { DRFS_TINFL_CR_RETURN_FOREVER(39, DRFS_TINFL_STATUS_FAILED); }
      while ((counter) && (num_bits))
      {
        DRFS_TINFL_GET_BITS(51, dist, 8);
        while (pOut_buf_cur >= pOut_buf_end) { DRFS_TINFL_CR_RETURN(52, DRFS_TINFL_STATUS_HAS_MORE_OUTPUT); }
        *pOut_buf_cur++ = (drfs_mz_uint8)dist;
        counter--;
      }
      while (counter)
      {
        size_t n; while (pOut_buf_cur >= pOut_buf_end) { DRFS_TINFL_CR_RETURN(9, DRFS_TINFL_STATUS_HAS_MORE_OUTPUT); }
        while (pIn_buf_cur >= pIn_buf_end)
        {
          if (decomp_flags & DRFS_TINFL_FLAG_HAS_MORE_INPUT)
          {
            DRFS_TINFL_CR_RETURN(38, DRFS_TINFL_STATUS_NEEDS_MORE_INPUT);
          }
          else
          {
            DRFS_TINFL_CR_RETURN_FOREVER(40, DRFS_TINFL_STATUS_FAILED);
          }
        }
        n = DRFS_MZ_MIN(DRFS_MZ_MIN((size_t)(pOut_buf_end - pOut_buf_cur), (size_t)(pIn_buf_end - pIn_buf_cur)), counter);
        DRFS_TINFL_MEMCPY(pOut_buf_cur, pIn_buf_cur, n); pIn_buf_cur += n; pOut_buf_cur += n; counter -= (drfs_mz_uint)n;
      }
    }
    else if (r->m_type == 3)
    {
      DRFS_TINFL_CR_RETURN_FOREVER(10, DRFS_TINFL_STATUS_FAILED);
    }
    else
    {
      if (r->m_type == 1)
      {
        drfs_mz_uint8 *p = r->m_tables[0].m_code_size; drfs_mz_uint i;
        r->m_table_sizes[0] = 288; r->m_table_sizes[1] = 32; DRFS_TINFL_MEMSET(r->m_tables[1].m_code_size, 5, 32);
        for ( i = 0; i <= 143; ++i) *p++ = 8; for ( ; i <= 255; ++i) *p++ = 9; for ( ; i <= 279; ++i) *p++ = 7; for ( ; i <= 287; ++i) *p++ = 8;
      }
      else
      {
        for (counter = 0; counter < 3; counter++) { DRFS_TINFL_GET_BITS(11, r->m_table_sizes[counter], "\05\05\04"[counter]); r->m_table_sizes[counter] += s_min_table_sizes[counter]; }
        DRFS_MZ_CLEAR_OBJ(r->m_tables[2].m_code_size); for (counter = 0; counter < r->m_table_sizes[2]; counter++) { drfs_mz_uint s; DRFS_TINFL_GET_BITS(14, s, 3); r->m_tables[2].m_code_size[s_length_dezigzag[counter]] = (drfs_mz_uint8)s; }
        r->m_table_sizes[2] = 19;
      }
      for ( ; (int)r->m_type >= 0; r->m_type--)
      {
        int tree_next, tree_cur; drfs_tinfl_huff_table *pTable;
        drfs_mz_uint i, j, used_syms, total, sym_index, next_code[17], total_syms[16]; pTable = &r->m_tables[r->m_type]; DRFS_MZ_CLEAR_OBJ(total_syms); DRFS_MZ_CLEAR_OBJ(pTable->m_look_up); DRFS_MZ_CLEAR_OBJ(pTable->m_tree);
        for (i = 0; i < r->m_table_sizes[r->m_type]; ++i) total_syms[pTable->m_code_size[i]]++;
        used_syms = 0, total = 0; next_code[0] = next_code[1] = 0;
        for (i = 1; i <= 15; ++i) { used_syms += total_syms[i]; next_code[i + 1] = (total = ((total + total_syms[i]) << 1)); }
        if ((65536 != total) && (used_syms > 1))
        {
          DRFS_TINFL_CR_RETURN_FOREVER(35, DRFS_TINFL_STATUS_FAILED);
        }
        for (tree_next = -1, sym_index = 0; sym_index < r->m_table_sizes[r->m_type]; ++sym_index)
        {
          drfs_mz_uint rev_code = 0, l, cur_code, code_size = pTable->m_code_size[sym_index]; if (!code_size) continue;
          cur_code = next_code[code_size]++; for (l = code_size; l > 0; l--, cur_code >>= 1) rev_code = (rev_code << 1) | (cur_code & 1);
          if (code_size <= DRFS_TINFL_FAST_LOOKUP_BITS) { drfs_mz_int16 k = (drfs_mz_int16)((code_size << 9) | sym_index); while (rev_code < DRFS_TINFL_FAST_LOOKUP_SIZE) { pTable->m_look_up[rev_code] = k; rev_code += (1 << code_size); } continue; }
          if (0 == (tree_cur = pTable->m_look_up[rev_code & (DRFS_TINFL_FAST_LOOKUP_SIZE - 1)])) { pTable->m_look_up[rev_code & (DRFS_TINFL_FAST_LOOKUP_SIZE - 1)] = (drfs_mz_int16)tree_next; tree_cur = tree_next; tree_next -= 2; }
          rev_code >>= (DRFS_TINFL_FAST_LOOKUP_BITS - 1);
          for (j = code_size; j > (DRFS_TINFL_FAST_LOOKUP_BITS + 1); j--)
          {
            tree_cur -= ((rev_code >>= 1) & 1);
            if (!pTable->m_tree[-tree_cur - 1]) { pTable->m_tree[-tree_cur - 1] = (drfs_mz_int16)tree_next; tree_cur = tree_next; tree_next -= 2; } else tree_cur = pTable->m_tree[-tree_cur - 1];
          }
          tree_cur -= ((rev_code >>= 1) & 1); pTable->m_tree[-tree_cur - 1] = (drfs_mz_int16)sym_index;
        }
        if (r->m_type == 2)
        {
          for (counter = 0; counter < (r->m_table_sizes[0] + r->m_table_sizes[1]); )
          {
            drfs_mz_uint s; DRFS_TINFL_HUFF_DECODE(16, dist, &r->m_tables[2]); if (dist < 16) { r->m_len_codes[counter++] = (drfs_mz_uint8)dist; continue; }
            if ((dist == 16) && (!counter))
            {
              DRFS_TINFL_CR_RETURN_FOREVER(17, DRFS_TINFL_STATUS_FAILED);
            }
            num_extra = "\02\03\07"[dist - 16]; DRFS_TINFL_GET_BITS(18, s, num_extra); s += "\03\03\013"[dist - 16];
            DRFS_TINFL_MEMSET(r->m_len_codes + counter, (dist == 16) ? r->m_len_codes[counter - 1] : 0, s); counter += s;
          }
          if ((r->m_table_sizes[0] + r->m_table_sizes[1]) != counter)
          {
            DRFS_TINFL_CR_RETURN_FOREVER(21, DRFS_TINFL_STATUS_FAILED);
          }
          DRFS_TINFL_MEMCPY(r->m_tables[0].m_code_size, r->m_len_codes, r->m_table_sizes[0]); DRFS_TINFL_MEMCPY(r->m_tables[1].m_code_size, r->m_len_codes + r->m_table_sizes[0], r->m_table_sizes[1]);
        }
      }
      for ( ; ; )
      {
        drfs_mz_uint8 *pSrc;
        for ( ; ; )
        {
          if (((pIn_buf_end - pIn_buf_cur) < 4) || ((pOut_buf_end - pOut_buf_cur) < 2))
          {
            DRFS_TINFL_HUFF_DECODE(23, counter, &r->m_tables[0]);
            if (counter >= 256)
              break;
            while (pOut_buf_cur >= pOut_buf_end) { DRFS_TINFL_CR_RETURN(24, DRFS_TINFL_STATUS_HAS_MORE_OUTPUT); }
            *pOut_buf_cur++ = (drfs_mz_uint8)counter;
          }
          else
          {
            int sym2; drfs_mz_uint code_len;
#if DRFS_TINFL_USE_64BIT_BITBUF
            if (num_bits < 30) { bit_buf |= (((drfs_tinfl_bit_buf_t)DRFS_MZ_READ_LE32(pIn_buf_cur)) << num_bits); pIn_buf_cur += 4; num_bits += 32; }
#else
            if (num_bits < 15) { bit_buf |= (((drfs_tinfl_bit_buf_t)DRFS_MZ_READ_LE16(pIn_buf_cur)) << num_bits); pIn_buf_cur += 2; num_bits += 16; }
#endif
            if ((sym2 = r->m_tables[0].m_look_up[bit_buf & (DRFS_TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
              code_len = sym2 >> 9;
            else
            {
              code_len = DRFS_TINFL_FAST_LOOKUP_BITS; do { sym2 = r->m_tables[0].m_tree[~sym2 + ((bit_buf >> code_len++) & 1)]; } while (sym2 < 0);
            }
            counter = sym2; bit_buf >>= code_len; num_bits -= code_len;
            if (counter & 256)
              break;

#if !DRFS_TINFL_USE_64BIT_BITBUF
            if (num_bits < 15) { bit_buf |= (((drfs_tinfl_bit_buf_t)DRFS_MZ_READ_LE16(pIn_buf_cur)) << num_bits); pIn_buf_cur += 2; num_bits += 16; }
#endif
            if ((sym2 = r->m_tables[0].m_look_up[bit_buf & (DRFS_TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
              code_len = sym2 >> 9;
            else
            {
              code_len = DRFS_TINFL_FAST_LOOKUP_BITS; do { sym2 = r->m_tables[0].m_tree[~sym2 + ((bit_buf >> code_len++) & 1)]; } while (sym2 < 0);
            }
            bit_buf >>= code_len; num_bits -= code_len;

            pOut_buf_cur[0] = (drfs_mz_uint8)counter;
            if (sym2 & 256)
            {
              pOut_buf_cur++;
              counter = sym2;
              break;
            }
            pOut_buf_cur[1] = (drfs_mz_uint8)sym2;
            pOut_buf_cur += 2;
          }
        }
        if ((counter &= 511) == 256) break;

        num_extra = s_length_extra[counter - 257]; counter = s_length_base[counter - 257];
        if (num_extra) { drfs_mz_uint extra_bits; DRFS_TINFL_GET_BITS(25, extra_bits, num_extra); counter += extra_bits; }

        DRFS_TINFL_HUFF_DECODE(26, dist, &r->m_tables[1]);
        num_extra = s_dist_extra[dist]; dist = s_dist_base[dist];
        if (num_extra) { drfs_mz_uint extra_bits; DRFS_TINFL_GET_BITS(27, extra_bits, num_extra); dist += extra_bits; }

        dist_from_out_buf_start = pOut_buf_cur - pOut_buf_start;
        if ((dist > dist_from_out_buf_start) && (decomp_flags & DRFS_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF))
        {
          DRFS_TINFL_CR_RETURN_FOREVER(37, DRFS_TINFL_STATUS_FAILED);
        }

        pSrc = pOut_buf_start + ((dist_from_out_buf_start - dist) & out_buf_size_mask);

        if ((DRFS_MZ_MAX(pOut_buf_cur, pSrc) + counter) > pOut_buf_end)
        {
          while (counter--)
          {
            while (pOut_buf_cur >= pOut_buf_end) { DRFS_TINFL_CR_RETURN(53, DRFS_TINFL_STATUS_HAS_MORE_OUTPUT); }
            *pOut_buf_cur++ = pOut_buf_start[(dist_from_out_buf_start++ - dist) & out_buf_size_mask];
          }
          continue;
        }
#if DRFS_MINIZ_USE_UNALIGNED_LOADS_AND_STORES
        else if ((counter >= 9) && (counter <= dist))
        {
          const drfs_mz_uint8 *pSrc_end = pSrc + (counter & ~7);
          do
          {
            ((drfs_drfs_mz_uint32 *)pOut_buf_cur)[0] = ((const drfs_drfs_mz_uint32 *)pSrc)[0];
            ((drfs_drfs_mz_uint32 *)pOut_buf_cur)[1] = ((const drfs_drfs_mz_uint32 *)pSrc)[1];
            pOut_buf_cur += 8;
          } while ((pSrc += 8) < pSrc_end);
          if ((counter &= 7) < 3)
          {
            if (counter)
            {
              pOut_buf_cur[0] = pSrc[0];
              if (counter > 1)
                pOut_buf_cur[1] = pSrc[1];
              pOut_buf_cur += counter;
            }
            continue;
          }
        }
#endif
        do
        {
          pOut_buf_cur[0] = pSrc[0];
          pOut_buf_cur[1] = pSrc[1];
          pOut_buf_cur[2] = pSrc[2];
          pOut_buf_cur += 3; pSrc += 3;
        } while ((int)(counter -= 3) > 2);
        if ((int)counter > 0)
        {
          pOut_buf_cur[0] = pSrc[0];
          if ((int)counter > 1)
            pOut_buf_cur[1] = pSrc[1];
          pOut_buf_cur += counter;
        }
      }
    }
  } while (!(r->m_final & 1));
  if (decomp_flags & DRFS_TINFL_FLAG_PARSE_ZLIB_HEADER)
  {
    DRFS_TINFL_SKIP_BITS(32, num_bits & 7); for (counter = 0; counter < 4; ++counter) { drfs_mz_uint s; if (num_bits) DRFS_TINFL_GET_BITS(41, s, 8); else DRFS_TINFL_GET_BYTE(42, s); r->m_z_adler32 = (r->m_z_adler32 << 8) | s; }
  }
  DRFS_TINFL_CR_RETURN_FOREVER(34, DRFS_TINFL_STATUS_DONE);
  DRFS_TINFL_CR_FINISH

common_exit:
  r->m_num_bits = num_bits; r->m_bit_buf = bit_buf; r->m_dist = dist; r->m_counter = counter; r->m_num_extra = num_extra; r->m_dist_from_out_buf_start = dist_from_out_buf_start;
  *pIn_buf_size = pIn_buf_cur - pIn_buf_next; *pOut_buf_size = pOut_buf_cur - pOut_buf_next;
  if ((decomp_flags & (DRFS_TINFL_FLAG_PARSE_ZLIB_HEADER | DRFS_TINFL_FLAG_COMPUTE_ADLER32)) && (status >= 0))
  {
    const drfs_mz_uint8 *ptr = pOut_buf_next; size_t buf_len = *pOut_buf_size;
    drfs_drfs_mz_uint32 i, s1 = r->m_check_adler32 & 0xffff, s2 = r->m_check_adler32 >> 16; size_t block_len = buf_len % 5552;
    while (buf_len)
    {
      for (i = 0; i + 7 < block_len; i += 8, ptr += 8)
      {
        s1 += ptr[0], s2 += s1; s1 += ptr[1], s2 += s1; s1 += ptr[2], s2 += s1; s1 += ptr[3], s2 += s1;
        s1 += ptr[4], s2 += s1; s1 += ptr[5], s2 += s1; s1 += ptr[6], s2 += s1; s1 += ptr[7], s2 += s1;
      }
      for ( ; i < block_len; ++i) s1 += *ptr++, s2 += s1;
      s1 %= 65521U, s2 %= 65521U; buf_len -= block_len; block_len = 5552;
    }
    r->m_check_adler32 = (s2 << 16) + s1; if ((status == DRFS_TINFL_STATUS_DONE) && (decomp_flags & DRFS_TINFL_FLAG_PARSE_ZLIB_HEADER) && (r->m_check_adler32 != r->m_z_adler32)) status = DRFS_TINFL_STATUS_ADLER32_MISMATCH;
  }
  return status;
}

// Higher level helper functions.
void *drfs_tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags)
{
  drfs_tinfl_decompressor decomp; void *pBuf = NULL, *pNew_buf; size_t src_buf_ofs = 0, out_buf_capacity = 0;
  *pOut_len = 0;
  drfs_tinfl_init(&decomp);
  for ( ; ; )
  {
    size_t src_buf_size = src_buf_len - src_buf_ofs, dst_buf_size = out_buf_capacity - *pOut_len, new_out_buf_capacity;
    drfs_tinfl_status status = drfs_tinfl_decompress(&decomp, (const drfs_mz_uint8*)pSrc_buf + src_buf_ofs, &src_buf_size, (drfs_mz_uint8*)pBuf, pBuf ? (drfs_mz_uint8*)pBuf + *pOut_len : NULL, &dst_buf_size,
      (flags & ~DRFS_TINFL_FLAG_HAS_MORE_INPUT) | DRFS_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
    if ((status < 0) || (status == DRFS_TINFL_STATUS_NEEDS_MORE_INPUT))
    {
      DRFS_MZ_FREE(pBuf); *pOut_len = 0; return NULL;
    }
    src_buf_ofs += src_buf_size;
    *pOut_len += dst_buf_size;
    if (status == DRFS_TINFL_STATUS_DONE) break;
    new_out_buf_capacity = out_buf_capacity * 2; if (new_out_buf_capacity < 128) new_out_buf_capacity = 128;
    pNew_buf = DRFS_MZ_REALLOC(pBuf, new_out_buf_capacity);
    if (!pNew_buf)
    {
      DRFS_MZ_FREE(pBuf); *pOut_len = 0; return NULL;
    }
    pBuf = pNew_buf; out_buf_capacity = new_out_buf_capacity;
  }
  return pBuf;
}

size_t drfs_tinfl_decompress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags)
{
  drfs_tinfl_decompressor decomp; drfs_tinfl_status status; drfs_tinfl_init(&decomp);
  status = drfs_tinfl_decompress(&decomp, (const drfs_mz_uint8*)pSrc_buf, &src_buf_len, (drfs_mz_uint8*)pOut_buf, (drfs_mz_uint8*)pOut_buf, &out_buf_len, (flags & ~DRFS_TINFL_FLAG_HAS_MORE_INPUT) | DRFS_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
  return (status != DRFS_TINFL_STATUS_DONE) ? DRFS_TINFL_DECOMPRESS_MEM_TO_MEM_FAILED : out_buf_len;
}

int drfs_tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size, tinfl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags)
{
  int result = 0;
  drfs_tinfl_decompressor decomp;
  drfs_mz_uint8 *pDict = (drfs_mz_uint8*)DRFS_MZ_MALLOC(DRFS_TINFL_LZ_DICT_SIZE); size_t in_buf_ofs = 0, dict_ofs = 0;
  if (!pDict)
    return DRFS_TINFL_STATUS_FAILED;
  drfs_tinfl_init(&decomp);
  for ( ; ; )
  {
    size_t in_buf_size = *pIn_buf_size - in_buf_ofs, dst_buf_size = DRFS_TINFL_LZ_DICT_SIZE - dict_ofs;
    drfs_tinfl_status status = drfs_tinfl_decompress(&decomp, (const drfs_mz_uint8*)pIn_buf + in_buf_ofs, &in_buf_size, pDict, pDict + dict_ofs, &dst_buf_size,
      (flags & ~(DRFS_TINFL_FLAG_HAS_MORE_INPUT | DRFS_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)));
    in_buf_ofs += in_buf_size;
    if ((dst_buf_size) && (!(*pPut_buf_func)(pDict + dict_ofs, (int)dst_buf_size, pPut_buf_user)))
      break;
    if (status != DRFS_TINFL_STATUS_HAS_MORE_OUTPUT)
    {
      result = (status == DRFS_TINFL_STATUS_DONE);
      break;
    }
    dict_ofs = (dict_ofs + dst_buf_size) & (DRFS_TINFL_LZ_DICT_SIZE - 1);
  }
  DRFS_MZ_FREE(pDict);
  *pIn_buf_size = in_buf_ofs;
  return result;
}


// ------------------- .ZIP archive reading

#ifndef DRFS_MINIZ_NO_ARCHIVE_APIS

#define DRFS_MZ_TOLOWER(c) ((((c) >= 'A') && ((c) <= 'Z')) ? ((c) - 'A' + 'a') : (c))

// Various ZIP archive enums. To completely avoid cross platform compiler alignment and platform endian issues, miniz.c doesn't use structs for any of this stuff.
enum
{
  // ZIP archive identifiers and record sizes
  DRFS_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG = 0x06054b50, DRFS_MZ_ZIP_CENTRAL_DIR_HEADER_SIG = 0x02014b50, DRFS_MZ_ZIP_LOCAL_DIR_HEADER_SIG = 0x04034b50,
  DRFS_MZ_ZIP_LOCAL_DIR_HEADER_SIZE = 30, DRFS_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE = 46, DRFS_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE = 22,
  // Central directory header record offsets
  DRFS_MZ_ZIP_CDH_SIG_OFS = 0, DRFS_MZ_ZIP_CDH_VERSION_MADE_BY_OFS = 4, DRFS_MZ_ZIP_CDH_VERSION_NEEDED_OFS = 6, DRFS_MZ_ZIP_CDH_BIT_FLAG_OFS = 8,
  DRFS_MZ_ZIP_CDH_METHOD_OFS = 10, DRFS_MZ_ZIP_CDH_FILE_TIME_OFS = 12, DRFS_MZ_ZIP_CDH_FILE_DATE_OFS = 14, DRFS_MZ_ZIP_CDH_CRC32_OFS = 16,
  DRFS_MZ_ZIP_CDH_COMPRESSED_SIZE_OFS = 20, DRFS_MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS = 24, DRFS_MZ_ZIP_CDH_FILENAME_LEN_OFS = 28, DRFS_MZ_ZIP_CDH_EXTRA_LEN_OFS = 30,
  DRFS_MZ_ZIP_CDH_COMMENT_LEN_OFS = 32, DRFS_MZ_ZIP_CDH_DISK_START_OFS = 34, DRFS_MZ_ZIP_CDH_INTERNAL_ATTR_OFS = 36, DRFS_MZ_ZIP_CDH_EXTERNAL_ATTR_OFS = 38, DRFS_MZ_ZIP_CDH_LOCAL_HEADER_OFS = 42,
  // Local directory header offsets
  DRFS_MZ_ZIP_LDH_SIG_OFS = 0, DRFS_MZ_ZIP_LDH_VERSION_NEEDED_OFS = 4, DRFS_MZ_ZIP_LDH_BIT_FLAG_OFS = 6, DRFS_MZ_ZIP_LDH_METHOD_OFS = 8, DRFS_MZ_ZIP_LDH_FILE_TIME_OFS = 10,
  DRFS_MZ_ZIP_LDH_FILE_DATE_OFS = 12, DRFS_MZ_ZIP_LDH_CRC32_OFS = 14, DRFS_MZ_ZIP_LDH_COMPRESSED_SIZE_OFS = 18, DRFS_MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS = 22,
  DRFS_MZ_ZIP_LDH_FILENAME_LEN_OFS = 26, DRFS_MZ_ZIP_LDH_EXTRA_LEN_OFS = 28,
  // End of central directory offsets
  DRFS_MZ_ZIP_ECDH_SIG_OFS = 0, DRFS_MZ_ZIP_ECDH_NUM_THIS_DISK_OFS = 4, DRFS_MZ_ZIP_ECDH_NUM_DISK_CDIR_OFS = 6, DRFS_MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS = 8,
  DRFS_MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS = 10, DRFS_MZ_ZIP_ECDH_CDIR_SIZE_OFS = 12, DRFS_MZ_ZIP_ECDH_CDIR_OFS_OFS = 16, DRFS_MZ_ZIP_ECDH_COMMENT_SIZE_OFS = 20,
};

typedef struct
{
  void *m_p;
  size_t m_size, m_capacity;
  drfs_mz_uint m_element_size;
} drfs_mz_zip_array;

struct drfs_mz_zip_internal_state_tag
{
  drfs_mz_zip_array m_central_dir;
  drfs_mz_zip_array m_central_dir_offsets;
  drfs_mz_zip_array m_sorted_central_dir_offsets;
  void* *m_pFile;
  void *m_pMem;
  size_t m_mem_size;
  size_t m_mem_capacity;
};

#define DRFS_MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(array_ptr, element_size) (array_ptr)->m_element_size = element_size
#define DRFS_MZ_ZIP_ARRAY_ELEMENT(array_ptr, element_type, index) ((element_type *)((array_ptr)->m_p))[index]

static DRFS_MZ_FORCEINLINE void drfs_mz_zip_array_clear(drfs_mz_zip_archive *pZip, drfs_mz_zip_array *pArray)
{
  pZip->m_pFree(pZip->m_pAlloc_opaque, pArray->m_p);
  memset(pArray, 0, sizeof(drfs_mz_zip_array));
}

static drfs_mz_bool drfs_mz_zip_array_ensure_capacity(drfs_mz_zip_archive *pZip, drfs_mz_zip_array *pArray, size_t min_new_capacity, drfs_mz_uint growing)
{
  void *pNew_p; size_t new_capacity = min_new_capacity; DRFS_MZ_ASSERT(pArray->m_element_size); if (pArray->m_capacity >= min_new_capacity) return DRFS_MZ_TRUE;
  if (growing) { new_capacity = DRFS_MZ_MAX(1, pArray->m_capacity); while (new_capacity < min_new_capacity) new_capacity *= 2; }
  if (NULL == (pNew_p = pZip->m_pRealloc(pZip->m_pAlloc_opaque, pArray->m_p, pArray->m_element_size, new_capacity))) return DRFS_MZ_FALSE;
  pArray->m_p = pNew_p; pArray->m_capacity = new_capacity;
  return DRFS_MZ_TRUE;
}

static DRFS_MZ_FORCEINLINE drfs_mz_bool drfs_mz_zip_array_reserve(drfs_mz_zip_archive *pZip, drfs_mz_zip_array *pArray, size_t new_capacity, drfs_mz_uint growing)
{
  if (new_capacity > pArray->m_capacity) { if (!drfs_mz_zip_array_ensure_capacity(pZip, pArray, new_capacity, growing)) return DRFS_MZ_FALSE; }
  return DRFS_MZ_TRUE;
}

static DRFS_MZ_FORCEINLINE drfs_mz_bool drfs_mz_zip_array_resize(drfs_mz_zip_archive *pZip, drfs_mz_zip_array *pArray, size_t new_size, drfs_mz_uint growing)
{
  if (new_size > pArray->m_capacity) { if (!drfs_mz_zip_array_ensure_capacity(pZip, pArray, new_size, growing)) return DRFS_MZ_FALSE; }
  pArray->m_size = new_size;
  return DRFS_MZ_TRUE;
}

static DRFS_MZ_FORCEINLINE drfs_mz_bool drfs_mz_zip_array_ensure_room(drfs_mz_zip_archive *pZip, drfs_mz_zip_array *pArray, size_t n)
{
  return drfs_mz_zip_array_reserve(pZip, pArray, pArray->m_size + n, DRFS_MZ_TRUE);
}

static DRFS_MZ_FORCEINLINE drfs_mz_bool drfs_mz_zip_array_push_back(drfs_mz_zip_archive *pZip, drfs_mz_zip_array *pArray, const void *pElements, size_t n)
{
  size_t orig_size = pArray->m_size; if (!drfs_mz_zip_array_resize(pZip, pArray, orig_size + n, DRFS_MZ_TRUE)) return DRFS_MZ_FALSE;
  memcpy((drfs_mz_uint8*)pArray->m_p + orig_size * pArray->m_element_size, pElements, n * pArray->m_element_size);
  return DRFS_MZ_TRUE;
}

#ifndef DRFS_MINIZ_NO_TIME
static time_t drfs_mz_zip_dos_to_time_t(int dos_time, int dos_date)
{
  struct tm tm;
  memset(&tm, 0, sizeof(tm)); tm.tm_isdst = -1;
  tm.tm_year = ((dos_date >> 9) & 127) + 1980 - 1900; tm.tm_mon = ((dos_date >> 5) & 15) - 1; tm.tm_mday = dos_date & 31;
  tm.tm_hour = (dos_time >> 11) & 31; tm.tm_min = (dos_time >> 5) & 63; tm.tm_sec = (dos_time << 1) & 62;
  return mktime(&tm);
}
#endif

static drfs_mz_bool drfs_mz_zip_reader_init_internal(drfs_mz_zip_archive *pZip, drfs_drfs_mz_uint32 flags)
{
  (void)flags;
  if ((!pZip) || (pZip->m_pState) || (pZip->m_zip_mode != DRFS_MZ_ZIP_MODE_INVALID))
    return DRFS_MZ_FALSE;

  if (!pZip->m_pAlloc) pZip->m_pAlloc = drfs__def_alloc_func;
  if (!pZip->m_pFree) pZip->m_pFree = drfs__def_free_func;
  if (!pZip->m_pRealloc) pZip->m_pRealloc = drfs__def_realloc_func;

  pZip->m_zip_mode = DRFS_MZ_ZIP_MODE_READING;
  pZip->m_archive_size = 0;
  pZip->m_central_directory_file_ofs = 0;
  pZip->m_total_files = 0;

  if (NULL == (pZip->m_pState = (drfs_mz_zip_internal_state *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, sizeof(drfs_mz_zip_internal_state))))
    return DRFS_MZ_FALSE;
  memset(pZip->m_pState, 0, sizeof(drfs_mz_zip_internal_state));
  DRFS_MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir, sizeof(drfs_mz_uint8));
  DRFS_MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir_offsets, sizeof(drfs_drfs_mz_uint32));
  DRFS_MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_sorted_central_dir_offsets, sizeof(drfs_drfs_mz_uint32));
  return DRFS_MZ_TRUE;
}

static DRFS_MZ_FORCEINLINE drfs_mz_bool drfs_mz_zip_reader_filename_less(const drfs_mz_zip_array *pCentral_dir_array, const drfs_mz_zip_array *pCentral_dir_offsets, drfs_mz_uint l_index, drfs_mz_uint r_index)
{
  const drfs_mz_uint8 *pL = &DRFS_MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_array, drfs_mz_uint8, DRFS_MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, drfs_drfs_mz_uint32, l_index)), *pE;
  const drfs_mz_uint8 *pR = &DRFS_MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_array, drfs_mz_uint8, DRFS_MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, drfs_drfs_mz_uint32, r_index));
  drfs_mz_uint l_len = DRFS_MZ_READ_LE16(pL + DRFS_MZ_ZIP_CDH_FILENAME_LEN_OFS), r_len = DRFS_MZ_READ_LE16(pR + DRFS_MZ_ZIP_CDH_FILENAME_LEN_OFS);
  drfs_mz_uint8 l = 0, r = 0;
  pL += DRFS_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE; pR += DRFS_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
  pE = pL + DRFS_MZ_MIN(l_len, r_len);
  while (pL < pE)
  {
    if ((l = DRFS_MZ_TOLOWER(*pL)) != (r = DRFS_MZ_TOLOWER(*pR)))
      break;
    pL++; pR++;
  }
  return (pL == pE) ? (l_len < r_len) : (l < r);
}

#define DRFS_MZ_SWAP_UINT32(a, b) do { drfs_drfs_mz_uint32 t = a; a = b; b = t; } DRFS_MZ_MACRO_END

// Heap sort of lowercased filenames, used to help accelerate plain central directory searches by drfs_mz_zip_reader_locate_file(). (Could also use qsort(), but it could allocate memory.)
static void drfs_mz_zip_reader_sort_central_dir_offsets_by_filename(drfs_mz_zip_archive *pZip)
{
  drfs_mz_zip_internal_state *pState = pZip->m_pState;
  const drfs_mz_zip_array *pCentral_dir_offsets = &pState->m_central_dir_offsets;
  const drfs_mz_zip_array *pCentral_dir = &pState->m_central_dir;
  drfs_drfs_mz_uint32 *pIndices = &DRFS_MZ_ZIP_ARRAY_ELEMENT(&pState->m_sorted_central_dir_offsets, drfs_drfs_mz_uint32, 0);
  const int size = pZip->m_total_files;
  int start = (size - 2) >> 1, end;
  while (start >= 0)
  {
    int child, root = start;
    for ( ; ; )
    {
      if ((child = (root << 1) + 1) >= size)
        break;
      child += (((child + 1) < size) && (drfs_mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[child], pIndices[child + 1])));
      if (!drfs_mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[root], pIndices[child]))
        break;
      DRFS_MZ_SWAP_UINT32(pIndices[root], pIndices[child]); root = child;
    }
    start--;
  }

  end = size - 1;
  while (end > 0)
  {
    int child, root = 0;
    DRFS_MZ_SWAP_UINT32(pIndices[end], pIndices[0]);
    for ( ; ; )
    {
      if ((child = (root << 1) + 1) >= end)
        break;
      child += (((child + 1) < end) && drfs_mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[child], pIndices[child + 1]));
      if (!drfs_mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[root], pIndices[child]))
        break;
      DRFS_MZ_SWAP_UINT32(pIndices[root], pIndices[child]); root = child;
    }
    end--;
  }
}

static drfs_mz_bool drfs_mz_zip_reader_read_central_dir(drfs_mz_zip_archive *pZip, drfs_drfs_mz_uint32 flags)
{
  drfs_mz_uint cdir_size, num_this_disk, cdir_disk_index;
  drfs_mz_uint64 cdir_ofs;
  drfs_mz_int64 cur_file_ofs;
  const drfs_mz_uint8 *p;
  drfs_drfs_mz_uint32 buf_u32[4096 / sizeof(drfs_drfs_mz_uint32)]; drfs_mz_uint8 *pBuf = (drfs_mz_uint8 *)buf_u32;
  drfs_mz_bool sort_central_dir = ((flags & DRFS_MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY) == 0);
  // Basic sanity checks - reject files which are too small, and check the first 4 bytes of the file to make sure a local header is there.
  if (pZip->m_archive_size < DRFS_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)
    return DRFS_MZ_FALSE;
  // Find the end of central directory record by scanning the file from the end towards the beginning.
  cur_file_ofs = DRFS_MZ_MAX((drfs_mz_int64)pZip->m_archive_size - (drfs_mz_int64)sizeof(buf_u32), 0);
  for ( ; ; )
  {
    int i, n = (int)DRFS_MZ_MIN(sizeof(buf_u32), pZip->m_archive_size - cur_file_ofs);
    if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pBuf, n) != (drfs_mz_uint)n)
      return DRFS_MZ_FALSE;
    for (i = n - 4; i >= 0; --i)
      if (DRFS_MZ_READ_LE32(pBuf + i) == DRFS_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG)
        break;
    if (i >= 0)
    {
      cur_file_ofs += i;
      break;
    }
    if ((!cur_file_ofs) || ((pZip->m_archive_size - cur_file_ofs) >= (0xFFFF + DRFS_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)))
      return DRFS_MZ_FALSE;
    cur_file_ofs = DRFS_MZ_MAX(cur_file_ofs - (sizeof(buf_u32) - 3), 0);
  }
  // Read and verify the end of central directory record.
  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pBuf, DRFS_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE) != DRFS_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)
    return DRFS_MZ_FALSE;
  if ((DRFS_MZ_READ_LE32(pBuf + DRFS_MZ_ZIP_ECDH_SIG_OFS) != DRFS_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG) ||
      ((pZip->m_total_files = DRFS_MZ_READ_LE16(pBuf + DRFS_MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS)) != DRFS_MZ_READ_LE16(pBuf + DRFS_MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS)))
    return DRFS_MZ_FALSE;

  num_this_disk = DRFS_MZ_READ_LE16(pBuf + DRFS_MZ_ZIP_ECDH_NUM_THIS_DISK_OFS);
  cdir_disk_index = DRFS_MZ_READ_LE16(pBuf + DRFS_MZ_ZIP_ECDH_NUM_DISK_CDIR_OFS);
  if (((num_this_disk | cdir_disk_index) != 0) && ((num_this_disk != 1) || (cdir_disk_index != 1)))
    return DRFS_MZ_FALSE;

  if ((cdir_size = DRFS_MZ_READ_LE32(pBuf + DRFS_MZ_ZIP_ECDH_CDIR_SIZE_OFS)) < pZip->m_total_files * DRFS_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE)
    return DRFS_MZ_FALSE;

  cdir_ofs = DRFS_MZ_READ_LE32(pBuf + DRFS_MZ_ZIP_ECDH_CDIR_OFS_OFS);
  if ((cdir_ofs + (drfs_mz_uint64)cdir_size) > pZip->m_archive_size)
    return DRFS_MZ_FALSE;

  pZip->m_central_directory_file_ofs = cdir_ofs;

  if (pZip->m_total_files)
  {
     drfs_mz_uint i, n;

    // Read the entire central directory into a heap block, and allocate another heap block to hold the unsorted central dir file record offsets, and another to hold the sorted indices.
    if ((!drfs_mz_zip_array_resize(pZip, &pZip->m_pState->m_central_dir, cdir_size, DRFS_MZ_FALSE)) ||
        (!drfs_mz_zip_array_resize(pZip, &pZip->m_pState->m_central_dir_offsets, pZip->m_total_files, DRFS_MZ_FALSE)))
      return DRFS_MZ_FALSE;

    if (sort_central_dir)
    {
      if (!drfs_mz_zip_array_resize(pZip, &pZip->m_pState->m_sorted_central_dir_offsets, pZip->m_total_files, DRFS_MZ_FALSE))
        return DRFS_MZ_FALSE;
    }

    if (pZip->m_pRead(pZip->m_pIO_opaque, cdir_ofs, pZip->m_pState->m_central_dir.m_p, cdir_size) != cdir_size)
      return DRFS_MZ_FALSE;

    // Now create an index into the central directory file records, do some basic sanity checking on each record, and check for zip64 entries (which are not yet supported).
    p = (const drfs_mz_uint8 *)pZip->m_pState->m_central_dir.m_p;
    for (n = cdir_size, i = 0; i < pZip->m_total_files; ++i)
    {
      drfs_mz_uint total_header_size, comp_size, decomp_size, disk_index;
      if ((n < DRFS_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE) || (DRFS_MZ_READ_LE32(p) != DRFS_MZ_ZIP_CENTRAL_DIR_HEADER_SIG))
        return DRFS_MZ_FALSE;
      DRFS_MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, drfs_drfs_mz_uint32, i) = (drfs_drfs_mz_uint32)(p - (const drfs_mz_uint8 *)pZip->m_pState->m_central_dir.m_p);
      if (sort_central_dir)
        DRFS_MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_sorted_central_dir_offsets, drfs_drfs_mz_uint32, i) = i;
      comp_size = DRFS_MZ_READ_LE32(p + DRFS_MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);
      decomp_size = DRFS_MZ_READ_LE32(p + DRFS_MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS);
      if (((!DRFS_MZ_READ_LE32(p + DRFS_MZ_ZIP_CDH_METHOD_OFS)) && (decomp_size != comp_size)) || (decomp_size && !comp_size) || (decomp_size == 0xFFFFFFFF) || (comp_size == 0xFFFFFFFF))
        return DRFS_MZ_FALSE;
      disk_index = DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_DISK_START_OFS);
      if ((disk_index != num_this_disk) && (disk_index != 1))
        return DRFS_MZ_FALSE;
      if (((drfs_mz_uint64)DRFS_MZ_READ_LE32(p + DRFS_MZ_ZIP_CDH_LOCAL_HEADER_OFS) + DRFS_MZ_ZIP_LOCAL_DIR_HEADER_SIZE + comp_size) > pZip->m_archive_size)
        return DRFS_MZ_FALSE;
      if ((total_header_size = DRFS_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_FILENAME_LEN_OFS) + DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_EXTRA_LEN_OFS) + DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_COMMENT_LEN_OFS)) > n)
        return DRFS_MZ_FALSE;
      n -= total_header_size; p += total_header_size;
    }
  }

  if (sort_central_dir)
    drfs_mz_zip_reader_sort_central_dir_offsets_by_filename(pZip);

  return DRFS_MZ_TRUE;
}

drfs_mz_bool drfs_mz_zip_reader_init(drfs_mz_zip_archive *pZip, drfs_mz_uint64 size, drfs_drfs_mz_uint32 flags)
{
  if ((!pZip) || (!pZip->m_pRead))
    return DRFS_MZ_FALSE;
  if (!drfs_mz_zip_reader_init_internal(pZip, flags))
    return DRFS_MZ_FALSE;
  pZip->m_archive_size = size;
  if (!drfs_mz_zip_reader_read_central_dir(pZip, flags))
  {
    drfs_mz_zip_reader_end(pZip);
    return DRFS_MZ_FALSE;
  }
  return DRFS_MZ_TRUE;
}

drfs_mz_uint drfs_mz_zip_reader_get_num_files(drfs_mz_zip_archive *pZip)
{
  return pZip ? pZip->m_total_files : 0;
}

static DRFS_MZ_FORCEINLINE const drfs_mz_uint8 *drfs_mz_zip_reader_get_cdh(drfs_mz_zip_archive *pZip, drfs_mz_uint file_index)
{
  if ((!pZip) || (!pZip->m_pState) || (file_index >= pZip->m_total_files) || (pZip->m_zip_mode != DRFS_MZ_ZIP_MODE_READING))
    return NULL;
  return &DRFS_MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir, drfs_mz_uint8, DRFS_MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, drfs_drfs_mz_uint32, file_index));
}

drfs_mz_bool drfs_mz_zip_reader_is_file_a_directory(drfs_mz_zip_archive *pZip, drfs_mz_uint file_index)
{
  drfs_mz_uint filename_len, external_attr;
  const drfs_mz_uint8 *p = drfs_mz_zip_reader_get_cdh(pZip, file_index);
  if (!p)
    return DRFS_MZ_FALSE;

  // First see if the filename ends with a '/' character.
  filename_len = DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_FILENAME_LEN_OFS);
  if (filename_len)
  {
    if (*(p + DRFS_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_len - 1) == '/')
      return DRFS_MZ_TRUE;
  }

  // Bugfix: This code was also checking if the internal attribute was non-zero, which wasn't correct.
  // Most/all zip writers (hopefully) set DOS file/directory attributes in the low 16-bits, so check for the DOS directory flag and ignore the source OS ID in the created by field.
  // FIXME: Remove this check? Is it necessary - we already check the filename.
  external_attr = DRFS_MZ_READ_LE32(p + DRFS_MZ_ZIP_CDH_EXTERNAL_ATTR_OFS);
  if ((external_attr & 0x10) != 0)
    return DRFS_MZ_TRUE;

  return DRFS_MZ_FALSE;
}

drfs_mz_bool drfs_mz_zip_reader_file_stat(drfs_mz_zip_archive *pZip, drfs_mz_uint file_index, drfs_drfs_mz_zip_archive_file_stat *pStat)
{
  drfs_mz_uint n;
  const drfs_mz_uint8 *p = drfs_mz_zip_reader_get_cdh(pZip, file_index);
  if ((!p) || (!pStat))
    return DRFS_MZ_FALSE;

  // Unpack the central directory record.
  pStat->m_file_index = file_index;
  pStat->m_central_dir_ofs = DRFS_MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, drfs_drfs_mz_uint32, file_index);
  pStat->m_version_made_by = DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_VERSION_MADE_BY_OFS);
  pStat->m_version_needed = DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_VERSION_NEEDED_OFS);
  pStat->m_bit_flag = DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_BIT_FLAG_OFS);
  pStat->m_method = DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_METHOD_OFS);
#ifndef DRFS_MINIZ_NO_TIME
  pStat->m_time = drfs_mz_zip_dos_to_time_t(DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_FILE_TIME_OFS), DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_FILE_DATE_OFS));
#endif
  pStat->m_crc32 = DRFS_MZ_READ_LE32(p + DRFS_MZ_ZIP_CDH_CRC32_OFS);
  pStat->m_comp_size = DRFS_MZ_READ_LE32(p + DRFS_MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);
  pStat->m_uncomp_size = DRFS_MZ_READ_LE32(p + DRFS_MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS);
  pStat->m_internal_attr = DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_INTERNAL_ATTR_OFS);
  pStat->m_external_attr = DRFS_MZ_READ_LE32(p + DRFS_MZ_ZIP_CDH_EXTERNAL_ATTR_OFS);
  pStat->m_local_header_ofs = DRFS_MZ_READ_LE32(p + DRFS_MZ_ZIP_CDH_LOCAL_HEADER_OFS);

  // Copy as much of the filename and comment as possible.
  n = DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_FILENAME_LEN_OFS); n = DRFS_MZ_MIN(n, DRFS_MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE - 1);
  memcpy(pStat->m_filename, p + DRFS_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, n); pStat->m_filename[n] = '\0';

  n = DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_COMMENT_LEN_OFS); n = DRFS_MZ_MIN(n, DRFS_MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE - 1);
  pStat->m_comment_size = n;
  memcpy(pStat->m_comment, p + DRFS_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_FILENAME_LEN_OFS) + DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_EXTRA_LEN_OFS), n); pStat->m_comment[n] = '\0';

  return DRFS_MZ_TRUE;
}

drfs_mz_uint drfs_mz_zip_reader_get_filename(drfs_mz_zip_archive *pZip, drfs_mz_uint file_index, char *pFilename, drfs_mz_uint filename_buf_size)
{
  drfs_mz_uint n;
  const drfs_mz_uint8 *p = drfs_mz_zip_reader_get_cdh(pZip, file_index);
  if (!p) { if (filename_buf_size) pFilename[0] = '\0'; return 0; }
  n = DRFS_MZ_READ_LE16(p + DRFS_MZ_ZIP_CDH_FILENAME_LEN_OFS);
  if (filename_buf_size)
  {
    n = DRFS_MZ_MIN(n, filename_buf_size - 1);
    memcpy(pFilename, p + DRFS_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, n);
    pFilename[n] = '\0';
  }
  return n + 1;
}

static DRFS_MZ_FORCEINLINE drfs_mz_bool drfs_mz_zip_reader_string_equal(const char *pA, const char *pB, drfs_mz_uint len, drfs_mz_uint flags)
{
  drfs_mz_uint i;
  if (flags & DRFS_MZ_ZIP_FLAG_CASE_SENSITIVE)
    return 0 == memcmp(pA, pB, len);
  for (i = 0; i < len; ++i)
    if (DRFS_MZ_TOLOWER(pA[i]) != DRFS_MZ_TOLOWER(pB[i]))
      return DRFS_MZ_FALSE;
  return DRFS_MZ_TRUE;
}

static DRFS_MZ_FORCEINLINE int drfs_mz_zip_reader_filename_compare(const drfs_mz_zip_array *pCentral_dir_array, const drfs_mz_zip_array *pCentral_dir_offsets, drfs_mz_uint l_index, const char *pR, drfs_mz_uint r_len)
{
  const drfs_mz_uint8 *pL = &DRFS_MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_array, drfs_mz_uint8, DRFS_MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, drfs_drfs_mz_uint32, l_index)), *pE;
  drfs_mz_uint l_len = DRFS_MZ_READ_LE16(pL + DRFS_MZ_ZIP_CDH_FILENAME_LEN_OFS);
  drfs_mz_uint8 l = 0, r = 0;
  pL += DRFS_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
  pE = pL + DRFS_MZ_MIN(l_len, r_len);
  while (pL < pE)
  {
    if ((l = DRFS_MZ_TOLOWER(*pL)) != (r = DRFS_MZ_TOLOWER(*pR)))
      break;
    pL++; pR++;
  }
  return (pL == pE) ? (int)(l_len - r_len) : (l - r);
}

static int drfs_mz_zip_reader_locate_file_binary_search(drfs_mz_zip_archive *pZip, const char *pFilename)
{
  drfs_mz_zip_internal_state *pState = pZip->m_pState;
  const drfs_mz_zip_array *pCentral_dir_offsets = &pState->m_central_dir_offsets;
  const drfs_mz_zip_array *pCentral_dir = &pState->m_central_dir;
  drfs_drfs_mz_uint32 *pIndices = &DRFS_MZ_ZIP_ARRAY_ELEMENT(&pState->m_sorted_central_dir_offsets, drfs_drfs_mz_uint32, 0);
  const int size = pZip->m_total_files;
  const drfs_mz_uint filename_len = (drfs_mz_uint)strlen(pFilename);
  int l = 0, h = size - 1;
  while (l <= h)
  {
    int m = (l + h) >> 1, file_index = pIndices[m], comp = drfs_mz_zip_reader_filename_compare(pCentral_dir, pCentral_dir_offsets, file_index, pFilename, filename_len);
    if (!comp)
      return file_index;
    else if (comp < 0)
      l = m + 1;
    else
      h = m - 1;
  }
  return -1;
}

int drfs_mz_zip_reader_locate_file(drfs_mz_zip_archive *pZip, const char *pName, const char *pComment, drfs_mz_uint flags)
{
  drfs_mz_uint file_index; size_t name_len, comment_len;
  if ((!pZip) || (!pZip->m_pState) || (!pName) || (pZip->m_zip_mode != DRFS_MZ_ZIP_MODE_READING))
    return -1;
  if (((flags & (DRFS_MZ_ZIP_FLAG_IGNORE_PATH | DRFS_MZ_ZIP_FLAG_CASE_SENSITIVE)) == 0) && (!pComment) && (pZip->m_pState->m_sorted_central_dir_offsets.m_size))
    return drfs_mz_zip_reader_locate_file_binary_search(pZip, pName);
  name_len = strlen(pName); if (name_len > 0xFFFF) return -1;
  comment_len = pComment ? strlen(pComment) : 0; if (comment_len > 0xFFFF) return -1;
  for (file_index = 0; file_index < pZip->m_total_files; file_index++)
  {
    const drfs_mz_uint8 *pHeader = &DRFS_MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir, drfs_mz_uint8, DRFS_MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, drfs_drfs_mz_uint32, file_index));
    drfs_mz_uint filename_len = DRFS_MZ_READ_LE16(pHeader + DRFS_MZ_ZIP_CDH_FILENAME_LEN_OFS);
    const char *pFilename = (const char *)pHeader + DRFS_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
    if (filename_len < name_len)
      continue;
    if (comment_len)
    {
      drfs_mz_uint file_extra_len = DRFS_MZ_READ_LE16(pHeader + DRFS_MZ_ZIP_CDH_EXTRA_LEN_OFS), file_comment_len = DRFS_MZ_READ_LE16(pHeader + DRFS_MZ_ZIP_CDH_COMMENT_LEN_OFS);
      const char *pFile_comment = pFilename + filename_len + file_extra_len;
      if ((file_comment_len != comment_len) || (!drfs_mz_zip_reader_string_equal(pComment, pFile_comment, file_comment_len, flags)))
        continue;
    }
    if ((flags & DRFS_MZ_ZIP_FLAG_IGNORE_PATH) && (filename_len))
    {
      int ofs = filename_len - 1;
      do
      {
        if ((pFilename[ofs] == '/') || (pFilename[ofs] == '\\') || (pFilename[ofs] == ':'))
          break;
      } while (--ofs >= 0);
      ofs++;
      pFilename += ofs; filename_len -= ofs;
    }
    if ((filename_len == name_len) && (drfs_mz_zip_reader_string_equal(pName, pFilename, filename_len, flags)))
      return file_index;
  }
  return -1;
}

drfs_mz_bool drfs_mz_zip_reader_extract_to_mem_no_alloc(drfs_mz_zip_archive *pZip, drfs_mz_uint file_index, void *pBuf, size_t buf_size, drfs_mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size)
{
  int status = DRFS_TINFL_STATUS_DONE;
  drfs_mz_uint64 needed_size, cur_file_ofs, comp_remaining, out_buf_ofs = 0, read_buf_size, read_buf_ofs = 0, read_buf_avail;
  drfs_drfs_mz_zip_archive_file_stat file_stat;
  void *pRead_buf;
  drfs_drfs_mz_uint32 local_header_u32[(DRFS_MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(drfs_drfs_mz_uint32) - 1) / sizeof(drfs_drfs_mz_uint32)]; drfs_mz_uint8 *pLocal_header = (drfs_mz_uint8 *)local_header_u32;
  drfs_tinfl_decompressor inflator;

  if ((buf_size) && (!pBuf))
    return DRFS_MZ_FALSE;

  if (!drfs_mz_zip_reader_file_stat(pZip, file_index, &file_stat))
    return DRFS_MZ_FALSE;

  // Empty file, or a directory (but not always a directory - I've seen odd zips with directories that have compressed data which inflates to 0 bytes)
  if (!file_stat.m_comp_size)
    return DRFS_MZ_TRUE;

  // Entry is a subdirectory (I've seen old zips with dir entries which have compressed deflate data which inflates to 0 bytes, but these entries claim to uncompress to 512 bytes in the headers).
  // I'm torn how to handle this case - should it fail instead?
  if (drfs_mz_zip_reader_is_file_a_directory(pZip, file_index))
    return DRFS_MZ_TRUE;

  // Encryption and patch files are not supported.
  if (file_stat.m_bit_flag & (1 | 32))
    return DRFS_MZ_FALSE;

  // This function only supports stored and deflate.
  if ((!(flags & DRFS_MZ_ZIP_FLAG_COMPRESSED_DATA)) && (file_stat.m_method != 0) && (file_stat.m_method != DRFS_MZ_DEFLATED))
    return DRFS_MZ_FALSE;

  // Ensure supplied output buffer is large enough.
  needed_size = (flags & DRFS_MZ_ZIP_FLAG_COMPRESSED_DATA) ? file_stat.m_comp_size : file_stat.m_uncomp_size;
  if (buf_size < needed_size)
    return DRFS_MZ_FALSE;

  // Read and parse the local directory entry.
  cur_file_ofs = file_stat.m_local_header_ofs;
  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pLocal_header, DRFS_MZ_ZIP_LOCAL_DIR_HEADER_SIZE) != DRFS_MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
    return DRFS_MZ_FALSE;
  if (DRFS_MZ_READ_LE32(pLocal_header) != DRFS_MZ_ZIP_LOCAL_DIR_HEADER_SIG)
    return DRFS_MZ_FALSE;

  cur_file_ofs += DRFS_MZ_ZIP_LOCAL_DIR_HEADER_SIZE + DRFS_MZ_READ_LE16(pLocal_header + DRFS_MZ_ZIP_LDH_FILENAME_LEN_OFS) + DRFS_MZ_READ_LE16(pLocal_header + DRFS_MZ_ZIP_LDH_EXTRA_LEN_OFS);
  if ((cur_file_ofs + file_stat.m_comp_size) > pZip->m_archive_size)
    return DRFS_MZ_FALSE;

  if ((flags & DRFS_MZ_ZIP_FLAG_COMPRESSED_DATA) || (!file_stat.m_method))
  {
    // The file is stored or the caller has requested the compressed data.
    if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pBuf, (size_t)needed_size) != needed_size)
      return DRFS_MZ_FALSE;
    return ((flags & DRFS_MZ_ZIP_FLAG_COMPRESSED_DATA) != 0) || (drfs_mz_crc32(DRFS_MZ_CRC32_INIT, (const drfs_mz_uint8 *)pBuf, (size_t)file_stat.m_uncomp_size) == file_stat.m_crc32);
  }

  // Decompress the file either directly from memory or from a file input buffer.
  drfs_tinfl_init(&inflator);

  if (pZip->m_pState->m_pMem)
  {
    // Read directly from the archive in memory.
    pRead_buf = (drfs_mz_uint8 *)pZip->m_pState->m_pMem + cur_file_ofs;
    read_buf_size = read_buf_avail = file_stat.m_comp_size;
    comp_remaining = 0;
  }
  else if (pUser_read_buf)
  {
    // Use a user provided read buffer.
    if (!user_read_buf_size)
      return DRFS_MZ_FALSE;
    pRead_buf = (drfs_mz_uint8 *)pUser_read_buf;
    read_buf_size = user_read_buf_size;
    read_buf_avail = 0;
    comp_remaining = file_stat.m_comp_size;
  }
  else
  {
    // Temporarily allocate a read buffer.
    read_buf_size = DRFS_MZ_MIN(file_stat.m_comp_size, DRFS_MZ_ZIP_MAX_IO_BUF_SIZE);
#ifdef _MSC_VER
    if (((0, sizeof(size_t) == sizeof(drfs_drfs_mz_uint32))) && (read_buf_size > 0x7FFFFFFF))
#else
    if (((sizeof(size_t) == sizeof(drfs_drfs_mz_uint32))) && (read_buf_size > 0x7FFFFFFF))
#endif
      return DRFS_MZ_FALSE;
    if (NULL == (pRead_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)read_buf_size)))
      return DRFS_MZ_FALSE;
    read_buf_avail = 0;
    comp_remaining = file_stat.m_comp_size;
  }

  do
  {
    size_t in_buf_size, out_buf_size = (size_t)(file_stat.m_uncomp_size - out_buf_ofs);
    if ((!read_buf_avail) && (!pZip->m_pState->m_pMem))
    {
      read_buf_avail = DRFS_MZ_MIN(read_buf_size, comp_remaining);
      if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pRead_buf, (size_t)read_buf_avail) != read_buf_avail)
      {
        status = DRFS_TINFL_STATUS_FAILED;
        break;
      }
      cur_file_ofs += read_buf_avail;
      comp_remaining -= read_buf_avail;
      read_buf_ofs = 0;
    }
    in_buf_size = (size_t)read_buf_avail;
    status = drfs_tinfl_decompress(&inflator, (drfs_mz_uint8 *)pRead_buf + read_buf_ofs, &in_buf_size, (drfs_mz_uint8 *)pBuf, (drfs_mz_uint8 *)pBuf + out_buf_ofs, &out_buf_size, DRFS_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF | (comp_remaining ? DRFS_TINFL_FLAG_HAS_MORE_INPUT : 0));
    read_buf_avail -= in_buf_size;
    read_buf_ofs += in_buf_size;
    out_buf_ofs += out_buf_size;
  } while (status == DRFS_TINFL_STATUS_NEEDS_MORE_INPUT);

  if (status == DRFS_TINFL_STATUS_DONE)
  {
    // Make sure the entire file was decompressed, and check its CRC.
    if ((out_buf_ofs != file_stat.m_uncomp_size) || (drfs_mz_crc32(DRFS_MZ_CRC32_INIT, (const drfs_mz_uint8 *)pBuf, (size_t)file_stat.m_uncomp_size) != file_stat.m_crc32))
      status = DRFS_TINFL_STATUS_FAILED;
  }

  if ((!pZip->m_pState->m_pMem) && (!pUser_read_buf))
    pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);

  return status == DRFS_TINFL_STATUS_DONE;
}

drfs_mz_bool drfs_mz_zip_reader_extract_file_to_mem_no_alloc(drfs_mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, drfs_mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size)
{
  int file_index = drfs_mz_zip_reader_locate_file(pZip, pFilename, NULL, flags);
  if (file_index < 0)
    return DRFS_MZ_FALSE;
  return drfs_mz_zip_reader_extract_to_mem_no_alloc(pZip, file_index, pBuf, buf_size, flags, pUser_read_buf, user_read_buf_size);
}

drfs_mz_bool drfs_mz_zip_reader_extract_to_mem(drfs_mz_zip_archive *pZip, drfs_mz_uint file_index, void *pBuf, size_t buf_size, drfs_mz_uint flags)
{
  return drfs_mz_zip_reader_extract_to_mem_no_alloc(pZip, file_index, pBuf, buf_size, flags, NULL, 0);
}

drfs_mz_bool drfs_mz_zip_reader_extract_file_to_mem(drfs_mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, drfs_mz_uint flags)
{
  return drfs_mz_zip_reader_extract_file_to_mem_no_alloc(pZip, pFilename, pBuf, buf_size, flags, NULL, 0);
}

void *drfs_mz_zip_reader_extract_to_heap(drfs_mz_zip_archive *pZip, drfs_mz_uint file_index, size_t *pSize, drfs_mz_uint flags)
{
  drfs_mz_uint64 comp_size, uncomp_size, alloc_size;
  const drfs_mz_uint8 *p = drfs_mz_zip_reader_get_cdh(pZip, file_index);
  void *pBuf;

  if (pSize)
    *pSize = 0;
  if (!p)
    return NULL;

  comp_size = DRFS_MZ_READ_LE32(p + DRFS_MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);
  uncomp_size = DRFS_MZ_READ_LE32(p + DRFS_MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS);

  alloc_size = (flags & DRFS_MZ_ZIP_FLAG_COMPRESSED_DATA) ? comp_size : uncomp_size;
#ifdef _MSC_VER
  if (((0, sizeof(size_t) == sizeof(drfs_drfs_mz_uint32))) && (alloc_size > 0x7FFFFFFF))
#else
  if (((sizeof(size_t) == sizeof(drfs_drfs_mz_uint32))) && (alloc_size > 0x7FFFFFFF))
#endif
    return NULL;
  if (NULL == (pBuf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)alloc_size)))
    return NULL;

  if (!drfs_mz_zip_reader_extract_to_mem(pZip, file_index, pBuf, (size_t)alloc_size, flags))
  {
    pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
    return NULL;
  }

  if (pSize) *pSize = (size_t)alloc_size;
  return pBuf;
}

drfs_mz_bool drfs_mz_zip_reader_end(drfs_mz_zip_archive *pZip)
{
  if ((!pZip) || (!pZip->m_pState) || (!pZip->m_pAlloc) || (!pZip->m_pFree) || (pZip->m_zip_mode != DRFS_MZ_ZIP_MODE_READING))
    return DRFS_MZ_FALSE;

  if (pZip->m_pState)
  {
    drfs_mz_zip_internal_state *pState = pZip->m_pState; pZip->m_pState = NULL;
    drfs_mz_zip_array_clear(pZip, &pState->m_central_dir);
    drfs_mz_zip_array_clear(pZip, &pState->m_central_dir_offsets);
    drfs_mz_zip_array_clear(pZip, &pState->m_sorted_central_dir_offsets);

    pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
  }
  pZip->m_zip_mode = DRFS_MZ_ZIP_MODE_INVALID;

  return DRFS_MZ_TRUE;
}

#endif // #ifndef DRFS_MINIZ_NO_ARCHIVE_APIS

#ifdef __cplusplus
}
#endif

#endif // DRFS_MINIZ_HEADER_FILE_ONLY

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
#if defined(_MSC_VER)
    #pragma warning(pop)
#endif




typedef struct
{
    // The current index of the iterator. When this hits the file count, the iteration is finished.
    unsigned int index;

    // The directory being iterated.
    char directoryPath[DRFS_MAX_PATH];

}drfs_iterator_zip;

typedef struct
{
    // The file index within the archive.
    drfs_mz_uint index;

    // A pointer to the buffer containing the entire uncompressed data of the file. Unfortunately this is the only way I'm aware of for
    // reading file data from miniz.c so we'll just stick with it for now. We use a pointer to an 8-bit type so we can easily calculate
    // offsets.
    drfs_mz_uint8* pData;

    // The size of the file in bytes so we can guard against overflowing reads.
    size_t sizeInBytes;

    // The current position of the file's read pointer.
    size_t readPointer;

}drfs_openedfile_zip;

static size_t drfs_drfs_mz_file_read_func(void *pOpaque, drfs_mz_uint64 file_ofs, void *pBuf, size_t n)
{
    // The opaque type is a pointer to a drfs_file object which represents the file of the archive.
    drfs_file* pZipFile = (drfs_file*)pOpaque;
    assert(pZipFile != NULL);

    if (!drfs_lock(pZipFile)) {
        return 0;
    }

    drfs_seek_nolock(pZipFile, (dr_int64)file_ofs, drfs_origin_start);

    size_t bytesRead;
    drfs_result result = drfs_read_nolock(pZipFile, pBuf, (unsigned int)n, &bytesRead);
    if (result != drfs_success) {
        // Failed to read the file.
        bytesRead = 0;
    }

    drfs_unlock(pZipFile);
    return (size_t)bytesRead;
}


static dr_bool32 drfs_is_valid_extension__zip(const char* extension)
{
    return drfs__stricmp(extension, "zip") == 0;
}

static drfs_result drfs_open_archive__zip(drfs_file* pArchiveFile, unsigned int accessMode, drfs_handle* pHandleOut)
{
    assert(pArchiveFile != NULL);
    assert(pHandleOut != NULL);
    assert(drfs_tell(pArchiveFile) == 0);

    *pHandleOut = NULL;

    // Only support read-only mode at the moment.
    if ((accessMode & DRFS_WRITE) != 0) {
        return drfs_permission_denied;
    }


    drfs_mz_zip_archive* pZip = (drfs_mz_zip_archive*)malloc(sizeof(drfs_mz_zip_archive));
    if (pZip == NULL) {
        return drfs_out_of_memory;
    }

    memset(pZip, 0, sizeof(drfs_mz_zip_archive));

    pZip->m_pRead = drfs_drfs_mz_file_read_func;
    pZip->m_pIO_opaque = pArchiveFile;
    if (!drfs_mz_zip_reader_init(pZip, drfs_size(pArchiveFile), 0)) {
        free(pZip);
        return drfs_invalid_archive;
    }

    *pHandleOut = pZip;
    return drfs_success;
}

static void drfs_close_archive__zip(drfs_handle archive)
{
    assert(archive != NULL);

    drfs_mz_zip_reader_end((drfs_mz_zip_archive*)archive);
    free(archive);
}

static drfs_result drfs_get_file_info__zip(drfs_handle archive, const char* relativePath, drfs_file_info* fi)
{
    assert(archive != NULL);

    drfs_mz_zip_archive* pZip = (drfs_mz_zip_archive*)archive;
    int fileIndex = drfs_mz_zip_reader_locate_file(pZip, relativePath, NULL, DRFS_MZ_ZIP_FLAG_CASE_SENSITIVE);
    if (fileIndex == -1)
    {
        // We failed to locate the file, but there's a chance it could actually be a folder. Here's the problem - folders
        // can be named such that they include a trailing slash. We'll want to check for that. Another problem is that
        // sometimes the folders won't actually be included in the central directory at all which means we need to do a
        // manual check across every file in the archive.
        char relativePathWithSlash[DRFS_MAX_PATH];
        drfs__strcpy_s(relativePathWithSlash, sizeof(relativePathWithSlash), relativePath);
        drfs__strcat_s(relativePathWithSlash, sizeof(relativePathWithSlash), "/");
        fileIndex = drfs_mz_zip_reader_locate_file(pZip, relativePath, NULL, DRFS_MZ_ZIP_FLAG_CASE_SENSITIVE);
        if (fileIndex == -1)
        {
            // We still couldn't find the directory even with the trailing slash. There's a chace it's a folder that's
            // simply not included in the central directory. It's appears the "Send to -> Compressed (zipped) folder"
            // functionality in Windows does this.
            drfs_mz_uint numFiles = drfs_mz_zip_reader_get_num_files(pZip);
            for (drfs_mz_uint iFile = 0; iFile < numFiles; ++iFile)
            {
                char filePath[DRFS_MAX_PATH];
                if (drfs_mz_zip_reader_get_filename(pZip, iFile, filePath, DRFS_MAX_PATH) > 0)
                {
                    if (drfs_drpath_is_child(filePath, relativePath))
                    {
                        // This file is within a folder with a path of relativePath which means we can imply that relativePath
                        // is a folder.
                        drfs__strcpy_s(fi->absolutePath, sizeof(fi->absolutePath), relativePath);
                        fi->sizeInBytes      = 0;
                        fi->lastModifiedTime = 0;
                        fi->attributes       = DRFS_FILE_ATTRIBUTE_READONLY | DRFS_FILE_ATTRIBUTE_DIRECTORY;

                        return drfs_success;
                    }
                }
            }

            return drfs_does_not_exist;
        }
    }

    assert(fileIndex != -1);

    if (fi != NULL)
    {
        drfs_drfs_mz_zip_archive_file_stat zipStat;
        if (drfs_mz_zip_reader_file_stat(pZip, (drfs_mz_uint)fileIndex, &zipStat))
        {
            drfs__strcpy_s(fi->absolutePath, sizeof(fi->absolutePath), relativePath);
            fi->sizeInBytes      = zipStat.m_uncomp_size;
            fi->lastModifiedTime = (dr_uint64)zipStat.m_time;
            fi->attributes       = DRFS_FILE_ATTRIBUTE_READONLY;
            if (drfs_mz_zip_reader_is_file_a_directory(pZip, (drfs_mz_uint)fileIndex)) {
                fi->attributes |= DRFS_FILE_ATTRIBUTE_DIRECTORY;
            }

            return drfs_success;
        }
    }

    return drfs_success;
}

static drfs_handle drfs_begin_iteration__zip(drfs_handle archive, const char* relativePath)
{
    assert(relativePath != NULL);

    drfs_mz_zip_archive* pZip = (drfs_mz_zip_archive*)archive;
    assert(pZip != NULL);

    int directoryFileIndex = -1;
    if (relativePath[0] == '\0') {
        directoryFileIndex = 0;
    } else {
        directoryFileIndex = drfs_mz_zip_reader_locate_file(pZip, relativePath, NULL, DRFS_MZ_ZIP_FLAG_CASE_SENSITIVE);
    }

    if (directoryFileIndex == -1)
    {
        // The same issue applies here as documented in drfs_get_file_info__zip().
        char relativePathWithSlash[DRFS_MAX_PATH];
        drfs__strcpy_s(relativePathWithSlash, sizeof(relativePathWithSlash), relativePath);
        drfs__strcat_s(relativePathWithSlash, sizeof(relativePathWithSlash), "/");
        directoryFileIndex = drfs_mz_zip_reader_locate_file(pZip, relativePath, NULL, DRFS_MZ_ZIP_FLAG_CASE_SENSITIVE);
        if (directoryFileIndex == -1)
        {
            // We still couldn't find the directory even with the trailing slash. There's a chace it's a folder that's
            // simply not included in the central directory. It's appears the "Send to -> Compressed (zipped) folder"
            // functionality in Windows does this.
            drfs_mz_uint numFiles = drfs_mz_zip_reader_get_num_files(pZip);
            for (drfs_mz_uint iFile = 0; iFile < numFiles; ++iFile)
            {
                char filePath[DRFS_MAX_PATH];
                if (drfs_mz_zip_reader_get_filename(pZip, iFile, filePath, DRFS_MAX_PATH) > 0)
                {
                    if (drfs_drpath_is_child(filePath, relativePath))
                    {
                        // This file is within a folder with a path of relativePath which means we can imply that relativePath
                        // is a folder.
                        goto on_success;
                    }
                }
            }

            return NULL;
        }
    }



on_success:;
    drfs_iterator_zip* pZipIterator = (drfs_iterator_zip*)malloc(sizeof(drfs_iterator_zip));
    if (pZipIterator != NULL)
    {
        pZipIterator->index = 0;
        drfs__strcpy_s(pZipIterator->directoryPath, sizeof(pZipIterator->directoryPath), relativePath);
    }

    return pZipIterator;
}

static void drfs_end_iteration__zip(drfs_handle archive, drfs_handle iterator)
{
    (void)archive;
    assert(archive != NULL);
    assert(iterator != NULL);

    free(iterator);
}

static dr_bool32 drfs_next_iteration__zip(drfs_handle archive, drfs_handle iterator, drfs_file_info* fi)
{
    (void)archive;
    assert(archive != NULL);
    assert(iterator != NULL);

    drfs_iterator_zip* pZipIterator = (drfs_iterator_zip*)iterator;
    if (pZipIterator == NULL) {
        return DR_FALSE;
    }

    drfs_mz_zip_archive* pZip = (drfs_mz_zip_archive*)archive;
    while (pZipIterator->index < drfs_mz_zip_reader_get_num_files(pZip))
    {
        unsigned int iFile = pZipIterator->index++;

        char filePath[DRFS_MAX_PATH];
        if (drfs_mz_zip_reader_get_filename(pZip, iFile, filePath, DRFS_MAX_PATH) > 0)
        {
            if (drfs_drpath_is_child(filePath, pZipIterator->directoryPath))
            {
                if (fi != NULL)
                {
                    drfs_drfs_mz_zip_archive_file_stat zipStat;
                    if (drfs_mz_zip_reader_file_stat(pZip, iFile, &zipStat))
                    {
                        drfs__strcpy_s(fi->absolutePath, sizeof(fi->absolutePath), filePath);
                        fi->sizeInBytes      = zipStat.m_uncomp_size;
                        fi->lastModifiedTime = (dr_uint64)zipStat.m_time;
                        fi->attributes       = DRFS_FILE_ATTRIBUTE_READONLY;
                        if (drfs_mz_zip_reader_is_file_a_directory(pZip, iFile)) {
                            fi->attributes |= DRFS_FILE_ATTRIBUTE_DIRECTORY;
                        }

                        // If we have a directory we need to ensure we don't have a trailing slash.
                        if ((fi->attributes & DRFS_FILE_ATTRIBUTE_DIRECTORY) != 0) {
                            size_t absolutePathLen = strlen(fi->absolutePath);
                            if (absolutePathLen > 0 && (fi->absolutePath[absolutePathLen - 1] == '/' || fi->absolutePath[absolutePathLen - 1] == '\\')) {
                                fi->absolutePath[absolutePathLen - 1] = '\0';
                            }
                        }
                    }
                }

                return DR_TRUE;
            }
        }
    }

    return DR_FALSE;
}

static drfs_result drfs_open_file__zip(drfs_handle archive, const char* relativePath, unsigned int accessMode, drfs_handle* pHandleOut)
{
    assert(archive != NULL);
    assert(pHandleOut != NULL);
    assert(relativePath != NULL);

    *pHandleOut = NULL;

    // Only supporting read-only for now.
    if ((accessMode & DRFS_WRITE) != 0) {
        return drfs_permission_denied;
    }


    drfs_mz_zip_archive* pZip = (drfs_mz_zip_archive*)archive;
    int fileIndex = drfs_mz_zip_reader_locate_file(pZip, relativePath, NULL, DRFS_MZ_ZIP_FLAG_CASE_SENSITIVE);
    if (fileIndex == -1) {
        return drfs_does_not_exist;
    }

    drfs_openedfile_zip* pOpenedFile = (drfs_openedfile_zip*)malloc(sizeof(*pOpenedFile));
    if (pOpenedFile == NULL) {
        return drfs_out_of_memory;
    }

    pOpenedFile->pData = (drfs_mz_uint8*)drfs_mz_zip_reader_extract_to_heap(pZip, (drfs_mz_uint)fileIndex, &pOpenedFile->sizeInBytes, 0);
    if (pOpenedFile->pData == NULL) {
        free(pOpenedFile);
        return drfs_unknown_error;
    }

    pOpenedFile->index = (drfs_mz_uint)fileIndex;
    pOpenedFile->readPointer = 0;

    *pHandleOut = pOpenedFile;
    return drfs_success;
}

static void drfs_close_file__zip(drfs_handle archive, drfs_handle file)
{
    drfs_openedfile_zip* pOpenedFile = (drfs_openedfile_zip*)file;
    assert(pOpenedFile != NULL);

    drfs_mz_zip_archive* pZip = (drfs_mz_zip_archive*)archive;
    assert(pZip != NULL);

    pZip->m_pFree(pZip->m_pAlloc_opaque, pOpenedFile->pData);
    free(pOpenedFile);
}

static drfs_result drfs_read_file__zip(drfs_handle archive, drfs_handle file, void* pDataOut, size_t bytesToRead, size_t* pBytesReadOut)
{
    (void)archive;
    assert(archive != NULL);
    assert(file != NULL);
    assert(pDataOut != NULL);
    assert(bytesToRead > 0);

    drfs_openedfile_zip* pOpenedFile = (drfs_openedfile_zip*)file;
    if (pOpenedFile == NULL) {
        return drfs_invalid_args;
    }

    size_t bytesAvailable = pOpenedFile->sizeInBytes - pOpenedFile->readPointer;
    if (bytesAvailable < bytesToRead) {
        bytesToRead = bytesAvailable;
    }

    if (bytesToRead == 0) {
        return drfs_at_end_of_file;   // Nothing left to read.
    }


    memcpy(pDataOut, pOpenedFile->pData + pOpenedFile->readPointer, bytesToRead);
    pOpenedFile->readPointer += bytesToRead;

    if (pBytesReadOut) {
        *pBytesReadOut = bytesToRead;
    }

    return drfs_success;
}

static drfs_result drfs_write_file__zip(drfs_handle archive, drfs_handle file, const void* pData, size_t bytesToWrite, size_t* pBytesWrittenOut)
{
    (void)archive;
    (void)file;
    (void)pData;
    (void)bytesToWrite;

    assert(archive != NULL);
    assert(file != NULL);
    assert(pData != NULL);
    assert(bytesToWrite > 0);

    // All files are read-only for now.
    if (pBytesWrittenOut) {
        *pBytesWrittenOut = 0;
    }

    return drfs_permission_denied;
}

static drfs_result drfs_seek_file__zip(drfs_handle archive, drfs_handle file, dr_int64 bytesToSeek, drfs_seek_origin origin)
{
    (void)archive;

    assert(archive != NULL);
    assert(file != NULL);

    drfs_openedfile_zip* pOpenedFile = (drfs_openedfile_zip*)file;
    if (pOpenedFile == NULL) {
        return drfs_invalid_args;
    }

    dr_uint64 newPos = pOpenedFile->readPointer;
    if (origin == drfs_origin_current)
    {
        if ((dr_int64)newPos + bytesToSeek >= 0)
        {
            newPos = (dr_uint64)((dr_int64)newPos + bytesToSeek);
        }
        else
        {
            // Trying to seek to before the beginning of the file.
            return drfs_invalid_args;
        }
    }
    else if (origin == drfs_origin_start)
    {
        assert(bytesToSeek >= 0);
        newPos = (dr_uint64)bytesToSeek;
    }
    else if (origin == drfs_origin_end)
    {
        assert(bytesToSeek >= 0);
        if ((dr_uint64)bytesToSeek <= pOpenedFile->sizeInBytes)
        {
            newPos = pOpenedFile->sizeInBytes - (dr_uint64)bytesToSeek;
        }
        else
        {
            // Trying to seek to before the beginning of the file.
            return drfs_invalid_args;
        }
    }
    else
    {
        // Should never get here.
        return drfs_unknown_error;
    }


    if (newPos > pOpenedFile->sizeInBytes) {
        return drfs_invalid_args;
    }

    pOpenedFile->readPointer = (size_t)newPos;
    return drfs_success;
}

static dr_uint64 drfs_tell_file__zip(drfs_handle archive, drfs_handle file)
{
    (void)archive;

    drfs_openedfile_zip* pOpenedFile = (drfs_openedfile_zip*)file;
    assert(pOpenedFile != NULL);

    return pOpenedFile->readPointer;
}

static dr_uint64 drfs_file_size__zip(drfs_handle archive, drfs_handle file)
{
    (void)archive;

    drfs_openedfile_zip* pOpenedFile = (drfs_openedfile_zip*)file;
    assert(pOpenedFile != NULL);

    return pOpenedFile->sizeInBytes;
}

static void drfs_flush__zip(drfs_handle archive, drfs_handle file)
{
    (void)archive;
    (void)file;

    assert(archive != NULL);
    assert(file != NULL);

    // All files are read-only for now.
}


static void drfs_register_zip_backend(drfs_context* pContext)
{
    if (pContext == NULL) {
        return;
    }

    drfs_archive_callbacks callbacks;
    callbacks.is_valid_extension = drfs_is_valid_extension__zip;
    callbacks.open_archive       = drfs_open_archive__zip;
    callbacks.close_archive      = drfs_close_archive__zip;
    callbacks.get_file_info      = drfs_get_file_info__zip;
    callbacks.begin_iteration    = drfs_begin_iteration__zip;
    callbacks.end_iteration      = drfs_end_iteration__zip;
    callbacks.next_iteration     = drfs_next_iteration__zip;
    callbacks.delete_file        = NULL;
    callbacks.move_file          = NULL;
    callbacks.create_directory   = NULL;
    callbacks.copy_file          = NULL;
    callbacks.open_file          = drfs_open_file__zip;
    callbacks.close_file         = drfs_close_file__zip;
    callbacks.read_file          = drfs_read_file__zip;
    callbacks.write_file         = drfs_write_file__zip;
    callbacks.seek_file          = drfs_seek_file__zip;
    callbacks.tell_file          = drfs_tell_file__zip;
    callbacks.file_size          = drfs_file_size__zip;
    callbacks.flush_file         = drfs_flush__zip;
    drfs_register_archive_backend(pContext, callbacks);
}
#endif  //DR_FS_NO_ZIP



///////////////////////////////////////////////////////////////////////////////
//
// Quake 2 PAK
//
///////////////////////////////////////////////////////////////////////////////
#ifndef DR_FS_NO_PAK
typedef struct
{
    char path[64];

}drfs_path_pak;

typedef struct
{
    // The file name.
    char name[56];

    // The position within the file of the first byte of the file.
    unsigned int offset;

    // The size of the file, in bytes.
    unsigned int sizeInBytes;

}drfs_file_pak;

typedef struct
{
    // A pointer to the archive file for reading data.
    drfs_file* pArchiveFile;


    // The 4-byte identifiers: "PACK"
    char id[4];

    // The offset of the directory.
    unsigned int directoryOffset;

    // The size of the directory. This should a multiple of 64.
    unsigned int directoryLength;


    // The access mode.
    unsigned int accessMode;

    // A pointer to the buffer containing the file information. The number of items in this array is equal to directoryLength / 64.
    drfs_file_pak* pFiles;

}drfs_archive_pak;


typedef struct
{
    // The current index of the iterator. When this hits childCount, the iteration is finished.
    unsigned int index;

    // The directory being iterated.
    char directoryPath[DRFS_MAX_PATH];


    // The number of directories that have previously been iterated.
    unsigned int processedDirCount;

    // The directories that were previously iterated.
    drfs_path_pak* pProcessedDirs;

}drfs_iterator_pak;

static dr_bool32 drfs_iterator_pak_append_processed_dir(drfs_iterator_pak* pIterator, const char* path)
{
    if (pIterator != NULL && path != NULL)
    {
        drfs_path_pak* pOldBuffer = pIterator->pProcessedDirs;
        drfs_path_pak* pNewBuffer = (drfs_path_pak*)malloc(sizeof(drfs_path_pak) * (pIterator->processedDirCount + 1));

        if (pNewBuffer != 0)
        {
            for (unsigned int iDst = 0; iDst < pIterator->processedDirCount; ++iDst)
            {
                pNewBuffer[iDst] = pOldBuffer[iDst];
            }

            drfs__strcpy_s(pNewBuffer[pIterator->processedDirCount].path, 64, path);


            pIterator->pProcessedDirs     = pNewBuffer;
            pIterator->processedDirCount += 1;

            drfs_free(pOldBuffer);

            return 1;
        }
    }

    return 0;
}

static dr_bool32 drfs_iterator_pak_has_dir_been_processed(drfs_iterator_pak* pIterator, const char* path)
{
    for (unsigned int i = 0; i < pIterator->processedDirCount; ++i)
    {
        if (strcmp(path, pIterator->pProcessedDirs[i].path) == 0)
        {
            return 1;
        }
    }

    return 0;
}


typedef struct
{
    // The offset of the first byte of the file's data.
    size_t offsetInArchive;

    // The size of the file in bytes so we can guard against overflowing reads.
    size_t sizeInBytes;

    // The current position of the file's read pointer.
    size_t readPointer;

}drfs_openedfile_pak;



static drfs_archive_pak* drfs_pak_create(drfs_file* pArchiveFile, unsigned int accessMode)
{
    drfs_archive_pak* pak = (drfs_archive_pak*)malloc(sizeof(*pak));
    if (pak != NULL)
    {
        pak->pArchiveFile    = pArchiveFile;
        pak->directoryOffset = 0;
        pak->directoryLength = 0;
        pak->accessMode      = accessMode;
        pak->pFiles          = NULL;
    }

    return pak;
}

static void drfs_pak_delete(drfs_archive_pak* pArchive)
{
    free(pArchive->pFiles);
    free(pArchive);
}




static dr_bool32 drfs_is_valid_extension__pak(const char* extension)
{
    return drfs__stricmp(extension, "pak") == 0;
}


static drfs_result drfs_open_archive__pak(drfs_file* pArchiveFile, unsigned int accessMode, drfs_handle* pHandleOut)
{
    assert(pArchiveFile != NULL);
    assert(pHandleOut != NULL);
    assert(drfs_tell(pArchiveFile) == 0);

    *pHandleOut = NULL;

    if (!drfs_lock(pArchiveFile)) {
        return drfs_unknown_error;
    }

    drfs_result result = drfs_success;
    drfs_archive_pak* pak = drfs_pak_create(pArchiveFile, accessMode);
    if (pak != NULL)
    {
        // First 4 bytes should equal "PACK"
        if (drfs_read_nolock(pArchiveFile, pak->id, 4, NULL) == drfs_success)
        {
            if (pak->id[0] == 'P' && pak->id[1] == 'A' && pak->id[2] == 'C' && pak->id[3] == 'K')
            {
                if (drfs_read_nolock(pArchiveFile, &pak->directoryOffset, 4, NULL) == drfs_success)
                {
                    if (drfs_read_nolock(pArchiveFile, &pak->directoryLength, 4, NULL) == drfs_success)
                    {
                        // We loaded the header just fine so now we want to allocate space for each file in the directory and load them. Note that
                        // this does not load the file data itself, just information about the files like their name and size.
                        if (pak->directoryLength % 64 == 0)
                        {
                            unsigned int fileCount = pak->directoryLength / 64;
                            if (fileCount > 0)
                            {
                                assert((sizeof(drfs_file_pak) * fileCount) == pak->directoryLength);

                                pak->pFiles = (drfs_file_pak*)malloc(pak->directoryLength);
                                if (pak->pFiles != NULL)
                                {
                                    // Seek to the directory listing before trying to read it.
                                    if (drfs_seek_nolock(pArchiveFile, pak->directoryOffset, drfs_origin_start) == drfs_success)
                                    {
                                        size_t bytesRead;
                                        if (drfs_read_nolock(pArchiveFile, pak->pFiles, pak->directoryLength, &bytesRead) == drfs_success && bytesRead == pak->directoryLength)
                                        {
                                            // All good!
                                        }
                                        else
                                        {
                                            // Failed to read the directory listing.
                                            drfs_pak_delete(pak);
                                            pak = NULL;
                                            result = drfs_invalid_archive;
                                        }
                                    }
                                    else
                                    {
                                        // Failed to seek to the directory listing.
                                        drfs_pak_delete(pak);
                                        pak = NULL;
                                        result = drfs_invalid_archive;
                                    }
                                }
                                else
                                {
                                    // Failed to allocate memory for the file info buffer.
                                    drfs_pak_delete(pak);
                                    pak = NULL;
                                    result = drfs_out_of_memory;
                                }
                            }
                        }
                        else
                        {
                            // The directory length is not a multiple of 64 - something is wrong with the file.
                            drfs_pak_delete(pak);
                            pak = NULL;
                            result = drfs_invalid_archive;
                        }
                    }
                    else
                    {
                        // Failed to read the directory length.
                        drfs_pak_delete(pak);
                        pak = NULL;
                        result = drfs_invalid_archive;
                    }
                }
                else
                {
                    // Failed to read the directory offset.
                    drfs_pak_delete(pak);
                    pak = NULL;
                    result = drfs_invalid_archive;
                }
            }
            else
            {
                // Not a pak file.
                drfs_pak_delete(pak);
                pak = NULL;
                result = drfs_invalid_archive;
            }
        }
        else
        {
            // Failed to read the header.
            drfs_pak_delete(pak);
            pak = NULL;
            result = drfs_invalid_archive;
        }
    }

    drfs_unlock(pArchiveFile);
    *pHandleOut = pak;
    return result;
}

static void drfs_close_archive__pak(drfs_handle archive)
{
    drfs_archive_pak* pPak = (drfs_archive_pak*)archive;
    assert(pPak != NULL);

    drfs_pak_delete(pPak);
}

static drfs_result drfs_get_file_info__pak(drfs_handle archive, const char* relativePath, drfs_file_info* fi)
{
    // We can determine whether or not the path refers to a file or folder by checking it the path is parent of any
    // files in the archive. If so, it's a folder, otherwise it's a file (so long as it exists).
    drfs_archive_pak* pak = (drfs_archive_pak*)archive;
    assert(pak != NULL);

    unsigned int fileCount = pak->directoryLength / 64;
    for (unsigned int i = 0; i < fileCount; ++i)
    {
        drfs_file_pak* pFile = pak->pFiles + i;
        if (strcmp(pFile->name, relativePath) == 0)
        {
            // It's a file.
            drfs__strcpy_s(fi->absolutePath, sizeof(fi->absolutePath), relativePath);
            fi->sizeInBytes      = (dr_uint64)pFile->sizeInBytes;
            fi->lastModifiedTime = 0;
            fi->attributes       = DRFS_FILE_ATTRIBUTE_READONLY;

            return drfs_success;
        }
        else if (drfs_drpath_is_descendant(pFile->name, relativePath))
        {
            // It's a directory.
            drfs__strcpy_s(fi->absolutePath, sizeof(fi->absolutePath), relativePath);
            fi->sizeInBytes      = 0;
            fi->lastModifiedTime = 0;
            fi->attributes       = DRFS_FILE_ATTRIBUTE_READONLY | DRFS_FILE_ATTRIBUTE_DIRECTORY;

            return drfs_success;
        }
    }

    return drfs_does_not_exist;
}

static drfs_handle drfs_begin_iteration__pak(drfs_handle archive, const char* relativePath)
{
    (void)archive;
    assert(relativePath != NULL);

    drfs_iterator_pak* pIterator = (drfs_iterator_pak*)malloc(sizeof(drfs_iterator_pak));
    if (pIterator != NULL)
    {
        pIterator->index = 0;
        drfs__strcpy_s(pIterator->directoryPath, sizeof(pIterator->directoryPath), relativePath);
        pIterator->processedDirCount = 0;
        pIterator->pProcessedDirs    = NULL;
    }

    return pIterator;
}

static void drfs_end_iteration__pak(drfs_handle archive, drfs_handle iterator)
{
    (void)archive;

    drfs_iterator_pak* pIterator = (drfs_iterator_pak*)iterator;
    assert(pIterator != NULL);

    free(pIterator);
}

static dr_bool32 drfs_next_iteration__pak(drfs_handle archive, drfs_handle iterator, drfs_file_info* fi)
{
    drfs_iterator_pak* pIterator = (drfs_iterator_pak*)iterator;
    assert(pIterator != NULL);

    drfs_archive_pak* pak = (drfs_archive_pak*)archive;
    assert(pak != NULL);

    unsigned int fileCount = pak->directoryLength / 64;
    while (pIterator->index < fileCount)
    {
        unsigned int iFile = pIterator->index++;

        drfs_file_pak* pFile = pak->pFiles + iFile;
        if (drfs_drpath_is_child(pFile->name, pIterator->directoryPath))
        {
            // It's a file.
            drfs__strcpy_s(fi->absolutePath, DRFS_MAX_PATH, pFile->name);
            fi->sizeInBytes      = (dr_uint64)pFile->sizeInBytes;
            fi->lastModifiedTime = 0;
            fi->attributes       = DRFS_FILE_ATTRIBUTE_READONLY;

            return DR_TRUE;
        }
        else if (drfs_drpath_is_descendant(pFile->name, pIterator->directoryPath))
        {
            // It's a directory. This needs special handling because we don't want to iterate over the same directory multiple times.
            const char* childDirEnd = pFile->name + strlen(pIterator->directoryPath) + 1;    // +1 for the slash.
            while (childDirEnd[0] != '\0' && childDirEnd[0] != '/' && childDirEnd[0] != '\\')
            {
                childDirEnd += 1;
            }

            char childDir[64];
            memcpy(childDir, pFile->name, childDirEnd - pFile->name);
            childDir[childDirEnd - pFile->name] = '\0';

            if (!drfs_iterator_pak_has_dir_been_processed(pIterator, childDir))
            {
                // It's a directory.
                drfs__strcpy_s(fi->absolutePath, DRFS_MAX_PATH, childDir);
                fi->sizeInBytes      = 0;
                fi->lastModifiedTime = 0;
                fi->attributes       = DRFS_FILE_ATTRIBUTE_READONLY | DRFS_FILE_ATTRIBUTE_DIRECTORY;

                drfs_iterator_pak_append_processed_dir(pIterator, childDir);

                return DR_TRUE;
            }
        }
    }

    return DR_FALSE;
}

static drfs_result drfs_open_file__pak(drfs_handle archive, const char* relativePath, unsigned int accessMode, drfs_handle* pHandleOut)
{
    assert(relativePath != NULL);
    assert(pHandleOut != NULL);

    // Only supporting read-only for now.
    if ((accessMode & DRFS_WRITE) != 0) {
        return drfs_permission_denied;
    }


    drfs_archive_pak* pak = (drfs_archive_pak*)archive;
    assert(pak != NULL);

    for (unsigned int iFile = 0; iFile < (pak->directoryLength / 64); ++iFile)
    {
        if (strcmp(relativePath, pak->pFiles[iFile].name) == 0)
        {
            // We found the file.
            drfs_openedfile_pak* pOpenedFile = (drfs_openedfile_pak*)malloc(sizeof(*pOpenedFile));
            if (pOpenedFile != NULL)
            {
                pOpenedFile->offsetInArchive = pak->pFiles[iFile].offset;
                pOpenedFile->sizeInBytes     = pak->pFiles[iFile].sizeInBytes;
                pOpenedFile->readPointer     = 0;

                *pHandleOut = pOpenedFile;
                return drfs_success;
            }
        }
    }


    return drfs_does_not_exist;
}

static void drfs_close_file__pak(drfs_handle archive, drfs_handle file)
{
    (void)archive;

    drfs_openedfile_pak* pOpenedFile = (drfs_openedfile_pak*)file;
    assert(pOpenedFile != NULL);

    free(pOpenedFile);
}

static drfs_result drfs_read_file__pak(drfs_handle archive, drfs_handle file, void* pDataOut, size_t bytesToRead, size_t* pBytesReadOut)
{
    assert(pDataOut != NULL);
    assert(bytesToRead > 0);

    drfs_archive_pak* pak = (drfs_archive_pak*)archive;
    assert(pak != NULL);

    drfs_openedfile_pak* pOpenedFile = (drfs_openedfile_pak*)file;
    assert(pOpenedFile != NULL);

    // The read pointer should never go past the file size.
    assert(pOpenedFile->sizeInBytes >= pOpenedFile->readPointer);


    size_t bytesAvailable = pOpenedFile->sizeInBytes - pOpenedFile->readPointer;
    if (bytesAvailable < bytesToRead) {
        bytesToRead = bytesAvailable;     // Safe cast, as per the check above.
    }

    if (!drfs_lock(pak->pArchiveFile)) {
        return drfs_unknown_error;
    }

    drfs_seek_nolock(pak->pArchiveFile, (dr_int64)(pOpenedFile->offsetInArchive + pOpenedFile->readPointer), drfs_origin_start);
    drfs_result result = drfs_read_nolock(pak->pArchiveFile, pDataOut, bytesToRead, pBytesReadOut);
    if (result == drfs_success) {
        pOpenedFile->readPointer += bytesToRead;
    }

    drfs_unlock(pak->pArchiveFile);
    return result;
}

static drfs_result drfs_write_file__pak(drfs_handle archive, drfs_handle file, const void* pData, size_t bytesToWrite, size_t* pBytesWrittenOut)
{
    (void)archive;
    (void)file;
    (void)pData;
    (void)bytesToWrite;

    assert(archive != NULL);
    assert(file != NULL);
    assert(pData != NULL);
    assert(bytesToWrite > 0);

    // All files are read-only for now.
    if (pBytesWrittenOut) {
        *pBytesWrittenOut = 0;
    }

    return drfs_permission_denied;
}

static drfs_result drfs_seek_file__pak(drfs_handle archive, drfs_handle file, dr_int64 bytesToSeek, drfs_seek_origin origin)
{
    (void)archive;

    drfs_openedfile_pak* pOpenedFile = (drfs_openedfile_pak*)file;
    assert(pOpenedFile != NULL);

    dr_uint64 newPos = pOpenedFile->readPointer;
    if (origin == drfs_origin_current) {
        if ((dr_int64)newPos + bytesToSeek >= 0) {
            newPos = (dr_uint64)((dr_int64)newPos + bytesToSeek);
        } else {
            return drfs_invalid_args;   // Trying to seek to before the beginning of the file.
        }
    } else if (origin == drfs_origin_start) {
        assert(bytesToSeek >= 0);
        newPos = (dr_uint64)bytesToSeek;
    } else if (origin == drfs_origin_end) {
        assert(bytesToSeek >= 0);
        if ((dr_uint64)bytesToSeek <= pOpenedFile->sizeInBytes) {
            newPos = pOpenedFile->sizeInBytes - (dr_uint64)bytesToSeek;
        } else {
            return drfs_invalid_args;   // Trying to seek to before the beginning of the file.
        }
    } else {
        return drfs_unknown_error;  // Should never get here.
    }


    if (newPos > pOpenedFile->sizeInBytes) {
        return drfs_invalid_args;
    }

    pOpenedFile->readPointer = (size_t)newPos;
    return drfs_success;
}

static dr_uint64 drfs_tell_file__pak(drfs_handle archive, drfs_handle file)
{
    (void)archive;

    drfs_openedfile_pak* pOpenedFile = (drfs_openedfile_pak*)file;
    assert(pOpenedFile != NULL);

    return pOpenedFile->readPointer;
}

static dr_uint64 drfs_file_size__pak(drfs_handle archive, drfs_handle file)
{
    (void)archive;

    drfs_openedfile_pak* pOpenedFile = (drfs_openedfile_pak*)file;
    assert(pOpenedFile != NULL);

    return pOpenedFile->sizeInBytes;
}

static void drfs_flush__pak(drfs_handle archive, drfs_handle file)
{
    (void)archive;
    (void)file;

    assert(archive != NULL);
    assert(file != NULL);

    // All files are read-only for now.
}

static void drfs_register_pak_backend(drfs_context* pContext)
{
    if (pContext == NULL) {
        return;
    }

    drfs_archive_callbacks callbacks;
    callbacks.is_valid_extension = drfs_is_valid_extension__pak;
    callbacks.open_archive       = drfs_open_archive__pak;
    callbacks.close_archive      = drfs_close_archive__pak;
    callbacks.get_file_info      = drfs_get_file_info__pak;
    callbacks.begin_iteration    = drfs_begin_iteration__pak;
    callbacks.end_iteration      = drfs_end_iteration__pak;
    callbacks.next_iteration     = drfs_next_iteration__pak;
    callbacks.delete_file        = NULL;
    callbacks.move_file          = NULL;
    callbacks.create_directory   = NULL;
    callbacks.copy_file          = NULL;
    callbacks.open_file          = drfs_open_file__pak;
    callbacks.close_file         = drfs_close_file__pak;
    callbacks.read_file          = drfs_read_file__pak;
    callbacks.write_file         = drfs_write_file__pak;
    callbacks.seek_file          = drfs_seek_file__pak;
    callbacks.tell_file          = drfs_tell_file__pak;
    callbacks.file_size          = drfs_file_size__pak;
    callbacks.flush_file         = drfs_flush__pak;
    drfs_register_archive_backend(pContext, callbacks);
}
#endif  //DR_FS_NO_PAK



///////////////////////////////////////////////////////////////////////////////
//
// Wavefront MTL
//
///////////////////////////////////////////////////////////////////////////////
#ifndef DR_FS_NO_MTL
typedef struct
{
    // The byte offset within the archive
    dr_uint64 offset;

    // The size of the file, in bytes.
    dr_uint64 sizeInBytes;

    // The name of the material. The specification says this can be any length, but we're going to clamp it to 255 + null terminator which should be fine.
    char name[256];

}drfs_file_mtl;

typedef struct
{
    // A pointer to the archive's file so we can read data.
    drfs_file* pArchiveFile;

    // The access mode.
    unsigned int accessMode;

    // The buffer containing the list of files.
    drfs_file_mtl* pFiles;

    // The number of files in the archive.
    unsigned int fileCount;

}drfs_archive_mtl;

typedef struct
{
    // The current index of the iterator. When this hits the file count, the iteration is finished.
    unsigned int index;

}drfs_iterator_mtl;

typedef struct
{
    // The offset within the archive file the first byte of the file is located.
    dr_uint64 offsetInArchive;

    // The size of the file in bytes so we can guard against overflowing reads.
    dr_uint64 sizeInBytes;

    // The current position of the file's read pointer.
    dr_uint64 readPointer;

}drfs_openedfile_mtl;


static drfs_archive_mtl* drfs_mtl_create(drfs_file* pArchiveFile, unsigned int accessMode)
{
    drfs_archive_mtl* mtl = (drfs_archive_mtl*)malloc(sizeof(drfs_archive_mtl));
    if (mtl != NULL)
    {
        mtl->pArchiveFile = pArchiveFile;
        mtl->accessMode   = accessMode;
        mtl->pFiles       = NULL;
        mtl->fileCount    = 0;
    }

    return mtl;
}

static void drfs_mtl_delete(drfs_archive_mtl* pArchive)
{
    free(pArchive->pFiles);
    free(pArchive);
}

static void drfs_mtl_addfile(drfs_archive_mtl* pArchive, drfs_file_mtl* pFile)
{
    if (pArchive != NULL && pFile != NULL)
    {
        drfs_file_mtl* pOldBuffer = pArchive->pFiles;
        drfs_file_mtl* pNewBuffer = (drfs_file_mtl*)malloc(sizeof(drfs_file_mtl) * (pArchive->fileCount + 1));

        if (pNewBuffer != 0)
        {
            for (unsigned int iDst = 0; iDst < pArchive->fileCount; ++iDst) {
                pNewBuffer[iDst] = pOldBuffer[iDst];
            }

            pNewBuffer[pArchive->fileCount] = *pFile;

            pArchive->pFiles     = pNewBuffer;
            pArchive->fileCount += 1;

            free(pOldBuffer);
        }
    }
}


typedef struct
{
    dr_uint64 archiveSizeInBytes;
    dr_uint64 bytesRemaining;
    drfs_file*  pFile;
    char*          chunkPointer;
    char*          chunkEnd;
    char           chunk[4096];
    unsigned int   chunkSize;

}drfs_openarchive_mtl_state;

static dr_bool32 drfs_mtl_loadnextchunk(drfs_openarchive_mtl_state* pState)
{
    assert(pState != NULL);

    if (pState->bytesRemaining > 0)
    {
        pState->chunkSize = (pState->bytesRemaining > 4096) ? 4096 : (unsigned int)pState->bytesRemaining;
        assert(pState->chunkSize);

        if (drfs_read_nolock(pState->pFile, pState->chunk, pState->chunkSize, NULL) == drfs_success)
        {
            pState->bytesRemaining -= pState->chunkSize;
            pState->chunkPointer = pState->chunk;
            pState->chunkEnd     = pState->chunk + pState->chunkSize;

            return DR_TRUE;
        }
        else
        {
            // An error occured while reading. Just reset everything to make it look like an error occured.
            pState->bytesRemaining = 0;
            pState->chunkSize      = 0;
            pState->chunkPointer   = pState->chunk;
            pState->chunkEnd       = pState->chunkPointer;
        }
    }

    return DR_FALSE;
}

static dr_bool32 drfs_mtl_loadnewmtl(drfs_openarchive_mtl_state* pState)
{
    assert(pState != NULL);

    const char newmtl[7] = "newmtl";
    for (unsigned int i = 0; i < 6; ++i)
    {
        // Check if we need a new chunk.
        if (pState->chunkPointer >= pState->chunkEnd)
        {
            if (!drfs_mtl_loadnextchunk(pState))
            {
                return DR_FALSE;
            }
        }


        if (pState->chunkPointer[0] != newmtl[i])
        {
            return DR_FALSE;
        }

        pState->chunkPointer += 1;
    }

    // At this point the first 6 characters equal "newmtl".
    return DR_TRUE;
}

static dr_bool32 drfs_mtl_skipline(drfs_openarchive_mtl_state* pState)
{
    assert(pState != NULL);

    // Keep looping until we find a new line character.
    while (pState->chunkPointer < pState->chunkEnd)
    {
        if (pState->chunkPointer[0] == '\n')
        {
            // Found the new line. Now move forward by one to get past the new line character.
            pState->chunkPointer += 1;
            if (pState->chunkPointer >= pState->chunkEnd)
            {
                return drfs_mtl_loadnextchunk(pState);
            }

            return DR_TRUE;
        }

        pState->chunkPointer += 1;
    }

    // If we get here it means we got past the end of the chunk. We just read the next chunk and call this recursively.
    if (drfs_mtl_loadnextchunk(pState))
    {
        return drfs_mtl_skipline(pState);
    }

    return DR_FALSE;
}

static dr_bool32 drfs_mtl_skipwhitespace(drfs_openarchive_mtl_state* pState)
{
    assert(pState != NULL);

    while (pState->chunkPointer < pState->chunkEnd)
    {
        const char c = pState->chunkPointer[0];
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
        {
            return DR_TRUE;
        }

        pState->chunkPointer += 1;
    }

    if (drfs_mtl_loadnextchunk(pState))
    {
        return drfs_mtl_skipwhitespace(pState);
    }

    return DR_FALSE;
}

static dr_bool32 drfs_mtl_loadmtlname(drfs_openarchive_mtl_state* pState, void* dst, unsigned int dstSizeInBytes)
{
    assert(pState != NULL);

    // We loop over character by character until we find a whitespace, "#" character or the end of the file.
    char* dst8 = (char*)dst;
    while (dstSizeInBytes > 0 && pState->chunkPointer < pState->chunkEnd)
    {
        const char c = pState->chunkPointer[0];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '#')
        {
            // We've found the end of the name. Null terminate and return.
            *dst8 = '\0';
            return DR_TRUE;
        }
        else
        {
            *dst8++ = c;
            dstSizeInBytes -= 1;
            pState->chunkPointer += 1;
        }
    }

    // At this point we either ran out of space in the destination buffer or the chunk.
    if (dstSizeInBytes > 0)
    {
        // We got to the end of the chunk, so we need to load the next chunk and call this recursively.
        assert(pState->chunkPointer == pState->chunkEnd);

        if (drfs_mtl_loadnextchunk(pState))
        {
            return drfs_mtl_loadmtlname(pState, dst8, dstSizeInBytes);
        }
        else
        {
            // We reached the end of the file, but the name may be valid.
            return DR_TRUE;
        }
    }
    else
    {
        // We ran out of room in the buffer.
        return DR_FALSE;
    }
}


static dr_bool32 drfs_is_valid_extension__mtl(const char* extension)
{
    return drfs__stricmp(extension, "mtl") == 0;
}

static drfs_result drfs_open_archive__mtl(drfs_file* pArchiveFile, unsigned int accessMode, drfs_handle* pHandleOut)
{
    assert(pArchiveFile != NULL);
    assert(pHandleOut != NULL);
    assert(drfs_tell(pArchiveFile) == 0);

    *pHandleOut = NULL;

    if (!drfs_lock(pArchiveFile)) {
        return drfs_unknown_error;
    }

    drfs_archive_mtl* mtl = drfs_mtl_create(pArchiveFile, accessMode);
    if (mtl == NULL) {
        return drfs_invalid_archive;
    }

    drfs_result result = drfs_success;

    // We create a state object that is used to help us with chunk management.
    drfs_openarchive_mtl_state state;
    state.pFile              = pArchiveFile;
    state.archiveSizeInBytes = drfs_size_nolock(pArchiveFile);
    state.bytesRemaining     = state.archiveSizeInBytes;
    state.chunkSize          = 0;
    state.chunkPointer       = state.chunk;
    state.chunkEnd           = state.chunk;
    if (drfs_mtl_loadnextchunk(&state))
    {
        while (state.bytesRemaining > 0 || state.chunkPointer < state.chunkEnd)
        {
            ptrdiff_t bytesRemainingInChunk = state.chunkEnd - state.chunkPointer;
            assert(bytesRemainingInChunk > 0);

            dr_uint64 newmtlOffset = state.archiveSizeInBytes - state.bytesRemaining - ((dr_uint64)bytesRemainingInChunk);

            if (drfs_mtl_loadnewmtl(&state))
            {
                if (state.chunkPointer[0] == ' ' || state.chunkPointer[0] == '\t')
                {
                    // We found a new material. We need to iterate until we hit the first whitespace, "#", or the end of the file.
                    if (drfs_mtl_skipwhitespace(&state))
                    {
                        drfs_file_mtl file;
                        if (drfs_mtl_loadmtlname(&state, file.name, 256))
                        {
                            // Everything worked out. We now need to create the file and add it to our list. At this point we won't know the size. We determine
                            // the size in a post-processing step later.
                            file.offset = newmtlOffset;
                            drfs_mtl_addfile(mtl, &file);
                        }
                    }
                }
            }

            // Move to the next line.
            drfs_mtl_skipline(&state);
        }


        // The files will have been read at this point, but we need to do a post-processing step to retrieve the size of each file.
        for (unsigned int iFile = 0; iFile < mtl->fileCount; ++iFile)
        {
            if (iFile < mtl->fileCount - 1)
            {
                // It's not the last item. The size of this item is the offset of the next file minus the offset of this file.
                mtl->pFiles[iFile].sizeInBytes = mtl->pFiles[iFile + 1].offset - mtl->pFiles[iFile].offset;
            }
            else
            {
                // It's the last item. The size of this item is the size of the archive file minus the file's offset.
                mtl->pFiles[iFile].sizeInBytes = state.archiveSizeInBytes - mtl->pFiles[iFile].offset;
            }
        }
    }
    else
    {
        drfs_mtl_delete(mtl);
        mtl = NULL;
        result = drfs_invalid_archive;
    }

    drfs_unlock(pArchiveFile);
    *pHandleOut = mtl;
    return result;
}

static void drfs_close_archive__mtl(drfs_handle archive)
{
    drfs_archive_mtl* mtl = (drfs_archive_mtl*)archive;
    assert(mtl != NULL);

    drfs_mtl_delete(mtl);
}

static drfs_result drfs_get_file_info__mtl(drfs_handle archive, const char* relativePath, drfs_file_info* fi)
{
    drfs_archive_mtl* mtl = (drfs_archive_mtl*)archive;
    assert(mtl != NULL);

    for (unsigned int iFile = 0; iFile < mtl->fileCount; ++iFile)
    {
        if (strcmp(relativePath, mtl->pFiles[iFile].name) == 0)
        {
            // We found the file.
            if (fi != NULL)
            {
                drfs__strcpy_s(fi->absolutePath, sizeof(fi->absolutePath), relativePath);
                fi->sizeInBytes      = mtl->pFiles[iFile].sizeInBytes;
                fi->lastModifiedTime = 0;
                fi->attributes       = DRFS_FILE_ATTRIBUTE_READONLY;
            }

            return drfs_success;
        }
    }

    return drfs_does_not_exist;
}

static drfs_handle drfs_begin_iteration__mtl(drfs_handle archive, const char* relativePath)
{
    assert(relativePath != NULL);

    drfs_archive_mtl* mtl = (drfs_archive_mtl*)archive;
    assert(mtl != NULL);

    if (mtl->fileCount > 0)
    {
        if (relativePath[0] == '\0' || (relativePath[0] == '/' && relativePath[1] == '\0'))     // This is a flat archive, so no sub-folders.
        {
            drfs_iterator_mtl* pIterator = (drfs_iterator_mtl*)malloc(sizeof(*pIterator));
            if (pIterator != NULL)
            {
                pIterator->index = 0;
                return pIterator;
            }
        }
    }

    return NULL;
}

static void drfs_end_iteration__mtl(drfs_handle archive, drfs_handle iterator)
{
    (void)archive;

    drfs_iterator_mtl* pIterator = (drfs_iterator_mtl*)iterator;
    free(pIterator);
}

static dr_bool32 drfs_next_iteration__mtl(drfs_handle archive, drfs_handle iterator, drfs_file_info* fi)
{
    drfs_archive_mtl* mtl = (drfs_archive_mtl*)archive;
    assert(mtl != NULL);

    drfs_iterator_mtl* pIterator = (drfs_iterator_mtl*)iterator;
    assert(pIterator != NULL);

    if (pIterator->index < mtl->fileCount)
    {
        if (fi != NULL)
        {
            drfs__strcpy_s(fi->absolutePath, DRFS_MAX_PATH, mtl->pFiles[pIterator->index].name);
            fi->sizeInBytes      = mtl->pFiles[pIterator->index].sizeInBytes;
            fi->lastModifiedTime = 0;
            fi->attributes       = DRFS_FILE_ATTRIBUTE_READONLY;
        }

        pIterator->index += 1;
        return DR_TRUE;
    }

    return DR_FALSE;
}

static drfs_result drfs_open_file__mtl(drfs_handle archive, const char* relativePath, unsigned int accessMode, drfs_handle* pHandleOut)
{
    assert(relativePath != NULL);
    assert(pHandleOut != NULL);

    *pHandleOut = NULL;

    // Only supporting read-only for now.
    if ((accessMode & DRFS_WRITE) != 0) {
        return drfs_permission_denied;
    }

    drfs_archive_mtl* mtl = (drfs_archive_mtl*)archive;
    assert(mtl != NULL);

    for (unsigned int iFile = 0; iFile < mtl->fileCount; ++iFile)
    {
        if (strcmp(relativePath, mtl->pFiles[iFile].name) == 0)
        {
            // We found the file.
            drfs_openedfile_mtl* pOpenedFile = (drfs_openedfile_mtl*)malloc(sizeof(drfs_openedfile_mtl));
            if (pOpenedFile != NULL)
            {
                pOpenedFile->offsetInArchive = mtl->pFiles[iFile].offset;
                pOpenedFile->sizeInBytes     = mtl->pFiles[iFile].sizeInBytes;
                pOpenedFile->readPointer     = 0;

                *pHandleOut = pOpenedFile;
                return drfs_success;
            }
        }
    }

    return drfs_does_not_exist;
}

static void drfs_close_file__mtl(drfs_handle archive, drfs_handle file)
{
    (void)archive;

    drfs_openedfile_mtl* pOpenedFile = (drfs_openedfile_mtl*)file;
    assert(pOpenedFile != NULL);

    free(pOpenedFile);
}

static drfs_result drfs_read_file__mtl(drfs_handle archive, drfs_handle file, void* pDataOut, size_t bytesToRead, size_t* pBytesReadOut)
{
    assert(pDataOut != NULL);
    assert(bytesToRead > 0);

    drfs_archive_mtl* mtl = (drfs_archive_mtl*)archive;
    assert(mtl != NULL);

    drfs_openedfile_mtl* pOpenedFile = (drfs_openedfile_mtl*)file;
    assert(pOpenedFile != NULL);

    // The read pointer should never go past the file size.
    assert(pOpenedFile->sizeInBytes >= pOpenedFile->readPointer);

    dr_uint64 bytesAvailable = pOpenedFile->sizeInBytes - pOpenedFile->readPointer;
    if (bytesAvailable < bytesToRead) {
        bytesToRead = (size_t)bytesAvailable;     // Safe cast, as per the check above.
    }

    if (!drfs_lock(mtl->pArchiveFile)) {
        return drfs_unknown_error;
    }

    drfs_seek_nolock(mtl->pArchiveFile, (dr_int64)(pOpenedFile->offsetInArchive + pOpenedFile->readPointer), drfs_origin_start);
    drfs_result result = drfs_read_nolock(mtl->pArchiveFile, pDataOut, bytesToRead, pBytesReadOut);
    if (result == drfs_success) {
        pOpenedFile->readPointer += bytesToRead;
    }

    return result;
}

static drfs_result drfs_write_file__mtl(drfs_handle archive, drfs_handle file, const void* pData, size_t bytesToWrite, size_t* pBytesWrittenOut)
{
    (void)archive;
    (void)file;
    (void)pData;
    (void)bytesToWrite;

    assert(archive != NULL);
    assert(file != NULL);
    assert(pData != NULL);
    assert(bytesToWrite > 0);

    // All files are read-only for now.
    if (pBytesWrittenOut) {
        *pBytesWrittenOut = 0;
    }

    return drfs_success;
}

static drfs_result drfs_seek_file__mtl(drfs_handle archive, drfs_handle file, dr_int64 bytesToSeek, drfs_seek_origin origin)
{
    (void)archive;

    drfs_openedfile_mtl* pOpenedFile = (drfs_openedfile_mtl*)file;
    assert(pOpenedFile != NULL);

    dr_uint64 newPos = pOpenedFile->readPointer;
    if (origin == drfs_origin_current) {
        if ((dr_int64)newPos + bytesToSeek >= 0) {
            newPos = (dr_uint64)((dr_int64)newPos + bytesToSeek);
        } else {
            return drfs_invalid_args;   // Trying to seek to before the beginning of the file.
        }
    } else if (origin == drfs_origin_start) {
        assert(bytesToSeek >= 0);
        newPos = (dr_uint64)bytesToSeek;
    } else if (origin == drfs_origin_end) {
        assert(bytesToSeek >= 0);
        if ((dr_uint64)bytesToSeek <= pOpenedFile->sizeInBytes) {
            newPos = pOpenedFile->sizeInBytes - (dr_uint64)bytesToSeek;
        } else {
            return drfs_invalid_args;   // Trying to seek to before the beginning of the file.
        }
    } else {
        return drfs_unknown_error;  // Should never get here.
    }


    if (newPos > pOpenedFile->sizeInBytes) {
        return drfs_invalid_args;
    }

    pOpenedFile->readPointer = newPos;
    return drfs_success;
}

static dr_uint64 drfs_tell_file__mtl(drfs_handle archive, drfs_handle file)
{
    (void)archive;

    drfs_openedfile_mtl* pOpenedFile = (drfs_openedfile_mtl*)file;
    assert(pOpenedFile != NULL);

    return pOpenedFile->readPointer;
}

static dr_uint64 drfs_file_size__mtl(drfs_handle archive, drfs_handle file)
{
    (void)archive;

    drfs_openedfile_mtl* pOpenedFile = (drfs_openedfile_mtl*)file;
    assert(pOpenedFile != NULL);

    return pOpenedFile->sizeInBytes;
}

static void drfs_flush__mtl(drfs_handle archive, drfs_handle file)
{
    (void)archive;
    (void)file;

    assert(archive != NULL);
    assert(file != NULL);

    // All files are read-only for now.
}


static void drfs_register_mtl_backend(drfs_context* pContext)
{
    if (pContext == NULL) {
        return;
    }

    drfs_archive_callbacks callbacks;
    callbacks.is_valid_extension = drfs_is_valid_extension__mtl;
    callbacks.open_archive       = drfs_open_archive__mtl;
    callbacks.close_archive      = drfs_close_archive__mtl;
    callbacks.get_file_info      = drfs_get_file_info__mtl;
    callbacks.begin_iteration    = drfs_begin_iteration__mtl;
    callbacks.end_iteration      = drfs_end_iteration__mtl;
    callbacks.next_iteration     = drfs_next_iteration__mtl;
    callbacks.delete_file        = NULL;
    callbacks.move_file          = NULL;
    callbacks.create_directory   = NULL;
    callbacks.copy_file          = NULL;
    callbacks.open_file          = drfs_open_file__mtl;
    callbacks.close_file         = drfs_close_file__mtl;
    callbacks.read_file          = drfs_read_file__mtl;
    callbacks.write_file         = drfs_write_file__mtl;
    callbacks.seek_file          = drfs_seek_file__mtl;
    callbacks.tell_file          = drfs_tell_file__mtl;
    callbacks.file_size          = drfs_file_size__mtl;
    callbacks.flush_file         = drfs_flush__mtl;
    drfs_register_archive_backend(pContext, callbacks);
}
#endif  //DR_FS_NO_MTL



#endif  //DR_FS_IMPLEMENTATION

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

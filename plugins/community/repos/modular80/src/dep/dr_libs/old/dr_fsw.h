// Public Domain. See "unlicense" statement at the end of this file.

// ABOUT
//
// dr_fsw is a simple library for watching for changes to the file system. This is not a full-featured library
// and is only intended for basic use cases.
//
// Limitations:
// - Only Windows is supported at the moment.
//
//
//
// USAGE
//
// This is a single-file library. To use it, do something like the following in one .c file.
//   #define DR_FSW_IMPLEMENTATION
//   #include "dr_fsw.h"
//
// You can then #include this file in other parts of the program as you would with any other header file.
//
// Example:
//      // Startup.
//      drfsw_context* context = drfsw_create_context();
//      if (context == NULL)
//      {
//          // There was an error creating the context...
//      }
//      
//      drfsw_add_directory(context, "C:/My/Folder");
//      drfsw_add_directory(context, "C:/My/Other/Folder");
//      
//      ...
//      
//      // Meanwhile, in another thread...
//      void MyOtherThreadEntryProc()
//      {
//          drfsw_event e;
//          while (isMyApplicationStillAlive && drfsw_next_event(context, &e))
//          {
//              switch (e.type)
//              {
//              case drfsw_event_type_created: OnFileCreated(e.absolutePath); break;
//              case drfsw_event_type_deleted: OnFileDeleted(e.absolutePath); break;
//              case drfsw_event_type_renamed: OnFileRenamed(e.absolutePath, e.absolutePathNew); break;
//              case drfsw_event_type_updated: OnFileUpdated(e.absolutePath); break;
//              default: break;
//              }
//          }
//      }
//      
//      ...
//      
//      // Shutdown
//      drfsw_context* contextOld = context;
//      context = NULL;
//      drfsw_delete_context(contextOld);
//
// Directories are watched recursively, so try to avoid using this on high level directories
// like "C:\". Also avoid watching a directory that is a descendant of another directory that's
// already being watched.
// 
// It does not matter whether or not paths are specified with forward or back slashes; the
// library will normalize all of that internally depending on the platform. Note, however,
// that events always report their paths with forward slashes.
// 
// You don't need to wait for events on a separate thread, however drfsw_next_event() is
// a blocking call, so it's usually best to do so. Alternatively, you can use
// drfsw_peek_event() which is the same, except non-blocking. Avoid using both
// drfsw_next_event() and drfsw_peek_event() at the same time because both will remove
// the event from the internal queue so it likely won't work the way you expect.
// 
// The shutdown sequence is a bit strange, but since another thread is accessing that pointer,
// you should set any shared pointers to null before calling drfsw_delete_context(). That way,
// the next call to drfsw_next_event() will pass in a null pointer which will cause it to
// immediately return with zero and thus break the loop and terminate the thread. It is up to
// the application to ensure the pointer passed to drfsw_next_event() is valid. Deleting a context
// will cause a waiting call to drfsw_next_event() to return 0, however there is a chance that the
// context is deleted before drfsw_next_event() has entered into it's wait state. It's up to the
// application to make sure the context remains valid.
//
//
//
// QUICK NOTES
//  - Files that are not on the machine's local file system will not be detected (such as files on a network drive).
//  - In some cases, renaming files won't be detected. Instead it may be implemented as a delete/create pair.
//
//  - Win32: Every directory that is watched becomes "in use" by the operating system. It is still possible
//    to modify the files and folders inside the watched directory, however.
//  - Win32: There is a known issue with the ReadDirectoryChangesW() watch technique (which is used internally)
//    where some events won't get processed if a large number of files change in a short period of time.

#ifndef dr_fsw_h
#define dr_fsw_h

#ifdef __cplusplus
extern "C" {
#endif

// The maximum length of a path in bytes, including the null terminator. If a path exceeds this amount, it will be set to an empty
// string. When this is changed the source file will need to be recompiled. Most of the time leaving this at 256 is fine, but it's
// not a problem to increase the size if you are encountering issues. Note that increasing this value will increase memory usage
// on both the heap and the stack.
#ifndef DRFSW_MAX_PATH
//#define DRFSW_MAX_PATH    256U
#define DRFSW_MAX_PATH    1024U
//#define DRFSW_MAX_PATH    4096U
#endif

// The maximum size of the event queue before it overflows.
#define DRFSW_EVENT_QUEUE_SIZE        1024U


// The different event types.
typedef enum 
{
    drfsw_event_type_created,
    drfsw_event_type_deleted,
    drfsw_event_type_renamed,
    drfsw_event_type_updated

} drfsw_event_type;


// Structure containing information about an event.
typedef struct
{
    // The type of the event: created, deleted, renamed or updated.
    drfsw_event_type type;

    // The absolute path of the file. For renamed events, this is the old name.
    char absolutePath[DRFSW_MAX_PATH];

    // The new file name. This is only used for renamed events. For other event types, this will be an empty string.
    char absolutePathNew[DRFSW_MAX_PATH];

    // The absolute base path. For renamed events, this is the old base path.
    char absoluteBasePath[DRFSW_MAX_PATH];

    // The absolute base path for the new file name. This is only used for renamed events. For other event types, this will be an empty string.
    char absoluteBasePathNew[DRFSW_MAX_PATH];

} drfsw_event;


typedef void* drfsw_context;


// Creates a file system watcher.
//
// This will create a background thread that will do the actual checking.
drfsw_context* drfsw_create_context(void);

// Deletes the given file system watcher.
//
// This will not return until the thread watching for changes has returned.
//
// You do not need to remove the watched directories beforehand - this function will make sure everything is cleaned up properly.
void drfsw_delete_context(drfsw_context* pContext);


// Adds a directory to watch. This will watch for files and folders recursively.
int drfsw_add_directory(drfsw_context* pContext, const char* absolutePath);

// Removes a watched directory.
void drfsw_remove_directory(drfsw_context* pContext, const char* absolutePath);

// Helper for removing every watched directory.
void drfsw_remove_all_directories(drfsw_context* pContext);

// Determines whether or not the given directory is being watched.
int drfsw_is_watching_directory(drfsw_context* pContext, const char* absolutePath);


// Waits for an event from the file system.
//
// This is a blocking function. Call drfsw_peek_event() to do a non-blocking call. If an error occurs, or the context is deleted, 0
// will be returned and the memory pointed to by pEventOut will be undefined.
//
// This can be called from any thread, however it should not be called from multiple threads simultaneously.
//
// Use caution when using this combined with drfsw_peek_event(). In almost all cases you should use just one or the other at any
// given time.
//
// It is up to the application to ensure the context is still valid before calling this function.
//
// Example Usage:
//
// void MyFSWatcher() {
//     drfsw_event e;
//     while (isMyContextStillAlive && drfsw_next_event(context, e)) {
//         // Do something with the event...
//     }
// }
int drfsw_next_event(drfsw_context* pContext, drfsw_event* pEventOut);

// Checks to see if there is a pending event, and if so, returns non-zero and fills the given structure with the event details. This
// removes the event from the queue.
//
// This can be called from any thread, however it should not be called from multiple threads simultaneously.
//
// It is up to the application to ensure the context is still valid before calling this function.
int drfsw_peek_event(drfsw_context* pContext, drfsw_event* pEventOut);


#ifdef __cplusplus
}
#endif

#endif  //dr_fsw_h


///////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DR_FSW_IMPLEMENTATION

// NOTES:
//
// Win32 and ReadDirectoryChangesW
//
// Here is how watching for changes via the ReadDirectoryChangesW() works:
//  1) You create a handle to the directory with CreateFile()
//  2) You pass this handle to ReadDirectoryChangesW(), including a pointer to a function that is called when changes to the directory are made.
//  3) From the aforementioned callback, ReadDirectoryChangesW() needs to be called again
//
// There are, however, a lot of details that need to be handled correctly in order for this to work
//
// First of all, the callback passed to ReadDirectoryChangesW() will not be called unless the calling thread is in an alertable state. A thread
// is put into an alertable state with WaitForMultipleObjectsEx() (the Ex version is important since it has an extra parameter that lets you
// put the thread into an alertable state). Using this blocks the thread which means you need to create a worker thread in the background.

#if defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-align"
#endif

#ifndef DRFSW_PRIVATE
#define DRFSW_PRIVATE static
#endif

// The number of FILE_NOTIFY_INFORMATION structures in the buffer that's passed to ReadDirectoryChangesW()
#define WIN32_RDC_FNI_COUNT     DRFSW_EVENT_QUEUE_SIZE

#include <assert.h>

#if defined(_WIN32)
#include <windows.h>

DRFSW_PRIVATE void* drfsw_malloc(size_t sizeInBytes)
{
    return HeapAlloc(GetProcessHeap(), 0, sizeInBytes);
}

DRFSW_PRIVATE void drfsw_free(void* p)
{
    HeapFree(GetProcessHeap(), 0, p);
}

DRFSW_PRIVATE void drfsw_memcpy(void* dst, const void* src, size_t sizeInBytes)
{
    CopyMemory(dst, src, sizeInBytes);
}

DRFSW_PRIVATE void drfsw_zeromemory(void* dst, size_t sizeInBytes)
{
    ZeroMemory(dst, sizeInBytes);
}
#else
#include <stdlib.h>
#include <string.h>

DRFSW_PRIVATE void* drfsw_malloc(size_t sizeInBytes)
{
    return malloc(sizeInBytes);
}

DRFSW_PRIVATE void drfsw_free(void* p)
{
    free(p);
}

DRFSW_PRIVATE void drfsw_memcpy(void* dst, const void* src, size_t sizeInBytes)
{
    memcpy(dst, src, sizeInBytes);
}

DRFSW_PRIVATE void drfsw_zeromemory(void* dst, size_t sizeInBytes)
{
    memset(dst, 0, sizeInBytes);
}
#endif


DRFSW_PRIVATE int drfsw_strcpy(char* dst, unsigned int dstSizeInBytes, const char* src)
{
#if defined(_MSC_VER)
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
    
    char* iDst = dst;
    const char* iSrc = src;
    size_t remainingSizeInBytes = dstSizeInBytes;
    while (remainingSizeInBytes > 0 && iSrc[0] != '\0')
    {
        iDst[0] = iSrc[0];

        iDst += 1;
        iSrc += 1;
        remainingSizeInBytes -= 1;
    }

    if (remainingSizeInBytes > 0) {
        iDst[0] = '\0';
    } else {
        dst[0] = '\0';
        return ERANGE;
    }

    return 0;
#endif
}


DRFSW_PRIVATE int drfsw_event_init(drfsw_event* pEvent, drfsw_event_type type, const char* absolutePath, const char* absolutePathNew, const char* absoluteBasePath, const char* absoluteBasePathNew)
{
    if (pEvent != NULL)
    {
        pEvent->type = type;

        if (absolutePath != NULL) {
            drfsw_strcpy(pEvent->absolutePath, DRFSW_MAX_PATH, absolutePath);
        } else {
            drfsw_zeromemory(pEvent->absolutePath, DRFSW_MAX_PATH);
        }

        if (absolutePathNew != NULL) {
            drfsw_strcpy(pEvent->absolutePathNew, DRFSW_MAX_PATH, absolutePathNew);
        } else {
            drfsw_zeromemory(pEvent->absolutePathNew, DRFSW_MAX_PATH);
        }


        if (absoluteBasePath != NULL) {
            drfsw_strcpy(pEvent->absoluteBasePath, DRFSW_MAX_PATH, absoluteBasePath);
        } else {
            drfsw_zeromemory(pEvent->absoluteBasePath, DRFSW_MAX_PATH);
        }

        if (absoluteBasePathNew != NULL) {
            drfsw_strcpy(pEvent->absoluteBasePathNew, DRFSW_MAX_PATH, absoluteBasePathNew);
        } else {
            drfsw_zeromemory(pEvent->absoluteBasePathNew, DRFSW_MAX_PATH);
        }

        return 1;
    }

    return 0;
}


typedef struct
{
    // The buffer containing the events in the queue.
    drfsw_event* pBuffer;

    // The size of the buffer, in drfsw_event's.
    unsigned int bufferSize;

    // The number of items in the queue.
    unsigned int count;

    // The index of the first item in the queue.
    unsigned int indexFirst;

#if defined(_WIN32)
    // The semaphore for blocking in drfsw_next_event().
    HANDLE hSemaphore;

    // The mutex for synchronizing access to the buffer. This is needed because drfsw_next_event() will need to read the buffer while
    // another thread is filling it with events. In addition, it will help to keep drfsw_next_event() and drfsw_peek_event() playing
    // nicely with each other.
    HANDLE hLock;
#endif

} drfsw_event_queue;

DRFSW_PRIVATE int drfsw_event_queue_init(drfsw_event_queue* pQueue)
{
    if (pQueue != NULL)
    {
        pQueue->pBuffer    = NULL;
        pQueue->bufferSize = 0;
        pQueue->indexFirst = 0;
        pQueue->count      = 0;

#if defined(_WIN32)
        pQueue->hSemaphore = CreateSemaphoreW(NULL, 0, DRFSW_EVENT_QUEUE_SIZE, NULL);
        if (pQueue->hSemaphore == NULL)
        {
            drfsw_free(pQueue->pBuffer);
            return 0;
        }

        pQueue->hLock = CreateEventW(NULL, FALSE, TRUE, NULL);
        if (pQueue->hLock == NULL)
        {
            CloseHandle(pQueue->hSemaphore);
            drfsw_free(pQueue->pBuffer);
            return 0;
        }
#endif

        return 1;
    }

    return 0;
}

DRFSW_PRIVATE void drfsw_event_queue_uninit(drfsw_event_queue* pQueue)
{
    if (pQueue != NULL)
    {
        drfsw_free(pQueue->pBuffer);
        pQueue->pBuffer = NULL;

        pQueue->bufferSize = 0;
        pQueue->indexFirst = 0;
        pQueue->count      = 0;

#if defined(_WIN32)
        CloseHandle(pQueue->hSemaphore);
        pQueue->hSemaphore = NULL;

        CloseHandle(pQueue->hLock);
        pQueue->hLock = NULL;
#endif
    }
}

DRFSW_PRIVATE unsigned int drfsw_event_queue_getcount(drfsw_event_queue* pQueue)
{
    if (pQueue != NULL)
    {
        return pQueue->count;
    }

    return 0;
}

DRFSW_PRIVATE void drfsw_event_queue_inflate(drfsw_event_queue* pQueue)
{
    if (pQueue != NULL)
    {
        unsigned int newBufferSize = pQueue->bufferSize + 1;
        if (pQueue->bufferSize > 0)
        {
            newBufferSize = pQueue->bufferSize*2;
        }

        drfsw_event* pOldBuffer = pQueue->pBuffer;
        drfsw_event* pNewBuffer = (drfsw_event*)drfsw_malloc(newBufferSize * sizeof(drfsw_event));

        for (unsigned int iDst = 0; iDst < pQueue->count; ++iDst)
        {
            unsigned int iSrc = (pQueue->indexFirst + iDst) % pQueue->bufferSize;
            drfsw_memcpy(pNewBuffer + iDst, pOldBuffer + iSrc, sizeof(drfsw_event));
        }


        pQueue->bufferSize = newBufferSize;
        pQueue->pBuffer    = pNewBuffer;
        pQueue->indexFirst = 0;

        drfsw_free(pOldBuffer);
    }
}

DRFSW_PRIVATE int drfsw_event_queue_pushback(drfsw_event_queue* pQueue, drfsw_event* pEvent)
{
    if (pQueue != NULL)
    {
        if (pEvent != NULL)
        {
            unsigned int count = drfsw_event_queue_getcount(pQueue);
            if (count == DRFSW_EVENT_QUEUE_SIZE)
            {
                // We've hit the limit.
                return 0;
            }

            if (count == pQueue->bufferSize)
            {
                drfsw_event_queue_inflate(pQueue);
                assert(count < pQueue->bufferSize);
            }


            // Insert the value.
            unsigned int iDst = (pQueue->indexFirst + pQueue->count) % pQueue->bufferSize;
            drfsw_memcpy(pQueue->pBuffer + iDst, pEvent, sizeof(drfsw_event));


            // Increment the counter.
            pQueue->count += 1;


            return 1;
        }
    }

    return 0;
}

DRFSW_PRIVATE int drfsw_event_queue_pop(drfsw_event_queue* pQueue, drfsw_event* pEventOut)
{
    if (pQueue != NULL && pQueue->count > 0)
    {
        if (pEventOut != NULL)
        {
            drfsw_memcpy(pEventOut, pQueue->pBuffer + pQueue->indexFirst, sizeof(drfsw_event));
            pQueue->indexFirst = (pQueue->indexFirst + 1) % pQueue->bufferSize;
        }

        pQueue->count -= 1;


        return 1;
    }

    return 0;
}



// A simple function for appending a relative path to an absolute path. This does not resolve "." and ".." components.
DRFSW_PRIVATE int drfsw_make_absolute_path(const char* absolutePart, const char* relativePart, char absolutePathOut[DRFSW_MAX_PATH])
{
    size_t absolutePartLength = strlen(absolutePart);
    size_t relativePartLength = strlen(relativePart);

    if (absolutePartLength > 0)
    {
        if (absolutePart[absolutePartLength - 1] == '/')
        {
            absolutePartLength -= 1;
        }

        if (absolutePartLength > DRFSW_MAX_PATH)
        {
            absolutePartLength = DRFSW_MAX_PATH - 1;
        }
    }

    if (absolutePartLength + relativePartLength + 1 > DRFSW_MAX_PATH)
    {
        relativePartLength = DRFSW_MAX_PATH - 1 - absolutePartLength - 1;     // -1 for the null terminate and -1 for the slash.
    }


    // Absolute part.
    memcpy(absolutePathOut, absolutePart, absolutePartLength);

    // Slash.
    absolutePathOut[absolutePartLength] = '/';

    // Relative part.
    memcpy(absolutePathOut + absolutePartLength + 1, relativePart, relativePartLength);

    // Null terminator.
    absolutePathOut[absolutePartLength + 1 + relativePartLength] = '\0';


    return 1;
}

// Replaces the back slashes with forward slashes in the given string. This operates on the string in place.
DRFSW_PRIVATE int drfsw_to_forward_slashes(char* path)
{
    if (path != NULL)
    {
        unsigned int counter = 0;
        while (*path++ != '\0' && counter++ < DRFSW_MAX_PATH)
        {
            if (*path == '\\')
            {
                *path = '/';
            }
        }

        return 1;
    }

    return 0;
}


typedef struct
{
    // A pointer to the buffer containing pointers to the objects.
    void** buffer;

    // The size of the buffer, in pointers.
    unsigned int bufferSize;

    // The number of pointers in the list.
    unsigned int count;

} drfsw_list;

DRFSW_PRIVATE int drfsw_list_init(drfsw_list* pList)
{
    if (pList != NULL)
    {
        pList->buffer     = NULL;
        pList->bufferSize = 0;
        pList->count      = 0;

        return 1;
    }

    return 0;
}

DRFSW_PRIVATE void drfsw_list_uninit(drfsw_list* pList)
{
    if (pList != NULL)
    {
        drfsw_free(pList->buffer);
    }
}

DRFSW_PRIVATE void drfsw_list_inflate(drfsw_list* pList)
{
    if (pList != NULL)
    {
        unsigned int newBufferSize = pList->bufferSize + 1;
        if (pList->bufferSize > 0)
        {
            newBufferSize = pList->bufferSize*2;
        }

        void** pOldBuffer = pList->buffer;
        void** pNewBuffer = (void**)drfsw_malloc(newBufferSize*sizeof(void*));

        // Move everything over to the new buffer.
        for (unsigned int i = 0; i < pList->count; ++i)
        {
            pNewBuffer[i] = pOldBuffer[i];
        }


        pList->bufferSize = newBufferSize;
        pList->buffer     = pNewBuffer;
    }
}

DRFSW_PRIVATE void drfsw_list_pushback(drfsw_list* pList, void* pItem)
{
    if (pList != NULL)
    {
        if (pList->count == pList->bufferSize)
        {
            drfsw_list_inflate(pList);
            assert(pList->count < pList->bufferSize);

            pList->buffer[pList->count] = pItem;
            pList->count += 1;
        }
    }
}

DRFSW_PRIVATE void drfsw_list_removebyindex(drfsw_list* pList, unsigned int index)
{
    if (pList != NULL)
    {
        assert(index < pList->count);
        assert(pList->count > 0);

        // Just move everything down one slot.
        for (unsigned int i = index; index < pList->count - 1; ++i)
        {
            pList->buffer[i] = pList->buffer[i + 1];
        }

        pList->count -= 1;
    }
}



#if defined(_WIN32)
#define DRFSW_MAX_PATH_W      (DRFSW_MAX_PATH / sizeof(wchar_t))

///////////////////////////////////////////////
// ReadDirectoryChangesW

static const int WIN32_RDC_PENDING_WATCH  = (1 << 0);
static const int WIN32_RDC_PENDING_DELETE = (1 << 1);


// Replaces the forward slashes to back slashes for use with Win32. This operates on the string in place.
DRFSW_PRIVATE int drfsw_to_back_slashes_wchar(wchar_t* path)
{
    if (path != NULL)
    {
        unsigned int counter = 0;
        while (*path++ != L'\0' && counter++ < DRFSW_MAX_PATH_W)
        {
            if (*path == L'/')
            {
                *path = L'\\';
            }
        }

        return 1;
    }

    return 0;
}

// Converts a UTF-8 string to wchar_t for use with Win32. Free the returned pointer with drfsw_free().
DRFSW_PRIVATE int drfsw_utf8_to_wchar(const char* str, wchar_t wstrOut[DRFSW_MAX_PATH_W])
{
    int wcharsWritten = MultiByteToWideChar(CP_UTF8, 0, str, -1, wstrOut, DRFSW_MAX_PATH_W);
    if (wcharsWritten > 0)
    {
        assert((unsigned int)wcharsWritten <= DRFSW_MAX_PATH_W);
        return 1;
    }

    return 0;
}

DRFSW_PRIVATE int drfsw_wchar_to_utf8(const wchar_t* wstr, int wstrCC, char pathOut[DRFSW_MAX_PATH])
{
    int bytesWritten = WideCharToMultiByte(CP_UTF8, 0, wstr, wstrCC, pathOut, DRFSW_MAX_PATH - 1, NULL, NULL);
    if (bytesWritten > 0)
    {
        assert((unsigned int)bytesWritten < DRFSW_MAX_PATH);
        pathOut[bytesWritten] = '\0';

        return 1;
    }

    return 0;
}

// Converts a UTF-8 path to wchar_t and converts the slashes to backslashes for use with Win32. Free the returned pointer with drfsw_free().
DRFSW_PRIVATE int drfsw_to_win32_path_wchar(const char* path, wchar_t wpathOut[DRFSW_MAX_PATH_W])
{
    if (drfsw_utf8_to_wchar(path, wpathOut))
    {
        return drfsw_to_back_slashes_wchar(wpathOut);
    }

    return 0;
}

// Converts a wchar_t Win32 path to a char unix style path (forward slashes instead of back).
DRFSW_PRIVATE int drfsw_from_win32_path(const wchar_t* wpath, int wpathCC, char pathOut[DRFSW_MAX_PATH])
{
    if (drfsw_wchar_to_utf8(wpath, wpathCC, pathOut))
    {
        return drfsw_to_forward_slashes(pathOut);
    }

    return 0;
}


DRFSW_PRIVATE VOID CALLBACK drfsw_win32_completionroutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
DRFSW_PRIVATE VOID CALLBACK drfsw_win32_schedulewatchAPC(ULONG_PTR dwParam);
DRFSW_PRIVATE VOID CALLBACK drfsw_win32_cancelioAPC(ULONG_PTR dwParam);

typedef struct
{
    // Thelist containing pointers to the watched directory objects. This is not thread safe.
    drfsw_list list;

    // The lock for synchronizing access to the list.
    HANDLE hLock;

} drfsw_directory_list_win32;

// Structure representing the watcher context for the Win32 RDC method.
typedef struct
{
    // The list of watched directories.
    drfsw_directory_list_win32 watchedDirectories;

    // The event queue.
    drfsw_event_queue eventQueue;


    // A handle to the watcher thread.
    HANDLE hThread;

    // The event that will become signaled when the watcher thread needs to be terminated.
    HANDLE hTerminateEvent;

    // The semaphore which is used when deleting a watched folder. This starts off at 0, and the maximum count is 1. When a watched
    // directory is removed, the calling thread will wait on this semaphore while the worker thread does the deletion.
    HANDLE hDeleteDirSemaphore;

    // Whether or not the watch thread needs to be terminated.
    BOOL terminateThread;

} drfsw_context_win32;

DRFSW_PRIVATE drfsw_context* drfsw_create_context_win32(void);
DRFSW_PRIVATE void drfsw_delete_context_win32(drfsw_context_win32* pContext);
DRFSW_PRIVATE int drfsw_add_directory_win32(drfsw_context_win32* pContext, const char* absolutePath);
DRFSW_PRIVATE void drfsw_remove_directory_win32(drfsw_context_win32* pContext, const char* absolutePath);
DRFSW_PRIVATE void drfsw_remove_all_directories_win32(drfsw_context_win32* pContext);
DRFSW_PRIVATE int drfsw_is_watching_directory_win32(drfsw_context_win32* pContext, const char* absolutePath);
DRFSW_PRIVATE int drfsw_next_event_win32(drfsw_context_win32* pContext, drfsw_event* pEventOut);
DRFSW_PRIVATE int drfsw_peek_event_win32(drfsw_context_win32* pContext, drfsw_event* pEventOut);
DRFSW_PRIVATE void drfsw_postevent_win32(drfsw_context_win32* pContext, drfsw_event* pEvent);


// Structure representing a directory that's being watched with the Win32 RDC method.
typedef struct
{
    // A pointer to the context that owns this directory.
    drfsw_context_win32* pContext;

    // The absolute path of the directory being watched.
    char absolutePath[DRFSW_MAX_PATH];

    // The handle representing the directory. This is created with CreateFile() which means the directory itself will become locked
    // because the operating system see's it as "in use". It is possible to modify the files and folder inside the directory, though.
    HANDLE hDirectory;

    // This is required for for ReadDirectoryChangesW().
	OVERLAPPED overlapped;

    // A pointer to the buffer containing the notification objects that is passed to the notification callback specified with
    // ReadDirectoryChangesW(). This must be aligned to a DWORD boundary, but drfsw_malloc() will do that for us, so that should not
    // be an issue.
    FILE_NOTIFY_INFORMATION* pFNIBuffer1;
    FILE_NOTIFY_INFORMATION* pFNIBuffer2;

    // The size of the file notification information buffer, in bytes.
    DWORD fniBufferSizeInBytes;

    // Flags describing the state of the directory.
    int flags;

} drfsw_directory_win32;

DRFSW_PRIVATE int drfsw_directory_win32_beginwatch(drfsw_directory_win32* pDirectory);


DRFSW_PRIVATE void drfsw_directory_win32_uninit(drfsw_directory_win32* pDirectory)
{
    if (pDirectory != NULL)
    {
        if (pDirectory->hDirectory != NULL)
        {
            CloseHandle(pDirectory->hDirectory);
            pDirectory->hDirectory = NULL;
        }

        drfsw_free(pDirectory->pFNIBuffer1);
        pDirectory->pFNIBuffer1 = NULL;

        drfsw_free(pDirectory->pFNIBuffer2);
        pDirectory->pFNIBuffer2 = NULL;
    }
}

DRFSW_PRIVATE int drfsw_directory_win32_init(drfsw_directory_win32* pDirectory, drfsw_context_win32* pContext, const char* absolutePath)
{
    if (pDirectory != NULL)
    {
        pDirectory->pContext             = pContext;
        drfsw_zeromemory(pDirectory->absolutePath, DRFSW_MAX_PATH);
        pDirectory->hDirectory           = NULL;
        pDirectory->pFNIBuffer1          = NULL;
        pDirectory->pFNIBuffer2          = NULL;
        pDirectory->fniBufferSizeInBytes = 0;
        pDirectory->flags                = 0;

        size_t length = strlen(absolutePath);
        if (length > 0)
        {
            memcpy(pDirectory->absolutePath, absolutePath, length);
            pDirectory->absolutePath[length] = '\0';

            wchar_t absolutePathWithBackSlashes[DRFSW_MAX_PATH_W];
            if (drfsw_to_win32_path_wchar(absolutePath, absolutePathWithBackSlashes))
            {
                pDirectory->hDirectory = CreateFileW(
                    absolutePathWithBackSlashes,
                    FILE_LIST_DIRECTORY,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                    NULL);
                if (pDirectory->hDirectory != INVALID_HANDLE_VALUE)
                {
                    // From MSDN:
                    //
                    // Using a completion routine. To receive notification through a completion routine, do not associate the directory with a
                    // completion port. Specify a completion routine in lpCompletionRoutine. This routine is called whenever the operation has
                    // been completed or canceled while the thread is in an alertable wait state. The hEvent member of the OVERLAPPED structure
                    // is not used by the system, so you can use it yourself.
                    ZeroMemory(&pDirectory->overlapped, sizeof(pDirectory->overlapped));
                    pDirectory->overlapped.hEvent = pDirectory;

                    pDirectory->fniBufferSizeInBytes = WIN32_RDC_FNI_COUNT * sizeof(FILE_NOTIFY_INFORMATION);
                    pDirectory->pFNIBuffer1 = (FILE_NOTIFY_INFORMATION*)drfsw_malloc(pDirectory->fniBufferSizeInBytes);
                    pDirectory->pFNIBuffer2 = (FILE_NOTIFY_INFORMATION*)drfsw_malloc(pDirectory->fniBufferSizeInBytes);
                    if (pDirectory->pFNIBuffer1 != NULL && pDirectory->pFNIBuffer2 != NULL)
                    {
                        // At this point the directory is initialized, however it is not yet being watched. The watch needs to be triggered from
                        // the worker thread. To do this, we need to signal hPendingWatchEvent, however that needs to be done after the context
                        // has added the directory to it's internal list.
                        return 1;
                    }
                    else
                    {
                        drfsw_directory_win32_uninit(pDirectory);
                    }
                }
                else
                {
                    drfsw_directory_win32_uninit(pDirectory);
                }
            }
            else
            {
                drfsw_directory_win32_uninit(pDirectory);
            }
        }
    }

    return 0;
}

DRFSW_PRIVATE int drfsw_directory_win32_schedulewatch(drfsw_directory_win32* pDirectory)
{
    if (pDirectory != NULL)
    {
        pDirectory->flags |= WIN32_RDC_PENDING_WATCH;
        QueueUserAPC(drfsw_win32_schedulewatchAPC, pDirectory->pContext->hThread, (ULONG_PTR)pDirectory);

        return 1;
    }

    return 0;
}

DRFSW_PRIVATE int drfsw_directory_win32_scheduledelete(drfsw_directory_win32* pDirectory)
{
    if (pDirectory != NULL)
    {
        pDirectory->flags |= WIN32_RDC_PENDING_DELETE;
        QueueUserAPC(drfsw_win32_cancelioAPC, pDirectory->pContext->hThread, (ULONG_PTR)pDirectory);

        return 1;
    }

    return 0;
}

DRFSW_PRIVATE int drfsw_directory_win32_beginwatch(drfsw_directory_win32* pDirectory)
{
    // This function should only be called from the worker thread.

    if (pDirectory != NULL)
    {
        DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;
        DWORD dwBytes        = 0;
        if (ReadDirectoryChangesW(pDirectory->hDirectory, pDirectory->pFNIBuffer1, pDirectory->fniBufferSizeInBytes, TRUE, dwNotifyFilter, &dwBytes, &pDirectory->overlapped, drfsw_win32_completionroutine))
        {
            pDirectory->flags &= ~WIN32_RDC_PENDING_WATCH;
            return 1;
        }
    }

    return 0;
}




DRFSW_PRIVATE int drfsw_directory_list_win32_init(drfsw_directory_list_win32* pDirectories)
{
    if (pDirectories != NULL)
    {
        drfsw_list_init(&pDirectories->list);

        pDirectories->hLock = CreateEvent(NULL, FALSE, TRUE, NULL);
        if (pDirectories->hLock != NULL)
        {
            return 1;
        }
    }

    return 0;
}

DRFSW_PRIVATE void drfsw_directory_list_win32_uninit(drfsw_directory_list_win32* pDirectories)
{
    if (pDirectories != NULL)
    {
        drfsw_list_uninit(&pDirectories->list);

        CloseHandle(pDirectories->hLock);
        pDirectories->hLock = NULL;
    }
}





DRFSW_PRIVATE VOID CALLBACK drfsw_win32_completionroutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    drfsw_directory_win32* pDirectory = (drfsw_directory_win32*)lpOverlapped->hEvent;
    if (pDirectory != NULL)
    {
        if (dwErrorCode == ERROR_OPERATION_ABORTED)
        {
            // When we get here, CancelIo() was called on the directory. We treat this as a signal that the context has requested that the directory
            // be deleted. At this point the directory has been removed from the context's internal list and we just need to uninitialize and free
            // the directory object.
            drfsw_context_win32* pContext = pDirectory->pContext;
            drfsw_directory_win32_uninit(pDirectory);
            drfsw_free(pDirectory);

            ReleaseSemaphore(pContext->hDeleteDirSemaphore, 1, NULL);
            return;
        }

        assert(dwNumberOfBytesTransfered >= sizeof(FILE_NOTIFY_INFORMATION));

        // At this point we're not actually watching the directory - there is a chance that as we're executing this section there
        // are changes to the file system whose events will go undetected. We need to call ReadDirectoryChangesW() again as soon as
        // possible. This routine is always called from the worker thread, and only while it's in an alertable state. Therefore it
        // is safe for us to use a simple front/back buffer type system to make it as quick as possible to resume watching operations.
        FILE_NOTIFY_INFORMATION* temp = pDirectory->pFNIBuffer1;
        pDirectory->pFNIBuffer1 = pDirectory->pFNIBuffer2;
        pDirectory->pFNIBuffer2 = temp;


        // Begin watching again (call ReadDirectoryChangesW() again) as soon as possible. At this point we are not currently watching
        // for changes to the directory, so we need to start that before posting events. To start watching we need to send a signal to
        // the worker thread which will do the actual call to ReadDirectoryChangesW().
        drfsw_directory_win32_schedulewatch(pDirectory);


        // Now we loop through all of our notifications and post the event to the context for later processing by drfsw_next_event()
        // and drfsw_peek_event().
        char absolutePathOld[DRFSW_MAX_PATH];
        char absoluteBasePathOld[DRFSW_MAX_PATH];
        drfsw_context_win32* pContext = pDirectory->pContext;     // Just for convenience.


        FILE_NOTIFY_INFORMATION* pFNI = pDirectory->pFNIBuffer2;
        for (;;)
        {
            char relativePath[DRFSW_MAX_PATH];
            if (drfsw_from_win32_path(pFNI->FileName, pFNI->FileNameLength / sizeof(wchar_t), relativePath))
            {
                char absolutePath[DRFSW_MAX_PATH];
                if (drfsw_make_absolute_path(pDirectory->absolutePath, relativePath, absolutePath))
                {
                    switch (pFNI->Action)
                    {
                    case FILE_ACTION_ADDED:
                        {
                            drfsw_event e;
                            if (drfsw_event_init(&e, drfsw_event_type_created, absolutePath, NULL, pDirectory->absolutePath, NULL))
                            {
                                drfsw_postevent_win32(pContext, &e);
                            }

                            break;
                        }

                    case FILE_ACTION_REMOVED:
                        {
                            drfsw_event e;
                            if (drfsw_event_init(&e, drfsw_event_type_deleted, absolutePath, NULL, pDirectory->absolutePath, NULL))
                            {
                                drfsw_postevent_win32(pContext, &e);
                            }

                            break;
                        }

                    case FILE_ACTION_RENAMED_OLD_NAME:
                        {
                            drfsw_strcpy(absolutePathOld,     sizeof(absolutePathOld),     absolutePath);
                            drfsw_strcpy(absoluteBasePathOld, sizeof(absoluteBasePathOld), pDirectory->absolutePath);

                            break;
                        }
                    case FILE_ACTION_RENAMED_NEW_NAME:
                        {
                            drfsw_event e;
                            if (drfsw_event_init(&e, drfsw_event_type_renamed, absolutePathOld, absolutePath, absoluteBasePathOld, pDirectory->absolutePath))
                            {
                                drfsw_postevent_win32(pContext, &e);
                            }

                            break;
                        }

                    case FILE_ACTION_MODIFIED:
                        {
                            drfsw_event e;
                            if (drfsw_event_init(&e, drfsw_event_type_updated, absolutePath, NULL, pDirectory->absolutePath, NULL))
                            {
                                drfsw_postevent_win32(pContext, &e);
                            }

                            break;
                        }

                    default: break;
                    }
                }
            }



            if (pFNI->NextEntryOffset == 0)
            {
                break;
            }

            pFNI = (FILE_NOTIFY_INFORMATION*)(((char*)pFNI) + pFNI->NextEntryOffset);
        }
    }
}

DRFSW_PRIVATE VOID CALLBACK drfsw_win32_schedulewatchAPC(ULONG_PTR dwParam)
{
    drfsw_directory_win32* pDirectory = (drfsw_directory_win32*)dwParam;
    if (pDirectory != NULL)
    {
        if ((pDirectory->flags & WIN32_RDC_PENDING_WATCH) != 0)
        {
            drfsw_directory_win32_beginwatch(pDirectory);
        }
    }
}

DRFSW_PRIVATE VOID CALLBACK drfsw_win32_cancelioAPC(ULONG_PTR dwParam)
{
    drfsw_directory_win32* pDirectory = (drfsw_directory_win32*)dwParam;
    if (pDirectory != NULL)
    {
        if ((pDirectory->flags & WIN32_RDC_PENDING_DELETE) != 0)
        {
            // We don't free the directory structure from here. Instead we just call CancelIo(). This will trigger
            // the ERROR_OPERATION_ABORTED error in the notification callback which is where the real delete will
            // occur. That is also where the synchronization lock is released that the thread that called
            // drfsw_delete_directory() is waiting on.
            CancelIo(pDirectory->hDirectory);

            // The directory needs to be removed from the context's list. The directory object will be freed in the
            // notification callback in response to ERROR_OPERATION_ABORTED which will be triggered by the previous
            // call to CancelIo().
            for (unsigned int i = 0; i < pDirectory->pContext->watchedDirectories.list.count; ++i)
            {
                if (pDirectory == pDirectory->pContext->watchedDirectories.list.buffer[i])
                {
                    drfsw_list_removebyindex(&pDirectory->pContext->watchedDirectories.list, i);
                    break;
                }
            }
        }
    }
}




DRFSW_PRIVATE DWORD WINAPI _WatcherThreadProc_RDC(drfsw_context_win32 *pContextRDC)
{
    while (!pContextRDC->terminateThread)
    {
        // Important that we use the Ex version here because we need to put the thread into an alertable state (last argument). If the thread is not put into
        // an alertable state, ReadDirectoryChangesW() won't ever call the notification event.
        DWORD rc = WaitForSingleObjectEx(pContextRDC->hTerminateEvent, INFINITE, TRUE);
        switch (rc)
        {
        case WAIT_OBJECT_0 + 0:
            {
                // The context has signaled that it needs to be deleted.
                pContextRDC->terminateThread = TRUE;
                break;
            }

        case WAIT_IO_COMPLETION:
        default:
            {
                // Nothing to do.
                break;
            }
        }
    }

    return 0;
}

DRFSW_PRIVATE drfsw_context* drfsw_create_context_win32()
{
    drfsw_context_win32* pContext = (drfsw_context_win32*)drfsw_malloc(sizeof(drfsw_context_win32));
    if (pContext != NULL)
    {
        if (drfsw_directory_list_win32_init(&pContext->watchedDirectories))
        {
            if (drfsw_event_queue_init(&pContext->eventQueue))
            {
                pContext->hTerminateEvent     = CreateEvent(NULL, FALSE, FALSE, NULL);
                pContext->hDeleteDirSemaphore = CreateSemaphoreW(NULL, 0, 1, NULL);
                pContext->terminateThread = FALSE;

                if (pContext->hTerminateEvent != NULL)
                {
                    pContext->hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_WatcherThreadProc_RDC, pContext, 0, NULL);
                    if (pContext->hThread != NULL)
                    {
                        // Everything went well.
                    }
                    else
                    {
                        CloseHandle(pContext->hTerminateEvent);

                        drfsw_free(pContext);
                        pContext = NULL;
                    }
                }
                else
                {
                    drfsw_free(pContext);
                    pContext = NULL;
                }
            }
        }
    }

    return (drfsw_context*)pContext;
}

DRFSW_PRIVATE void drfsw_delete_context_win32(drfsw_context_win32* pContext)
{
    if (pContext != NULL)
    {
        // Every watched directory needs to be removed.
        drfsw_remove_all_directories_win32(pContext);


        // Signal the close event, and wait for the thread to finish.
        SignalObjectAndWait(pContext->hTerminateEvent, pContext->hThread, INFINITE, FALSE);

        // The thread has finished, so close the handle.
        CloseHandle(pContext->hThread);
        pContext->hThread = NULL;


        // We need to wait for the event queue to finish up before deleting the context for real. If we don't do this nextevent() may try
        // to access the context and then crash.
        WaitForSingleObject(pContext->eventQueue.hLock, INFINITE);
        drfsw_event_queue_uninit(&pContext->eventQueue);      // <-- This will close pContext->eventQueue.hLock so no need to call SetEvent().


        // The worker thread events need to be closed.
        CloseHandle(pContext->hTerminateEvent);
        pContext->hTerminateEvent = NULL;


        // The semaphore we use for deleting directories.
        CloseHandle(pContext->hDeleteDirSemaphore);
        pContext->hDeleteDirSemaphore = NULL;


        drfsw_directory_list_win32_uninit(&pContext->watchedDirectories);

        // Free the memory.
        drfsw_free(pContext);
    }
}

DRFSW_PRIVATE drfsw_directory_win32* drfsw_find_directory_win32(drfsw_context_win32* pContext, const char* absolutePath, unsigned int* pIndexOut)
{
    assert(pContext != NULL);

    for (unsigned int iDirectory = 0; iDirectory < pContext->watchedDirectories.list.count; ++iDirectory)
    {
        drfsw_directory_win32* pDirectory = (drfsw_directory_win32*)pContext->watchedDirectories.list.buffer[iDirectory];
        if (pDirectory != NULL)
        {
            if (strcmp(absolutePath, pDirectory->absolutePath) == 0)
            {
                if (pIndexOut != NULL)
                {
                    *pIndexOut = iDirectory;
                }

                return pDirectory;
            }
        }
    }

    return NULL;
}

DRFSW_PRIVATE int drfsw_add_directory_win32(drfsw_context_win32* pContext, const char* absolutePath)
{
    if (pContext != NULL)
    {
        drfsw_directory_win32* pDirectory = (drfsw_directory_win32*)drfsw_malloc(sizeof(drfsw_directory_win32));
        if (pDirectory != NULL)
        {
            if (!drfsw_is_watching_directory_win32(pContext, absolutePath))
            {
                if (drfsw_directory_win32_init(pDirectory, pContext, absolutePath))
                {
                    // At this point the directory has been initialized, but it's not yet being watched. To do this, we need to call ReadDirectoryChangesW() from
                    // the worker thread which means we need to signal an event which the worker thread will be waiting on. When the worker thread receives the
                    // signal, it will iterate over each directory in the context and check for the ones that are pending watching. Therefore, before signaling
                    // the event, we need to make sure the directory is added to the context's list.
                    WaitForSingleObject(pContext->watchedDirectories.hLock, INFINITE);
                    {
                        drfsw_list_pushback(&pContext->watchedDirectories.list, pDirectory);
                    }
                    SetEvent(pContext->watchedDirectories.hLock);

                    // The directory is now in the list and we can send the signal.
                    drfsw_directory_win32_schedulewatch(pDirectory);


                    return 1;
                }
                else
                {
                    drfsw_free(pDirectory);
                    pDirectory = NULL;
                }
            }
            else
            {
                drfsw_free(pDirectory);
                pDirectory = NULL;
            }
        }
    }



    // An error occured if we got here.
    return 0;
}

DRFSW_PRIVATE void drfsw_remove_directory_win32_no_lock(drfsw_context_win32* pContext, const char* absolutePath)
{
    assert(pContext != NULL);

    unsigned int index;
    drfsw_directory_win32* pDirectory = drfsw_find_directory_win32(pContext, absolutePath, &index);
    if (pDirectory != NULL)
    {
        // When removing a directory, we need to call CancelIo() on the file handle we created for the directory. The problem is that
        // this needs to be called on the worker thread in order for watcher notification callback function to get the correct error
        // code. To do this we need to signal an event which the worker thread is waiting on. The worker thread will then call CancelIo()
        // which in turn will trigger the correct error code in the notification callback. The notification callback is where the
        // the object will be deleted for real and will release the synchronization lock that this function is waiting on below.
        drfsw_directory_win32_scheduledelete(pDirectory);

        // Now we need to wait for the relevant event to become signaled. This will become signaled when the worker thread has finished
        // deleting the file handle and whatnot from it's end.
        WaitForSingleObject(pContext->hDeleteDirSemaphore, INFINITE);
    }
}

DRFSW_PRIVATE void drfsw_remove_directory_win32(drfsw_context_win32* pContext, const char* absolutePath)
{
    if (pContext != NULL)
    {
        WaitForSingleObject(pContext->watchedDirectories.hLock, INFINITE);
        {
            drfsw_remove_directory_win32_no_lock(pContext, absolutePath);
        }
        SetEvent(pContext->watchedDirectories.hLock);
    }
}

DRFSW_PRIVATE void drfsw_remove_all_directories_win32(drfsw_context_win32* pContext)
{
    if (pContext != NULL)
    {
        WaitForSingleObject(pContext->watchedDirectories.hLock, INFINITE);
        {
            for (unsigned int i = pContext->watchedDirectories.list.count; i > 0; --i)
            {
                drfsw_remove_directory_win32_no_lock(pContext, ((drfsw_directory_win32*)pContext->watchedDirectories.list.buffer[i - 1])->absolutePath);
            }
        }
        SetEvent(pContext->watchedDirectories.hLock);
    }
}

DRFSW_PRIVATE int drfsw_is_watching_directory_win32(drfsw_context_win32* pContext, const char* absolutePath)
{
    if (pContext != NULL)
    {
        return drfsw_find_directory_win32(pContext, absolutePath, NULL) != NULL;
    }

    return 0;
}


DRFSW_PRIVATE int drfsw_next_event_win32(drfsw_context_win32* pContext, drfsw_event* pEvent)
{
    int result = 0;
    if (pContext != NULL && !pContext->terminateThread)
    {
        // Wait for either the semaphore or the thread to terminate.
        HANDLE hEvents[2];
        hEvents[0] = pContext->hThread;
        hEvents[1] = pContext->eventQueue.hSemaphore;

        DWORD rc = WaitForMultipleObjects(sizeof(hEvents) / sizeof(hEvents[0]), hEvents, FALSE, INFINITE);
        switch (rc)
        {
        case WAIT_OBJECT_0 + 0:
            {
                // The thread has been terminated.
                result = 0;
                break;
            }

        case WAIT_OBJECT_0 + 1:
            {
                // We're past the semaphore block, so now we can copy of the event. We need to lock the queue before doing this in case another thread wants
                // to try pushing another event onto the queue.
                if (!pContext->terminateThread)
                {
                    DWORD eventLockResult = WaitForSingleObject(pContext->eventQueue.hLock, INFINITE);
                    if (eventLockResult == WAIT_OBJECT_0)
                    {
                        drfsw_event_queue_pop(&pContext->eventQueue, pEvent);
                    }
                    SetEvent(pContext->eventQueue.hLock);

                    if (eventLockResult == WAIT_OBJECT_0)
                    {
                        result = 1;
                    }
                    else
                    {
                        // The lock returned early for some reason which means there must have been some kind of error or the context has been destroyed.
                        result = 0;
                    }
                }

                break;
            }

        default: break;
        }
    }

    return result;
}

DRFSW_PRIVATE int drfsw_peek_event_win32(drfsw_context_win32* pContext, drfsw_event* pEvent)
{
    int result = 0;
    if (pContext != NULL)
    {
        DWORD eventLockResult = WaitForSingleObject(pContext->eventQueue.hLock, INFINITE);
        {
            // Make sure we decrement our semaphore counter. Don't block here.
            WaitForSingleObject(pContext->eventQueue.hSemaphore, 0);

            // Now we just copy over the data by popping it from the queue.
            if (eventLockResult == WAIT_OBJECT_0)
            {
                if (drfsw_event_queue_getcount(&pContext->eventQueue) > 0)
                {
                    drfsw_event_queue_pop(&pContext->eventQueue, pEvent);
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                // Waiting on the event queue lock failed for some reason. It could mean that the context has been deleted.
                result = 0;
            }
        }
        SetEvent(pContext->eventQueue.hLock);
    }

    return result;
}

DRFSW_PRIVATE void drfsw_postevent_win32(drfsw_context_win32* pContext, drfsw_event* pEvent)
{
    if (pContext != NULL && pEvent != NULL)
    {
        // Add the event to the queue.
        WaitForSingleObject(pContext->eventQueue.hLock, INFINITE);
        {
            drfsw_event_queue_pushback(&pContext->eventQueue, pEvent);
        }
        SetEvent(pContext->eventQueue.hLock);


        // Release the semaphore so that drfsw_next_event() can handle it.
        ReleaseSemaphore(pContext->eventQueue.hSemaphore, 1, NULL);
    }
}



////////////////////////////////////
// Public API for Win32

drfsw_context* drfsw_create_context()
{
    return drfsw_create_context_win32();
}

void drfsw_delete_context(drfsw_context* pContext)
{
    drfsw_delete_context_win32((drfsw_context_win32*)pContext);
}


int drfsw_add_directory(drfsw_context* pContext, const char* absolutePath)
{
    return drfsw_add_directory_win32((drfsw_context_win32*)pContext, absolutePath);
}

void drfsw_remove_directory(drfsw_context* pContext, const char* absolutePath)
{
    drfsw_remove_directory_win32((drfsw_context_win32*)pContext, absolutePath);
}

void drfsw_remove_all_directories(drfsw_context* pContext)
{
    drfsw_remove_all_directories_win32((drfsw_context_win32*)pContext);
}

int drfsw_is_watching_directory(drfsw_context* pContext, const char* absolutePath)
{
    return drfsw_is_watching_directory_win32((drfsw_context_win32*)pContext, absolutePath);
}


int drfsw_next_event(drfsw_context* pContext, drfsw_event* pEventOut)
{
    return drfsw_next_event_win32((drfsw_context_win32*)pContext, pEventOut);
}

int drfsw_peek_event(drfsw_context* pContext, drfsw_event* pEventOut)
{
    return drfsw_peek_event_win32((drfsw_context_win32*)pContext, pEventOut);
}


#endif  // Win32


#if defined(__clang__)
    #pragma GCC diagnostic pop
#endif

#endif //DR_FSW_IMPLEMENTATION




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

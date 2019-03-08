/*
Opus audio decoder. Choice of public domain or MIT-0. See license statements at the end of this file.
dr_opus - v0.0.0 (Unreleased) - xxxx-xx-xx

David Reid - mackron@gmail.com
*/

/* ====== WORK-IN-PROGRESSS ====== */

#ifndef dr_opus_h
#define dr_opus_h

#include <stddef.h> /* For size_t. */

/* Sized types. Prefer built-in types. Fall back to stdint. */
#ifdef _MSC_VER
    #if defined(__clang__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wlanguage-extension-token"
        #pragma GCC diagnostic ignored "-Wc++11-long-long"
    #endif
    typedef   signed __int8  dropus_int8;
    typedef unsigned __int8  dropus_uint8;
    typedef   signed __int16 dropus_int16;
    typedef unsigned __int16 dropus_uint16;
    typedef   signed __int32 dropus_int32;
    typedef unsigned __int32 dropus_uint32;
    typedef   signed __int64 dropus_int64;
    typedef unsigned __int64 dropus_uint64;
    #if defined(__clang__)
        #pragma GCC diagnostic pop
    #endif
#else
    #define DROPUS_HAS_STDINT
    #include <stdint.h>
    typedef int8_t   dropus_int8;
    typedef uint8_t  dropus_uint8;
    typedef int16_t  dropus_int16;
    typedef uint16_t dropus_uint16;
    typedef int32_t  dropus_int32;
    typedef uint32_t dropus_uint32;
    typedef int64_t  dropus_int64;
    typedef uint64_t dropus_uint64;
#endif

#ifdef DROPUS_HAS_STDINT
    typedef uintptr_t dropus_uintptr;
#else
    #if defined(_WIN32)
        #if defined(_WIN64)
            typedef dropus_uint64 dropus_uintptr;
        #else
            typedef dropus_uint32 dropus_uintptr;
        #endif
    #elif defined(__GNUC__)
        #if defined(__LP64__)
            typedef dropus_uint64 dropus_uintptr;
        #else
            typedef dropus_uint32 dropus_uintptr;
        #endif
    #else
        typedef dropus_uint64 dropus_uintptr;   /* Fallback. */
    #endif
#endif

typedef dropus_uint8  dropus_bool8;
typedef dropus_uint32 dropus_bool32;
#define DROPUS_TRUE   1
#define DROPUS_FALSE  0

typedef void* dropus_handle;
typedef void* dropus_ptr;
typedef void (* dropus_proc)(void);

#ifndef NULL
#define NULL 0
#endif

#if defined(SIZE_MAX)
    #define DROPUS_SIZE_MAX SIZE_MAX
#else
    #define DROPUS_SIZE_MAX 0xFFFFFFFF  /* When SIZE_MAX is not defined by the standard library just default to the maximum 32-bit unsigned integer. */
#endif


#ifdef _MSC_VER
#define DROPUS_INLINE __forceinline
#else
#ifdef __GNUC__
#define DROPUS_INLINE __inline__ __attribute__((always_inline))
#else
#define DROPUS_INLINE inline
#endif
#endif

typedef int dropus_result;
#define DROPUS_SUCCESS           0
#define DROPUS_ERROR            -1  /* Generic or unknown error. */
#define DROPUS_INVALID_ARGS     -2
#define DROPUS_BAD_DATA         -100

/***********************************************************************************************************************************************************

Low-Level Opus Stream API

This API is where the low-level decoding takes place. In order to use this, you must know the packet structure of the Opus stream. This is usually the job of
encapsulations such as Ogg and Matroska.

************************************************************************************************************************************************************/
#define DROPUS_MAX_OPUS_FRAMES_PER_PACKET   48      /* RFC 6716 - Section 3.2.5 */
#define DROPUS_MAX_PCM_FRAMES_PER_PACKET    6144    /* RFC 6716 - Section 3.2.5. Maximum of 120ms. Maximum rate is 48kHz. 6144 = 120*48. */

typedef enum
{
    dropus_mode_silk,
    dropus_mode_celt,
    dropus_mode_hybrid
} dropus_mode;

typedef struct
{
    dropus_uint16 sizeInBytes;
} dropus_stream_frame;

typedef struct
{
    dropus_uint8 toc;   /* TOC byte. RFC 6716 - Section 3.1 */
    dropus_stream_frame frames[DROPUS_MAX_OPUS_FRAMES_PER_PACKET];
} dropus_stream_packet;

typedef struct
{
    dropus_stream_packet packet;   /* The current packet. */
} dropus_stream;

/*
Initializes a new low-level Opus stream object.
*/
dropus_result dropus_stream_init(dropus_stream* pOpusStream);

/*
Decodes a packet from the given compressed data.
*/
dropus_result dropus_stream_decode_packet(dropus_stream* pOpusStream, const void* pData, size_t dataSize);



/***********************************************************************************************************************************************************

High-Level Opus Decoding API

************************************************************************************************************************************************************/

typedef enum
{
    dropus_seek_origin_start,
    dropus_seek_origin_current
} dropus_seek_origin;

typedef size_t (* dropus_read_proc)(void* pUserData, void* pBufferOut, size_t bytesToRead);
typedef dropus_bool32 (* dropus_seek_proc)(void* pUserData, int offset, dropus_seek_origin origin);

typedef struct
{
    dropus_read_proc onRead;
    dropus_seek_proc onSeek;
    void* pUserData;
    void* pFile;    /* Only used for decoders that were opened against a file. */
    struct
    {
        const dropus_uint8* pData;
        size_t dataSize;
        size_t currentReadPos;
    } memory;       /* Only used for decoders that were opened against a block of memory. */
} dropus;

/*
Initializes a pre-allocated decoder object from callbacks.
*/
dropus_bool32 dropus_init(dropus* pOpus, dropus_read_proc onRead, dropus_seek_proc onSeek, void* pUserData);

#ifndef DR_OPUS_NO_STDIO
/*
Initializes a pre-allocated decoder object from a file.

This keeps hold of the file handle throughout the lifetime of the decoder and closes it in dropus_uninit().
*/
dropus_bool32 dropus_init_file(dropus* pOpus, const char* pFilePath);
#endif

/*
Initializes a pre-allocated decoder object from a block of memory.

This does not make a copy of the memory.
*/
dropus_bool32 dropus_init_memory(dropus* pOpus, const void* pData, size_t dataSize);

/*
Uninitializes an Opus decoder.
*/
void dropus_uninit(dropus* pOpus);


#endif  /* dr_opus_h */

/************************************************************************************************************************************************************
 ************************************************************************************************************************************************************

 IMPLEMENTATION

 ************************************************************************************************************************************************************
 ************************************************************************************************************************************************************/
#ifdef DR_OPUS_IMPLEMENTATION
#include <stdlib.h>
#include <string.h>
#ifndef DR_OPUS_NO_STDIO
#include <stdio.h>
#endif

/* CPU Architecture. */
#if defined(__x86_64__) || defined(_M_X64)
    #define DROPUS_X64
#elif defined(__i386) || defined(_M_IX86)
    #define DROPUS_X86
#elif defined(__arm__) || defined(_M_ARM)
    #define DROPUS_ARM
#endif

/* Compile-time CPU feature support. */
#if !defined(DR_OPUS_NO_SIMD) && (defined(DROPUS_X86) || defined(DROPUS_X64))
    #if defined(_MSC_VER) && !defined(__clang__)
        #if _MSC_VER >= 1400
            #include <intrin.h>
            static DROPUS_INLINE void dropus__cpuid(int info[4], int fid)
            {
                __cpuid(info, fid);
            }
        #else
            #define DROPUS_NO_CPUID
        #endif
    #else
        #if defined(__GNUC__) || defined(__clang__)
            static DROPUS_INLINE void dropus__cpuid(int info[4], int fid)
            {
                /*
                It looks like the -fPIC option uses the ebx register which GCC complains about. We can work around this by just using a different register, the
                specific register of which I'm letting the compiler decide on. The "k" prefix is used to specify a 32-bit register. The {...} syntax is for
                supporting different assembly dialects.
                
                What's basically happening is that we're saving and restoring the ebx register manually.
                */
                #if defined(DROPUS_X86) && defined(__PIC__)
                    __asm__ __volatile__ (
                        "xchg{l} {%%}ebx, %k1;"
                        "cpuid;"
                        "xchg{l} {%%}ebx, %k1;"
                        : "=a"(info[0]), "=&r"(info[1]), "=c"(info[2]), "=d"(info[3]) : "a"(fid), "c"(0)
                    );
                #else
                    __asm__ __volatile__ (
                        "cpuid" : "=a"(info[0]), "=b"(info[1]), "=c"(info[2]), "=d"(info[3]) : "a"(fid), "c"(0)
                    );
                #endif
            }
        #else
            #define DROPUS_NO_CPUID
        #endif
    #endif
#else
    #define DROPUS_NO_CPUID
#endif


#if defined(_MSC_VER) && _MSC_VER >= 1500 && (defined(DROPUS_X86) || defined(DROPUS_X64))
    #define DROPUS_HAS_LZCNT_INTRINSIC
#elif (defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)))
    #define DROPUS_HAS_LZCNT_INTRINSIC
#elif defined(__clang__)
    #if __has_builtin(__builtin_clzll) || __has_builtin(__builtin_clzl)
        #define DROPUS_HAS_LZCNT_INTRINSIC
    #endif
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1300
    #define DROPUS_HAS_BYTESWAP16_INTRINSIC
    #define DROPUS_HAS_BYTESWAP32_INTRINSIC
    #define DROPUS_HAS_BYTESWAP64_INTRINSIC
#elif defined(__clang__)
    #if __has_builtin(__builtin_bswap16)
        #define DROPUS_HAS_BYTESWAP16_INTRINSIC
    #endif
    #if __has_builtin(__builtin_bswap32)
        #define DROPUS_HAS_BYTESWAP32_INTRINSIC
    #endif
    #if __has_builtin(__builtin_bswap64)
        #define DROPUS_HAS_BYTESWAP64_INTRINSIC
    #endif
#elif defined(__GNUC__)
    #if ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
        #define DROPUS_HAS_BYTESWAP32_INTRINSIC
        #define DROPUS_HAS_BYTESWAP64_INTRINSIC
    #endif
    #if ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
        #define DROPUS_HAS_BYTESWAP16_INTRINSIC
    #endif
#endif


#ifndef DROPUS_ASSERT
#include <assert.h>
#define DROPUS_ASSERT(expression)           assert(expression)
#endif
#ifndef DROPUS_COPY_MEMORY
#define DROPUS_COPY_MEMORY(dst, src, sz)    memcpy((dst), (src), (sz))
#endif
#ifndef DROPUS_ZERO_MEMORY
#define DROPUS_ZERO_MEMORY(p, sz)           memset((p), 0, (sz))
#endif
#ifndef DROPUS_ZERO_OBJECT
#define DROPUS_ZERO_OBJECT(p)               DROPUS_ZERO_MEMORY((p), sizeof(*(p)))
#endif


/*********************************** 
Endian Management
************************************/
static DROPUS_INLINE dropus_bool32 dropus__is_little_endian()
{
#if defined(DROPUS_X86) || defined(DROPUS_X64)
    return DROPUS_TRUE;
#else
    int n = 1;
    return (*(char*)&n) == 1;
#endif
}

static DROPUS_INLINE dropus_uint16 dropus__swap_endian_uint16(dropus_uint16 n)
{
#ifdef DROPUS_HAS_BYTESWAP16_INTRINSIC
    #if defined(_MSC_VER)
        return _byteswap_ushort(n);
    #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap16(n);
    #else
        #error "This compiler does not support the byte swap intrinsic."
    #endif
#else
    return ((n & 0xFF00) >> 8) |
           ((n & 0x00FF) << 8);
#endif
}

static DROPUS_INLINE dropus_uint32 dropus__swap_endian_uint32(dropus_uint32 n)
{
#ifdef DROPUS_HAS_BYTESWAP32_INTRINSIC
    #if defined(_MSC_VER)
        return _byteswap_ulong(n);
    #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap32(n);
    #else
        #error "This compiler does not support the byte swap intrinsic."
    #endif
#else
    return ((n & 0xFF000000) >> 24) |
           ((n & 0x00FF0000) >>  8) |
           ((n & 0x0000FF00) <<  8) |
           ((n & 0x000000FF) << 24);
#endif
}

static DROPUS_INLINE dropus_uint64 dropus__swap_endian_uint64(dropus_uint64 n)
{
#ifdef DROPUS_HAS_BYTESWAP64_INTRINSIC
    #if defined(_MSC_VER)
        return _byteswap_uint64(n);
    #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap64(n);
    #else
        #error "This compiler does not support the byte swap intrinsic."
    #endif
#else
    return ((n & (dropus_uint64)0xFF00000000000000) >> 56) |
           ((n & (dropus_uint64)0x00FF000000000000) >> 40) |
           ((n & (dropus_uint64)0x0000FF0000000000) >> 24) |
           ((n & (dropus_uint64)0x000000FF00000000) >>  8) |
           ((n & (dropus_uint64)0x00000000FF000000) <<  8) |
           ((n & (dropus_uint64)0x0000000000FF0000) << 24) |
           ((n & (dropus_uint64)0x000000000000FF00) << 40) |
           ((n & (dropus_uint64)0x00000000000000FF) << 56);
#endif
}


static DROPUS_INLINE dropus_uint16 dropus__be2host_16(dropus_uint16 n)
{
#ifdef __linux__
    return be16toh(n);
#else
    if (dropus__is_little_endian()) {
        return dropus__swap_endian_uint16(n);
    }

    return n;
#endif
}

static DROPUS_INLINE dropus_uint32 dropus__be2host_32(dropus_uint32 n)
{
#ifdef __linux__
    return be32toh(n);
#else
    if (dropus__is_little_endian()) {
        return dropus__swap_endian_uint32(n);
    }

    return n;
#endif
}

static DROPUS_INLINE dropus_uint64 dropus__be2host_64(dropus_uint64 n)
{
#ifdef __linux__
    return be64toh(n);
#else
    if (dropus__is_little_endian()) {
        return dropus__swap_endian_uint64(n);
    }

    return n;
#endif
}


static DROPUS_INLINE dropus_uint32 dropus__le2host_32(dropus_uint32 n)
{
#ifdef __linux__
    return le32toh(n);
#else
    if (!dropus__is_little_endian()) {
        return dropus__swap_endian_uint32(n);
    }

    return n;
#endif
}


/***********************************************************************************************************************************************************

Low-Level Opus Stream API

************************************************************************************************************************************************************/
#define DROPUS_MAX_FRAME_SIZE_IN_BYTES  1275
#define DROPUS_MAX_PACKET_SIZE_IN_BYTES DROPUS_MAX_FRAME_SIZE_IN_BYTES*DROPUS_MAX_OPUS_FRAMES_PER_PACKET


/*********************************** 
RFC 6716 - Section 3.1 The TOC Byte
************************************/
DROPUS_INLINE dropus_uint8 dropus_toc_config(dropus_uint8 toc)
{
    return (toc & 0xF8) >> 3;
}

DROPUS_INLINE dropus_uint8 dropus_toc_s(dropus_uint8 toc)
{
    return (toc & 0x04) >> 2;
}

DROPUS_INLINE dropus_uint8 dropus_toc_c(dropus_uint8 toc)
{
    return (toc & 0x03);
}

DROPUS_INLINE dropus_mode dropus_toc_config_mode(dropus_uint8 config)
{
    /* Table 2 in RFC 6716 */
    static dropus_mode modes[32] = {
        dropus_mode_silk,   dropus_mode_silk,   dropus_mode_silk, dropus_mode_silk, /*  0...3  */
        dropus_mode_silk,   dropus_mode_silk,   dropus_mode_silk, dropus_mode_silk, /*  4...7  */
        dropus_mode_silk,   dropus_mode_silk,   dropus_mode_silk, dropus_mode_silk, /*  8...11 */
        dropus_mode_hybrid, dropus_mode_hybrid,                                     /* 12...13 */
        dropus_mode_hybrid, dropus_mode_hybrid,                                     /* 14...15 */
        dropus_mode_celt,   dropus_mode_celt,   dropus_mode_celt, dropus_mode_celt, /* 16...19 */
        dropus_mode_celt,   dropus_mode_celt,   dropus_mode_celt, dropus_mode_celt, /* 20...23 */
        dropus_mode_celt,   dropus_mode_celt,   dropus_mode_celt, dropus_mode_celt, /* 24...27 */
        dropus_mode_celt,   dropus_mode_celt,   dropus_mode_celt, dropus_mode_celt  /* 28...31 */
    };

    DROPUS_ASSERT(config < 32);
    return modes[config];
}

DROPUS_INLINE dropus_mode dropus_toc_mode(dropus_uint8 toc)
{
    return dropus_toc_config_mode(dropus_toc_config(toc));
}

DROPUS_INLINE dropus_uint32 dropus_toc_config_sample_rate(dropus_uint8 config)
{
    /* Table 2 with Table 1 in RFC 6716 */
    static dropus_uint32 rates[32] = {
        8000,  8000,  8000,  8000,  /*  0...3  */
        12000, 12000, 12000, 12000, /*  4...7  */
        16000, 16000, 16000, 16000, /*  8...11 */
        24000, 24000,               /* 12...13 */
        48000, 48000,               /* 14...15 */
        8000,  8000,  8000,  8000,  /* 16...19 */
        16000, 16000, 16000, 16000, /* 20...23 */
        24000, 24000, 24000, 24000, /* 24...27 */
        48000, 48000, 48000, 48000  /* 28...31 */
    };

    DROPUS_ASSERT(config < 32);
    return rates[config];
}

DROPUS_INLINE dropus_uint32 dropus_toc_sample_rate(dropus_uint8 toc)
{
    return dropus_toc_config_sample_rate(dropus_toc_config(toc));
}

DROPUS_INLINE dropus_uint32 dropus_toc_sample_rate_ms(dropus_uint8 toc)
{
    return dropus_toc_sample_rate(toc) / 1000;
}

DROPUS_INLINE dropus_uint32 dropus_toc_config_frame_size_in_pcm_frames(dropus_uint8 config)
{
    /* Table 2 with Table 1 in RFC 6716 */
    static dropus_uint32 sizes[32] = {
        80,  160, 320, 480, /*  0...3  */
        120, 240, 480, 720, /*  4...7  */
        160, 320, 640, 960, /*  8...11 */
        240, 480,           /* 12...13 */
        480, 960,           /* 14...15 */
        20,  40,  80,  160, /* 16...19 */
        40,  80,  160, 320, /* 20...23 */
        60,  120, 240, 480, /* 24...27 */
        120, 240, 480, 960  /* 28...31 */
    };

    DROPUS_ASSERT(config < 32);
    return sizes[config];
}

DROPUS_INLINE dropus_uint32 dropus_toc_frame_size_in_pcm_frames(dropus_uint8 toc)
{
    return dropus_toc_config_frame_size_in_pcm_frames(dropus_toc_config(toc));
}


dropus_result dropus_stream_init(dropus_stream* pOpusStream)
{
    if (pOpusStream == NULL) {
        return DROPUS_INVALID_ARGS;
    }

    DROPUS_ZERO_OBJECT(pOpusStream);

    return DROPUS_SUCCESS;
}

dropus_result dropus_stream_decode_packet(dropus_stream* pOpusStream, const void* pData, size_t dataSize)
{
    const dropus_uint8* pRunningData8 = (const dropus_uint8*)pData;
    dropus_uint8 toc; /* Table of Contents byte. */
    dropus_uint16 frameCount;
    dropus_uint16 frameSizes[DROPUS_MAX_OPUS_FRAMES_PER_PACKET];
    dropus_uint32 code;

    if (pOpusStream == NULL || pData == NULL) {
        return DROPUS_INVALID_ARGS;
    }

    DROPUS_ASSERT(DROPUS_MAX_PACKET_SIZE_IN_BYTES < 65536);
    if (dataSize > DROPUS_MAX_PACKET_SIZE_IN_BYTES) {
        return DROPUS_BAD_DATA;
    }

    /* RFC 6716 - Section 3.4 [R1] Packets are at least one byte. */
    if (dataSize < 1) {
        return DROPUS_BAD_DATA;
    }

    /* The TOC byte specifies the structure of the packet. */
    toc = pRunningData8[0];
    pRunningData8 += 1;
    
    /*
    We need to look at the code to know the frames making up the packet are structured. We will do a pre-processing step to
    extract basic information about each frame in the packet.
    */
    code = dropus_toc_c(toc);
    switch (code) {
        case 0: /* RFC 6716 - Section 3.2.2. Code 0: One Frame in the Packet */
        {
            dropus_uint16 frameSize = (dropus_uint16)(dataSize-1);

            /* RFC 6716 - Section 3.4 [R2] No implicit frame length is larger than 1275 bytes. */
            if (frameSize > DROPUS_MAX_FRAME_SIZE_IN_BYTES) {
                return DROPUS_BAD_DATA;
            }

            frameCount = 1;
            frameSizes[0] = frameSize;
        } break;

        case 1: /* RFC 6716 - Section 3.2.3. Code 1: Two Frames in the Packet, Each with Equal Compressed Size */
        {
            dropus_uint16 frameSize;

            /* RFC 6716 - Section 3.4 [R3] Code 1 packets have an odd total length, N, so that (N-1)/2 is an integer. */
            if ((dataSize & 1) != 0) {
                return DROPUS_BAD_DATA;
            }

            frameSize = (dropus_uint16)(dataSize-1)/2;

            /* RFC 6716 - Section 3.4 [R2] No implicit frame length is larger than 1275 bytes. */
            if (frameSize > DROPUS_MAX_FRAME_SIZE_IN_BYTES) {
                return DROPUS_BAD_DATA;
            }

            frameCount = 2;
            frameSizes[0] = frameSize;
            frameSizes[1] = frameSize;
        } break;

        case 2: /* RFC 6716 - Section 3.2.4. Code 2: Two Frames in the Packet, with Different Compressed Sizes */
        {
            dropus_uint8 byte0;
            dropus_uint8 byte1;
            dropus_uint16 frameSize0 = 0;
            dropus_uint16 frameSize1 = 0;
            dropus_uint16 headerByteCount;

            /* RFC 6716 - Section 3.4 [R4] Code 2 packets have enough bytes after the TOC for a valid frame length, and that length is no larger than the number of bytes remaining in the packet. */
            if (dataSize < 2) {
                return DROPUS_BAD_DATA;
            }

            /* RFC 6716 - Section 3.2.1. Frame Length Coding */
            byte0 = pRunningData8[0]; pRunningData8 += 1;
            if (byte0 == 0) {
                /*
                Section 3.2.1 of RFC 6716 says the following:

                    "Any Opus frame in any mode MAY have a length of 0.
                
                This implies to me that this is a valid case. dr_opus is going to handle this by setting the PCM frame count to 0 for this packet.
                */
                frameSize0 = 0;
                frameSize1 = 0;
            } else {
                if (byte0 >= 1 && byte0 <= 251) {
                    frameSize0 = byte0;
                }
                if (byte0 >= 252 && byte0 <= 255) {
                    /* RFC 6716 - Section 3.4 [R4] Code 2 packets have enough bytes after the TOC for a valid frame length, and that length is no larger than the number of bytes remaining in the packet. */
                    if (dataSize < 3) {
                        return DROPUS_BAD_DATA;
                    }

                    byte1 = pRunningData8[0]; pRunningData8 += 1;
                    frameSize0 = (byte1*4) + byte0;
                }

                headerByteCount = (dropus_uint16)(pRunningData8 - (const dropus_uint8*)pData);   /* This is a safe case because the maximum difference will be 3. */

                /* RFC 6716 - Section 3.4 [R2] No implicit frame length is larger than 1275 bytes. */
                if (frameSize0 > DROPUS_MAX_FRAME_SIZE_IN_BYTES) {
                    return DROPUS_BAD_DATA;
                }

                /* RFC 6716 - Section 3.4 [R4] Code 2 packets have enough bytes after the TOC for a valid frame length, and that length is no larger than the number of bytes remaining in the packet. */
                if (((dataSize-headerByteCount)+frameSize0) > dataSize) {
                    return DROPUS_BAD_DATA;
                }

                frameSize1 = (dropus_uint16)(dataSize-headerByteCount-frameSize0);    /* Safe cast because dataSize is guaranteed to be < 65536 at this point since it was checked at the top of this function. */

                /* RFC 6716 - Section 3.4 [R2] No implicit frame length is larger than 1275 bytes. */
                if (frameSize1 > DROPUS_MAX_FRAME_SIZE_IN_BYTES) {
                    return DROPUS_BAD_DATA;
                }

                /* RFC 6716 - Section 3.4 [R4] Code 2 packets have enough bytes after the TOC for a valid frame length, and that length is no larger than the number of bytes remaining in the packet. */
                if (((dataSize-headerByteCount)+frameSize0+frameSize1) > dataSize) {
                    return DROPUS_BAD_DATA;
                }
            }
            
            frameCount = 2;
            frameSizes[0] = frameSize0;
            frameSizes[1] = frameSize1;
        } break;

        case 3: /* RFC 6716 - Section 3.2.5. Code 3: A Signaled Number of Frames in the Packet */
        {
            dropus_uint8  frameCountByte;
            dropus_uint8  v;                /* Is VBR? */
            dropus_uint8  p;                /* Has padding? */
            dropus_uint8  M;                /* Frame count. */
            dropus_uint16 P;                /* The size of the padding. Must never be more than dataSize-2. */
            dropus_uint16 R;                /* The number of bytes remaining in the packet after subtracting the TOC, frame count byte and padding. */
            dropus_uint32 ms;               /* Total length in milliseconds. */
            dropus_uint32 paddingByteCount; /* The number of bytes making up the size of the padding. Only used for validation. */
            
            /*
            RFC 6716 - Section 3.2.5:
                "Code 3 packets MUST have at least 2 bytes [R6,R7]."
            */
            if (dataSize < 2) {
                return DROPUS_BAD_DATA;
            }

            frameCountByte = pRunningData8[0]; pRunningData8 += 1;
            v = (frameCountByte & 0x80) >> 7;
            p = (frameCountByte & 0x40) >> 6;
            M = (frameCountByte & 0x3F);

            /* RFC 6716 - Section 3.4 [R5] Code 3 packets contain at least one frame, but no more than 120 ms of audio total. */
            ms = (M * dropus_toc_frame_size_in_pcm_frames(toc)) / dropus_toc_sample_rate_ms(toc);
            if (M < 1 || ms > 120) {
                return DROPUS_BAD_DATA;
            }

            /* Sanity check to ensure the frame count is never greather than the maximum allowed. */
            if (M > DROPUS_MAX_OPUS_FRAMES_PER_PACKET)  {
                return DROPUS_BAD_DATA;
            }

            /* Padding bytes. Need to run this in a loop. */
            P = 0;
            paddingByteCount = 0;
            if (p != 0) {
                for (size_t iPaddingByte = 0; iPaddingByte < dataSize-2; ++iPaddingByte) {
                    dropus_uint8 paddingByte = pRunningData8[0]; pRunningData8 += 1;
                    P += paddingByte;
                    paddingByteCount += 1;

                    /* A padding byte not equal to 255 signals the last padding byte. */
                    if (paddingByte == 255) {
                        /* There must be an additional byte available in this case. */
                        if (iPaddingByte+1 >= dataSize-2) {
                            return DROPUS_BAD_DATA;
                        }
                    } else {
                        break;  /* Reached the end of the padding bytes. */
                    }
                }
            }

            /* Safety check. */
            if (P > dataSize-2) {
                return DROPUS_BAD_DATA;
            }

            /* R = bytes remaining. */
            R = (dropus_uint16)(dataSize-2-P);

            if (v == 0) {
                /* CBR */
                dropus_uint16 frameSize = R/M;

                /* RFC 6716 - Section 3.4 [R2] No implicit frame length is larger than 1275 bytes. */
                if (frameSize > DROPUS_MAX_FRAME_SIZE_IN_BYTES) {
                    return DROPUS_BAD_DATA;
                }

                /* RFC 6716 - Section 3.4 [R6] ... */
                if (dataSize < 2) {                     /* ... The length of a CBR code 3 packet, N, is at least two bytes ... */
                    return DROPUS_BAD_DATA;
                }
                if (paddingByteCount+P > dataSize-2) {  /* ... the number of bytes added to indicate the padding size plus the trailing padding bytes themselves, P, is no more than N-2 ... */
                    return DROPUS_BAD_DATA;
                }
                if (frameSize*M != (dataSize-2-P)) {    /* ... the frame count, M, satisfies the constraint that (N-2-P) is a non-negative integer multiple of M ... */
                    return DROPUS_BAD_DATA;
                }

                frameCount = M;
                for (dropus_uint16 iFrame = 0; iFrame < frameCount; ++iFrame) {
                    frameSizes[frameSize];
                }
            } else {
                /* VBR */
                dropus_uint16 totalFrameSizeExceptLast = 0; /* Used later for checking [R7]. */
                dropus_uintptr headerSizeInBytes;           /* For validation and deriving the size of the last frame. */

                frameCount = M;
                for (dropus_uint16 iFrame = 0; iFrame < frameCount-1; ++iFrame) {
                    dropus_uint8 byte0;
                    dropus_uint8 byte1;

                    if ((dropus_uintptr)(pRunningData8 - (const dropus_uint8*)pData) < dataSize) {
                        return DROPUS_BAD_DATA; /* Ran out of data in the packet. Implicitly handles part of [R7]. */
                    }

                    byte0 = pRunningData8[0]; pRunningData8 += 1;
                    if (byte0 == 0) {
                        frameSizes[iFrame] = 0;
                    } else {
                        if (byte0 >= 1 && byte0 <= 251) {
                            frameSizes[iFrame] = byte0;
                        }
                        if (byte0 >= 252 && byte0 <= 255) {
                            if ((dropus_uintptr)(pRunningData8 - (const dropus_uint8*)pData) < dataSize) {
                                return DROPUS_BAD_DATA; /* Ran out of data in the packet. Implicitly handles part of [R7]. */
                            }

                            byte1 = pRunningData8[0]; pRunningData8 += 1;
                            frameSizes[iFrame] = (byte1*4) + byte0;

                            /* RFC 6716 - Section 3.4 [R2] No implicit frame length is larger than 1275 bytes. */
                            if (frameSizes[iFrame] > DROPUS_MAX_FRAME_SIZE_IN_BYTES) {
                                return DROPUS_BAD_DATA;
                            }
                        }
                    }

                    totalFrameSizeExceptLast += frameSizes[iFrame];
                }

                headerSizeInBytes = (dropus_uintptr)(pRunningData8 - (const dropus_uint8*)pData);

                /*
                RFC 6716 - Section 3.4 [R6]
                    VBR code 3 packets are large enough to contain all the header
                    bytes (TOC byte, frame count byte, any padding length bytes,
                    and any frame length bytes), plus the length of the first M-1
                    frames, plus any trailing padding bytes.
                */
                if ((headerSizeInBytes + totalFrameSizeExceptLast + P) > dataSize) {
                    return DROPUS_BAD_DATA;
                }

                /* The size of the last frame is derived. */
                frameSizes[frameCount-1] = (dropus_uint16)(dataSize - headerSizeInBytes - totalFrameSizeExceptLast - P); /* Safe cast thanks to the myriad of validation done beforehand. */

                /* RFC 6716 - Section 3.4 [R2] No implicit frame length is larger than 1275 bytes. */
                if (frameSizes[frameCount-1] > DROPUS_MAX_FRAME_SIZE_IN_BYTES) {
                    return DROPUS_BAD_DATA;
                }
            }
        } break;

        /* Will never hit this, but need the default to keep some compilers quiet. */
        default: return DROPUS_BAD_DATA;
    }

    /* At this point, pRunningData8 should be sitting on the first byte of the first frame in the packet. */

    /* TODO: Decoding. */

    return DROPUS_SUCCESS;
}



/***********************************************************************************************************************************************************

High-Level Opus Decoding API

************************************************************************************************************************************************************/

dropus_bool32 dropus_init_internal(dropus* pOpus, dropus_read_proc onRead, dropus_seek_proc onSeek, void* pUserData)
{
    DROPUS_ASSERT(pOpus != NULL);
    DROPUS_ASSERT(onRead != NULL);

    /* Must always have an onRead callback. */
    if (onRead == NULL) {
        return DROPUS_FALSE;
    }

    pOpus->onRead = onRead;
    pOpus->onSeek = onSeek;
    pOpus->pUserData = pUserData;

    /* TODO: Implement me. */
    
    return DROPUS_TRUE;
}

dropus_bool32 dropus_init(dropus* pOpus, dropus_read_proc onRead, dropus_seek_proc onSeek, void* pUserData)
{
    if (pOpus == NULL) {
        return DROPUS_FALSE;    /* Invalid args. */
    }

    DROPUS_ZERO_OBJECT(pOpus);

    return dropus_init_internal(pOpus, onRead, onSeek, pUserData);
}

#ifndef DR_OPUS_NO_STDIO
FILE* dropus_fopen(const char* filename, const char* mode)
{
    FILE* pFile;
#ifdef _MSC_VER
    if (fopen_s(&pFile, filename, mode) != 0) {
        return NULL;
    }
#else
    pFile = fopen(filename, mode);
    if (pFile == NULL) {
        return NULL;
    }
#endif

    return pFile;
}

int dropus_fclose(FILE* pFile)
{
    return fclose(pFile);
}


size_t dropus_on_read_stdio(void* pUserData, void* pBufferOut, size_t bytesToRead)
{
    return fread(pBufferOut, 1, bytesToRead, (FILE*)pUserData);
}

dropus_bool32 dropus_on_seek_stdio(void* pUserData, int offset, dropus_seek_origin origin)
{
    return fseek((FILE*)pUserData, offset, (origin == dropus_seek_origin_current) ? SEEK_CUR : SEEK_SET) == 0;
}

dropus_bool32 dropus_init_file(dropus* pOpus, const char* pFilePath)
{
    FILE* pFile;
    dropus_bool32 successful;

    if (pOpus == NULL) {
        return DROPUS_FALSE;    /* Invalid args. */
    }

    DROPUS_ZERO_OBJECT(pOpus);

    if (pFilePath == NULL || pFilePath[0] == '\0') {
        return DROPUS_FALSE;    /* Invalid args. */
    }

    pFile = dropus_fopen(pFilePath, "rb");
    if (pFile == NULL) {
        return DROPUS_FALSE;    /* Failed to open file. */
    }

    pOpus->pFile = (void*)pFile;

    successful = dropus_init_internal(pOpus, dropus_on_read_stdio, dropus_on_seek_stdio, NULL);
    if (!successful) {
        dropus_fclose(pFile);
        return DROPUS_FALSE;
    }
    
    return DROPUS_TRUE;
}
#endif

static size_t dropus_on_read_memory(void* pUserData, void* pBufferOut, size_t bytesToRead)
{
    size_t bytesRemaining;
    dropus* pOpus = (dropus*)pUserData;

    DROPUS_ASSERT(pOpus != NULL);
    DROPUS_ASSERT(pOpus->memory.dataSize >= pOpus->memory.currentReadPos);

    bytesRemaining = pOpus->memory.dataSize - pOpus->memory.currentReadPos;
    if (bytesToRead > bytesRemaining) {
        bytesToRead = bytesRemaining;
    }

    if (bytesToRead > 0) {
        DROPUS_COPY_MEMORY(pBufferOut, pOpus->memory.pData + pOpus->memory.currentReadPos, bytesToRead);
        pOpus->memory.currentReadPos += bytesToRead;
    }

    return bytesToRead;
}

static dropus_bool32 dropus_on_seek_memory(void* pUserData, int byteOffset, dropus_seek_origin origin)
{
    dropus* pOpus = (dropus*)pUserData;
    DROPUS_ASSERT(pOpus != NULL);

    if (origin == dropus_seek_origin_current) {
        if (byteOffset > 0) {
            if (pOpus->memory.currentReadPos + byteOffset > pOpus->memory.dataSize) {
                byteOffset = (int)(pOpus->memory.dataSize - pOpus->memory.currentReadPos);  /* Trying to seek too far forward. */
            }
        } else {
            if (pOpus->memory.currentReadPos < (size_t)-byteOffset) {
                byteOffset = -(int)pOpus->memory.currentReadPos;  /* Trying to seek too far backwards. */
            }
        }

        /* This will never underflow thanks to the clamps above. */
        pOpus->memory.currentReadPos += byteOffset;
    } else {
        if ((dropus_uint32)byteOffset <= pOpus->memory.dataSize) {
            pOpus->memory.currentReadPos = byteOffset;
        } else {
            pOpus->memory.currentReadPos = pOpus->memory.dataSize;  /* Trying to seek too far forward. */
        }
    }

    return DROPUS_TRUE;
}

dropus_bool32 dropus_init_memory(dropus* pOpus, const void* pData, size_t dataSize)
{
    if (pOpus == NULL) {
        return DROPUS_FALSE;    /* Invalid args. */
    }

    DROPUS_ZERO_OBJECT(pOpus);

    if (pData == NULL || dataSize == 0) {
        return DROPUS_FALSE;    /* Invalid args. */
    }

    pOpus->memory.pData = (const dropus_uint8*)pData;
    pOpus->memory.dataSize = dataSize;
    pOpus->memory.currentReadPos = 0;

    return dropus_init_internal(pOpus, dropus_on_read_memory, dropus_on_seek_memory, NULL);
}


void dropus_uninit(dropus* pOpus)
{
    if (pOpus == NULL) {
        return;
    }

#ifndef DR_OPUS_NO_STDIO
    /* Since dr_opus manages the stdio FILE object make sure it's closed on uninitialization. */
    if (pOpus->pFile != NULL) {
        dropus_fclose((FILE*)pOpus->pFile);
    }
#endif
}

#endif  /* DR_OPUS_IMPLEMENTATION */

/*
This software is available as a choice of the following licenses. Choose
whichever you prefer.

===============================================================================
ALTERNATIVE 1 - Public Domain (www.unlicense.org)
===============================================================================
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>

===============================================================================
ALTERNATIVE 2 - MIT No Attribution
===============================================================================
Copyright 2018 David Reid

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*#define DR_OPUS_DEBUGGING*/

#define DR_OPUS_IMPLEMENTATION
#include "../../wip/dr_opus.h"
#include "../common/dr_common.c"

#include <stdio.h>

/* Forward declare our debugging entry point if necessary. */
#ifdef DR_OPUS_DEBUGGING
int main_debugging(int argc, char** argv);
#endif

dropus_result test_standard_vector(const char* pFilePath)
{
    dropus_result result;
    dropus_stream stream;
    void* pFileData;
    size_t fileSize;
    const dropus_uint8* pRunningData8;
    size_t runningPos;
    dropus_uint32 iPacket = 0;  /* For error reporting. */


    /* Load the entire file into memory first. */
    pFileData = dr_open_and_read_file(pFilePath, &fileSize);
    if (pFileData == NULL) {
        return DROPUS_ERROR;    /* File not found. */
    }

    /* Now initialize the stream in preparation for decoding packets. */
    result = dropus_stream_init(&stream);
    if (result != DROPUS_SUCCESS) {
        free(pFileData);
        return result;
    }

    /*
    Here is where we scan the test vector. The way the file is structured is quite simple. It is made up of a list of Opus packets, each
    of which include an 8 byte header where the first 4 bytes is the size in bytes of the Opus packet (little endian) and the next 4 bytes
    contain the 32-bit range state which we'll use for validation.
    */
    pRunningData8 = (const dropus_uint8*)pFileData;
    runningPos    = 0;

    /* For each packet... */
    while (runningPos < fileSize) {
        dropus_result decodeResult;
        dropus_uint32 packetSize;
        dropus_uint32 rangeState;

        memcpy(&packetSize, pRunningData8 + 0, 4);
        memcpy(&rangeState, pRunningData8 + 4, 4);

        packetSize = dropus__be2host_32(packetSize);
        rangeState = dropus__be2host_32(rangeState);

        pRunningData8 += 8;
        runningPos    += 8;

        /* Safety. Break if we've run out of data. */
        if ((runningPos + packetSize) > fileSize) {
            printf("WARNING: Ran out of data before the end of the file.\n");
            break;
        }

        decodeResult = dropus_stream_decode_packet(&stream, pRunningData8, packetSize);
        if (result != DROPUS_SUCCESS) {
            result = DROPUS_ERROR;
            printf("Failed to decode packet %d\n", iPacket);
        }

        pRunningData8 += packetSize;
        runningPos    += packetSize;
        iPacket += 1;
    }

    free(pFileData);
    return result;
}

dropus_result test_standard_vectors_folder(const char* pFolderPath)
{
    dropus_bool32 foundError = DROPUS_FALSE;
    dr_file_iterator iteratorState;
    dr_file_iterator* pFile;

    pFile = dr_file_iterator_begin(pFolderPath, &iteratorState);
    while (pFile != NULL) {
        /* Only look at files with the extension "bit". */
        if (dr_extension_equal(pFile->relativePath, "bit")) {
            if (test_standard_vector(pFile->absolutePath) != DROPUS_SUCCESS) {
                foundError = DROPUS_TRUE;
            }
        }

        pFile = dr_file_iterator_next(pFile);
    }

    if (foundError) {
        return DROPUS_ERROR;
    } else {
        return DROPUS_SUCCESS;
    }
}

dropus_result test_standard_vectors()
{
    dropus_bool32 foundError = DROPUS_FALSE;

    /* Two groups of standard test vectors. The original vectors and the "new" vectors. */

    /* Original vectors. */
    if (test_standard_vectors_folder("testvectors/opus/opus_testvectors") != DROPUS_SUCCESS) {
        foundError = DROPUS_TRUE;
    }

    /* New vectors. */
    if (test_standard_vectors_folder("testvectors/opus/opus_newvectors") != DROPUS_SUCCESS) {
        foundError = DROPUS_TRUE;
    }

    if (foundError) {
        return DROPUS_ERROR;
    } else {
        return DROPUS_SUCCESS;
    }
}

dropus_result test_ogg_vectors()
{
    dropus_result result;

    /* Ogg Opus test vectors are in the "oggopus" folder. */
    result = DROPUS_SUCCESS;

    return result;
}

int main(int argc, char** argv)
{
    dropus_result result;

    /* Do debugging stuff first. */
#ifdef DR_OPUS_DEBUGGING
    main_debugging(argc, argv);
#endif
    
    /* Test standard vectors first. */
    result = test_standard_vectors();
    if (result != DROPUS_SUCCESS) {
        /*return (int)result;*/
    }

    /* Ogg Opus. */
    result = test_ogg_vectors();
    if (result != DROPUS_SUCCESS) {
        /*return (int)result;*/
    }

    (void)argc;
    (void)argv;
    return 0;
}

#ifdef DR_OPUS_DEBUGGING
int main_debugging(int argc, char** argv)
{
    dr_file_iterator iterator;
    dr_file_iterator* pFile = dr_file_iterator_begin("testvectors/opus/oggopus", &iterator);
    while (pFile != NULL) {
        if (pFile->isDirectory) {
            printf("DIRECTORY: %s : %s\n", pFile->relativePath, pFile->absolutePath);
        } else {
            printf("FILE: %s : %s\n", pFile->relativePath, pFile->absolutePath);
        }
        
        pFile = dr_file_iterator_next(pFile);
    }

    (void)argc;
    (void)argv;
    return 0;
}
#endif

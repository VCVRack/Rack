
#ifdef _WIN32
#include <windows.h>
#endif

typedef unsigned int dr_bool32;

/*
String Helpers
*/
errno_t dr_append_path(char* dst, size_t dstSize, const char* base, const char* other)
{
    errno_t err;
    size_t  len;

    /* TODO: Return the correct error codes here. */
    if (dst == NULL) {
        return -1;
    }
    if (base == NULL || other == NULL) {
        return -1;
    }

    err = strcpy_s(dst, dstSize, base);
    if (err != 0) {
        return err;
    }

    len = strlen(dst);
    if (len > 0) {
        /* Append the slash if required. */
        if (dst[len-1] != '/' && dst[len-1] != '\\') {
            err = strcat_s(dst, dstSize, "/");
            if (err != 0) {
                dst[0] = '\0';
                return err;
            }

            len += 1;   /* +1 to the length to account for the slash. */
        }
    }

    err = strcat_s(dst, dstSize, other);
    if (err != 0) {
        dst[0] = '\0';
        return err;
    }

    /* Success. */
    return 0;
}

const char* dr_extension(const char* path)
{
    const char* extension = path;
    const char* lastoccurance = NULL;

    if (path == NULL) {
        return NULL;
    }

    /* Just find the last '.' and return. */
    while (extension[0] != '\0') {
        if (extension[0] == '.') {
            extension    += 1;
            lastoccurance = extension;
        }

        extension += 1;
    }

    return (lastoccurance != 0) ? lastoccurance : extension;
}

dr_bool32 dr_extension_equal(const char* path, const char* extension)
{
    if (path == NULL || extension == NULL) {
        return 0;
    }

    const char* ext1 = extension;
    const char* ext2 = dr_extension(path);

#ifdef _MSC_VER
    return _stricmp(ext1, ext2) == 0;
#else
    return strcasecmp(ext1, ext2) == 0;
#endif
}




/*
File Iterator

dr_file_iterator state;
dr_file_iterator* pFile = dr_file_iterator_begin("the/folder/path", &state);
while (pFile != NULL) {
    // Do something with pFile.
    pFile = dr_file_iterator_next(pFile);
}

Limitations:
  - Only supports file paths up to 256 characters.
*/
typedef struct
{
    char folderPath[256];
    char relativePath[256]; /* Relative to the original folder path. */
    char absolutePath[256]; /* Concatenation of folderPath and relativePath. */
    dr_bool32 isDirectory;
#ifdef _WIN32
    HANDLE hFind;
#else
#endif
} dr_file_iterator;

dr_file_iterator* dr_file_iterator_begin(const char* pFolderPath, dr_file_iterator* pState)
{
#ifdef _WIN32
    char searchQuery[MAX_PATH];
    unsigned int searchQueryLength;
    WIN32_FIND_DATAA ffd;
    HANDLE hFind;
#endif

    if (pState == NULL) {
        return NULL;
    }

    memset(pState, 0, sizeof(*pState));

    if (pFolderPath == NULL) {
        return NULL;
    }

#ifdef _WIN32
    strcpy_s(searchQuery, sizeof(searchQuery), pFolderPath);

    searchQueryLength = (unsigned int)strlen(searchQuery);
    if (searchQueryLength >= MAX_PATH - 3) {
        return NULL; /* Path is too long. */
    }

    searchQuery[searchQueryLength + 0] = '\\';
    searchQuery[searchQueryLength + 1] = '*';
    searchQuery[searchQueryLength + 2] = '\0';

    hFind = FindFirstFileA(searchQuery, &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        return NULL; /* Failed to begin search. */
    }

    /* Skip past "." and ".." directories. */
    while (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0) {
        if (!FindNextFileA(hFind, &ffd)) {
            return NULL;    /* Couldn't find anything. */
        }
    }

    pState->hFind = hFind;


    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        pState->isDirectory = 1;
    } else {
        pState->isDirectory = 0;
    }

    strcpy_s(pState->relativePath, sizeof(pState->relativePath), ffd.cFileName);
#else
    return NULL;    /* Not yet implemented. */
#endif

    /* Getting here means everything was successful. We can now set some state before returning. */
    strcpy_s(pState->folderPath, sizeof(pState->folderPath), pFolderPath);
    dr_append_path(pState->absolutePath, sizeof(pState->absolutePath), pState->folderPath, pState->relativePath);

    return pState;
}

dr_file_iterator* dr_file_iterator_next(dr_file_iterator* pState)
{
#ifdef _WIN32
    WIN32_FIND_DATAA ffd;
#endif

    if (pState == NULL) {
        return NULL;
    }

#ifdef _WIN32
    if (!FindNextFileA(pState->hFind, &ffd)) {
        /* Couldn't find anything else. */
        FindClose(pState->hFind);
        return NULL;
    }

    /* Found something. */
    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        pState->isDirectory = 1;
    } else {
        pState->isDirectory = 0;
    }

    strcpy_s(pState->relativePath, sizeof(pState->relativePath), ffd.cFileName);
#else
    return NULL; /* Not yet implemented. */
#endif

    /* Success */
    dr_append_path(pState->absolutePath, sizeof(pState->absolutePath), pState->folderPath, pState->relativePath);
    return pState;
}


/*
File Management

Free file data with free().
*/
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

void* dr_open_and_read_file_with_extra_data(const char* pFilePath, size_t* pFileSizeOut, size_t extraBytes)
{
    FILE* pFile;
    size_t fileSize;
    size_t bytesRead;
    void* pFileData;

    /* Safety. */
    if (pFileSizeOut) {
        *pFileSizeOut = 0;   
    }

    if (pFilePath == NULL) {
        return NULL;
    }

    /* TODO: Use 64-bit versions of the FILE APIs. */

    pFile = dr_fopen(pFilePath, "rb");
    if (pFile == NULL) {
        return NULL;
    }

    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    if (fileSize + extraBytes > SIZE_MAX) {
        fclose(pFile);
        return NULL;    /* File is too big. */
    }

    pFileData = malloc((size_t)fileSize + extraBytes);    /* <-- Safe cast due to the check above. */
    if (pFileData == NULL) {
        fclose(pFile);
        return NULL;    /* Failed to allocate memory for the file. Good chance the file is too big. */
    }

    bytesRead = fread(pFileData, 1, (size_t)fileSize, pFile);
    if (bytesRead != fileSize) {
        free(pFileData);
        fclose(pFile);
        return NULL;    /* Failed to read every byte from the file. */
    }

    fclose(pFile);

    if (pFileSizeOut) {
        *pFileSizeOut = (size_t)fileSize;
    }

    return pFileData;
}

void* dr_open_and_read_file(const char* pFilePath, size_t* pFileSizeOut)
{
    return dr_open_and_read_file_with_extra_data(pFilePath, pFileSizeOut, 0);
}
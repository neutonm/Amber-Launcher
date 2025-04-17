#include "core/common.h"
#include <commands/archive.h>

#include <core/command.h>

#include <ext/miniz.h>

#include <lua.h>
#include <lauxlib.h>

#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

/* Define the maximum path length */
#define MAX_PATH_LEN 1024

/* Helper function to create directories recursively */
static int 
_CreateDirectories(const char *path) 
{
    char temp[MAX_PATH_LEN];
    size_t len;
    int status = 0;

    /* Copy the path to a temporary buffer */
    strncpy(temp, path, sizeof(temp));
    temp[sizeof(temp) - 1] = '\0';

    len = strlen(temp);
    if (temp[len - 1] == '/')
        temp[len - 1] = '\0';

    for (size_t i = 1; i < strlen(temp); i++) {
        if (temp[i] == '/') {
            temp[i] = '\0';
            /* Check if the directory exists */
            if (mkdir(temp, 0755) != 0) {
                if (errno != EEXIST) {
                    perror("mkdir");
                    return -1;
                }
            }
            temp[i] = '/';
        }
    }

    /* Create the final directory */
    if (mkdir(temp, 0755) != 0) {
        if (errno != EEXIST) {
            perror("mkdir");
            return -1;
        }
    }

    return status;
}

/* Helper function to join two paths */
static void 
_join_paths(const char *path1, const char *path2, char *output, size_t size) 
{
    snprintf(output, size, "%s/%s", path1, path2);
}

static CBOOL
_ExtractArchive(const char* sArchivePath, const char* sExtractPath)
{
    int dFileCount;
    int i;
    char file_path[MAX_PATH_LEN];
    mz_zip_archive tZipArchive;
    memset(&tZipArchive, 0, sizeof(tZipArchive));

    /* Initialize the Zip reader */
    if (!mz_zip_reader_init_file(&tZipArchive, sArchivePath, 0)) 
    {
        fprintf(stderr, "Failed to initialize zip archive: %s\n", sArchivePath);
        return CFALSE;
    }

    /* Get the number of files in the archive */
    dFileCount = mz_zip_reader_get_num_files(&tZipArchive);
    if (dFileCount < 0) 
    {
        fprintf(stderr, "Failed to get number of files in zip archive.\n");
        mz_zip_reader_end(&tZipArchive);
        return CFALSE;
    }

    /* Iterate through each file in the archive */
    for (i = 0; i < dFileCount; i++) 
    {
        char *last_slash;
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&tZipArchive, i, &file_stat)) {
            fprintf(stderr, "Failed to get file stat for index %d.\n", i);
            continue;
        }

        /* Skip directories */
        if (mz_zip_reader_is_file_a_directory(&tZipArchive, i)) 
        {
            /* Construct the full directory path */
            char dir_path[MAX_PATH_LEN];
            _join_paths(sExtractPath, file_stat.m_filename, dir_path, sizeof(dir_path));

            /* Create the directory */
            if (_CreateDirectories(dir_path) != 0) 
            {
                fprintf(stderr, "Failed to create directory: %s\n", dir_path);
            }

            continue;
        }

        /* Construct the full file path */
        _join_paths(sExtractPath, file_stat.m_filename, file_path, sizeof(file_path));

        /* Extract the directory part from the file path */
        last_slash = strrchr(file_path, '/');
        if (last_slash != NULL) {
            /* Temporarily terminate the string to isolate the directory path */
            *last_slash = '\0';

            /* Create the necessary directories */
            if (_CreateDirectories(file_path) != 0) 
            {
                fprintf(stderr, "Failed to create directories for: %s\n", file_path);
            }

            /* Restore the slash */
            *last_slash = '/';
        }

        /* Extract the file to the target path */
        if (!mz_zip_reader_extract_to_file(&tZipArchive, i, file_path, 0)) {
            fprintf(stderr, "Failed to extract file: %s\n", file_path);
        } else {
            printf("Extracted: %s\n", file_path);
        }
    }

    /* Close the Zip reader */
    mz_zip_reader_end(&tZipArchive);

    return CTRUE;
}

CAPI CBOOL
SCommand_Callback_Archive(const SCommand* pSelf, const SCommandArg* pArgs, const unsigned int dNumArgs)
{
    const char *zip_filename = "assets/archive2.zip";
    const char *extract_path = "tests/extract";
    CBOOL bResult;

    UNUSED(pSelf);
    UNUSED(pArgs);
    UNUSED(dNumArgs);

    bResult = _ExtractArchive(zip_filename, extract_path);

    return bResult;
}

CAPI int
LUA_ArchiveExtract(struct lua_State* L)
{
    const char* sZipPath        = luaL_checkstring(L, 1);
    const char* sExtractPath    = luaL_checkstring(L, 2);
    CBOOL bResult;

    bResult = _ExtractArchive(sZipPath, sExtractPath);

    lua_pushboolean(L, bResult);

    return 1;
}

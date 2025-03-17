#include <commands/music.h>

#include <core/command.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
/* Windows-specific headers */
#include <windows.h>
#else
/* POSIX headers */
#include <dirent.h>
#endif

#define MINIMP3_IMPLEMENTATION
#define MINIMP3_ONLY_MP3
#include <ext/minimp3.h>

#define MINIMP3_MAX_SAMPLES_PER_FRAME (1152*2)

#define MAX_PATH_LENGTH 260  /* Maximum path length */
#define MAX_FILENAME_LENGTH 80
#define MAX_DIRPATH_LENGTH 160

#ifdef _WIN32
    #define DIR_SEP '\\'
#else
    #define DIR_SEP '/'
#endif

/* WAV header structure */
typedef struct {
    char riff[4];             /* "RIFF" */
    unsigned int size;        /* File size - 8 */
    char wave[4];             /* "WAVE" */
    char fmt[4];              /* "fmt " */
    unsigned int fmt_size;    /* Size of fmt chunk (16 for PCM) */
    unsigned short format;    /* Format code (1 for PCM) */
    unsigned short channels;  /* Number of channels */
    unsigned int sample_rate; /* Sample rate */
    unsigned int byte_rate;   /* Bytes per second */
    unsigned short block_align;    /* Block align */
    unsigned short bits_per_sample;/* Bits per sample */
    char data[4];             /* "data" */
    unsigned int data_size;   /* Size of data chunk */
} WAVHeader;

/* Function to check if a string represents a number */
static int 
is_number(const char *str) {
    while (*str) {
        if (*str < '0' || *str > '9')
            return 0;
        str++;
    }
    return 1;
}

/* static void 
_ConstructFullPath(char *full_path, size_t size, const char *dir, const char *filename) 
{
    size_t dir_len = strlen(dir);
    
    if (dir_len > 0 && (dir[dir_len - 1] == '/' || dir[dir_len - 1] == '\\')) {
        snprintf(full_path, size, "%s%s", dir, filename);
    } else {
        snprintf(full_path, size, "%s%c%s", dir, DIR_SEP, filename);
    }
} */

/* Function to process an MP3 file */
static void 
_ConvertMP3ToWAV(const char *file_name) 
{
    char base_name[MAX_PATH_LENGTH];
    char dir_name[MAX_PATH_LENGTH];
    FILE *mp3_file;
    long mp3_size;
    unsigned char *mp3_data;
    size_t read_size;
    mp3dec_t mp3d;
    char wav_name[MAX_PATH_LENGTH];
    char *dot;
    FILE *wav_file;
    WAVHeader header;
    size_t mp3_data_pos;
    int samples;
    mp3dec_frame_info_t info;
    short pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
    unsigned int total_samples;
    char bak_name[MAX_PATH_LENGTH];

    /* Extract directory path and base name from file_name */
    const char *base = file_name;
    const char *p = file_name;
    const char *last_sep = NULL;
    while (*p) {
        if (*p == '/' || *p == '\\') {
            last_sep = p;
            base = p + 1;  // Move past the '/' or '\\'
        }
        p++;
    }

    /* Copy directory path */
    if (last_sep) {
        size_t dir_len = last_sep - file_name + 1; // Include the separator
        if (dir_len >= MAX_PATH_LENGTH) {
            fprintf(stderr, "Directory path too long: %s\n", file_name);
            return;
        }
        strncpy(dir_name, file_name, dir_len);
        dir_name[dir_len] = '\0';
    } else {
        /* No directory in file_name */
        dir_name[0] = '\0';
    }

    /* Remove file extension to get base_name */
    strncpy(base_name, base, MAX_PATH_LENGTH - 1);
    base_name[MAX_PATH_LENGTH - 1] = '\0';
    dot = strrchr(base_name, '.');
    if (dot) {
        *dot = '\0';  // Remove the extension
    }

    /* Check if base name is a number */
    if (is_number(base_name)) {
        /* Open MP3 file */
        mp3_file = fopen(file_name, "rb");
        if (!mp3_file) {
            fprintf(stderr, "Failed to open %s\n", file_name);
            return;
        }

        /* Get MP3 file size */
        fseek(mp3_file, 0, SEEK_END);
        mp3_size = ftell(mp3_file);
        rewind(mp3_file);

        /* Allocate memory for MP3 data */
        mp3_data = (unsigned char *)malloc(mp3_size);
        if (!mp3_data) {
            fprintf(stderr, "Failed to allocate memory for %s\n", file_name);
            fclose(mp3_file);
            return;
        }

        /* Read MP3 data */
        read_size = fread(mp3_data, 1, mp3_size, mp3_file);
        fclose(mp3_file);
        if (read_size != (size_t)mp3_size) {
            fprintf(stderr, "Failed to read %s\n", file_name);
            free(mp3_data);
            return;
        }

        /* Initialize decoder */
        mp3dec_init(&mp3d);

        /* Construct WAV file name with directory path */
        snprintf(wav_name, 4096, "%s%s.wav", dir_name, base_name);
        wav_file = fopen(wav_name, "wb");
        if (!wav_file) {
            fprintf(stderr, "Failed to open %s\n", wav_name);
            free(mp3_data);
            return;
        }

        /* Prepare WAV header */
        memcpy(header.riff, "RIFF", 4);
        memcpy(header.wave, "WAVE", 4);
        memcpy(header.fmt, "fmt ", 4);
        memcpy(header.data, "data", 4);

        header.fmt_size = 16;
        header.format = 1; /* PCM */
        header.channels = 0; /* To be set later */
        header.sample_rate = 0; /* To be set later */
        header.bits_per_sample = 16; /* Using 16 bits per sample */
        header.byte_rate = 0; /* To be calculated */
        header.block_align = 0; /* To be calculated */
        header.data_size = 0; /* To be updated */
        header.size = 0; /* To be updated */

        /* Write placeholder header */
        fwrite(&header, sizeof(WAVHeader), 1, wav_file);

        /* Decode MP3 data */
        mp3_data_pos = 0;
        total_samples = 0;

        while (mp3_data_pos < read_size) {
            samples = mp3dec_decode_frame(&mp3d, mp3_data + mp3_data_pos, (int)(read_size - mp3_data_pos), pcm, &info);
            mp3_data_pos += info.frame_bytes;

            if (samples > 0) {
                if (header.channels == 0) {
                    /* Set header values based on first frame */
                    header.channels = (short)info.channels;
                    header.sample_rate = info.hz;
                    header.byte_rate = header.sample_rate * header.channels * header.bits_per_sample / 8;
                    header.block_align = (short)(header.channels * header.bits_per_sample / 8);
                }

                /* Write PCM data to WAV file */
                fwrite(pcm, sizeof(short), samples * info.channels, wav_file);
                total_samples += samples;
            } else if (info.frame_bytes == 0) {
                /* No more frames */
                break;
            }
        }

        /* Update header with correct sizes */
        header.data_size = total_samples * header.channels * sizeof(short);
        header.size = 4 + (8 + header.fmt_size) + (8 + header.data_size);

        /* Write updated header */
        rewind(wav_file);
        fwrite(&header, sizeof(WAVHeader), 1, wav_file);

        /* Clean up */
        fclose(wav_file);
        free(mp3_data);

        /* Rename MP3 file to .bak.mp3 in the same directory */
        snprintf(bak_name, 4096, "%s%s.bak.mp3", dir_name, base_name);
        if (rename(file_name, bak_name) != 0) {
            fprintf(stderr, "Failed to rename %s to %s\n", file_name, bak_name);
        } else {
            printf("Processed %s\n", file_name);
        }
    }
}


static int _test(void) 
{
#ifdef _WIN32
    /* Windows-specific directory traversal */
    WIN32_FIND_DATA find_data;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char dir_search[MAX_PATH_LENGTH];
    const char *file_name;
    size_t len;

    /* Change the directory search path to include the "Music" directory */
    snprintf(dir_search, MAX_PATH_LENGTH, "\\Music\\*.mp3");

    hFind = FindFirstFile(dir_search, &find_data);

    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "No mp3 files found in 'Music'.\n");
        return 1;
    }

    do {
        file_name = find_data.cFileName;
        len = strlen(file_name);
        if (len > 4 && strcmp(file_name + len - 4, ".mp3") == 0) {
            /* Construct the full path to the file */
            char full_path[MAX_PATH_LENGTH];
            snprintf(full_path, MAX_PATH_LENGTH, "\\Music\\%s", file_name);
            _ConvertMP3ToWAV(full_path);
        }
    } while (FindNextFile(hFind, &find_data) != 0);

    FindClose(hFind);

#else
    /* POSIX directory traversal */
    DIR *d;
    struct dirent *dir;
    const char *file_name;
    size_t len;

    d = opendir("./Music");

    if (d) 
    {
        /* Define the directory path */
        const char *dir_path = "./Music/";
        while ((dir = readdir(d)) != NULL) 
        {
            file_name = dir->d_name;
            len = strlen(file_name);
            if (len > 4 && strcmp(file_name + len - 4, ".mp3") == 0) 
            {
                /* Construct the full path to the file */
                char full_path[MAX_PATH_LENGTH];
                snprintf(full_path, 4096, "%s%s", dir_path, file_name);
                printf("Processing: %s\n", full_path);
                _ConvertMP3ToWAV(full_path);
            }
        }
        closedir(d);
    } else {
        fprintf(stderr, "Failed to open directory ./Music.\n");
        return 1;
    }
#endif
    return 0;
}

CAPI void 
SCommand_Callback_ConvertMusic(const SCommand* pSelf, const SCommandArg* pArg)
{
    UNUSED(pSelf);
    UNUSED(pArg);

    _test();
}

int
LUA_ConvertMP3ToWAV(struct lua_State* L)
{
    const char* sPath = luaL_checkstring(L, 1);

    _ConvertMP3ToWAV(sPath);

    return 0;
}


#ifndef PSF_FILE_H
#define PSF_FILE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PsfFileResult {
    PSF_FILE_OK = 0,
    PSF_FILE_ERROR_INVALID_ARGUMENT = -1,
    PSF_FILE_ERROR_OPEN_FAILED = -2,
    PSF_FILE_ERROR_READ_FAILED = -3,
    PSF_FILE_ERROR_BAD_FORMAT = -4,
    PSF_FILE_ERROR_TOO_LARGE = -5
} PsfFileResult;

typedef struct PsfFileInfo {
    uint8_t version;
    uint32_t reserved_size;
    uint32_t program_size;
    uint32_t program_crc;
    uint64_t tag_offset;
    uint32_t tag_size;
    char *tags;
} PsfFileInfo;

PsfFileResult psf_file_read_info(const char *path, PsfFileInfo *out_info);
void psf_file_free_info(PsfFileInfo *info);
const char *psf_file_result_string(PsfFileResult result);
int psf_file_is_psf2(const PsfFileInfo *info);

#ifdef __cplusplus
}
#endif

#endif


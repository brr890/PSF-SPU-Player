#include "psf_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PSF_TAG_MARKER "[TAG]"
#define PSF_MAX_TAG_SIZE (1024u * 1024u)

static uint32_t read_le32(const unsigned char *bytes)
{
    return ((uint32_t)bytes[0]) |
        ((uint32_t)bytes[1] << 8) |
        ((uint32_t)bytes[2] << 16) |
        ((uint32_t)bytes[3] << 24);
}

static PsfFileResult read_exact(FILE *file, void *data, size_t size)
{
    if (size == 0) {
        return PSF_FILE_OK;
    }

    return fread(data, 1, size, file) == size ? PSF_FILE_OK : PSF_FILE_ERROR_READ_FAILED;
}

PsfFileResult psf_file_read_info(const char *path, PsfFileInfo *out_info)
{
    FILE *file;
    unsigned char header[16];
    long file_size;
    uint64_t tag_offset;
    uint32_t tag_size;
    char marker[5];

    if (path == NULL || out_info == NULL) {
        return PSF_FILE_ERROR_INVALID_ARGUMENT;
    }

    memset(out_info, 0, sizeof(*out_info));

    file = fopen(path, "rb");
    if (file == NULL) {
        return PSF_FILE_ERROR_OPEN_FAILED;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return PSF_FILE_ERROR_READ_FAILED;
    }

    file_size = ftell(file);
    if (file_size < 16) {
        fclose(file);
        return PSF_FILE_ERROR_BAD_FORMAT;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return PSF_FILE_ERROR_READ_FAILED;
    }

    if (read_exact(file, header, sizeof(header)) != PSF_FILE_OK) {
        fclose(file);
        return PSF_FILE_ERROR_READ_FAILED;
    }

    if (header[0] != 'P' || header[1] != 'S' || header[2] != 'F') {
        fclose(file);
        return PSF_FILE_ERROR_BAD_FORMAT;
    }

    out_info->version = header[3];
    out_info->reserved_size = read_le32(&header[4]);
    out_info->program_size = read_le32(&header[8]);
    out_info->program_crc = read_le32(&header[12]);

    tag_offset = 16ull + out_info->reserved_size + out_info->program_size;
    if (tag_offset + 5 > (uint64_t)file_size) {
        fclose(file);
        return PSF_FILE_OK;
    }

    if (fseek(file, (long)tag_offset, SEEK_SET) != 0) {
        fclose(file);
        return PSF_FILE_ERROR_READ_FAILED;
    }

    if (read_exact(file, marker, 5) != PSF_FILE_OK) {
        fclose(file);
        return PSF_FILE_OK;
    }

    if (memcmp(marker, PSF_TAG_MARKER, 5) != 0) {
        fclose(file);
        return PSF_FILE_OK;
    }

    tag_size = (uint32_t)((uint64_t)file_size - tag_offset - 5u);
    if (tag_size > PSF_MAX_TAG_SIZE) {
        fclose(file);
        return PSF_FILE_ERROR_TOO_LARGE;
    }

    out_info->tags = (char *)calloc((size_t)tag_size + 1u, 1);
    if (out_info->tags == NULL) {
        fclose(file);
        return PSF_FILE_ERROR_TOO_LARGE;
    }

    if (read_exact(file, out_info->tags, tag_size) != PSF_FILE_OK) {
        psf_file_free_info(out_info);
        fclose(file);
        return PSF_FILE_ERROR_READ_FAILED;
    }

    out_info->tag_offset = tag_offset;
    out_info->tag_size = tag_size;

    fclose(file);
    return PSF_FILE_OK;
}

void psf_file_free_info(PsfFileInfo *info)
{
    if (info == NULL) {
        return;
    }

    free(info->tags);
    memset(info, 0, sizeof(*info));
}

const char *psf_file_result_string(PsfFileResult result)
{
    switch (result) {
    case PSF_FILE_OK:
        return "ok";
    case PSF_FILE_ERROR_INVALID_ARGUMENT:
        return "invalid argument";
    case PSF_FILE_ERROR_OPEN_FAILED:
        return "open failed";
    case PSF_FILE_ERROR_READ_FAILED:
        return "read failed";
    case PSF_FILE_ERROR_BAD_FORMAT:
        return "bad format";
    case PSF_FILE_ERROR_TOO_LARGE:
        return "too large";
    default:
        return "unknown";
    }
}

int psf_file_is_psf2(const PsfFileInfo *info)
{
    return info != NULL && info->version == 0x02u;
}

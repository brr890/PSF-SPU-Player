#include "spu2log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

struct Spu2LogHandle {
    FILE *file;
    Spu2LogLiveState live;
#ifdef _WIN32
    HANDLE shared_mapping;
#endif
    Spu2LogSharedHeader *shared;
    uint32_t flags;
    int closed;
};

static void publish_live_state(Spu2LogHandle *handle)
{
    if (handle == NULL || handle->shared == NULL) {
        return;
    }

    handle->shared->sequence++;
    handle->shared->state = handle->live;
    handle->shared->sequence++;
}

static void set_voice_key_flags(Spu2LogHandle *handle, uint8_t core, uint32_t voice_mask, int key_on)
{
    uint8_t voice;

    for (voice = 0; voice < 24; ++voice) {
        Spu2LogVoiceSnapshot *snapshot;

        if ((voice_mask & (1u << voice)) == 0) {
            continue;
        }

        snapshot = &handle->live.voices[core][voice];
        snapshot->core = core;
        snapshot->voice = voice;

        if (key_on) {
            snapshot->active = 1;
            snapshot->flags |= SPU2LOG_VOICE_KEY_ON;
            snapshot->flags &= ~SPU2LOG_VOICE_RELEASE;
            if (snapshot->adsr_phase == SPU2LOG_ADSR_OFF) {
                snapshot->adsr_phase = SPU2LOG_ADSR_ATTACK;
            }
        } else {
            snapshot->flags &= ~SPU2LOG_VOICE_KEY_ON;
            snapshot->flags |= SPU2LOG_VOICE_RELEASE;
            snapshot->adsr_phase = SPU2LOG_ADSR_RELEASE;
        }
    }
}

static Spu2LogResult write_bytes(Spu2LogHandle *handle, const void *data, size_t size)
{
    if (handle == NULL || handle->closed || handle->file == NULL) {
        return SPU2LOG_ERROR_CLOSED;
    }

    if (size == 0) {
        return SPU2LOG_OK;
    }

    if (fwrite(data, 1, size, handle->file) != size) {
        return SPU2LOG_ERROR_WRITE_FAILED;
    }

    return SPU2LOG_OK;
}

static Spu2LogResult write_event(
    Spu2LogHandle *handle,
    uint32_t type,
    uint64_t sample_pos,
    const void *payload,
    uint32_t payload_size)
{
    Spu2LogEventHeader header;
    Spu2LogResult result;

    header.type = type;
    header.size = (uint32_t)(sizeof(header) + payload_size);
    header.sample_pos = sample_pos;

    result = write_bytes(handle, &header, sizeof(header));
    if (result != SPU2LOG_OK) {
        return result;
    }

    return write_bytes(handle, payload, payload_size);
}

SPU2LOG_API Spu2LogResult spu2log_open(
    Spu2LogHandle **out_handle,
    const char *path,
    uint32_t sample_rate,
    uint32_t flags,
    const char *source_name)
{
    Spu2LogFileHeader header;
    Spu2LogHandle *handle;
    size_t source_len;

    if (out_handle == NULL || path == NULL || path[0] == '\0' || sample_rate == 0) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    *out_handle = NULL;

    handle = (Spu2LogHandle *)calloc(1, sizeof(*handle));
    if (handle == NULL) {
        return SPU2LOG_ERROR_OPEN_FAILED;
    }

    handle->file = fopen(path, "wb");
    if (handle->file == NULL) {
        free(handle);
        return SPU2LOG_ERROR_OPEN_FAILED;
    }

    handle->flags = flags;
    handle->closed = 0;
    handle->live.version = SPU2LOG_VERSION;
    handle->live.sample_rate = sample_rate;
    handle->live.flags = flags;

    source_len = source_name != NULL ? strlen(source_name) : 0;
    if (source_len > UINT32_MAX) {
        fclose(handle->file);
        free(handle);
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    header.magic = SPU2LOG_MAGIC;
    header.version = SPU2LOG_VERSION;
    header.header_size = (uint16_t)sizeof(header);
    header.sample_rate = sample_rate;
    header.flags = flags;
    header.created_unix_time = (uint64_t)time(NULL);
    header.source_name_length = (uint32_t)source_len;

    if (write_bytes(handle, &header, sizeof(header)) != SPU2LOG_OK ||
        write_bytes(handle, source_name, source_len) != SPU2LOG_OK) {
        fclose(handle->file);
        free(handle);
        return SPU2LOG_ERROR_WRITE_FAILED;
    }

    *out_handle = handle;
    return SPU2LOG_OK;
}

SPU2LOG_API Spu2LogResult spu2log_close(Spu2LogHandle *handle)
{
    Spu2LogResult result = SPU2LOG_OK;

    if (handle == NULL) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    if (!handle->closed && handle->file != NULL) {
        if (fflush(handle->file) != 0) {
            result = SPU2LOG_ERROR_WRITE_FAILED;
        }
        if (fclose(handle->file) != 0) {
            result = SPU2LOG_ERROR_WRITE_FAILED;
        }
    }

    spu2log_disable_shared_state(handle);

    handle->closed = 1;
    handle->file = NULL;
    free(handle);
    return result;
}

SPU2LOG_API Spu2LogResult spu2log_reg_write(
    Spu2LogHandle *handle,
    uint64_t sample_pos,
    uint32_t address,
    uint16_t value)
{
    Spu2LogRegWriteEvent event;

    if (handle == NULL) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    handle->live.last_sample_pos = sample_pos;
    publish_live_state(handle);

    if ((handle->flags & SPU2LOG_FLAG_REG_WRITES) == 0) {
        return SPU2LOG_OK;
    }

    event.address = address;
    event.value = value;
    event.reserved = 0;

    return write_event(handle, SPU2LOG_EVENT_REG_WRITE, sample_pos, &event, sizeof(event));
}

SPU2LOG_API Spu2LogResult spu2log_key_on(
    Spu2LogHandle *handle,
    uint64_t sample_pos,
    uint8_t core,
    uint32_t voice_mask)
{
    Spu2LogKeyEvent event;

    if (handle == NULL || core > 1 || (voice_mask & 0xFF000000u) != 0) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    event.core = core;
    event.reserved0 = 0;
    event.reserved1 = 0;
    event.voice_mask = voice_mask;

    handle->live.last_sample_pos = sample_pos;
    handle->live.cores[core].core = core;
    handle->live.cores[core].key_on_mask |= voice_mask;
    handle->live.cores[core].key_off_mask &= ~voice_mask;
    set_voice_key_flags(handle, core, voice_mask, 1);
    publish_live_state(handle);

    return write_event(handle, SPU2LOG_EVENT_KEY_ON, sample_pos, &event, sizeof(event));
}

SPU2LOG_API Spu2LogResult spu2log_key_off(
    Spu2LogHandle *handle,
    uint64_t sample_pos,
    uint8_t core,
    uint32_t voice_mask)
{
    Spu2LogKeyEvent event;

    if (handle == NULL || core > 1 || (voice_mask & 0xFF000000u) != 0) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    event.core = core;
    event.reserved0 = 0;
    event.reserved1 = 0;
    event.voice_mask = voice_mask;

    handle->live.last_sample_pos = sample_pos;
    handle->live.cores[core].core = core;
    handle->live.cores[core].key_off_mask |= voice_mask;
    handle->live.cores[core].key_on_mask &= ~voice_mask;
    set_voice_key_flags(handle, core, voice_mask, 0);
    publish_live_state(handle);

    return write_event(handle, SPU2LOG_EVENT_KEY_OFF, sample_pos, &event, sizeof(event));
}

SPU2LOG_API Spu2LogResult spu2log_voice_snapshot(
    Spu2LogHandle *handle,
    uint64_t sample_pos,
    const Spu2LogVoiceSnapshot *snapshot)
{
    if (handle == NULL || snapshot == NULL || snapshot->core > 1 || snapshot->voice > 23) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    handle->live.last_sample_pos = sample_pos;
    handle->live.voices[snapshot->core][snapshot->voice] = *snapshot;
    publish_live_state(handle);

    if ((handle->flags & SPU2LOG_FLAG_SNAPSHOTS) == 0) {
        return SPU2LOG_OK;
    }

    return write_event(
        handle,
        SPU2LOG_EVENT_VOICE_SNAPSHOT,
        sample_pos,
        snapshot,
        (uint32_t)sizeof(*snapshot));
}

SPU2LOG_API Spu2LogResult spu2log_core_snapshot(
    Spu2LogHandle *handle,
    uint64_t sample_pos,
    const Spu2LogCoreSnapshot *snapshot)
{
    if (handle == NULL || snapshot == NULL || snapshot->core > 1) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    handle->live.last_sample_pos = sample_pos;
    handle->live.cores[snapshot->core] = *snapshot;
    publish_live_state(handle);

    if ((handle->flags & SPU2LOG_FLAG_SNAPSHOTS) == 0) {
        return SPU2LOG_OK;
    }

    return write_event(
        handle,
        SPU2LOG_EVENT_CORE_SNAPSHOT,
        sample_pos,
        snapshot,
        (uint32_t)sizeof(*snapshot));
}

SPU2LOG_API Spu2LogResult spu2log_get_live_state(
    Spu2LogHandle *handle,
    Spu2LogLiveState *out_state)
{
    if (handle == NULL || out_state == NULL) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    if (handle->closed) {
        return SPU2LOG_ERROR_CLOSED;
    }

    *out_state = handle->live;
    return SPU2LOG_OK;
}

SPU2LOG_API Spu2LogResult spu2log_enable_shared_state(
    Spu2LogHandle *handle,
    const char *mapping_name)
{
    if (handle == NULL || mapping_name == NULL || mapping_name[0] == '\0') {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

#ifndef _WIN32
    (void)mapping_name;
    return SPU2LOG_ERROR_OPEN_FAILED;
#else
    spu2log_disable_shared_state(handle);

    handle->shared_mapping = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        (DWORD)sizeof(Spu2LogSharedHeader),
        mapping_name);
    if (handle->shared_mapping == NULL) {
        return SPU2LOG_ERROR_OPEN_FAILED;
    }

    handle->shared = (Spu2LogSharedHeader *)MapViewOfFile(
        handle->shared_mapping,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(Spu2LogSharedHeader));
    if (handle->shared == NULL) {
        CloseHandle(handle->shared_mapping);
        handle->shared_mapping = NULL;
        return SPU2LOG_ERROR_OPEN_FAILED;
    }

    memset(handle->shared, 0, sizeof(*handle->shared));
    handle->shared->magic = SPU2LOG_MAGIC;
    handle->shared->version = SPU2LOG_VERSION;
    handle->shared->size = (uint32_t)sizeof(*handle->shared);
    publish_live_state(handle);
    handle->flags |= SPU2LOG_FLAG_LIVE_SHARED;
    return SPU2LOG_OK;
#endif
}

SPU2LOG_API Spu2LogResult spu2log_disable_shared_state(Spu2LogHandle *handle)
{
    if (handle == NULL) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    if (handle->shared != NULL) {
        UnmapViewOfFile(handle->shared);
        handle->shared = NULL;
    }

    if (handle->shared_mapping != NULL) {
        CloseHandle(handle->shared_mapping);
        handle->shared_mapping = NULL;
    }
#else
    handle->shared = NULL;
#endif

    handle->flags &= ~SPU2LOG_FLAG_LIVE_SHARED;
    return SPU2LOG_OK;
}

SPU2LOG_API Spu2LogResult spu2log_open_shared_state(
    const char *mapping_name,
    Spu2LogSharedHeader **out_shared)
{
    if (mapping_name == NULL || mapping_name[0] == '\0' || out_shared == NULL) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    *out_shared = NULL;

#ifndef _WIN32
    return SPU2LOG_ERROR_OPEN_FAILED;
#else
    {
        HANDLE mapping = OpenFileMappingA(FILE_MAP_READ, FALSE, mapping_name);
        Spu2LogSharedHeader *shared;

        if (mapping == NULL) {
            return SPU2LOG_ERROR_OPEN_FAILED;
        }

        shared = (Spu2LogSharedHeader *)MapViewOfFile(
            mapping,
            FILE_MAP_READ,
            0,
            0,
            sizeof(Spu2LogSharedHeader));
        CloseHandle(mapping);

        if (shared == NULL) {
            return SPU2LOG_ERROR_OPEN_FAILED;
        }

        if (shared->magic != SPU2LOG_MAGIC || shared->version != SPU2LOG_VERSION ||
            shared->size != sizeof(Spu2LogSharedHeader)) {
            UnmapViewOfFile(shared);
            return SPU2LOG_ERROR_BAD_FORMAT;
        }

        *out_shared = shared;
        return SPU2LOG_OK;
    }
#endif
}

SPU2LOG_API Spu2LogResult spu2log_close_shared_state(Spu2LogSharedHeader *shared)
{
    if (shared == NULL) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    return UnmapViewOfFile(shared) ? SPU2LOG_OK : SPU2LOG_ERROR_CLOSED;
#else
    return SPU2LOG_ERROR_OPEN_FAILED;
#endif
}

SPU2LOG_API Spu2LogResult spu2log_copy_shared_state(
    const Spu2LogSharedHeader *shared,
    Spu2LogLiveState *out_state,
    uint32_t *out_sequence)
{
    uint32_t first;
    uint32_t second;
    int tries;

    if (shared == NULL || out_state == NULL) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    for (tries = 0; tries < 100; ++tries) {
        first = shared->sequence;
        *out_state = shared->state;
        second = shared->sequence;

        if (first == second && (second & 1u) == 0) {
            if (out_sequence != NULL) {
                *out_sequence = second;
            }
            return SPU2LOG_OK;
        }
    }

    return SPU2LOG_ERROR_READ_FAILED;
}

SPU2LOG_API const char *spu2log_result_string(Spu2LogResult result)
{
    switch (result) {
    case SPU2LOG_OK:
        return "ok";
    case SPU2LOG_ERROR_INVALID_ARGUMENT:
        return "invalid argument";
    case SPU2LOG_ERROR_OPEN_FAILED:
        return "open failed";
    case SPU2LOG_ERROR_WRITE_FAILED:
        return "write failed";
    case SPU2LOG_ERROR_CLOSED:
        return "closed";
    case SPU2LOG_ERROR_BAD_FORMAT:
        return "bad format";
    case SPU2LOG_ERROR_READ_FAILED:
        return "read failed";
    default:
        return "unknown";
    }
}

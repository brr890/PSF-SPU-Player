#ifndef SPU2LOG_H
#define SPU2LOG_H

#include <stdint.h>

#ifdef _WIN32
#  ifdef SPU2LOG_STATIC
#    define SPU2LOG_API
#  elif defined(SPU2LOG_BUILD_DLL)
#    define SPU2LOG_API __declspec(dllexport)
#  else
#    define SPU2LOG_API __declspec(dllimport)
#  endif
#else
#  define SPU2LOG_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SPU2LOG_MAGIC 0x324C5053u
#define SPU2LOG_VERSION 1u

#define SPU2LOG_FLAG_REG_WRITES 0x00000001u
#define SPU2LOG_FLAG_SNAPSHOTS  0x00000002u
#define SPU2LOG_FLAG_LIVE_SHARED 0x00000004u

#define SPU2LOG_VOICE_KEY_ON  0x00000001u
#define SPU2LOG_VOICE_RELEASE 0x00000002u
#define SPU2LOG_VOICE_NOISE   0x00000004u
#define SPU2LOG_VOICE_PMOD    0x00000008u
#define SPU2LOG_VOICE_REVERB  0x00000010u
#define SPU2LOG_VOICE_DRY_L   0x00000020u
#define SPU2LOG_VOICE_DRY_R   0x00000040u
#define SPU2LOG_VOICE_WET_L   0x00000080u
#define SPU2LOG_VOICE_WET_R   0x00000100u

typedef enum Spu2LogResult {
    SPU2LOG_OK = 0,
    SPU2LOG_ERROR_INVALID_ARGUMENT = -1,
    SPU2LOG_ERROR_OPEN_FAILED = -2,
    SPU2LOG_ERROR_WRITE_FAILED = -3,
    SPU2LOG_ERROR_CLOSED = -4,
    SPU2LOG_ERROR_BAD_FORMAT = -5,
    SPU2LOG_ERROR_READ_FAILED = -6
} Spu2LogResult;

typedef enum Spu2LogEventType {
    SPU2LOG_EVENT_REG_WRITE = 1,
    SPU2LOG_EVENT_KEY_ON = 2,
    SPU2LOG_EVENT_KEY_OFF = 3,
    SPU2LOG_EVENT_VOICE_SNAPSHOT = 4,
    SPU2LOG_EVENT_CORE_SNAPSHOT = 5,
    SPU2LOG_EVENT_MARKER = 6
} Spu2LogEventType;

typedef enum Spu2LogAdsrPhase {
    SPU2LOG_ADSR_OFF = 0,
    SPU2LOG_ADSR_ATTACK = 1,
    SPU2LOG_ADSR_DECAY = 2,
    SPU2LOG_ADSR_SUSTAIN = 3,
    SPU2LOG_ADSR_RELEASE = 4
} Spu2LogAdsrPhase;

typedef struct Spu2LogHandle Spu2LogHandle;

typedef struct Spu2LogFileHeader {
    uint32_t magic;
    uint16_t version;
    uint16_t header_size;
    uint32_t sample_rate;
    uint32_t flags;
    uint64_t created_unix_time;
    uint32_t source_name_length;
} Spu2LogFileHeader;

typedef struct Spu2LogEventHeader {
    uint32_t type;
    uint32_t size;
    uint64_t sample_pos;
} Spu2LogEventHeader;

typedef struct Spu2LogRegWriteEvent {
    uint32_t address;
    uint16_t value;
    uint16_t reserved;
} Spu2LogRegWriteEvent;

typedef struct Spu2LogKeyEvent {
    uint8_t core;
    uint8_t reserved0;
    uint16_t reserved1;
    uint32_t voice_mask;
} Spu2LogKeyEvent;

typedef struct Spu2LogVoiceSnapshot {
    uint8_t core;
    uint8_t voice;
    uint8_t active;
    uint8_t adsr_phase;
    uint8_t noise_clock;
    uint8_t reserved_voice[3];
    uint16_t vol_l;
    uint16_t vol_r;
    uint16_t pitch;
    uint16_t adsr1;
    uint16_t adsr2;
    uint16_t envx;
    uint32_t ssa;
    uint32_t lsa;
    uint32_t nax;
    uint32_t flags;
    uint32_t sample_end;
} Spu2LogVoiceSnapshot;

typedef struct Spu2LogCoreSnapshot {
    uint8_t core;
    uint8_t reserved0;
    uint16_t reserved1;
    uint16_t master_l;
    uint16_t master_r;
    uint16_t reverb_l;
    uint16_t reverb_r;
    uint32_t key_on_mask;
    uint32_t key_off_mask;
    uint32_t noise_mask;
    uint32_t pmod_mask;
    uint32_t reverb_mask;
    uint32_t flags;
} Spu2LogCoreSnapshot;

typedef struct Spu2LogLiveState {
    uint32_t version;
    uint32_t sample_rate;
    uint32_t flags;
    uint64_t last_sample_pos;
    Spu2LogCoreSnapshot cores[2];
    Spu2LogVoiceSnapshot voices[2][24];
} Spu2LogLiveState;

typedef struct Spu2LogSharedHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t size;
    uint32_t sequence;
    Spu2LogLiveState state;
} Spu2LogSharedHeader;

SPU2LOG_API Spu2LogResult spu2log_open(
    Spu2LogHandle **out_handle,
    const char *path,
    uint32_t sample_rate,
    uint32_t flags,
    const char *source_name);

SPU2LOG_API Spu2LogResult spu2log_close(Spu2LogHandle *handle);

SPU2LOG_API Spu2LogResult spu2log_reg_write(
    Spu2LogHandle *handle,
    uint64_t sample_pos,
    uint32_t address,
    uint16_t value);

SPU2LOG_API Spu2LogResult spu2log_key_on(
    Spu2LogHandle *handle,
    uint64_t sample_pos,
    uint8_t core,
    uint32_t voice_mask);

SPU2LOG_API Spu2LogResult spu2log_key_off(
    Spu2LogHandle *handle,
    uint64_t sample_pos,
    uint8_t core,
    uint32_t voice_mask);

SPU2LOG_API Spu2LogResult spu2log_voice_snapshot(
    Spu2LogHandle *handle,
    uint64_t sample_pos,
    const Spu2LogVoiceSnapshot *snapshot);

SPU2LOG_API Spu2LogResult spu2log_core_snapshot(
    Spu2LogHandle *handle,
    uint64_t sample_pos,
    const Spu2LogCoreSnapshot *snapshot);

SPU2LOG_API Spu2LogResult spu2log_get_live_state(
    Spu2LogHandle *handle,
    Spu2LogLiveState *out_state);

SPU2LOG_API Spu2LogResult spu2log_enable_shared_state(
    Spu2LogHandle *handle,
    const char *mapping_name);

SPU2LOG_API Spu2LogResult spu2log_disable_shared_state(Spu2LogHandle *handle);

SPU2LOG_API Spu2LogResult spu2log_open_shared_state(
    const char *mapping_name,
    Spu2LogSharedHeader **out_shared);

SPU2LOG_API Spu2LogResult spu2log_close_shared_state(Spu2LogSharedHeader *shared);

SPU2LOG_API Spu2LogResult spu2log_copy_shared_state(
    const Spu2LogSharedHeader *shared,
    Spu2LogLiveState *out_state,
    uint32_t *out_sequence);

SPU2LOG_API const char *spu2log_result_string(Spu2LogResult result);

#ifdef __cplusplus
}
#endif

#endif

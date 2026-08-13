#ifndef PSF2_CORE_BRIDGE_H
#define PSF2_CORE_BRIDGE_H

#include "spu2log_spu2_adapter.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Psf2CoreBridge Psf2CoreBridge;

typedef enum Psf2CoreBridgeResult {
    PSF2_CORE_BRIDGE_OK = 0,
    PSF2_CORE_BRIDGE_ERROR_INVALID_ARGUMENT = -1,
    PSF2_CORE_BRIDGE_ERROR_OPEN_FAILED = -2,
    PSF2_CORE_BRIDGE_ERROR_RENDER_FAILED = -3,
    PSF2_CORE_BRIDGE_ERROR_UNSUPPORTED = -4
} Psf2CoreBridgeResult;

typedef struct Psf2CoreCallbacks {
    void *user;
    Spu2LogResult (*spu2_write16)(void *user, uint64_t sample_pos, uint32_t address, uint16_t value);
    Spu2LogResult (*voice_snapshot)(void *user, uint64_t sample_pos, const Spu2LogVoiceSnapshot *snapshot);
    Spu2LogResult (*core_snapshot)(void *user, uint64_t sample_pos, const Spu2LogCoreSnapshot *snapshot);
} Psf2CoreCallbacks;

typedef struct Psf2CoreProvider {
    const char *name;
    const char *license;
    Psf2CoreBridgeResult (*open)(
        Psf2CoreBridge **out_core,
        const char *path,
        uint32_t sample_rate,
        const Psf2CoreCallbacks *callbacks);
    Psf2CoreBridgeResult (*render)(
        Psf2CoreBridge *core,
        int16_t *stereo_pcm,
        uint32_t frames,
        uint32_t *out_frames);
    void (*close)(Psf2CoreBridge *core);
} Psf2CoreProvider;

typedef struct Psf2LogPlayback {
    const Psf2CoreProvider *provider;
    Psf2CoreBridge *core;
    Spu2LogHandle *log;
    Spu2LogSpu2Adapter adapter;
    uint32_t sample_rate;
    uint64_t rendered_frames;
} Psf2LogPlayback;

Psf2CoreBridgeResult psf2log_playback_open(
    Psf2LogPlayback *playback,
    const Psf2CoreProvider *provider,
    const char *psf2_path,
    const char *log_path,
    uint32_t sample_rate);

Psf2CoreBridgeResult psf2log_playback_render(
    Psf2LogPlayback *playback,
    int16_t *stereo_pcm,
    uint32_t frames,
    uint32_t *out_frames);

void psf2log_playback_close(Psf2LogPlayback *playback);

const char *psf2_core_bridge_result_string(Psf2CoreBridgeResult result);

#ifdef __cplusplus
}
#endif

#endif

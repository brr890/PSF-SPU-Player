#include "psf2_core_bridge.h"

#include <string.h>

static Spu2LogResult bridge_spu2_write16(
    void *user,
    uint64_t sample_pos,
    uint32_t address,
    uint16_t value)
{
    Psf2LogPlayback *playback = (Psf2LogPlayback *)user;

    if (playback == NULL) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    spu2log_spu2_set_sample_pos(&playback->adapter, sample_pos);
    return spu2log_spu2_write16(&playback->adapter, address, value);
}

static Spu2LogResult bridge_voice_snapshot(
    void *user,
    uint64_t sample_pos,
    const Spu2LogVoiceSnapshot *snapshot)
{
    Psf2LogPlayback *playback = (Psf2LogPlayback *)user;

    if (playback == NULL || snapshot == NULL) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    return spu2log_voice_snapshot(playback->log, sample_pos, snapshot);
}

static Spu2LogResult bridge_core_snapshot(
    void *user,
    uint64_t sample_pos,
    const Spu2LogCoreSnapshot *snapshot)
{
    Psf2LogPlayback *playback = (Psf2LogPlayback *)user;

    if (playback == NULL || snapshot == NULL) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    return spu2log_core_snapshot(playback->log, sample_pos, snapshot);
}

Psf2CoreBridgeResult psf2log_playback_open(
    Psf2LogPlayback *playback,
    const Psf2CoreProvider *provider,
    const char *psf2_path,
    const char *log_path,
    uint32_t sample_rate)
{
    Psf2CoreCallbacks callbacks;
    Spu2LogResult log_result;
    Psf2CoreBridgeResult core_result;

    if (playback == NULL || provider == NULL || provider->open == NULL ||
        provider->render == NULL || provider->close == NULL ||
        psf2_path == NULL || log_path == NULL || sample_rate == 0) {
        return PSF2_CORE_BRIDGE_ERROR_INVALID_ARGUMENT;
    }

    memset(playback, 0, sizeof(*playback));
    playback->provider = provider;
    playback->sample_rate = sample_rate;

    log_result = spu2log_open(
        &playback->log,
        log_path,
        sample_rate,
        SPU2LOG_FLAG_REG_WRITES | SPU2LOG_FLAG_SNAPSHOTS,
        psf2_path);
    if (log_result != SPU2LOG_OK) {
        return PSF2_CORE_BRIDGE_ERROR_OPEN_FAILED;
    }

    spu2log_spu2_adapter_init(&playback->adapter, playback->log, sample_rate / 60u);

    callbacks.user = playback;
    callbacks.spu2_write16 = bridge_spu2_write16;
    callbacks.voice_snapshot = bridge_voice_snapshot;
    callbacks.core_snapshot = bridge_core_snapshot;

    core_result = provider->open(&playback->core, psf2_path, sample_rate, &callbacks);
    if (core_result != PSF2_CORE_BRIDGE_OK) {
        psf2log_playback_close(playback);
        return core_result;
    }

    return PSF2_CORE_BRIDGE_OK;
}

Psf2CoreBridgeResult psf2log_playback_render(
    Psf2LogPlayback *playback,
    int16_t *stereo_pcm,
    uint32_t frames,
    uint32_t *out_frames)
{
    Psf2CoreBridgeResult result;
    uint32_t rendered = 0;

    if (playback == NULL || playback->core == NULL || playback->provider == NULL ||
        playback->provider->render == NULL || stereo_pcm == NULL || frames == 0) {
        return PSF2_CORE_BRIDGE_ERROR_INVALID_ARGUMENT;
    }

    result = playback->provider->render(playback->core, stereo_pcm, frames, &rendered);
    if (result != PSF2_CORE_BRIDGE_OK) {
        return result;
    }

    playback->rendered_frames += rendered;
    spu2log_spu2_set_sample_pos(&playback->adapter, playback->rendered_frames);

    if (out_frames != NULL) {
        *out_frames = rendered;
    }

    return PSF2_CORE_BRIDGE_OK;
}

void psf2log_playback_close(Psf2LogPlayback *playback)
{
    if (playback == NULL) {
        return;
    }

    if (playback->core != NULL && playback->provider != NULL && playback->provider->close != NULL) {
        playback->provider->close(playback->core);
    }

    if (playback->log != NULL) {
        spu2log_spu2_flush_snapshots(&playback->adapter);
        spu2log_close(playback->log);
    }

    memset(playback, 0, sizeof(*playback));
}

const char *psf2_core_bridge_result_string(Psf2CoreBridgeResult result)
{
    switch (result) {
    case PSF2_CORE_BRIDGE_OK:
        return "ok";
    case PSF2_CORE_BRIDGE_ERROR_INVALID_ARGUMENT:
        return "invalid argument";
    case PSF2_CORE_BRIDGE_ERROR_OPEN_FAILED:
        return "open failed";
    case PSF2_CORE_BRIDGE_ERROR_RENDER_FAILED:
        return "render failed";
    case PSF2_CORE_BRIDGE_ERROR_UNSUPPORTED:
        return "unsupported";
    default:
        return "unknown";
    }
}

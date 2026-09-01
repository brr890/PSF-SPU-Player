#ifndef PS_STREAM_DECODER_H
#define PS_STREAM_DECODER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PsStreamDecoder PsStreamDecoder;

PsStreamDecoder *ps_stream_decoder_create(
    const uint8_t *data,
    uint32_t size,
    uint32_t output_rate);

void ps_stream_decoder_destroy(PsStreamDecoder *decoder);

uint32_t ps_stream_decoder_render(
    PsStreamDecoder *decoder,
    int16_t *stereo_pcm,
    uint32_t frames);

uint64_t ps_stream_decoder_total_frames(const PsStreamDecoder *decoder);

int ps_stream_decoder_is_ps1(const PsStreamDecoder *decoder);

#ifdef __cplusplus
}
#endif

#endif

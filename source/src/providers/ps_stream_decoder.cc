#include "ps_stream_decoder.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define PS_STREAM_XA_SECTOR_BYTES 0x930u
#define PS_STREAM_XA_DATA_OFFSET 0x18u
#define PS_STREAM_XA_FRAME_BYTES 0x80u
#define PS_STREAM_MAX_SOURCE_FRAMES 4032u

typedef enum PsStreamKind {
    PS_STREAM_KIND_NONE = 0,
    PS_STREAM_KIND_XA = 1,
    PS_STREAM_KIND_PS_ADPCM = 2,
    PS_STREAM_KIND_PCM16LE = 3
} PsStreamKind;

struct PsStreamDecoder {
    const uint8_t *data;
    uint32_t size;
    uint32_t output_rate;
    uint32_t source_rate;
    uint32_t channels;
    PsStreamKind kind;
    uint8_t is_ps1;

    uint32_t data_start;
    uint32_t data_end;
    uint32_t next_offset;
    uint32_t interleave;
    uint32_t interleave_offset;
    uint16_t xa_config;
    uint8_t xa_bits;

    int32_t history1[2];
    int32_t history2[2];
    int16_t source_buffer[PS_STREAM_MAX_SOURCE_FRAMES * 2u];
    uint32_t source_buffer_frames;
    uint32_t source_buffer_pos;

    int16_t current_left;
    int16_t current_right;
    int16_t next_left;
    int16_t next_right;
    uint64_t resample_phase;
    uint64_t total_source_frames;
    uint64_t total_output_frames;
    uint64_t output_position;
    uint8_t resampler_initialized;
    uint8_t source_exhausted;
    uint8_t finished;
};

static uint32_t read_le32(const uint8_t *data)
{
    return (uint32_t)data[0] |
        ((uint32_t)data[1] << 8) |
        ((uint32_t)data[2] << 16) |
        ((uint32_t)data[3] << 24);
}

static uint32_t read_be32(const uint8_t *data)
{
    return ((uint32_t)data[0] << 24) |
        ((uint32_t)data[1] << 16) |
        ((uint32_t)data[2] << 8) |
        (uint32_t)data[3];
}

static int16_t clamp_i16(int64_t value)
{
    if (value > INT16_MAX) {
        return INT16_MAX;
    }
    if (value < INT16_MIN) {
        return INT16_MIN;
    }
    return (int16_t)value;
}

static int32_t clamp_i32(int64_t value)
{
    if (value > INT32_MAX) {
        return INT32_MAX;
    }
    if (value < INT32_MIN) {
        return INT32_MIN;
    }
    return (int32_t)value;
}

static int xa_sector_is_sync(const uint8_t *sector, uint32_t remaining)
{
    static const uint8_t sync[12] = {
        0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0x00
    };

    return remaining >= PS_STREAM_XA_SECTOR_BYTES &&
        memcmp(sector, sync, sizeof(sync)) == 0;
}

static int xa_sector_is_audio(const uint8_t *sector)
{
    uint8_t submode = sector[0x12];

    return (submode & 0x08u) == 0 &&
        (submode & 0x04u) != 0 &&
        (submode & 0x02u) == 0;
}

static int xa_coding_info(
    uint8_t coding,
    uint32_t *out_channels,
    uint32_t *out_rate,
    uint8_t *out_bits)
{
    uint32_t channels;
    uint32_t rate;
    uint8_t bits;

    switch (coding & 0x03u) {
    case 0:
        channels = 1;
        break;
    case 1:
        channels = 2;
        break;
    default:
        return 0;
    }
    switch ((coding >> 2) & 0x03u) {
    case 0:
        rate = 37800u;
        break;
    case 1:
        rate = 18900u;
        break;
    default:
        return 0;
    }
    switch ((coding >> 4) & 0x03u) {
    case 0:
        bits = 4;
        break;
    case 1:
        bits = 8;
        break;
    default:
        return 0;
    }
    if (bits == 8 && channels == 1) {
        return 0;
    }
    if (out_channels != nullptr) {
        *out_channels = channels;
    }
    if (out_rate != nullptr) {
        *out_rate = rate;
    }
    if (out_bits != nullptr) {
        *out_bits = bits;
    }
    return 1;
}

static int init_xa(PsStreamDecoder *decoder)
{
    uint32_t start = 0;
    uint32_t offset;
    uint32_t channels = 0;
    uint32_t rate = 0;
    uint8_t bits = 0;
    uint16_t target_config = 0;
    uint64_t source_frames = 0;
    int found = 0;

    if (decoder->size >= 0x2cu &&
        memcmp(decoder->data, "RIFF", 4) == 0 &&
        memcmp(decoder->data + 8, "CDXA", 4) == 0) {
        start = 0x2cu;
    } else if (!xa_sector_is_sync(decoder->data, decoder->size)) {
        return 0;
    }

    for (offset = start;
         offset <= decoder->size - PS_STREAM_XA_SECTOR_BYTES;
         offset += PS_STREAM_XA_SECTOR_BYTES) {
        const uint8_t *sector = decoder->data + offset;
        uint32_t sector_channels;
        uint32_t sector_rate;
        uint8_t sector_bits;
        uint16_t config;

        if (!xa_sector_is_sync(sector, decoder->size - offset) ||
            !xa_sector_is_audio(sector) ||
            !xa_coding_info(
                sector[0x13], &sector_channels, &sector_rate, &sector_bits)) {
            continue;
        }
        config = (uint16_t)(((uint16_t)sector[0x10] << 8) | sector[0x11]);
        if (!found) {
            target_config = config;
            channels = sector_channels;
            rate = sector_rate;
            bits = sector_bits;
            found = 1;
        }
        if (config == target_config && sector_channels == channels &&
            sector_rate == rate && sector_bits == bits) {
            uint32_t groups = (sector[0x12] & 0x20u) != 0 ? 18u : 16u;
            uint32_t subframes = bits == 8 ? 4u : 8u;
            source_frames += (uint64_t)groups * 28u * subframes / channels;
        }
    }
    if (!found || source_frames == 0) {
        return 0;
    }

    decoder->kind = PS_STREAM_KIND_XA;
    decoder->is_ps1 = 1;
    decoder->data_start = start;
    decoder->data_end = decoder->size;
    decoder->next_offset = start;
    decoder->channels = channels;
    decoder->source_rate = rate;
    decoder->xa_bits = bits;
    decoder->xa_config = target_config;
    decoder->total_source_frames = source_frames;
    return 1;
}

static int init_sshd(PsStreamDecoder *decoder)
{
    uint32_t codec;
    uint32_t rate;
    uint32_t channels;
    uint32_t interleave;
    uint32_t body_size;
    uint32_t start;
    uint64_t source_frames;

    if (decoder->size < 0x28u ||
        memcmp(decoder->data, "SShd", 4) != 0 ||
        memcmp(decoder->data + 0x20u, "SSbd", 4) != 0) {
        return 0;
    }

    codec = read_le32(decoder->data + 0x08u);
    rate = read_le32(decoder->data + 0x0cu);
    channels = read_le32(decoder->data + 0x10u);
    interleave = read_le32(decoder->data + 0x14u);
    body_size = read_le32(decoder->data + 0x24u);
    if (rate < 4000u || rate > 192000u || channels == 0 || channels > 2u ||
        interleave == 0) {
        return 0;
    }

    if (body_size > decoder->size - 0x28u) {
        body_size = decoder->size - 0x28u;
    }
    start = decoder->size - body_size >= 0x800u ? 0x800u : 0x28u;
    if (start >= decoder->size) {
        return 0;
    }
    if (body_size > decoder->size - start) {
        body_size = decoder->size - start;
    }
    if (body_size == 0) {
        return 0;
    }

    if (codec == 0x10u || codec == 0x02u) {
        if (interleave < 0x10u || (interleave & 0x0fu) != 0) {
            return 0;
        }
        decoder->kind = PS_STREAM_KIND_PS_ADPCM;
        source_frames = ((uint64_t)body_size / (16u * channels)) * 28u;
    } else if (codec == 0x01u || codec == 0x80000001u) {
        if (interleave < 2u || (interleave & 1u) != 0) {
            return 0;
        }
        decoder->kind = PS_STREAM_KIND_PCM16LE;
        source_frames = (uint64_t)body_size / (2u * channels);
    } else {
        return 0;
    }

    decoder->is_ps1 = 0;
    decoder->data_start = start;
    decoder->data_end = start + body_size;
    decoder->next_offset = start;
    decoder->interleave = interleave;
    decoder->interleave_offset = 0;
    decoder->channels = channels;
    decoder->source_rate = rate;
    decoder->total_source_frames = source_frames;
    return source_frames != 0;
}

static int init_vag(PsStreamDecoder *decoder)
{
    uint32_t data_size;
    uint32_t rate;
    uint32_t start;

    if (decoder->size < 0x40u || memcmp(decoder->data, "VAGp", 4) != 0) {
        return 0;
    }
    data_size = read_be32(decoder->data + 0x0cu);
    rate = read_be32(decoder->data + 0x10u);
    start = 0x30u;
    if (rate < 4000u || rate > 192000u || start >= decoder->size) {
        return 0;
    }
    if (data_size == 0 || data_size > decoder->size - start) {
        data_size = decoder->size - start;
    }
    data_size &= ~15u;
    if (data_size == 0) {
        return 0;
    }

    decoder->kind = PS_STREAM_KIND_PS_ADPCM;
    decoder->is_ps1 = 0;
    decoder->data_start = start;
    decoder->data_end = start + data_size;
    decoder->next_offset = start;
    decoder->interleave = 16u;
    decoder->interleave_offset = 0;
    decoder->channels = 1;
    decoder->source_rate = rate;
    decoder->total_source_frames = ((uint64_t)data_size / 16u) * 28u;
    return 1;
}

static void decode_xa_channel(
    PsStreamDecoder *decoder,
    const uint8_t *frame,
    uint32_t channel,
    uint32_t channels,
    uint32_t bits,
    uint32_t output_base,
    uint32_t output_stride)
{
    static const int coefficients[4][2] = {
        {0, 0}, {60, 0}, {115, -52}, {98, -55}
    };
    uint32_t subframes = bits == 8 ? 4u : 8u;
    uint32_t unit_count = subframes / channels;
    uint32_t unit;
    uint32_t output = output_base;
    int32_t history1 = decoder->history1[channel];
    int32_t history2 = decoder->history2[channel];

    for (unit = 0; unit < unit_count; ++unit) {
        uint32_t parameter_offset = bits == 8 ?
            unit * channels + channel :
            4u + unit * channels + channel;
        uint8_t parameter = frame[parameter_offset];
        uint32_t predictor = parameter >> 4;
        uint32_t shift = parameter & 0x0fu;
        uint32_t sample_index;

        if (predictor > 3u) {
            predictor = 0;
        }
        if (shift > (bits == 8 ? 8u : 12u)) {
            shift = bits == 8 ? 8u : 9u;
        }

        for (sample_index = 0; sample_index < 28u; ++sample_index) {
            uint32_t sample_offset;
            int32_t encoded;
            int64_t prediction;
            int32_t decoded;

            if (bits == 8) {
                sample_offset = channels == 1 ?
                    0x10u + sample_index * 4u + unit :
                    0x10u + sample_index * 4u + unit * 2u + channel;
                encoded = (int8_t)frame[sample_offset];
                encoded = (encoded * 256) >> shift;
            } else {
                uint8_t packed;
                int32_t nibble;

                sample_offset = channels == 1 ?
                    0x10u + sample_index * 4u + unit / 2u :
                    0x10u + sample_index * 4u + unit;
                packed = frame[sample_offset];
                if ((channels == 1 && (unit & 1u) != 0) ||
                    (channels == 2 && channel == 1u)) {
                    nibble = (packed >> 4) & 0x0f;
                } else {
                    nibble = packed & 0x0f;
                }
                if ((nibble & 8) != 0) {
                    nibble -= 16;
                }
                encoded = (nibble * 4096) >> shift;
            }

            prediction = (int64_t)coefficients[predictor][0] * history1 +
                (int64_t)coefficients[predictor][1] * history2 + 32;
            decoded = clamp_i32((int64_t)encoded + (prediction >> 6));
            history2 = history1;
            history1 = decoded;
            decoder->source_buffer[output * output_stride + channel] =
                clamp_i16(decoded);
            ++output;
        }
    }

    decoder->history1[channel] = history1;
    decoder->history2[channel] = history2;
}

static int fill_xa_buffer(PsStreamDecoder *decoder)
{
    while (decoder->next_offset <=
           decoder->data_end - PS_STREAM_XA_SECTOR_BYTES) {
        const uint8_t *sector = decoder->data + decoder->next_offset;
        uint32_t sector_offset = decoder->next_offset;
        uint16_t config;
        uint32_t sector_channels;
        uint32_t sector_rate;
        uint8_t sector_bits;
        uint32_t groups;
        uint32_t group;
        uint32_t group_frames;

        decoder->next_offset += PS_STREAM_XA_SECTOR_BYTES;
        if (!xa_sector_is_sync(sector, decoder->data_end - sector_offset) ||
            !xa_sector_is_audio(sector)) {
            continue;
        }
        config = (uint16_t)(((uint16_t)sector[0x10] << 8) | sector[0x11]);
        if (config != decoder->xa_config ||
            !xa_coding_info(
                sector[0x13], &sector_channels, &sector_rate, &sector_bits) ||
            sector_channels != decoder->channels ||
            sector_rate != decoder->source_rate ||
            sector_bits != decoder->xa_bits) {
            continue;
        }

        groups = (sector[0x12] & 0x20u) != 0 ? 18u : 16u;
        group_frames = 28u * (decoder->xa_bits == 8 ? 4u : 8u) /
            decoder->channels;
        for (group = 0; group < groups; ++group) {
            const uint8_t *frame = sector + PS_STREAM_XA_DATA_OFFSET +
                group * PS_STREAM_XA_FRAME_BYTES;
            uint32_t channel;

            for (channel = 0; channel < decoder->channels; ++channel) {
                decode_xa_channel(
                    decoder,
                    frame,
                    channel,
                    decoder->channels,
                    decoder->xa_bits,
                    group * group_frames,
                    decoder->channels);
            }
        }
        decoder->source_buffer_frames = groups * group_frames;
        decoder->source_buffer_pos = 0;
        return 1;
    }
    return 0;
}

static void decode_ps_adpcm_block(
    PsStreamDecoder *decoder,
    const uint8_t *block,
    uint32_t channel)
{
    static const int coefficients[5][2] = {
        {0, 0}, {60, 0}, {115, -52}, {98, -55}, {122, -60}
    };
    uint32_t predictor = block[0] >> 4;
    uint32_t shift = block[0] & 0x0fu;
    int32_t history1 = decoder->history1[channel];
    int32_t history2 = decoder->history2[channel];
    uint32_t sample_index;

    if (predictor > 4u) {
        predictor = 0;
    }
    if (shift > 12u) {
        shift = 9u;
    }

    for (sample_index = 0; sample_index < 28u; ++sample_index) {
        uint8_t packed = block[2u + sample_index / 2u];
        int32_t nibble = (sample_index & 1u) != 0 ?
            (packed >> 4) & 0x0f : packed & 0x0f;
        int64_t prediction;
        int32_t decoded;

        if ((nibble & 8) != 0) {
            nibble -= 16;
        }
        prediction = ((int64_t)history1 * coefficients[predictor][0] +
            (int64_t)history2 * coefficients[predictor][1]) >> 6;
        decoded = clamp_i32(((nibble * 4096) >> shift) + prediction);
        history2 = history1;
        history1 = decoded;
        decoder->source_buffer[sample_index * decoder->channels + channel] =
            clamp_i16(decoded);
    }

    decoder->history1[channel] = history1;
    decoder->history2[channel] = history2;
}

static int fill_interleaved_buffer(PsStreamDecoder *decoder)
{
    uint32_t bytes_per_frame = decoder->kind == PS_STREAM_KIND_PS_ADPCM ?
        16u : 2u;
    uint32_t channel;

    if (decoder->next_offset >= decoder->data_end ||
        decoder->interleave_offset + bytes_per_frame > decoder->interleave) {
        return 0;
    }
    for (channel = 0; channel < decoder->channels; ++channel) {
        uint64_t offset = (uint64_t)decoder->next_offset +
            (uint64_t)channel * decoder->interleave +
            decoder->interleave_offset;

        if (offset + bytes_per_frame > decoder->data_end) {
            return 0;
        }
        if (decoder->kind == PS_STREAM_KIND_PS_ADPCM) {
            decode_ps_adpcm_block(decoder, decoder->data + offset, channel);
        } else {
            uint16_t value = (uint16_t)decoder->data[offset] |
                ((uint16_t)decoder->data[offset + 1u] << 8);
            decoder->source_buffer[channel] = (int16_t)value;
        }
    }

    decoder->source_buffer_frames =
        decoder->kind == PS_STREAM_KIND_PS_ADPCM ? 28u : 1u;
    decoder->source_buffer_pos = 0;
    decoder->interleave_offset += bytes_per_frame;
    if (decoder->interleave_offset >= decoder->interleave) {
        uint64_t next = (uint64_t)decoder->next_offset +
            (uint64_t)decoder->interleave * decoder->channels;
        decoder->next_offset = next > UINT32_MAX ? decoder->data_end :
            (uint32_t)next;
        decoder->interleave_offset = 0;
    }
    return 1;
}

static int get_source_frame(
    PsStreamDecoder *decoder,
    int16_t *out_left,
    int16_t *out_right)
{
    uint32_t position;

    if (decoder->source_buffer_pos >= decoder->source_buffer_frames) {
        decoder->source_buffer_frames = 0;
        decoder->source_buffer_pos = 0;
        if (decoder->kind == PS_STREAM_KIND_XA) {
            if (!fill_xa_buffer(decoder)) {
                return 0;
            }
        } else if (!fill_interleaved_buffer(decoder)) {
            return 0;
        }
    }

    position = decoder->source_buffer_pos++ * decoder->channels;
    *out_left = decoder->source_buffer[position];
    *out_right = decoder->channels > 1u ?
        decoder->source_buffer[position + 1u] : *out_left;
    return 1;
}

extern "C" PsStreamDecoder *ps_stream_decoder_create(
    const uint8_t *data,
    uint32_t size,
    uint32_t output_rate)
{
    PsStreamDecoder *decoder;

    if (data == nullptr || size < 16u || output_rate == 0) {
        return nullptr;
    }
    decoder = (PsStreamDecoder *)calloc(1, sizeof(*decoder));
    if (decoder == nullptr) {
        return nullptr;
    }
    decoder->data = data;
    decoder->size = size;
    decoder->output_rate = output_rate;

    if (!init_xa(decoder) && !init_sshd(decoder) && !init_vag(decoder)) {
        free(decoder);
        return nullptr;
    }
    decoder->total_output_frames =
        (decoder->total_source_frames * output_rate +
         decoder->source_rate - 1u) / decoder->source_rate;
    return decoder;
}

extern "C" void ps_stream_decoder_destroy(PsStreamDecoder *decoder)
{
    free(decoder);
}

extern "C" uint32_t ps_stream_decoder_render(
    PsStreamDecoder *decoder,
    int16_t *stereo_pcm,
    uint32_t frames)
{
    uint32_t rendered = 0;

    if (decoder == nullptr || stereo_pcm == nullptr || frames == 0 ||
        decoder->finished) {
        return 0;
    }
    if (!decoder->resampler_initialized) {
        if (!get_source_frame(
                decoder, &decoder->current_left, &decoder->current_right)) {
            decoder->finished = 1;
            return 0;
        }
        if (!get_source_frame(
                decoder, &decoder->next_left, &decoder->next_right)) {
            decoder->next_left = decoder->current_left;
            decoder->next_right = decoder->current_right;
            decoder->source_exhausted = 1;
        }
        decoder->resampler_initialized = 1;
    }

    while (rendered < frames && !decoder->finished &&
           decoder->output_position < decoder->total_output_frames) {
        int64_t left_delta =
            (int64_t)decoder->next_left - decoder->current_left;
        int64_t right_delta =
            (int64_t)decoder->next_right - decoder->current_right;

        stereo_pcm[rendered * 2u] = clamp_i16(
            decoder->current_left +
            (left_delta * (int64_t)decoder->resample_phase) /
                decoder->output_rate);
        stereo_pcm[rendered * 2u + 1u] = clamp_i16(
            decoder->current_right +
            (right_delta * (int64_t)decoder->resample_phase) /
                decoder->output_rate);
        ++rendered;
        ++decoder->output_position;
        decoder->resample_phase += decoder->source_rate;

        while (decoder->resample_phase >= decoder->output_rate) {
            decoder->resample_phase -= decoder->output_rate;
            if (decoder->source_exhausted) {
                decoder->finished = 1;
                break;
            }
            decoder->current_left = decoder->next_left;
            decoder->current_right = decoder->next_right;
            if (!get_source_frame(
                    decoder, &decoder->next_left, &decoder->next_right)) {
                decoder->next_left = decoder->current_left;
                decoder->next_right = decoder->current_right;
                decoder->source_exhausted = 1;
            }
        }
    }
    if (decoder->output_position >= decoder->total_output_frames) {
        decoder->finished = 1;
    }
    return rendered;
}

extern "C" uint64_t ps_stream_decoder_total_frames(
    const PsStreamDecoder *decoder)
{
    return decoder != nullptr ? decoder->total_output_frames : 0;
}

extern "C" int ps_stream_decoder_is_ps1(const PsStreamDecoder *decoder)
{
    return decoder != nullptr && decoder->is_ps1 != 0;
}

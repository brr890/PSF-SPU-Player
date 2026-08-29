#include "psf2_provider_imported.h"
#include "spu2log_audacious_hooks.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libaudcore/index.h>

#include "psx.h"
#include "peops2/stdafx.h"
#include "peops2/externals.h"
#include "peops2/spu.h"

#define AUDACIOUS_BOOTSTRAP_EXTRA_SLICES 65536u
#define AUDACIOUS_STEADY_EXTRA_SLICES 0u
#ifdef PSF2LOG_ANDROID_LIGHTWEIGHT
#define AUDACIOUS_SNAPSHOT_INTERVAL_FRAMES 735u
#else
#define AUDACIOUS_SNAPSHOT_INTERVAL_FRAMES 735u
#endif
#define AUDACIOUS_PSF1_TIMBRE_SCAN_CPU_DIVIDER 128u
#define AUDACIOUS_SAMPLE_END_CACHE_SIZE 1024u
#define AUDACIOUS_SPU2_RAM_BYTES (2u * 1024u * 1024u)
#define AUDACIOUS_PSX_RAM_BYTES (2u * 1024u * 1024u)
#define AUDACIOUS_AKAO_MAX_SEQUENCES 128u
#define AUDACIOUS_AKAO_MAX_TEMPOS 256u
#define AUDACIOUS_AKAO_FAST_COUNTER_RETRIES 16u
#define AUDACIOUS_AKAO_MAX_COUNTER_RETRIES 64u
#define AUDACIOUS_AKAO_MAX_REJECTED_COUNTERS 32u
#define AUDACIOUS_SONY_SEQ_HEADER_SIZE 15u
#define AUDACIOUS_SONY_SEQ_TABLE_SIZE 176u
#define AUDACIOUS_SONY_SEQ_TABLE_SCAN_ATTEMPTS 16u
#define AUDACIOUS_SPU2_P_MVOLL(core) ((0x28u * (uint32_t)(core)) + 0x760u)
#define AUDACIOUS_SPU2_P_MVOLR(core) ((0x28u * (uint32_t)(core)) + 0x762u)

typedef enum AudaciousPsfMode {
    AUDACIOUS_PSF_MODE_PSF1 = 1,
    AUDACIOUS_PSF_MODE_PSF2 = 2
} AudaciousPsfMode;

typedef struct AudaciousAkaoRange {
    uint32_t start;
    uint32_t end;
} AudaciousAkaoRange;

typedef struct AudaciousAkaoMonitor {
    Psf1AkaoPlaybackState state;
    AudaciousAkaoRange sequence_ranges[AUDACIOUS_AKAO_MAX_SEQUENCES];
    uint16_t tempo_values[AUDACIOUS_AKAO_MAX_TEMPOS];
    uint8_t tempo_formats[AUDACIOUS_AKAO_MAX_TEMPOS];
    uint32_t sequence_count;
    uint32_t tempo_count;
    uint32_t counter_base;
    uint32_t tempo_address;
    uint16_t last_beat;
    uint16_t last_tick;
    uint16_t last_measure;
    uint32_t rejected_counter_bases[AUDACIOUS_AKAO_MAX_REJECTED_COUNTERS];
    uint64_t next_sequence_scan_sample;
    uint64_t next_counter_scan_sample;
    uint64_t counter_candidate_sample;
    uint8_t sequences_scanned;
    uint8_t sequence_scan_attempts;
    uint8_t counter_scan_attempts;
    uint8_t rejected_counter_count;
    uint8_t confidence;
    uint8_t early_format;
    uint8_t started;
} AudaciousAkaoMonitor;

typedef struct AudaciousSonySeqMonitor {
    Psf1AkaoPlaybackState state;
    uint32_t sequence_address;
    uint32_t table_address;
    uint32_t division;
    uint32_t tempo_us;
    uint32_t numerator;
    uint32_t denominator;
    uint32_t table_initial_tempo_bpm;
    uint64_t elapsed_slices;
    uint64_t next_table_scan_sample;
    uint8_t table_scan_attempts;
    uint8_t header_valid;
    uint8_t table_valid;
    uint8_t valid;
    uint8_t started;
} AudaciousSonySeqMonitor;

typedef struct AudaciousPsf2Core {
    Psf2CoreCallbacks callbacks;
    uint8_t *input_data;
    uint32_t input_size;
    uint32_t sample_rate;
    AudaciousPsfMode mode;
    uint64_t sample_pos;
    uint64_t spu2_write_count;
    uint32_t slice_count;
    uint32_t tempo_accumulator;
    uint64_t next_snapshot_sample;
    uint32_t sample_end_cache_start[AUDACIOUS_SAMPLE_END_CACHE_SIZE];
    uint32_t sample_end_cache_value[AUDACIOUS_SAMPLE_END_CACHE_SIZE];
    AudaciousAkaoMonitor akao;
    AudaciousSonySeqMonitor sony_seq;
    int16_t *pcm;
    uint32_t requested_frames;
    uint32_t rendered_frames;
} AudaciousPsf2Core;

static char g_library_dir[1024];
static AudaciousPsf2Core *g_rendering_core;
static volatile int g_tempo_percent = 100;
static volatile int g_frame_advance_mode = 0;
static volatile uint32_t g_frame_advance_steps = 0;
static volatile uint32_t g_frame_advance_adsr_updates = 0;
static volatile int g_abort_render = 0;
static volatile int g_fast_timbre_scan = 0;
static uint32_t g_noise_force_on_masks[2];
static uint32_t g_noise_force_off_masks[2];
static uint32_t g_pmod_force_on_masks[2];
static uint32_t g_pmod_force_off_masks[2];
static uint32_t g_adsr_force_masks[2];

bool stop_flag = false;

static uint16_t audacious_psx_ram_u16(uint32_t offset)
{
    const uint8_t *ram = (const uint8_t *)psx_ram;

    if (offset + 1u >= AUDACIOUS_PSX_RAM_BYTES) {
        return 0;
    }
    return (uint16_t)((uint16_t)ram[offset] | ((uint16_t)ram[offset + 1u] << 8));
}

static uint32_t audacious_psx_ram_u32(uint32_t offset)
{
    return (uint32_t)audacious_psx_ram_u16(offset) |
        ((uint32_t)audacious_psx_ram_u16(offset + 2u) << 16);
}

static uint16_t audacious_psx_ram_be16(uint32_t offset)
{
    const uint8_t *ram = (const uint8_t *)psx_ram;

    if (offset + 1u >= AUDACIOUS_PSX_RAM_BYTES) {
        return 0;
    }
    return (uint16_t)(((uint16_t)ram[offset] << 8) | (uint16_t)ram[offset + 1u]);
}

static uint32_t audacious_psx_ram_be24(uint32_t offset)
{
    const uint8_t *ram = (const uint8_t *)psx_ram;

    if (offset + 2u >= AUDACIOUS_PSX_RAM_BYTES) {
        return 0;
    }
    return ((uint32_t)ram[offset] << 16) |
        ((uint32_t)ram[offset + 1u] << 8) |
        (uint32_t)ram[offset + 2u];
}

static uint32_t audacious_psx_ram_be32(uint32_t offset)
{
    return ((uint32_t)audacious_psx_ram_be16(offset) << 16) |
        (uint32_t)audacious_psx_ram_be16(offset + 2u);
}

static int audacious_psx_pointer_offset(uint32_t pointer, uint32_t *out_offset)
{
    uint32_t segment = pointer & 0xe0000000u;
    uint32_t offset;

    if (segment != 0x80000000u && segment != 0xa0000000u) {
        return 0;
    }
    offset = pointer & 0x001fffffu;
    if (offset >= AUDACIOUS_PSX_RAM_BYTES) {
        return 0;
    }
    if (out_offset != nullptr) {
        *out_offset = offset;
    }
    return 1;
}

static uint32_t audacious_sony_seq_table_tempo_bpm(uint32_t table_address)
{
    uint32_t tempo = audacious_psx_ram_u32(table_address + 0x94u);

    if (tempo < 20u || tempo > 400u) {
        tempo = audacious_psx_ram_u32(table_address + 0x8cu);
    }
    return tempo >= 20u && tempo <= 400u ? tempo : 0u;
}

static int audacious_sony_seq_table_valid(uint32_t table_address)
{
    const uint8_t *ram = (const uint8_t *)psx_ram;
    uint32_t sequence_pointer;
    uint32_t initial_pointer;
    uint32_t repeat_pointer;
    uint32_t base_pointer;
    uint32_t sequence_offset;
    uint32_t division;
    uint32_t numerator;
    uint32_t denominator_power;
    uint32_t index;

    if (table_address + AUDACIOUS_SONY_SEQ_TABLE_SIZE > AUDACIOUS_PSX_RAM_BYTES) {
        return 0;
    }
    sequence_pointer = audacious_psx_ram_u32(table_address);
    initial_pointer = audacious_psx_ram_u32(table_address + 4u);
    repeat_pointer = audacious_psx_ram_u32(table_address + 8u);
    base_pointer = audacious_psx_ram_u32(table_address + 12u);
    if (!audacious_psx_pointer_offset(sequence_pointer, &sequence_offset) ||
        !audacious_psx_pointer_offset(initial_pointer, nullptr) ||
        !audacious_psx_pointer_offset(repeat_pointer, nullptr) ||
        !audacious_psx_pointer_offset(base_pointer, nullptr) ||
        initial_pointer != repeat_pointer || repeat_pointer != base_pointer) {
        return 0;
    }
    if (ram[table_address + 0x14u] > 1u ||
        ram[table_address + 0x26u] > 15u) {
        return 0;
    }
    numerator = ram[table_address + 0x24u];
    denominator_power = ram[table_address + 0x25u];
    division = audacious_psx_ram_u16(table_address + 0x50u);
    if (numerator == 0u || numerator > 32u || denominator_power > 7u ||
        division < 12u || division > 1920u ||
        audacious_sony_seq_table_tempo_bpm(table_address) == 0u ||
        audacious_psx_ram_u16(table_address + 0x58u) > 127u ||
        audacious_psx_ram_u16(table_address + 0x5au) > 127u ||
        audacious_psx_ram_u32(table_address + 0x98u) >= 0x10000u) {
        return 0;
    }
    for (index = 0; index < 16u; ++index) {
        if (ram[table_address + 0x27u + index] > 127u ||
            ram[table_address + 0x37u + index] > 127u ||
            audacious_psx_ram_u16(table_address + 0x60u + index * 2u) > 127u) {
            return 0;
        }
    }
    return sequence_offset < AUDACIOUS_PSX_RAM_BYTES;
}

static int audacious_sony_seq_find_table(AudaciousSonySeqMonitor *monitor)
{
    uint32_t offset;

    if (monitor == nullptr) {
        return 0;
    }
    for (offset = 0;
         offset + AUDACIOUS_SONY_SEQ_TABLE_SIZE <= AUDACIOUS_PSX_RAM_BYTES;
         offset += 4u) {
        if (!audacious_sony_seq_table_valid(offset)) {
            continue;
        }
        monitor->table_address = offset;
        monitor->table_valid = 1u;
        return 1;
    }
    return 0;
}

static void audacious_sony_seq_apply_table(AudaciousSonySeqMonitor *monitor)
{
    const uint8_t *ram = (const uint8_t *)psx_ram;
    uint32_t tempo_bpm;
    uint32_t denominator_power;
    uint32_t sequence_offset;

    if (monitor == nullptr || !monitor->table_valid ||
        !audacious_sony_seq_table_valid(monitor->table_address)) {
        return;
    }
    tempo_bpm = audacious_sony_seq_table_tempo_bpm(monitor->table_address);
    denominator_power = ram[monitor->table_address + 0x25u];
    monitor->division = audacious_psx_ram_u16(monitor->table_address + 0x50u);
    monitor->numerator = ram[monitor->table_address + 0x24u];
    monitor->denominator = 1u << denominator_power;
    if (monitor->table_initial_tempo_bpm == 0u) {
        monitor->table_initial_tempo_bpm = tempo_bpm;
    }
    if (!monitor->header_valid || tempo_bpm != monitor->table_initial_tempo_bpm) {
        monitor->tempo_us = (60000000u + tempo_bpm / 2u) / tempo_bpm;
    }
    if (!monitor->header_valid &&
        audacious_psx_pointer_offset(
            audacious_psx_ram_u32(monitor->table_address + 4u),
            &sequence_offset)) {
        monitor->sequence_address = sequence_offset;
    }
    monitor->valid = 1u;
}

static void audacious_sony_seq_scan(AudaciousPsf2Core *core)
{
    const uint8_t *ram = (const uint8_t *)psx_ram;
    AudaciousSonySeqMonitor *monitor;
    uint32_t offset;

    if (core == nullptr || core->mode != AUDACIOUS_PSF_MODE_PSF1) {
        return;
    }
    monitor = &core->sony_seq;
    memset(monitor, 0, sizeof(*monitor));
    for (offset = 0;
         offset + AUDACIOUS_SONY_SEQ_HEADER_SIZE <= AUDACIOUS_PSX_RAM_BYTES;
         ++offset) {
        uint32_t version;
        uint32_t division;
        uint32_t tempo_us;
        uint32_t numerator;
        uint32_t denominator_power;

        if (ram[offset] != 'p' || ram[offset + 1u] != 'Q' ||
            ram[offset + 2u] != 'E' || ram[offset + 3u] != 'S') {
            continue;
        }
        version = audacious_psx_ram_be32(offset + 4u);
        division = audacious_psx_ram_be16(offset + 8u);
        tempo_us = audacious_psx_ram_be24(offset + 10u);
        numerator = ram[offset + 13u];
        denominator_power = ram[offset + 14u];
        if (version != 1u || division == 0u || division > 0x7fffu ||
            tempo_us < 10000u || tempo_us > 10000000u ||
            numerator == 0u || numerator > 32u || denominator_power > 7u) {
            continue;
        }
        monitor->sequence_address = offset;
        monitor->division = division;
        monitor->tempo_us = tempo_us;
        monitor->numerator = numerator;
        monitor->denominator = 1u << denominator_power;
        monitor->header_valid = 1u;
        monitor->valid = 1u;
        monitor->state.tempo = tempo_us;
        monitor->state.beats_per_measure = (uint16_t)numerator;
        monitor->state.ticks_per_beat = (uint16_t)((division * 4u) /
            monitor->denominator);
        monitor->state.beat_denominator = (uint16_t)monitor->denominator;
        monitor->state.driver_type = PSF1_MUSIC_DRIVER_SONY_SEQ;
        break;
    }
    if (audacious_sony_seq_find_table(monitor)) {
        audacious_sony_seq_apply_table(monitor);
    }
    if (!monitor->valid) {
        monitor->table_scan_attempts = 1u;
        monitor->next_table_scan_sample = core->sample_rate / 4u;
    }
}

static void audacious_sony_seq_rescan_table(AudaciousPsf2Core *core)
{
    AudaciousSonySeqMonitor *monitor;

    if (core == nullptr || core->mode != AUDACIOUS_PSF_MODE_PSF1) {
        return;
    }
    monitor = &core->sony_seq;
    if (monitor->valid ||
        monitor->table_scan_attempts >= AUDACIOUS_SONY_SEQ_TABLE_SCAN_ATTEMPTS ||
        core->sample_pos < monitor->next_table_scan_sample) {
        return;
    }
    ++monitor->table_scan_attempts;
    monitor->next_table_scan_sample = core->sample_pos + core->sample_rate / 4u;
    if (audacious_sony_seq_find_table(monitor)) {
        audacious_sony_seq_apply_table(monitor);
    }
}

static void audacious_sony_seq_start(AudaciousPsf2Core *core)
{
    AudaciousSonySeqMonitor *monitor;

    if (core == nullptr || core->mode != AUDACIOUS_PSF_MODE_PSF1) {
        return;
    }
    monitor = &core->sony_seq;
    if (!monitor->valid || monitor->started) {
        return;
    }
    monitor->started = 1u;
    monitor->elapsed_slices = 0;
    monitor->state.detected = 1u;
}

static void audacious_sony_seq_advance(AudaciousPsf2Core *core, uint32_t slices)
{
    if (core == nullptr || !core->sony_seq.valid ||
        !core->sony_seq.started || slices == 0u) {
        return;
    }
    core->sony_seq.elapsed_slices += slices;
}

static void audacious_sony_seq_update(AudaciousPsf2Core *core)
{
    AudaciousSonySeqMonitor *monitor;
    uint64_t ticks_per_beat;
    uint64_t ticks_per_measure;
    uint64_t total_ticks;
    double exact_ticks;
    uint64_t measure;

    if (core == nullptr || core->mode != AUDACIOUS_PSF_MODE_PSF1) {
        return;
    }
    monitor = &core->sony_seq;
    audacious_sony_seq_apply_table(monitor);
    if (!monitor->valid || !monitor->started || core->sample_rate == 0u ||
        monitor->tempo_us == 0u || monitor->denominator == 0u) {
        return;
    }
    ticks_per_beat = ((uint64_t)monitor->division * 4u) / monitor->denominator;
    if (ticks_per_beat == 0u) {
        return;
    }
    ticks_per_measure = ticks_per_beat * monitor->numerator;
    exact_ticks = ((double)monitor->elapsed_slices * 1000000.0 *
        (double)monitor->division) /
        ((double)core->sample_rate * (double)monitor->tempo_us);
    total_ticks = exact_ticks > 0.0 ? (uint64_t)exact_ticks : 0u;
    measure = ticks_per_measure != 0u ? total_ticks / ticks_per_measure : 0u;
    monitor->state.tempo = monitor->tempo_us;
    monitor->state.beats_per_measure = (uint16_t)monitor->numerator;
    monitor->state.ticks_per_beat = (uint16_t)ticks_per_beat;
    monitor->state.current_beat = (uint16_t)((total_ticks / ticks_per_beat) %
        monitor->numerator);
    monitor->state.current_tick = (uint16_t)(total_ticks % ticks_per_beat);
    monitor->state.measure = (uint16_t)(measure > 0xffffu ? 0xffffu : measure);
    monitor->state.beat_denominator = (uint16_t)monitor->denominator;
    monitor->state.detected = 1u;
    monitor->state.early_format = 0u;
    monitor->state.driver_type = PSF1_MUSIC_DRIVER_SONY_SEQ;
}

static unsigned audacious_popcount32(uint32_t value)
{
    unsigned count = 0;

    while (value != 0) {
        value &= value - 1u;
        ++count;
    }
    return count;
}

static void audacious_akao_add_tempo(
    AudaciousAkaoMonitor *monitor,
    uint16_t value,
    uint8_t format)
{
    uint32_t index;

    if (monitor == nullptr || value == 0) {
        return;
    }
    for (index = 0; index < monitor->tempo_count &&
         monitor->tempo_values[index] < value; ++index) {
    }
    if (index < monitor->tempo_count && monitor->tempo_values[index] == value) {
        monitor->tempo_formats[index] |= format;
        return;
    }
    if (monitor->tempo_count < AUDACIOUS_AKAO_MAX_TEMPOS) {
        memmove(&monitor->tempo_values[index + 1u],
            &monitor->tempo_values[index],
            (monitor->tempo_count - index) * sizeof(monitor->tempo_values[0]));
        memmove(&monitor->tempo_formats[index + 1u],
            &monitor->tempo_formats[index],
            (monitor->tempo_count - index) * sizeof(monitor->tempo_formats[0]));
        monitor->tempo_values[index] = value;
        monitor->tempo_formats[index] = format;
        ++monitor->tempo_count;
    }
}

static uint8_t audacious_akao_tempo_format(
    const AudaciousAkaoMonitor *monitor,
    uint16_t value)
{
    uint32_t low = 0;
    uint32_t high;

    if (monitor == nullptr || value == 0) {
        return 0;
    }
    high = monitor->tempo_count;
    while (low < high) {
        uint32_t middle = low + (high - low) / 2u;
        if (monitor->tempo_values[middle] == value) {
            return monitor->tempo_formats[middle];
        }
        if (monitor->tempo_values[middle] < value) {
            low = middle + 1u;
        } else {
            high = middle;
        }
    }
    return 0;
}

static int audacious_akao_in_sequence(
    const AudaciousAkaoMonitor *monitor,
    uint32_t address)
{
    uint32_t index;

    for (index = 0; index < monitor->sequence_count; ++index) {
        if (address >= monitor->sequence_ranges[index].start &&
            address < monitor->sequence_ranges[index].end) {
            return 1;
        }
    }
    return 0;
}

static void audacious_akao_scan_sequences(AudaciousAkaoMonitor *monitor)
{
    const uint8_t *ram = (const uint8_t *)psx_ram;
    uint32_t offset;

    if (monitor == nullptr) {
        return;
    }
    monitor->sequence_count = 0;
    monitor->tempo_count = 0;
    memset(monitor->tempo_formats, 0, sizeof(monitor->tempo_formats));
    for (offset = 0; offset + 0x48u < AUDACIOUS_PSX_RAM_BYTES; ++offset) {
        uint32_t size;
        uint32_t sequence_end;
        uint32_t early_mask;
        uint32_t late_mask;
        uint32_t early_start = 0;
        uint32_t late_start = 0;
        uint32_t late_table_end = 0;
        uint32_t late_first_track_start = 0;
        int tempo_found = 0;

        if (ram[offset] != 'A' || ram[offset + 1u] != 'K' ||
            ram[offset + 2u] != 'A' || ram[offset + 3u] != 'O') {
            continue;
        }
        size = audacious_psx_ram_u16(offset + 6u);
        if (size < 8u || size > 0x10000u ||
            offset + size + 0x10u > AUDACIOUS_PSX_RAM_BYTES) {
            continue;
        }
        sequence_end = offset + size + 0x10u;
        early_mask = audacious_psx_ram_u32(offset + 0x10u);
        late_mask = audacious_psx_ram_u32(offset + 0x20u);

        if (early_mask != 0 && (early_mask & 0xff000000u) == 0 &&
            audacious_popcount32(early_mask) <= 24u) {
            uint32_t field = offset + 0x14u;
            uint32_t relative = audacious_psx_ram_u16(field);
            uint32_t start = field + 2u + relative;

            if (start >= offset + 0x14u && start < sequence_end) {
                early_start = start;
            }
        }
        if (late_mask != 0) {
            unsigned first_channel = 0;
            unsigned last_channel = 31u;
            unsigned channel;
            uint32_t field;
            uint32_t relative;
            uint32_t start;

            while (first_channel < 32u && (late_mask & (1u << first_channel)) == 0) {
                ++first_channel;
            }
            if (first_channel < 32u) {
                field = offset + 0x40u + first_channel * 2u;
                relative = audacious_psx_ram_u16(field);
                start = field + relative;
                if (start >= offset + 0x40u && start < sequence_end) {
                    late_start = start;
                }
            }
            while (last_channel > 0u && (late_mask & (1u << last_channel)) == 0) {
                --last_channel;
            }
            late_table_end = offset + 0x40u + (last_channel + 1u) * 2u;
            for (channel = 0; channel <= last_channel; ++channel) {
                uint32_t command_limit;
                uint32_t command;

                if ((late_mask & (1u << channel)) == 0) {
                    continue;
                }
                field = offset + 0x40u + channel * 2u;
                relative = audacious_psx_ram_u16(field);
                start = field + relative;
                if (start >= late_table_end && start < sequence_end &&
                    (late_first_track_start == 0 || start < late_first_track_start)) {
                    late_first_track_start = start;
                }
                if (start < late_table_end || start >= sequence_end) {
                    continue;
                }
                command_limit = start + 256u;
                if (command_limit > sequence_end) {
                    command_limit = sequence_end;
                }
                for (command = start; command + 3u < command_limit; ++command) {
                    if (ram[command] == 0xfeu && ram[command + 1u] == 0x00u) {
                        audacious_akao_add_tempo(
                            monitor, audacious_psx_ram_u16(command + 2u), 2u);
                        tempo_found = 1;
                        break;
                    }
                }
            }
        }
        if (late_table_end != 0 && late_first_track_start > late_table_end) {
            uint32_t command_limit = late_first_track_start;
            uint32_t command;

            if (command_limit > late_table_end + 1024u) {
                command_limit = late_table_end + 1024u;
            }
            for (command = late_table_end; command + 3u < command_limit; ++command) {
                if (ram[command] == 0xfeu && ram[command + 1u] == 0x00u) {
                    audacious_akao_add_tempo(
                        monitor, audacious_psx_ram_u16(command + 2u), 2u);
                    tempo_found = 1;
                    break;
                }
            }
        }
        if (late_start != 0) {
            uint32_t command_limit = late_start + 256u;
            uint32_t command;
            if (command_limit > sequence_end) {
                command_limit = sequence_end;
            }
            for (command = late_start; command + 3u < command_limit; ++command) {
                if (ram[command] == 0xfeu && ram[command + 1u] == 0x00u) {
                    audacious_akao_add_tempo(
                        monitor, audacious_psx_ram_u16(command + 2u), 2u);
                    tempo_found = 1;
                    break;
                }
            }
        }
        if (early_start != 0) {
            uint32_t command_limit = early_start + 256u;
            uint32_t command;
            if (command_limit > sequence_end) {
                command_limit = sequence_end;
            }
            for (command = early_start; command + 2u < command_limit; ++command) {
                if (ram[command] == 0xe8u) {
                    audacious_akao_add_tempo(
                        monitor, audacious_psx_ram_u16(command + 1u), 1u);
                    tempo_found = 1;
                    break;
                }
            }
        }
        if (tempo_found && monitor->sequence_count < AUDACIOUS_AKAO_MAX_SEQUENCES) {
            monitor->sequence_ranges[monitor->sequence_count].start = offset;
            monitor->sequence_ranges[monitor->sequence_count].end = sequence_end;
            ++monitor->sequence_count;
        }
    }
    monitor->sequences_scanned = monitor->sequence_count != 0 && monitor->tempo_count != 0;
}

static int audacious_akao_counter_fields_valid(uint32_t base)
{
    uint16_t beats_per_measure;
    uint16_t current_beat;
    uint16_t ticks_per_beat;
    uint16_t current_tick;

    if (base < 0x80u || base + 8u > AUDACIOUS_PSX_RAM_BYTES) {
        return 0;
    }
    beats_per_measure = audacious_psx_ram_u16(base - 2u);
    current_beat = audacious_psx_ram_u16(base);
    ticks_per_beat = audacious_psx_ram_u16(base + 2u);
    current_tick = audacious_psx_ram_u16(base + 4u);
    return beats_per_measure >= 1u && beats_per_measure <= 16u &&
        current_beat < beats_per_measure &&
        ticks_per_beat >= 12u && ticks_per_beat <= 192u &&
        current_tick < ticks_per_beat;
}

static int audacious_akao_find_counter(AudaciousAkaoMonitor *monitor)
{
    uint32_t tempo_address;
    uint32_t best_base = 0;
    uint32_t best_tempo_address = 0;
    uint8_t best_tempo_format = 0;
    int best_score = INT_MIN;

    if (monitor == nullptr || !monitor->sequences_scanned) {
        return 0;
    }
    for (tempo_address = 0; tempo_address + 2u <= AUDACIOUS_PSX_RAM_BYTES;
         tempo_address += 2u) {
        uint32_t distance;
        uint8_t tempo_format = audacious_akao_tempo_format(
            monitor, audacious_psx_ram_u16(tempo_address));

        if (tempo_format == 0) {
            continue;
        }
        for (distance = 0x10u; distance <= 0x100u; distance += 2u) {
            uint32_t base = tempo_address + distance;
            int score;
            uint8_t rejected_index;

            if (base + 8u > AUDACIOUS_PSX_RAM_BYTES ||
                !audacious_akao_counter_fields_valid(base) ||
                audacious_akao_in_sequence(monitor, base)) {
                continue;
            }
            for (rejected_index = 0;
                 rejected_index < monitor->rejected_counter_count;
                 ++rejected_index) {
                if (monitor->rejected_counter_bases[rejected_index] == base) {
                    break;
                }
            }
            if (rejected_index < monitor->rejected_counter_count) {
                continue;
            }
            score = 1000 - (int)(distance > 0x3cu ?
                distance - 0x3cu : 0x3cu - distance) * 8;
            if (audacious_psx_ram_u16(base - 2u) == 4u) {
                score += 20;
            }
            if (audacious_psx_ram_u16(base + 2u) == 0x30u) {
                score += 20;
            }
            if (score > best_score) {
                best_score = score;
                best_base = base;
                best_tempo_address = tempo_address;
                best_tempo_format = tempo_format;
            }
        }
    }
    if (best_base == 0 || best_tempo_address == 0) {
        return 0;
    }
    monitor->counter_base = best_base;
    monitor->tempo_address = best_tempo_address;
    monitor->early_format =
        (best_tempo_format & 1u) != 0 && (best_tempo_format & 2u) == 0;
    monitor->last_beat = audacious_psx_ram_u16(best_base);
    monitor->last_tick = audacious_psx_ram_u16(best_base + 4u);
    monitor->last_measure = audacious_psx_ram_u16(best_base + 6u);
    monitor->confidence = 0;
    return 1;
}

static void audacious_akao_reject_counter(
    AudaciousAkaoMonitor *monitor,
    uint64_t sample_pos)
{
    if (monitor == nullptr) {
        return;
    }
    if (monitor->counter_base != 0u &&
        monitor->rejected_counter_count < AUDACIOUS_AKAO_MAX_REJECTED_COUNTERS) {
        monitor->rejected_counter_bases[monitor->rejected_counter_count++] =
            monitor->counter_base;
    }
    monitor->counter_base = 0u;
    monitor->tempo_address = 0u;
    monitor->counter_candidate_sample = 0u;
    monitor->confidence = 0u;
    memset(&monitor->state, 0, sizeof(monitor->state));
    monitor->next_counter_scan_sample = sample_pos + AUDACIOUS_SNAPSHOT_INTERVAL_FRAMES;
}

static void audacious_akao_update(AudaciousPsf2Core *core)
{
    AudaciousAkaoMonitor *monitor;
    uint16_t beat;
    uint16_t tick;
    uint16_t measure;

    if (core == nullptr || core->mode != AUDACIOUS_PSF_MODE_PSF1) {
        return;
    }
    monitor = &core->akao;
    if (!monitor->sequences_scanned) {
        if (monitor->sequence_scan_attempts >= 16u ||
            core->sample_pos < monitor->next_sequence_scan_sample) {
            return;
        }
        ++monitor->sequence_scan_attempts;
        audacious_akao_scan_sequences(monitor);
        monitor->next_sequence_scan_sample = core->sample_pos + core->sample_rate / 4u;
    }
    if (!monitor->sequences_scanned) {
        return;
    }
    if (!monitor->started) {
        return;
    }
    if (monitor->counter_base == 0) {
        if (monitor->counter_scan_attempts >= AUDACIOUS_AKAO_MAX_COUNTER_RETRIES ||
            core->sample_pos < monitor->next_counter_scan_sample) {
            return;
        }
        ++monitor->counter_scan_attempts;
        monitor->next_counter_scan_sample = core->sample_pos +
            (monitor->counter_scan_attempts < AUDACIOUS_AKAO_FAST_COUNTER_RETRIES ?
                AUDACIOUS_SNAPSHOT_INTERVAL_FRAMES : core->sample_rate / 4u);
        if (!audacious_akao_find_counter(monitor)) {
            return;
        }
        monitor->counter_candidate_sample = core->sample_pos;
        return;
    }
    if (!audacious_akao_counter_fields_valid(monitor->counter_base)) {
        audacious_akao_reject_counter(monitor, core->sample_pos);
        return;
    }

    beat = audacious_psx_ram_u16(monitor->counter_base);
    tick = audacious_psx_ram_u16(monitor->counter_base + 4u);
    measure = audacious_psx_ram_u16(monitor->counter_base + 6u);
    if (beat != monitor->last_beat || tick != monitor->last_tick ||
        measure != monitor->last_measure) {
        if (monitor->confidence < 3u) {
            ++monitor->confidence;
        }
    } else if (monitor->confidence == 0u && core->sample_rate != 0u &&
        core->sample_pos - monitor->counter_candidate_sample >=
            core->sample_rate / 4u) {
        audacious_akao_reject_counter(monitor, core->sample_pos);
        return;
    }
    monitor->last_beat = beat;
    monitor->last_tick = tick;
    monitor->last_measure = measure;
    if (monitor->confidence == 0u) {
        return;
    }
    monitor->state.tempo = audacious_psx_ram_u16(monitor->tempo_address);
    monitor->state.beats_per_measure =
        audacious_psx_ram_u16(monitor->counter_base - 2u);
    monitor->state.ticks_per_beat =
        audacious_psx_ram_u16(monitor->counter_base + 2u);
    monitor->state.current_beat = beat;
    monitor->state.current_tick = tick;
    monitor->state.measure = measure;
    monitor->state.beat_denominator = monitor->state.ticks_per_beat != 0u ?
        (uint16_t)(192u / monitor->state.ticks_per_beat) : 0u;
    monitor->state.detected = 1u;
    monitor->state.early_format = monitor->early_format;
    monitor->state.driver_type = monitor->early_format ?
        PSF1_MUSIC_DRIVER_AKAO_EARLY : PSF1_MUSIC_DRIVER_AKAO_LATE;
}

static void audacious_emit_internal_snapshots(AudaciousPsf2Core *core);
static void audacious_emit_internal_snapshots_masked(
    AudaciousPsf2Core *core,
    int selected_core,
    uint32_t voice_mask);

void setendless(int e);
int SPUasync(uint32_t cycles, void (*update)(const void *, int));
extern "C" void psf2log_peops_emit_snapshots(
    void *user,
    uint64_t sample_pos,
    Spu2LogResult (*core_snapshot)(void *user, uint64_t sample_pos, const Spu2LogCoreSnapshot *snapshot),
    Spu2LogResult (*voice_snapshot)(void *user, uint64_t sample_pos, const Spu2LogVoiceSnapshot *snapshot));
extern "C" void psf2log_peops_set_mute_mask(uint32_t mask);
extern "C" void psf2log_peops_set_mute_mask_immediate(uint32_t mask);
extern "C" void psf2log_peops_set_timbre_solo(int enabled, const uint32_t *starts, const uint32_t *loops, const uint32_t *flags, uint32_t count);
extern "C" void psf2log_peops_set_main_enabled(int enabled);
extern "C" void psf2log_peops_set_reverb_enabled(int enabled);
extern "C" void psf2log_peops_set_text_log_enabled(int enabled);
extern "C" void psf2log_peops_set_reverb_override_masks(uint32_t force_on_mask, uint32_t force_off_mask);
extern "C" void psf2log_peops_set_noise_override_masks(uint32_t force_on_mask, uint32_t force_off_mask);
extern "C" void psf2log_peops_set_pmod_override_masks(uint32_t force_on_mask, uint32_t force_off_mask);
extern "C" void psf2log_peops_set_adsr_force_mask(uint32_t mask);
extern "C" void psf2log_peops_set_adsr_value(uint32_t voice, uint32_t field, uint32_t value);
extern "C" void psf2log_peops_set_adsr_freeze(int enabled);
extern "C" void psf2log_peops_set_noise_clock(uint32_t value);
extern "C" void psf2log_peops_set_reverb_value(uint32_t side, uint32_t value);
extern "C" void psf2log_peops_set_pitch(uint32_t voice, uint32_t value);
extern "C" void psf2log_peops_set_pitch_lock(uint32_t voice, int enabled, uint32_t value);
extern "C" void psf2log_peops_restore_pitch(uint32_t voice);
extern "C" void psf2log_peops_set_volume(uint32_t voice, uint32_t side, uint32_t value);
extern "C" void psf2log_peops_set_volume_lock(uint32_t voice, uint32_t side, int enabled, uint32_t value);
extern "C" uint32_t psf2log_peops_copy_sample(
    uint32_t start_addr,
    uint32_t loop_addr,
    uint8_t *out_data,
    uint32_t capacity,
    uint32_t *out_loop_offset,
    uint32_t *out_end_flags);
extern "C" void psf2log_peops2_set_mute_masks(uint32_t core0_mask, uint32_t core1_mask);
extern "C" void psf2log_peops2_set_mute_masks_immediate(uint32_t core0_mask, uint32_t core1_mask);
extern "C" void psf2log_peops2_set_timbre_solo(int enabled, const uint32_t *starts, const uint32_t *loops, const uint32_t *flags, uint32_t count);
extern "C" void psf2log_peops2_set_reverb_override_masks(uint32_t core0_on, uint32_t core1_on, uint32_t core0_off, uint32_t core1_off);
extern "C" void psf2log_peops2_set_noise_override_masks(uint32_t core0_on, uint32_t core1_on, uint32_t core0_off, uint32_t core1_off);
extern "C" void psf2log_peops2_set_pmod_override_masks(uint32_t core0_on, uint32_t core1_on, uint32_t core0_off, uint32_t core1_off);
extern "C" void psf2log_peops2_set_adsr_force_masks(uint32_t core0_mask, uint32_t core1_mask);
extern "C" void psf2log_peops2_set_adsr_value(uint32_t core, uint32_t voice, uint32_t field, uint32_t value);
extern "C" void psf2log_peops2_set_adsr_freeze(int enabled);
extern "C" void psf2log_peops2_set_noise_clock(uint32_t core, uint32_t value);
extern "C" void psf2log_peops2_set_reverb_value(uint32_t core, uint32_t side, uint32_t value);
extern "C" void psf2log_peops2_set_pitch(uint32_t core, uint32_t voice, uint32_t value);
extern "C" void psf2log_peops2_set_pitch_lock(uint32_t core, uint32_t voice, int enabled, uint32_t value);
extern "C" void psf2log_peops2_restore_pitch(uint32_t core, uint32_t voice);
extern "C" void psf2log_peops2_set_volume(uint32_t core, uint32_t voice, uint32_t side, uint32_t value);
extern "C" void psf2log_peops2_set_volume_lock(uint32_t core, uint32_t voice, uint32_t side, int enabled, uint32_t value);

extern "C" void psf2log_set_imported_tempo_percent(int tempo_percent)
{
    if (tempo_percent < 0) {
        tempo_percent = 0;
    }
    if (tempo_percent > 0 && tempo_percent < 10) {
        tempo_percent = 10;
    }
    if (tempo_percent > 200) {
        tempo_percent = 200;
    }
    g_tempo_percent = tempo_percent;
}

extern "C" void psf2log_set_imported_frame_advance_mode(int enabled)
{
    g_frame_advance_mode = enabled ? 1 : 0;
    psf2log_set_imported_adsr_freeze(enabled);
    if (!enabled) {
        g_frame_advance_steps = 0;
        g_frame_advance_adsr_updates = 0;
    }
}

extern "C" void psf2log_step_imported_frame_advance(uint32_t ticks)
{
    if (ticks == 0) {
        return;
    }
    g_frame_advance_steps += ticks;
    g_frame_advance_adsr_updates += ticks;
}

extern "C" void psf2log_set_imported_adsr_freeze(int enabled)
{
    psf2log_peops_set_adsr_freeze(enabled);
    psf2log_peops2_set_adsr_freeze(enabled);
}

static int audacious_frame_advance_has_step(void)
{
    return g_frame_advance_mode && g_frame_advance_steps > 0;
}

static void audacious_frame_advance_prepare_audio(void)
{
    if (!g_frame_advance_mode) {
        return;
    }
    psf2log_set_imported_adsr_freeze(g_frame_advance_adsr_updates == 0);
}

static void audacious_frame_advance_finish_audio(void)
{
    if (!g_frame_advance_mode) {
        return;
    }
    if (g_frame_advance_adsr_updates > 0) {
        g_frame_advance_adsr_updates -= 1u;
    }
    psf2log_set_imported_adsr_freeze(1);
}

extern "C" void psf2log_set_imported_reverb_enabled(int enabled)
{
    iUseReverb = enabled ? 1 : 0;
    psf2log_peops_set_reverb_enabled(enabled);
}

extern "C" void psf2log_set_imported_main_enabled(int enabled)
{
    iUseMain = enabled ? 1 : 0;
    psf2log_peops_set_main_enabled(enabled);
}

extern "C" void psf2log_set_imported_text_log_enabled(int enabled)
{
    psf2log_peops_set_text_log_enabled(enabled);
}

extern "C" void psf2log_set_imported_voice_mute_masks(uint32_t core0_mask, uint32_t core1_mask)
{
    psf2log_peops_set_mute_mask(core0_mask);
    psf2log_peops2_set_mute_masks(core0_mask, core1_mask);
}

extern "C" void psf2log_set_imported_voice_mute_masks_immediate(uint32_t core0_mask, uint32_t core1_mask)
{
    psf2log_peops_set_mute_mask_immediate(core0_mask);
    psf2log_peops2_set_mute_masks_immediate(core0_mask, core1_mask);
}

extern "C" void psf2log_set_imported_timbre_solo(int enabled, const uint32_t *starts, const uint32_t *loops, const uint32_t *flags, uint32_t count)
{
    psf2log_peops_set_timbre_solo(enabled, starts, loops, flags, count);
    psf2log_peops2_set_timbre_solo(enabled, starts, loops, flags, count);
}

extern "C" void psf2log_set_imported_voice_reverb_masks(uint32_t core0_on, uint32_t core1_on, uint32_t core0_off, uint32_t core1_off)
{
    psf2log_peops_set_reverb_override_masks(core0_on, core0_off);
    psf2log_peops2_set_reverb_override_masks(core0_on, core1_on, core0_off, core1_off);
}

extern "C" void psf2log_set_imported_voice_noise_masks(uint32_t core0_on, uint32_t core1_on, uint32_t core0_off, uint32_t core1_off)
{
    g_noise_force_on_masks[0] = core0_on & 0x00ffffffu;
    g_noise_force_on_masks[1] = core1_on & 0x00ffffffu;
    g_noise_force_off_masks[0] = core0_off & 0x00ffffffu;
    g_noise_force_off_masks[1] = core1_off & 0x00ffffffu;
    psf2log_peops_set_noise_override_masks(core0_on, core0_off);
    psf2log_peops2_set_noise_override_masks(core0_on, core1_on, core0_off, core1_off);
}

extern "C" void psf2log_set_imported_voice_pmod_masks(uint32_t core0_on, uint32_t core1_on, uint32_t core0_off, uint32_t core1_off)
{
    g_pmod_force_on_masks[0] = core0_on & 0x00ffffffu;
    g_pmod_force_on_masks[1] = core1_on & 0x00ffffffu;
    g_pmod_force_off_masks[0] = core0_off & 0x00ffffffu;
    g_pmod_force_off_masks[1] = core1_off & 0x00ffffffu;
    psf2log_peops_set_pmod_override_masks(core0_on, core0_off);
    psf2log_peops2_set_pmod_override_masks(core0_on, core1_on, core0_off, core1_off);
}

extern "C" void psf2log_set_imported_voice_adsr_force_masks(uint32_t core0_mask, uint32_t core1_mask)
{
    g_adsr_force_masks[0] = core0_mask & 0x00ffffffu;
    g_adsr_force_masks[1] = core1_mask & 0x00ffffffu;
    psf2log_peops_set_adsr_force_mask(core0_mask);
    psf2log_peops2_set_adsr_force_masks(core0_mask, core1_mask);
}

extern "C" void psf2log_set_imported_voice_adsr_value(uint32_t core, uint32_t voice, uint32_t field, uint32_t value)
{
    psf2log_peops_set_adsr_value(voice, field, value);
    psf2log_peops2_set_adsr_value(core, voice, field, value);
}

extern "C" void psf2log_set_imported_noise_clock(uint32_t core, uint32_t value)
{
    psf2log_peops_set_noise_clock(value);
    psf2log_peops2_set_noise_clock(core, value);
}

extern "C" void psf2log_set_imported_reverb_value(uint32_t core, uint32_t side, uint32_t value)
{
    uint32_t raw = ((value & 0x00ffu) << 8) | ((value >> 8) & 0x00ffu);

    psf2log_peops_set_reverb_value(side, raw);
    psf2log_peops2_set_reverb_value(core, side, raw);
}

extern "C" void psf2log_set_imported_voice_pitch(uint32_t core, uint32_t voice, uint32_t value)
{
    psf2log_peops_set_pitch(voice, value);
    psf2log_peops2_set_pitch(core, voice, value);
}

extern "C" void psf2log_set_imported_voice_pitch_lock(uint32_t core, uint32_t voice, int enabled, uint32_t value)
{
    psf2log_peops_set_pitch_lock(voice, enabled, value);
    psf2log_peops2_set_pitch_lock(core, voice, enabled, value);
}

extern "C" void psf2log_restore_imported_voice_pitch(uint32_t core, uint32_t voice)
{
    psf2log_peops_restore_pitch(voice);
    psf2log_peops2_restore_pitch(core, voice);
}

extern "C" void psf2log_set_imported_voice_volume(uint32_t core, uint32_t voice, uint32_t side, uint32_t value)
{
    psf2log_peops_set_volume(voice, side, value);
    psf2log_peops2_set_volume(core, voice, side, value);
}

extern "C" void psf2log_set_imported_voice_volume_lock(uint32_t core, uint32_t voice, uint32_t side, int enabled, uint32_t value)
{
    psf2log_peops_set_volume_lock(voice, side, enabled, value);
    psf2log_peops2_set_volume_lock(core, voice, side, enabled, value);
}

extern "C" void psf2log_abort_imported_render(void)
{
    g_abort_render = 1;
    spu2log_audacious_debug_set_abort_requested(1);
    stop_flag = true;
    mips_shorten_frame();
    mips_set_icount(0);
}

static int read_entire_file(const char *path, uint8_t **out_data, uint32_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *data;

    if (path == nullptr || out_data == nullptr || out_size == nullptr) {
        return 0;
    }

    file = fopen(path, "rb");
    if (file == nullptr) {
        return 0;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }

    size = ftell(file);
    if (size < 0 || size > 0x7fffffffL) {
        fclose(file);
        return 0;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }

    data = (uint8_t *)malloc((size_t)size);
    if (data == nullptr) {
        fclose(file);
        return 0;
    }

    if (size != 0 && fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return 0;
    }

    fclose(file);
    *out_data = data;
    *out_size = (uint32_t)size;
    return 1;
}

static void set_library_dir_from_path(const char *path)
{
    const char *slash;
    const char *backslash;
    size_t length;

    g_library_dir[0] = '\0';
    if (path == nullptr) {
        return;
    }

    slash = strrchr(path, '/');
    backslash = strrchr(path, '\\');
    if (backslash != nullptr && (slash == nullptr || backslash > slash)) {
        slash = backslash;
    }

    if (slash == nullptr) {
        return;
    }

    length = (size_t)(slash - path + 1);
    if (length >= sizeof(g_library_dir)) {
        length = sizeof(g_library_dir) - 1;
    }

    memcpy(g_library_dir, path, length);
    g_library_dir[length] = '\0';
}

Index<char> ao_get_lib(char *filename)
{
    char path[1400];
    uint8_t *data = nullptr;
    uint32_t size = 0;
    Index<char> output;

    if (filename == nullptr || filename[0] == '\0') {
        return output;
    }

    snprintf(path, sizeof(path), "%s%s", g_library_dir, filename);
    if (!read_entire_file(path, &data, &size)) {
        if (!read_entire_file(filename, &data, &size)) {
            fprintf(stderr, "audacious provider: library not found: %s\n", path);
            return output;
        }
    }

    output.append((const char *)data, size);
    free(data);
    return output;
}

static uint16_t audacious_filter_psf2_key_event_value(
    const AudaciousPsf2Core *core,
    uint32_t address,
    uint16_t value)
{
    uint32_t offset;
    uint32_t core_offset;
    unsigned core_index;
    unsigned first_voice;
    unsigned voice_count;
    unsigned bit_index;
    int key_on;
    uint16_t filtered = 0;

    if (core == nullptr || core->mode != AUDACIOUS_PSF_MODE_PSF2) {
        return value;
    }

    offset = address & 0x7ffu;
    core_offset = offset & 0x3ffu;
    core_index = (offset >> 10) & 1u;
    if (core_offset == 0x1a0u || core_offset == 0x1a4u) {
        first_voice = 0;
        voice_count = 16;
    } else if (core_offset == 0x1a2u || core_offset == 0x1a6u) {
        first_voice = 16;
        voice_count = 8;
    } else {
        return value;
    }
    key_on = core_offset == 0x1a0u || core_offset == 0x1a2u;

    for (bit_index = 0; bit_index < voice_count; ++bit_index) {
        unsigned voice = first_voice + bit_index;
        const SPUCHAN2 *channel;
        int active;

        if ((value & (uint16_t)(1u << bit_index)) == 0) {
            continue;
        }
        channel = &s_chan[(core_index * 24u) + voice];
        active = channel->bOn || channel->bNew || channel->ADSRX.EnvelopeVol > 0 ||
            (g_adsr_force_masks[core_index] & (1u << voice)) != 0;
        if ((key_on && channel->bNew && channel->pStart != nullptr) ||
            (!key_on && channel->bStop && active)) {
            filtered |= (uint16_t)(1u << bit_index);
        }
    }

    return filtered;
}

static void audacious_spu2_write16(
    void *user,
    uint64_t sample_pos,
    uint32_t address,
    uint16_t value)
{
    AudaciousPsf2Core *core = (AudaciousPsf2Core *)user;

    if (core != nullptr && core->callbacks.spu2_write16 != nullptr) {
        uint16_t event_value = audacious_filter_psf2_key_event_value(core, address, value);

        if (core->mode == AUDACIOUS_PSF_MODE_PSF1 &&
            value != 0u &&
            (address == 0x1f801d88u || address == 0x1f801d8au)) {
            if (core->sony_seq.valid && !core->sony_seq.started) {
                audacious_sony_seq_start(core);
            }
            core->akao.started = 1u;
        }
        core->spu2_write_count += 1;
        (void)core->callbacks.spu2_write16(core->callbacks.user, sample_pos, address, event_value);
        if (g_fast_timbre_scan && value != 0) {
            if (core->mode == AUDACIOUS_PSF_MODE_PSF1 &&
                (address == 0x1f801d88u || address == 0x1f801d8au)) {
                audacious_emit_internal_snapshots(core);
            } else if (core->mode == AUDACIOUS_PSF_MODE_PSF2 &&
                ((address & 0x3ffu) == 0x1d0u || (address & 0x3ffu) == 0x1d2u)) {
                uint32_t mask = (address & 0x3ffu) == 0x1d2u
                    ? ((uint32_t)value << 16)
                    : (uint32_t)value;
                int selected_core = (int)((address >> 10) & 1u);

                audacious_emit_internal_snapshots_masked(core, selected_core, mask & 0x00ffffffu);
            }
        }
    }
}

static uint16_t clamp_u16_int(int value)
{
    if (value <= 0) {
        return 0;
    }
    if (value >= 0xffff) {
        return 0xffff;
    }
    return (uint16_t)value;
}

static uint16_t swap_u16(uint16_t value)
{
    return (uint16_t)((value << 8) | (value >> 8));
}

static uint32_t spu_ptr_to_addr(const unsigned char *ptr)
{
    const unsigned char *spu_end;

    if (ptr == nullptr || spuMemC == nullptr || ptr < spuMemC) {
        return 0;
    }

    spu_end = spuMemC + (1024u * 1024u * sizeof(unsigned short));
    if (ptr >= spu_end) {
        return 0;
    }

    return (uint32_t)((ptr - spuMemC) >> 1);
}

static uint32_t audacious_spu2_sample_end(AudaciousPsf2Core *core, uint32_t start_addr)
{
    uint32_t slot;
    uint32_t probe;
    uint32_t stored_start;
    size_t byte_offset;
    uint32_t end_addr = start_addr;

    if (core == nullptr || core->mode != AUDACIOUS_PSF_MODE_PSF2 ||
        spuMemC == nullptr || start_addr == 0 || start_addr >= 0x100000u) {
        return 0;
    }
    stored_start = start_addr + 1u;
    slot = (start_addr * 2654435761u) & (AUDACIOUS_SAMPLE_END_CACHE_SIZE - 1u);
    for (probe = 0; probe < 8u; ++probe) {
        uint32_t index = (slot + probe) & (AUDACIOUS_SAMPLE_END_CACHE_SIZE - 1u);

        if (core->sample_end_cache_start[index] == stored_start) {
            return core->sample_end_cache_value[index];
        }
        if (core->sample_end_cache_start[index] == 0) {
            slot = index;
            break;
        }
    }

    byte_offset = (size_t)start_addr << 1;
    while (byte_offset + 16u <= AUDACIOUS_SPU2_RAM_BYTES) {
        uint8_t flags = spuMemC[byte_offset + 1u];

        byte_offset += 16u;
        if ((flags & 1u) != 0) {
            end_addr = (uint32_t)(byte_offset >> 1);
            break;
        }
    }
    core->sample_end_cache_start[slot] = stored_start;
    core->sample_end_cache_value[slot] = end_addr;
    return end_addr;
}

extern "C" uint32_t psf2log_copy_imported_sample(
    Psf2CoreBridge *bridge,
    uint32_t start_addr,
    uint32_t loop_addr,
    uint8_t *out_data,
    uint32_t capacity,
    uint32_t *out_loop_offset,
    uint32_t *out_end_flags)
{
    AudaciousPsf2Core *core = (AudaciousPsf2Core *)bridge;
    size_t start_byte;
    size_t loop_byte;
    size_t offset;
    uint32_t length;
    uint32_t copy_length;
    uint32_t end_flags = 0;

    if (out_loop_offset != nullptr) {
        *out_loop_offset = 0;
    }
    if (out_end_flags != nullptr) {
        *out_end_flags = 0;
    }
    if (core == nullptr) {
        return 0;
    }
    if (core->mode == AUDACIOUS_PSF_MODE_PSF1) {
        return psf2log_peops_copy_sample(
            start_addr,
            loop_addr,
            out_data,
            capacity,
            out_loop_offset,
            out_end_flags);
    }
    if (core->mode != AUDACIOUS_PSF_MODE_PSF2 || spuMemC == nullptr ||
        start_addr == 0 || start_addr >= 0x100000u) {
        return 0;
    }

    start_byte = (size_t)(start_addr & 0x000fffffu) << 1;
    loop_byte = (size_t)(loop_addr & 0x000fffffu) << 1;
    offset = start_byte;
    while (offset + 16u <= AUDACIOUS_SPU2_RAM_BYTES) {
        end_flags = spuMemC[offset + 1u];
        offset += 16u;
        if ((end_flags & 1u) != 0) {
            break;
        }
    }
    if (offset <= start_byte || offset > AUDACIOUS_SPU2_RAM_BYTES) {
        return 0;
    }
    length = (uint32_t)(offset - start_byte);
    copy_length = length < capacity ? length : capacity;
    if (out_data != nullptr && copy_length != 0) {
        memcpy(out_data, spuMemC + start_byte, copy_length);
    }
    if (out_loop_offset != nullptr && loop_byte >= start_byte && loop_byte < offset) {
        *out_loop_offset = (uint32_t)(loop_byte - start_byte);
    }
    if (out_end_flags != nullptr) {
        *out_end_flags = end_flags;
    }
    return length;
}

extern "C" int psf2log_get_imported_ps1_akao_state(
    Psf2CoreBridge *bridge,
    Psf1AkaoPlaybackState *out_state)
{
    AudaciousPsf2Core *core = (AudaciousPsf2Core *)bridge;

    if (out_state == nullptr) {
        return 0;
    }
    memset(out_state, 0, sizeof(*out_state));
    if (core == nullptr || core->mode != AUDACIOUS_PSF_MODE_PSF1) {
        return 0;
    }
    audacious_sony_seq_rescan_table(core);
    if (core->sony_seq.valid) {
        audacious_sony_seq_update(core);
        if (!core->sony_seq.state.detected) {
            return 0;
        }
        *out_state = core->sony_seq.state;
        return 1;
    }
    audacious_akao_update(core);
    if (!core->akao.state.detected) {
        return 0;
    }
    *out_state = core->akao.state;
    return 1;
}

static int effective_mask_bit(uint32_t original, uint32_t force_on, uint32_t force_off, uint8_t voice)
{
    uint32_t bit = 1u << voice;

    if ((force_off & bit) != 0) {
        return 0;
    }
    if ((force_on & bit) != 0) {
        return 1;
    }
    return (original & bit) != 0;
}

static int audacious_adsr_forced(uint8_t core, uint8_t voice)
{
    return (g_adsr_force_masks[core] & (1u << voice)) != 0;
}

static int audacious_channel_active(const SPUCHAN2 *channel, uint8_t core, uint8_t voice)
{
    return channel->bOn || channel->bNew || channel->ADSRX.EnvelopeVol > 0 || audacious_adsr_forced(core, voice);
}

static uint8_t audacious_adsr_phase(const SPUCHAN2 *channel, uint8_t core, uint8_t voice)
{
    if (!audacious_channel_active(channel, core, voice)) {
        return SPU2LOG_ADSR_OFF;
    }

    if (channel->bStop && !audacious_adsr_forced(core, voice)) {
        return SPU2LOG_ADSR_RELEASE;
    }

    switch (channel->ADSRX.State) {
    case 0:
        return SPU2LOG_ADSR_ATTACK;
    case 1:
        return SPU2LOG_ADSR_DECAY;
    case 2:
        return SPU2LOG_ADSR_SUSTAIN;
    default:
        return SPU2LOG_ADSR_SUSTAIN;
    }
}

static uint32_t audacious_voice_flags(const SPUCHAN2 *channel, uint8_t core, uint8_t voice)
{
    uint32_t flags = 0;
    int active = audacious_channel_active(channel, core, voice);
    int adsr_forced = audacious_adsr_forced(core, voice);
    uint32_t bit = 1u << voice;
    int noise = effective_mask_bit(channel->bNoise ? bit : 0u, g_noise_force_on_masks[core], g_noise_force_off_masks[core], voice);
    int pmod = effective_mask_bit(channel->bFMod ? bit : 0u, g_pmod_force_on_masks[core], g_pmod_force_off_masks[core], voice);

    if (active) {
        flags |= SPU2LOG_VOICE_KEY_ON;
    }
    if (active && channel->bStop && !adsr_forced) {
        flags |= SPU2LOG_VOICE_RELEASE;
    }
    if (active && noise) {
        flags |= SPU2LOG_VOICE_NOISE;
    }
    if (active && pmod) {
        flags |= SPU2LOG_VOICE_PMOD;
    }
    if (active && (channel->bReverbL || channel->bReverbR || channel->bRVBActive)) {
        flags |= SPU2LOG_VOICE_REVERB;
    }
    if (active && channel->bVolumeL) {
        flags |= SPU2LOG_VOICE_DRY_L;
    }
    if (active && channel->bVolumeR) {
        flags |= SPU2LOG_VOICE_DRY_R;
    }
    if (active && channel->bReverbL) {
        flags |= SPU2LOG_VOICE_WET_L;
    }
    if (active && channel->bReverbR) {
        flags |= SPU2LOG_VOICE_WET_R;
    }

    return flags;
}

static uint16_t audacious_adsr1_from_channel(const SPUCHAN2 *channel)
{
    uint16_t value = 0;

    if (channel == nullptr) {
        return 0;
    }

    if (channel->ADSRX.AttackModeExp) {
        value |= 0x8000u;
    }
    value |= (uint16_t)((channel->ADSRX.AttackRate & 0x7f) << 8);
    value |= (uint16_t)((channel->ADSRX.DecayRate & 0x0f) << 4);
    value |= (uint16_t)(channel->ADSRX.SustainLevel & 0x0f);
    return value;
}

static uint16_t audacious_adsr2_from_channel(const SPUCHAN2 *channel)
{
    uint16_t value = 0;

    if (channel == nullptr) {
        return 0;
    }

    if (channel->ADSRX.SustainModeExp) {
        value |= 0x8000u;
    }
    if (!channel->ADSRX.SustainIncrease) {
        value |= 0x4000u;
    }
    value |= (uint16_t)((channel->ADSRX.SustainRate & 0x7f) << 6);
    if (channel->ADSRX.ReleaseModeExp) {
        value |= 0x0020u;
    }
    value |= (uint16_t)(channel->ADSRX.ReleaseRate & 0x1f);
    return value;
}

static void audacious_emit_internal_snapshots_masked(
    AudaciousPsf2Core *core,
    int selected_core,
    uint32_t voice_mask)
{
    uint8_t core_index;
    uint8_t voice;

    if (core == nullptr) {
        return;
    }

    if (core->mode == AUDACIOUS_PSF_MODE_PSF1) {
        audacious_sony_seq_rescan_table(core);
        if (core->sony_seq.valid) {
            audacious_sony_seq_update(core);
        } else {
            audacious_akao_update(core);
        }
        psf2log_peops_emit_snapshots(
            core->callbacks.user,
            core->sample_pos,
            core->callbacks.core_snapshot,
            core->callbacks.voice_snapshot);
        return;
    }

    for (core_index = 0; core_index < 2; ++core_index) {
        Spu2LogCoreSnapshot snapshot;
        uint32_t active_mask = 0;
        uint32_t noise_mask = 0;
        uint32_t pmod_mask = 0;
        uint32_t reverb_mask = 0;

        if (selected_core >= 0 && core_index != (uint8_t)selected_core) {
            continue;
        }
        memset(&snapshot, 0, sizeof(snapshot));
        snapshot.core = core_index;
        snapshot.master_l = swap_u16(regArea[AUDACIOUS_SPU2_P_MVOLL(core_index) >> 1]);
        snapshot.master_r = swap_u16(regArea[AUDACIOUS_SPU2_P_MVOLR(core_index) >> 1]);
        snapshot.reverb_l = swap_u16(clamp_u16_int(rvb[core_index].VolLeft));
        snapshot.reverb_r = swap_u16(clamp_u16_int(rvb[core_index].VolRight));
        snapshot.flags = (uint32_t)spuCtrl2[core_index];

        for (voice = 0; voice < 24; ++voice) {
            const SPUCHAN2 *channel = &s_chan[(core_index * 24u) + voice];
            uint32_t bit = 1u << voice;

            if (audacious_channel_active(channel, core_index, voice)) {
                active_mask |= bit;
            }
            if (effective_mask_bit(channel->bNoise ? bit : 0u, g_noise_force_on_masks[core_index], g_noise_force_off_masks[core_index], voice)) {
                noise_mask |= bit;
            }
            if (effective_mask_bit(channel->bFMod ? bit : 0u, g_pmod_force_on_masks[core_index], g_pmod_force_off_masks[core_index], voice)) {
                pmod_mask |= bit;
            }
            if (channel->bReverbL || channel->bReverbR || channel->bRVBActive) {
                reverb_mask |= bit;
            }
        }

        snapshot.key_on_mask = active_mask;
        snapshot.noise_mask = noise_mask;
        snapshot.pmod_mask = pmod_mask;
        snapshot.reverb_mask = reverb_mask;

        if (core->callbacks.core_snapshot != nullptr) {
            (void)core->callbacks.core_snapshot(
                core->callbacks.user,
                core->sample_pos,
                &snapshot);
        }

        for (voice = 0; voice < 24; ++voice) {
            const SPUCHAN2 *channel = &s_chan[(core_index * 24u) + voice];
            Spu2LogVoiceSnapshot snapshot_voice;

            if (selected_core >= 0 && (voice_mask & (1u << voice)) == 0) {
                continue;
            }
            memset(&snapshot_voice, 0, sizeof(snapshot_voice));
            snapshot_voice.core = core_index;
            snapshot_voice.voice = voice;
            snapshot_voice.active = (uint8_t)(audacious_channel_active(channel, core_index, voice) ? 1 : 0);
            snapshot_voice.adsr_phase = audacious_adsr_phase(channel, core_index, voice);
            if (snapshot_voice.active) {
                snapshot_voice.vol_l = clamp_u16_int(channel->iLeftVolume);
                snapshot_voice.vol_r = clamp_u16_int(channel->iRightVolume);
                snapshot_voice.pitch = clamp_u16_int(channel->iRawPitch);
                snapshot_voice.adsr1 = audacious_adsr1_from_channel(channel);
                snapshot_voice.adsr2 = audacious_adsr2_from_channel(channel);
                snapshot_voice.envx = audacious_adsr_forced(core_index, voice) ? 0x7fffu : clamp_u16_int(channel->ADSRX.EnvelopeVol >> 16);
                snapshot_voice.ssa = spu_ptr_to_addr(channel->pStart);
                snapshot_voice.lsa = spu_ptr_to_addr(channel->pLoop);
                snapshot_voice.nax = spu_ptr_to_addr(channel->pCurr);
                snapshot_voice.sample_end = audacious_spu2_sample_end(core, snapshot_voice.ssa);
            }
            snapshot_voice.noise_clock = (uint8_t)(((spuCtrl2[core_index] & 0x3f00u) >> 8) & 0x3fu);
            snapshot_voice.flags = audacious_voice_flags(channel, core_index, voice);

            if (core->callbacks.voice_snapshot != nullptr) {
                (void)core->callbacks.voice_snapshot(
                    core->callbacks.user,
                    core->sample_pos,
                    &snapshot_voice);
            }
        }
    }
}

static void audacious_emit_internal_snapshots(AudaciousPsf2Core *core)
{
    audacious_emit_internal_snapshots_masked(core, -1, 0x00ffffffu);
}

extern "C" void psf2log_emit_imported_snapshot(Psf2CoreBridge *bridge)
{
    AudaciousPsf2Core *core = (AudaciousPsf2Core *)bridge;
    audacious_emit_internal_snapshots(core);
}

extern "C" void psf2log_rebase_imported_sample_position(
    Psf2CoreBridge *bridge,
    uint64_t sample_pos)
{
    AudaciousPsf2Core *core = (AudaciousPsf2Core *)bridge;

    if (core == nullptr) {
        return;
    }
    core->sample_pos = sample_pos;
    core->next_snapshot_sample =
        ((sample_pos / AUDACIOUS_SNAPSHOT_INTERVAL_FRAMES) + 1u) *
        AUDACIOUS_SNAPSHOT_INTERVAL_FRAMES;
    spu2log_audacious_set_sample_pos(sample_pos);
}

extern "C" Psf2CoreBridgeResult psf2log_scan_imported_timbres(
    Psf2CoreBridge *bridge,
    uint32_t sequence_frames,
    uint32_t *out_scanned_frames)
{
    AudaciousPsf2Core *core = (AudaciousPsf2Core *)bridge;
    uint32_t scanned = 0;

    if (out_scanned_frames != nullptr) {
        *out_scanned_frames = 0;
    }
    if (core == nullptr || sequence_frames == 0) {
        return PSF2_CORE_BRIDGE_ERROR_INVALID_ARGUMENT;
    }

    g_fast_timbre_scan = 1;
    while (!g_abort_render && scanned < sequence_frames) {
        uint32_t step = 1u;

        if (core->mode == AUDACIOUS_PSF_MODE_PSF1) {
            uint32_t cpu_step;

            step = AUDACIOUS_PSF1_TIMBRE_SCAN_CPU_DIVIDER;
            if (step > sequence_frames - scanned) {
                step = sequence_frames - scanned;
            }
            cpu_step = core->sample_pos < 44100u * 2u ? step : (step + 7u) / 8u;
            psx_hw_scan_slice(step, cpu_step);
            audacious_sony_seq_advance(core, step);
        } else {
            ps2_hw_scan_slice();
        }
        core->slice_count += step;
        core->sample_pos += step;
        scanned += step;

        if (core->slice_count >= 44100u / 60u) {
            if (core->mode == AUDACIOUS_PSF_MODE_PSF1) {
                psx_hw_frame();
            } else {
                ps2_hw_frame();
            }
            core->slice_count -= 44100u / 60u;
        }

    }

    g_fast_timbre_scan = 0;
    spu2log_audacious_set_sample_pos(core->sample_pos);
    audacious_emit_internal_snapshots(core);
    if (out_scanned_frames != nullptr) {
        *out_scanned_frames = scanned;
    }
    return g_abort_render ? PSF2_CORE_BRIDGE_ERROR_RENDER_FAILED : PSF2_CORE_BRIDGE_OK;
}

static void audacious_audio_update(const void *data, int bytes)
{
    AudaciousPsf2Core *core = g_rendering_core;
    const int16_t *samples = (const int16_t *)data;
    uint32_t frames;
    uint32_t remaining;
    uint32_t sample_index;

    if (core == nullptr || data == nullptr || bytes <= 0) {
        return;
    }

    frames = (uint32_t)bytes / (uint32_t)(sizeof(int16_t) * 2u);
    remaining = core->requested_frames - core->rendered_frames;
    if (frames > remaining) {
        frames = remaining;
    }

    if (frames != 0) {
        if (core->mode == AUDACIOUS_PSF_MODE_PSF1) {
            audacious_sony_seq_rescan_table(core);
        }
        if (core->mode == AUDACIOUS_PSF_MODE_PSF1 &&
            core->sony_seq.valid && !core->sony_seq.started) {
            for (sample_index = 0; sample_index < frames * 2u; ++sample_index) {
                if (samples[sample_index] != 0) {
                    audacious_sony_seq_start(core);
                    break;
                }
            }
        }
        memcpy(core->pcm + core->rendered_frames * 2u, data, frames * 2u * sizeof(int16_t));
        core->rendered_frames += frames;
        core->sample_pos += frames;
        spu2log_audacious_set_sample_pos(core->sample_pos);
    }
}

static Psf2CoreBridgeResult audacious_open(
    Psf2CoreBridge **out_core,
    const char *path,
    uint32_t sample_rate,
    const Psf2CoreCallbacks *callbacks)
{
    AudaciousPsf2Core *core;
    uint8_t *data = nullptr;
    uint32_t size = 0;

    if (out_core == nullptr || path == nullptr || callbacks == nullptr ||
        callbacks->spu2_write16 == nullptr || sample_rate == 0) {
        return PSF2_CORE_BRIDGE_ERROR_INVALID_ARGUMENT;
    }

    if (!read_entire_file(path, &data, &size)) {
        return PSF2_CORE_BRIDGE_ERROR_OPEN_FAILED;
    }

    core = (AudaciousPsf2Core *)calloc(1, sizeof(*core));
    if (core == nullptr) {
        free(data);
        return PSF2_CORE_BRIDGE_ERROR_OPEN_FAILED;
    }

    core->callbacks = *callbacks;
    core->input_data = data;
    core->input_size = size;
    core->sample_rate = sample_rate;
    core->mode = (size >= 4 && data[0] == 'P' && data[1] == 'S' && data[2] == 'F' && data[3] == 0x01) ?
        AUDACIOUS_PSF_MODE_PSF1 : AUDACIOUS_PSF_MODE_PSF2;
    core->next_snapshot_sample = AUDACIOUS_SNAPSHOT_INTERVAL_FRAMES;
    set_library_dir_from_path(path);
    stop_flag = false;
    g_abort_render = 0;
    spu2log_audacious_debug_set_abort_requested(0);
    psf_refresh = -1;
    spu2log_audacious_set_sample_pos(0);
    spu2log_audacious_debug_reset();
    spu2log_audacious_set_spu2_write16_callback(audacious_spu2_write16, core);

    if (core->mode == AUDACIOUS_PSF_MODE_PSF1) {
        setendless(1);
        if (psf_start(data, size) != 1) {
            fprintf(stderr, "audacious provider: psf_start failed\n");
            spu2log_audacious_set_spu2_write16_callback(nullptr, nullptr);
            core->input_data = nullptr;
            core->input_size = 0;
            free(core);
            free(data);
            return PSF2_CORE_BRIDGE_ERROR_OPEN_FAILED;
        }
        setendless(1);
        audacious_sony_seq_scan(core);
    } else if (core->mode == AUDACIOUS_PSF_MODE_PSF2) {
        setendless2(1);
        if (psf2_start(data, size) != 1) {
            fprintf(stderr, "audacious provider: psf2_start failed\n");
            spu2log_audacious_set_spu2_write16_callback(nullptr, nullptr);
            core->input_data = nullptr;
            core->input_size = 0;
            free(core);
            free(data);
            return PSF2_CORE_BRIDGE_ERROR_OPEN_FAILED;
        }
    } else {
        spu2log_audacious_set_spu2_write16_callback(nullptr, nullptr);
        core->input_data = nullptr;
        core->input_size = 0;
        free(core);
        free(data);
        return PSF2_CORE_BRIDGE_ERROR_OPEN_FAILED;
    }

    *out_core = (Psf2CoreBridge *)core;
    return PSF2_CORE_BRIDGE_OK;
}

static void audacious_run_cpu_for_output_sample(AudaciousPsf2Core *core)
{
    int tempo_percent = g_tempo_percent;

    if (g_frame_advance_mode) {
        if (g_frame_advance_steps == 0) {
            return;
        }
        tempo_percent = 100;
    }
    if (tempo_percent < 10) {
        if (tempo_percent <= 0) {
            return;
        }
        tempo_percent = 10;
    }
    if (tempo_percent > 200) {
        tempo_percent = 200;
    }

    core->tempo_accumulator += (uint32_t)tempo_percent;
    while (!g_abort_render && core->tempo_accumulator >= 100u) {
        spu2log_audacious_debug_set_stage(3);
        if (core->mode == AUDACIOUS_PSF_MODE_PSF1) {
            psx_hw_slice();
            audacious_sony_seq_advance(core, 1u);
        } else {
            ps2_hw_slice();
        }
        spu2log_audacious_debug_set_stage(1);
        core->slice_count += 1;
        core->tempo_accumulator -= 100u;
        if (g_frame_advance_mode && g_frame_advance_steps > 0) {
            g_frame_advance_steps -= 1u;
        }

        if (core->slice_count >= 44100u / 60u) {
            spu2log_audacious_debug_set_stage(4);
            if (core->mode == AUDACIOUS_PSF_MODE_PSF1) {
                psx_hw_frame();
            } else {
                ps2_hw_frame();
            }
            spu2log_audacious_debug_set_stage(1);
            core->slice_count = 0;
        }
    }
}

static Psf2CoreBridgeResult audacious_render(
    Psf2CoreBridge *bridge,
    int16_t *stereo_pcm,
    uint32_t frames,
    uint32_t *out_frames)
{
    AudaciousPsf2Core *core = (AudaciousPsf2Core *)bridge;
    uint32_t iterations;
    uint32_t max_iterations;
    uint64_t render_start_sample;

    if (core == nullptr || stereo_pcm == nullptr || frames == 0) {
        return PSF2_CORE_BRIDGE_ERROR_INVALID_ARGUMENT;
    }

    memset(stereo_pcm, 0, frames * 2u * sizeof(int16_t));
    core->pcm = stereo_pcm;
    core->requested_frames = frames;
    core->rendered_frames = 0;
    iterations = 0;
    max_iterations = frames + (core->spu2_write_count == 0 ?
        AUDACIOUS_BOOTSTRAP_EXTRA_SLICES : AUDACIOUS_STEADY_EXTRA_SLICES);
    render_start_sample = core->sample_pos;
    g_rendering_core = core;

    while (!g_abort_render && core->rendered_frames < frames && iterations < max_iterations) {
        uint32_t bounded_iterations = iterations < frames ? iterations : frames;
        if (core->mode == AUDACIOUS_PSF_MODE_PSF1) {
            audacious_run_cpu_for_output_sample(core);
            spu2log_audacious_debug_set_stage(1);
            spu2log_audacious_set_sample_pos(render_start_sample + bounded_iterations);
            spu2log_audacious_debug_set_stage(2);
            audacious_frame_advance_prepare_audio();
            SPUasync(384, audacious_audio_update);
            audacious_frame_advance_finish_audio();
        } else {
            if (audacious_frame_advance_has_step()) {
                audacious_run_cpu_for_output_sample(core);
            }
            spu2log_audacious_debug_set_stage(1);
            spu2log_audacious_set_sample_pos(render_start_sample + bounded_iterations);
            spu2log_audacious_debug_set_stage(2);
            audacious_frame_advance_prepare_audio();
            SPU2async(audacious_audio_update);
            audacious_frame_advance_finish_audio();
            spu2log_audacious_debug_set_stage(1);
            if (!g_frame_advance_mode) {
                audacious_run_cpu_for_output_sample(core);
            }
        }
        spu2log_audacious_debug_set_stage(1);
        iterations += 1;

        if (core->sample_pos >= core->next_snapshot_sample) {
            spu2log_audacious_debug_set_stage(5);
            audacious_emit_internal_snapshots(core);
            spu2log_audacious_debug_set_stage(1);
            core->next_snapshot_sample += AUDACIOUS_SNAPSHOT_INTERVAL_FRAMES;
        }
    }

    spu2log_audacious_debug_set_stage(0);
    g_rendering_core = nullptr;
    core->pcm = nullptr;

    if (out_frames != nullptr) {
        *out_frames = core->rendered_frames;
    }

    if (core->rendered_frames == 0) {
        core->rendered_frames = frames;
        core->sample_pos += frames;
        spu2log_audacious_set_sample_pos(core->sample_pos);
        if (out_frames != nullptr) {
            *out_frames = core->rendered_frames;
        }
    }

    while (core->sample_pos >= core->next_snapshot_sample) {
        audacious_emit_internal_snapshots(core);
        core->next_snapshot_sample += AUDACIOUS_SNAPSHOT_INTERVAL_FRAMES;
    }

    if (g_abort_render) {
        return PSF2_CORE_BRIDGE_ERROR_RENDER_FAILED;
    }

    if (getenv("PSF2LOG_DEBUG_RENDER") != nullptr) {
        fprintf(stderr,
            "audacious provider: render frames=%lu iterations=%lu rendered=%lu callback_writes=%llu hook_writes=%llu sample=%llu slices=%llu cpu_slices=%llu hw_writes=%llu stage=%lu cur_thread=%d pc=0x%08lx op=0x%08lx delayr=0x%08lx delayv=0x%08lx last_hw=0x%08lx\n",
            (unsigned long)frames,
            (unsigned long)iterations,
            (unsigned long)core->rendered_frames,
            (unsigned long long)core->spu2_write_count,
            (unsigned long long)spu2log_audacious_get_spu2_write16_count(),
            (unsigned long long)core->sample_pos,
            (unsigned long long)spu2log_audacious_debug_get_slice_count(),
            (unsigned long long)spu2log_audacious_debug_get_cpu_slice_count(),
            (unsigned long long)spu2log_audacious_debug_get_hw_write_count(),
            (unsigned long)spu2log_audacious_debug_get_stage(),
            spu2log_audacious_debug_get_current_thread(),
            (unsigned long)spu2log_audacious_debug_get_pc(),
            (unsigned long)spu2log_audacious_debug_get_opcode(),
            (unsigned long)spu2log_audacious_debug_get_delayr(),
            (unsigned long)spu2log_audacious_debug_get_delayv(),
            (unsigned long)spu2log_audacious_debug_get_last_hw_write());
    }

    return PSF2_CORE_BRIDGE_OK;
}

static void audacious_close(Psf2CoreBridge *bridge)
{
    AudaciousPsf2Core *core = (AudaciousPsf2Core *)bridge;

    if (core == nullptr) {
        return;
    }

    g_abort_render = 1;
    stop_flag = true;
    if (core->mode == AUDACIOUS_PSF_MODE_PSF1) {
        psf_stop();
    } else {
        psf2_stop();
    }
    stop_flag = true;
    spu2log_audacious_set_spu2_write16_callback(nullptr, nullptr);
    free(core->input_data);
    core->input_data = nullptr;
    core->input_size = 0;
    free(core);
}

static const Psf2CoreProvider audacious_provider = {
    "audacious-psf-psf2-core",
    "BSD + GPL/PeOPS/PeOPS2",
    audacious_open,
    audacious_render,
    audacious_close
};

extern "C" const Psf2CoreProvider *psf2log_get_imported_provider(void)
{
    return &audacious_provider;
}

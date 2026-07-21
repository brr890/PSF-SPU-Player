#include "spu2log_spu2_adapter.h"

#include <string.h>

#define SPU2_CORE_STRIDE 0x0400u
#define SPU2_VOICE_STRIDE 0x10u
#define SPU2_VOICE_AREA_SIZE (24u * SPU2_VOICE_STRIDE)

#define SPU2_VOICE_VOL_L 0x00u
#define SPU2_VOICE_VOL_R 0x02u
#define SPU2_VOICE_PITCH 0x04u
#define SPU2_VOICE_ADSR1 0x06u
#define SPU2_VOICE_ADSR2 0x08u
#define SPU2_VOICE_ENVX  0x0au
#define SPU2_VOICE_VOLX  0x0cu

#define SPU2_REG_PMON  0x0180u
#define SPU2_REG_NON   0x0184u
#define SPU2_REG_VMIXEL 0x018cu
#define SPU2_REG_VMIXER 0x0194u
#define SPU2_REG_MMIX  0x0198u
#define SPU2_REG_KON   0x01a0u
#define SPU2_REG_KON_HI 0x01a2u
#define SPU2_REG_KOFF  0x01a4u
#define SPU2_REG_KOFF_HI 0x01a6u

static uint32_t lower24(uint32_t value)
{
    return value & 0x00ffffffu;
}

static uint32_t register_voice_mask(uint16_t value, int high)
{
    if (high) {
        return ((uint32_t)value & 0x00ffu) << 16;
    }
    return (uint32_t)value;
}

static void apply_voice_mask_flag(
    Spu2LogSpu2Adapter *adapter,
    uint8_t core,
    uint32_t mask,
    uint32_t flag,
    int enabled)
{
    uint8_t voice;

    mask = lower24(mask);
    for (voice = 0; voice < 24; ++voice) {
        if ((mask & (1u << voice)) == 0) {
            continue;
        }

        if (enabled) {
            adapter->voices[core][voice].flags |= flag;
        } else {
            adapter->voices[core][voice].flags &= ~flag;
        }
    }
}

static void apply_key_event(
    Spu2LogSpu2Adapter *adapter,
    uint8_t core,
    uint32_t mask,
    int key_on)
{
    uint8_t voice;

    mask = lower24(mask);
    for (voice = 0; voice < 24; ++voice) {
        Spu2LogVoiceSnapshot *snapshot;

        if ((mask & (1u << voice)) == 0) {
            continue;
        }

        snapshot = &adapter->voices[core][voice];
        snapshot->active = 1;

        if (key_on) {
            snapshot->flags |= SPU2LOG_VOICE_KEY_ON;
            snapshot->flags &= ~SPU2LOG_VOICE_RELEASE;
            snapshot->adsr_phase = SPU2LOG_ADSR_ATTACK;
        } else {
            snapshot->flags &= ~SPU2LOG_VOICE_KEY_ON;
            snapshot->flags |= SPU2LOG_VOICE_RELEASE;
            snapshot->adsr_phase = SPU2LOG_ADSR_RELEASE;
        }
    }
}

static Spu2LogResult maybe_flush_snapshots(Spu2LogSpu2Adapter *adapter)
{
    if (adapter->snapshot_interval_samples == 0) {
        return SPU2LOG_OK;
    }

    if (adapter->sample_pos < adapter->next_snapshot_sample) {
        return SPU2LOG_OK;
    }

    adapter->next_snapshot_sample = adapter->sample_pos + adapter->snapshot_interval_samples;
    return spu2log_spu2_flush_snapshots(adapter);
}

void spu2log_spu2_adapter_init(
    Spu2LogSpu2Adapter *adapter,
    Spu2LogHandle *log,
    uint32_t snapshot_interval_samples)
{
    uint8_t core;
    uint8_t voice;

    if (adapter == NULL) {
        return;
    }

    memset(adapter, 0, sizeof(*adapter));
    adapter->log = log;
    adapter->snapshot_interval_samples = snapshot_interval_samples;
    adapter->next_snapshot_sample = snapshot_interval_samples;

    for (core = 0; core < 2; ++core) {
        adapter->cores[core].core = core;
        for (voice = 0; voice < 24; ++voice) {
            adapter->voices[core][voice].core = core;
            adapter->voices[core][voice].voice = voice;
            adapter->voices[core][voice].adsr_phase = SPU2LOG_ADSR_OFF;
        }
    }
}

void spu2log_spu2_set_sample_pos(
    Spu2LogSpu2Adapter *adapter,
    uint64_t sample_pos)
{
    if (adapter == NULL) {
        return;
    }

    adapter->sample_pos = sample_pos;
}

Spu2LogResult spu2log_spu2_write16(
    Spu2LogSpu2Adapter *adapter,
    uint32_t address,
    uint16_t value)
{
    uint32_t core_offset;
    uint8_t core;
    Spu2LogResult result;

    if (adapter == NULL || adapter->log == NULL) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    result = spu2log_reg_write(adapter->log, adapter->sample_pos, address, value);
    if (result != SPU2LOG_OK) {
        return result;
    }

    core = (uint8_t)((address / SPU2_CORE_STRIDE) & 1u);
    core_offset = address % SPU2_CORE_STRIDE;

    if (core_offset < SPU2_VOICE_AREA_SIZE) {
        uint8_t voice = (uint8_t)(core_offset / SPU2_VOICE_STRIDE);
        uint32_t field = core_offset % SPU2_VOICE_STRIDE;
        Spu2LogVoiceSnapshot *snapshot = &adapter->voices[core][voice];

        switch (field) {
        case SPU2_VOICE_VOL_L:
            snapshot->vol_l = value;
            break;
        case SPU2_VOICE_VOL_R:
            snapshot->vol_r = value;
            break;
        case SPU2_VOICE_PITCH:
            snapshot->pitch = value;
            break;
        case SPU2_VOICE_ADSR1:
            snapshot->adsr1 = value;
            break;
        case SPU2_VOICE_ADSR2:
            snapshot->adsr2 = value;
            break;
        case SPU2_VOICE_ENVX:
            snapshot->envx = value;
            if (value == 0 && snapshot->adsr_phase == SPU2LOG_ADSR_RELEASE) {
                snapshot->active = 0;
                snapshot->adsr_phase = SPU2LOG_ADSR_OFF;
            }
            break;
        case SPU2_VOICE_VOLX:
            break;
        default:
            break;
        }

        return maybe_flush_snapshots(adapter);
    }

    switch (core_offset) {
    case SPU2_REG_PMON:
        adapter->cores[core].pmod_mask = lower24(value);
        apply_voice_mask_flag(adapter, core, value, SPU2LOG_VOICE_PMOD, 1);
        break;
    case SPU2_REG_NON:
        adapter->cores[core].noise_mask = lower24(value);
        apply_voice_mask_flag(adapter, core, value, SPU2LOG_VOICE_NOISE, 1);
        break;
    case SPU2_REG_VMIXEL:
        adapter->cores[core].reverb_mask = lower24(value);
        apply_voice_mask_flag(adapter, core, value, SPU2LOG_VOICE_REVERB, 1);
        apply_voice_mask_flag(adapter, core, value, SPU2LOG_VOICE_WET_L, 1);
        break;
    case SPU2_REG_VMIXER:
        adapter->cores[core].reverb_mask |= lower24(value);
        apply_voice_mask_flag(adapter, core, value, SPU2LOG_VOICE_REVERB, 1);
        apply_voice_mask_flag(adapter, core, value, SPU2LOG_VOICE_WET_R, 1);
        break;
    case SPU2_REG_MMIX:
        adapter->cores[core].flags = value;
        break;
    case SPU2_REG_KON:
    case SPU2_REG_KON_HI: {
        uint32_t mask = register_voice_mask(value, core_offset == SPU2_REG_KON_HI);
        adapter->cores[core].key_on_mask |= mask;
        adapter->cores[core].key_off_mask &= ~mask;
        apply_key_event(adapter, core, mask, 1);
        result = spu2log_key_on(adapter->log, adapter->sample_pos, core, mask);
        if (result != SPU2LOG_OK) {
            return result;
        }
        break;
    }
    case SPU2_REG_KOFF:
    case SPU2_REG_KOFF_HI: {
        uint32_t mask = register_voice_mask(value, core_offset == SPU2_REG_KOFF_HI);
        adapter->cores[core].key_off_mask |= mask;
        adapter->cores[core].key_on_mask &= ~mask;
        apply_key_event(adapter, core, mask, 0);
        result = spu2log_key_off(adapter->log, adapter->sample_pos, core, mask);
        if (result != SPU2LOG_OK) {
            return result;
        }
        break;
    }
    default:
        break;
    }

    return maybe_flush_snapshots(adapter);
}

Spu2LogResult spu2log_spu2_flush_snapshots(Spu2LogSpu2Adapter *adapter)
{
    uint8_t core;
    uint8_t voice;
    Spu2LogResult result;

    if (adapter == NULL || adapter->log == NULL) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    for (core = 0; core < 2; ++core) {
        result = spu2log_core_snapshot(adapter->log, adapter->sample_pos, &adapter->cores[core]);
        if (result != SPU2LOG_OK) {
            return result;
        }

        for (voice = 0; voice < 24; ++voice) {
            const Spu2LogVoiceSnapshot *snapshot = &adapter->voices[core][voice];

            if (!snapshot->active && snapshot->flags == 0 && snapshot->envx == 0) {
                continue;
            }

            result = spu2log_voice_snapshot(adapter->log, adapter->sample_pos, snapshot);
            if (result != SPU2LOG_OK) {
                return result;
            }
        }
    }

    return SPU2LOG_OK;
}

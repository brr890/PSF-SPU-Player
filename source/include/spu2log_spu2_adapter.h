#ifndef SPU2LOG_SPU2_ADAPTER_H
#define SPU2LOG_SPU2_ADAPTER_H

#include "spu2log.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Spu2LogSpu2Adapter {
    Spu2LogHandle *log;
    Spu2LogCoreSnapshot cores[2];
    Spu2LogVoiceSnapshot voices[2][24];
    uint64_t sample_pos;
    uint32_t snapshot_interval_samples;
    uint64_t next_snapshot_sample;
} Spu2LogSpu2Adapter;

void spu2log_spu2_adapter_init(
    Spu2LogSpu2Adapter *adapter,
    Spu2LogHandle *log,
    uint32_t snapshot_interval_samples);

void spu2log_spu2_set_sample_pos(
    Spu2LogSpu2Adapter *adapter,
    uint64_t sample_pos);

Spu2LogResult spu2log_spu2_write16(
    Spu2LogSpu2Adapter *adapter,
    uint32_t address,
    uint16_t value);

Spu2LogResult spu2log_spu2_flush_snapshots(Spu2LogSpu2Adapter *adapter);

#ifdef __cplusplus
}
#endif

#endif


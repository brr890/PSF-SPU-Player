#ifndef SPU2LOG_AUDACIOUS_HOOKS_H
#define SPU2LOG_AUDACIOUS_HOOKS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*Spu2LogAudaciousSpu2Write16Callback)(
    void *user,
    uint64_t sample_pos,
    uint32_t address,
    uint16_t value);

void spu2log_audacious_set_spu2_write16_callback(
    Spu2LogAudaciousSpu2Write16Callback callback,
    void *user);

void spu2log_audacious_set_sample_pos(uint64_t sample_pos);
void spu2log_audacious_advance_sample_pos(uint32_t frames);
uint64_t spu2log_audacious_get_sample_pos(void);
uint64_t spu2log_audacious_get_spu2_write16_count(void);

void spu2log_audacious_spu2_write16(uint32_t address, uint16_t value);

void spu2log_audacious_debug_reset(void);
void spu2log_audacious_debug_set_abort_requested(int requested);
int spu2log_audacious_debug_get_abort_requested(void);
void spu2log_audacious_debug_set_stage(uint32_t stage);
uint32_t spu2log_audacious_debug_get_stage(void);
void spu2log_audacious_debug_set_hle(uint32_t pc, uint32_t subcall);
uint32_t spu2log_audacious_debug_get_hle_pc(void);
uint32_t spu2log_audacious_debug_get_hle_subcall(void);
void spu2log_audacious_debug_ps2_slice(
    int current_thread,
    int ran_cpu,
    uint32_t pc,
    uint32_t opcode,
    uint32_t sp,
    uint32_t ra,
    uint32_t v0,
    uint32_t a0,
    uint32_t a1,
    uint32_t delayr,
    uint32_t delayv);
void spu2log_audacious_debug_hw_write(uint32_t address);
uint64_t spu2log_audacious_debug_get_slice_count(void);
uint64_t spu2log_audacious_debug_get_cpu_slice_count(void);
uint64_t spu2log_audacious_debug_get_hw_write_count(void);
int spu2log_audacious_debug_get_current_thread(void);
uint32_t spu2log_audacious_debug_get_last_hw_write(void);
uint32_t spu2log_audacious_debug_get_pc(void);
uint32_t spu2log_audacious_debug_get_opcode(void);
uint32_t spu2log_audacious_debug_get_sp(void);
uint32_t spu2log_audacious_debug_get_ra(void);
uint32_t spu2log_audacious_debug_get_v0(void);
uint32_t spu2log_audacious_debug_get_a0(void);
uint32_t spu2log_audacious_debug_get_a1(void);
uint32_t spu2log_audacious_debug_get_delayr(void);
uint32_t spu2log_audacious_debug_get_delayv(void);

#ifdef __cplusplus
}
#endif

#endif

#include "spu2log_audacious_hooks.h"

static Spu2LogAudaciousSpu2Write16Callback g_spu2_write16_callback;
static void *g_spu2_write16_user;
static uint64_t g_sample_pos;
static uint64_t g_spu2_write16_count;
static uint64_t g_debug_slice_count;
static uint64_t g_debug_cpu_slice_count;
static uint64_t g_debug_hw_write_count;
static int g_debug_current_thread = -1;
static uint32_t g_debug_last_hw_write;
static uint32_t g_debug_pc;
static uint32_t g_debug_opcode;
static uint32_t g_debug_sp;
static uint32_t g_debug_ra;
static uint32_t g_debug_v0;
static uint32_t g_debug_a0;
static uint32_t g_debug_a1;
static uint32_t g_debug_delayr;
static uint32_t g_debug_delayv;
static volatile int g_debug_abort_requested;
static volatile uint32_t g_debug_stage;
static uint32_t g_debug_hle_pc;
static uint32_t g_debug_hle_subcall;

void spu2log_audacious_set_spu2_write16_callback(
    Spu2LogAudaciousSpu2Write16Callback callback,
    void *user)
{
    g_spu2_write16_callback = callback;
    g_spu2_write16_user = user;
}

void spu2log_audacious_set_sample_pos(uint64_t sample_pos)
{
    g_sample_pos = sample_pos;
}

void spu2log_audacious_advance_sample_pos(uint32_t frames)
{
    g_sample_pos += frames;
}

uint64_t spu2log_audacious_get_sample_pos(void)
{
    return g_sample_pos;
}

uint64_t spu2log_audacious_get_spu2_write16_count(void)
{
    return g_spu2_write16_count;
}

void spu2log_audacious_spu2_write16(uint32_t address, uint16_t value)
{
    g_spu2_write16_count += 1;
    if (g_spu2_write16_callback != 0) {
        g_spu2_write16_callback(g_spu2_write16_user, g_sample_pos, address, value);
    }
}

void spu2log_audacious_debug_reset(void)
{
    g_debug_slice_count = 0;
    g_debug_cpu_slice_count = 0;
    g_debug_hw_write_count = 0;
    g_debug_current_thread = -1;
    g_debug_last_hw_write = 0;
    g_debug_pc = 0;
    g_debug_opcode = 0;
    g_debug_sp = 0;
    g_debug_ra = 0;
    g_debug_v0 = 0;
    g_debug_a0 = 0;
    g_debug_a1 = 0;
    g_debug_delayr = 0;
    g_debug_delayv = 0;
    g_debug_abort_requested = 0;
    g_debug_stage = 0;
    g_debug_hle_pc = 0;
    g_debug_hle_subcall = 0;
}

void spu2log_audacious_debug_set_abort_requested(int requested)
{
    g_debug_abort_requested = requested ? 1 : 0;
}

int spu2log_audacious_debug_get_abort_requested(void)
{
    return g_debug_abort_requested;
}

void spu2log_audacious_debug_set_stage(uint32_t stage)
{
    g_debug_stage = stage;
}

uint32_t spu2log_audacious_debug_get_stage(void)
{
    return g_debug_stage;
}

void spu2log_audacious_debug_set_hle(uint32_t pc, uint32_t subcall)
{
    g_debug_hle_pc = pc;
    g_debug_hle_subcall = subcall;
}

uint32_t spu2log_audacious_debug_get_hle_pc(void)
{
    return g_debug_hle_pc;
}

uint32_t spu2log_audacious_debug_get_hle_subcall(void)
{
    return g_debug_hle_subcall;
}

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
    uint32_t delayv)
{
    g_debug_slice_count += 1;
    g_debug_current_thread = current_thread;
    g_debug_pc = pc;
    g_debug_opcode = opcode;
    g_debug_sp = sp;
    g_debug_ra = ra;
    g_debug_v0 = v0;
    g_debug_a0 = a0;
    g_debug_a1 = a1;
    g_debug_delayr = delayr;
    g_debug_delayv = delayv;
    if (ran_cpu) {
        g_debug_cpu_slice_count += 1;
    }
}

void spu2log_audacious_debug_hw_write(uint32_t address)
{
    g_debug_hw_write_count += 1;
    g_debug_last_hw_write = address;
}

uint64_t spu2log_audacious_debug_get_slice_count(void)
{
    return g_debug_slice_count;
}

uint64_t spu2log_audacious_debug_get_cpu_slice_count(void)
{
    return g_debug_cpu_slice_count;
}

uint64_t spu2log_audacious_debug_get_hw_write_count(void)
{
    return g_debug_hw_write_count;
}

int spu2log_audacious_debug_get_current_thread(void)
{
    return g_debug_current_thread;
}

uint32_t spu2log_audacious_debug_get_last_hw_write(void)
{
    return g_debug_last_hw_write;
}

uint32_t spu2log_audacious_debug_get_pc(void)
{
    return g_debug_pc;
}

uint32_t spu2log_audacious_debug_get_opcode(void)
{
    return g_debug_opcode;
}

uint32_t spu2log_audacious_debug_get_sp(void)
{
    return g_debug_sp;
}

uint32_t spu2log_audacious_debug_get_ra(void)
{
    return g_debug_ra;
}

uint32_t spu2log_audacious_debug_get_v0(void)
{
    return g_debug_v0;
}

uint32_t spu2log_audacious_debug_get_a0(void)
{
    return g_debug_a0;
}

uint32_t spu2log_audacious_debug_get_a1(void)
{
    return g_debug_a1;
}

uint32_t spu2log_audacious_debug_get_delayr(void)
{
    return g_debug_delayr;
}

uint32_t spu2log_audacious_debug_get_delayv(void)
{
    return g_debug_delayv;
}

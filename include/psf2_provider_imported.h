#ifndef PSF2_PROVIDER_IMPORTED_H
#define PSF2_PROVIDER_IMPORTED_H

#include "psf2_core_bridge.h"

#ifdef __cplusplus
extern "C" {
#endif

const Psf2CoreProvider *psf2log_get_imported_provider(void);
void psf2log_set_imported_tempo_percent(int tempo_percent);
void psf2log_set_imported_frame_advance_mode(int enabled);
void psf2log_step_imported_frame_advance(uint32_t ticks);
void psf2log_set_imported_adsr_freeze(int enabled);
void psf2log_emit_imported_snapshot(Psf2CoreBridge *core);
void psf2log_rebase_imported_sample_position(Psf2CoreBridge *core, uint64_t sample_pos);
Psf2CoreBridgeResult psf2log_scan_imported_timbres(
    Psf2CoreBridge *core,
    uint32_t sequence_frames,
    uint32_t *out_scanned_frames);
uint32_t psf2log_copy_imported_sample(
    Psf2CoreBridge *core,
    uint32_t start_addr,
    uint32_t loop_addr,
    uint8_t *out_data,
    uint32_t capacity,
    uint32_t *out_loop_offset,
    uint32_t *out_end_flags);
void psf2log_set_imported_reverb_enabled(int enabled);
void psf2log_set_imported_main_enabled(int enabled);
void psf2log_set_imported_text_log_enabled(int enabled);
void psf2log_set_imported_voice_mute_masks(uint32_t core0_mask, uint32_t core1_mask);
void psf2log_set_imported_voice_mute_masks_immediate(uint32_t core0_mask, uint32_t core1_mask);
void psf2log_set_imported_timbre_solo(int enabled, const uint32_t *starts, const uint32_t *loops, const uint32_t *flags, uint32_t count);
void psf2log_set_imported_voice_reverb_masks(uint32_t core0_on, uint32_t core1_on, uint32_t core0_off, uint32_t core1_off);
void psf2log_set_imported_voice_noise_masks(uint32_t core0_on, uint32_t core1_on, uint32_t core0_off, uint32_t core1_off);
void psf2log_set_imported_voice_pmod_masks(uint32_t core0_on, uint32_t core1_on, uint32_t core0_off, uint32_t core1_off);
void psf2log_set_imported_voice_adsr_force_masks(uint32_t core0_mask, uint32_t core1_mask);
void psf2log_set_imported_voice_adsr_value(uint32_t core, uint32_t voice, uint32_t field, uint32_t value);
void psf2log_set_imported_noise_clock(uint32_t core, uint32_t value);
void psf2log_set_imported_reverb_value(uint32_t core, uint32_t side, uint32_t value);
void psf2log_set_imported_voice_pitch(uint32_t core, uint32_t voice, uint32_t value);
void psf2log_set_imported_voice_pitch_lock(uint32_t core, uint32_t voice, int enabled, uint32_t value);
void psf2log_restore_imported_voice_pitch(uint32_t core, uint32_t voice);
void psf2log_set_imported_voice_volume(uint32_t core, uint32_t voice, uint32_t side, uint32_t value);
void psf2log_set_imported_voice_volume_lock(uint32_t core, uint32_t voice, uint32_t side, int enabled, uint32_t value);
void psf2log_abort_imported_render(void);

#ifdef __cplusplus
}
#endif

#endif

#include "psf2_core_bridge.h"
#include "psf_file.h"
#include "psf2_player_version.h"
#include "psf2_provider_imported.h"
#include "spu2log.h"
#include "spu2log_audacious_hooks.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <mmsystem.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1
#define DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 19
#endif

#define PLAYER_TIMER_ID 1u
#define PLAYER_CLICK_TIMER_ID 2u
#define PLAYER_PLAYLIST_SWITCH_TIMER_ID 3u
#define PLAYER_DIRECT_RETRY_TIMER_ID 4u
#define PLAYER_DISPLAY_FPS 120u
#define PLAYER_TIMER_MS 8u
#define PLAYER_CLICK_DELAY_MS 220u
#define PLAYER_CLICK_REPEAT_MS 85u
#define PLAYER_PLAYLIST_SWITCH_TIMER_MS 15u
#define PLAYER_DIRECT_RETRY_TIMER_MS 60u
#define PLAYER_START_RETRY_TIMEOUT_MS 5000u
#define PLAYER_RENDER_FRAMES 735u
#define PLAYER_FRAME_ADVANCE_TICK_FRAMES 192u
#define PLAYER_SEEK_RENDER_FRAMES 65536u
#define PLAYER_SAMPLE_RATE 44100u
#define PLAYER_WM_WORKER_UPDATE (WM_APP + 1u)
#define PLAYER_WM_PLAYLIST_PLAY (WM_APP + 2u)
#define PLAYER_WM_PLAYBACK_ENDED (WM_APP + 3u)
#define PLAYER_WM_AUDIO_BUFFER_DONE (WM_APP + 4u)
#define PLAYER_WM_AUDIO_STARTING (WM_APP + 5u)
#define PLAYER_AUDIO_BUFFERS 16u
#define PLAYER_AUDIO_PREBUFFER_BUFFERS 8u
#define PLAYER_STARTUP_SILENCE_BUFFERS 3u
#define VOICE_EVENT_DISPLAY_BUFFERS 3u
#define PLAYER_MAX_OUTPUT_FRAMES PLAYER_RENDER_FRAMES
#define PLAYER_STARTUP_FADE_FRAMES 128u
#define PLAYER_TRACK_SWITCH_FADE_FRAMES 256u
#define VOICE_DISPLAY_HOLD_SAMPLES ((PLAYER_SAMPLE_RATE * 100u) / 1000u)
#define TIMBRE_SOLO_MAX_KEYS 64u
#define TIMBRE_SOLO_LOOKUP_SIZE 128u
#define TIMBRE_GROUP_MAX_SAMPLE_GAP 0x0800u
#define TIMBRE_PS2_GROUP_MAX_SAMPLE_GAP 0x0200u
#define TIMBRE_PERCUSSION_MAX_SAMPLE_GAP 0x1000u
#define TIMBRE_PERCUSSION_MAX_PITCH_SPAN 0x0180u
#define TIMBRE_PERCUSSION_MIN_SAMPLES 3u
#define TIMBRE_PRESCAN_FALLBACK_SECONDS 180u
#define TIMBRE_PRESCAN_MAX_SECONDS 300u
#define TIMBRE_PRESCAN_BATCH_FRAMES PLAYER_SAMPLE_RATE
#define PREVIEW_MAX_CACHED_SAMPLES 1024u
#define PREVIEW_MAX_VOICES 12u
#define PREVIEW_MAX_ADPCM_BYTES (512u * 1024u)
#define PREVIEW_REVERB_FRAMES 4096u
#define PREVIEW_AUDIO_FRAMES 256u
#define PREVIEW_AUDIO_BUFFERS 8u
#define PREVIEW_HANDOFF_FRAMES 128u
#define PREVIEW_LOOP_SMOOTH_FRAMES 64u
#define PREVIEW_DEFAULT_VOLUME 0x2000u
#define PREVIEW_FIRST_MIDI_NOTE 24
#define PREVIEW_LAST_MIDI_NOTE 84
#define PSF1_TRACK_KEY_FIRST_NOTE 0
#define PSF1_TRACK_KEY_LAST_NOTE 107
#define PSF1_TRACK_KEYBOARD_HEIGHT 20
#define PSF1_TRACK_KEYBOARD_GAP 4
#define PSF1_TUNING_CACHE_SIZE 512u
#define PSF1_TUNING_MAX_ADPCM_BYTES 16384u
#define PSF1_TUNING_MAX_LAG 1536u
#define PSF1_BACKWARD_REDIRECT_MAX_ADPCM_BYTES 512u
#define PSF1_TUNING_PENDING 0u
#define PSF1_TUNING_READY 1u
#define PSF1_TUNING_PROCESSING 2u
#define PSF1_TUNING_REFRESH_PENDING 3u
#define PSF1_TUNING_REFRESH_PROCESSING 4u

#define IDM_OPEN 1001
#define IDM_STOP 1002
#define IDM_EXIT 1003
#define IDM_ACTIVE_ONLY 1004
#define IDM_REVERB_ENABLED 1005
#define IDM_MAIN_ENABLED 1006
#define IDM_ENV_COLOR_GREEN 1010
#define IDM_ENV_COLOR_BLUE 1011
#define IDM_ENV_COLOR_CYAN 1012
#define IDM_ENV_COLOR_AMBER 1013
#define IDM_ENV_COLOR_RED 1014
#define IDM_ENV_COLOR_MAGENTA 1015
#define IDM_ENV_COLOR_VIOLET 1016
#define IDM_ENV_COLOR_WHITE 1017
#define IDM_ENV_COLOR_CUSTOM 1018
#define IDM_LR_COLOR_GREEN 1020
#define IDM_LR_COLOR_BLUE 1021
#define IDM_LR_COLOR_CYAN 1022
#define IDM_LR_COLOR_AMBER 1023
#define IDM_LR_COLOR_RED 1024
#define IDM_LR_COLOR_MAGENTA 1025
#define IDM_LR_COLOR_VIOLET 1026
#define IDM_LR_COLOR_WHITE 1027
#define IDM_LR_COLOR_CUSTOM 1028
#define IDM_ABOUT 1030
#define IDM_VOLUME_0 1040
#define IDM_VOLUME_200 1060
#define IDM_PERF_LOW 1070
#define IDM_PERF_MIDDLE 1071
#define IDM_PERF_HIGH 1072
#define IDM_DEBUG_EDIT_CONTROLS 1080
#define IDM_FONT_SELECT 1081
#define IDM_PLAYBACK_ONLY 1082
#define IDM_FRAME_ADVANCE 1083
#define IDM_THEME_SYSTEM 1084
#define IDM_THEME_LIGHT 1085
#define IDM_THEME_DARK 1086
#define IDM_TIMBRE_SCAN 1087
#define IDM_OPEN_FOLDER 1090
#define IDM_PLAYLIST_SHOW 1091
#define IDM_PLAYLIST_LOAD 1092
#define IDM_PLAYLIST_SAVE 1093
#define IDM_PLAYLIST_TOPMOST 1094
#define IDM_TAG_FADE_ENABLED 1095
#define IDM_PLAYLIST_ORDER_NEXT 1096
#define IDM_PLAYLIST_ORDER_RANDOM 1097
#define IDM_PLAYLIST_ORDER_SINGLE 1098
#define IDM_SPEED_PRESET_FIRST 1100
#define IDM_SPEED_PRESET_LAST 1108
#define IDM_KEYBOARD_VALUES 1110
#define IDM_KEYBOARD_VALUES_AND_KEYS 1111
#define IDM_KEYBOARD_KEYS_ONLY 1112
#define SPEED_PRESET_COUNT 9u
#define MENU_SEPARATOR_MAGIC ((ULONG_PTR)0x5053465355505345ull)
#define MENU_BAR_ITEM_MAGIC ((ULONG_PTR)0x5053465355424152ull)
#define IDC_PLAYLIST_LIST 3101
#define IDC_PLAYLIST_PREV 3104
#define IDC_PLAYLIST_NEXT 3105
#define IDC_PLAYLIST_PLAY 3106
#define IDC_PLAYLIST_SAVE 3107
#define IDC_PLAYLIST_LOAD 3108
#define IDC_PLAYLIST_DELETE 3109
#define IDC_PLAYLIST_STOP 3110
#define IDC_PLAYLIST_REPLACE 3111
#define IDC_PLAYLIST_APPEND 3112
#define PLAYLIST_MAX_ITEMS 1024
#define IDC_TIMBRE_LIST 3201
#define IDC_TIMBRE_CLEAR 3202
#define IDC_TIMBRE_TOPMOST 3203
#define IDC_TIMBRE_HIDE 3204
#define TIMBRE_LIST_MAX_ITEMS 256
#define TIMBRE_LIST_GROUP_ROW 0xffffu
#define TIMBRE_LIST_MAX_ROWS (TIMBRE_LIST_MAX_ITEMS * (TIMBRE_SOLO_MAX_KEYS + 1u))
#define IDC_PREVIEW_AR 3301
#define IDC_PREVIEW_DR 3302
#define IDC_PREVIEW_SL 3303
#define IDC_PREVIEW_SR 3304
#define IDC_PREVIEW_RR 3305
#define IDC_PREVIEW_PITCH 3306
#define IDC_PREVIEW_VOL_L 3307
#define IDC_PREVIEW_VOL_R 3308
#define IDC_PREVIEW_ATTACK_EXP 3309
#define IDC_PREVIEW_SUSTAIN_EXP 3310
#define IDC_PREVIEW_SUSTAIN_DEC 3311
#define IDC_PREVIEW_RELEASE_EXP 3312
#define IDC_PREVIEW_NOISE 3313
#define IDC_PREVIEW_PMOD 3314
#define IDC_PREVIEW_REVERB 3315
#define IDC_PREVIEW_VOL_SLIDER 3316
#define IDC_PREVIEW_STATUS 3317
#define IDC_OPEN 2001
#define IDC_PLAY 2002
#define IDC_PAUSE 2003
#define IDC_STOP 2004
#define IDC_SPEED 2005
#define IDC_MAIN_CHECK 2006
#define IDC_REVERB_CHECK 2007
#define IDC_SPEED_LABEL 2008
#define IDC_VOLUME_LABEL 2009
#define IDC_TIME_LABEL 2010
#define IDI_APP_ICON 101

#define COL_TEXT_X 8
#define COL_ENV_TEXT_X 296
#define COL_ENV_BAR_X 330
#define COL_VOL_BAR_X 510
#define COL_FLAGS_X 622
#define CONTROLS_HEIGHT 30
#define TIME_LABEL_MIN_WIDTH 112
#define PLAYER_DEFAULT_WIDTH 1688
#define PLAYER_DEFAULT_HEIGHT 563
#define PLAYER_PSF1_WIDTH 890
#define PLAYER_PSF1_HEIGHT 1080
#define PLAYER_PSF1_KEYBOARD_ONLY_HEIGHT 679
#define PLAYER_FULLSCREEN_CHROME_PERCENT 85
#define PLAYER_PSF2_FULLSCREEN_CONTENT_PERCENT 98
#define PLAYER_PSF2_FULLSCREEN_VERTICAL_PERCENT 108
#define PLAYER_WINDOW_SNAP_DISTANCE 12
#define PLAYER_MIN_WIDTH PLAYER_PSF1_WIDTH
#define PLAYER_MIN_HEIGHT PLAYER_DEFAULT_HEIGHT
#define CORE_PANEL_WIDTH 832
#define CORE_PANEL_GAP 8
#define CORE0_X 8
#define CORE1_X (CORE0_X + CORE_PANEL_WIDTH + CORE_PANEL_GAP)
#define SPU2_CORE_STRIDE 0x0400u
#define PS1_SPU_BASE 0x1f801c00u
#define PS1_SPU_END 0x1f801dffu
#define PS1_REG_KON 0x0188u
#define PS1_REG_KON_HI 0x018au
#define PS1_REG_KOFF 0x018cu
#define PS1_REG_KOFF_HI 0x018eu
#define SPU2_REG_KON 0x01a0u
#define SPU2_REG_KON_HI 0x01a2u
#define SPU2_REG_KOFF 0x01a4u
#define SPU2_REG_KOFF_HI 0x01a6u
#define CUSTOM_COLOR_INDEX 8
#define CUSTOM_COLOR_COUNT 16
#define IDC_COLOR_CODE 3001
#define IDC_COLOR_PICK 3002
#define IDC_COLOR_OK 3003
#define IDC_COLOR_CANCEL 3004
#define IDC_VALUE_EDIT 3010
#define IDC_VALUE_OK 3011
#define IDC_VALUE_CANCEL 3012
#define IDC_VALUE_SLIDER 3013
#define IDC_REVERB_EDIT_L 3014
#define IDC_REVERB_EDIT_R 3015
#define IDC_REVERB_SLIDER_L 3016
#define IDC_REVERB_SLIDER_R 3017
#define IDC_REVERB_LINK 3018
#define IDC_PHASE_INVERT_L 3019
#define IDC_PHASE_INVERT_R 3025
#define IDC_FONT_LIST 3020
#define IDC_FONT_OK 3021
#define IDC_FONT_CANCEL 3022
#define IDC_FONT_APPLY 3023
#define IDC_FONT_SIZE 3024
#define NOISE_CLOCK_MAX 0x3fu
#define ADSR_FIELD_ATTACK 0u
#define ADSR_FIELD_DECAY 1u
#define ADSR_FIELD_SUSTAIN_LEVEL 2u
#define ADSR_FIELD_SUSTAIN_RATE 3u
#define ADSR_FIELD_RELEASE 4u
#define VOICE_FLAG_COLUMN_NONE 0
#define VOICE_FLAG_COLUMN_NOISE 1
#define VOICE_FLAG_COLUMN_PMOD 2
#define VOICE_FLAG_COLUMN_REVERB 3
#define VOICE_FLAG_COLUMN_NOISE_CLOCK 4
#define VOICE_FLAG_COLUMN_ADSR0 10
#define VOICE_FLAG_COLUMN_REVERB_L 30
#define VOICE_FLAG_COLUMN_REVERB_R 31
#define VOICE_FLAG_COLUMN_REVERB_BOTH 33
#define VOICE_TEXT_COLUMN_NONE 0
#define VOICE_TEXT_COLUMN_ADSR_STATE 1
#define VOICE_TEXT_COLUMN_PITCH 2
#define VOICE_TEXT_COLUMN_VOL_L 3
#define VOICE_TEXT_COLUMN_VOL_R 4
#define THEME_SYSTEM 0
#define THEME_LIGHT 1
#define THEME_DARK 2
#define PSF1_DISPLAY_VALUES 0
#define PSF1_DISPLAY_VALUES_KEYBOARD 1
#define PSF1_DISPLAY_KEYBOARD_ONLY 2
#define PSF1_DISPLAY_MODE_COUNT 3

typedef struct {
    ULONG_PTR magic;
    const char *label;
    int menu_bar;
} OwnerMenuItemData;

static const int g_speed_menu_values[SPEED_PRESET_COUNT] = {
    10, 25, 50, 75, 100, 125, 150, 175, 200
};

static const char *const g_speed_menu_labels[SPEED_PRESET_COUNT] = {
    "10%", "25%", "50%", "75%", "100%", "125%", "150%", "175%", "200%"
};

typedef struct PreviewSample {
    uint32_t key;
    uint32_t ssa;
    uint32_t lsa;
    uint32_t flags;
    uint8_t core;
    uint16_t reference_pitch;
    uint16_t c4_pitch;
    uint16_t vol_l;
    uint16_t vol_r;
    uint16_t pitch_min;
    uint16_t pitch_max;
    uint8_t noise_clock;
    uint8_t loop_enabled;
    uint8_t tuning_attempted;
    int16_t *pcm;
    uint32_t pcm_frames;
    uint32_t loop_frame;
} PreviewSample;

typedef struct PreviewVoice {
    int active;
    int releasing;
    int midi_note;
    unsigned sample_index;
    uint64_t phase;
    uint64_t phase_step;
    int64_t envelope;
    uint8_t adsr_state;
    uint32_t noise_lfsr;
    uint32_t handoff_frames;
    int32_t loop_correction;
    uint32_t loop_smooth_frames;
    uint32_t loop_smooth_total;
    int32_t last_raw_sample;
    int have_last_raw_sample;
} PreviewVoice;

typedef struct Psf1TuningEntry {
    uint32_t key;
    uint32_t ssa;
    uint32_t lsa;
    uint16_t c4_pitch;
    uint8_t status;
} Psf1TuningEntry;

typedef struct AudioDisplaySnapshot {
    Spu2LogLiveState live;
    Psf1AkaoPlaybackState akao;
    uint8_t key_on_events[2][24];
    uint8_t release_events[2][24];
    uint64_t voice_display_hold_until[2][24];
    Spu2LogVoiceSnapshot voice_display_hold[2][24];
    uint32_t psf1_voice_timbre_key[24];
    uint64_t timeline_sample_pos;
    uint64_t sequence;
    int stopped_display;
    int startup_track_dimmed;
    int valid;
} AudioDisplaySnapshot;

typedef struct PlayerState {
    Spu2LogLiveState live;
    Spu2LogLiveState frame_live;
    Psf1AkaoPlaybackState akao;
    uint32_t sequence;
    const Psf2CoreProvider *provider;
    Psf2CoreBridge *core;
    HWAVEOUT wave;
    int16_t *pcm;
    int16_t *audio_buffers[PLAYER_AUDIO_BUFFERS];
    WAVEHDR audio_headers[PLAYER_AUDIO_BUFFERS];
    int audio_in_use[PLAYER_AUDIO_BUFFERS];
    AudioDisplaySnapshot audio_display_snapshots[PLAYER_AUDIO_BUFFERS];
    AudioDisplaySnapshot audible_display_snapshot;
    int audible_display_valid;
    unsigned audio_queued_buffers;
    unsigned audio_queue_low_water;
    int audio_started;
    uint64_t audio_chunks_queued;
    uint64_t audio_underruns;
    int16_t last_output_l;
    int16_t last_output_r;
    int has_last_output;
    volatile LONG graceful_stop_requested;
    HANDLE thread;
    HANDLE stop_event;
    CRITICAL_SECTION lock;
    int lock_ready;
    HWND hwnd;
    HMENU keyboard_parent_menu;
    HMENU keyboard_submenu;
    UINT keyboard_parent_position;
    HDC paint_dc;
    HBITMAP paint_bitmap;
    HBITMAP paint_old_bitmap;
    HDC psf1_keyboard_dc;
    HBITMAP psf1_keyboard_bitmap;
    HBITMAP psf1_keyboard_old_bitmap;
    HDC psf1_keyboard_base_dc;
    HBITMAP psf1_keyboard_base_bitmap;
    HBITMAP psf1_keyboard_base_old_bitmap;
    int psf1_keyboard_dark;
    int psf1_keyboard_width;
    HFONT meter_font;
    HFONT meter_bold_font;
    int meter_line_height;
    int paint_width;
    int paint_height;
    uint64_t display_render_sequence;
    int display_render_source;
    int gauge_only_paint_pending;
    int paint_frame_initialized;
    RECT voice_number_hit_rects[2][24];
    uint8_t voice_number_hit_valid[2][24];
    HWND open_button;
    HWND play_button;
    HWND pause_button;
    HWND stop_button;
    HWND speed_label;
    HWND speed_slider;
    HWND volume_label;
    HWND main_check;
    HWND reverb_check;
    HWND time_label;
    HWND voice_scrollbar;
    int playing;
    int paused;
    int seek_discarding;
    int startup_silence_trim;
    int ps2_startup_origin_valid;
    uint64_t ps2_startup_origin_sample;
    int timbre_prescanning;
    int timbre_list_locked;
    int seek_request;
    uint64_t seek_target_sample;
    int speed_percent;
    int tab_speed_active;
    int scroll_y;
    int content_height;
    int window_move_drag_active;
    int window_move_cursor_offset_x;
    int window_move_cursor_offset_y;
    int window_move_width;
    int window_move_height;
    int hide_inactive;
    int main_enabled;
    int reverb_enabled;
    int text_log_enabled;
    int debug_edit_controls;
    int playback_only;
    int psf1_display_mode;
    int timbre_scan_enabled;
    int frame_advance;
    int frame_live_valid;
    uint64_t frame_capture_until;
    uint32_t frame_step_request;
    int frame_audio_transition;
    int theme_mode;
    int playlist_topmost;
    int timbre_topmost;
    int volume_percent;
    int performance_profile;
    int tag_fade_enabled;
    int playlist_random_mode;
    int playlist_single_track_mode;
    int stopped_display;
    int ps2_startup_track_dimmed;
    uint8_t psf_version;
    int env_color_index;
    int lr_color_index;
    COLORREF env_custom_color;
    COLORREF lr_custom_color;
    int theme_env_color_index[2];
    int theme_lr_color_index[2];
    COLORREF theme_env_custom_color[2];
    COLORREF theme_lr_custom_color[2];
    COLORREF custom_colors[CUSTOM_COLOR_COUNT];
    uint64_t total_samples;
    uint64_t fade_samples;
    uint64_t timeline_source_anchor;
    uint64_t timeline_scaled_anchor;
    int timeline_speed_percent;
    int timeline_valid;
    uint64_t time_label_tenths;
    uint64_t time_label_total_samples;
    int time_label_cache_valid;
    uint64_t last_gauge_update_ms;
    int gauge_animation_active;
    uint8_t key_on_event_buffers[2][24];
    uint8_t release_event_buffers[2][24];
    uint8_t release_event_latched[2][24];
    uint64_t voice_display_hold_until[2][24];
    Spu2LogVoiceSnapshot voice_display_hold[2][24];
    uint32_t psf1_voice_timbre_key[24];
    Psf1TuningEntry psf1_tuning_cache[PSF1_TUNING_CACHE_SIZE];
    unsigned psf1_tuning_cache_count;
    uint32_t voice_mute_mask[2];
    int timbre_solo_enabled;
    uint32_t timbre_solo_keys[TIMBRE_SOLO_MAX_KEYS];
    uint32_t timbre_solo_ssa[TIMBRE_SOLO_MAX_KEYS];
    uint32_t timbre_solo_lsa[TIMBRE_SOLO_MAX_KEYS];
    uint32_t timbre_solo_flags[TIMBRE_SOLO_MAX_KEYS];
    unsigned timbre_solo_key_count;
    uint16_t timbre_solo_adsr1;
    uint16_t timbre_solo_adsr2;
    uint32_t timbre_solo_flag_class;
    uint32_t timbre_list_keys[TIMBRE_LIST_MAX_ITEMS][TIMBRE_SOLO_MAX_KEYS];
    uint32_t timbre_list_ssa_keys[TIMBRE_LIST_MAX_ITEMS][TIMBRE_SOLO_MAX_KEYS];
    uint32_t timbre_list_lsa_keys[TIMBRE_LIST_MAX_ITEMS][TIMBRE_SOLO_MAX_KEYS];
    uint32_t timbre_list_end_keys[TIMBRE_LIST_MAX_ITEMS][TIMBRE_SOLO_MAX_KEYS];
    uint16_t timbre_list_pitch_min[TIMBRE_LIST_MAX_ITEMS][TIMBRE_SOLO_MAX_KEYS];
    uint16_t timbre_list_pitch_max[TIMBRE_LIST_MAX_ITEMS][TIMBRE_SOLO_MAX_KEYS];
    unsigned timbre_list_key_count[TIMBRE_LIST_MAX_ITEMS];
    uint16_t timbre_list_adsr1[TIMBRE_LIST_MAX_ITEMS];
    uint16_t timbre_list_adsr2[TIMBRE_LIST_MAX_ITEMS];
    uint32_t timbre_list_flags[TIMBRE_LIST_MAX_ITEMS];
    uint32_t timbre_list_ssa[TIMBRE_LIST_MAX_ITEMS];
    uint32_t timbre_list_lsa[TIMBRE_LIST_MAX_ITEMS];
    uint8_t timbre_list_is_percussion[TIMBRE_LIST_MAX_ITEMS];
    uint8_t timbre_list_expanded[TIMBRE_LIST_MAX_ITEMS];
    unsigned timbre_list_count;
    uint32_t voice_reverb_force_on_mask[2];
    uint32_t voice_reverb_force_off_mask[2];
    uint32_t voice_noise_force_on_mask[2];
    uint32_t voice_noise_force_off_mask[2];
    uint32_t voice_pmod_force_on_mask[2];
    uint32_t voice_pmod_force_off_mask[2];
    uint32_t voice_adsr_force_mask[2];
    uint32_t voice_pitch_lock_mask[2];
    uint16_t voice_pitch_lock_value[2][24];
    uint32_t voice_volume_lock_mask[2][2];
    uint16_t voice_volume_lock_value[2][2][24];
    uint16_t default_adsr1[2][24];
    uint16_t default_adsr2[2][24];
    uint16_t default_pitch[2][24];
    uint16_t default_vol_l[2][24];
    uint16_t default_vol_r[2][24];
    uint8_t default_voice_valid[2][24];
    uint8_t default_noise_clock[2];
    uint16_t default_reverb_l[2];
    uint16_t default_reverb_r[2];
    uint8_t default_core_valid[2];
    uint8_t manual_reverb_value_valid[2][2];
    uint16_t debug_saved_adsr1[2][24];
    uint16_t debug_saved_adsr2[2][24];
    uint16_t debug_saved_pitch[2][24];
    uint16_t debug_saved_vol_l[2][24];
    uint16_t debug_saved_vol_r[2][24];
    uint8_t debug_saved_voice_valid[2][24];
    uint32_t debug_adsr_touched_mask[2];
    uint8_t debug_saved_noise_clock[2];
    uint16_t debug_saved_reverb_l[2];
    uint16_t debug_saved_reverb_r[2];
    uint8_t debug_saved_core_valid[2];
    uint32_t gauge_env[2][24];
    uint32_t gauge_vol_l[2][24];
    uint32_t gauge_vol_r[2][24];
    uint8_t gauge_valid[2][24];
    HWND playlist_hwnd;
    HWND playlist_listbox;
    WNDPROC playlist_listbox_proc;
    int playlist_play_in_progress;
    int playlist_pending_play_index;
    int playlist_append_mode;
    int playback_from_playlist;
    uint32_t playback_generation;
    int direct_start_retry_armed;
    int direct_start_retry_remaining;
    int direct_start_retry_from_playlist;
    int direct_start_retry_playlist_index;
    DWORD direct_start_retry_deadline;
    char direct_start_retry_path[MAX_PATH];
    uint32_t playlist_random_state;
    HWND timbre_hwnd;
    HWND timbre_listbox;
    WNDPROC timbre_listbox_proc;
    int timbre_drag_active;
    int timbre_drag_anchor_row;
    int timbre_drag_last_caret;
    int timbre_drag_row_count;
    int timbre_drag_select_value;
    uint8_t timbre_drag_base_selection[TIMBRE_LIST_MAX_ROWS];
    HWND preview_hwnd;
    HWAVEOUT preview_wave;
    HANDLE preview_audio_thread;
    HANDLE preview_audio_stop_event;
    int16_t *preview_audio_buffers[PREVIEW_AUDIO_BUFFERS];
    WAVEHDR preview_audio_headers[PREVIEW_AUDIO_BUFFERS];
    int preview_audio_in_use[PREVIEW_AUDIO_BUFFERS];
    PreviewSample preview_samples[PREVIEW_MAX_CACHED_SAMPLES];
    unsigned preview_sample_count;
    unsigned preview_selected_samples[TIMBRE_SOLO_MAX_KEYS];
    unsigned preview_selected_sample_count;
    PreviewVoice preview_voices[PREVIEW_MAX_VOICES];
    uint16_t preview_adsr1;
    uint16_t preview_adsr2;
    uint16_t preview_pitch;
    uint16_t preview_reference_pitch;
    uint16_t preview_vol_l;
    uint16_t preview_vol_r;
    uint16_t preview_reverb_value_l;
    uint16_t preview_reverb_value_r;
    uint32_t preview_flags;
    uint8_t preview_noise_clock;
    int preview_active_note;
    int preview_octave_shift;
    uint8_t preview_key_down[128];
    int preview_controls_updating;
    int32_t preview_reverb_l[PREVIEW_REVERB_FRAMES];
    int32_t preview_reverb_r[PREVIEW_REVERB_FRAMES];
    uint32_t preview_reverb_pos;
    HFONT ui_font;
    HFONT maximized_ui_font;
    int maximized_ui_font_scale;
    int time_label_base_width;
    int time_label_base_x;
    int time_label_base_y;
    int time_label_base_height;
    char ui_font_path[MAX_PATH];
    char ui_font_face[LF_FACESIZE];
    int ui_font_size;
    char playlist_paths[PLAYLIST_MAX_ITEMS][MAX_PATH];
    char playlist_last_folder[MAX_PATH];
    char playlist_last_file[MAX_PATH];
    unsigned playlist_count;
    int playlist_current;
    int pending_click_active;
    int pending_click_button;
    int pending_click_repeating;
    unsigned pending_click_core;
    unsigned pending_click_voice;
    unsigned pending_click_column;
    char input_path[MAX_PATH];
    char status[512];
    char time_label_text[80];
    HBRUSH menu_brush;
    HBRUSH playlist_menu_brush;
} PlayerState;

static void update_scrollbar(HWND hwnd, PlayerState *state);
static void update_settings_menu_check(HWND hwnd, const PlayerState *state);
static void update_time_label(PlayerState *state);
static int fast_forward_core(PlayerState *state, uint64_t frames_to_skip, uint64_t base_sample);
static int should_hold_voice_for_display(const Spu2LogVoiceSnapshot *voice);
static void psf1_queue_tuning_locked(PlayerState *state, const Spu2LogVoiceSnapshot *voice);
static void psf1_process_tuning_request(PlayerState *state);
static int normalize_theme_mode(int mode);
static int is_dark_theme_active(const PlayerState *state);
static void apply_app_menu_dark_mode(int dark);
static void sync_active_gauge_colors_from_theme(PlayerState *state);
static void store_active_gauge_colors_to_theme(PlayerState *state);
static COLORREF player_background_color(const PlayerState *state);
static COLORREF player_text_color(const PlayerState *state);
static COLORREF player_inactive_text_color(const PlayerState *state);
static COLORREF player_muted_text_color(const PlayerState *state);
static int reopen_core_for_seek(PlayerState *state, uint64_t *out_sample_pos);
static void start_playback(HWND hwnd, PlayerState *state, const char *path);
static void start_direct_playback(HWND hwnd, PlayerState *state, const char *path);
static void cancel_direct_start_retry(HWND hwnd, PlayerState *state);
static void process_direct_start_retry(HWND hwnd, PlayerState *state);
static void show_playlist_window(HWND hwnd, PlayerState *state);
static void playlist_play_index(HWND hwnd, PlayerState *state, int index);
static void playlist_process_pending_play(HWND hwnd, PlayerState *state);
static void show_timbre_window(HWND hwnd, PlayerState *state);
static void timbre_refresh_listbox(PlayerState *state);
static void show_preview_keyboard(PlayerState *state, unsigned group, int sample);
static void preview_stop_all_locked(PlayerState *state);
static void preview_free_samples_locked(PlayerState *state);
static void preview_start_standalone_audio(PlayerState *state);
static void preview_stop_standalone_audio(PlayerState *state);
static int read_font_family_from_file(const char *font_path, char *out_face, size_t out_size);
static void rebuild_ui_font(PlayerState *state);
static void apply_ui_font_to_window(HWND hwnd, HFONT font);
static HFONT create_ui_font_from_face(const char *face, int point_size);
static void layout_player_controls(HWND hwnd, PlayerState *state);
static void restore_debug_edits_to_saved_values(HWND hwnd, PlayerState *state);
static int get_player_display_line_height(HWND hwnd);
static int maximized_voice_rows_target_bottom(
    HWND hwnd,
    const PlayerState *state);

static int g_text_log_enabled = 1;
static int g_window_fixed_width = PLAYER_DEFAULT_WIDTH;
static int g_window_fixed_height = PLAYER_DEFAULT_HEIGHT;
static int g_main_window_was_maximized = 0;
static int g_main_restore_position_valid = 0;
static int g_main_restore_position_active = 0;
static int g_main_restore_x = 0;
static int g_main_restore_y = 0;
static int g_window_nonclient_width = 0;
static int g_window_nonclient_height = 0;
static int g_window_base_client_width = PLAYER_DEFAULT_WIDTH;
static int g_window_base_client_height = PLAYER_DEFAULT_HEIGHT;
static HFONT g_ui_font = NULL;

static void show_about_dialog(HWND hwnd)
{
    PlayerState *state = hwnd != NULL ? (PlayerState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA) : NULL;
    apply_app_menu_dark_mode(state != NULL && is_dark_theme_active(state));
    MessageBoxA(hwnd,
        "PSF SPU Player " PSF2_PLAYER_VERSION_DISPLAY "\n"
        "\n"
        "Copyright (c) 2026 brr890",
        "About PSF SPU Player",
        MB_OK);
}

static void get_player_log_path(char *out_path, size_t out_size)
{
    char module_path[MAX_PATH];
    char *last_slash;

    if (out_size == 0) {
        return;
    }
    out_path[0] = '\0';

    if (GetModuleFileNameA(NULL, module_path, sizeof(module_path)) == 0) {
        snprintf(out_path, out_size, "psf_spu_player.log");
        return;
    }

    last_slash = strrchr(module_path, '\\');
    if (last_slash != NULL) {
        last_slash[1] = '\0';
        snprintf(out_path, out_size, "%spsf_spu_player.log", module_path);
    } else {
        snprintf(out_path, out_size, "psf_spu_player.log");
    }
}

static void get_player_ini_path(char *out_path, size_t out_size)
{
    char module_path[MAX_PATH];
    char *last_slash;

    if (out_size == 0) {
        return;
    }
    out_path[0] = '\0';

    if (GetModuleFileNameA(NULL, module_path, sizeof(module_path)) == 0) {
        snprintf(out_path, out_size, "psf_spu_player.ini");
        return;
    }

    last_slash = strrchr(module_path, '\\');
    if (last_slash != NULL) {
        last_slash[1] = '\0';
        snprintf(out_path, out_size, "%spsf_spu_player.ini", module_path);
    } else {
        snprintf(out_path, out_size, "psf_spu_player.ini");
    }
}

static void write_profile_int(const char *path, const char *section, const char *key, int value)
{
    char text[32];

    snprintf(text, sizeof(text), "%d", value);
    WritePrivateProfileStringA(section, key, text, path);
}

static int load_text_log_enabled(void)
{
    char path[MAX_PATH];

    get_player_ini_path(path, sizeof(path));
    return GetPrivateProfileIntA("Debug", "LogEnabled", 0, path) ? 1 : 0;
}

static void save_text_log_enabled(int enabled)
{
    char path[MAX_PATH];

    get_player_ini_path(path, sizeof(path));
    write_profile_int(path, "Debug", "LogEnabled", enabled ? 1 : 0);
}

static void load_ui_font_settings(PlayerState *state)
{
    char path[MAX_PATH];

    if (state == NULL) {
        return;
    }
    get_player_ini_path(path, sizeof(path));
    state->ui_font_path[0] = '\0';
    GetPrivateProfileStringA("Font", "Face", "", state->ui_font_face, sizeof(state->ui_font_face), path);
    state->ui_font_size = GetPrivateProfileIntA("Font", "Size", state->ui_font_size, path);
    if (state->ui_font_size < 8) {
        state->ui_font_size = 9;
    } else if (state->ui_font_size > 16) {
        state->ui_font_size = 16;
    }
}

static void save_ui_font_settings(const PlayerState *state)
{
    char path[MAX_PATH];

    if (state == NULL) {
        return;
    }
    get_player_ini_path(path, sizeof(path));
    WritePrivateProfileStringA("Font", "Path", "", path);
    WritePrivateProfileStringA("Font", "Face", state->ui_font_face, path);
    write_profile_int(path, "Font", "Size", state->ui_font_size);
}

static uint8_t normalize_saved_psf_version(int value)
{
    return value == 0x01 ? 0x01u : 0x02u;
}

static uint8_t load_last_psf_version(void)
{
    char path[MAX_PATH];

    get_player_ini_path(path, sizeof(path));
    return normalize_saved_psf_version(GetPrivateProfileIntA("Window", "LastPsfVersion", 0x02, path));
}

static void save_last_psf_version(uint8_t psf_version)
{
    char path[MAX_PATH];

    get_player_ini_path(path, sizeof(path));
    write_profile_int(path, "Window", "LastPsfVersion", psf_version == 0x01u ? 0x01 : 0x02);
}

static int normalize_psf1_display_mode(int mode)
{
    return mode >= PSF1_DISPLAY_VALUES && mode < PSF1_DISPLAY_MODE_COUNT ?
        mode : PSF1_DISPLAY_VALUES_KEYBOARD;
}

static int load_psf1_display_mode(void)
{
    static const int missing = -1;
    char path[MAX_PATH];
    int mode;

    get_player_ini_path(path, sizeof(path));
    mode = (int)GetPrivateProfileIntA(
        "Window", "Psf1DisplayMode", missing, path);
    if (mode == missing) {
        return GetPrivateProfileIntA(
            "Window", "Psf1KeyboardVisible", 1, path) ?
            PSF1_DISPLAY_VALUES_KEYBOARD : PSF1_DISPLAY_VALUES;
    }
    return normalize_psf1_display_mode(mode);
}

static void save_psf1_display_mode(int mode)
{
    char path[MAX_PATH];

    mode = normalize_psf1_display_mode(mode);
    get_player_ini_path(path, sizeof(path));
    write_profile_int(path, "Window", "Psf1DisplayMode", mode);
    write_profile_int(path, "Window", "Psf1KeyboardVisible",
        mode != PSF1_DISPLAY_VALUES ? 1 : 0);
}

static const char *psf1_mode_window_key(int display_mode, int field)
{
    static const char *keys[PSF1_DISPLAY_MODE_COUNT][4] = {
        {"Psf1NormalX", "Psf1NormalY", "Psf1NormalWidth", "Psf1NormalHeight"},
        {"Psf1KeyboardX", "Psf1KeyboardY", "Psf1KeyboardWidth", "Psf1KeyboardHeight"},
        {"Psf1KeyboardOnlyX", "Psf1KeyboardOnlyY", "Psf1KeyboardOnlyWidth", "Psf1KeyboardOnlyHeight"}
    };

    return keys[normalize_psf1_display_mode(display_mode)][field];
}

static int psf1_mode_default_height(int display_mode)
{
    switch (normalize_psf1_display_mode(display_mode)) {
    case PSF1_DISPLAY_VALUES_KEYBOARD:
        return PLAYER_PSF1_HEIGHT;
    case PSF1_DISPLAY_KEYBOARD_ONLY:
        return PLAYER_PSF1_KEYBOARD_ONLY_HEIGHT;
    default:
        return PLAYER_DEFAULT_HEIGHT;
    }
}

static int load_psf1_mode_window_position(int display_mode, int *x, int *y)
{
    static const int missing = 0x7fffffff;
    const char *x_key = psf1_mode_window_key(display_mode, 0);
    const char *y_key = psf1_mode_window_key(display_mode, 1);
    char path[MAX_PATH];
    int loaded_x;
    int loaded_y;

    if (x == NULL || y == NULL) {
        return 0;
    }
    get_player_ini_path(path, sizeof(path));
    loaded_x = (int)GetPrivateProfileIntA("Window", x_key, missing, path);
    loaded_y = (int)GetPrivateProfileIntA("Window", y_key, missing, path);
    if (loaded_x == missing || loaded_y == missing) {
        return 0;
    }
    *x = loaded_x;
    *y = loaded_y;
    return 1;
}

static int load_psf1_mode_window_size(
    int display_mode,
    int *width,
    int *height)
{
    static const int missing = 0x7fffffff;
    const char *width_key = psf1_mode_window_key(display_mode, 2);
    const char *height_key = psf1_mode_window_key(display_mode, 3);
    char path[MAX_PATH];
    int loaded_width;
    int loaded_height;
    int minimum_height;

    if (width == NULL || height == NULL) {
        return 0;
    }
    get_player_ini_path(path, sizeof(path));
    loaded_width = (int)GetPrivateProfileIntA(
        "Window", width_key, missing, path);
    loaded_height = (int)GetPrivateProfileIntA(
        "Window", height_key, missing, path);
    minimum_height = psf1_mode_default_height(display_mode);
    if (loaded_width == missing || loaded_height == missing ||
        loaded_width != PLAYER_PSF1_WIDTH || loaded_height < minimum_height) {
        return 0;
    }
    *width = loaded_width;
    *height = loaded_height;
    return 1;
}

static int get_window_saved_rect(HWND hwnd, RECT *rect)
{
    WINDOWPLACEMENT placement;

    if (hwnd == NULL || rect == NULL) {
        return 0;
    }
    if (!IsZoomed(hwnd) && !IsIconic(hwnd)) {
        return GetWindowRect(hwnd, rect) != 0;
    }
    ZeroMemory(&placement, sizeof(placement));
    placement.length = sizeof(placement);
    if (!GetWindowPlacement(hwnd, &placement)) {
        return 0;
    }
    *rect = placement.rcNormalPosition;
    return 1;
}

static int window_is_or_restores_maximized(HWND hwnd)
{
    WINDOWPLACEMENT placement;

    if (hwnd == NULL) {
        return 0;
    }
    if (IsZoomed(hwnd)) {
        return 1;
    }
    ZeroMemory(&placement, sizeof(placement));
    placement.length = sizeof(placement);
    return GetWindowPlacement(hwnd, &placement) &&
        (placement.flags & WPF_RESTORETOMAXIMIZED) != 0;
}

static void save_psf1_mode_window_bounds(HWND hwnd, int display_mode)
{
    const char *x_key = psf1_mode_window_key(display_mode, 0);
    const char *y_key = psf1_mode_window_key(display_mode, 1);
    const char *width_key = psf1_mode_window_key(display_mode, 2);
    const char *height_key = psf1_mode_window_key(display_mode, 3);
    char path[MAX_PATH];
    RECT rect;

    if (hwnd == NULL || IsZoomed(hwnd) || IsIconic(hwnd)) {
        return;
    }
    if (!get_window_saved_rect(hwnd, &rect)) {
        return;
    }
    get_player_ini_path(path, sizeof(path));
    write_profile_int(path, "Window", x_key, rect.left);
    write_profile_int(path, "Window", y_key, rect.top);
    write_profile_int(path, "Window", width_key, rect.right - rect.left);
    write_profile_int(path, "Window", height_key, rect.bottom - rect.top);
}

static void load_window_bounds(
    const PlayerState *state,
    int *x,
    int *y,
    int *width,
    int *height)
{
    char path[MAX_PATH];
    uint8_t last_psf_version;
    int default_width;

    if (x == NULL || y == NULL || width == NULL || height == NULL) {
        return;
    }

    last_psf_version = load_last_psf_version();
    default_width = last_psf_version == 0x01u ? PLAYER_PSF1_WIDTH : PLAYER_DEFAULT_WIDTH;
    *x = CW_USEDEFAULT;
    *y = CW_USEDEFAULT;
    *width = default_width;
    *height = last_psf_version == 0x01u && state != NULL ?
        psf1_mode_default_height(state->psf1_display_mode) : PLAYER_DEFAULT_HEIGHT;

    get_player_ini_path(path, sizeof(path));
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) {
        return;
    }

    *x = GetPrivateProfileIntA("Window", "X", *x, path);
    *y = GetPrivateProfileIntA("Window", "Y", *y, path);
    if (last_psf_version == 0x01u && state != NULL) {
        load_psf1_mode_window_position(state->psf1_display_mode, x, y);
        load_psf1_mode_window_size(
            state->psf1_display_mode, width, height);
    }
}

static int load_window_maximized(void)
{
    char path[MAX_PATH];

    get_player_ini_path(path, sizeof(path));
    return GetPrivateProfileIntA("Window", "Maximized", 0, path) != 0;
}

static void save_window_bounds(HWND hwnd, const PlayerState *state)
{
    char path[MAX_PATH];
    RECT rect;
    int maximized;

    if (hwnd == NULL) {
        return;
    }
    if (!get_window_saved_rect(hwnd, &rect)) {
        return;
    }

    get_player_ini_path(path, sizeof(path));
    maximized = window_is_or_restores_maximized(hwnd);
    write_profile_int(path, "Window", "Maximized", maximized);
    write_profile_int(path, "Window", "X", rect.left);
    write_profile_int(path, "Window", "Y", rect.top);
    write_profile_int(path, "Window", "Width", rect.right - rect.left);
    write_profile_int(path, "Window", "Height", rect.bottom - rect.top);
    if (state != NULL && state->psf_version == 0x01u) {
        save_psf1_mode_window_bounds(hwnd, state->psf1_display_mode);
        save_psf1_display_mode(state->psf1_display_mode);
    }
}

static void load_playlist_window_bounds(int *x, int *y, int *width, int *height)
{
    char path[MAX_PATH];

    if (x == NULL || y == NULL || width == NULL || height == NULL) {
        return;
    }

    *x = CW_USEDEFAULT;
    *y = CW_USEDEFAULT;
    *width = 300;
    *height = 300;

    get_player_ini_path(path, sizeof(path));
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) {
        return;
    }

    *x = GetPrivateProfileIntA("PlaylistWindow", "X", *x, path);
    *y = GetPrivateProfileIntA("PlaylistWindow", "Y", *y, path);
    *width = GetPrivateProfileIntA("PlaylistWindow", "Width", *width, path);
    *height = GetPrivateProfileIntA("PlaylistWindow", "Height", *height, path);
    if ((*width == 520 && *height == 420) || (*width == 420 && *height == 300)) {
        *width = 300;
        *height = 300;
    }
    if (*width < 260) {
        *width = 260;
    }
    if (*height < 180) {
        *height = 180;
    }
}

static void save_playlist_window_bounds(HWND hwnd)
{
    char path[MAX_PATH];
    RECT rect;

    if (hwnd == NULL || IsIconic(hwnd)) {
        return;
    }
    if (!GetWindowRect(hwnd, &rect)) {
        return;
    }

    get_player_ini_path(path, sizeof(path));
    write_profile_int(path, "PlaylistWindow", "X", rect.left);
    write_profile_int(path, "PlaylistWindow", "Y", rect.top);
    write_profile_int(path, "PlaylistWindow", "Width", rect.right - rect.left);
    write_profile_int(path, "PlaylistWindow", "Height", rect.bottom - rect.top);
}

static void load_timbre_window_bounds(int *x, int *y, int *width, int *height)
{
    char path[MAX_PATH];

    if (x == NULL || y == NULL || width == NULL || height == NULL) {
        return;
    }

    *x = CW_USEDEFAULT;
    *y = CW_USEDEFAULT;
    *width = 560;
    *height = 380;

    get_player_ini_path(path, sizeof(path));
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) {
        return;
    }

    *x = GetPrivateProfileIntA("TimbreWindow", "X", *x, path);
    *y = GetPrivateProfileIntA("TimbreWindow", "Y", *y, path);
    *width = GetPrivateProfileIntA("TimbreWindow", "Width", *width, path);
    *height = GetPrivateProfileIntA("TimbreWindow", "Height", *height, path);
    if (*width < 320) {
        *width = 320;
    }
    if (*height < 180) {
        *height = 180;
    }
}

static void save_timbre_window_bounds(HWND hwnd)
{
    char path[MAX_PATH];
    RECT rect;

    if (hwnd == NULL || IsIconic(hwnd)) {
        return;
    }
    if (!GetWindowRect(hwnd, &rect)) {
        return;
    }

    get_player_ini_path(path, sizeof(path));
    write_profile_int(path, "TimbreWindow", "X", rect.left);
    write_profile_int(path, "TimbreWindow", "Y", rect.top);
    write_profile_int(path, "TimbreWindow", "Width", rect.right - rect.left);
    write_profile_int(path, "TimbreWindow", "Height", rect.bottom - rect.top);
}

static void load_preview_window_position(int *x, int *y)
{
    char path[MAX_PATH];

    if (x == NULL || y == NULL) {
        return;
    }
    *x = CW_USEDEFAULT;
    *y = CW_USEDEFAULT;
    get_player_ini_path(path, sizeof(path));
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) {
        return;
    }
    *x = GetPrivateProfileIntA("PreviewWindow", "X", *x, path);
    *y = GetPrivateProfileIntA("PreviewWindow", "Y", *y, path);
}

static void save_preview_window_position(HWND hwnd)
{
    char path[MAX_PATH];
    RECT rect;

    if (hwnd == NULL || IsIconic(hwnd) || !GetWindowRect(hwnd, &rect)) {
        return;
    }
    get_player_ini_path(path, sizeof(path));
    write_profile_int(path, "PreviewWindow", "X", rect.left);
    write_profile_int(path, "PreviewWindow", "Y", rect.top);
}

static int load_playlist_topmost(void)
{
    char path[MAX_PATH];

    get_player_ini_path(path, sizeof(path));
    return GetPrivateProfileIntA("PlaylistWindow", "Topmost", 0, path) ? 1 : 0;
}

static void save_playlist_topmost(int topmost)
{
    char path[MAX_PATH];

    get_player_ini_path(path, sizeof(path));
    write_profile_int(path, "PlaylistWindow", "Topmost", topmost ? 1 : 0);
}

static int load_playlist_append_mode(void)
{
    char path[MAX_PATH];

    get_player_ini_path(path, sizeof(path));
    return GetPrivateProfileIntA("Playlist", "AppendMode", 1, path) ? 1 : 0;
}

static void save_playlist_append_mode(int append_mode)
{
    char path[MAX_PATH];

    get_player_ini_path(path, sizeof(path));
    write_profile_int(path, "Playlist", "AppendMode", append_mode ? 1 : 0);
}

static void path_parent_folder(const char *path, char *out_folder, size_t out_size)
{
    const char *last_slash;
    size_t length;

    if (out_folder == NULL || out_size == 0) {
        return;
    }
    out_folder[0] = '\0';
    if (path == NULL || path[0] == '\0') {
        return;
    }
    last_slash = strrchr(path, '\\');
    if (last_slash == NULL) {
        last_slash = strrchr(path, '/');
    }
    if (last_slash == NULL) {
        return;
    }
    length = (size_t)(last_slash - path);
    if (length == 2u && path[1] == ':' && last_slash[0] != '\0') {
        length = 3u;
    }
    if (length >= out_size) {
        length = out_size - 1;
    }
    memcpy(out_folder, path, length);
    out_folder[length] = '\0';
}

static void load_playlist_last_paths(PlayerState *state)
{
    char path[MAX_PATH];

    if (state == NULL) {
        return;
    }
    get_player_ini_path(path, sizeof(path));
    GetPrivateProfileStringA("Playlist", "LastFolder", "", state->playlist_last_folder, sizeof(state->playlist_last_folder), path);
    GetPrivateProfileStringA("Playlist", "LastFile", "", state->playlist_last_file, sizeof(state->playlist_last_file), path);
}

static void save_playlist_last_paths(const PlayerState *state)
{
    char path[MAX_PATH];

    if (state == NULL) {
        return;
    }
    get_player_ini_path(path, sizeof(path));
    WritePrivateProfileStringA("Playlist", "LastFolder", state->playlist_last_folder, path);
    WritePrivateProfileStringA("Playlist", "LastFile", state->playlist_last_file, path);
}

static void remember_playlist_file(PlayerState *state, const char *file_path)
{
    if (state == NULL || file_path == NULL || file_path[0] == '\0') {
        return;
    }
    snprintf(state->playlist_last_file, sizeof(state->playlist_last_file), "%s", file_path);
    path_parent_folder(file_path, state->playlist_last_folder, sizeof(state->playlist_last_folder));
    save_playlist_last_paths(state);
}

static void remember_playlist_folder(PlayerState *state, const char *folder_path)
{
    if (state == NULL || folder_path == NULL || folder_path[0] == '\0') {
        return;
    }
    snprintf(state->playlist_last_folder, sizeof(state->playlist_last_folder), "%s", folder_path);
    save_playlist_last_paths(state);
}

static void load_color_dialog_bounds(int *x, int *y)
{
    char path[MAX_PATH];

    if (x == NULL || y == NULL) {
        return;
    }

    *x = CW_USEDEFAULT;
    *y = CW_USEDEFAULT;

    get_player_ini_path(path, sizeof(path));
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) {
        return;
    }

    *x = GetPrivateProfileIntA("ColorDialog", "X", *x, path);
    *y = GetPrivateProfileIntA("ColorDialog", "Y", *y, path);
}

static void save_color_dialog_bounds(HWND hwnd)
{
    char path[MAX_PATH];
    RECT rect;

    if (hwnd == NULL || IsIconic(hwnd)) {
        return;
    }
    if (!GetWindowRect(hwnd, &rect)) {
        return;
    }

    get_player_ini_path(path, sizeof(path));
    write_profile_int(path, "ColorDialog", "X", rect.left);
    write_profile_int(path, "ColorDialog", "Y", rect.top);
}

static void load_font_dialog_bounds(int *x, int *y)
{
    char path[MAX_PATH];

    if (x == NULL || y == NULL) {
        return;
    }

    *x = CW_USEDEFAULT;
    *y = CW_USEDEFAULT;

    get_player_ini_path(path, sizeof(path));
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) {
        return;
    }

    *x = GetPrivateProfileIntA("FontDialog", "X", *x, path);
    *y = GetPrivateProfileIntA("FontDialog", "Y", *y, path);
}

static void save_font_dialog_bounds(HWND hwnd)
{
    char path[MAX_PATH];
    RECT rect;

    if (hwnd == NULL || IsIconic(hwnd)) {
        return;
    }
    if (!GetWindowRect(hwnd, &rect)) {
        return;
    }

    get_player_ini_path(path, sizeof(path));
    write_profile_int(path, "FontDialog", "X", rect.left);
    write_profile_int(path, "FontDialog", "Y", rect.top);
}

static void player_log(const char *format, ...)
{
    char path[MAX_PATH];
    SYSTEMTIME now;
    FILE *file;
    va_list args;
    static int first_log_write = 1;

    if (!g_text_log_enabled) {
        return;
    }

    get_player_log_path(path, sizeof(path));
    file = fopen(path, first_log_write ? "wb" : "ab");
    if (file == NULL) {
        return;
    }
    first_log_write = 0;

    GetLocalTime(&now);
    fprintf(file, "%04u-%02u-%02u %02u:%02u:%02u.%03u ",
        (unsigned)now.wYear,
        (unsigned)now.wMonth,
        (unsigned)now.wDay,
        (unsigned)now.wHour,
        (unsigned)now.wMinute,
        (unsigned)now.wSecond,
        (unsigned)now.wMilliseconds);

    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);

    fputc('\n', file);
    fclose(file);
}

static LONG WINAPI player_unhandled_exception_filter(EXCEPTION_POINTERS *exception_info)
{
    DWORD code = 0;
    void *address = NULL;
    HMODULE module = GetModuleHandleA(NULL);

    if (exception_info != NULL && exception_info->ExceptionRecord != NULL) {
        code = exception_info->ExceptionRecord->ExceptionCode;
        address = exception_info->ExceptionRecord->ExceptionAddress;
    }

    player_log("CRASH exception=0x%08lx address=%p module=%p rva=0x%Ix",
        (unsigned long)code,
        address,
        (void *)module,
        (size_t)((char *)address - (char *)module));
    return EXCEPTION_EXECUTE_HANDLER;
}

static void player_log_audacious_debug_state(const char *reason)
{
    player_log(
        "audacious debug %s sample=%llu hook_writes=%llu slices=%llu cpu_slices=%llu hw_writes=%llu stage=%lu cur_thread=%d pc=0x%08lX op=0x%08lX sp=0x%08lX ra=0x%08lX v0=0x%08lX a0=0x%08lX a1=0x%08lX delayr=0x%08lX delayv=0x%08lX hle_pc=0x%08lX hle_sub=0x%02lX last_hw=0x%08lX",
        reason != NULL ? reason : "(none)",
        (unsigned long long)spu2log_audacious_get_sample_pos(),
        (unsigned long long)spu2log_audacious_get_spu2_write16_count(),
        (unsigned long long)spu2log_audacious_debug_get_slice_count(),
        (unsigned long long)spu2log_audacious_debug_get_cpu_slice_count(),
        (unsigned long long)spu2log_audacious_debug_get_hw_write_count(),
        (unsigned long)spu2log_audacious_debug_get_stage(),
        spu2log_audacious_debug_get_current_thread(),
        (unsigned long)spu2log_audacious_debug_get_pc(),
        (unsigned long)spu2log_audacious_debug_get_opcode(),
        (unsigned long)spu2log_audacious_debug_get_sp(),
        (unsigned long)spu2log_audacious_debug_get_ra(),
        (unsigned long)spu2log_audacious_debug_get_v0(),
        (unsigned long)spu2log_audacious_debug_get_a0(),
        (unsigned long)spu2log_audacious_debug_get_a1(),
        (unsigned long)spu2log_audacious_debug_get_delayr(),
        (unsigned long)spu2log_audacious_debug_get_delayv(),
        (unsigned long)spu2log_audacious_debug_get_hle_pc(),
        (unsigned long)spu2log_audacious_debug_get_hle_subcall(),
        (unsigned long)spu2log_audacious_debug_get_last_hw_write());
}

static int16_t pcm_peak_abs(const int16_t *pcm, uint32_t frames)
{
    uint32_t i;
    int peak = 0;

    if (pcm == NULL) {
        return 0;
    }

    for (i = 0; i < frames * 2u; ++i) {
        int value = pcm[i];
        if (value < 0) {
            value = -value;
        }
        if (value > peak) {
            peak = value;
        }
    }

    if (peak > 32767) {
        peak = 32767;
    }
    return (int16_t)peak;
}

static void lock_state(PlayerState *state)
{
    if (state != NULL && state->lock_ready) {
        EnterCriticalSection(&state->lock);
    }
}

static void unlock_state(PlayerState *state)
{
    if (state != NULL && state->lock_ready) {
        LeaveCriticalSection(&state->lock);
    }
}

static int effective_speed_percent_locked(const PlayerState *state)
{
    if (state == NULL) {
        return 100;
    }
    return state->tab_speed_active ? 200 : state->speed_percent;
}

static uint64_t scale_timeline_delta(uint64_t delta, int speed_percent)
{
    uint64_t whole;
    uint64_t remainder;

    if (speed_percent < 0) {
        speed_percent = 0;
    }
    whole = delta / 100u;
    remainder = delta % 100u;
    return whole * (uint64_t)speed_percent +
        (remainder * (uint64_t)speed_percent) / 100u;
}

static uint64_t timeline_sample_at_locked(const PlayerState *state, uint64_t source_sample)
{
    if (state == NULL || !state->timeline_valid) {
        return 0;
    }
    if (source_sample < state->timeline_source_anchor) {
        return 0;
    }
    return state->timeline_scaled_anchor + scale_timeline_delta(
        source_sample - state->timeline_source_anchor,
        state->timeline_speed_percent);
}

static void rebase_timeline_speed_locked(PlayerState *state, int speed_percent)
{
    uint64_t source_sample;
    uint64_t scaled_sample;

    if (state == NULL) {
        return;
    }
    if (!state->timeline_valid) {
        state->timeline_speed_percent = speed_percent;
        return;
    }
    source_sample = state->live.last_sample_pos;
    scaled_sample = timeline_sample_at_locked(state, source_sample);
    state->timeline_source_anchor = source_sample;
    state->timeline_scaled_anchor = scaled_sample;
    state->timeline_speed_percent = speed_percent;
    state->timeline_valid = 1;
}

static void set_status(PlayerState *state, const char *status)
{
    if (state == NULL || status == NULL) {
        return;
    }

    lock_state(state);
    snprintf(state->status, sizeof(state->status), "%s", status);
    unlock_state(state);
}

static int get_playing(PlayerState *state)
{
    int playing;

    if (state == NULL) {
        return 0;
    }

    lock_state(state);
    playing = state->playing;
    unlock_state(state);
    return playing;
}

static void set_playing(PlayerState *state, int playing)
{
    if (state == NULL) {
        return;
    }

    lock_state(state);
    state->playing = playing;
    unlock_state(state);
}

static int get_paused(PlayerState *state)
{
    int paused;

    if (state == NULL) {
        return 0;
    }

    lock_state(state);
    paused = state->paused;
    unlock_state(state);
    return paused;
}

static void set_paused(PlayerState *state, int paused)
{
    if (state == NULL) {
        return;
    }

    lock_state(state);
    state->paused = paused ? 1 : 0;
    unlock_state(state);
}

static void update_pause_button_label(PlayerState *state)
{
    int paused;

    if (state == NULL || state->pause_button == NULL) {
        return;
    }

    paused = get_paused(state);
    SetWindowTextA(state->pause_button, paused ? "Resume" : "Pause");
}

static void toggle_pause_playback(HWND hwnd, PlayerState *state)
{
    int paused;

    if (state == NULL || state->core == NULL || !get_playing(state)) {
        return;
    }

    paused = !get_paused(state);
    set_paused(state, paused);
    if (state->wave != NULL && state->audio_started) {
        if (paused) {
            waveOutPause(state->wave);
        } else {
            waveOutRestart(state->wave);
        }
    }
    set_status(state, paused ? "Paused" : "Playing direct");
    update_pause_button_label(state);
    if (!paused) {
        state->has_last_output = 0;
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

static void reset_live_display(PlayerState *state)
{
    uint8_t core;
    uint8_t voice;

    if (state == NULL) {
        return;
    }

    lock_state(state);
    ZeroMemory(&state->live, sizeof(state->live));
    ZeroMemory(&state->akao, sizeof(state->akao));
    state->live.version = SPU2LOG_VERSION;
    state->live.sample_rate = PLAYER_SAMPLE_RATE;
    state->stopped_display = 1;
    for (core = 0; core < 2; ++core) {
        state->live.cores[core].core = core;
        for (voice = 0; voice < 24; ++voice) {
            state->live.voices[core][voice].core = core;
            state->live.voices[core][voice].voice = voice;
            state->live.voices[core][voice].adsr_phase = SPU2LOG_ADSR_OFF;
        }
    }
    state->sequence += 1;
    ZeroMemory(state->key_on_event_buffers, sizeof(state->key_on_event_buffers));
    ZeroMemory(state->release_event_buffers, sizeof(state->release_event_buffers));
    ZeroMemory(state->release_event_latched, sizeof(state->release_event_latched));
    ZeroMemory(state->voice_display_hold_until, sizeof(state->voice_display_hold_until));
    ZeroMemory(state->voice_display_hold, sizeof(state->voice_display_hold));
    ZeroMemory(state->default_adsr1, sizeof(state->default_adsr1));
    ZeroMemory(state->default_adsr2, sizeof(state->default_adsr2));
    ZeroMemory(state->default_pitch, sizeof(state->default_pitch));
    ZeroMemory(state->default_vol_l, sizeof(state->default_vol_l));
    ZeroMemory(state->default_vol_r, sizeof(state->default_vol_r));
    ZeroMemory(state->default_voice_valid, sizeof(state->default_voice_valid));
    ZeroMemory(state->default_noise_clock, sizeof(state->default_noise_clock));
    ZeroMemory(state->default_reverb_l, sizeof(state->default_reverb_l));
    ZeroMemory(state->default_reverb_r, sizeof(state->default_reverb_r));
    ZeroMemory(state->default_core_valid, sizeof(state->default_core_valid));
    ZeroMemory(state->manual_reverb_value_valid, sizeof(state->manual_reverb_value_valid));
    ZeroMemory(state->debug_saved_adsr1, sizeof(state->debug_saved_adsr1));
    ZeroMemory(state->debug_saved_adsr2, sizeof(state->debug_saved_adsr2));
    ZeroMemory(state->debug_saved_pitch, sizeof(state->debug_saved_pitch));
    ZeroMemory(state->debug_saved_vol_l, sizeof(state->debug_saved_vol_l));
    ZeroMemory(state->debug_saved_vol_r, sizeof(state->debug_saved_vol_r));
    ZeroMemory(state->debug_saved_voice_valid, sizeof(state->debug_saved_voice_valid));
    ZeroMemory(state->debug_saved_noise_clock, sizeof(state->debug_saved_noise_clock));
    ZeroMemory(state->debug_saved_reverb_l, sizeof(state->debug_saved_reverb_l));
    ZeroMemory(state->debug_saved_reverb_r, sizeof(state->debug_saved_reverb_r));
    ZeroMemory(state->debug_saved_core_valid, sizeof(state->debug_saved_core_valid));
    ZeroMemory(state->gauge_env, sizeof(state->gauge_env));
    ZeroMemory(state->gauge_vol_l, sizeof(state->gauge_vol_l));
    ZeroMemory(state->gauge_vol_r, sizeof(state->gauge_vol_r));
    ZeroMemory(state->gauge_valid, sizeof(state->gauge_valid));
    state->last_gauge_update_ms = 0;
    state->gauge_animation_active = 0;
    state->timeline_source_anchor = 0;
    state->timeline_scaled_anchor = 0;
    state->timeline_speed_percent = effective_speed_percent_locked(state);
    state->timeline_valid = 1;
    ZeroMemory(state->audio_display_snapshots, sizeof(state->audio_display_snapshots));
    ZeroMemory(&state->audible_display_snapshot, sizeof(state->audible_display_snapshot));
    state->audible_display_valid = 0;
    unlock_state(state);
}

static void reset_stopped_display(PlayerState *state)
{
    uint8_t core;
    uint8_t voice;

    if (state == NULL) {
        return;
    }

    lock_state(state);
    ZeroMemory(&state->live, sizeof(state->live));
    ZeroMemory(&state->akao, sizeof(state->akao));
    state->live.version = SPU2LOG_VERSION;
    state->live.sample_rate = PLAYER_SAMPLE_RATE;
    state->stopped_display = 1;
    state->ps2_startup_track_dimmed = 0;
    for (core = 0; core < 2; ++core) {
        state->live.cores[core].core = core;
        state->live.cores[core].reverb_l = state->default_core_valid[core] ? state->default_reverb_l[core] : 0u;
        state->live.cores[core].reverb_r = state->default_core_valid[core] ? state->default_reverb_r[core] : 0u;
        state->live.cores[core].flags = (uint32_t)((state->default_core_valid[core] ? state->default_noise_clock[core] : 0u) << 8);
        for (voice = 0; voice < 24; ++voice) {
            Spu2LogVoiceSnapshot *v = &state->live.voices[core][voice];
            v->core = core;
            v->voice = voice;
            v->adsr_phase = SPU2LOG_ADSR_SUSTAIN;
            v->adsr1 = 0u;
            v->adsr2 = 0u;
            v->noise_clock = state->default_core_valid[core] ? state->default_noise_clock[core] : 0u;
        }
    }
    state->sequence += 1;
    ZeroMemory(state->key_on_event_buffers, sizeof(state->key_on_event_buffers));
    ZeroMemory(state->release_event_buffers, sizeof(state->release_event_buffers));
    ZeroMemory(state->release_event_latched, sizeof(state->release_event_latched));
    ZeroMemory(state->voice_display_hold_until, sizeof(state->voice_display_hold_until));
    ZeroMemory(state->voice_display_hold, sizeof(state->voice_display_hold));
    ZeroMemory(state->debug_saved_adsr1, sizeof(state->debug_saved_adsr1));
    ZeroMemory(state->debug_saved_adsr2, sizeof(state->debug_saved_adsr2));
    ZeroMemory(state->debug_saved_pitch, sizeof(state->debug_saved_pitch));
    ZeroMemory(state->debug_saved_vol_l, sizeof(state->debug_saved_vol_l));
    ZeroMemory(state->debug_saved_vol_r, sizeof(state->debug_saved_vol_r));
    ZeroMemory(state->debug_saved_voice_valid, sizeof(state->debug_saved_voice_valid));
    ZeroMemory(state->debug_saved_noise_clock, sizeof(state->debug_saved_noise_clock));
    ZeroMemory(state->debug_saved_reverb_l, sizeof(state->debug_saved_reverb_l));
    ZeroMemory(state->debug_saved_reverb_r, sizeof(state->debug_saved_reverb_r));
    ZeroMemory(state->debug_saved_core_valid, sizeof(state->debug_saved_core_valid));
    ZeroMemory(state->gauge_env, sizeof(state->gauge_env));
    ZeroMemory(state->gauge_vol_l, sizeof(state->gauge_vol_l));
    ZeroMemory(state->gauge_vol_r, sizeof(state->gauge_vol_r));
    ZeroMemory(state->gauge_valid, sizeof(state->gauge_valid));
    state->last_gauge_update_ms = 0;
    state->gauge_animation_active = 0;
    state->timeline_source_anchor = 0;
    state->timeline_scaled_anchor = 0;
    state->timeline_speed_percent = effective_speed_percent_locked(state);
    state->timeline_valid = 1;
    ZeroMemory(state->audio_display_snapshots, sizeof(state->audio_display_snapshots));
    ZeroMemory(&state->audible_display_snapshot, sizeof(state->audible_display_snapshot));
    state->audible_display_valid = 0;
    unlock_state(state);
}

static int get_speed_percent(PlayerState *state)
{
    int speed_percent;

    if (state == NULL) {
        return 100;
    }

    lock_state(state);
    speed_percent = state->speed_percent;
    unlock_state(state);
    return speed_percent;
}

static int get_effective_speed_percent(PlayerState *state)
{
    int speed_percent;
    int tab_speed_active;

    if (state == NULL) {
        return 100;
    }

    lock_state(state);
    speed_percent = state->speed_percent;
    tab_speed_active = state->tab_speed_active;
    unlock_state(state);
    return tab_speed_active ? 200 : speed_percent;
}

static void apply_effective_speed_percent(PlayerState *state)
{
    psf2log_set_imported_tempo_percent(get_effective_speed_percent(state));
}

static void update_speed_menu_check(HWND hwnd, const PlayerState *state)
{
    HMENU menu;
    unsigned i;

    if (hwnd == NULL || state == NULL) {
        return;
    }
    menu = GetMenu(hwnd);
    if (menu == NULL) {
        return;
    }
    for (i = 0; i < SPEED_PRESET_COUNT; ++i) {
        CheckMenuItem(menu,
            IDM_SPEED_PRESET_FIRST + i,
            MF_BYCOMMAND |
                (state->speed_percent == g_speed_menu_values[i] ?
                    MF_CHECKED : MF_UNCHECKED));
    }
}

static void set_speed_percent(HWND hwnd, PlayerState *state, int speed_percent)
{
    if (state == NULL) {
        return;
    }

    if (speed_percent < 10) {
        speed_percent = 10;
    }
    if (speed_percent > 200) {
        speed_percent = 200;
    }
    speed_percent = ((speed_percent + 2) / 5) * 5;

    lock_state(state);
    state->speed_percent = speed_percent;
    rebase_timeline_speed_locked(state, effective_speed_percent_locked(state));
    unlock_state(state);

    apply_effective_speed_percent(state);
    if (state->speed_label != NULL) {
        InvalidateRect(state->speed_label, NULL, TRUE);
    }
    if (state->speed_slider != NULL &&
        (int)SendMessageA(state->speed_slider, TBM_GETPOS, 0, 0) != speed_percent) {
        SendMessageA(state->speed_slider, TBM_SETPOS, TRUE, speed_percent);
    }
    update_speed_menu_check(hwnd, state);
    InvalidateRect(hwnd, NULL, FALSE);
}

static void set_tab_speed_active(HWND hwnd, PlayerState *state, int active)
{
    int changed;

    if (state == NULL) {
        return;
    }

    lock_state(state);
    changed = (state->tab_speed_active != (active ? 1 : 0));
    if (changed) {
        rebase_timeline_speed_locked(state, effective_speed_percent_locked(state));
    }
    state->tab_speed_active = active ? 1 : 0;
    if (changed) {
        state->timeline_speed_percent = effective_speed_percent_locked(state);
    }
    unlock_state(state);

    if (changed) {
        apply_effective_speed_percent(state);
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

static void save_volume_percent(int volume_percent)
{
    char path[MAX_PATH];

    get_player_ini_path(path, sizeof(path));
    write_profile_int(path, "Audio", "VolumePercent", volume_percent);
}

static int normalize_volume_percent(int volume_percent)
{
    if (volume_percent < 0) {
        return 0;
    }
    if (volume_percent > 200) {
        return 200;
    }
    return ((volume_percent + 5) / 10) * 10;
}

static int get_volume_percent(PlayerState *state)
{
    int volume_percent;

    if (state == NULL) {
        return 100;
    }

    lock_state(state);
    volume_percent = state->volume_percent;
    unlock_state(state);
    return normalize_volume_percent(volume_percent);
}

static void set_volume_percent(HWND hwnd, PlayerState *state, int volume_percent, int save_setting)
{
    if (state == NULL) {
        return;
    }

    volume_percent = normalize_volume_percent(volume_percent);
    lock_state(state);
    state->volume_percent = volume_percent;
    unlock_state(state);

    if (save_setting) {
        save_volume_percent(volume_percent);
    }
    if (state->volume_label != NULL) {
        InvalidateRect(state->volume_label, NULL, TRUE);
    }
    update_settings_menu_check(hwnd, state);
    InvalidateRect(hwnd, NULL, FALSE);
}

static void adjust_volume_percent(HWND hwnd, PlayerState *state, int delta_percent)
{
    if (state == NULL) {
        return;
    }

    set_volume_percent(hwnd, state, get_volume_percent(state) + delta_percent, 1);
}

static int normalize_color_index(int value)
{
    if (value < 0 || value > CUSTOM_COLOR_INDEX) {
        return 0;
    }
    return value;
}

static COLORREF normalize_color_value(int value, COLORREF fallback)
{
    if (value < 0 || value > 0x00ffffff) {
        return fallback;
    }
    return (COLORREF)value;
}

static COLORREF read_profile_color(const char *path, const char *section, const char *key, COLORREF fallback)
{
    char text[32];
    unsigned value;

    GetPrivateProfileStringA(section, key, "", text, sizeof(text), path);
    if (text[0] == '\0') {
        return fallback;
    }
    if (text[0] == '#') {
        if (sscanf(text + 1, "%x", &value) == 1 && value <= 0x00ffffffu) {
            return RGB((value >> 16) & 0xffu, (value >> 8) & 0xffu, value & 0xffu);
        }
        return fallback;
    }
    if (sscanf(text, "%u", &value) == 1 && value <= 0x00ffffffu) {
        return (COLORREF)value;
    }
    if (sscanf(text, "0x%x", &value) == 1 && value <= 0x00ffffffu) {
        return RGB((value >> 16) & 0xffu, (value >> 8) & 0xffu, value & 0xffu);
    }
    if (strlen(text) == 6u && sscanf(text, "%x", &value) == 1 && value <= 0x00ffffffu) {
        return RGB((value >> 16) & 0xffu, (value >> 8) & 0xffu, value & 0xffu);
    }
    return fallback;
}

static void write_profile_color(const char *path, const char *section, const char *key, COLORREF color)
{
    char text[16];

    snprintf(text,
        sizeof(text),
        "#%02X%02X%02X",
        (unsigned)GetRValue(color),
        (unsigned)GetGValue(color),
        (unsigned)GetBValue(color));
    WritePrivateProfileStringA(section, key, text, path);
}

static void init_custom_colors(PlayerState *state)
{
    static const COLORREF defaults[CUSTOM_COLOR_COUNT] = {
        RGB(80, 180, 95),
        RGB(60, 180, 205),
        RGB(80, 125, 220),
        RGB(215, 155, 55),
        RGB(210, 80, 70),
        RGB(190, 85, 190),
        RGB(130, 105, 220),
        RGB(215, 215, 215),
        RGB(40, 40, 40),
        RGB(80, 80, 80),
        RGB(120, 120, 120),
        RGB(160, 160, 160),
        RGB(200, 200, 200),
        RGB(240, 240, 240),
        RGB(0, 0, 0),
        RGB(255, 255, 255)
    };

    if (state == NULL) {
        return;
    }

    memcpy(state->custom_colors, defaults, sizeof(defaults));
}

static void load_display_settings(PlayerState *state)
{
    char path[MAX_PATH];
    unsigned i;

    if (state == NULL) {
        return;
    }

    get_player_ini_path(path, sizeof(path));
    state->env_color_index = normalize_color_index(GetPrivateProfileIntA("Display", "EnvColor", state->env_color_index, path));
    state->lr_color_index = normalize_color_index(GetPrivateProfileIntA("Display", "LrColor", state->lr_color_index, path));
    state->env_custom_color = read_profile_color(path, "Display", "EnvCustomColor", state->env_custom_color);
    state->lr_custom_color = read_profile_color(path, "Display", "LrCustomColor", state->lr_custom_color);
    state->theme_env_color_index[0] = normalize_color_index(GetPrivateProfileIntA("LightTheme", "EnvColor", state->env_color_index, path));
    state->theme_lr_color_index[0] = normalize_color_index(GetPrivateProfileIntA("LightTheme", "LrColor", state->lr_color_index, path));
    state->theme_env_custom_color[0] = read_profile_color(path, "LightTheme", "EnvCustomColor", state->env_custom_color);
    state->theme_lr_custom_color[0] = read_profile_color(path, "LightTheme", "LrCustomColor", state->lr_custom_color);
    state->theme_env_color_index[1] = normalize_color_index(GetPrivateProfileIntA("DarkTheme", "EnvColor", state->env_color_index, path));
    state->theme_lr_color_index[1] = normalize_color_index(GetPrivateProfileIntA("DarkTheme", "LrColor", state->lr_color_index, path));
    state->theme_env_custom_color[1] = read_profile_color(path, "DarkTheme", "EnvCustomColor", state->env_custom_color);
    state->theme_lr_custom_color[1] = read_profile_color(path, "DarkTheme", "LrCustomColor", state->lr_custom_color);
    state->playback_only = GetPrivateProfileIntA("Display", "PlaybackOnly", state->playback_only, path) ? 1 : 0;
    state->timbre_scan_enabled = GetPrivateProfileIntA("Display", "TimbreScan", state->timbre_scan_enabled, path) ? 1 : 0;
    state->timbre_topmost = GetPrivateProfileIntA("TimbreWindow", "Topmost", state->timbre_topmost, path) ? 1 : 0;
    state->theme_mode = normalize_theme_mode(GetPrivateProfileIntA("Display", "Theme", state->theme_mode, path));
    sync_active_gauge_colors_from_theme(state);
    state->performance_profile = GetPrivateProfileIntA("Display", "PerformanceProfile", state->performance_profile, path);
    if (state->performance_profile < 0 || state->performance_profile > 2) {
        state->performance_profile = 2;
    }
    state->volume_percent = normalize_volume_percent(GetPrivateProfileIntA("Audio", "VolumePercent", state->volume_percent, path));
    state->tag_fade_enabled = GetPrivateProfileIntA(
        "Playback", "TagFadeEnabled", state->tag_fade_enabled, path) ? 1 : 0;
    state->playlist_random_mode = GetPrivateProfileIntA(
        "Playlist", "RandomMode", state->playlist_random_mode, path) ? 1 : 0;
    state->playlist_single_track_mode = GetPrivateProfileIntA(
        "Playlist", "SingleTrackMode", state->playlist_single_track_mode, path) ? 1 : 0;
    if (state->playlist_single_track_mode) {
        state->playlist_random_mode = 0;
    }
    for (i = 0; i < CUSTOM_COLOR_COUNT; ++i) {
        char key[32];
        snprintf(key, sizeof(key), "Stock%02u", i);
        state->custom_colors[i] = read_profile_color(path, "CustomColors", key, state->custom_colors[i]);
    }
}

static void save_display_settings(const PlayerState *state)
{
    char path[MAX_PATH];
    unsigned i;

    if (state == NULL) {
        return;
    }

    get_player_ini_path(path, sizeof(path));
    write_profile_int(path, "Display", "EnvColor", state->env_color_index);
    write_profile_int(path, "Display", "LrColor", state->lr_color_index);
    write_profile_color(path, "Display", "EnvCustomColor", state->env_custom_color);
    write_profile_color(path, "Display", "LrCustomColor", state->lr_custom_color);
    write_profile_int(path, "LightTheme", "EnvColor", state->theme_env_color_index[0]);
    write_profile_int(path, "LightTheme", "LrColor", state->theme_lr_color_index[0]);
    write_profile_color(path, "LightTheme", "EnvCustomColor", state->theme_env_custom_color[0]);
    write_profile_color(path, "LightTheme", "LrCustomColor", state->theme_lr_custom_color[0]);
    write_profile_int(path, "DarkTheme", "EnvColor", state->theme_env_color_index[1]);
    write_profile_int(path, "DarkTheme", "LrColor", state->theme_lr_color_index[1]);
    write_profile_color(path, "DarkTheme", "EnvCustomColor", state->theme_env_custom_color[1]);
    write_profile_color(path, "DarkTheme", "LrCustomColor", state->theme_lr_custom_color[1]);
    write_profile_int(path, "Display", "PlaybackOnly", state->playback_only ? 1 : 0);
    write_profile_int(path, "Display", "TimbreScan", state->timbre_scan_enabled ? 1 : 0);
    write_profile_int(path, "TimbreWindow", "Topmost", state->timbre_topmost ? 1 : 0);
    write_profile_int(path, "Display", "Theme", normalize_theme_mode(state->theme_mode));
    write_profile_int(path, "Display", "PerformanceProfile", state->performance_profile);
    write_profile_int(path, "Playback", "TagFadeEnabled",
        state->tag_fade_enabled ? 1 : 0);
    write_profile_int(path, "Playlist", "RandomMode",
        state->playlist_random_mode ? 1 : 0);
    write_profile_int(path, "Playlist", "SingleTrackMode",
        state->playlist_single_track_mode ? 1 : 0);
    for (i = 0; i < CUSTOM_COLOR_COUNT; ++i) {
        char key[32];
        snprintf(key, sizeof(key), "Stock%02u", i);
        write_profile_color(path, "CustomColors", key, state->custom_colors[i]);
    }
}

static COLORREF env_color_from_index(int color_index, COLORREF custom_color)
{
    switch (normalize_color_index(color_index)) {
    case 1:
        return RGB(80, 125, 220);
    case 2:
        return RGB(60, 180, 205);
    case 3:
        return RGB(215, 155, 55);
    case 4:
        return RGB(210, 80, 70);
    case 5:
        return RGB(190, 85, 190);
    case 6:
        return RGB(130, 105, 220);
    case 7:
        return RGB(215, 215, 215);
    case CUSTOM_COLOR_INDEX:
        return custom_color;
    default:
        return RGB(80, 180, 95);
    }
}

static COLORREF lighten_color(COLORREF color)
{
    int red = GetRValue(color);
    int green = GetGValue(color);
    int blue = GetBValue(color);

    red += (255 - red) / 3;
    green += (255 - green) / 3;
    blue += (255 - blue) / 3;
    return RGB(red, green, blue);
}

static COLORREF blend_color(COLORREF color, COLORREF target, unsigned color_weight, unsigned target_weight)
{
    unsigned total = color_weight + target_weight;
    unsigned red;
    unsigned green;
    unsigned blue;

    if (total == 0) {
        return color;
    }
    red = ((unsigned)GetRValue(color) * color_weight + (unsigned)GetRValue(target) * target_weight) / total;
    green = ((unsigned)GetGValue(color) * color_weight + (unsigned)GetGValue(target) * target_weight) / total;
    blue = ((unsigned)GetBValue(color) * color_weight + (unsigned)GetBValue(target) * target_weight) / total;
    return RGB(red, green, blue);
}

static COLORREF muted_gauge_color(COLORREF color, COLORREF background_color)
{
    return blend_color(color, background_color, 1u, 3u);
}

static COLORREF lr_left_color_from_index(int color_index, COLORREF custom_color)
{
    switch (normalize_color_index(color_index)) {
    case 1:
        return RGB(80, 125, 220);
    case 2:
        return RGB(50, 175, 210);
    case 3:
        return RGB(215, 155, 55);
    case 4:
        return RGB(210, 80, 70);
    case 5:
        return RGB(190, 85, 190);
    case 6:
        return RGB(130, 105, 220);
    case 7:
        return RGB(200, 200, 200);
    case CUSTOM_COLOR_INDEX:
        return custom_color;
    default:
        return RGB(80, 180, 95);
    }
}

static COLORREF lr_right_color_from_index(int color_index, COLORREF custom_color)
{
    switch (normalize_color_index(color_index)) {
    case 1:
        return RGB(80, 170, 220);
    case 2:
        return RGB(95, 205, 225);
    case 3:
        return RGB(230, 185, 75);
    case 4:
        return RGB(225, 115, 100);
    case 5:
        return RGB(215, 120, 215);
    case 6:
        return RGB(165, 135, 235);
    case 7:
        return RGB(235, 235, 235);
    case CUSTOM_COLOR_INDEX:
        return lighten_color(custom_color);
    default:
        return RGB(115, 205, 120);
    }
}

static int normalize_performance_profile(int profile)
{
    if (profile < 0 || profile > 2) {
        return 2;
    }
    return profile;
}

static int normalize_theme_mode(int mode)
{
    if (mode < THEME_SYSTEM || mode > THEME_DARK) {
        return THEME_SYSTEM;
    }
    return mode;
}

static int windows_apps_use_light_theme(void)
{
    HKEY key;
    DWORD value = 1;
    DWORD size = sizeof(value);

    if (RegOpenKeyExA(HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            0,
            KEY_READ,
            &key) != ERROR_SUCCESS) {
        return 1;
    }
    if (RegQueryValueExA(key, "AppsUseLightTheme", NULL, NULL, (LPBYTE)&value, &size) != ERROR_SUCCESS) {
        value = 1;
    }
    RegCloseKey(key);
    return value != 0;
}

static int is_dark_theme_active(const PlayerState *state)
{
    int mode;

    if (state == NULL) {
        return 0;
    }
    mode = normalize_theme_mode(state->theme_mode);
    if (mode == THEME_DARK) {
        return 1;
    }
    if (mode == THEME_LIGHT) {
        return 0;
    }
    return !windows_apps_use_light_theme();
}

static COLORREF player_background_color(const PlayerState *state)
{
    return is_dark_theme_active(state) ? RGB(0, 0, 0) : GetSysColor(COLOR_WINDOW);
}

static COLORREF player_text_color(const PlayerState *state)
{
    return is_dark_theme_active(state) ? RGB(235, 235, 235) : RGB(20, 20, 20);
}

static COLORREF player_inactive_text_color(const PlayerState *state)
{
    return is_dark_theme_active(state) ? RGB(85, 85, 85) : RGB(205, 205, 205);
}

static COLORREF player_muted_text_color(const PlayerState *state)
{
    return is_dark_theme_active(state) ? RGB(120, 120, 120) : RGB(210, 210, 210);
}

static int active_theme_slot(const PlayerState *state)
{
    return is_dark_theme_active(state) ? 1 : 0;
}

static void sync_active_gauge_colors_from_theme(PlayerState *state)
{
    int slot;

    if (state == NULL) {
        return;
    }
    slot = active_theme_slot(state);
    state->env_color_index = state->theme_env_color_index[slot];
    state->lr_color_index = state->theme_lr_color_index[slot];
    state->env_custom_color = state->theme_env_custom_color[slot];
    state->lr_custom_color = state->theme_lr_custom_color[slot];
}

static void store_active_gauge_colors_to_theme(PlayerState *state)
{
    int slot;

    if (state == NULL) {
        return;
    }
    slot = active_theme_slot(state);
    state->theme_env_color_index[slot] = state->env_color_index;
    state->theme_lr_color_index[slot] = state->lr_color_index;
    state->theme_env_custom_color[slot] = state->env_custom_color;
    state->theme_lr_custom_color[slot] = state->lr_custom_color;
}

typedef HRESULT (WINAPI *DwmSetWindowAttributeProc)(HWND, DWORD, LPCVOID, DWORD);
typedef HRESULT (WINAPI *SetWindowThemeProc)(HWND, LPCWSTR, LPCWSTR);
typedef int (WINAPI *SetPreferredAppModeProc)(int);
typedef void (WINAPI *FlushMenuThemesProc)(void);

static void apply_app_menu_dark_mode(int dark)
{
    HMODULE ux_theme;
    SetPreferredAppModeProc set_preferred_app_mode;
    FlushMenuThemesProc flush_menu_themes;

    ux_theme = LoadLibraryA("uxtheme.dll");
    if (ux_theme == NULL) {
        return;
    }
    set_preferred_app_mode = (SetPreferredAppModeProc)GetProcAddress(ux_theme, MAKEINTRESOURCEA(135));
    flush_menu_themes = (FlushMenuThemesProc)GetProcAddress(ux_theme, MAKEINTRESOURCEA(136));
    if (set_preferred_app_mode != NULL) {
        (void)set_preferred_app_mode(dark ? 2 : 3);
    }
    if (flush_menu_themes != NULL) {
        flush_menu_themes();
    }
    FreeLibrary(ux_theme);
}

static void apply_window_dark_title(HWND hwnd, const PlayerState *state)
{
    HMODULE dwm;
    DwmSetWindowAttributeProc set_attribute;
    BOOL dark;

    if (hwnd == NULL) {
        return;
    }
    dwm = LoadLibraryA("dwmapi.dll");
    if (dwm == NULL) {
        return;
    }
    set_attribute = (DwmSetWindowAttributeProc)GetProcAddress(dwm, "DwmSetWindowAttribute");
    if (set_attribute != NULL) {
        dark = is_dark_theme_active(state) ? TRUE : FALSE;
        if (set_attribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark)) != S_OK) {
            (void)set_attribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &dark, sizeof(dark));
        }
    }
    FreeLibrary(dwm);
}

static void apply_voice_scrollbar_theme(PlayerState *state)
{
    HMODULE ux_theme;
    SetWindowThemeProc set_window_theme;
    HRESULT result;

    if (state == NULL || state->voice_scrollbar == NULL) {
        return;
    }
    ux_theme = LoadLibraryA("uxtheme.dll");
    if (ux_theme == NULL) {
        return;
    }
    set_window_theme = (SetWindowThemeProc)GetProcAddress(
        ux_theme, "SetWindowTheme");
    if (set_window_theme != NULL) {
        result = set_window_theme(state->voice_scrollbar,
            is_dark_theme_active(state) ? L"DarkMode_Explorer" : L"Explorer",
            NULL);
        if (FAILED(result) && is_dark_theme_active(state)) {
            (void)set_window_theme(state->voice_scrollbar, L"Explorer", NULL);
        }
        SendMessageA(state->voice_scrollbar, WM_THEMECHANGED, 0, 0);
        RedrawWindow(state->voice_scrollbar, NULL, NULL,
            RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
    }
    FreeLibrary(ux_theme);
}

static void apply_menu_info_recursive(HMENU menu, const MENUINFO *info)
{
    int i;
    int count;

    if (menu == NULL || info == NULL) {
        return;
    }
    SetMenuInfo(menu, info);
    count = GetMenuItemCount(menu);
    for (i = 0; i < count; ++i) {
        HMENU sub_menu = GetSubMenu(menu, i);
        if (sub_menu != NULL) {
            apply_menu_info_recursive(sub_menu, info);
        }
    }
}

static void apply_menu_theme(HWND hwnd, PlayerState *state)
{
    HMENU menu;
    MENUINFO info;
    HBRUSH *brush_slot;

    if (hwnd == NULL || state == NULL) {
        return;
    }
    menu = GetMenu(hwnd);
    if (menu == NULL) {
        return;
    }
    brush_slot = (hwnd == state->playlist_hwnd) ? &state->playlist_menu_brush : &state->menu_brush;
    if (*brush_slot != NULL) {
        DeleteObject(*brush_slot);
        *brush_slot = NULL;
    }
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);
    info.fMask = MIM_BACKGROUND;
    if (is_dark_theme_active(state)) {
        *brush_slot = CreateSolidBrush(RGB(0, 0, 0));
    } else {
        *brush_slot = CreateSolidBrush(RGB(255, 255, 255));
    }
    info.hbrBack = *brush_slot;
    apply_menu_info_recursive(menu, &info);
    DrawMenuBar(hwnd);
}

static void apply_window_theme(HWND hwnd, PlayerState *state)
{
    apply_app_menu_dark_mode(is_dark_theme_active(state));
    apply_window_dark_title(hwnd, state);
    apply_menu_theme(hwnd, state);
    apply_voice_scrollbar_theme(state);
    if (hwnd != NULL && state != NULL && state->playlist_hwnd != NULL) {
        apply_window_dark_title(state->playlist_hwnd, state);
        apply_menu_theme(state->playlist_hwnd, state);
    }
    if (hwnd != NULL && state != NULL && state->timbre_hwnd != NULL) {
        apply_window_dark_title(state->timbre_hwnd, state);
    }
    if (hwnd != NULL && state != NULL && state->preview_hwnd != NULL) {
        apply_window_dark_title(state->preview_hwnd, state);
    }
}

static void fill_player_background(HDC hdc, const RECT *rect, const PlayerState *state)
{
    HBRUSH brush = (HBRUSH)GetStockObject(DC_BRUSH);

    if (hdc == NULL || rect == NULL) {
        return;
    }
    SetDCBrushColor(hdc, player_background_color(state));
    FillRect(hdc, rect, brush);
}

static int is_dark_owner_draw_button_id(unsigned id)
{
    return id == IDC_OPEN ||
        id == IDC_PLAY ||
        id == IDC_PAUSE ||
        id == IDC_STOP ||
        id == IDC_PLAYLIST_PREV ||
        id == IDC_PLAYLIST_NEXT ||
        id == IDC_PLAYLIST_PLAY ||
        id == IDC_PLAYLIST_STOP ||
        id == IDC_PLAYLIST_DELETE ||
        id == IDC_PLAYLIST_REPLACE ||
        id == IDC_PLAYLIST_APPEND ||
        id == IDC_TIMBRE_CLEAR ||
        id == IDC_TIMBRE_HIDE ||
        id == IDC_FONT_OK ||
        id == IDC_FONT_APPLY ||
        id == IDC_FONT_CANCEL ||
        id == IDC_COLOR_PICK ||
        id == IDC_COLOR_OK ||
        id == IDC_COLOR_CANCEL ||
        id == IDOK ||
        id == IDCANCEL;
}

static LRESULT themed_dialog_control_color(WPARAM wparam, const PlayerState *state)
{
    HDC hdc = (HDC)wparam;
    if (hdc != NULL && state != NULL && is_dark_theme_active(state)) {
        SetTextColor(hdc, player_text_color(state));
        SetBkColor(hdc, player_background_color(state));
        return (LRESULT)GetStockObject(BLACK_BRUSH);
    }
    return 0;
}

static int draw_header_percent_label(LPARAM lparam, const PlayerState *state)
{
    DRAWITEMSTRUCT *draw = (DRAWITEMSTRUCT *)lparam;
    RECT label_rect;
    RECT value_rect;
    SIZE value_size;
    const char *label;
    char value[16];
    HBRUSH background;
    HFONT font;
    HFONT old_font = NULL;
    COLORREF old_text;
    int old_bk;

    if (draw == NULL || state == NULL || draw->CtlType != ODT_STATIC ||
        (draw->CtlID != IDC_SPEED_LABEL && draw->CtlID != IDC_VOLUME_LABEL)) {
        return 0;
    }

    label = draw->CtlID == IDC_SPEED_LABEL ? "Speed" : "Vol";
    snprintf(value, sizeof(value), "%d%%",
        draw->CtlID == IDC_SPEED_LABEL ? state->speed_percent : state->volume_percent);
    background = is_dark_theme_active(state) ?
        (HBRUSH)GetStockObject(BLACK_BRUSH) : GetSysColorBrush(COLOR_BTNFACE);
    FillRect(draw->hDC, &draw->rcItem, background);

    font = (HFONT)SendMessageA(draw->hwndItem, WM_GETFONT, 0, 0);
    if (font != NULL) {
        old_font = (HFONT)SelectObject(draw->hDC, font);
    }
    old_text = SetTextColor(draw->hDC,
        is_dark_theme_active(state) ? RGB(255, 255, 255) : GetSysColor(COLOR_BTNTEXT));
    old_bk = SetBkMode(draw->hDC, TRANSPARENT);

    label_rect = draw->rcItem;
    DrawTextA(draw->hDC, label, -1, &label_rect,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    ZeroMemory(&value_size, sizeof(value_size));
    GetTextExtentPoint32A(draw->hDC, value, (int)strlen(value), &value_size);
    value_rect = draw->rcItem;
    value_rect.left = value_rect.right - value_size.cx - 2;
    if (value_rect.left < value_rect.right && value_rect.left < draw->rcItem.left) {
        value_rect.left = draw->rcItem.left;
    }
    FillRect(draw->hDC, &value_rect, background);
    value_rect.right -= 1;
    DrawTextA(draw->hDC, value, -1, &value_rect,
        DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    SetBkMode(draw->hDC, old_bk);
    SetTextColor(draw->hDC, old_text);
    if (old_font != NULL && old_font != HGDI_ERROR) {
        SelectObject(draw->hDC, old_font);
    }
    return 1;
}

static int draw_owner_button(LPARAM lparam, const PlayerState *state)
{
    DRAWITEMSTRUCT *draw = (DRAWITEMSTRUCT *)lparam;
    char text[64];
    HBRUSH brush;
    COLORREF old_text;
    int old_bk;
    UINT edge = EDGE_RAISED;
    UINT text_flags = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
    COLORREF bg;
    COLORREF fg;
    int latched = 0;

    if (draw == NULL || draw->CtlType != ODT_BUTTON || !is_dark_owner_draw_button_id((unsigned)draw->CtlID)) {
        return 0;
    }

    if (state != NULL && draw->CtlID == IDC_PLAYLIST_REPLACE) {
        latched = !state->playlist_append_mode;
    } else if (state != NULL && draw->CtlID == IDC_PLAYLIST_APPEND) {
        latched = state->playlist_append_mode;
    }

    if (latched && (draw->CtlID == IDC_PLAYLIST_REPLACE ||
        draw->CtlID == IDC_PLAYLIST_APPEND)) {
        bg = is_dark_theme_active(state) ? RGB(55, 55, 55) : RGB(210, 210, 210);
        fg = is_dark_theme_active(state) ? RGB(255, 255, 255) : RGB(0, 0, 0);
    } else if (latched && is_dark_theme_active(state)) {
        bg = RGB(35, 70, 105);
        fg = RGB(255, 255, 255);
    } else if (latched) {
        bg = GetSysColor(COLOR_HIGHLIGHT);
        fg = GetSysColor(COLOR_HIGHLIGHTTEXT);
    } else if (is_dark_theme_active(state)) {
        bg = RGB(0, 0, 0);
        fg = RGB(255, 255, 255);
    } else {
        bg = GetSysColor(COLOR_BTNFACE);
        fg = GetSysColor(COLOR_BTNTEXT);
    }
    if ((draw->itemState & ODS_SELECTED) || latched) {
        edge = EDGE_SUNKEN;
    }
    if (draw->itemState & ODS_DISABLED) {
        fg = GetSysColor(COLOR_GRAYTEXT);
    }

    brush = CreateSolidBrush(bg);
    if (brush != NULL) {
        FillRect(draw->hDC, &draw->rcItem, brush);
        DeleteObject(brush);
    }
    DrawEdge(draw->hDC, &draw->rcItem, edge, BF_RECT);
    GetWindowTextA(draw->hwndItem, text, sizeof(text));
    old_text = SetTextColor(draw->hDC, fg);
    old_bk = SetBkMode(draw->hDC, TRANSPARENT);
    if (draw->itemState & ODS_FOCUS) {
        RECT focus_rect = draw->rcItem;
        InflateRect(&focus_rect, -3, -3);
        DrawFocusRect(draw->hDC, &focus_rect);
    }
    DrawTextA(draw->hDC, text, -1, &draw->rcItem, text_flags);
    SetBkMode(draw->hDC, old_bk);
    SetTextColor(draw->hDC, old_text);
    return 1;
}

static void append_darkable_menu_separator(HMENU menu)
{
    AppendMenuA(menu, MF_OWNERDRAW | MF_DISABLED, 0, (LPCSTR)MENU_SEPARATOR_MAGIC);
}

static void append_darkable_menu_item(HMENU menu, UINT id, const char *label)
{
    AppendMenuA(menu, MF_OWNERDRAW, id, label);
}

static const OwnerMenuItemData g_menu_bar_file = { MENU_BAR_ITEM_MAGIC, "File", 1 };
static const OwnerMenuItemData g_menu_bar_settings = { MENU_BAR_ITEM_MAGIC, "Settings", 1 };
static const OwnerMenuItemData g_menu_bar_help = { MENU_BAR_ITEM_MAGIC, "Help", 1 };
static const OwnerMenuItemData g_menu_bar_playlist = { MENU_BAR_ITEM_MAGIC, "Playlist", 1 };

static const OwnerMenuItemData *owner_menu_data_from_item_data(ULONG_PTR item_data)
{
    const OwnerMenuItemData *data = (const OwnerMenuItemData *)item_data;

    if (data == &g_menu_bar_file ||
        data == &g_menu_bar_settings ||
        data == &g_menu_bar_help ||
        data == &g_menu_bar_playlist) {
        return data;
    }
    return NULL;
}

static void append_darkable_menu_popup(HMENU menu, HMENU popup, const char *label)
{
    AppendMenuA(menu, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)popup, label);
}

static void append_darkable_menu_bar_popup(HMENU menu, HMENU popup, const char *label)
{
    const OwnerMenuItemData *data = &g_menu_bar_file;

    if (strcmp(label, "Settings") == 0) {
        data = &g_menu_bar_settings;
    } else if (strcmp(label, "Help") == 0) {
        data = &g_menu_bar_help;
    } else if (strcmp(label, "Playlist") == 0) {
        data = &g_menu_bar_playlist;
    }
    AppendMenuA(menu, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)popup, (LPCSTR)data);
}

static const char *volume_menu_label_for_id(UINT id)
{
    static char labels[21][8];
    static int initialized = 0;
    unsigned i;

    if (!initialized) {
        for (i = 0; i <= 20; ++i) {
            snprintf(labels[i], sizeof(labels[i]), "%u%%", i * 10u);
        }
        initialized = 1;
    }
    if (id < IDM_VOLUME_0 || id > IDM_VOLUME_200) {
        return NULL;
    }
    return labels[id - IDM_VOLUME_0];
}

static int measure_owner_menu_item(LPARAM lparam)
{
    MEASUREITEMSTRUCT *measure = (MEASUREITEMSTRUCT *)lparam;
    const OwnerMenuItemData *data;
    const char *label;
    int menu_bar = 0;
    SIZE text_size;
    HDC hdc;
    HFONT old_font;

    if (measure == NULL || measure->CtlType != ODT_MENU) {
        return 0;
    }
    if ((ULONG_PTR)measure->itemData == MENU_SEPARATOR_MAGIC) {
        measure->itemHeight = 8;
        measure->itemWidth = 160;
        return 1;
    }
    data = owner_menu_data_from_item_data((ULONG_PTR)measure->itemData);
    if (data != NULL && data->magic == MENU_BAR_ITEM_MAGIC) {
        label = data->label;
        menu_bar = data->menu_bar;
    } else {
        label = (const char *)measure->itemData;
    }
    if ((label == NULL || label[0] == '\0') && measure->itemID >= IDM_VOLUME_0 && measure->itemID <= IDM_VOLUME_200) {
        label = volume_menu_label_for_id(measure->itemID);
    }
    if (label != NULL && label[0] != '\0') {
        measure->itemHeight = GetSystemMetrics(SM_CYMENU);
        if (menu_bar) {
            text_size.cx = (LONG)(strlen(label) * 8u);
            hdc = GetDC(NULL);
            if (hdc != NULL) {
                old_font = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
                GetTextExtentPoint32A(hdc, label, (int)strlen(label), &text_size);
                SelectObject(hdc, old_font);
                ReleaseDC(NULL, hdc);
            }
            measure->itemWidth = (UINT)(text_size.cx + 14);
        } else {
            measure->itemWidth = (UINT)(strlen(label) * 8u + 36u);
        }
        return 1;
    }
    return 0;
}

static int draw_owner_menu_item(LPARAM lparam, const PlayerState *state)
{
    DRAWITEMSTRUCT *draw = (DRAWITEMSTRUCT *)lparam;
    HBRUSH brush;
    COLORREF line;
    COLORREF text_color;
    const OwnerMenuItemData *data;
    const char *label;
    RECT line_rect;
    int y;
    int menu_bar = 0;
    int disabled;

    if (draw == NULL || draw->CtlType != ODT_MENU) {
        return 0;
    }

    if ((ULONG_PTR)draw->itemData == MENU_SEPARATOR_MAGIC) {
        brush = CreateSolidBrush(is_dark_theme_active(state) ? RGB(0, 0, 0) : RGB(255, 255, 255));
        if (brush != NULL) {
            FillRect(draw->hDC, &draw->rcItem, brush);
            DeleteObject(brush);
        }
        line = is_dark_theme_active(state) ? RGB(255, 255, 255) : GetSysColor(COLOR_3DSHADOW);
        y = draw->rcItem.top + ((draw->rcItem.bottom - draw->rcItem.top) / 2);
        SetDCBrushColor(draw->hDC, line);
        line_rect.left = draw->rcItem.left + 8;
        line_rect.top = y;
        line_rect.right = draw->rcItem.right - 8;
        line_rect.bottom = y + 1;
        FillRect(draw->hDC, &line_rect, (HBRUSH)GetStockObject(DC_BRUSH));
        return 1;
    }

    data = owner_menu_data_from_item_data((ULONG_PTR)draw->itemData);
    if (data != NULL && data->magic == MENU_BAR_ITEM_MAGIC) {
        label = data->label;
        menu_bar = data->menu_bar;
    } else {
        label = (const char *)draw->itemData;
    }
    if ((label == NULL || label[0] == '\0') && draw->itemID >= IDM_VOLUME_0 && draw->itemID <= IDM_VOLUME_200) {
        label = volume_menu_label_for_id(draw->itemID);
    }
    if (label == NULL || label[0] == '\0') {
        return 0;
    }
    disabled = (draw->itemState & (ODS_DISABLED | ODS_GRAYED)) != 0;
    brush = CreateSolidBrush(is_dark_theme_active(state) ?
        ((!disabled && (draw->itemState & ODS_SELECTED)) ?
            RGB(32, 32, 32) : RGB(0, 0, 0)) :
        ((!disabled && (draw->itemState & ODS_SELECTED)) ?
            GetSysColor(COLOR_HIGHLIGHT) : RGB(255, 255, 255)));
    if (brush != NULL) {
        FillRect(draw->hDC, &draw->rcItem, brush);
        DeleteObject(brush);
    }
    text_color = disabled ?
        (is_dark_theme_active(state) ? RGB(128, 128, 128) :
            GetSysColor(COLOR_GRAYTEXT)) :
        (is_dark_theme_active(state) ? RGB(255, 255, 255) :
            ((draw->itemState & ODS_SELECTED) ?
                GetSysColor(COLOR_HIGHLIGHTTEXT) :
                GetSysColor(COLOR_MENUTEXT)));
    SetTextColor(draw->hDC, text_color);
    SetBkMode(draw->hDC, TRANSPARENT);
    if (draw->itemState & ODS_CHECKED) {
        RECT check_rect = draw->rcItem;
        check_rect.right = check_rect.left + 22;
        DrawTextA(draw->hDC, "*", -1, &check_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    {
        RECT text_rect = draw->rcItem;
        if (menu_bar) {
            DrawTextA(draw->hDC, label, -1, &text_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        } else {
            text_rect.left += 24;
            DrawTextA(draw->hDC, label, -1, &text_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
    }
    return 1;
}

static LRESULT draw_speed_slider_custom(NMHDR *header, const PlayerState *state)
{
    NMCUSTOMDRAW *draw;
    HBRUSH brush;
    RECT rect;
    RECT thumb_rect;
    RECT channel_rect;

    if (header == NULL || state == NULL || header->hwndFrom != state->speed_slider || !is_dark_theme_active(state)) {
        return CDRF_DODEFAULT;
    }

    draw = (NMCUSTOMDRAW *)header;
    if (draw->dwDrawStage == CDDS_PREPAINT) {
        brush = CreateSolidBrush(RGB(0, 0, 0));
        if (brush != NULL) {
            FillRect(draw->hdc, &draw->rc, brush);
            DeleteObject(brush);
        }
        return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
    }
    if (draw->dwDrawStage == CDDS_POSTPAINT) {
        SendMessageA(state->speed_slider, TBM_GETCHANNELRECT, 0, (LPARAM)&channel_rect);
        SendMessageA(state->speed_slider, TBM_GETTHUMBRECT, 0, (LPARAM)&thumb_rect);
        brush = CreateSolidBrush(RGB(0, 0, 0));
        if (brush != NULL) {
            FillRect(draw->hdc, &draw->rc, brush);
            DeleteObject(brush);
        }
        brush = CreateSolidBrush(RGB(255, 255, 255));
        if (brush != NULL) {
            FillRect(draw->hdc, &channel_rect, brush);
            DeleteObject(brush);
        }
        brush = CreateSolidBrush(RGB(0, 0, 0));
        if (brush != NULL) {
            FillRect(draw->hdc, &thumb_rect, brush);
            DeleteObject(brush);
        }
        FrameRect(draw->hdc, &thumb_rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
        return CDRF_SKIPDEFAULT;
    }
    if (draw->dwDrawStage == CDDS_ITEMPREPAINT) {
        rect = draw->rc;
        if (draw->dwItemSpec == TBCD_CHANNEL) {
            brush = CreateSolidBrush(RGB(255, 255, 255));
            if (brush != NULL) {
                FillRect(draw->hdc, &rect, brush);
                DeleteObject(brush);
            }
            return CDRF_SKIPDEFAULT;
        }
        if (draw->dwItemSpec == TBCD_THUMB) {
            brush = CreateSolidBrush(RGB(0, 0, 0));
            if (brush != NULL) {
                FillRect(draw->hdc, &rect, brush);
                DeleteObject(brush);
            }
            FrameRect(draw->hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
            return CDRF_SKIPDEFAULT;
        }
        if (draw->dwItemSpec == TBCD_TICS) {
            return CDRF_SKIPDEFAULT;
        }
    }
    return CDRF_DODEFAULT;
}

static UINT timer_ms_for_performance_profile(int profile)
{
    switch (normalize_performance_profile(profile)) {
    case 0:
        return 33u;
    case 1:
        return 16u;
    default:
        return PLAYER_TIMER_MS;
    }
}

static void restart_display_timer(HWND hwnd, const PlayerState *state)
{
    UINT timer_ms = PLAYER_TIMER_MS;

    if (hwnd == NULL) {
        return;
    }
    if (state != NULL) {
        timer_ms = timer_ms_for_performance_profile(state->performance_profile);
    }

    KillTimer(hwnd, PLAYER_TIMER_ID);
    SetTimer(hwnd, PLAYER_TIMER_ID, timer_ms, NULL);
}

static void update_playlist_settings_menu_check(const PlayerState *state)
{
    HMENU menu;

    if (state == NULL || state->playlist_hwnd == NULL) {
        return;
    }
    menu = GetMenu(state->playlist_hwnd);
    if (menu == NULL) {
        return;
    }
    CheckMenuItem(menu, IDM_TAG_FADE_ENABLED,
        MF_BYCOMMAND | (state->tag_fade_enabled ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_PLAYLIST_ORDER_NEXT,
        MF_BYCOMMAND | (!state->playlist_single_track_mode &&
            !state->playlist_random_mode ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_PLAYLIST_ORDER_RANDOM,
        MF_BYCOMMAND | (!state->playlist_single_track_mode &&
            state->playlist_random_mode ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_PLAYLIST_ORDER_SINGLE,
        MF_BYCOMMAND | (state->playlist_single_track_mode ? MF_CHECKED : MF_UNCHECKED));
    DrawMenuBar(state->playlist_hwnd);
}

static void update_settings_menu_check(HWND hwnd, const PlayerState *state)
{
    HMENU menu;
    HMENU keyboard_menu;

    if (hwnd == NULL || state == NULL) {
        return;
    }

    menu = GetMenu(hwnd);
    if (menu == NULL) {
        return;
    }
    keyboard_menu = state->keyboard_submenu != NULL ?
        state->keyboard_submenu : menu;

    CheckMenuItem(menu, IDM_REVERB_ENABLED,
        MF_BYCOMMAND | (state->reverb_enabled ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_MAIN_ENABLED,
        MF_BYCOMMAND | (state->main_enabled ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_DEBUG_EDIT_CONTROLS,
        MF_BYCOMMAND | (state->debug_edit_controls ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_FRAME_ADVANCE,
        MF_BYCOMMAND | (state->frame_advance ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_PLAYBACK_ONLY,
        MF_BYCOMMAND | (state->playback_only ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_TAG_FADE_ENABLED,
        MF_BYCOMMAND | (state->tag_fade_enabled ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_TIMBRE_SCAN,
        MF_BYCOMMAND | (state->timbre_scan_enabled ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(keyboard_menu, IDM_KEYBOARD_VALUES,
        MF_BYCOMMAND | (normalize_psf1_display_mode(state->psf1_display_mode) ==
            PSF1_DISPLAY_VALUES ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(keyboard_menu, IDM_KEYBOARD_VALUES_AND_KEYS,
        MF_BYCOMMAND | (normalize_psf1_display_mode(state->psf1_display_mode) ==
            PSF1_DISPLAY_VALUES_KEYBOARD ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(keyboard_menu, IDM_KEYBOARD_KEYS_ONLY,
        MF_BYCOMMAND | (normalize_psf1_display_mode(state->psf1_display_mode) ==
            PSF1_DISPLAY_KEYBOARD_ONLY ? MF_CHECKED : MF_UNCHECKED));
    EnableMenuItem(keyboard_menu, IDM_KEYBOARD_VALUES,
        MF_BYCOMMAND | (state->psf_version == 0x01u ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(keyboard_menu, IDM_KEYBOARD_VALUES_AND_KEYS,
        MF_BYCOMMAND | (state->psf_version == 0x01u ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(keyboard_menu, IDM_KEYBOARD_KEYS_ONLY,
        MF_BYCOMMAND | (state->psf_version == 0x01u ? MF_ENABLED : MF_GRAYED));
    if (state->keyboard_parent_menu != NULL) {
        MENUITEMINFOA item;
        int should_attach = state->psf_version == 0x01u;

        ZeroMemory(&item, sizeof(item));
        item.cbSize = sizeof(item);
        item.fMask = MIIM_STATE | MIIM_SUBMENU;
        item.fState = should_attach ?
            MFS_ENABLED : (MFS_DISABLED | MFS_GRAYED);
        item.hSubMenu = should_attach ? state->keyboard_submenu : NULL;
        SetMenuItemInfoA(
            state->keyboard_parent_menu,
            state->keyboard_parent_position,
            TRUE,
            &item);
    }
    CheckMenuItem(menu, IDM_THEME_SYSTEM,
        MF_BYCOMMAND | (normalize_theme_mode(state->theme_mode) == THEME_SYSTEM ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_THEME_LIGHT,
        MF_BYCOMMAND | (normalize_theme_mode(state->theme_mode) == THEME_LIGHT ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_THEME_DARK,
        MF_BYCOMMAND | (normalize_theme_mode(state->theme_mode) == THEME_DARK ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_PERF_LOW, MF_BYCOMMAND | (state->performance_profile == 0 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_PERF_MIDDLE, MF_BYCOMMAND | (state->performance_profile == 1 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_PERF_HIGH, MF_BYCOMMAND | (state->performance_profile == 2 ? MF_CHECKED : MF_UNCHECKED));
    update_speed_menu_check(hwnd, state);
    CheckMenuItem(menu, IDM_ENV_COLOR_GREEN, MF_BYCOMMAND | (state->env_color_index == 0 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_ENV_COLOR_BLUE, MF_BYCOMMAND | (state->env_color_index == 1 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_ENV_COLOR_CYAN, MF_BYCOMMAND | (state->env_color_index == 2 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_ENV_COLOR_AMBER, MF_BYCOMMAND | (state->env_color_index == 3 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_ENV_COLOR_RED, MF_BYCOMMAND | (state->env_color_index == 4 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_ENV_COLOR_MAGENTA, MF_BYCOMMAND | (state->env_color_index == 5 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_ENV_COLOR_VIOLET, MF_BYCOMMAND | (state->env_color_index == 6 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_ENV_COLOR_WHITE, MF_BYCOMMAND | (state->env_color_index == 7 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_ENV_COLOR_CUSTOM, MF_BYCOMMAND | (state->env_color_index == CUSTOM_COLOR_INDEX ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_LR_COLOR_GREEN, MF_BYCOMMAND | (state->lr_color_index == 0 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_LR_COLOR_BLUE, MF_BYCOMMAND | (state->lr_color_index == 1 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_LR_COLOR_CYAN, MF_BYCOMMAND | (state->lr_color_index == 2 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_LR_COLOR_AMBER, MF_BYCOMMAND | (state->lr_color_index == 3 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_LR_COLOR_RED, MF_BYCOMMAND | (state->lr_color_index == 4 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_LR_COLOR_MAGENTA, MF_BYCOMMAND | (state->lr_color_index == 5 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_LR_COLOR_VIOLET, MF_BYCOMMAND | (state->lr_color_index == 6 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_LR_COLOR_WHITE, MF_BYCOMMAND | (state->lr_color_index == 7 ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_LR_COLOR_CUSTOM, MF_BYCOMMAND | (state->lr_color_index == CUSTOM_COLOR_INDEX ? MF_CHECKED : MF_UNCHECKED));
    {
        unsigned volume_percent;
        UINT checked_id = (UINT)(IDM_VOLUME_0 + (normalize_volume_percent(state->volume_percent) / 10));
        for (volume_percent = 0; volume_percent <= 200; volume_percent += 20) {
            UINT id = (UINT)(IDM_VOLUME_0 + (volume_percent / 10));
            CheckMenuItem(menu, id, MF_BYCOMMAND | (id == checked_id ? MF_CHECKED : MF_UNCHECKED));
        }
    }
    update_playlist_settings_menu_check(state);
    DrawMenuBar(hwnd);
}

static void set_tag_fade_enabled(HWND hwnd, PlayerState *state, int enabled)
{
    if (state == NULL) {
        return;
    }
    state->tag_fade_enabled = enabled ? 1 : 0;
    save_display_settings(state);
    if (state->hwnd != NULL) {
        update_settings_menu_check(state->hwnd, state);
    } else if (hwnd != NULL) {
        update_settings_menu_check(hwnd, state);
    }
    update_playlist_settings_menu_check(state);
}

static void set_playlist_order_mode(PlayerState *state, int single_track, int random_mode)
{
    if (state == NULL) {
        return;
    }
    state->playlist_single_track_mode = single_track ? 1 : 0;
    state->playlist_random_mode = !state->playlist_single_track_mode && random_mode ? 1 : 0;
    save_display_settings(state);
    update_playlist_settings_menu_check(state);
}

static void set_debug_edit_controls(HWND hwnd, PlayerState *state, int enabled)
{
    if (state == NULL) {
        return;
    }
    if (!enabled && state->debug_edit_controls) {
        restore_debug_edits_to_saved_values(hwnd, state);
    }
    state->debug_edit_controls = enabled ? 1 : 0;
    update_settings_menu_check(hwnd, state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void toggle_debug_edit_controls(HWND hwnd, PlayerState *state)
{
    if (state == NULL) {
        return;
    }
    set_debug_edit_controls(hwnd, state, !state->debug_edit_controls);
}

static void set_playback_only_mode(HWND hwnd, PlayerState *state, int enabled)
{
    if (state == NULL) {
        return;
    }
    state->playback_only = enabled ? 1 : 0;
    save_display_settings(state);
    update_settings_menu_check(hwnd, state);
    restart_display_timer(hwnd, state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void toggle_playback_only_mode(HWND hwnd, PlayerState *state)
{
    if (state == NULL) {
        return;
    }
    set_playback_only_mode(hwnd, state, !state->playback_only);
}

static void recreate_speed_slider(HWND hwnd, PlayerState *state)
{
    if (hwnd == NULL || state == NULL || state->speed_slider == NULL) {
        return;
    }
    DestroyWindow(state->speed_slider);
    state->speed_slider = CreateWindowA(TRACKBAR_CLASSA, "", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
        252, 1, 128, 26, hwnd, (HMENU)(UINT_PTR)IDC_SPEED, NULL, NULL);
    if (state->speed_slider != NULL) {
        SendMessageA(state->speed_slider, TBM_SETRANGE, TRUE, MAKELPARAM(10, 200));
        SendMessageA(state->speed_slider, TBM_SETTICFREQ, 10, 0);
        SendMessageA(state->speed_slider, TBM_SETLINESIZE, 0, 10);
        SendMessageA(state->speed_slider, TBM_SETPAGESIZE, 0, 10);
        SendMessageA(state->speed_slider, TBM_SETPOS, TRUE, state->speed_percent);
        if (state->ui_font != NULL) {
            SendMessageA(state->speed_slider, WM_SETFONT, (WPARAM)state->ui_font, TRUE);
        }
    }
}

static void set_theme_mode(HWND hwnd, PlayerState *state, int mode)
{
    if (state == NULL) {
        return;
    }
    store_active_gauge_colors_to_theme(state);
    state->theme_mode = normalize_theme_mode(mode);
    sync_active_gauge_colors_from_theme(state);
    save_display_settings(state);
    apply_window_theme(hwnd, state);
    update_settings_menu_check(hwnd, state);
    recreate_speed_slider(hwnd, state);
    layout_player_controls(hwnd, state);
    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
    if (state->playlist_hwnd != NULL) {
        RedrawWindow(state->playlist_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
    }
    if (state->timbre_hwnd != NULL) {
        RedrawWindow(state->timbre_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
    }
    if (state->preview_hwnd != NULL) {
        RedrawWindow(state->preview_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
    }
}

static void set_frame_advance_mode(HWND hwnd, PlayerState *state, int enabled)
{
    if (state == NULL) {
        return;
    }
    lock_state(state);
    state->frame_advance = enabled ? 1 : 0;
    if (state->frame_advance) {
        state->frame_live = state->live;
        state->frame_live_valid = 1;
        state->frame_capture_until = 0;
        state->frame_step_request = 0;
        state->frame_audio_transition = 0;
    } else {
        state->frame_live_valid = 0;
        state->frame_capture_until = 0;
        state->frame_step_request = 0;
        state->frame_audio_transition = 0;
    }
    unlock_state(state);
    if (enabled) {
        psf2log_set_imported_frame_advance_mode(1);
        psf2log_set_imported_adsr_freeze(1);
        set_status(state, "Frame advance");
    } else {
        psf2log_set_imported_frame_advance_mode(0);
        psf2log_set_imported_adsr_freeze(0);
        apply_effective_speed_percent(state);
    }
    update_settings_menu_check(hwnd, state);
    update_time_label(state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void toggle_frame_advance_mode(HWND hwnd, PlayerState *state)
{
    if (state == NULL) {
        return;
    }
    set_frame_advance_mode(hwnd, state, !state->frame_advance);
}

static int debug_edit_controls_active(PlayerState *state)
{
    int active = 0;

    if (state == NULL || !state->debug_edit_controls || state->playback_only) {
        return 0;
    }

    lock_state(state);
    active = state->playing && state->core != NULL;
    unlock_state(state);
    return active;
}

static void update_main_reverb_controls(PlayerState *state)
{
    int main_enabled;
    int reverb_enabled;

    if (state == NULL) {
        return;
    }

    lock_state(state);
    main_enabled = state->main_enabled;
    reverb_enabled = state->reverb_enabled;
    unlock_state(state);

    if (state->main_check != NULL) {
        SendMessageA(state->main_check, BM_SETCHECK, main_enabled ? BST_CHECKED : BST_UNCHECKED, 0);
    }
    if (state->reverb_check != NULL) {
        SendMessageA(state->reverb_check, BM_SETCHECK, reverb_enabled ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

static uint32_t voice_timbre_key(const Spu2LogVoiceSnapshot *voice)
{
    uint32_t start_addr;
    uint32_t flag_class;

    if (voice == NULL) {
        return 0;
    }
    start_addr = voice->ssa & 0x000fffffu;
    if (start_addr == 0) {
        return 0;
    }
    flag_class = voice->flags & (SPU2LOG_VOICE_NOISE | SPU2LOG_VOICE_PMOD);
    return 0x80000000u | ((flag_class & 0x0cu) << 18) | start_addr;
}

static uint32_t voice_timbre_loop_addr(const Spu2LogVoiceSnapshot *voice)
{
    uint32_t start_addr;
    uint32_t loop_addr;
    uint32_t sample_end;

    if (voice == NULL) {
        return 0;
    }
    start_addr = voice->ssa & 0x000fffffu;
    loop_addr = voice->lsa & 0x000fffffu;
    sample_end = voice->sample_end & 0x000fffffu;
    if (loop_addr < start_addr ||
        (sample_end > start_addr && loop_addr >= sample_end)) {
        return start_addr;
    }
    return loop_addr != 0 ? loop_addr : start_addr;
}

static uint32_t voice_timbre_flag_class(const Spu2LogVoiceSnapshot *voice)
{
    uint32_t flags;

    if (voice == NULL) {
        return 0;
    }
    flags = voice->flags;
    return flags & (SPU2LOG_VOICE_NOISE | SPU2LOG_VOICE_PMOD);
}

static int psf1_find_tuning_locked(const PlayerState *state, uint32_t key)
{
    unsigned i;

    if (state == NULL || key == 0) {
        return -1;
    }
    for (i = 0; i < state->psf1_tuning_cache_count; ++i) {
        if (state->psf1_tuning_cache[i].key == key) {
            return (int)i;
        }
    }
    return -1;
}

static void psf1_queue_tuning_locked(PlayerState *state, const Spu2LogVoiceSnapshot *voice)
{
    Psf1TuningEntry *entry;
    uint32_t key;
    uint32_t ssa;
    uint32_t lsa;
    int existing;

    if (state == NULL || voice == NULL || state->psf_version != 0x01u ||
        voice->voice >= 24u || voice->envx == 0 ||
        (voice->flags & SPU2LOG_VOICE_NOISE) != 0) {
        return;
    }
    key = voice_timbre_key(voice);
    if (key == 0) {
        return;
    }
    ssa = voice->ssa & 0x000fffffu;
    lsa = voice->lsa & 0x000fffffu;
    if (lsa == 0u) {
        lsa = ssa;
    }
    state->psf1_voice_timbre_key[voice->voice] = key;
    existing = psf1_find_tuning_locked(state, key);
    if (existing >= 0) {
        int previous_backward;
        int current_backward;

        entry = &state->psf1_tuning_cache[existing];
        /* The loop pointer can still contain the driver's previous/default
           value at the first snapshot. Prefer a settled in-sample loop and
           never let another voice move the shared cache back to a stale
           backward pointer. */
        previous_backward = entry->lsa < entry->ssa;
        current_backward = lsa < ssa;
        if (lsa != entry->lsa && previous_backward &&
            (!current_backward || lsa > entry->lsa)) {
            entry->lsa = lsa;
            if (entry->status == PSF1_TUNING_READY) {
                entry->status = PSF1_TUNING_REFRESH_PENDING;
            }
        }
        return;
    }
    if (state->psf1_tuning_cache_count >= PSF1_TUNING_CACHE_SIZE) {
        return;
    }
    entry = &state->psf1_tuning_cache[state->psf1_tuning_cache_count++];
    ZeroMemory(entry, sizeof(*entry));
    entry->key = key;
    entry->ssa = ssa;
    entry->lsa = lsa;
}

static int timbre_solo_has_key_locked(const PlayerState *state, uint32_t key)
{
    unsigned i;

    if (state == NULL || key == 0) {
        return 0;
    }
    for (i = 0; i < state->timbre_solo_key_count; ++i) {
        if (state->timbre_solo_keys[i] == key) {
            return 1;
        }
    }
    return 0;
}

static int timbre_solo_add_key_locked(PlayerState *state, uint32_t key, uint32_t ssa, uint32_t lsa, uint32_t flags)
{
    if (state == NULL || key == 0 || timbre_solo_has_key_locked(state, key)) {
        return 0;
    }
    if (state->timbre_solo_key_count >= TIMBRE_SOLO_MAX_KEYS) {
        return 0;
    }
    state->timbre_solo_keys[state->timbre_solo_key_count] = key;
    state->timbre_solo_ssa[state->timbre_solo_key_count] = ssa & 0x000fffffu;
    state->timbre_solo_lsa[state->timbre_solo_key_count] = lsa & 0x000fffffu;
    state->timbre_solo_flags[state->timbre_solo_key_count] =
        flags & (SPU2LOG_VOICE_NOISE | SPU2LOG_VOICE_PMOD);
    state->timbre_solo_key_count++;
    return 1;
}

static int timbre_solo_remove_key_locked(PlayerState *state, uint32_t key)
{
    unsigned i;

    if (state == NULL || key == 0) {
        return 0;
    }
    for (i = 0; i < state->timbre_solo_key_count; ++i) {
        if (state->timbre_solo_keys[i] == key) {
            unsigned last = state->timbre_solo_key_count - 1u;

            state->timbre_solo_keys[i] = state->timbre_solo_keys[last];
            state->timbre_solo_ssa[i] = state->timbre_solo_ssa[last];
            state->timbre_solo_lsa[i] = state->timbre_solo_lsa[last];
            state->timbre_solo_flags[i] = state->timbre_solo_flags[last];
            state->timbre_solo_keys[last] = 0;
            state->timbre_solo_ssa[last] = 0;
            state->timbre_solo_lsa[last] = 0;
            state->timbre_solo_flags[last] = 0;
            state->timbre_solo_key_count = last;
            return 1;
        }
    }
    return 0;
}

static int timbre_list_group_is_selected_locked(const PlayerState *state, unsigned index)
{
    unsigned k;

    if (state == NULL || index >= state->timbre_list_count ||
        state->timbre_list_key_count[index] == 0 || !state->timbre_solo_enabled) {
        return 0;
    }
    for (k = 0; k < state->timbre_list_key_count[index]; ++k) {
        if (!timbre_solo_has_key_locked(state, state->timbre_list_keys[index][k])) {
            return 0;
        }
    }
    return 1;
}

static void reset_timbre_solo_locked(PlayerState *state)
{
    if (state == NULL) {
        return;
    }
    state->timbre_solo_enabled = 0;
    state->timbre_solo_key_count = 0;
    state->timbre_solo_adsr1 = 0;
    state->timbre_solo_adsr2 = 0;
    state->timbre_solo_flag_class = 0;
    ZeroMemory(state->timbre_solo_keys, sizeof(state->timbre_solo_keys));
    ZeroMemory(state->timbre_solo_ssa, sizeof(state->timbre_solo_ssa));
    ZeroMemory(state->timbre_solo_lsa, sizeof(state->timbre_solo_lsa));
    ZeroMemory(state->timbre_solo_flags, sizeof(state->timbre_solo_flags));
}

static void preview_stop_all_locked(PlayerState *state)
{
    if (state == NULL) {
        return;
    }
    ZeroMemory(state->preview_voices, sizeof(state->preview_voices));
    ZeroMemory(state->preview_key_down, sizeof(state->preview_key_down));
    ZeroMemory(state->preview_reverb_l, sizeof(state->preview_reverb_l));
    ZeroMemory(state->preview_reverb_r, sizeof(state->preview_reverb_r));
    state->preview_reverb_pos = 0;
    state->preview_active_note = -1;
}

static void preview_handoff_all_locked(PlayerState *state)
{
    unsigned i;

    if (state == NULL) {
        return;
    }
    for (i = 0; i < PREVIEW_MAX_VOICES; ++i) {
        if (state->preview_voices[i].active) {
            state->preview_voices[i].handoff_frames = PREVIEW_HANDOFF_FRAMES;
        }
    }
    ZeroMemory(state->preview_key_down, sizeof(state->preview_key_down));
    state->preview_active_note = -1;
}

static void preview_handoff_released_locked(PlayerState *state, int restarting_note)
{
    unsigned i;

    if (state == NULL) {
        return;
    }
    for (i = 0; i < PREVIEW_MAX_VOICES; ++i) {
        PreviewVoice *voice = &state->preview_voices[i];

        if (voice->active &&
            (voice->releasing || voice->midi_note == restarting_note ||
             voice->midi_note < 0 || voice->midi_note >= 128 ||
             !state->preview_key_down[voice->midi_note])) {
            voice->handoff_frames = PREVIEW_HANDOFF_FRAMES;
        }
    }
}

static void preview_free_samples_locked(PlayerState *state)
{
    unsigned i;

    if (state == NULL) {
        return;
    }
    preview_stop_all_locked(state);
    for (i = 0; i < state->preview_sample_count; ++i) {
        free(state->preview_samples[i].pcm);
        state->preview_samples[i].pcm = NULL;
    }
    ZeroMemory(state->preview_samples, sizeof(state->preview_samples));
    ZeroMemory(state->preview_selected_samples, sizeof(state->preview_selected_samples));
    state->preview_sample_count = 0;
    state->preview_selected_sample_count = 0;
}

static int16_t *preview_decode_adpcm(
    const uint8_t *data,
    uint32_t length,
    uint32_t loop_offset,
    uint32_t *out_frames,
    uint32_t *out_loop_frame)
{
    static const int coefficients[5][2] = {
        {0, 0}, {60, 0}, {115, -52}, {98, -55}, {122, -60}
    };
    uint32_t blocks;
    uint32_t block;
    uint32_t frame = 0;
    uint32_t flagged_loop_offset = 0xffffffffu;
    int history1 = 0;
    int history2 = 0;
    int16_t *pcm;

    if (out_frames != NULL) {
        *out_frames = 0;
    }
    if (out_loop_frame != NULL) {
        *out_loop_frame = 0;
    }
    if (data == NULL || length < 16u) {
        return NULL;
    }
    blocks = length / 16u;
    pcm = (int16_t *)malloc((size_t)blocks * 28u * sizeof(*pcm));
    if (pcm == NULL) {
        return NULL;
    }
    for (block = 0; block < blocks; ++block) {
        const uint8_t *source = data + block * 16u;
        unsigned predictor = source[0] >> 4;
        unsigned shift = source[0] & 0x0fu;
        unsigned byte_index;

        if ((source[1] & 4u) != 0 && flagged_loop_offset == 0xffffffffu) {
            flagged_loop_offset = block * 16u;
        }

        if (predictor > 4u) {
            predictor = 0;
        }
        for (byte_index = 2u; byte_index < 16u; ++byte_index) {
            unsigned nibble_index;
            for (nibble_index = 0; nibble_index < 2u; ++nibble_index) {
                int nibble = nibble_index == 0
                    ? (source[byte_index] & 0x0f)
                    : (source[byte_index] >> 4);
                int sample;

                if (nibble >= 8) {
                    nibble -= 16;
                }
                sample = (nibble << 12) >> shift;
                sample += ((history1 * coefficients[predictor][0]) +
                    (history2 * coefficients[predictor][1]) + 32) >> 6;
                if (sample > 32767) {
                    sample = 32767;
                } else if (sample < -32768) {
                    sample = -32768;
                }
                history2 = history1;
                history1 = sample;
                pcm[frame++] = (int16_t)sample;
            }
        }
    }
    if (out_frames != NULL) {
        *out_frames = frame;
    }
    if (flagged_loop_offset < length) {
        loop_offset = flagged_loop_offset;
    }
    if (out_loop_frame != NULL && loop_offset < length) {
        *out_loop_frame = (loop_offset / 16u) * 28u;
    }
    return pcm;
}

static int preview_find_sample_locked(const PlayerState *state, uint32_t key)
{
    unsigned i;

    if (state == NULL || key == 0) {
        return -1;
    }
    for (i = 0; i < state->preview_sample_count; ++i) {
        if (state->preview_samples[i].key == key) {
            return (int)i;
        }
    }
    return -1;
}

static void preview_cache_sample_locked(
    PlayerState *state,
    const Spu2LogVoiceSnapshot *voice,
    uint32_t key,
    uint32_t loop_addr)
{
    PreviewSample *sample;
    uint8_t *encoded;
    uint32_t length;
    uint32_t loop_offset = 0;
    uint32_t end_flags = 0;
    uint32_t decoded_frames = 0;
    uint32_t loop_frame = 0;
    int found;

    if (state == NULL || voice == NULL || state->core == NULL || key == 0) {
        return;
    }
    found = preview_find_sample_locked(state, key);
    if (found >= 0) {
        sample = &state->preview_samples[found];
        if (sample->pitch_min == 0 || voice->pitch < sample->pitch_min) {
            sample->pitch_min = voice->pitch;
        }
        if (voice->pitch > sample->pitch_max) {
            sample->pitch_max = voice->pitch;
        }
        sample->flags |= voice->flags;
        return;
    }
    if (state->preview_sample_count >= PREVIEW_MAX_CACHED_SAMPLES) {
        return;
    }
    length = psf2log_copy_imported_sample(
        state->core,
        voice->ssa & 0x000fffffu,
        loop_addr,
        NULL,
        0,
        &loop_offset,
        &end_flags);
    if (length < 16u || length > PREVIEW_MAX_ADPCM_BYTES) {
        return;
    }
    encoded = (uint8_t *)malloc(length);
    if (encoded == NULL) {
        return;
    }
    if (psf2log_copy_imported_sample(
            state->core,
            voice->ssa & 0x000fffffu,
            loop_addr,
            encoded,
            length,
            &loop_offset,
            &end_flags) != length) {
        free(encoded);
        return;
    }
    sample = &state->preview_samples[state->preview_sample_count];
    sample->pcm = preview_decode_adpcm(
        encoded,
        length,
        loop_offset,
        &decoded_frames,
        &loop_frame);
    free(encoded);
    if (sample->pcm == NULL || decoded_frames == 0) {
        free(sample->pcm);
        ZeroMemory(sample, sizeof(*sample));
        return;
    }
    sample->key = key;
    sample->ssa = voice->ssa & 0x000fffffu;
    sample->lsa = loop_addr;
    sample->flags = voice->flags;
    sample->core = voice->core < 2u ? voice->core : 0u;
    sample->vol_l = voice->vol_l;
    sample->vol_r = voice->vol_r;
    sample->reference_pitch = voice->pitch != 0 ? voice->pitch : 0x1000u;
    sample->pitch_min = voice->pitch;
    sample->pitch_max = voice->pitch;
    sample->noise_clock = voice->noise_clock;
    sample->pcm_frames = decoded_frames;
    sample->loop_frame = loop_frame < decoded_frames ? loop_frame : 0;
    sample->loop_enabled = ((end_flags & 7u) == 3u && sample->loop_frame < decoded_frames) ? 1u : 0u;
    state->preview_sample_count++;
}

static void reset_timbre_list_locked(PlayerState *state)
{
    if (state == NULL) {
        return;
    }
    state->timbre_list_locked = 0;
    state->timbre_list_count = 0;
    ZeroMemory(state->timbre_list_key_count, sizeof(state->timbre_list_key_count));
    ZeroMemory(state->timbre_list_keys, sizeof(state->timbre_list_keys));
    ZeroMemory(state->timbre_list_ssa_keys, sizeof(state->timbre_list_ssa_keys));
    ZeroMemory(state->timbre_list_lsa_keys, sizeof(state->timbre_list_lsa_keys));
    ZeroMemory(state->timbre_list_end_keys, sizeof(state->timbre_list_end_keys));
    ZeroMemory(state->timbre_list_pitch_min, sizeof(state->timbre_list_pitch_min));
    ZeroMemory(state->timbre_list_pitch_max, sizeof(state->timbre_list_pitch_max));
    ZeroMemory(state->timbre_list_adsr1, sizeof(state->timbre_list_adsr1));
    ZeroMemory(state->timbre_list_adsr2, sizeof(state->timbre_list_adsr2));
    ZeroMemory(state->timbre_list_flags, sizeof(state->timbre_list_flags));
    ZeroMemory(state->timbre_list_ssa, sizeof(state->timbre_list_ssa));
    ZeroMemory(state->timbre_list_lsa, sizeof(state->timbre_list_lsa));
    ZeroMemory(state->timbre_list_is_percussion, sizeof(state->timbre_list_is_percussion));
    ZeroMemory(state->timbre_list_expanded, sizeof(state->timbre_list_expanded));
}

static int timbre_list_find_key_locked(const PlayerState *state, uint32_t key, unsigned *out_sample)
{
    unsigned i;
    unsigned k;

    if (state == NULL || key == 0) {
        return -1;
    }
    for (i = 0; i < state->timbre_list_count; ++i) {
        for (k = 0; k < state->timbre_list_key_count[i]; ++k) {
            if (state->timbre_list_keys[i][k] == key) {
                if (out_sample != NULL) {
                    *out_sample = k;
                }
                return (int)i;
            }
        }
    }
    return -1;
}

static int timbre_list_find_group_locked(const PlayerState *state, const Spu2LogVoiceSnapshot *voice)
{
    uint16_t adsr1;
    uint16_t adsr2;
    uint32_t flags;
    unsigned i;

    if (state == NULL || voice == NULL) {
        return -1;
    }
    adsr1 = voice->adsr1;
    adsr2 = voice->adsr2;
    flags = voice_timbre_flag_class(voice);
    for (i = 0; i < state->timbre_list_count; ++i) {
        if (state->timbre_list_adsr1[i] == adsr1 &&
            state->timbre_list_adsr2[i] == adsr2 &&
            state->timbre_list_flags[i] == flags &&
            state->timbre_list_key_count[i] < TIMBRE_SOLO_MAX_KEYS) {
            return (int)i;
        }
    }
    return -1;
}

static int timbre_list_add_voice_locked(PlayerState *state, const Spu2LogVoiceSnapshot *voice)
{
    uint32_t key;
    uint32_t loop_addr;
    int found;
    unsigned index;
    unsigned sample = 0;

    if (state == NULL || voice == NULL || !should_hold_voice_for_display(voice)) {
        return 0;
    }
    /* The PS1 driver briefly keys its null/silence entry while initializing. */
    if (state->psf_version == 0x01 &&
        (voice->ssa & 0x000fffffu) == 0x0200u &&
        voice->adsr1 == 0 && voice->adsr2 == 0) {
        return 0;
    }
    if (state->psf_version == 0x02u &&
        (voice->sample_end <= (voice->ssa & 0x000fffffu) ||
         (voice->adsr1 == 0 && voice->adsr2 == 0))) {
        return 0;
    }
    key = voice_timbre_key(voice);
    if (key == 0) {
        return 0;
    }
    loop_addr = voice_timbre_loop_addr(voice);
    preview_cache_sample_locked(state, voice, key, loop_addr);
    found = timbre_list_find_key_locked(state, key, &sample);
    if (found >= 0) {
        uint16_t pitch = voice->pitch;

        index = (unsigned)found;
        if (state->timbre_list_pitch_min[index][sample] == 0 ||
            pitch < state->timbre_list_pitch_min[index][sample]) {
            state->timbre_list_pitch_min[index][sample] = pitch;
        }
        if (pitch > state->timbre_list_pitch_max[index][sample]) {
            state->timbre_list_pitch_max[index][sample] = pitch;
        }
        return 0;
    }
    found = timbre_list_find_group_locked(state, voice);
    if (found >= 0) {
        index = (unsigned)found;
        if (state->timbre_list_key_count[index] >= TIMBRE_SOLO_MAX_KEYS) {
            return 0;
        }
    } else {
        if (state->timbre_list_count >= TIMBRE_LIST_MAX_ITEMS) {
            return 0;
        }
        index = state->timbre_list_count++;
        state->timbre_list_adsr1[index] = voice->adsr1;
        state->timbre_list_adsr2[index] = voice->adsr2;
        state->timbre_list_flags[index] = voice_timbre_flag_class(voice);
        state->timbre_list_ssa[index] = voice->ssa;
        state->timbre_list_lsa[index] = loop_addr;
    }
    sample = state->timbre_list_key_count[index];
    state->timbre_list_keys[index][sample] = key;
    state->timbre_list_ssa_keys[index][sample] = voice->ssa & 0x000fffffu;
    state->timbre_list_lsa_keys[index][sample] = loop_addr;
    state->timbre_list_end_keys[index][sample] = voice->sample_end & 0x000fffffu;
    state->timbre_list_pitch_min[index][sample] = voice->pitch;
    state->timbre_list_pitch_max[index][sample] = voice->pitch;
    state->timbre_list_key_count[index]++;
    return 1;
}

typedef struct TimbreCandidate {
    uint32_t key;
    uint32_t ssa;
    uint32_t lsa;
    uint32_t sample_end;
    uint16_t adsr1;
    uint16_t adsr2;
    uint16_t pitch_min;
    uint16_t pitch_max;
    uint32_t flags;
} TimbreCandidate;

static uint32_t timbre_candidate_extent(const TimbreCandidate *item)
{
    uint32_t extent;

    if (item == NULL) {
        return 0;
    }
    extent = item->sample_end;
    if (extent < item->ssa) {
        extent = item->ssa;
    }
    if (extent < item->lsa) {
        extent = item->lsa;
    }
    return extent;
}

static int timbre_candidate_has_narrow_pitch(const TimbreCandidate *item)
{
    if (item == NULL || item->pitch_min == 0 || item->pitch_max < item->pitch_min) {
        return 0;
    }
    return (unsigned)(item->pitch_max - item->pitch_min) <= TIMBRE_PERCUSSION_MAX_PITCH_SPAN;
}

static int compare_timbre_candidate(const void *left_value, const void *right_value)
{
    const TimbreCandidate *left = (const TimbreCandidate *)left_value;
    const TimbreCandidate *right = (const TimbreCandidate *)right_value;

    if (left->ssa != right->ssa) {
        return left->ssa < right->ssa ? -1 : 1;
    }
    if (left->lsa != right->lsa) {
        return left->lsa < right->lsa ? -1 : 1;
    }
    if (left->adsr1 != right->adsr1) {
        return left->adsr1 < right->adsr1 ? -1 : 1;
    }
    if (left->adsr2 != right->adsr2) {
        return left->adsr2 < right->adsr2 ? -1 : 1;
    }
    return left->flags < right->flags ? -1 : left->flags > right->flags ? 1 : 0;
}

static void timbre_list_regroup_locked(PlayerState *state)
{
    TimbreCandidate *items;
    uint8_t *percussion;
    unsigned item_count = 0;
    unsigned old_group;
    unsigned i;
    unsigned percussion_count = 0;
    uint32_t previous_extent = 0;

    if (state == NULL || state->timbre_list_count == 0) {
        return;
    }
    items = (TimbreCandidate *)calloc(
        TIMBRE_LIST_MAX_ITEMS * TIMBRE_SOLO_MAX_KEYS,
        sizeof(*items));
    if (items == NULL) {
        return;
    }
    for (old_group = 0; old_group < state->timbre_list_count; ++old_group) {
        unsigned sample;
        for (sample = 0; sample < state->timbre_list_key_count[old_group]; ++sample) {
            TimbreCandidate *item;
            if (item_count >= TIMBRE_LIST_MAX_ITEMS * TIMBRE_SOLO_MAX_KEYS) {
                break;
            }
            item = &items[item_count++];
            item->key = state->timbre_list_keys[old_group][sample];
            item->ssa = state->timbre_list_ssa_keys[old_group][sample];
            item->lsa = state->timbre_list_lsa_keys[old_group][sample];
            item->sample_end = state->timbre_list_end_keys[old_group][sample];
            item->adsr1 = state->timbre_list_adsr1[old_group];
            item->adsr2 = state->timbre_list_adsr2[old_group];
            item->pitch_min = state->timbre_list_pitch_min[old_group][sample];
            item->pitch_max = state->timbre_list_pitch_max[old_group][sample];
            item->flags = state->timbre_list_flags[old_group];
        }
    }
    qsort(items, item_count, sizeof(*items), compare_timbre_candidate);
    percussion = (uint8_t *)calloc(item_count, sizeof(*percussion));

    reset_timbre_list_locked(state);
    if (percussion != NULL) {
        unsigned run_start = 0;

        while (run_start < item_count) {
            unsigned run_end = run_start + 1u;
            unsigned one_shot_count = 0;
            unsigned narrow_anchor_count = 0;
            unsigned distinct_adsr = 0;
            uint32_t adsr_pairs[TIMBRE_SOLO_MAX_KEYS];
            unsigned j;

            ZeroMemory(adsr_pairs, sizeof(adsr_pairs));
            while (run_end < item_count &&
                items[run_end].ssa <= items[run_end - 1u].ssa + TIMBRE_PERCUSSION_MAX_SAMPLE_GAP) {
                run_end++;
            }
            for (j = run_start; j < run_end; ++j) {
                unsigned a;
                uint32_t adsr_pair;
                int seen = 0;

                if (items[j].ssa != items[j].lsa) {
                    continue;
                }
                one_shot_count++;
                if (timbre_candidate_has_narrow_pitch(&items[j])) {
                    narrow_anchor_count++;
                }
                adsr_pair = ((uint32_t)items[j].adsr1 << 16) | items[j].adsr2;
                for (a = 0; a < distinct_adsr; ++a) {
                    if (adsr_pairs[a] == adsr_pair) {
                        seen = 1;
                        break;
                    }
                }
                if (!seen && distinct_adsr < TIMBRE_SOLO_MAX_KEYS) {
                    adsr_pairs[distinct_adsr++] = adsr_pair;
                }
            }
            if (one_shot_count >= TIMBRE_PERCUSSION_MIN_SAMPLES &&
                narrow_anchor_count >= 2u &&
                (distinct_adsr >= 2u || one_shot_count >= 5u)) {
                for (j = run_start; j < run_end; ++j) {
                    if (items[j].ssa == items[j].lsa && timbre_candidate_has_narrow_pitch(&items[j])) {
                        percussion[j] = 1;
                        percussion_count++;
                    }
                }
                for (j = run_start; j < run_end; ++j) {
                    unsigned k;

                    if (percussion[j]) {
                        continue;
                    }
                    for (k = run_start; k < run_end; ++k) {
                        uint32_t distance;

                        if (!percussion[k]) {
                            continue;
                        }
                        distance = items[j].ssa > items[k].ssa
                            ? items[j].ssa - items[k].ssa
                            : items[k].ssa - items[j].ssa;
                        if (distance <= TIMBRE_GROUP_MAX_SAMPLE_GAP &&
                            (items[j].ssa == items[j].lsa ||
                             timbre_candidate_has_narrow_pitch(&items[j]))) {
                            percussion[j] = 1;
                            percussion_count++;
                            break;
                        }
                    }
                }
            }
            run_start = run_end;
        }
    }
    if (percussion_count >= TIMBRE_PERCUSSION_MIN_SAMPLES) {
        unsigned group = 0;

        for (i = 0; i < item_count; ++i) {
            TimbreCandidate *item = &items[i];
            unsigned key_index;

            if (!percussion[i]) {
                continue;
            }
            if (state->timbre_list_count == 0 ||
                state->timbre_list_key_count[group] >= TIMBRE_SOLO_MAX_KEYS) {
                if (state->timbre_list_count >= TIMBRE_LIST_MAX_ITEMS) {
                    break;
                }
                group = state->timbre_list_count++;
                state->timbre_list_adsr1[group] = item->adsr1;
                state->timbre_list_adsr2[group] = item->adsr2;
                state->timbre_list_flags[group] = item->flags;
                state->timbre_list_ssa[group] = item->ssa;
                state->timbre_list_lsa[group] = item->lsa;
                state->timbre_list_is_percussion[group] = 1;
            }
            key_index = state->timbre_list_key_count[group]++;
            state->timbre_list_keys[group][key_index] = item->key;
            state->timbre_list_ssa_keys[group][key_index] = item->ssa;
            state->timbre_list_lsa_keys[group][key_index] = item->lsa;
            state->timbre_list_end_keys[group][key_index] = item->sample_end;
            state->timbre_list_pitch_min[group][key_index] = item->pitch_min;
            state->timbre_list_pitch_max[group][key_index] = item->pitch_max;
        }
    }
    for (i = 0; i < item_count; ++i) {
        TimbreCandidate *item = &items[i];
        unsigned group;
        uint32_t group_gap = state->psf_version == 0x02u
            ? TIMBRE_PS2_GROUP_MAX_SAMPLE_GAP
            : TIMBRE_GROUP_MAX_SAMPLE_GAP;
        int start_new_group = 1;

        if (percussion_count >= TIMBRE_PERCUSSION_MIN_SAMPLES && percussion[i]) {
            continue;
        }
        if (state->timbre_list_count > 0 &&
            !state->timbre_list_is_percussion[state->timbre_list_count - 1u]) {
            group = state->timbre_list_count - 1u;
            if (state->timbre_list_adsr1[group] == item->adsr1 &&
                state->timbre_list_adsr2[group] == item->adsr2 &&
                state->timbre_list_flags[group] == item->flags &&
                item->ssa <= previous_extent + group_gap &&
                state->timbre_list_key_count[group] < TIMBRE_SOLO_MAX_KEYS) {
                start_new_group = 0;
            }
        }
        if (start_new_group) {
            if (state->timbre_list_count >= TIMBRE_LIST_MAX_ITEMS) {
                break;
            }
            group = state->timbre_list_count++;
            state->timbre_list_adsr1[group] = item->adsr1;
            state->timbre_list_adsr2[group] = item->adsr2;
            state->timbre_list_flags[group] = item->flags;
            state->timbre_list_ssa[group] = item->ssa;
            state->timbre_list_lsa[group] = item->lsa;
            previous_extent = timbre_candidate_extent(item);
        } else {
            uint32_t item_extent = timbre_candidate_extent(item);
            if (item_extent > previous_extent) {
                previous_extent = item_extent;
            }
        }
        state->timbre_list_keys[group][state->timbre_list_key_count[group]] = item->key;
        state->timbre_list_ssa_keys[group][state->timbre_list_key_count[group]] = item->ssa;
        state->timbre_list_lsa_keys[group][state->timbre_list_key_count[group]] = item->lsa;
        state->timbre_list_end_keys[group][state->timbre_list_key_count[group]] = item->sample_end;
        state->timbre_list_pitch_min[group][state->timbre_list_key_count[group]] = item->pitch_min;
        state->timbre_list_pitch_max[group][state->timbre_list_key_count[group]] = item->pitch_max;
        state->timbre_list_key_count[group]++;
    }
    free(percussion);
    free(items);
}

static void effective_voice_mute_masks_locked(
    const PlayerState *state,
    const Spu2LogLiveState *live,
    uint32_t *core0_mask,
    uint32_t *core1_mask)
{
    uint32_t masks[2];
    uint32_t selected_keys[TIMBRE_SOLO_LOOKUP_SIZE];
    unsigned core;
    unsigned voice;

    masks[0] = state->voice_mute_mask[0] & 0x00ffffffu;
    masks[1] = state->voice_mute_mask[1] & 0x00ffffffu;
    if (state->timbre_solo_enabled && state->timbre_solo_key_count > 0) {
        unsigned selected;

        ZeroMemory(selected_keys, sizeof(selected_keys));
        for (selected = 0; selected < state->timbre_solo_key_count; ++selected) {
            uint32_t key = state->timbre_solo_keys[selected];
            unsigned slot;

            if (key == 0) {
                continue;
            }
            slot = (unsigned)((key * 2654435761u) & (TIMBRE_SOLO_LOOKUP_SIZE - 1u));
            while (selected_keys[slot] != 0 && selected_keys[slot] != key) {
                slot = (slot + 1u) & (TIMBRE_SOLO_LOOKUP_SIZE - 1u);
            }
            selected_keys[slot] = key;
        }
        for (core = 0; core < 2; ++core) {
            for (voice = 0; voice < 24; ++voice) {
                const Spu2LogVoiceSnapshot *snapshot = &live->voices[core][voice];
                uint32_t key = voice_timbre_key(snapshot);
                uint32_t bit = 1u << voice;
                unsigned slot = (unsigned)((key * 2654435761u) & (TIMBRE_SOLO_LOOKUP_SIZE - 1u));
                int selected_voice = 0;

                if (key != 0) {
                    while (selected_keys[slot] != 0) {
                        if (selected_keys[slot] == key) {
                            selected_voice = 1;
                            break;
                        }
                        slot = (slot + 1u) & (TIMBRE_SOLO_LOOKUP_SIZE - 1u);
                    }
                }
                if (selected_voice) {
                    masks[core] &= ~bit;
                } else {
                    masks[core] |= bit;
                }
            }
        }
    }
    *core0_mask = masks[0] & 0x00ffffffu;
    *core1_mask = masks[1] & 0x00ffffffu;
}

static void apply_voice_mute_masks_mode(PlayerState *state, int immediate)
{
    uint32_t core0_mask = 0;
    uint32_t core1_mask = 0;
    uint32_t timbre_starts[TIMBRE_SOLO_MAX_KEYS];
    uint32_t timbre_loops[TIMBRE_SOLO_MAX_KEYS];
    uint32_t timbre_flags[TIMBRE_SOLO_MAX_KEYS];
    uint32_t timbre_key_count = 0;
    int timbre_enabled = 0;

    if (state == NULL) {
        return;
    }

    lock_state(state);
    timbre_enabled = state->timbre_solo_enabled && state->timbre_solo_key_count > 0;
    if (timbre_enabled) {
        core0_mask = state->voice_mute_mask[0] & 0x00ffffffu;
        core1_mask = state->voice_mute_mask[1] & 0x00ffffffu;
    } else {
        effective_voice_mute_masks_locked(state, &state->live, &core0_mask, &core1_mask);
    }
    if (timbre_enabled) {
        unsigned i;
        timbre_key_count = state->timbre_solo_key_count;
        if (timbre_key_count > TIMBRE_SOLO_MAX_KEYS) {
            timbre_key_count = TIMBRE_SOLO_MAX_KEYS;
        }
        for (i = 0; i < timbre_key_count; ++i) {
            timbre_starts[i] = state->timbre_solo_ssa[i];
            timbre_loops[i] = state->timbre_solo_lsa[i];
            timbre_flags[i] = state->timbre_solo_flags[i];
        }
    }
    unlock_state(state);
    psf2log_set_imported_timbre_solo(timbre_enabled, timbre_starts, timbre_loops, timbre_flags, timbre_key_count);
    if (immediate) {
        psf2log_set_imported_voice_mute_masks_immediate(core0_mask, core1_mask);
    } else {
        psf2log_set_imported_voice_mute_masks(core0_mask, core1_mask);
    }
}

static void apply_voice_mute_masks(PlayerState *state)
{
    apply_voice_mute_masks_mode(state, 0);
}

static void apply_voice_mute_masks_immediate(PlayerState *state)
{
    apply_voice_mute_masks_mode(state, 1);
}

static void apply_voice_reverb_masks(PlayerState *state)
{
    uint32_t core0_on = 0;
    uint32_t core1_on = 0;
    uint32_t core0_off = 0;
    uint32_t core1_off = 0;

    if (state == NULL) {
        return;
    }

    lock_state(state);
    core0_on = state->voice_reverb_force_on_mask[0] & 0x00ffffffu;
    core1_on = state->voice_reverb_force_on_mask[1] & 0x00ffffffu;
    core0_off = state->voice_reverb_force_off_mask[0] & 0x00ffffffu;
    core1_off = state->voice_reverb_force_off_mask[1] & 0x00ffffffu;
    unlock_state(state);
    psf2log_set_imported_voice_reverb_masks(core0_on, core1_on, core0_off, core1_off);
}

static void apply_voice_noise_masks(PlayerState *state)
{
    uint32_t core0_on = 0;
    uint32_t core1_on = 0;
    uint32_t core0_off = 0;
    uint32_t core1_off = 0;

    if (state == NULL) {
        return;
    }

    lock_state(state);
    core0_on = state->voice_noise_force_on_mask[0] & 0x00ffffffu;
    core1_on = state->voice_noise_force_on_mask[1] & 0x00ffffffu;
    core0_off = state->voice_noise_force_off_mask[0] & 0x00ffffffu;
    core1_off = state->voice_noise_force_off_mask[1] & 0x00ffffffu;
    unlock_state(state);
    psf2log_set_imported_voice_noise_masks(core0_on, core1_on, core0_off, core1_off);
}

static void apply_voice_pmod_masks(PlayerState *state)
{
    uint32_t core0_on = 0;
    uint32_t core1_on = 0;
    uint32_t core0_off = 0;
    uint32_t core1_off = 0;

    if (state == NULL) {
        return;
    }

    lock_state(state);
    core0_on = state->voice_pmod_force_on_mask[0] & 0x00ffffffu;
    core1_on = state->voice_pmod_force_on_mask[1] & 0x00ffffffu;
    core0_off = state->voice_pmod_force_off_mask[0] & 0x00ffffffu;
    core1_off = state->voice_pmod_force_off_mask[1] & 0x00ffffffu;
    unlock_state(state);
    psf2log_set_imported_voice_pmod_masks(core0_on, core1_on, core0_off, core1_off);
}

static void apply_voice_adsr_force_masks(PlayerState *state)
{
    uint32_t core0_mask = 0;
    uint32_t core1_mask = 0;

    if (state == NULL) {
        return;
    }

    lock_state(state);
    core0_mask = state->voice_adsr_force_mask[0] & 0x00ffffffu;
    core1_mask = state->voice_adsr_force_mask[1] & 0x00ffffffu;
    unlock_state(state);
    psf2log_set_imported_voice_adsr_force_masks(core0_mask, core1_mask);
}

static void clear_imported_voice_pitch_locks(void)
{
    unsigned core;
    unsigned voice;

    for (core = 0; core < 2u; ++core) {
        for (voice = 0; voice < 24u; ++voice) {
            psf2log_set_imported_voice_pitch_lock(core, voice, 0, 0);
        }
    }
}

static void clear_imported_voice_volume_locks(void)
{
    unsigned core;
    unsigned voice;
    unsigned side;

    for (core = 0; core < 2u; ++core) {
        for (voice = 0; voice < 24u; ++voice) {
            for (side = 0; side < 2u; ++side) {
                psf2log_set_imported_voice_volume_lock(core, voice, side, 0, 0);
            }
        }
    }
}

static void clear_voice_effect_overrides(PlayerState *state)
{
    if (state == NULL) {
        return;
    }

    lock_state(state);
    state->voice_reverb_force_on_mask[0] = 0u;
    state->voice_reverb_force_on_mask[1] = 0u;
    state->voice_reverb_force_off_mask[0] = 0u;
    state->voice_reverb_force_off_mask[1] = 0u;
    state->voice_noise_force_on_mask[0] = 0u;
    state->voice_noise_force_on_mask[1] = 0u;
    state->voice_noise_force_off_mask[0] = 0u;
    state->voice_noise_force_off_mask[1] = 0u;
    state->voice_pmod_force_on_mask[0] = 0u;
    state->voice_pmod_force_on_mask[1] = 0u;
    state->voice_pmod_force_off_mask[0] = 0u;
    state->voice_pmod_force_off_mask[1] = 0u;
    state->voice_adsr_force_mask[0] = 0u;
    state->voice_adsr_force_mask[1] = 0u;
    ZeroMemory(state->debug_adsr_touched_mask, sizeof(state->debug_adsr_touched_mask));
    ZeroMemory(state->debug_saved_voice_valid, sizeof(state->debug_saved_voice_valid));
    ZeroMemory(state->debug_saved_core_valid, sizeof(state->debug_saved_core_valid));
    state->voice_pitch_lock_mask[0] = 0u;
    state->voice_pitch_lock_mask[1] = 0u;
    ZeroMemory(state->voice_pitch_lock_value, sizeof(state->voice_pitch_lock_value));
    ZeroMemory(state->voice_volume_lock_mask, sizeof(state->voice_volume_lock_mask));
    ZeroMemory(state->voice_volume_lock_value, sizeof(state->voice_volume_lock_value));
    unlock_state(state);

    apply_voice_reverb_masks(state);
    apply_voice_noise_masks(state);
    apply_voice_pmod_masks(state);
    apply_voice_adsr_force_masks(state);
    clear_imported_voice_pitch_locks();
    clear_imported_voice_volume_locks();
}

static void save_debug_voice_value_if_needed(PlayerState *state, unsigned core, unsigned voice)
{
    if (state == NULL || core >= 2u || voice >= 24u) {
        return;
    }

    lock_state(state);
    if (!state->debug_edit_controls) {
        unlock_state(state);
        return;
    }
    if (!state->debug_saved_voice_valid[core][voice]) {
        const Spu2LogVoiceSnapshot *v = &state->live.voices[core][voice];
        state->debug_saved_adsr1[core][voice] = v->adsr1;
        state->debug_saved_adsr2[core][voice] = v->adsr2;
        state->debug_saved_pitch[core][voice] = v->pitch;
        state->debug_saved_vol_l[core][voice] = v->vol_l;
        state->debug_saved_vol_r[core][voice] = v->vol_r;
        state->debug_saved_voice_valid[core][voice] = 1u;
    }
    unlock_state(state);
}

static void save_debug_core_value_if_needed(PlayerState *state, unsigned core)
{
    if (state == NULL || core >= 2u) {
        return;
    }

    lock_state(state);
    if (!state->debug_edit_controls) {
        unlock_state(state);
        return;
    }
    if (!state->debug_saved_core_valid[core]) {
        state->debug_saved_noise_clock[core] = (uint8_t)((state->live.cores[core].flags >> 8) & NOISE_CLOCK_MAX);
        state->debug_saved_reverb_l[core] = state->live.cores[core].reverb_l;
        state->debug_saved_reverb_r[core] = state->live.cores[core].reverb_r;
        state->debug_saved_core_valid[core] = 1u;
    }
    unlock_state(state);
}

static void apply_saved_adsr_value(unsigned core, unsigned voice, uint16_t adsr1, uint16_t adsr2)
{
    psf2log_set_imported_voice_adsr_value(core, voice, ADSR_FIELD_ATTACK, (unsigned)((adsr1 >> 8) & 0x7fu));
    psf2log_set_imported_voice_adsr_value(core, voice, ADSR_FIELD_DECAY, (unsigned)((adsr1 >> 4) & 0x0fu));
    psf2log_set_imported_voice_adsr_value(core, voice, ADSR_FIELD_SUSTAIN_LEVEL, (unsigned)(adsr1 & 0x0fu));
    psf2log_set_imported_voice_adsr_value(core, voice, ADSR_FIELD_SUSTAIN_RATE, (unsigned)((adsr2 >> 6) & 0x7fu));
    psf2log_set_imported_voice_adsr_value(core, voice, ADSR_FIELD_RELEASE, (unsigned)(adsr2 & 0x1fu));
}

static void restore_debug_edits_to_saved_values(HWND hwnd, PlayerState *state)
{
    uint8_t voice_valid[2][24];
    uint16_t adsr1[2][24];
    uint16_t adsr2[2][24];
    uint16_t pitch[2][24];
    uint16_t vol_l[2][24];
    uint16_t vol_r[2][24];
    uint32_t adsr_touched_mask[2];
    uint32_t pitch_touched_mask[2];
    uint32_t volume_touched_mask[2][2];
    uint8_t core_valid[2];
    uint8_t noise_clock[2];
    uint16_t reverb_l[2];
    uint16_t reverb_r[2];
    unsigned core;
    unsigned voice;

    if (state == NULL) {
        return;
    }

    lock_state(state);
    memcpy(voice_valid, state->debug_saved_voice_valid, sizeof(voice_valid));
    memcpy(adsr1, state->debug_saved_adsr1, sizeof(adsr1));
    memcpy(adsr2, state->debug_saved_adsr2, sizeof(adsr2));
    memcpy(pitch, state->debug_saved_pitch, sizeof(pitch));
    memcpy(vol_l, state->debug_saved_vol_l, sizeof(vol_l));
    memcpy(vol_r, state->debug_saved_vol_r, sizeof(vol_r));
    memcpy(adsr_touched_mask, state->debug_adsr_touched_mask, sizeof(adsr_touched_mask));
    memcpy(pitch_touched_mask, state->voice_pitch_lock_mask, sizeof(pitch_touched_mask));
    memcpy(volume_touched_mask, state->voice_volume_lock_mask, sizeof(volume_touched_mask));
    memcpy(core_valid, state->debug_saved_core_valid, sizeof(core_valid));
    memcpy(noise_clock, state->debug_saved_noise_clock, sizeof(noise_clock));
    memcpy(reverb_l, state->debug_saved_reverb_l, sizeof(reverb_l));
    memcpy(reverb_r, state->debug_saved_reverb_r, sizeof(reverb_r));
    state->voice_reverb_force_on_mask[0] = 0u;
    state->voice_reverb_force_on_mask[1] = 0u;
    state->voice_reverb_force_off_mask[0] = 0u;
    state->voice_reverb_force_off_mask[1] = 0u;
    state->voice_noise_force_on_mask[0] = 0u;
    state->voice_noise_force_on_mask[1] = 0u;
    state->voice_noise_force_off_mask[0] = 0u;
    state->voice_noise_force_off_mask[1] = 0u;
    state->voice_pmod_force_on_mask[0] = 0u;
    state->voice_pmod_force_on_mask[1] = 0u;
    state->voice_pmod_force_off_mask[0] = 0u;
    state->voice_pmod_force_off_mask[1] = 0u;
    state->voice_adsr_force_mask[0] &= ~adsr_touched_mask[0];
    state->voice_adsr_force_mask[1] &= ~adsr_touched_mask[1];
    state->voice_pitch_lock_mask[0] = 0u;
    state->voice_pitch_lock_mask[1] = 0u;
    ZeroMemory(state->voice_pitch_lock_value, sizeof(state->voice_pitch_lock_value));
    ZeroMemory(state->voice_volume_lock_mask, sizeof(state->voice_volume_lock_mask));
    ZeroMemory(state->voice_volume_lock_value, sizeof(state->voice_volume_lock_value));
    ZeroMemory(state->manual_reverb_value_valid, sizeof(state->manual_reverb_value_valid));

    for (core = 0; core < 2u; ++core) {
        if (core_valid[core]) {
            state->live.cores[core].flags = (state->live.cores[core].flags & ~0x3f00u) | ((uint32_t)(noise_clock[core] & NOISE_CLOCK_MAX) << 8);
            state->live.cores[core].reverb_l = reverb_l[core];
            state->live.cores[core].reverb_r = reverb_r[core];
        }
        for (voice = 0; voice < 24u; ++voice) {
            if (voice_valid[core][voice] &&
                (adsr_touched_mask[core] & (1u << voice)) != 0) {
                state->live.voices[core][voice].adsr1 = adsr1[core][voice];
                state->live.voices[core][voice].adsr2 = adsr2[core][voice];
            }
            if (voice_valid[core][voice] &&
                (pitch_touched_mask[core] & (1u << voice)) != 0) {
                state->live.voices[core][voice].pitch = pitch[core][voice];
            }
            if (voice_valid[core][voice] &&
                (volume_touched_mask[0][core] & (1u << voice)) != 0) {
                state->live.voices[core][voice].vol_l = vol_l[core][voice];
            }
            if (voice_valid[core][voice] &&
                (volume_touched_mask[1][core] & (1u << voice)) != 0) {
                state->live.voices[core][voice].vol_r = vol_r[core][voice];
            }
        }
    }

    ZeroMemory(state->debug_saved_adsr1, sizeof(state->debug_saved_adsr1));
    ZeroMemory(state->debug_saved_adsr2, sizeof(state->debug_saved_adsr2));
    ZeroMemory(state->debug_saved_pitch, sizeof(state->debug_saved_pitch));
    ZeroMemory(state->debug_saved_vol_l, sizeof(state->debug_saved_vol_l));
    ZeroMemory(state->debug_saved_vol_r, sizeof(state->debug_saved_vol_r));
    ZeroMemory(state->debug_saved_voice_valid, sizeof(state->debug_saved_voice_valid));
    ZeroMemory(state->debug_adsr_touched_mask, sizeof(state->debug_adsr_touched_mask));
    ZeroMemory(state->debug_saved_noise_clock, sizeof(state->debug_saved_noise_clock));
    ZeroMemory(state->debug_saved_reverb_l, sizeof(state->debug_saved_reverb_l));
    ZeroMemory(state->debug_saved_reverb_r, sizeof(state->debug_saved_reverb_r));
    ZeroMemory(state->debug_saved_core_valid, sizeof(state->debug_saved_core_valid));
    unlock_state(state);

    apply_voice_reverb_masks(state);
    apply_voice_noise_masks(state);
    apply_voice_pmod_masks(state);
    apply_voice_adsr_force_masks(state);
    clear_imported_voice_pitch_locks();
    clear_imported_voice_volume_locks();

    for (core = 0; core < 2u; ++core) {
        if (core_valid[core]) {
            psf2log_set_imported_noise_clock(core, noise_clock[core]);
            psf2log_set_imported_reverb_value(core, 0u, reverb_l[core]);
            psf2log_set_imported_reverb_value(core, 1u, reverb_r[core]);
        }
        for (voice = 0; voice < 24u; ++voice) {
            if (voice_valid[core][voice] &&
                (adsr_touched_mask[core] & (1u << voice)) != 0) {
                apply_saved_adsr_value(core, voice, adsr1[core][voice], adsr2[core][voice]);
            }
            if (voice_valid[core][voice] &&
                (pitch_touched_mask[core] & (1u << voice)) != 0) {
                psf2log_set_imported_voice_pitch(core, voice, pitch[core][voice]);
            }
            if (voice_valid[core][voice] &&
                (volume_touched_mask[0][core] & (1u << voice)) != 0) {
                psf2log_set_imported_voice_volume(core, voice, 0u, vol_l[core][voice]);
            }
            if (voice_valid[core][voice] &&
                (volume_touched_mask[1][core] & (1u << voice)) != 0) {
                psf2log_set_imported_voice_volume(core, voice, 1u, vol_r[core][voice]);
            }
        }
    }

    InvalidateRect(hwnd, NULL, TRUE);
}

static unsigned adsr_field_max(unsigned field)
{
    switch (field) {
    case ADSR_FIELD_ATTACK:
    case ADSR_FIELD_SUSTAIN_RATE:
        return 0x7fu;
    case ADSR_FIELD_DECAY:
    case ADSR_FIELD_SUSTAIN_LEVEL:
        return 0x0fu;
    case ADSR_FIELD_RELEASE:
        return 0x1fu;
    default:
        return 0u;
    }
}

static unsigned adsr_field_value(const Spu2LogVoiceSnapshot *voice, unsigned field)
{
    if (voice == NULL) {
        return 0u;
    }
    switch (field) {
    case ADSR_FIELD_ATTACK:
        return (unsigned)((voice->adsr1 >> 8) & 0x7fu);
    case ADSR_FIELD_DECAY:
        return (unsigned)((voice->adsr1 >> 4) & 0x0fu);
    case ADSR_FIELD_SUSTAIN_LEVEL:
        return (unsigned)(voice->adsr1 & 0x0fu);
    case ADSR_FIELD_SUSTAIN_RATE:
        return (unsigned)((voice->adsr2 >> 6) & 0x7fu);
    case ADSR_FIELD_RELEASE:
        return (unsigned)(voice->adsr2 & 0x1fu);
    default:
        return 0u;
    }
}

static void set_voice_adsr_value(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, unsigned field, unsigned value)
{
    unsigned max_value;

    if (state == NULL || core >= 2u || voice >= 24u || field > ADSR_FIELD_RELEASE) {
        return;
    }
    max_value = adsr_field_max(field);
    if (value > max_value) {
        value = max_value;
    }
    save_debug_voice_value_if_needed(state, core, voice);
    lock_state(state);
    if (state->debug_edit_controls) {
        state->debug_adsr_touched_mask[core] |= (1u << voice);
    }
    unlock_state(state);
    psf2log_set_imported_voice_adsr_value(core, voice, field, value);
    lock_state(state);
    switch (field) {
    case ADSR_FIELD_ATTACK:
        state->live.voices[core][voice].adsr1 = (uint16_t)((state->live.voices[core][voice].adsr1 & ~0x7f00u) | ((value & 0x7fu) << 8));
        break;
    case ADSR_FIELD_DECAY:
        state->live.voices[core][voice].adsr1 = (uint16_t)((state->live.voices[core][voice].adsr1 & ~0x00f0u) | ((value & 0x0fu) << 4));
        break;
    case ADSR_FIELD_SUSTAIN_LEVEL:
        state->live.voices[core][voice].adsr1 = (uint16_t)((state->live.voices[core][voice].adsr1 & ~0x000fu) | (value & 0x0fu));
        break;
    case ADSR_FIELD_SUSTAIN_RATE:
        state->live.voices[core][voice].adsr2 = (uint16_t)((state->live.voices[core][voice].adsr2 & ~0x1fc0u) | ((value & 0x7fu) << 6));
        break;
    case ADSR_FIELD_RELEASE:
        state->live.voices[core][voice].adsr2 = (uint16_t)((state->live.voices[core][voice].adsr2 & ~0x001fu) | (value & 0x1fu));
        break;
    default:
        break;
    }
    unlock_state(state);
    InvalidateRect(hwnd, NULL, FALSE);
}

typedef void (*HexValueChangedCallback)(void *user, unsigned value);

static void set_voice_pitch_value(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, unsigned value);
static void set_voice_volume_value(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, unsigned side, unsigned value);
static int choose_hex_value_ex(HWND owner, unsigned current_value, unsigned max_value, unsigned slider_step, unsigned *out_value, HexValueChangedCallback on_change, void *on_change_user);
static int choose_hex_value(HWND owner, unsigned current_value, unsigned max_value, unsigned *out_value);
static int choose_reverb_values(HWND owner, PlayerState *state, unsigned core, unsigned focus_side, int link_initial);
static int choose_voice_volume_values(HWND owner, PlayerState *state, unsigned core, unsigned voice, unsigned focus_side, int link_initial);

static void set_noise_clock_value(HWND hwnd, PlayerState *state, unsigned core, unsigned value)
{
    unsigned voice;

    if (state == NULL || core >= 2u) {
        return;
    }
    if (value > NOISE_CLOCK_MAX) {
        value = NOISE_CLOCK_MAX;
    }
    save_debug_core_value_if_needed(state, core);
    psf2log_set_imported_noise_clock(core, value);
    lock_state(state);
    state->live.cores[core].flags = (state->live.cores[core].flags & ~0x3f00u) | ((uint32_t)value << 8);
    for (voice = 0; voice < 24u; ++voice) {
        state->live.voices[core][voice].noise_clock = (uint8_t)value;
    }
    unlock_state(state);
    InvalidateRect(hwnd, NULL, FALSE);
}

static unsigned current_noise_clock_value(PlayerState *state, unsigned core)
{
    unsigned value;

    if (state == NULL || core >= 2u) {
        return 0u;
    }
    lock_state(state);
    value = (unsigned)((state->live.cores[core].flags & 0x3f00u) >> 8);
    unlock_state(state);
    return value & NOISE_CLOCK_MAX;
}

static unsigned current_adsr_value(PlayerState *state, unsigned core, unsigned voice, unsigned field)
{
    unsigned value;

    if (state == NULL || core >= 2u || voice >= 24u) {
        return 0u;
    }
    lock_state(state);
    value = adsr_field_value(&state->live.voices[core][voice], field);
    unlock_state(state);
    return value;
}

static unsigned current_reverb_value(PlayerState *state, unsigned core, unsigned side)
{
    unsigned value;

    if (state == NULL || core >= 2u || side >= 2u) {
        return 0u;
    }
    lock_state(state);
    value = side == 0u ? state->live.cores[core].reverb_l : state->live.cores[core].reverb_r;
    unlock_state(state);
    return value & 0xffffu;
}

static unsigned reverb_display_to_raw(unsigned value)
{
    value &= 0xffffu;
    return ((value & 0x00ffu) << 8) | ((value >> 8) & 0x00ffu);
}

static unsigned reverb_raw_to_display(unsigned value)
{
    return reverb_display_to_raw(value);
}

static void set_reverb_value(HWND hwnd, PlayerState *state, unsigned core, unsigned side, unsigned value)
{
    if (state == NULL || core >= 2u || side >= 2u) {
        return;
    }
    value &= 0xffffu;
    save_debug_core_value_if_needed(state, core);
    psf2log_set_imported_reverb_value(core, side, value);
    lock_state(state);
    if (side == 0u) {
        state->live.cores[core].reverb_l = (uint16_t)value;
    } else {
        state->live.cores[core].reverb_r = (uint16_t)value;
    }
    state->manual_reverb_value_valid[core][side] = 1u;
    unlock_state(state);
    InvalidateRect(hwnd, NULL, FALSE);
}

static void adjust_noise_clock_value(HWND hwnd, PlayerState *state, unsigned core, int delta)
{
    int value = (int)current_noise_clock_value(state, core) + delta;

    if (value < 0) {
        value = 0;
    }
    if (value > (int)NOISE_CLOCK_MAX) {
        value = (int)NOISE_CLOCK_MAX;
    }
    set_noise_clock_value(hwnd, state, core, (unsigned)value);
}

static void adjust_voice_adsr_value(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, unsigned field, int delta)
{
    int value;
    unsigned max_value;

    max_value = adsr_field_max(field);
    value = (int)current_adsr_value(state, core, voice, field) + delta;
    if (value < 0) {
        value = 0;
    }
    if ((unsigned)value > max_value) {
        value = (int)max_value;
    }
    set_voice_adsr_value(hwnd, state, core, voice, field, (unsigned)value);
}

static void adjust_reverb_value(HWND hwnd, PlayerState *state, unsigned core, unsigned side, int delta)
{
    int value = (int)current_reverb_value(state, core, side) + delta;

    if (value < 0) {
        value = 0;
    }
    if (value > 0xffff) {
        value = 0xffff;
    }
    set_reverb_value(hwnd, state, core, side, (unsigned)value);
}

static void adjust_reverb_both_values(HWND hwnd, PlayerState *state, unsigned core, int delta)
{
    adjust_reverb_value(hwnd, state, core, 0u, delta);
    adjust_reverb_value(hwnd, state, core, 1u, delta);
}

typedef enum HexApplyKind {
    HEX_APPLY_NOISE_CLOCK,
    HEX_APPLY_ADSR,
    HEX_APPLY_PITCH,
    HEX_APPLY_VOLUME,
    HEX_APPLY_REVERB,
    HEX_APPLY_REVERB_BOTH
} HexApplyKind;

typedef struct HexApplyContext {
    HWND hwnd;
    PlayerState *state;
    HexApplyKind kind;
    unsigned core;
    unsigned voice;
    unsigned field;
    unsigned side;
} HexApplyContext;

static void apply_hex_value_change(void *user, unsigned value)
{
    HexApplyContext *context = (HexApplyContext *)user;

    if (context == NULL || context->state == NULL) {
        return;
    }
    switch (context->kind) {
    case HEX_APPLY_NOISE_CLOCK:
        set_noise_clock_value(context->hwnd, context->state, context->core, value);
        break;
    case HEX_APPLY_ADSR:
        set_voice_adsr_value(context->hwnd, context->state, context->core, context->voice, context->field, value);
        break;
    case HEX_APPLY_PITCH:
        set_voice_pitch_value(context->hwnd, context->state, context->core, context->voice, value);
        break;
    case HEX_APPLY_VOLUME:
        set_voice_volume_value(context->hwnd, context->state, context->core, context->voice, context->side, value);
        break;
    case HEX_APPLY_REVERB:
        set_reverb_value(context->hwnd, context->state, context->core, context->side, value);
        break;
    case HEX_APPLY_REVERB_BOTH:
        set_reverb_value(context->hwnd, context->state, context->core, 0u, value);
        set_reverb_value(context->hwnd, context->state, context->core, 1u, value);
        break;
    default:
        break;
    }
}

static void input_noise_clock_value(HWND hwnd, PlayerState *state, unsigned core)
{
    HexApplyContext context;
    unsigned value;

    value = current_noise_clock_value(state, core);
    context.hwnd = hwnd;
    context.state = state;
    context.kind = HEX_APPLY_NOISE_CLOCK;
    context.core = core;
    context.voice = 0u;
    context.field = 0u;
    context.side = 0u;
    if (choose_hex_value_ex(hwnd, value, NOISE_CLOCK_MAX, 1u, &value, apply_hex_value_change, &context)) {
        set_noise_clock_value(hwnd, state, core, value);
    }
}

static void input_voice_adsr_value(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, unsigned field)
{
    HexApplyContext context;
    unsigned value;

    value = current_adsr_value(state, core, voice, field);
    context.hwnd = hwnd;
    context.state = state;
    context.kind = HEX_APPLY_ADSR;
    context.core = core;
    context.voice = voice;
    context.field = field;
    context.side = 0u;
    if (choose_hex_value_ex(hwnd, value, adsr_field_max(field), 1u, &value, apply_hex_value_change, &context)) {
        set_voice_adsr_value(hwnd, state, core, voice, field, value);
    }
}

static unsigned current_voice_pitch_value(PlayerState *state, unsigned core, unsigned voice)
{
    unsigned value;

    if (state == NULL || core >= 2u || voice >= 24u) {
        return 0u;
    }
    lock_state(state);
    value = state->live.voices[core][voice].pitch;
    unlock_state(state);
    return value & 0x3fffu;
}

static void set_voice_pitch_value(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, unsigned value)
{
    if (state == NULL || core >= 2u || voice >= 24u) {
        return;
    }
    if (value > 0x3fffu) {
        value = 0x3fffu;
    }
    save_debug_voice_value_if_needed(state, core, voice);
    lock_state(state);
    state->live.voices[core][voice].pitch = (uint16_t)value;
    state->voice_pitch_lock_value[core][voice] = (uint16_t)value;
    state->voice_pitch_lock_mask[core] |= (1u << voice);
    unlock_state(state);
    psf2log_set_imported_voice_pitch_lock(core, voice, 1, value);
    InvalidateRect(hwnd, NULL, FALSE);
}

static void input_voice_pitch_value(HWND hwnd, PlayerState *state, unsigned core, unsigned voice)
{
    HexApplyContext context;
    unsigned value;

    value = current_voice_pitch_value(state, core, voice);
    context.hwnd = hwnd;
    context.state = state;
    context.kind = HEX_APPLY_PITCH;
    context.core = core;
    context.voice = voice;
    context.field = 0u;
    context.side = 0u;
    if (choose_hex_value_ex(hwnd, value, 0x3fffu, 1u, &value, apply_hex_value_change, &context)) {
        set_voice_pitch_value(hwnd, state, core, voice, value);
    }
}

static unsigned current_voice_volume_value(PlayerState *state, unsigned core, unsigned voice, unsigned side)
{
    unsigned value;

    if (state == NULL || core >= 2u || voice >= 24u || side >= 2u) {
        return 0u;
    }
    lock_state(state);
    value = side == 0u ? state->live.voices[core][voice].vol_l : state->live.voices[core][voice].vol_r;
    unlock_state(state);
    return value & 0xffffu;
}

static void set_voice_volume_value(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, unsigned side, unsigned value)
{
    if (state == NULL || core >= 2u || voice >= 24u || side >= 2u) {
        return;
    }
    value &= 0xffffu;
    save_debug_voice_value_if_needed(state, core, voice);
    lock_state(state);
    if (side == 0u) {
        state->live.voices[core][voice].vol_l = (uint16_t)value;
    } else {
        state->live.voices[core][voice].vol_r = (uint16_t)value;
    }
    state->voice_volume_lock_value[side][core][voice] = (uint16_t)value;
    state->voice_volume_lock_mask[side][core] |= (1u << voice);
    unlock_state(state);
    psf2log_set_imported_voice_volume_lock(core, voice, side, 1, value);
    InvalidateRect(hwnd, NULL, FALSE);
}

static void input_voice_volume_value(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, unsigned side)
{
    (void)choose_voice_volume_values(hwnd, state, core, voice, side, 1);
}

static void lock_voice_volume_current_value(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, unsigned side)
{
    unsigned value;

    if (state == NULL || core >= 2u || voice >= 24u || side >= 2u) {
        return;
    }
    value = current_voice_volume_value(state, core, voice, side);
    set_voice_volume_value(hwnd, state, core, voice, side, value);
}

static void input_reverb_value(HWND hwnd, PlayerState *state, unsigned core, unsigned side)
{
    (void)choose_reverb_values(hwnd, state, core, side, 0);
}

static void input_reverb_both_values(HWND hwnd, PlayerState *state, unsigned core)
{
    (void)choose_reverb_values(hwnd, state, core, 0u, 1);
}

static void reset_noise_clock_value_to_default(HWND hwnd, PlayerState *state, unsigned core)
{
    unsigned value = 0u;

    if (state == NULL || core >= 2u) {
        return;
    }
    lock_state(state);
    if (state->default_core_valid[core]) {
        value = state->default_noise_clock[core];
    }
    unlock_state(state);
    set_noise_clock_value(hwnd, state, core, value);
}

static void reset_voice_adsr_value_to_default(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, unsigned field)
{
    unsigned value = 0u;
    Spu2LogVoiceSnapshot snapshot;

    if (state == NULL || core >= 2u || voice >= 24u || field > ADSR_FIELD_RELEASE) {
        return;
    }
    ZeroMemory(&snapshot, sizeof(snapshot));
    lock_state(state);
    if (state->default_voice_valid[core][voice]) {
        snapshot.adsr1 = state->default_adsr1[core][voice];
        snapshot.adsr2 = state->default_adsr2[core][voice];
    }
    unlock_state(state);
    value = adsr_field_value(&snapshot, field);
    set_voice_adsr_value(hwnd, state, core, voice, field, value);
}

static void reset_voice_pitch_value_to_default(HWND hwnd, PlayerState *state, unsigned core, unsigned voice)
{
    unsigned value = 0u;
    int has_default = 0;

    if (state == NULL || core >= 2u || voice >= 24u) {
        return;
    }
    lock_state(state);
    if (state->default_voice_valid[core][voice]) {
        value = state->default_pitch[core][voice];
        has_default = 1;
    }
    state->voice_pitch_lock_mask[core] &= ~(1u << voice);
    unlock_state(state);
    psf2log_set_imported_voice_pitch_lock(core, voice, 0, 0);
    if (has_default) {
        set_voice_pitch_value(hwnd, state, core, voice, value);
        lock_state(state);
        state->voice_pitch_lock_mask[core] &= ~(1u << voice);
        unlock_state(state);
        psf2log_set_imported_voice_pitch_lock(core, voice, 0, 0);
    } else {
        psf2log_restore_imported_voice_pitch(core, voice);
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

static void reset_voice_volume_value_to_default(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, unsigned side)
{
    unsigned value = 0u;
    int has_default = 0;

    if (state == NULL || core >= 2u || voice >= 24u || side >= 2u) {
        return;
    }
    lock_state(state);
    if (state->default_voice_valid[core][voice]) {
        value = side == 0u ? state->default_vol_l[core][voice] : state->default_vol_r[core][voice];
        has_default = 1;
    }
    state->voice_volume_lock_mask[side][core] &= ~(1u << voice);
    unlock_state(state);
    psf2log_set_imported_voice_volume_lock(core, voice, side, 0, 0);
    if (has_default) {
        psf2log_set_imported_voice_volume(core, voice, side, value);
        lock_state(state);
        if (side == 0u) {
            state->live.voices[core][voice].vol_l = (uint16_t)value;
        } else {
            state->live.voices[core][voice].vol_r = (uint16_t)value;
        }
        unlock_state(state);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

static void unlock_voice_volume_value(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, unsigned side)
{
    if (state == NULL || core >= 2u || voice >= 24u || side >= 2u) {
        return;
    }
    lock_state(state);
    state->voice_volume_lock_mask[side][core] &= ~(1u << voice);
    unlock_state(state);
    psf2log_set_imported_voice_volume_lock(core, voice, side, 0, 0);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void reset_reverb_value_to_default(HWND hwnd, PlayerState *state, unsigned core, unsigned side)
{
    unsigned value = 0u;
    int has_default = 0;

    if (state == NULL || core >= 2u || side >= 2u) {
        return;
    }
    lock_state(state);
    if (state->default_core_valid[core]) {
        value = side == 0u ? state->default_reverb_l[core] : state->default_reverb_r[core];
        has_default = 1;
    }
    unlock_state(state);
    if (!has_default) {
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }
    set_reverb_value(hwnd, state, core, side, value);
    lock_state(state);
    state->manual_reverb_value_valid[core][side] = 0u;
    unlock_state(state);
}

static void reset_reverb_both_values_to_default(HWND hwnd, PlayerState *state, unsigned core)
{
    reset_reverb_value_to_default(hwnd, state, core, 0u);
    reset_reverb_value_to_default(hwnd, state, core, 1u);
}

static int is_numeric_flag_column(unsigned column)
{
    return column == VOICE_FLAG_COLUMN_NOISE_CLOCK ||
        column == VOICE_FLAG_COLUMN_REVERB_L ||
        column == VOICE_FLAG_COLUMN_REVERB_R ||
        column == VOICE_FLAG_COLUMN_REVERB_BOTH ||
        (column >= VOICE_FLAG_COLUMN_ADSR0 && column < VOICE_FLAG_COLUMN_ADSR0 + 5);
}

static void perform_pending_numeric_click(HWND hwnd, PlayerState *state)
{
    int delta;
    unsigned column;

    if (state == NULL || !state->pending_click_active || !debug_edit_controls_active(state)) {
        if (state != NULL && state->pending_click_active) {
            KillTimer(hwnd, PLAYER_CLICK_TIMER_ID);
            state->pending_click_active = 0;
            state->pending_click_repeating = 0;
            state->pending_click_button = 0;
            ReleaseCapture();
        }
        return;
    }

    delta = state->pending_click_button == 2 ? -1 : 1;
    column = state->pending_click_column;
    if (column == VOICE_FLAG_COLUMN_NOISE_CLOCK) {
        adjust_noise_clock_value(hwnd, state, state->pending_click_core, delta);
    } else if (column == VOICE_FLAG_COLUMN_REVERB_BOTH) {
        adjust_reverb_both_values(hwnd, state, state->pending_click_core, delta);
    } else if (column == VOICE_FLAG_COLUMN_REVERB_L || column == VOICE_FLAG_COLUMN_REVERB_R) {
        adjust_reverb_value(
            hwnd,
            state,
            state->pending_click_core,
            column == VOICE_FLAG_COLUMN_REVERB_L ? 0u : 1u,
            delta);
    } else if (column >= VOICE_FLAG_COLUMN_ADSR0 && column < VOICE_FLAG_COLUMN_ADSR0 + 5) {
        adjust_voice_adsr_value(
            hwnd,
            state,
            state->pending_click_core,
            state->pending_click_voice,
            column - VOICE_FLAG_COLUMN_ADSR0,
            delta);
    }
}

static void cancel_pending_numeric_click(HWND hwnd, PlayerState *state)
{
    if (state == NULL) {
        return;
    }
    KillTimer(hwnd, PLAYER_CLICK_TIMER_ID);
    state->pending_click_active = 0;
    state->pending_click_repeating = 0;
    state->pending_click_button = 0;
    ReleaseCapture();
}

static void start_pending_numeric_click(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, unsigned column, int button)
{
    if (state == NULL || !is_numeric_flag_column(column)) {
        return;
    }
    KillTimer(hwnd, PLAYER_CLICK_TIMER_ID);
    state->pending_click_active = 1;
    state->pending_click_repeating = 0;
    state->pending_click_button = button;
    state->pending_click_core = core;
    state->pending_click_voice = voice;
    state->pending_click_column = column;
    SetCapture(hwnd);
    SetTimer(hwnd, PLAYER_CLICK_TIMER_ID, PLAYER_CLICK_DELAY_MS, NULL);
}

static int pending_click_button_is_down(const PlayerState *state)
{
    if (state == NULL || !state->pending_click_active) {
        return 0;
    }
    if (state->pending_click_button == 2) {
        return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    }
    return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
}

static void update_pending_numeric_click(HWND hwnd, PlayerState *state)
{
    if (state == NULL || !state->pending_click_active) {
        return;
    }

    perform_pending_numeric_click(hwnd, state);
    if (pending_click_button_is_down(state)) {
        state->pending_click_repeating = 1;
        SetTimer(hwnd, PLAYER_CLICK_TIMER_ID, PLAYER_CLICK_REPEAT_MS, NULL);
    } else {
        cancel_pending_numeric_click(hwnd, state);
    }
}

static void set_all_voice_mutes(HWND hwnd, PlayerState *state, int muted)
{
    if (state == NULL) {
        return;
    }

    lock_state(state);
    state->voice_mute_mask[0] = muted ? 0x00ffffffu : 0u;
    state->voice_mute_mask[1] = muted ? 0x00ffffffu : 0u;
    reset_timbre_solo_locked(state);
    unlock_state(state);
    apply_voice_mute_masks(state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void toggle_timbre_solo_from_voice(HWND hwnd, PlayerState *state, unsigned core, unsigned voice)
{
    Spu2LogVoiceSnapshot snapshot;
    uint32_t key;
    char message[128];

    if (state == NULL || core >= 2u || voice >= 24u) {
        return;
    }

    lock_state(state);
    snapshot = state->live.voices[core][voice];
    key = voice_timbre_key(&snapshot);
    if (key == 0) {
        unlock_state(state);
        set_status(state, "Timbre solo: no sample address");
        InvalidateRect(hwnd, NULL, TRUE);
        return;
    }
    if (state->timbre_solo_enabled && timbre_solo_has_key_locked(state, key)) {
        reset_timbre_solo_locked(state);
        state->voice_mute_mask[0] = 0u;
        state->voice_mute_mask[1] = 0u;
        unlock_state(state);
        set_status(state, "Timbre solo off");
    } else {
        reset_timbre_solo_locked(state);
        state->timbre_solo_enabled = 1;
        state->timbre_solo_adsr1 = snapshot.adsr1;
        state->timbre_solo_adsr2 = snapshot.adsr2;
        state->timbre_solo_flag_class = voice_timbre_flag_class(&snapshot);
        timbre_solo_add_key_locked(state,
            key,
            snapshot.ssa,
            voice_timbre_loop_addr(&snapshot),
            voice_timbre_flag_class(&snapshot));
        state->voice_mute_mask[0] = 0u;
        state->voice_mute_mask[1] = 0u;
        unlock_state(state);
        snprintf(message, sizeof(message), "Timbre solo: 0x%08X", key);
        set_status(state, message);
    }
    apply_voice_mute_masks(state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void sync_timbre_solo_from_listbox(HWND hwnd, PlayerState *state)
{
    char message[128];
    int row;
    int row_count;
    unsigned selected_rows = 0;

    if (state == NULL || state->timbre_listbox == NULL) {
        return;
    }

    lock_state(state);
    reset_timbre_solo_locked(state);
    row_count = (int)SendMessageA(state->timbre_listbox, LB_GETCOUNT, 0, 0);
    for (row = 0; row < row_count; ++row) {
        LRESULT item_data;
        unsigned index;
        unsigned sample;
        unsigned first_sample;
        unsigned sample_limit;
        unsigned k;

        if (SendMessageA(state->timbre_listbox, LB_GETSEL, (WPARAM)row, 0) <= 0) {
            continue;
        }
        item_data = SendMessageA(state->timbre_listbox, LB_GETITEMDATA, (WPARAM)row, 0);
        if (item_data == LB_ERR) {
            continue;
        }
        index = HIWORD((DWORD_PTR)item_data);
        sample = LOWORD((DWORD_PTR)item_data);
        if (index >= state->timbre_list_count || state->timbre_list_key_count[index] == 0) {
            continue;
        }
        if (sample == TIMBRE_LIST_GROUP_ROW) {
            if (state->timbre_list_key_count[index] > 1 &&
                state->timbre_list_expanded[index]) {
                continue;
            }
            first_sample = 0;
            sample_limit = state->timbre_list_key_count[index];
        } else if (sample < state->timbre_list_key_count[index]) {
            first_sample = sample;
            sample_limit = sample + 1u;
        } else {
            continue;
        }
        if (state->timbre_solo_key_count == 0) {
            state->timbre_solo_adsr1 = state->timbre_list_adsr1[index];
            state->timbre_solo_adsr2 = state->timbre_list_adsr2[index];
            state->timbre_solo_flag_class = state->timbre_list_flags[index];
        }
        state->timbre_solo_enabled = 1;
        for (k = first_sample; k < sample_limit; ++k) {
            timbre_solo_add_key_locked(state,
                state->timbre_list_keys[index][k],
                state->timbre_list_ssa_keys[index][k],
                state->timbre_list_lsa_keys[index][k],
                state->timbre_list_flags[index]);
        }
        selected_rows++;
        if (state->timbre_solo_key_count >= TIMBRE_SOLO_MAX_KEYS) {
            break;
        }
    }
    if (state->timbre_solo_key_count == 0) {
        reset_timbre_solo_locked(state);
    }
    state->voice_mute_mask[0] = 0u;
    state->voice_mute_mask[1] = 0u;
    if (state->timbre_solo_enabled) {
        snprintf(message,
            sizeof(message),
            "Timbre solo: %u row%s / %u sample%s",
            selected_rows,
            selected_rows == 1 ? "" : "s",
            state->timbre_solo_key_count,
            state->timbre_solo_key_count == 1 ? "" : "s");
    } else {
        snprintf(message, sizeof(message), "Timbre solo off");
    }
    unlock_state(state);

    set_status(state, message);
    apply_voice_mute_masks(state);
    if (hwnd != NULL) {
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

static void refresh_timbre_listbox_after_drag(PlayerState *state)
{
    if (state != NULL && state->timbre_hwnd != NULL) {
        timbre_refresh_listbox(state);
    }
}

static void toggle_timbre_scan_mode(HWND hwnd, PlayerState *state)
{
    int enabled;

    if (state == NULL) {
        return;
    }
    lock_state(state);
    enabled = state->timbre_scan_enabled ? 0 : 1;
    state->timbre_scan_enabled = enabled;
    reset_timbre_solo_locked(state);
    preview_free_samples_locked(state);
    reset_timbre_list_locked(state);
    if (!enabled) {
        state->timbre_list_locked = 1;
    }
    unlock_state(state);
    apply_voice_mute_masks(state);
    save_display_settings(state);
    update_settings_menu_check(hwnd, state);
    if (enabled) {
        show_timbre_window(hwnd, state);
    }
    timbre_refresh_listbox(state);
    set_status(state, enabled ? "Samples Scan on" : "Samples Scan off");
    InvalidateRect(hwnd, NULL, FALSE);
}

static void toggle_voice_solo(HWND hwnd, PlayerState *state, unsigned core, unsigned voice)
{
    uint32_t solo_mask;
    uint32_t current_mask;
    uint32_t all_mask = 0x00ffffffu;

    if (state == NULL || core >= 2u || voice >= 24u) {
        return;
    }
    solo_mask = all_mask & ~(1u << voice);
    lock_state(state);
    current_mask = state->voice_mute_mask[core] & all_mask;
    if (current_mask == solo_mask) {
        state->voice_mute_mask[core] = 0u;
    } else {
        state->voice_mute_mask[core] = solo_mask;
    }
    unlock_state(state);
    apply_voice_mute_masks(state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void set_voice_reverb_override(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, int enabled)
{
    uint32_t bit;

    if (state == NULL || core >= 2u || voice >= 24u) {
        return;
    }

    bit = 1u << voice;
    lock_state(state);
    if (enabled) {
        state->voice_reverb_force_on_mask[core] |= bit;
        state->voice_reverb_force_off_mask[core] &= ~bit;
    } else {
        state->voice_reverb_force_off_mask[core] |= bit;
        state->voice_reverb_force_on_mask[core] &= ~bit;
    }
    unlock_state(state);
    apply_voice_reverb_masks(state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void set_voice_noise_override(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, int enabled)
{
    uint32_t bit;

    if (state == NULL || core >= 2u || voice >= 24u) {
        return;
    }

    bit = 1u << voice;
    lock_state(state);
    if (enabled) {
        state->voice_noise_force_on_mask[core] |= bit;
        state->voice_noise_force_off_mask[core] &= ~bit;
    } else {
        state->voice_noise_force_off_mask[core] |= bit;
        state->voice_noise_force_on_mask[core] &= ~bit;
    }
    unlock_state(state);
    apply_voice_noise_masks(state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void set_voice_pmod_override(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, int enabled)
{
    uint32_t bit;

    if (state == NULL || core >= 2u || voice >= 24u) {
        return;
    }

    bit = 1u << voice;
    lock_state(state);
    if (enabled) {
        state->voice_pmod_force_on_mask[core] |= bit;
        state->voice_pmod_force_off_mask[core] &= ~bit;
    } else {
        state->voice_pmod_force_off_mask[core] |= bit;
        state->voice_pmod_force_on_mask[core] &= ~bit;
    }
    unlock_state(state);
    if (!enabled) {
        psf2log_restore_imported_voice_pitch(core, voice);
    }
    apply_voice_pmod_masks(state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static int current_voice_effect_enabled(PlayerState *state, unsigned core, unsigned voice, unsigned column)
{
    uint32_t bit;
    uint32_t flags;
    int enabled = 0;

    if (state == NULL || core >= 2u || voice >= 24u) {
        return 0;
    }

    bit = 1u << voice;
    lock_state(state);
    flags = state->live.voices[core][voice].flags;
    if (column == VOICE_FLAG_COLUMN_NOISE) {
        enabled = (flags & SPU2LOG_VOICE_NOISE) != 0;
        if ((state->voice_noise_force_on_mask[core] & bit) != 0) {
            enabled = 1;
        }
        if ((state->voice_noise_force_off_mask[core] & bit) != 0) {
            enabled = 0;
        }
    } else if (column == VOICE_FLAG_COLUMN_PMOD) {
        enabled = (flags & SPU2LOG_VOICE_PMOD) != 0;
        if ((state->voice_pmod_force_on_mask[core] & bit) != 0) {
            enabled = 1;
        }
        if ((state->voice_pmod_force_off_mask[core] & bit) != 0) {
            enabled = 0;
        }
    } else if (column == VOICE_FLAG_COLUMN_REVERB) {
        enabled = (flags & SPU2LOG_VOICE_REVERB) != 0;
        if ((state->voice_reverb_force_on_mask[core] & bit) != 0) {
            enabled = 1;
        }
        if ((state->voice_reverb_force_off_mask[core] & bit) != 0) {
            enabled = 0;
        }
    }
    unlock_state(state);
    return enabled;
}

static void toggle_voice_effect_override(HWND hwnd, PlayerState *state, unsigned core, unsigned voice, unsigned column)
{
    int enabled;

    enabled = !current_voice_effect_enabled(state, core, voice, column);
    if (column == VOICE_FLAG_COLUMN_NOISE) {
        set_voice_noise_override(hwnd, state, core, voice, enabled);
    } else if (column == VOICE_FLAG_COLUMN_PMOD) {
        set_voice_pmod_override(hwnd, state, core, voice, enabled);
    } else if (column == VOICE_FLAG_COLUMN_REVERB) {
        set_voice_reverb_override(hwnd, state, core, voice, enabled);
    }
}

static void toggle_voice_adsr_force(HWND hwnd, PlayerState *state, unsigned core, unsigned voice)
{
    if (state == NULL || core >= 2u || voice >= 24u) {
        return;
    }

    save_debug_voice_value_if_needed(state, core, voice);
    lock_state(state);
    state->debug_adsr_touched_mask[core] |= (1u << voice);
    state->voice_adsr_force_mask[core] ^= (1u << voice);
    if ((state->voice_adsr_force_mask[core] & (1u << voice)) != 0) {
        state->live.voices[core][voice].envx = 0x7fffu;
        if (state->live.voices[core][voice].adsr_phase == SPU2LOG_ADSR_OFF) {
            state->live.voices[core][voice].adsr_phase = SPU2LOG_ADSR_SUSTAIN;
        }
    }
    unlock_state(state);
    apply_voice_adsr_force_masks(state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void toggle_voice_pitch_lock(HWND hwnd, PlayerState *state, unsigned core, unsigned voice)
{
    uint32_t bit;

    if (state == NULL || core >= 2u || voice >= 24u) {
        return;
    }

    bit = 1u << voice;
    save_debug_voice_value_if_needed(state, core, voice);
    lock_state(state);
    if ((state->voice_pitch_lock_mask[core] & bit) != 0) {
        state->voice_pitch_lock_mask[core] &= ~bit;
        unlock_state(state);
        psf2log_set_imported_voice_pitch_lock(core, voice, 0, 0);
    } else {
        state->voice_pitch_lock_value[core][voice] = state->live.voices[core][voice].pitch;
        state->voice_pitch_lock_mask[core] |= bit;
        {
            uint16_t pitch = state->voice_pitch_lock_value[core][voice];
            unlock_state(state);
            psf2log_set_imported_voice_pitch_lock(core, voice, 1, pitch);
        }
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

static void toggle_voice_mute(HWND hwnd, PlayerState *state, unsigned core, unsigned voice)
{
    if (state == NULL || core >= 2u || voice >= 24u) {
        return;
    }

    lock_state(state);
    state->voice_mute_mask[core] ^= (1u << voice);
    unlock_state(state);
    apply_voice_mute_masks(state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void set_reverb_enabled(HWND hwnd, PlayerState *state, int enabled)
{
    if (state == NULL) {
        return;
    }

    lock_state(state);
    state->reverb_enabled = enabled ? 1 : 0;
    unlock_state(state);

    psf2log_set_imported_reverb_enabled(enabled);
    update_settings_menu_check(hwnd, state);
    update_main_reverb_controls(state);
}

static void set_main_enabled(HWND hwnd, PlayerState *state, int enabled)
{
    if (state == NULL) {
        return;
    }

    lock_state(state);
    state->main_enabled = enabled ? 1 : 0;
    unlock_state(state);

    psf2log_set_imported_main_enabled(enabled);
    update_settings_menu_check(hwnd, state);
    update_main_reverb_controls(state);
}

static void set_text_log_enabled(HWND hwnd, PlayerState *state, int enabled)
{
    if (state == NULL) {
        return;
    }

    lock_state(state);
    state->text_log_enabled = enabled ? 1 : 0;
    unlock_state(state);

    g_text_log_enabled = enabled ? 1 : 0;
    psf2log_set_imported_text_log_enabled(enabled);
    save_text_log_enabled(enabled);
    update_settings_menu_check(hwnd, state);
}

static void toggle_reverb_enabled(HWND hwnd, PlayerState *state)
{
    int enabled;

    if (state == NULL) {
        return;
    }

    lock_state(state);
    enabled = !state->reverb_enabled;
    unlock_state(state);

    set_reverb_enabled(hwnd, state, enabled);
}

static void toggle_main_enabled(HWND hwnd, PlayerState *state)
{
    int enabled;

    if (state == NULL) {
        return;
    }

    lock_state(state);
    enabled = !state->main_enabled;
    unlock_state(state);

    set_main_enabled(hwnd, state, enabled);
}

static void toggle_text_log_enabled(HWND hwnd, PlayerState *state)
{
    int enabled;

    if (state == NULL) {
        return;
    }

    lock_state(state);
    enabled = !state->text_log_enabled;
    unlock_state(state);

    set_text_log_enabled(hwnd, state, enabled);
}

static void set_env_color_index(HWND hwnd, PlayerState *state, int color_index)
{
    if (state == NULL) {
        return;
    }

    state->env_color_index = normalize_color_index(color_index);
    store_active_gauge_colors_to_theme(state);
    save_display_settings(state);
    update_settings_menu_check(hwnd, state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void set_lr_color_index(HWND hwnd, PlayerState *state, int color_index)
{
    if (state == NULL) {
        return;
    }

    state->lr_color_index = normalize_color_index(color_index);
    store_active_gauge_colors_to_theme(state);
    save_display_settings(state);
    update_settings_menu_check(hwnd, state);
    InvalidateRect(hwnd, NULL, TRUE);
}

typedef struct FontDialogState {
    PlayerState *player;
    HFONT dialog_font;
    char selected_face[LF_FACESIZE];
    int selected_size;
    int done;
    int accepted;
} FontDialogState;

static int CALLBACK enum_installed_font_proc(
    const LOGFONTA *logfont,
    const TEXTMETRICA *textmetric,
    DWORD font_type,
    LPARAM lparam)
{
    HWND listbox = (HWND)lparam;
    const char *face;

    (void)textmetric;
    (void)font_type;
    if (logfont == NULL || listbox == NULL) {
        return 1;
    }
    face = logfont->lfFaceName;
    if (face[0] == '@' || face[0] == '\0') {
        return 1;
    }
    if (SendMessageA(listbox, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)face) == LB_ERR) {
        SendMessageA(listbox, LB_ADDSTRING, 0, (LPARAM)face);
    }
    return 1;
}

static void populate_installed_font_list(HWND listbox, const char *current_face)
{
    HDC hdc;
    LOGFONTA logfont;
    int index;

    if (listbox == NULL) {
        return;
    }
    SendMessageA(listbox, LB_RESETCONTENT, 0, 0);
    hdc = GetDC(NULL);
    if (hdc != NULL) {
        ZeroMemory(&logfont, sizeof(logfont));
        logfont.lfCharSet = DEFAULT_CHARSET;
        EnumFontFamiliesExA(hdc, &logfont, enum_installed_font_proc, (LPARAM)listbox, 0);
        ReleaseDC(NULL, hdc);
    }
    if (current_face != NULL && current_face[0] != '\0') {
        index = (int)SendMessageA(listbox, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)current_face);
        if (index != LB_ERR) {
            SendMessageA(listbox, LB_SETCURSEL, (WPARAM)index, 0);
        }
    }
}

static void populate_font_size_combo(HWND combo, int current_size)
{
    int size;
    int selected = 1;

    if (combo == NULL) {
        return;
    }
    SendMessageA(combo, CB_RESETCONTENT, 0, 0);
    for (size = 8; size <= 16; ++size) {
        char text[8];
        snprintf(text, sizeof(text), "%d", size);
        SendMessageA(combo, CB_ADDSTRING, 0, (LPARAM)text);
        if (size == current_size) {
            selected = size - 8;
        }
    }
    SendMessageA(combo, CB_SETCURSEL, selected, 0);
}

static int font_dialog_get_selection(HWND hwnd, FontDialogState *dialog)
{
    HWND listbox;
    HWND combo;
    int index;
    int size_index;

    if (hwnd == NULL || dialog == NULL) {
        return 0;
    }
    listbox = GetDlgItem(hwnd, IDC_FONT_LIST);
    combo = GetDlgItem(hwnd, IDC_FONT_SIZE);
    index = (int)SendMessageA(listbox, LB_GETCURSEL, 0, 0);
    if (index == LB_ERR) {
        MessageBoxA(hwnd, "Select a font.", "Font", MB_OK | MB_ICONWARNING);
        return 0;
    }
    SendMessageA(listbox, LB_GETTEXT, (WPARAM)index, (LPARAM)dialog->selected_face);
    size_index = (int)SendMessageA(combo, CB_GETCURSEL, 0, 0);
    dialog->selected_size = (size_index >= 0) ? (size_index + 8) : 9;
    return 1;
}

static void apply_fixed_font_to_font_dialog(
    HWND hwnd,
    FontDialogState *dialog,
    const char *face)
{
    HFONT new_font;
    HFONT old_font;

    if (hwnd == NULL || dialog == NULL) {
        return;
    }
    new_font = create_ui_font_from_face(face, 10);
    if (new_font == NULL) {
        return;
    }
    old_font = dialog->dialog_font;
    dialog->dialog_font = new_font;
    apply_ui_font_to_window(hwnd, dialog->dialog_font);
    if (old_font != NULL) {
        DeleteObject(old_font);
    }
}

static void refresh_header_after_font_change(PlayerState *state)
{
    if (state == NULL || state->hwnd == NULL) {
        return;
    }
    layout_player_controls(state->hwnd, state);
    state->time_label_base_width = 0;
    state->time_label_cache_valid = 0;
    state->time_label_text[0] = '\0';
    update_time_label(state);
    layout_player_controls(state->hwnd, state);
}

static void apply_font_dialog_selection(HWND hwnd, FontDialogState *dialog)
{
    if (dialog == NULL || dialog->player == NULL) {
        return;
    }
    if (!font_dialog_get_selection(hwnd, dialog)) {
        return;
    }
    dialog->player->ui_font_path[0] = '\0';
    snprintf(dialog->player->ui_font_face, sizeof(dialog->player->ui_font_face), "%s", dialog->selected_face);
    dialog->player->ui_font_size = dialog->selected_size;
    rebuild_ui_font(dialog->player);
    save_ui_font_settings(dialog->player);
    apply_ui_font_to_window(dialog->player->hwnd, dialog->player->ui_font);
    refresh_header_after_font_change(dialog->player);
    apply_fixed_font_to_font_dialog(
        hwnd, dialog, dialog->player->ui_font_face);
    if (dialog->player->playlist_hwnd != NULL) {
        apply_ui_font_to_window(dialog->player->playlist_hwnd, dialog->player->ui_font);
    }
}

static LRESULT CALLBACK font_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    FontDialogState *dialog = (FontDialogState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCTA *create = (CREATESTRUCTA *)lparam;
        HWND listbox;
        HWND combo;

        dialog = (FontDialogState *)create->lpCreateParams;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)dialog);
        listbox = CreateWindowA("LISTBOX", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_BORDER,
            10, 10, 300, 230, hwnd, (HMENU)(UINT_PTR)IDC_FONT_LIST, NULL, NULL);
        CreateWindowA("STATIC", "Size", WS_CHILD | WS_VISIBLE,
            10, 250, 42, 18, hwnd, NULL, NULL, NULL);
        combo = CreateWindowA("COMBOBOX", "",
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
            54, 246, 70, 120, hwnd, (HMENU)(UINT_PTR)IDC_FONT_SIZE, NULL, NULL);
        CreateWindowA("BUTTON", "OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_OWNERDRAW,
            72, 280, 74, 24, hwnd, (HMENU)(UINT_PTR)IDC_FONT_OK, NULL, NULL);
        CreateWindowA("BUTTON", "Apply", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            154, 280, 74, 24, hwnd, (HMENU)(UINT_PTR)IDC_FONT_APPLY, NULL, NULL);
        CreateWindowA("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            236, 280, 74, 24, hwnd, (HMENU)(UINT_PTR)IDC_FONT_CANCEL, NULL, NULL);
        if (dialog != NULL) {
            apply_window_dark_title(hwnd, dialog->player);
            apply_fixed_font_to_font_dialog(
                hwnd, dialog, dialog->player->ui_font_face);
            populate_installed_font_list(listbox, dialog->player->ui_font_face);
            populate_font_size_combo(combo, dialog->player->ui_font_size);
        }
        return 0;
    }
    case WM_ERASEBKGND:
        if (dialog != NULL && dialog->player != NULL && is_dark_theme_active(dialog->player)) {
            RECT rect;
            GetClientRect(hwnd, &rect);
            fill_player_background((HDC)wparam, &rect, dialog->player);
            return 1;
        }
        break;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
        LRESULT result = themed_dialog_control_color(wparam, dialog != NULL ? dialog->player : NULL);
        if (result != 0) {
            return result;
        }
        break;
    }
    case WM_DRAWITEM:
        if (draw_owner_button(lparam, dialog != NULL ? dialog->player : NULL)) {
            return TRUE;
        }
        break;
    case WM_COMMAND:
        if (dialog != NULL && LOWORD(wparam) == IDC_FONT_APPLY) {
            apply_font_dialog_selection(hwnd, dialog);
            return 0;
        }
        if (dialog != NULL &&
            (LOWORD(wparam) == IDC_FONT_OK ||
             (LOWORD(wparam) == IDC_FONT_LIST && HIWORD(wparam) == LBN_DBLCLK))) {
            if (!font_dialog_get_selection(hwnd, dialog)) {
                return 0;
            }
            apply_font_dialog_selection(hwnd, dialog);
            dialog->accepted = 1;
            dialog->done = 1;
            save_font_dialog_bounds(hwnd);
            DestroyWindow(hwnd);
            return 0;
        }
        if (dialog != NULL && LOWORD(wparam) == IDC_FONT_CANCEL) {
            dialog->done = 1;
            save_font_dialog_bounds(hwnd);
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    case WM_CLOSE:
        if (dialog != NULL) {
            dialog->done = 1;
        }
        save_font_dialog_bounds(hwnd);
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        if (dialog != NULL && dialog->dialog_font != NULL) {
            DeleteObject(dialog->dialog_font);
            dialog->dialog_font = NULL;
        }
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static void choose_ui_font(HWND hwnd, PlayerState *state)
{
    static int class_registered = 0;
    WNDCLASSA wc;
    HWND dialog_hwnd;
    MSG msg;
    FontDialogState dialog;
    int dialog_x;
    int dialog_y;

    if (state == NULL) {
        return;
    }
    if (!class_registered) {
        ZeroMemory(&wc, sizeof(wc));
        wc.lpfnWndProc = font_dialog_proc;
        wc.hInstance = GetModuleHandleA(NULL);
        wc.lpszClassName = "PsfSpuFontDialog";
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        if (!RegisterClassA(&wc)) {
            return;
        }
        class_registered = 1;
    }

    ZeroMemory(&dialog, sizeof(dialog));
    dialog.player = state;
    dialog.selected_size = state->ui_font_size;
    load_font_dialog_bounds(&dialog_x, &dialog_y);
    if (hwnd != NULL) {
        EnableWindow(hwnd, FALSE);
    }
    dialog_hwnd = CreateWindowExA(
        WS_EX_DLGMODALFRAME,
        "PsfSpuFontDialog",
        "Font",
        WS_CAPTION | WS_SYSMENU | WS_POPUP,
        dialog_x,
        dialog_y,
        338,
        346,
        hwnd,
        NULL,
        GetModuleHandleA(NULL),
        &dialog);
    if (dialog_hwnd == NULL) {
        if (hwnd != NULL) {
            EnableWindow(hwnd, TRUE);
        }
        return;
    }
    ShowWindow(dialog_hwnd, SW_SHOW);
    UpdateWindow(dialog_hwnd);

    while (!dialog.done && GetMessageA(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessageA(dialog_hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    if (hwnd != NULL) {
        EnableWindow(hwnd, TRUE);
        SetForegroundWindow(hwnd);
    }
    (void)dialog.accepted;
}

static void color_to_hex(COLORREF color, char *buffer, size_t size)
{
    if (buffer == NULL || size == 0) {
        return;
    }

    snprintf(buffer, size, "#%02X%02X%02X",
        (unsigned)GetRValue(color),
        (unsigned)GetGValue(color),
        (unsigned)GetBValue(color));
}

static int parse_hex_digit(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return -1;
}

static int parse_hex_color(const char *text, COLORREF *out_color)
{
    char digits[7];
    unsigned i;
    unsigned pos = 0;
    int red;
    int green;
    int blue;

    if (text == NULL || out_color == NULL) {
        return 0;
    }

    while (*text == ' ' || *text == '\t') {
        ++text;
    }
    if (*text == '#') {
        ++text;
    } else if (text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        text += 2;
    }

    while (*text != '\0' && pos < 6) {
        if (*text != ' ' && *text != '\t') {
            digits[pos++] = *text;
        }
        ++text;
    }
    if (pos != 6) {
        return 0;
    }
    digits[6] = '\0';

    for (i = 0; i < 6; ++i) {
        if (parse_hex_digit(digits[i]) < 0) {
            return 0;
        }
    }

    red = (parse_hex_digit(digits[0]) << 4) | parse_hex_digit(digits[1]);
    green = (parse_hex_digit(digits[2]) << 4) | parse_hex_digit(digits[3]);
    blue = (parse_hex_digit(digits[4]) << 4) | parse_hex_digit(digits[5]);
    *out_color = RGB(red, green, blue);
    return 1;
}

static uint16_t read_be16(const unsigned char *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t read_be32(const unsigned char *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

static int utf16be_to_ansi(const unsigned char *data, uint16_t length, char *out, size_t out_size)
{
    WCHAR wide[LF_FACESIZE * 2];
    int chars;
    int i;

    if (data == NULL || out == NULL || out_size == 0) {
        return 0;
    }
    chars = (int)(length / 2);
    if (chars <= 0) {
        return 0;
    }
    if (chars >= (int)(sizeof(wide) / sizeof(wide[0]))) {
        chars = (int)(sizeof(wide) / sizeof(wide[0])) - 1;
    }
    for (i = 0; i < chars; ++i) {
        wide[i] = (WCHAR)read_be16(data + (i * 2));
    }
    wide[chars] = L'\0';
    return WideCharToMultiByte(CP_ACP, 0, wide, -1, out, (int)out_size, NULL, NULL) > 0;
}

static int copy_ascii_font_name(const unsigned char *data, uint16_t length, char *out, size_t out_size)
{
    size_t count;

    if (data == NULL || out == NULL || out_size == 0 || length == 0) {
        return 0;
    }
    count = length < out_size - 1 ? length : out_size - 1;
    memcpy(out, data, count);
    out[count] = '\0';
    return 1;
}

static int read_font_family_from_sfnt(const unsigned char *data, size_t size, uint32_t offset, char *out_face, size_t out_size)
{
    uint16_t table_count;
    uint16_t table;
    uint32_t name_offset = 0;
    uint32_t name_length = 0;
    uint16_t name_count;
    uint16_t string_offset;
    uint16_t rec;
    char fallback[LF_FACESIZE];

    if (data == NULL || out_face == NULL || out_size == 0 || offset + 12 > size) {
        return 0;
    }
    fallback[0] = '\0';
    table_count = read_be16(data + offset + 4);
    if (offset + 12u + ((uint32_t)table_count * 16u) > size) {
        return 0;
    }
    for (table = 0; table < table_count; ++table) {
        const unsigned char *entry = data + offset + 12u + ((uint32_t)table * 16u);
        if (memcmp(entry, "name", 4) == 0) {
            name_offset = read_be32(entry + 8);
            name_length = read_be32(entry + 12);
            break;
        }
    }
    if (name_offset == 0 || offset + name_offset + name_length > size || name_length < 6) {
        return 0;
    }

    data += offset + name_offset;
    size = name_length;
    name_count = read_be16(data + 2);
    string_offset = read_be16(data + 4);
    if (6u + ((uint32_t)name_count * 12u) > size || string_offset >= size) {
        return 0;
    }

    for (rec = 0; rec < name_count; ++rec) {
        const unsigned char *entry = data + 6u + ((uint32_t)rec * 12u);
        uint16_t platform = read_be16(entry);
        uint16_t language = read_be16(entry + 4);
        uint16_t name_id = read_be16(entry + 6);
        uint16_t length = read_be16(entry + 8);
        uint16_t str_off = read_be16(entry + 10);
        const unsigned char *str;

        if (name_id != 1 || (uint32_t)string_offset + str_off + length > size) {
            continue;
        }
        str = data + string_offset + str_off;
        if (platform == 3 && (language == 0x0411 || language == 0x0409 || language == 0)) {
            if (utf16be_to_ansi(str, length, out_face, out_size)) {
                return 1;
            }
        }
        if (fallback[0] == '\0') {
            if (platform == 3) {
                utf16be_to_ansi(str, length, fallback, sizeof(fallback));
            } else {
                copy_ascii_font_name(str, length, fallback, sizeof(fallback));
            }
        }
    }
    if (fallback[0] != '\0') {
        snprintf(out_face, out_size, "%s", fallback);
        return 1;
    }
    return 0;
}

static int read_font_family_from_file(const char *font_path, char *out_face, size_t out_size)
{
    FILE *file;
    long size_long;
    unsigned char *data;
    size_t size;
    uint32_t offset = 0;
    int ok = 0;

    if (font_path == NULL || out_face == NULL || out_size == 0) {
        return 0;
    }
    out_face[0] = '\0';
    file = fopen(font_path, "rb");
    if (file == NULL) {
        return 0;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }
    size_long = ftell(file);
    if (size_long <= 0 || size_long > 64L * 1024L * 1024L) {
        fclose(file);
        return 0;
    }
    rewind(file);
    size = (size_t)size_long;
    data = (unsigned char *)malloc(size);
    if (data == NULL) {
        fclose(file);
        return 0;
    }
    if (fread(data, 1, size, file) == size) {
        if (size >= 12 && memcmp(data, "ttcf", 4) == 0 && read_be32(data + 8) > 0 && size >= 16) {
            offset = read_be32(data + 12);
        }
        ok = read_font_family_from_sfnt(data, size, offset, out_face, out_size);
    }
    free(data);
    fclose(file);
    return ok;
}

static HFONT create_ui_font_from_face(const char *face, int point_size)
{
    LOGFONTA logfont;
    HFONT default_font;
    HDC hdc;

    if (point_size < 8 || point_size > 16) {
        point_size = 9;
    }
    ZeroMemory(&logfont, sizeof(logfont));
    default_font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    if (default_font != NULL) {
        GetObjectA(default_font, sizeof(logfont), &logfont);
    }
    hdc = GetDC(NULL);
    if (hdc != NULL) {
        logfont.lfHeight = -MulDiv(point_size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
        ReleaseDC(NULL, hdc);
    } else {
        logfont.lfHeight = -point_size;
    }
    logfont.lfWeight = FW_NORMAL;
    logfont.lfCharSet = DEFAULT_CHARSET;
    if (face != NULL && face[0] != '\0') {
        snprintf(logfont.lfFaceName, sizeof(logfont.lfFaceName), "%s", face);
    }
    return CreateFontIndirectA(&logfont);
}

static BOOL CALLBACK set_child_font_proc(HWND child, LPARAM param)
{
    HFONT font = (HFONT)param;
    SendMessageA(child, WM_SETFONT, (WPARAM)font, TRUE);
    return TRUE;
}

static void apply_ui_font_to_window(HWND hwnd, HFONT font)
{
    if (hwnd == NULL || font == NULL) {
        return;
    }
    SendMessageA(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
    EnumChildWindows(hwnd, set_child_font_proc, (LPARAM)font);
}

static void rebuild_ui_font(PlayerState *state)
{
    HFONT new_font = NULL;

    if (state == NULL) {
        return;
    }
    if (state->ui_font_face[0] != '\0') {
        new_font = create_ui_font_from_face(state->ui_font_face, state->ui_font_size);
    }
    if (state->maximized_ui_font != NULL) {
        DeleteObject(state->maximized_ui_font);
        state->maximized_ui_font = NULL;
        state->maximized_ui_font_scale = 0;
    }
    if (state->ui_font != NULL) {
        DeleteObject(state->ui_font);
        state->ui_font = NULL;
    }
    state->ui_font = new_font;
    g_ui_font = new_font;
}

typedef struct ColorDialogState {
    PlayerState *player;
    COLORREF color;
    COLORREF result;
    int done;
    int accepted;
} ColorDialogState;

static LRESULT CALLBACK color_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    ColorDialogState *dialog = (ColorDialogState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCTA *create = (CREATESTRUCTA *)lparam;
        char code[16];
        dialog = (ColorDialogState *)create->lpCreateParams;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)dialog);

        CreateWindowA("STATIC", "Color code (#RRGGBB)", WS_CHILD | WS_VISIBLE,
            12, 14, 145, 18, hwnd, NULL, NULL, NULL);
        color_to_hex(dialog->color, code, sizeof(code));
        CreateWindowA("EDIT", code, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            158, 11, 92, 22, hwnd, (HMENU)(UINT_PTR)IDC_COLOR_CODE, NULL, NULL);
        CreateWindowA("BUTTON", "Pick...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            12, 46, 74, 24, hwnd, (HMENU)(UINT_PTR)IDC_COLOR_PICK, NULL, NULL);
        CreateWindowA("BUTTON", "OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_OWNERDRAW,
            94, 46, 74, 24, hwnd, (HMENU)(UINT_PTR)IDC_COLOR_OK, NULL, NULL);
        CreateWindowA("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            176, 46, 74, 24, hwnd, (HMENU)(UINT_PTR)IDC_COLOR_CANCEL, NULL, NULL);
        if (dialog != NULL) {
            apply_window_dark_title(hwnd, dialog->player);
            apply_ui_font_to_window(hwnd, dialog->player->ui_font);
        }
        return 0;
    }
    case WM_ERASEBKGND:
        if (dialog != NULL && dialog->player != NULL && is_dark_theme_active(dialog->player)) {
            RECT rect;
            GetClientRect(hwnd, &rect);
            fill_player_background((HDC)wparam, &rect, dialog->player);
            return 1;
        }
        break;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLOREDIT: {
        LRESULT result = themed_dialog_control_color(wparam, dialog != NULL ? dialog->player : NULL);
        if (result != 0) {
            return result;
        }
        break;
    }
    case WM_DRAWITEM:
        if (draw_owner_button(lparam, dialog != NULL ? dialog->player : NULL)) {
            return TRUE;
        }
        break;
    case WM_COMMAND:
        if (dialog != NULL && LOWORD(wparam) == IDC_COLOR_PICK) {
            CHOOSECOLORA choose;
            char code[16];

            ZeroMemory(&choose, sizeof(choose));
            choose.lStructSize = sizeof(choose);
            choose.hwndOwner = hwnd;
            choose.rgbResult = dialog->color;
            choose.lpCustColors = dialog->player->custom_colors;
            choose.Flags = CC_FULLOPEN | CC_RGBINIT;
            apply_app_menu_dark_mode(is_dark_theme_active(dialog->player));
            if (ChooseColorA(&choose)) {
                dialog->color = choose.rgbResult;
                color_to_hex(dialog->color, code, sizeof(code));
                SetDlgItemTextA(hwnd, IDC_COLOR_CODE, code);
            }
            return 0;
        }
        if (dialog != NULL && LOWORD(wparam) == IDC_COLOR_OK) {
            char code[64];
            COLORREF parsed;
            GetDlgItemTextA(hwnd, IDC_COLOR_CODE, code, sizeof(code));
            if (!parse_hex_color(code, &parsed)) {
                MessageBoxA(hwnd, "Use #RRGGBB, for example #50B45F.", "Color code", MB_OK | MB_ICONWARNING);
                return 0;
            }
            dialog->result = parsed;
            dialog->accepted = 1;
            dialog->done = 1;
            save_color_dialog_bounds(hwnd);
            DestroyWindow(hwnd);
            return 0;
        }
        if (dialog != NULL && LOWORD(wparam) == IDC_COLOR_CANCEL) {
            dialog->done = 1;
            save_color_dialog_bounds(hwnd);
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    case WM_CLOSE:
        save_color_dialog_bounds(hwnd);
        if (dialog != NULL) {
            dialog->done = 1;
        }
        DestroyWindow(hwnd);
        return 0;
    }

    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static int choose_custom_gauge_color(HWND owner, PlayerState *state, COLORREF current_color, COLORREF *out_color)
{
    static int class_registered = 0;
    WNDCLASSA wc;
    HWND dialog_hwnd;
    MSG msg;
    ColorDialogState dialog;
    int dialog_x;
    int dialog_y;

    if (state == NULL || out_color == NULL) {
        return 0;
    }

    if (!class_registered) {
        ZeroMemory(&wc, sizeof(wc));
        wc.lpfnWndProc = color_dialog_proc;
        wc.hInstance = GetModuleHandleA(NULL);
        wc.lpszClassName = "Psf2GaugeColorDialog";
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        if (!RegisterClassA(&wc)) {
            return 0;
        }
        class_registered = 1;
    }

    ZeroMemory(&dialog, sizeof(dialog));
    dialog.player = state;
    dialog.color = current_color;
    dialog.result = current_color;
    load_color_dialog_bounds(&dialog_x, &dialog_y);

    if (owner != NULL) {
        EnableWindow(owner, FALSE);
    }
    dialog_hwnd = CreateWindowExA(
        WS_EX_DLGMODALFRAME,
        "Psf2GaugeColorDialog",
        "Custom color",
        WS_CAPTION | WS_SYSMENU | WS_POPUP,
        dialog_x,
        dialog_y,
        278,
        112,
        owner,
        NULL,
        GetModuleHandleA(NULL),
        &dialog);
    if (dialog_hwnd == NULL) {
        if (owner != NULL) {
            EnableWindow(owner, TRUE);
        }
        return 0;
    }

    ShowWindow(dialog_hwnd, SW_SHOW);
    UpdateWindow(dialog_hwnd);

    while (!dialog.done && GetMessageA(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessageA(dialog_hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    if (owner != NULL) {
        EnableWindow(owner, TRUE);
        SetForegroundWindow(owner);
    }

    if (dialog.accepted) {
        *out_color = dialog.result;
        return 1;
    }
    return 0;
}

typedef struct ValueDialogState {
    PlayerState *player;
    unsigned value;
    unsigned max_value;
    unsigned slider_step;
    int slider_pos;
    HexValueChangedCallback on_change;
    void *on_change_user;
    int done;
    int accepted;
} ValueDialogState;

static int parse_hex_u32(const char *text, unsigned *out_value)
{
    unsigned value = 0;
    int digit;
    int has_digit = 0;

    if (text == NULL || out_value == NULL) {
        return 0;
    }
    while (*text == ' ' || *text == '\t') {
        ++text;
    }
    if (text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        text += 2;
    }
    while (*text != '\0') {
        if (*text == ' ' || *text == '\t') {
            ++text;
            continue;
        }
        digit = parse_hex_digit(*text);
        if (digit < 0) {
            return 0;
        }
        value = (value << 4) | (unsigned)digit;
        has_digit = 1;
        ++text;
    }
    if (!has_digit) {
        return 0;
    }
    *out_value = value;
    return 1;
}

static LRESULT CALLBACK value_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    ValueDialogState *dialog = (ValueDialogState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCTA *create = (CREATESTRUCTA *)lparam;
        HWND edit;
        HWND slider;
        char code[16];
        dialog = (ValueDialogState *)create->lpCreateParams;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)dialog);
        CreateWindowA("STATIC", "Hex value", WS_CHILD | WS_VISIBLE,
            12, 14, 70, 18, hwnd, NULL, NULL, NULL);
        snprintf(code, sizeof(code), "%02X", dialog != NULL ? dialog->value : 0u);
        edit = CreateWindowA("EDIT", code, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            84, 11, 62, 22, hwnd, (HMENU)(UINT_PTR)IDC_VALUE_EDIT, NULL, NULL);
        slider = CreateWindowA(TRACKBAR_CLASSA, "", WS_CHILD | WS_VISIBLE | TBS_NOTICKS,
            12, 42, 194, 26, hwnd, (HMENU)(UINT_PTR)IDC_VALUE_SLIDER, NULL, NULL);
        CreateWindowA("BUTTON", "OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_OWNERDRAW,
            32, 78, 62, 24, hwnd, (HMENU)(UINT_PTR)IDOK, NULL, NULL);
        CreateWindowA("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            104, 78, 74, 24, hwnd, (HMENU)(UINT_PTR)IDCANCEL, NULL, NULL);
        if (slider != NULL && dialog != NULL) {
            if (dialog->slider_step > 1u) {
                dialog->slider_pos = 100;
                SendMessageA(slider, TBM_SETRANGEMIN, FALSE, 0);
                SendMessageA(slider, TBM_SETRANGEMAX, TRUE, 200);
                SendMessageA(slider, TBM_SETLINESIZE, 0, 1);
                SendMessageA(slider, TBM_SETPAGESIZE, 0, 16);
                SendMessageA(slider, TBM_SETPOS, TRUE, dialog->slider_pos);
            } else {
                SendMessageA(slider, TBM_SETRANGEMIN, FALSE, 0);
                SendMessageA(slider, TBM_SETRANGEMAX, TRUE, (LPARAM)dialog->max_value);
                SendMessageA(slider, TBM_SETLINESIZE, 0, dialog->max_value >= 0x100u ? 0x10u : 1u);
                SendMessageA(slider, TBM_SETPAGESIZE, 0, dialog->max_value >= 0x100u ? 0x100u : 4u);
                SendMessageA(slider, TBM_SETPOS, TRUE, (LPARAM)dialog->value);
            }
        }
        if (dialog != NULL) {
            apply_window_dark_title(hwnd, dialog->player);
        }
        apply_ui_font_to_window(hwnd, g_ui_font);
        if (edit != NULL) {
            SetFocus(edit);
            SendMessageA(edit, EM_SETSEL, 0, -1);
        }
        return 0;
    }
    case WM_HSCROLL:
        if (dialog != NULL && (HWND)lparam == GetDlgItem(hwnd, IDC_VALUE_SLIDER)) {
            char code[16];
            UINT scroll_code = LOWORD(wparam);
            unsigned value;

            if (scroll_code == SB_THUMBTRACK || scroll_code == SB_THUMBPOSITION) {
                value = (unsigned)HIWORD(wparam);
            } else {
                value = (unsigned)SendMessageA((HWND)lparam, TBM_GETPOS, 0, 0);
            }
            if (dialog->slider_step > 1u) {
                int delta = (int)value - dialog->slider_pos;
                int next_value;
                if (delta == 0) {
                    return 0;
                }
                next_value = (int)dialog->value + (delta * (int)dialog->slider_step);
                if (next_value < 0) {
                    next_value = 0;
                }
                if ((unsigned)next_value > dialog->max_value) {
                    next_value = (int)dialog->max_value;
                }
                value = (unsigned)next_value;
                dialog->slider_pos = 100;
                SendMessageA((HWND)lparam, TBM_SETPOS, TRUE, dialog->slider_pos);
            } else {
                if (value > dialog->max_value) {
                    value = dialog->max_value;
                }
            }
            dialog->value = value;
            snprintf(code, sizeof(code), "%02X", value);
            SetDlgItemTextA(hwnd, IDC_VALUE_EDIT, code);
            if (dialog->on_change != NULL) {
                dialog->on_change(dialog->on_change_user, value);
            }
            return 0;
        }
        break;
    case WM_ERASEBKGND:
        if (dialog != NULL && dialog->player != NULL && is_dark_theme_active(dialog->player)) {
            RECT rect;
            GetClientRect(hwnd, &rect);
            fill_player_background((HDC)wparam, &rect, dialog->player);
            return 1;
        }
        break;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLOREDIT: {
        LRESULT result = themed_dialog_control_color(wparam, dialog != NULL ? dialog->player : NULL);
        if (result != 0) {
            return result;
        }
        break;
    }
    case WM_DRAWITEM:
        if (draw_owner_button(lparam, dialog != NULL ? dialog->player : NULL)) {
            return TRUE;
        }
        break;
    case WM_COMMAND:
        if (dialog != NULL && (LOWORD(wparam) == IDC_VALUE_OK || LOWORD(wparam) == IDOK)) {
            char code[32];
            unsigned value;
            GetDlgItemTextA(hwnd, IDC_VALUE_EDIT, code, sizeof(code));
            if (!parse_hex_u32(code, &value)) {
                MessageBoxA(hwnd, "Use hexadecimal value, for example 0F.", "Value", MB_OK | MB_ICONWARNING);
                return 0;
            }
            if (value > dialog->max_value) {
                value = dialog->max_value;
            }
            dialog->value = value;
            dialog->accepted = 1;
            dialog->done = 1;
            DestroyWindow(hwnd);
            return 0;
        }
        if (dialog != NULL && (LOWORD(wparam) == IDC_VALUE_CANCEL || LOWORD(wparam) == IDCANCEL)) {
            dialog->done = 1;
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    case WM_CLOSE:
        if (dialog != NULL) {
            dialog->done = 1;
        }
        DestroyWindow(hwnd);
        return 0;
    }

    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static int choose_hex_value_ex(HWND owner, unsigned current_value, unsigned max_value, unsigned slider_step, unsigned *out_value, HexValueChangedCallback on_change, void *on_change_user)
{
    static int class_registered = 0;
    WNDCLASSA wc;
    HWND dialog_hwnd;
    RECT owner_rect;
    MSG msg;
    ValueDialogState dialog;
    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;

    if (out_value == NULL) {
        return 0;
    }
    if (!class_registered) {
        ZeroMemory(&wc, sizeof(wc));
        wc.lpfnWndProc = value_dialog_proc;
        wc.hInstance = GetModuleHandleA(NULL);
        wc.lpszClassName = "PsfSpuValueDialog";
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        if (!RegisterClassA(&wc)) {
            return 0;
        }
        class_registered = 1;
    }

    ZeroMemory(&dialog, sizeof(dialog));
    dialog.player = owner != NULL ? (PlayerState *)GetWindowLongPtrA(owner, GWLP_USERDATA) : NULL;
    dialog.value = current_value > max_value ? max_value : current_value;
    dialog.max_value = max_value;
    dialog.slider_step = slider_step == 0u ? 1u : slider_step;
    dialog.on_change = on_change;
    dialog.on_change_user = on_change_user;
    if (owner != NULL && GetWindowRect(owner, &owner_rect)) {
        x = owner_rect.left + 120;
        y = owner_rect.top + 80;
        EnableWindow(owner, FALSE);
    }
    dialog_hwnd = CreateWindowExA(
        WS_EX_DLGMODALFRAME,
        "PsfSpuValueDialog",
        "Value",
        WS_CAPTION | WS_SYSMENU | WS_POPUP,
        x,
        y,
        226,
        148,
        owner,
        NULL,
        GetModuleHandleA(NULL),
        &dialog);
    if (dialog_hwnd == NULL) {
        if (owner != NULL) {
            EnableWindow(owner, TRUE);
        }
        return 0;
    }
    ShowWindow(dialog_hwnd, SW_SHOW);
    UpdateWindow(dialog_hwnd);
    while (!dialog.done && GetMessageA(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessageA(dialog_hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    if (owner != NULL) {
        EnableWindow(owner, TRUE);
        SetForegroundWindow(owner);
    }
    if (dialog.accepted) {
        *out_value = dialog.value;
        return 1;
    }
    return 0;
}

static int choose_hex_value(HWND owner, unsigned current_value, unsigned max_value, unsigned *out_value)
{
    return choose_hex_value_ex(owner, current_value, max_value, 1u, out_value, NULL, NULL);
}

typedef struct ReverbDialogState {
    PlayerState *player;
    HWND owner;
    unsigned core;
    unsigned value_l;
    unsigned value_r;
    int invert_l;
    int invert_r;
    int link;
    int slider_pos_l;
    int slider_pos_r;
    UINT page_repeat_code;
    DWORD page_repeat_tick;
    unsigned page_repeat_count;
    int done;
    int accepted;
} ReverbDialogState;

#define PHASE_MAGNITUDE_MAX 0x3fffu

static unsigned clamp_phase_magnitude_int(int value)
{
    if (value < 0) {
        return 0u;
    }
    if (value > (int)PHASE_MAGNITUDE_MAX) {
        return PHASE_MAGNITUDE_MAX;
    }
    return (unsigned)value;
}

static unsigned phase_encode_value(unsigned magnitude, int inverted)
{
    magnitude = clamp_phase_magnitude_int((int)magnitude);
    if (!inverted || magnitude == 0u) {
        return magnitude;
    }
    return (0x8000u - magnitude) & 0xffffu;
}

static unsigned phase_decode_magnitude(unsigned value, int *inverted)
{
    value &= 0xffffu;
    if (value >= 0x4000u && value <= 0x7fffu) {
        if (inverted != NULL) {
            *inverted = 1;
        }
        return clamp_phase_magnitude_int((int)(0x8000u - value));
    }
    if (inverted != NULL) {
        *inverted = 0;
    }
    return clamp_phase_magnitude_int((int)value);
}

static void set_reverb_dialog_text(HWND hwnd, unsigned side, unsigned value)
{
    char code[16];
    snprintf(code, sizeof(code), "%04X", value & 0xffffu);
    SetDlgItemTextA(hwnd, side == 0u ? IDC_REVERB_EDIT_L : IDC_REVERB_EDIT_R, code);
}

static unsigned clamp_reverb_int(int value)
{
    if (value < 0) {
        return 0u;
    }
    if (value > 0xffff) {
        return 0xffffu;
    }
    return (unsigned)value;
}

static unsigned reverb_value_to_slider_pos(unsigned value)
{
    return value & 0xffffu;
}

static int clamp_slider_pos_int(int pos)
{
    if (pos < 0) {
        return 0;
    }
    if (pos > 200) {
        return 200;
    }
    return pos;
}

static int is_thumb_scroll_code(UINT scroll_code)
{
    return scroll_code == SB_THUMBPOSITION || scroll_code == SB_THUMBTRACK ||
        scroll_code == SB_LINELEFT || scroll_code == SB_LINERIGHT;
}

static int accelerated_page_delta(UINT scroll_code, UINT *repeat_code, DWORD *repeat_tick, unsigned *repeat_count)
{
    DWORD now;
    int sign;

    if (scroll_code == SB_PAGELEFT) {
        sign = -1;
    } else if (scroll_code == SB_PAGERIGHT) {
        sign = 1;
    } else {
        if (repeat_code != NULL) {
            *repeat_code = 0;
        }
        if (repeat_tick != NULL) {
            *repeat_tick = 0;
        }
        if (repeat_count != NULL) {
            *repeat_count = 0;
        }
        return 0;
    }

    now = GetTickCount();
    if (repeat_code != NULL && repeat_tick != NULL && repeat_count != NULL &&
        *repeat_code == scroll_code && now - *repeat_tick < 350u) {
        if (*repeat_count < 8u) {
            *repeat_count += 1u;
        }
    } else if (repeat_count != NULL) {
        *repeat_count = 0;
    }
    if (repeat_code != NULL) {
        *repeat_code = scroll_code;
    }
    if (repeat_tick != NULL) {
        *repeat_tick = now;
    }

    if (repeat_count != NULL && *repeat_count >= 4u) {
        return sign * 0x80;
    }
    if (repeat_count != NULL && *repeat_count >= 2u) {
        return sign * 0x40;
    }
    if (repeat_count != NULL && *repeat_count >= 1u) {
        return sign * 0x20;
    }
    return sign * 0x10;
}

static void apply_reverb_dialog_value(HWND hwnd, ReverbDialogState *dialog, unsigned side, unsigned value)
{
    if (dialog == NULL || dialog->player == NULL || side >= 2u) {
        return;
    }
    value &= 0xffffu;
    if (side == 0u) {
        dialog->value_l = value;
    } else {
        dialog->value_r = value;
    }
    set_reverb_dialog_text(hwnd, side, value);
    set_reverb_value(dialog->owner, dialog->player, dialog->core, side, value);
}

static void adjust_reverb_dialog_slider(HWND hwnd, ReverbDialogState *dialog, unsigned side, HWND slider, WPARAM scroll_wparam)
{
    UINT scroll_code;
    int pos;
    int *base_pos;
    int delta;
    int value_delta;
    int page_delta;
    unsigned old_value;
    unsigned new_value;

    if (dialog == NULL || slider == NULL || side >= 2u) {
        return;
    }

    scroll_code = LOWORD(scroll_wparam);
    if (scroll_code == SB_THUMBTRACK || scroll_code == SB_THUMBPOSITION) {
        pos = (int)(unsigned)HIWORD(scroll_wparam);
    } else {
        pos = (int)SendMessageA(slider, TBM_GETPOS, 0, 0);
    }
    base_pos = side == 0u ? &dialog->slider_pos_l : &dialog->slider_pos_r;
    delta = pos - *base_pos;
    if (delta == 0) {
        return;
    }
    page_delta = accelerated_page_delta(scroll_code, &dialog->page_repeat_code, &dialog->page_repeat_tick, &dialog->page_repeat_count);
    if (page_delta != 0) {
        value_delta = page_delta;
    } else {
        value_delta = delta;
    }
    old_value = side == 0u ? dialog->value_l : dialog->value_r;
    new_value = clamp_reverb_int((int)old_value + value_delta);

    apply_reverb_dialog_value(hwnd, dialog, side, new_value);
    *base_pos = (int)reverb_value_to_slider_pos(new_value);
    if (!is_thumb_scroll_code(scroll_code)) {
        SendMessageA(slider, TBM_SETPOS, TRUE, (LPARAM)*base_pos);
    }

    if (dialog->link) {
        unsigned other_side = side == 0u ? 1u : 0u;
        unsigned other_value = other_side == 0u ? dialog->value_l : dialog->value_r;
        HWND other_slider = GetDlgItem(hwnd, other_side == 0u ? IDC_REVERB_SLIDER_L : IDC_REVERB_SLIDER_R);
        int *other_base_pos = other_side == 0u ? &dialog->slider_pos_l : &dialog->slider_pos_r;
        other_value = clamp_reverb_int((int)other_value + value_delta);
        apply_reverb_dialog_value(hwnd, dialog, other_side, other_value);
        if (other_slider != NULL) {
            *other_base_pos = (int)reverb_value_to_slider_pos(other_value);
            SendMessageA(other_slider, TBM_SETPOS, TRUE, (LPARAM)*other_base_pos);
        }
    }
}

static LRESULT CALLBACK reverb_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    ReverbDialogState *dialog = (ReverbDialogState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCTA *create = (CREATESTRUCTA *)lparam;
        HWND slider_l;
        HWND slider_r;
        HWND link;
        HWND invert_l;
        HWND invert_r;

        dialog = (ReverbDialogState *)create->lpCreateParams;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)dialog);
        CreateWindowA("STATIC", "L", WS_CHILD | WS_VISIBLE, 12, 14, 14, 18, hwnd, NULL, NULL, NULL);
        CreateWindowA("STATIC", "R", WS_CHILD | WS_VISIBLE, 12, 54, 14, 18, hwnd, NULL, NULL, NULL);
        CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            30, 11, 50, 22, hwnd, (HMENU)(UINT_PTR)IDC_REVERB_EDIT_L, NULL, NULL);
        CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            30, 51, 50, 22, hwnd, (HMENU)(UINT_PTR)IDC_REVERB_EDIT_R, NULL, NULL);
        slider_l = CreateWindowA(TRACKBAR_CLASSA, "", WS_CHILD | WS_VISIBLE | TBS_NOTICKS,
            86, 8, 154, 26, hwnd, (HMENU)(UINT_PTR)IDC_REVERB_SLIDER_L, NULL, NULL);
        slider_r = CreateWindowA(TRACKBAR_CLASSA, "", WS_CHILD | WS_VISIBLE | TBS_NOTICKS,
            86, 48, 154, 26, hwnd, (HMENU)(UINT_PTR)IDC_REVERB_SLIDER_R, NULL, NULL);
        invert_l = CreateWindowA("BUTTON", "Inv", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            244, 11, 48, 20, hwnd, (HMENU)(UINT_PTR)IDC_PHASE_INVERT_L, NULL, NULL);
        invert_r = CreateWindowA("BUTTON", "Inv", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            244, 51, 48, 20, hwnd, (HMENU)(UINT_PTR)IDC_PHASE_INVERT_R, NULL, NULL);
        link = CreateWindowA("BUTTON", "Link L/R", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            12, 86, 86, 20, hwnd, (HMENU)(UINT_PTR)IDC_REVERB_LINK, NULL, NULL);
        CreateWindowA("BUTTON", "OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_OWNERDRAW,
            122, 84, 62, 24, hwnd, (HMENU)(UINT_PTR)IDOK, NULL, NULL);
        CreateWindowA("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            194, 84, 74, 24, hwnd, (HMENU)(UINT_PTR)IDCANCEL, NULL, NULL);

        if (dialog != NULL) {
            apply_window_dark_title(hwnd, dialog->player);
            dialog->slider_pos_l = (int)reverb_value_to_slider_pos(dialog->value_l);
            dialog->slider_pos_r = (int)reverb_value_to_slider_pos(dialog->value_r);
            set_reverb_dialog_text(hwnd, 0u, dialog->value_l);
            set_reverb_dialog_text(hwnd, 1u, dialog->value_r);
            if (link != NULL) {
                SendMessageA(link, BM_SETCHECK, dialog->link ? BST_CHECKED : BST_UNCHECKED, 0);
            }
            if (invert_l != NULL) {
                SendMessageA(invert_l, BM_SETCHECK, dialog->invert_l ? BST_CHECKED : BST_UNCHECKED, 0);
            }
            if (invert_r != NULL) {
                SendMessageA(invert_r, BM_SETCHECK, dialog->invert_r ? BST_CHECKED : BST_UNCHECKED, 0);
            }
        }
        if (slider_l != NULL) {
            SendMessageA(slider_l, TBM_SETRANGEMIN, FALSE, 0);
            SendMessageA(slider_l, TBM_SETRANGEMAX, TRUE, 0xffffu);
            SendMessageA(slider_l, TBM_SETLINESIZE, 0, 5);
            SendMessageA(slider_l, TBM_SETPAGESIZE, 0, 0x10);
            SendMessageA(slider_l, TBM_SETPOS, TRUE, dialog != NULL ? dialog->slider_pos_l : 0);
        }
        if (slider_r != NULL) {
            SendMessageA(slider_r, TBM_SETRANGEMIN, FALSE, 0);
            SendMessageA(slider_r, TBM_SETRANGEMAX, TRUE, 0xffffu);
            SendMessageA(slider_r, TBM_SETLINESIZE, 0, 5);
            SendMessageA(slider_r, TBM_SETPAGESIZE, 0, 0x10);
            SendMessageA(slider_r, TBM_SETPOS, TRUE, dialog != NULL ? dialog->slider_pos_r : 0);
        }
        apply_ui_font_to_window(hwnd, g_ui_font);
        return 0;
    }
    case WM_HSCROLL:
        if (dialog != NULL && (HWND)lparam == GetDlgItem(hwnd, IDC_REVERB_SLIDER_L)) {
            adjust_reverb_dialog_slider(hwnd, dialog, 0u, (HWND)lparam, wparam);
            return 0;
        }
        if (dialog != NULL && (HWND)lparam == GetDlgItem(hwnd, IDC_REVERB_SLIDER_R)) {
            adjust_reverb_dialog_slider(hwnd, dialog, 1u, (HWND)lparam, wparam);
            return 0;
        }
        break;
    case WM_ERASEBKGND:
        if (dialog != NULL && dialog->player != NULL && is_dark_theme_active(dialog->player)) {
            RECT rect;
            GetClientRect(hwnd, &rect);
            fill_player_background((HDC)wparam, &rect, dialog->player);
            return 1;
        }
        break;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLOREDIT: {
        LRESULT result = themed_dialog_control_color(wparam, dialog != NULL ? dialog->player : NULL);
        if (result != 0) {
            return result;
        }
        break;
    }
    case WM_DRAWITEM:
        if (draw_owner_button(lparam, dialog != NULL ? dialog->player : NULL)) {
            return TRUE;
        }
        break;
    case WM_COMMAND:
        if (dialog != NULL && LOWORD(wparam) == IDC_REVERB_LINK) {
            dialog->link = SendDlgItemMessageA(hwnd, IDC_REVERB_LINK, BM_GETCHECK, 0, 0) == BST_CHECKED;
            return 0;
        }
        if (dialog != NULL && LOWORD(wparam) == IDC_PHASE_INVERT_L) {
            dialog->invert_l = SendDlgItemMessageA(hwnd, IDC_PHASE_INVERT_L, BM_GETCHECK, 0, 0) == BST_CHECKED;
            dialog->value_l = dialog->invert_l ? (dialog->value_l | 0x8000u) : (dialog->value_l & 0x7fffu);
            apply_reverb_dialog_value(hwnd, dialog, 0u, dialog->value_l);
            dialog->slider_pos_l = (int)reverb_value_to_slider_pos(dialog->value_l);
            SendDlgItemMessageA(hwnd, IDC_REVERB_SLIDER_L, TBM_SETPOS, TRUE, dialog->slider_pos_l);
            return 0;
        }
        if (dialog != NULL && LOWORD(wparam) == IDC_PHASE_INVERT_R) {
            dialog->invert_r = SendDlgItemMessageA(hwnd, IDC_PHASE_INVERT_R, BM_GETCHECK, 0, 0) == BST_CHECKED;
            dialog->value_r = dialog->invert_r ? (dialog->value_r | 0x8000u) : (dialog->value_r & 0x7fffu);
            apply_reverb_dialog_value(hwnd, dialog, 1u, dialog->value_r);
            dialog->slider_pos_r = (int)reverb_value_to_slider_pos(dialog->value_r);
            SendDlgItemMessageA(hwnd, IDC_REVERB_SLIDER_R, TBM_SETPOS, TRUE, dialog->slider_pos_r);
            return 0;
        }
        if (dialog != NULL && (LOWORD(wparam) == IDOK || LOWORD(wparam) == IDC_VALUE_OK)) {
            char code_l[32];
            char code_r[32];
            unsigned value_l;
            unsigned value_r;
            GetDlgItemTextA(hwnd, IDC_REVERB_EDIT_L, code_l, sizeof(code_l));
            GetDlgItemTextA(hwnd, IDC_REVERB_EDIT_R, code_r, sizeof(code_r));
            if (!parse_hex_u32(code_l, &value_l) || !parse_hex_u32(code_r, &value_r)) {
                MessageBoxA(hwnd, "Use hexadecimal value, for example 70B7.", "Reverb", MB_OK | MB_ICONWARNING);
                return 0;
            }
            apply_reverb_dialog_value(hwnd, dialog, 0u, value_l & 0xffffu);
            apply_reverb_dialog_value(hwnd, dialog, 1u, value_r & 0xffffu);
            dialog->accepted = 1;
            dialog->done = 1;
            DestroyWindow(hwnd);
            return 0;
        }
        if (dialog != NULL && (LOWORD(wparam) == IDCANCEL || LOWORD(wparam) == IDC_VALUE_CANCEL)) {
            dialog->done = 1;
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    case WM_CLOSE:
        if (dialog != NULL) {
            dialog->done = 1;
        }
        DestroyWindow(hwnd);
        return 0;
    }

    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static int choose_reverb_values(HWND owner, PlayerState *state, unsigned core, unsigned focus_side, int link_initial)
{
    static int class_registered = 0;
    WNDCLASSA wc;
    HWND dialog_hwnd;
    RECT owner_rect;
    MSG msg;
    ReverbDialogState dialog;
    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;

    if (state == NULL || core >= 2u) {
        return 0;
    }
    if (!class_registered) {
        ZeroMemory(&wc, sizeof(wc));
        wc.lpfnWndProc = reverb_dialog_proc;
        wc.hInstance = GetModuleHandleA(NULL);
        wc.lpszClassName = "PsfSpuReverbDialog";
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        if (!RegisterClassA(&wc)) {
            return 0;
        }
        class_registered = 1;
    }

    ZeroMemory(&dialog, sizeof(dialog));
    dialog.player = state;
    dialog.owner = owner;
    dialog.core = core;
    dialog.value_l = current_reverb_value(state, core, 0u) & 0xffffu;
    dialog.value_r = current_reverb_value(state, core, 1u) & 0xffffu;
    dialog.invert_l = (dialog.value_l & 0x8000u) != 0;
    dialog.invert_r = (dialog.value_r & 0x8000u) != 0;
    dialog.link = link_initial ? 1 : 0;
    (void)focus_side;

    if (owner != NULL && GetWindowRect(owner, &owner_rect)) {
        x = owner_rect.left + 120;
        y = owner_rect.top + 80;
        EnableWindow(owner, FALSE);
    }
    dialog_hwnd = CreateWindowExA(
        WS_EX_DLGMODALFRAME,
        "PsfSpuReverbDialog",
        "Reverb",
        WS_CAPTION | WS_SYSMENU | WS_POPUP,
        x,
        y,
        310,
        150,
        owner,
        NULL,
        GetModuleHandleA(NULL),
        &dialog);
    if (dialog_hwnd == NULL) {
        if (owner != NULL) {
            EnableWindow(owner, TRUE);
        }
        return 0;
    }
    ShowWindow(dialog_hwnd, SW_SHOW);
    UpdateWindow(dialog_hwnd);
    while (!dialog.done && GetMessageA(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessageA(dialog_hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    if (owner != NULL) {
        EnableWindow(owner, TRUE);
        SetForegroundWindow(owner);
    }
    return dialog.accepted;
}

typedef struct VolumeDialogState {
    PlayerState *player;
    HWND owner;
    unsigned core;
    unsigned voice;
    unsigned value_l;
    unsigned value_r;
    int invert_l;
    int invert_r;
    int link;
    int slider_pos_l;
    int slider_pos_r;
    UINT page_repeat_code;
    DWORD page_repeat_tick;
    unsigned page_repeat_count;
    int done;
    int accepted;
} VolumeDialogState;

#define VOLUME_SLIDER_MAX PHASE_MAGNITUDE_MAX

static unsigned clamp_volume_slider_pos_int(int value)
{
    if (value < 0) {
        return 0u;
    }
    if (value > (int)VOLUME_SLIDER_MAX) {
        return VOLUME_SLIDER_MAX;
    }
    return (unsigned)value;
}

static unsigned volume_slider_pos_to_value(unsigned pos)
{
    return clamp_volume_slider_pos_int((int)pos);
}

static void set_volume_dialog_text(HWND hwnd, unsigned side, unsigned value)
{
    char code[16];
    snprintf(code, sizeof(code), "%04X", clamp_volume_slider_pos_int((int)value));
    SetDlgItemTextA(hwnd, side == 0u ? IDC_REVERB_EDIT_L : IDC_REVERB_EDIT_R, code);
}

static unsigned volume_value_to_slider_pos(unsigned value)
{
    return clamp_volume_slider_pos_int((int)value);
}

static void apply_volume_dialog_value(HWND hwnd, VolumeDialogState *dialog, unsigned side, unsigned value)
{
    unsigned encoded;
    int inverted;

    if (dialog == NULL || dialog->player == NULL || side >= 2u) {
        return;
    }
    value = clamp_volume_slider_pos_int((int)value);
    inverted = side == 0u ? dialog->invert_l : dialog->invert_r;
    encoded = phase_encode_value(value, inverted);
    if (side == 0u) {
        dialog->value_l = value;
    } else {
        dialog->value_r = value;
    }
    set_volume_dialog_text(hwnd, side, value);
    set_voice_volume_value(dialog->owner, dialog->player, dialog->core, dialog->voice, side, encoded);
}

static void adjust_volume_dialog_slider(HWND hwnd, VolumeDialogState *dialog, unsigned side, HWND slider, WPARAM scroll_wparam)
{
    UINT scroll_code;
    int pos;
    int *base_pos;
    int delta;
    int pos_delta;
    int page_delta;
    unsigned new_pos;
    unsigned new_value;

    if (dialog == NULL || slider == NULL || side >= 2u) {
        return;
    }

    scroll_code = LOWORD(scroll_wparam);
    if (scroll_code == SB_THUMBTRACK || scroll_code == SB_THUMBPOSITION) {
        pos = (int)(unsigned)HIWORD(scroll_wparam);
    } else {
        pos = (int)SendMessageA(slider, TBM_GETPOS, 0, 0);
    }
    base_pos = side == 0u ? &dialog->slider_pos_l : &dialog->slider_pos_r;
    delta = pos - *base_pos;
    if (delta == 0) {
        return;
    }
    page_delta = accelerated_page_delta(scroll_code, &dialog->page_repeat_code, &dialog->page_repeat_tick, &dialog->page_repeat_count);
    if (page_delta != 0) {
        pos_delta = page_delta;
    } else {
        pos_delta = delta;
    }
    new_pos = clamp_volume_slider_pos_int(*base_pos + pos_delta);
    new_value = volume_slider_pos_to_value(new_pos);

    apply_volume_dialog_value(hwnd, dialog, side, new_value);
    *base_pos = (int)new_pos;
    if (!is_thumb_scroll_code(scroll_code)) {
        SendMessageA(slider, TBM_SETPOS, TRUE, (LPARAM)*base_pos);
    }

    if (dialog->link) {
        unsigned other_side = side == 0u ? 1u : 0u;
        HWND other_slider = GetDlgItem(hwnd, other_side == 0u ? IDC_REVERB_SLIDER_L : IDC_REVERB_SLIDER_R);
        int *other_base_pos = other_side == 0u ? &dialog->slider_pos_l : &dialog->slider_pos_r;
        unsigned other_pos = clamp_volume_slider_pos_int(*other_base_pos + pos_delta);
        unsigned other_value = volume_slider_pos_to_value(other_pos);
        apply_volume_dialog_value(hwnd, dialog, other_side, other_value);
        if (other_slider != NULL) {
            *other_base_pos = (int)other_pos;
            SendMessageA(other_slider, TBM_SETPOS, TRUE, (LPARAM)*other_base_pos);
        }
    }
}

static LRESULT CALLBACK volume_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    VolumeDialogState *dialog = (VolumeDialogState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCTA *create = (CREATESTRUCTA *)lparam;
        HWND slider_l;
        HWND slider_r;
        HWND link;
        HWND invert_l;
        HWND invert_r;

        dialog = (VolumeDialogState *)create->lpCreateParams;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)dialog);
        CreateWindowA("STATIC", "L", WS_CHILD | WS_VISIBLE, 12, 14, 14, 18, hwnd, NULL, NULL, NULL);
        CreateWindowA("STATIC", "R", WS_CHILD | WS_VISIBLE, 12, 54, 14, 18, hwnd, NULL, NULL, NULL);
        CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            30, 11, 50, 22, hwnd, (HMENU)(UINT_PTR)IDC_REVERB_EDIT_L, NULL, NULL);
        CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            30, 51, 50, 22, hwnd, (HMENU)(UINT_PTR)IDC_REVERB_EDIT_R, NULL, NULL);
        slider_l = CreateWindowA(TRACKBAR_CLASSA, "", WS_CHILD | WS_VISIBLE | TBS_NOTICKS,
            86, 8, 154, 26, hwnd, (HMENU)(UINT_PTR)IDC_REVERB_SLIDER_L, NULL, NULL);
        slider_r = CreateWindowA(TRACKBAR_CLASSA, "", WS_CHILD | WS_VISIBLE | TBS_NOTICKS,
            86, 48, 154, 26, hwnd, (HMENU)(UINT_PTR)IDC_REVERB_SLIDER_R, NULL, NULL);
        invert_l = CreateWindowA("BUTTON", "Inv", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            244, 11, 48, 20, hwnd, (HMENU)(UINT_PTR)IDC_PHASE_INVERT_L, NULL, NULL);
        invert_r = CreateWindowA("BUTTON", "Inv", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            244, 51, 48, 20, hwnd, (HMENU)(UINT_PTR)IDC_PHASE_INVERT_R, NULL, NULL);
        link = CreateWindowA("BUTTON", "Link L/R", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            12, 86, 86, 20, hwnd, (HMENU)(UINT_PTR)IDC_REVERB_LINK, NULL, NULL);
        CreateWindowA("BUTTON", "OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_OWNERDRAW,
            122, 84, 62, 24, hwnd, (HMENU)(UINT_PTR)IDOK, NULL, NULL);
        CreateWindowA("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            194, 84, 74, 24, hwnd, (HMENU)(UINT_PTR)IDCANCEL, NULL, NULL);

        if (dialog != NULL) {
            apply_window_dark_title(hwnd, dialog->player);
            dialog->slider_pos_l = (int)volume_value_to_slider_pos(dialog->value_l);
            dialog->slider_pos_r = (int)volume_value_to_slider_pos(dialog->value_r);
            set_volume_dialog_text(hwnd, 0u, dialog->value_l);
            set_volume_dialog_text(hwnd, 1u, dialog->value_r);
            if (link != NULL) {
                SendMessageA(link, BM_SETCHECK, dialog->link ? BST_CHECKED : BST_UNCHECKED, 0);
            }
            if (invert_l != NULL) {
                SendMessageA(invert_l, BM_SETCHECK, dialog->invert_l ? BST_CHECKED : BST_UNCHECKED, 0);
            }
            if (invert_r != NULL) {
                SendMessageA(invert_r, BM_SETCHECK, dialog->invert_r ? BST_CHECKED : BST_UNCHECKED, 0);
            }
        }
        if (slider_l != NULL) {
            SendMessageA(slider_l, TBM_SETRANGEMIN, FALSE, 0);
            SendMessageA(slider_l, TBM_SETRANGEMAX, TRUE, VOLUME_SLIDER_MAX);
            SendMessageA(slider_l, TBM_SETLINESIZE, 0, 5);
            SendMessageA(slider_l, TBM_SETPAGESIZE, 0, 0x10);
            SendMessageA(slider_l, TBM_SETPOS, TRUE, dialog != NULL ? dialog->slider_pos_l : 0);
        }
        if (slider_r != NULL) {
            SendMessageA(slider_r, TBM_SETRANGEMIN, FALSE, 0);
            SendMessageA(slider_r, TBM_SETRANGEMAX, TRUE, VOLUME_SLIDER_MAX);
            SendMessageA(slider_r, TBM_SETLINESIZE, 0, 5);
            SendMessageA(slider_r, TBM_SETPAGESIZE, 0, 0x10);
            SendMessageA(slider_r, TBM_SETPOS, TRUE, dialog != NULL ? dialog->slider_pos_r : 0);
        }
        apply_ui_font_to_window(hwnd, g_ui_font);
        return 0;
    }
    case WM_HSCROLL:
        if (dialog != NULL && (HWND)lparam == GetDlgItem(hwnd, IDC_REVERB_SLIDER_L)) {
            adjust_volume_dialog_slider(hwnd, dialog, 0u, (HWND)lparam, wparam);
            return 0;
        }
        if (dialog != NULL && (HWND)lparam == GetDlgItem(hwnd, IDC_REVERB_SLIDER_R)) {
            adjust_volume_dialog_slider(hwnd, dialog, 1u, (HWND)lparam, wparam);
            return 0;
        }
        break;
    case WM_ERASEBKGND:
        if (dialog != NULL && dialog->player != NULL && is_dark_theme_active(dialog->player)) {
            RECT rect;
            GetClientRect(hwnd, &rect);
            fill_player_background((HDC)wparam, &rect, dialog->player);
            return 1;
        }
        break;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLOREDIT: {
        LRESULT result = themed_dialog_control_color(wparam, dialog != NULL ? dialog->player : NULL);
        if (result != 0) {
            return result;
        }
        break;
    }
    case WM_DRAWITEM:
        if (draw_owner_button(lparam, dialog != NULL ? dialog->player : NULL)) {
            return TRUE;
        }
        break;
    case WM_COMMAND:
        if (dialog != NULL && LOWORD(wparam) == IDC_REVERB_LINK) {
            dialog->link = SendDlgItemMessageA(hwnd, IDC_REVERB_LINK, BM_GETCHECK, 0, 0) == BST_CHECKED;
            return 0;
        }
        if (dialog != NULL && LOWORD(wparam) == IDC_PHASE_INVERT_L) {
            dialog->invert_l = SendDlgItemMessageA(hwnd, IDC_PHASE_INVERT_L, BM_GETCHECK, 0, 0) == BST_CHECKED;
            apply_volume_dialog_value(hwnd, dialog, 0u, dialog->value_l);
            return 0;
        }
        if (dialog != NULL && LOWORD(wparam) == IDC_PHASE_INVERT_R) {
            dialog->invert_r = SendDlgItemMessageA(hwnd, IDC_PHASE_INVERT_R, BM_GETCHECK, 0, 0) == BST_CHECKED;
            apply_volume_dialog_value(hwnd, dialog, 1u, dialog->value_r);
            return 0;
        }
        if (dialog != NULL && (LOWORD(wparam) == IDOK || LOWORD(wparam) == IDC_VALUE_OK)) {
            char code_l[32];
            char code_r[32];
            unsigned value_l;
            unsigned value_r;
            GetDlgItemTextA(hwnd, IDC_REVERB_EDIT_L, code_l, sizeof(code_l));
            GetDlgItemTextA(hwnd, IDC_REVERB_EDIT_R, code_r, sizeof(code_r));
            if (!parse_hex_u32(code_l, &value_l) || !parse_hex_u32(code_r, &value_r)) {
                MessageBoxA(hwnd, "Use hexadecimal value, for example 3FFF.", "Volume", MB_OK | MB_ICONWARNING);
                return 0;
            }
            apply_volume_dialog_value(hwnd, dialog, 0u, clamp_phase_magnitude_int((int)value_l));
            apply_volume_dialog_value(hwnd, dialog, 1u, clamp_phase_magnitude_int((int)value_r));
            dialog->accepted = 1;
            dialog->done = 1;
            DestroyWindow(hwnd);
            return 0;
        }
        if (dialog != NULL && (LOWORD(wparam) == IDCANCEL || LOWORD(wparam) == IDC_VALUE_CANCEL)) {
            dialog->done = 1;
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    case WM_CLOSE:
        if (dialog != NULL) {
            dialog->done = 1;
        }
        DestroyWindow(hwnd);
        return 0;
    }

    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static int choose_voice_volume_values(HWND owner, PlayerState *state, unsigned core, unsigned voice, unsigned focus_side, int link_initial)
{
    static int class_registered = 0;
    WNDCLASSA wc;
    HWND dialog_hwnd;
    RECT owner_rect;
    MSG msg;
    VolumeDialogState dialog;
    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;

    if (state == NULL || core >= 2u || voice >= 24u) {
        return 0;
    }
    if (!class_registered) {
        ZeroMemory(&wc, sizeof(wc));
        wc.lpfnWndProc = volume_dialog_proc;
        wc.hInstance = GetModuleHandleA(NULL);
        wc.lpszClassName = "PsfSpuVolumeDialog";
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        if (!RegisterClassA(&wc)) {
            return 0;
        }
        class_registered = 1;
    }

    ZeroMemory(&dialog, sizeof(dialog));
    dialog.player = state;
    dialog.owner = owner;
    dialog.core = core;
    dialog.voice = voice;
    dialog.value_l = phase_decode_magnitude(current_voice_volume_value(state, core, voice, 0u), &dialog.invert_l);
    dialog.value_r = phase_decode_magnitude(current_voice_volume_value(state, core, voice, 1u), &dialog.invert_r);
    dialog.link = link_initial ? 1 : 0;
    (void)focus_side;

    if (owner != NULL && GetWindowRect(owner, &owner_rect)) {
        x = owner_rect.left + 120;
        y = owner_rect.top + 80;
        EnableWindow(owner, FALSE);
    }
    dialog_hwnd = CreateWindowExA(
        WS_EX_DLGMODALFRAME,
        "PsfSpuVolumeDialog",
        "Vol L/R",
        WS_CAPTION | WS_SYSMENU | WS_POPUP,
        x,
        y,
        310,
        150,
        owner,
        NULL,
        GetModuleHandleA(NULL),
        &dialog);
    if (dialog_hwnd == NULL) {
        if (owner != NULL) {
            EnableWindow(owner, TRUE);
        }
        return 0;
    }
    ShowWindow(dialog_hwnd, SW_SHOW);
    UpdateWindow(dialog_hwnd);
    while (!dialog.done && GetMessageA(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessageA(dialog_hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    if (owner != NULL) {
        EnableWindow(owner, TRUE);
        SetForegroundWindow(owner);
    }
    return dialog.accepted;
}

static void set_env_custom_color(HWND hwnd, PlayerState *state)
{
    COLORREF color;

    if (state == NULL) {
        return;
    }

    color = state->env_custom_color;
    if (!choose_custom_gauge_color(hwnd, state, color, &color)) {
        return;
    }

    state->env_custom_color = color;
    state->env_color_index = CUSTOM_COLOR_INDEX;
    store_active_gauge_colors_to_theme(state);
    save_display_settings(state);
    update_settings_menu_check(hwnd, state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void set_lr_custom_color(HWND hwnd, PlayerState *state)
{
    COLORREF color;

    if (state == NULL) {
        return;
    }

    color = state->lr_custom_color;
    if (!choose_custom_gauge_color(hwnd, state, color, &color)) {
        return;
    }

    state->lr_custom_color = color;
    state->lr_color_index = CUSTOM_COLOR_INDEX;
    store_active_gauge_colors_to_theme(state);
    save_display_settings(state);
    update_settings_menu_check(hwnd, state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void set_performance_profile(HWND hwnd, PlayerState *state, int profile)
{
    if (state == NULL) {
        return;
    }

    state->performance_profile = normalize_performance_profile(profile);
    save_display_settings(state);
    restart_display_timer(hwnd, state);
    update_settings_menu_check(hwnd, state);
    InvalidateRect(hwnd, NULL, FALSE);
}

static const char *adsr_state_label(uint8_t phase, uint16_t adsr1, uint16_t adsr2)
{
    switch (phase) {
    case SPU2LOG_ADSR_ATTACK:
        return (adsr1 & 0x8000u) ? "exp attack" : "lin attack";
    case SPU2LOG_ADSR_DECAY:
        return "decay";
    case SPU2LOG_ADSR_SUSTAIN:
        return (adsr2 & 0x8000u) ? "exp sustain" : "lin sustain";
    case SPU2LOG_ADSR_RELEASE:
        return (adsr2 & 0x0020u) ? "exp release" : "lin release";
    case SPU2LOG_ADSR_OFF:
        return "off";
    default:
        return "unk";
    }
}

static void append_flags(char *buffer, size_t size, uint32_t flags)
{
    buffer[0] = '\0';

    if (flags & SPU2LOG_VOICE_KEY_ON) {
        strncat(buffer, "KEY ", size - strlen(buffer) - 1u);
    }
    if (flags & SPU2LOG_VOICE_RELEASE) {
        strncat(buffer, "REL ", size - strlen(buffer) - 1u);
    }
    if (flags & SPU2LOG_VOICE_NOISE) {
        strncat(buffer, "NOISE ", size - strlen(buffer) - 1u);
    }
    if (flags & SPU2LOG_VOICE_PMOD) {
        strncat(buffer, "PMOD ", size - strlen(buffer) - 1u);
    }
    if (flags & SPU2LOG_VOICE_REVERB) {
        strncat(buffer, "REV ", size - strlen(buffer) - 1u);
    }
    if (flags & SPU2LOG_VOICE_DRY_L) {
        strncat(buffer, "DL ", size - strlen(buffer) - 1u);
    }
    if (flags & SPU2LOG_VOICE_DRY_R) {
        strncat(buffer, "DR ", size - strlen(buffer) - 1u);
    }
    if (flags & SPU2LOG_VOICE_WET_L) {
        strncat(buffer, "WL ", size - strlen(buffer) - 1u);
    }
    if (flags & SPU2LOG_VOICE_WET_R) {
        strncat(buffer, "WR ", size - strlen(buffer) - 1u);
    }
}

static void format_voice_mode_columns(
    char *buffer,
    size_t size,
    uint32_t flags,
    int key_on_pulse,
    int release_pulse,
    uint8_t psf_version)
{
    char modulation = (flags & SPU2LOG_VOICE_PMOD) ? (psf_version == 0x01u ? 'F' : 'P') : '.';

    if (buffer == NULL || size == 0) {
        return;
    }
    if (size < 8u) {
        buffer[0] = '\0';
        return;
    }
    buffer[0] = key_on_pulse ? 'K' : '.';
    buffer[1] = ' ';
    buffer[2] = release_pulse ? 'R' : '.';
    buffer[3] = ' ';
    buffer[4] = (flags & SPU2LOG_VOICE_NOISE) ? 'N' : '.';
    buffer[5] = ' ';
    buffer[6] = modulation;
    buffer[7] = '\0';
}

static void format_adsr_rate_columns(char *buffer, size_t size, uint16_t adsr1, uint16_t adsr2)
{
    static const char hex_digits[] = "0123456789ABCDEF";
    unsigned attack_rate = (unsigned)((adsr1 >> 8) & 0x7fu);
    unsigned decay_rate = (unsigned)((adsr1 >> 4) & 0x0fu);
    unsigned sustain_level = (unsigned)(adsr1 & 0x0fu);
    unsigned sustain_rate = (unsigned)((adsr2 >> 6) & 0x7fu);
    unsigned release_rate = (unsigned)(adsr2 & 0x1fu);

    if (buffer == NULL || size == 0) {
        return;
    }
    if (size < 15u) {
        buffer[0] = '\0';
        return;
    }
    buffer[0] = hex_digits[(attack_rate >> 4) & 0x0fu];
    buffer[1] = hex_digits[attack_rate & 0x0fu];
    buffer[2] = ' ';
    buffer[3] = hex_digits[(decay_rate >> 4) & 0x0fu];
    buffer[4] = hex_digits[decay_rate & 0x0fu];
    buffer[5] = ' ';
    buffer[6] = hex_digits[(sustain_level >> 4) & 0x0fu];
    buffer[7] = hex_digits[sustain_level & 0x0fu];
    buffer[8] = ' ';
    buffer[9] = hex_digits[(sustain_rate >> 4) & 0x0fu];
    buffer[10] = hex_digits[sustain_rate & 0x0fu];
    buffer[11] = ' ';
    buffer[12] = hex_digits[(release_rate >> 4) & 0x0fu];
    buffer[13] = hex_digits[release_rate & 0x0fu];
    buffer[14] = '\0';
}

static void write_hex_u16(char *buffer, uint16_t value)
{
    static const char hex_digits[] = "0123456789ABCDEF";

    buffer[0] = hex_digits[(value >> 12) & 0x0fu];
    buffer[1] = hex_digits[(value >> 8) & 0x0fu];
    buffer[2] = hex_digits[(value >> 4) & 0x0fu];
    buffer[3] = hex_digits[value & 0x0fu];
}

static void format_hex_u16(char buffer[5], uint16_t value)
{
    write_hex_u16(buffer, value);
    buffer[4] = '\0';
}

static const char *adsr_rate_header_for_psf(uint8_t psf_version)
{
    return psf_version == 0x01u ?
        "K R N F Rv AR DR SL SR RR" :
        "K R N P Rv AR DR SL SR RR";
}

static unsigned noise_clock_from_core_flags(uint32_t flags)
{
    return (unsigned)((flags & 0x3f00u) >> 8) & NOISE_CLOCK_MAX;
}

static unsigned noise_clock_frequency_hz(unsigned noise_clock)
{
    unsigned divisor;

    noise_clock &= NOISE_CLOCK_MAX;
    divisor = (NOISE_CLOCK_MAX - noise_clock) + 1u;
    if (divisor == 0u) {
        divisor = 1u;
    }
    return (unsigned)((PLAYER_SAMPLE_RATE + (divisor / 2u)) / divisor);
}

static double psf1_tempo_bpm(
    uint32_t sequence_tempo,
    int early_format,
    int driver_type)
{
    const double divisor = early_format ? 214.998204082 : 218.453333333;

    if (driver_type == PSF1_MUSIC_DRIVER_SONY_SEQ) {
        return sequence_tempo != 0u ? 60000000.0 / (double)sequence_tempo : 0.0;
    }
    return (double)sequence_tempo / divisor;
}

static uint32_t effective_voice_flags(
    uint32_t flags,
    uint32_t reverb_on_mask,
    uint32_t reverb_off_mask,
    uint32_t noise_on_mask,
    uint32_t noise_off_mask,
    uint32_t pmod_on_mask,
    uint32_t pmod_off_mask,
    unsigned voice)
{
    uint32_t bit = 1u << voice;

    if ((reverb_on_mask & bit) != 0) {
        flags |= SPU2LOG_VOICE_REVERB | SPU2LOG_VOICE_WET_L | SPU2LOG_VOICE_WET_R;
    }
    if ((reverb_off_mask & bit) != 0) {
        flags &= ~(SPU2LOG_VOICE_REVERB | SPU2LOG_VOICE_WET_L | SPU2LOG_VOICE_WET_R);
    }
    if ((noise_on_mask & bit) != 0) {
        flags |= SPU2LOG_VOICE_NOISE;
    }
    if ((noise_off_mask & bit) != 0) {
        flags &= ~SPU2LOG_VOICE_NOISE;
    }
    if ((pmod_on_mask & bit) != 0) {
        flags |= SPU2LOG_VOICE_PMOD;
    }
    if ((pmod_off_mask & bit) != 0) {
        flags &= ~SPU2LOG_VOICE_PMOD;
    }
    return flags;
}

static uint32_t clamp_u32(uint32_t value, uint32_t max_value)
{
    return value > max_value ? max_value : value;
}

static uint32_t abs_volume(uint16_t value)
{
    int16_t signed_value = (int16_t)value;
    int32_t magnitude = signed_value < 0 ? -(int32_t)signed_value : (int32_t)signed_value;

    if (magnitude > 0x3fff) {
        magnitude = 0x3fff;
    }

    return (uint32_t)magnitude;
}

static uint32_t ps1_volume_bar_value(uint16_t value, uint16_t other_value)
{
    uint32_t magnitude = abs_volume(value);
    (void)other_value;
    return magnitude;
}

static uint32_t smooth_gauge_value(
    uint32_t current,
    uint32_t target,
    uint32_t max_value,
    int smooth_rise)
{
    uint32_t delta;
    uint32_t step;

    current = clamp_u32(current, max_value);
    target = clamp_u32(target, max_value);
    if (current == target) {
        return current;
    }

    if (target > current) {
        if (!smooth_rise) {
            return target;
        }
        delta = target - current;
        step = (delta + 1u) / 2u;
        return current + (step != 0 ? step : 1u);
    }

    if (target == 0 || current == 0) {
        return target;
    }

    delta = current - target;
    step = (delta + 3u) / 4u;
    return current - (step != 0 ? step : 1u);
}

static void update_gauge_display_values(PlayerState *state, const Spu2LogLiveState *live, uint8_t psf_version)
{
    uint64_t now_ms;
    uint64_t elapsed_ms;
    unsigned smoothing_steps = 1u;
    unsigned core_count;
    int animation_active = 0;
    unsigned core;
    unsigned voice;

    if (state == NULL || live == NULL) {
        return;
    }

    now_ms = GetTickCount64();
    if (state->last_gauge_update_ms != 0u && now_ms >= state->last_gauge_update_ms) {
        elapsed_ms = now_ms - state->last_gauge_update_ms;
        smoothing_steps = (unsigned)((elapsed_ms + PLAYER_TIMER_MS - 1u) / PLAYER_TIMER_MS);
        if (smoothing_steps < 1u) {
            smoothing_steps = 1u;
        } else if (smoothing_steps > 8u) {
            smoothing_steps = 8u;
        }
    }
    state->last_gauge_update_ms = now_ms;
    core_count = psf_version == 0x01u ? 1u : 2u;

    for (core = 0; core < core_count; ++core) {
        for (voice = 0; voice < 24; ++voice) {
            const Spu2LogVoiceSnapshot *v = &live->voices[core][voice];
            uint32_t env_target = v->envx;
            uint32_t vol_l_target;
            uint32_t vol_r_target;
            unsigned attack_rate = (unsigned)((v->adsr1 >> 8) & 0x7fu);
            int smooth_attack =
                v->adsr_phase == SPU2LOG_ADSR_ATTACK && attack_rate < 0x7cu;
            unsigned smoothing_step;

            if (psf_version == 0x01u) {
                vol_l_target = ps1_volume_bar_value(v->vol_l, v->vol_r);
                vol_r_target = ps1_volume_bar_value(v->vol_r, v->vol_l);
            } else {
                vol_l_target = abs_volume(v->vol_l);
                vol_r_target = abs_volume(v->vol_r);
            }

            if (!state->gauge_valid[core][voice] || state->stopped_display) {
                state->gauge_env[core][voice] = env_target;
                state->gauge_vol_l[core][voice] = vol_l_target;
                state->gauge_vol_r[core][voice] = vol_r_target;
                state->gauge_valid[core][voice] = 1u;
                continue;
            }

            for (smoothing_step = 0; smoothing_step < smoothing_steps; ++smoothing_step) {
                state->gauge_env[core][voice] = smooth_gauge_value(
                    state->gauge_env[core][voice], env_target, 0x7fffu,
                    smooth_attack);
            }
            if (state->gauge_env[core][voice] != env_target) {
                animation_active = 1;
            }
            state->gauge_vol_l[core][voice] = vol_l_target;
            state->gauge_vol_r[core][voice] = vol_r_target;
        }
    }
    state->gauge_animation_active = animation_active;
}

static void format_elapsed_time(char *buffer, size_t size, uint64_t sample_pos, uint32_t sample_rate)
{
    uint64_t tenths;
    unsigned minutes;
    unsigned seconds;
    unsigned tenth;

    if (size == 0) {
        return;
    }

    if (sample_rate == 0) {
        snprintf(buffer, size, "00:00.0");
        return;
    }

    tenths = ((sample_pos * 10u) + (sample_rate / 2u)) / sample_rate;
    minutes = (unsigned)(tenths / 600u);
    seconds = (unsigned)((tenths / 10u) % 60u);
    tenth = (unsigned)(tenths % 10u);

    snprintf(buffer, size, "%02u:%02u.%01u", minutes, seconds, tenth);
}

static int parse_time_tag_seconds(const char *text, double *out_seconds)
{
    double parts[3] = {0.0, 0.0, 0.0};
    unsigned part_count = 0;
    const char *cursor = text;

    if (text == NULL || out_seconds == NULL) {
        return 0;
    }

    while (*cursor != '\0' && part_count < 3) {
        char *end;
        double value;

        while (*cursor == ' ' || *cursor == '\t') {
            ++cursor;
        }
        value = strtod(cursor, &end);
        if (end == cursor || value < 0.0) {
            return 0;
        }
        parts[part_count++] = value;
        cursor = end;
        if (*cursor == ':') {
            ++cursor;
            continue;
        }
        break;
    }
    while (*cursor == ' ' || *cursor == '\t') {
        ++cursor;
    }
    if (*cursor != '\0') {
        return 0;
    }

    if (part_count == 1) {
        *out_seconds = parts[0];
    } else if (part_count == 2) {
        *out_seconds = (parts[0] * 60.0) + parts[1];
    } else if (part_count == 3) {
        *out_seconds = (parts[0] * 3600.0) + (parts[1] * 60.0) + parts[2];
    } else {
        return 0;
    }

    return *out_seconds >= 0.0;
}

typedef struct TimeDialogState {
    PlayerState *player;
    char text[64];
    double seconds;
    int done;
    int accepted;
} TimeDialogState;

static LRESULT CALLBACK time_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    TimeDialogState *dialog = (TimeDialogState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCTA *create = (CREATESTRUCTA *)lparam;
        HWND edit;
        dialog = (TimeDialogState *)create->lpCreateParams;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)dialog);
        CreateWindowA("STATIC", "Time", WS_CHILD | WS_VISIBLE,
            12, 14, 44, 18, hwnd, NULL, NULL, NULL);
        edit = CreateWindowA("EDIT", dialog != NULL ? dialog->text : "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            62, 11, 108, 22, hwnd, (HMENU)(UINT_PTR)IDC_VALUE_EDIT, NULL, NULL);
        CreateWindowA("BUTTON", "OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_OWNERDRAW,
            28, 48, 62, 24, hwnd, (HMENU)(UINT_PTR)IDOK, NULL, NULL);
        CreateWindowA("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            100, 48, 74, 24, hwnd, (HMENU)(UINT_PTR)IDCANCEL, NULL, NULL);
        if (dialog != NULL) {
            apply_window_dark_title(hwnd, dialog->player);
        }
        apply_ui_font_to_window(hwnd, g_ui_font);
        if (edit != NULL) {
            SetFocus(edit);
            SendMessageA(edit, EM_SETSEL, 0, -1);
        }
        return 0;
    }
    case WM_ERASEBKGND:
        if (dialog != NULL && dialog->player != NULL && is_dark_theme_active(dialog->player)) {
            RECT rect;
            GetClientRect(hwnd, &rect);
            fill_player_background((HDC)wparam, &rect, dialog->player);
            return 1;
        }
        break;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLOREDIT: {
        LRESULT result = themed_dialog_control_color(wparam, dialog != NULL ? dialog->player : NULL);
        if (result != 0) {
            return result;
        }
        break;
    }
    case WM_DRAWITEM:
        if (draw_owner_button(lparam, dialog != NULL ? dialog->player : NULL)) {
            return TRUE;
        }
        break;
    case WM_COMMAND:
        if (dialog != NULL && (LOWORD(wparam) == IDOK || LOWORD(wparam) == IDC_VALUE_OK)) {
            char text[64];
            double seconds;
            GetDlgItemTextA(hwnd, IDC_VALUE_EDIT, text, sizeof(text));
            if (!parse_time_tag_seconds(text, &seconds)) {
                MessageBoxA(hwnd, "Use seconds or mm:ss, for example 75 or 1:15.0.", "Seek", MB_OK | MB_ICONWARNING);
                return 0;
            }
            dialog->seconds = seconds;
            dialog->accepted = 1;
            dialog->done = 1;
            DestroyWindow(hwnd);
            return 0;
        }
        if (dialog != NULL && (LOWORD(wparam) == IDCANCEL || LOWORD(wparam) == IDC_VALUE_CANCEL)) {
            dialog->done = 1;
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    case WM_CLOSE:
        if (dialog != NULL) {
            dialog->done = 1;
        }
        DestroyWindow(hwnd);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static int choose_seek_time(HWND owner, PlayerState *state, uint64_t current_sample, double *out_seconds)
{
    static int class_registered = 0;
    WNDCLASSA wc;
    TimeDialogState dialog;
    HWND dialog_hwnd;
    RECT owner_rect;
    MSG msg;
    char current_text[32];
    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;

    if (out_seconds == NULL) {
        return 0;
    }
    if (!class_registered) {
        ZeroMemory(&wc, sizeof(wc));
        wc.lpfnWndProc = time_dialog_proc;
        wc.hInstance = GetModuleHandleA(NULL);
        wc.lpszClassName = "PsfSpuSeekTimeDialog";
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        if (!RegisterClassA(&wc)) {
            return 0;
        }
        class_registered = 1;
    }

    ZeroMemory(&dialog, sizeof(dialog));
    dialog.player = state;
    format_elapsed_time(current_text, sizeof(current_text), current_sample, PLAYER_SAMPLE_RATE);
    snprintf(dialog.text, sizeof(dialog.text), "%s", current_text);

    if (owner != NULL && GetWindowRect(owner, &owner_rect)) {
        x = owner_rect.left + 260;
        y = owner_rect.top + 60;
        EnableWindow(owner, FALSE);
    }
    dialog_hwnd = CreateWindowExA(
        WS_EX_DLGMODALFRAME,
        "PsfSpuSeekTimeDialog",
        "Seek",
        WS_CAPTION | WS_SYSMENU | WS_POPUP,
        x,
        y,
        214,
        116,
        owner,
        NULL,
        GetModuleHandleA(NULL),
        &dialog);
    if (dialog_hwnd == NULL) {
        if (owner != NULL) {
            EnableWindow(owner, TRUE);
        }
        return 0;
    }
    ShowWindow(dialog_hwnd, SW_SHOW);
    UpdateWindow(dialog_hwnd);
    while (!dialog.done && GetMessageA(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessageA(dialog_hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    if (owner != NULL) {
        EnableWindow(owner, TRUE);
        SetForegroundWindow(owner);
    }
    if (dialog.accepted) {
        *out_seconds = dialog.seconds;
        return 1;
    }
    return 0;
}

static void read_psf_timing_samples(
    const char *path,
    uint64_t *out_length_samples,
    uint64_t *out_fade_samples)
{
    PsfFileInfo info;
    PsfFileResult result;
    char *tags;
    char *line;
    uint64_t length_samples = 0;
    uint64_t fade_samples = 0;

    if (out_length_samples != NULL) {
        *out_length_samples = 0;
    }
    if (out_fade_samples != NULL) {
        *out_fade_samples = 0;
    }
    if (path == NULL) {
        return;
    }

    result = psf_file_read_info(path, &info);
    if (result != PSF_FILE_OK || info.tags == NULL) {
        return;
    }

    tags = _strdup(info.tags);
    psf_file_free_info(&info);
    if (tags == NULL) {
        return;
    }

    line = strtok(tags, "\r\n");
    while (line != NULL) {
        char *equals = strchr(line, '=');
        if (equals != NULL) {
            *equals = '\0';
            if (lstrcmpiA(line, "length") == 0 ||
                lstrcmpiA(line, "fade") == 0) {
                double seconds;
                if (parse_time_tag_seconds(equals + 1, &seconds)) {
                    uint64_t samples = (uint64_t)(
                        seconds * (double)PLAYER_SAMPLE_RATE + 0.5);
                    if (lstrcmpiA(line, "length") == 0) {
                        length_samples = samples;
                    } else {
                        fade_samples = samples;
                    }
                }
            }
        }
        line = strtok(NULL, "\r\n");
    }

    free(tags);
    if (out_length_samples != NULL) {
        *out_length_samples = length_samples;
    }
    if (out_fade_samples != NULL) {
        *out_fade_samples = fade_samples;
    }
}

static void trim_tag_value(char *value)
{
    char *end;

    if (value == NULL) {
        return;
    }

    while (*value == ' ' || *value == '\t') {
        memmove(value, value + 1, strlen(value));
    }

    end = value + strlen(value);
    while (end > value && (end[-1] == ' ' || end[-1] == '\t')) {
        --end;
    }
    *end = '\0';
}

static const char *path_basename(const char *path)
{
    const char *slash;
    const char *backslash;

    if (path == NULL) {
        return "";
    }
    slash = strrchr(path, '/');
    backslash = strrchr(path, '\\');
    if (slash == NULL || (backslash != NULL && backslash > slash)) {
        slash = backslash;
    }
    return slash != NULL ? slash + 1 : path;
}

static int runtime_path_to_wide(const char *path, wchar_t *out_path, size_t out_count)
{
    int required;

    if (path == NULL || out_path == NULL || out_count == 0) {
        return 0;
    }
    out_path[0] = L'\0';
    required = MultiByteToWideChar(CP_ACP, 0, path, -1, NULL, 0);
    if (required <= 0 || (size_t)required > out_count) {
        return 0;
    }
    return MultiByteToWideChar(CP_ACP, 0, path, -1, out_path, (int)out_count) > 0;
}

static int wide_path_to_runtime_path(const wchar_t *path, char *out_path, size_t out_size)
{
    wchar_t short_path[MAX_PATH];
    const wchar_t *source = path;
    BOOL used_default = FALSE;
    int required;

    if (path == NULL || out_path == NULL || out_size == 0) {
        return 0;
    }
    out_path[0] = '\0';
    required = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, path, -1,
        NULL, 0, NULL, &used_default);
    if (required <= 0 || used_default || (size_t)required > out_size) {
        DWORD short_length = GetShortPathNameW(path, short_path, MAX_PATH);
        if (short_length == 0 || short_length >= MAX_PATH) {
            return 0;
        }
        source = short_path;
        used_default = FALSE;
        required = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, source, -1,
            NULL, 0, NULL, &used_default);
        if (required <= 0 || used_default || (size_t)required > out_size) {
            return 0;
        }
    }
    return WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, source, -1,
        out_path, (int)out_size, NULL, NULL) > 0;
}

static uint8_t read_psf_header_version(const char *path)
{
    unsigned char header[4];
    FILE *file;

    if (path == NULL || path[0] == '\0') {
        return 0;
    }
    file = fopen(path, "rb");
    if (file == NULL) {
        return 0;
    }
    if (fread(header, 1, sizeof(header), file) != sizeof(header)) {
        fclose(file);
        return 0;
    }
    fclose(file);
    if (header[0] != 'P' || header[1] != 'S' || header[2] != 'F' ||
        (header[3] != 0x01u && header[3] != 0x02u)) {
        return 0;
    }
    return header[3];
}

static int is_psf_music_path(const char *path)
{
    const char *dot;

    if (path == NULL) {
        return 0;
    }
    dot = strrchr(path, '.');
    if (dot != NULL && (lstrcmpiA(dot, ".psflib") == 0 ||
        lstrcmpiA(dot, ".psf2lib") == 0 ||
        lstrcmpiA(dot, ".minipsflib") == 0 ||
        lstrcmpiA(dot, ".minipsf2lib") == 0)) {
        return 0;
    }
    if (dot != NULL && (lstrcmpiA(dot, ".psf") == 0 ||
        lstrcmpiA(dot, ".minipsf") == 0 ||
        lstrcmpiA(dot, ".psf2") == 0 ||
        lstrcmpiA(dot, ".minipsf2") == 0)) {
        return 1;
    }
    return read_psf_header_version(path) != 0;
}

static int is_archive_path(const char *path)
{
    static const char *extensions[] = {
        ".zip", ".7z", ".rar", ".lha", ".lzh"
    };
    size_t path_length;
    size_t i;

    if (path == NULL) {
        return 0;
    }
    path_length = strlen(path);
    for (i = 0; i < sizeof(extensions) / sizeof(extensions[0]); ++i) {
        size_t extension_length = strlen(extensions[i]);
        if (path_length >= extension_length &&
            lstrcmpiA(path + path_length - extension_length, extensions[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_directory_path(const char *path)
{
    DWORD attributes;

    if (path == NULL || path[0] == '\0') {
        return 0;
    }
    attributes = GetFileAttributesA(path);
    return attributes != INVALID_FILE_ATTRIBUTES &&
        (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

static void playlist_refresh_listbox(PlayerState *state)
{
    unsigned i;
    LRESULT list_count;

    if (state == NULL || state->playlist_listbox == NULL) {
        return;
    }
    list_count = SendMessageA(state->playlist_listbox, LB_GETCOUNT, 0, 0);
    if (list_count == (LRESULT)state->playlist_count) {
        SendMessageA(state->playlist_listbox, LB_SETSEL, FALSE, (LPARAM)-1);
        if (state->playlist_current >= 0 && (unsigned)state->playlist_current < state->playlist_count) {
            SendMessageA(state->playlist_listbox, LB_SETSEL, TRUE, (LPARAM)state->playlist_current);
            SendMessageA(state->playlist_listbox, LB_SETCARETINDEX, (WPARAM)state->playlist_current, FALSE);
        }
        return;
    }
    SendMessageA(state->playlist_listbox, LB_RESETCONTENT, 0, 0);
    for (i = 0; i < state->playlist_count; ++i) {
        SendMessageA(state->playlist_listbox, LB_ADDSTRING, 0, (LPARAM)path_basename(state->playlist_paths[i]));
    }
    if (state->playlist_current >= 0 && (unsigned)state->playlist_current < state->playlist_count) {
        SendMessageA(state->playlist_listbox, LB_SETSEL, TRUE, (LPARAM)state->playlist_current);
        SendMessageA(state->playlist_listbox, LB_SETCARETINDEX, (WPARAM)state->playlist_current, FALSE);
    }
}

static int playlist_add_path(PlayerState *state, const char *path)
{
    unsigned i;

    if (state == NULL || path == NULL || path[0] == '\0' || !is_psf_music_path(path)) {
        return 0;
    }
    for (i = 0; i < state->playlist_count; ++i) {
        if (lstrcmpiA(state->playlist_paths[i], path) == 0) {
            return 0;
        }
    }
    if (state->playlist_count >= PLAYLIST_MAX_ITEMS) {
        return 0;
    }
    snprintf(state->playlist_paths[state->playlist_count], MAX_PATH, "%s", path);
    state->playlist_count += 1u;
    playlist_refresh_listbox(state);
    return 1;
}

static void playlist_set_current_path(PlayerState *state, const char *path)
{
    unsigned i;

    if (state == NULL || path == NULL || path[0] == '\0') {
        return;
    }
    for (i = 0; i < state->playlist_count; ++i) {
        if (lstrcmpiA(state->playlist_paths[i], path) == 0) {
            state->playlist_current = (int)i;
            playlist_refresh_listbox(state);
            return;
        }
    }
}

static void playlist_clear(PlayerState *state)
{
    if (state == NULL) {
        return;
    }
    state->playlist_count = 0;
    state->playlist_current = -1;
    state->playlist_pending_play_index = -1;
    playlist_refresh_listbox(state);
}

static void playlist_delete_index(PlayerState *state, int index)
{
    unsigned i;
    unsigned selected;

    if (state == NULL || index < 0 || (unsigned)index >= state->playlist_count) {
        return;
    }
    selected = (unsigned)index;
    for (i = selected; i + 1u < state->playlist_count; ++i) {
        snprintf(state->playlist_paths[i], MAX_PATH, "%s", state->playlist_paths[i + 1u]);
    }
    if (state->playlist_count > 0) {
        state->playlist_count -= 1u;
    }
    if (state->playlist_current == index) {
        state->playlist_current = -1;
    } else if (state->playlist_current > index) {
        state->playlist_current -= 1;
    }
    if (state->playlist_pending_play_index == index) {
        state->playlist_pending_play_index = -1;
    } else if (state->playlist_pending_play_index > index) {
        state->playlist_pending_play_index -= 1;
    }
    playlist_refresh_listbox(state);
    if (state->playlist_listbox != NULL && state->playlist_count > 0) {
        if (selected >= state->playlist_count) {
            selected = state->playlist_count - 1u;
        }
        SendMessageA(state->playlist_listbox, LB_SETSEL, TRUE, (LPARAM)selected);
        SendMessageA(state->playlist_listbox, LB_SETCARETINDEX, (WPARAM)selected, FALSE);
    }
}

static int playlist_selected_index(PlayerState *state)
{
    LRESULT index;

    if (state == NULL || state->playlist_listbox == NULL) {
        return -1;
    }
    index = SendMessageA(state->playlist_listbox, LB_GETCARETINDEX, 0, 0);
    if (index >= 0 && (unsigned)index < state->playlist_count) {
        return (int)index;
    }
    index = SendMessageA(state->playlist_listbox, LB_GETCURSEL, 0, 0);
    if (index >= 0 && (unsigned)index < state->playlist_count) {
        return (int)index;
    }
    return -1;
}

static void playlist_delete_selection(PlayerState *state)
{
    int count;
    int *indices;
    int i;
    int fallback;

    if (state == NULL || state->playlist_listbox == NULL) {
        return;
    }
    count = (int)SendMessageA(state->playlist_listbox, LB_GETSELCOUNT, 0, 0);
    if (count <= 0) {
        playlist_delete_index(state, playlist_selected_index(state));
        return;
    }
    indices = (int *)calloc((size_t)count, sizeof(int));
    if (indices == NULL) {
        return;
    }
    if ((int)SendMessageA(state->playlist_listbox, LB_GETSELITEMS, (WPARAM)count, (LPARAM)indices) == LB_ERR) {
        free(indices);
        return;
    }
    fallback = indices[0];
    for (i = count - 1; i >= 0; --i) {
        playlist_delete_index(state, indices[i]);
    }
    if (state->playlist_listbox != NULL && state->playlist_count > 0) {
        if ((unsigned)fallback >= state->playlist_count) {
            fallback = (int)state->playlist_count - 1;
        }
        SendMessageA(state->playlist_listbox, LB_SETSEL, TRUE, (LPARAM)fallback);
        SendMessageA(state->playlist_listbox, LB_SETCARETINDEX, (WPARAM)fallback, FALSE);
    }
    free(indices);
}

static unsigned playlist_add_folder(PlayerState *state, const char *folder, int replace_existing)
{
    wchar_t folder_w[MAX_PATH];
    wchar_t pattern[MAX_PATH];
    WIN32_FIND_DATAW find_data;
    HANDLE find_handle;
    unsigned added = 0;
    int replaced = 0;

    if (state == NULL || folder == NULL || folder[0] == '\0') {
        return 0;
    }
    if (!runtime_path_to_wide(folder, folder_w, MAX_PATH) ||
        swprintf(pattern, MAX_PATH, L"%ls\\*.*", folder_w) < 0) {
        return 0;
    }
    find_handle = FindFirstFileW(pattern, &find_data);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return 0;
    }
    do {
        if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            wchar_t path_w[MAX_PATH];
            char path[MAX_PATH];
            if (swprintf(path_w, MAX_PATH, L"%ls\\%ls", folder_w, find_data.cFileName) >= 0 &&
                wide_path_to_runtime_path(path_w, path, sizeof(path)) &&
                is_psf_music_path(path)) {
                if (replace_existing && !replaced) {
                    playlist_clear(state);
                    replaced = 1;
                }
                added += playlist_add_path(state, path) ? 1u : 0u;
            }
        }
    } while (FindNextFileW(find_handle, &find_data));
    FindClose(find_handle);
    return added;
}

typedef struct ArchiveMusicPath {
    char path[MAX_PATH];
} ArchiveMusicPath;

static uint64_t archive_hash_bytes(uint64_t hash, const void *data, size_t size)
{
    const unsigned char *bytes = (const unsigned char *)data;
    size_t i;

    for (i = 0; i < size; ++i) {
        hash ^= bytes[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static uint64_t archive_cache_key(const char *path)
{
    WIN32_FILE_ATTRIBUTE_DATA attributes;
    char full_path[MAX_PATH];
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t i;

    full_path[0] = '\0';
    if (GetFullPathNameA(path, sizeof(full_path), full_path, NULL) == 0) {
        snprintf(full_path, sizeof(full_path), "%s", path);
    }
    for (i = 0; full_path[i] != '\0'; ++i) {
        unsigned char ch = (unsigned char)full_path[i];
        if (ch >= 'A' && ch <= 'Z') {
            ch = (unsigned char)(ch - 'A' + 'a');
        }
        hash = archive_hash_bytes(hash, &ch, 1u);
    }
    if (GetFileAttributesExA(path, GetFileExInfoStandard, &attributes)) {
        hash = archive_hash_bytes(hash, &attributes.nFileSizeHigh, sizeof(attributes.nFileSizeHigh));
        hash = archive_hash_bytes(hash, &attributes.nFileSizeLow, sizeof(attributes.nFileSizeLow));
        hash = archive_hash_bytes(hash, &attributes.ftLastWriteTime, sizeof(attributes.ftLastWriteTime));
    }
    return hash;
}

static int archive_regular_file_exists(const char *path)
{
    DWORD attributes = GetFileAttributesA(path);
    return attributes != INVALID_FILE_ATTRIBUTES &&
        (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

static int archive_find_extractor(char *out_path, size_t out_size, int *out_is_7zip)
{
    static const struct {
        const char *path;
        int is_7zip;
    } local_tools[] = {
        {"archive\\bsdtar.exe", 0},
        {"archive\\7z.exe", 1},
        {"archive\\7za.exe", 1},
        {"archive\\7zr.exe", 1},
        {"bsdtar.exe", 0},
        {"7z.exe", 1},
        {"7za.exe", 1},
        {"7zr.exe", 1},
        {"tar.exe", 0}
    };
    static const char *path_names[] = {
        "7z.exe", "7za.exe", "7zr.exe", "bsdtar.exe"
    };
    static const char *program_file_variables[] = {
        "ProgramW6432", "ProgramFiles", "ProgramFiles(x86)", "LOCALAPPDATA"
    };
    char module_path[MAX_PATH];
    char module_folder[MAX_PATH];
    char candidate[MAX_PATH];
    char program_folder[MAX_PATH];
    char system_folder[MAX_PATH];
    char windows_folder[MAX_PATH];
    DWORD found_length;
    size_t i;

    if (out_path == NULL || out_size == 0 || out_is_7zip == NULL) {
        return 0;
    }
    out_path[0] = '\0';
    *out_is_7zip = 0;
    module_path[0] = '\0';
    if (GetModuleFileNameA(NULL, module_path, sizeof(module_path)) != 0) {
        path_parent_folder(module_path, module_folder, sizeof(module_folder));
        for (i = 0; i < sizeof(local_tools) / sizeof(local_tools[0]); ++i) {
            snprintf(candidate, sizeof(candidate), "%s\\%s", module_folder, local_tools[i].path);
            if (archive_regular_file_exists(candidate)) {
                snprintf(out_path, out_size, "%s", candidate);
                *out_is_7zip = local_tools[i].is_7zip;
                return 1;
            }
        }
    }
    for (i = 0; i < sizeof(path_names) / sizeof(path_names[0]); ++i) {
        found_length = SearchPathA(NULL, path_names[i], NULL, sizeof(candidate), candidate, NULL);
        if (found_length != 0 && found_length < sizeof(candidate) &&
            archive_regular_file_exists(candidate)) {
            snprintf(out_path, out_size, "%s", candidate);
            *out_is_7zip = (i < 3u);
            return 1;
        }
    }
    for (i = 0; i < sizeof(program_file_variables) / sizeof(program_file_variables[0]); ++i) {
        if (GetEnvironmentVariableA(program_file_variables[i], program_folder,
                sizeof(program_folder)) == 0) {
            continue;
        }
        if (lstrcmpiA(program_file_variables[i], "LOCALAPPDATA") == 0) {
            snprintf(candidate, sizeof(candidate), "%s\\Programs\\7-Zip\\7z.exe", program_folder);
        } else {
            snprintf(candidate, sizeof(candidate), "%s\\7-Zip\\7z.exe", program_folder);
        }
        if (archive_regular_file_exists(candidate)) {
            snprintf(out_path, out_size, "%s", candidate);
            *out_is_7zip = 1;
            return 1;
        }
    }
    if (GetWindowsDirectoryA(windows_folder, sizeof(windows_folder)) != 0) {
        /* A 32-bit process uses Sysnative to reach 64-bit System32 under WOW64. */
        snprintf(candidate, sizeof(candidate), "%s\\Sysnative\\tar.exe", windows_folder);
        if (archive_regular_file_exists(candidate)) {
            snprintf(out_path, out_size, "%s", candidate);
            return 1;
        }
    }
    if (GetSystemDirectoryA(system_folder, sizeof(system_folder)) != 0) {
        snprintf(candidate, sizeof(candidate), "%s\\tar.exe", system_folder);
        if (archive_regular_file_exists(candidate)) {
            snprintf(out_path, out_size, "%s", candidate);
            return 1;
        }
    }
    return 0;
}

static int archive_prepare_cache(const char *archive_path, char *out_folder, size_t out_size, int *out_ready)
{
    char base[MAX_PATH];
    char root[MAX_PATH];
    char marker[MAX_PATH];
    uint64_t key;
    DWORD attributes;

    if (archive_path == NULL || out_folder == NULL || out_size == 0 || out_ready == NULL) {
        return 0;
    }
    base[0] = '\0';
    if (SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL,
            SHGFP_TYPE_CURRENT, base) != S_OK) {
        if (GetTempPathA(sizeof(base), base) == 0) {
            return 0;
        }
    }
    if (snprintf(root, sizeof(root), "%s\\PSF SPU Player\\ArchiveCache", base) >= (int)sizeof(root)) {
        return 0;
    }
    if (SHCreateDirectoryExA(NULL, root, NULL) != ERROR_SUCCESS &&
        GetFileAttributesA(root) == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }
    key = archive_cache_key(archive_path);
    if (snprintf(out_folder, out_size, "%s\\%016llX", root,
            (unsigned long long)key) >= (int)out_size) {
        return 0;
    }
    snprintf(marker, sizeof(marker), "%s\\.complete", out_folder);
    if (archive_regular_file_exists(marker)) {
        *out_ready = 1;
        return 1;
    }
    attributes = GetFileAttributesA(out_folder);
    if (attributes != INVALID_FILE_ATTRIBUTES) {
        if (snprintf(out_folder, out_size, "%s\\%016llX-%08lX", root,
                (unsigned long long)key, (unsigned long)GetTickCount()) >= (int)out_size) {
            return 0;
        }
    }
    if (SHCreateDirectoryExA(NULL, out_folder, NULL) != ERROR_SUCCESS &&
        GetFileAttributesA(out_folder) == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }
    *out_ready = 0;
    return 1;
}

static int archive_run_extractor(const char *archive_path, const char *output_folder)
{
    char extractor[MAX_PATH];
    char command_line[MAX_PATH * 3 + 128];
    STARTUPINFOA startup;
    PROCESS_INFORMATION process;
    DWORD exit_code = 1;
    HCURSOR previous_cursor;
    int is_7zip;

    if (!archive_find_extractor(extractor, sizeof(extractor), &is_7zip)) {
        player_log("archive extractor not found");
        return 0;
    }
    if (is_7zip) {
        snprintf(command_line, sizeof(command_line),
            "\"%s\" x -y -o\"%s\" \"%s\"",
            extractor, output_folder, archive_path);
    } else {
        snprintf(command_line, sizeof(command_line),
            "\"%s\" -xf \"%s\" -C \"%s\"",
            extractor, archive_path, output_folder);
    }
    player_log("archive extractor begin tool=%s mode=%s output=%s",
        extractor, is_7zip ? "7zip" : "tar", output_folder);
    ZeroMemory(&startup, sizeof(startup));
    ZeroMemory(&process, sizeof(process));
    startup.cb = sizeof(startup);
    previous_cursor = SetCursor(LoadCursorA(NULL, IDC_WAIT));
    if (!CreateProcessA(extractor, command_line, NULL, NULL, FALSE,
            CREATE_NO_WINDOW, NULL, NULL, &startup, &process)) {
        player_log("archive extractor launch failed error=%lu",
            (unsigned long)GetLastError());
        SetCursor(previous_cursor);
        return 0;
    }
    WaitForSingleObject(process.hProcess, INFINITE);
    GetExitCodeProcess(process.hProcess, &exit_code);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    SetCursor(previous_cursor);
    player_log("archive extractor end exit=%lu", (unsigned long)exit_code);
    return exit_code == 0;
}

static void archive_collect_music_files(const char *folder, ArchiveMusicPath *items,
    unsigned *count, unsigned capacity, unsigned depth)
{
    WIN32_FIND_DATAA find_data;
    HANDLE find_handle;
    char pattern[MAX_PATH];

    if (folder == NULL || items == NULL || count == NULL || *count >= capacity || depth > 32u) {
        return;
    }
    if (snprintf(pattern, sizeof(pattern), "%s\\*", folder) >= (int)sizeof(pattern)) {
        return;
    }
    find_handle = FindFirstFileA(pattern, &find_data);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        char path[MAX_PATH];
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }
        if (snprintf(path, sizeof(path), "%s\\%s", folder, find_data.cFileName) >= (int)sizeof(path)) {
            continue;
        }
        if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0) {
                archive_collect_music_files(path, items, count, capacity, depth + 1u);
            }
        } else if (is_psf_music_path(path) && *count < capacity) {
            snprintf(items[*count].path, sizeof(items[*count].path), "%s", path);
            *count += 1u;
        }
    } while (*count < capacity && FindNextFileA(find_handle, &find_data));
    FindClose(find_handle);
}

static int archive_music_path_compare(const void *left, const void *right)
{
    const ArchiveMusicPath *left_path = (const ArchiveMusicPath *)left;
    const ArchiveMusicPath *right_path = (const ArchiveMusicPath *)right;
    return lstrcmpiA(left_path->path, right_path->path);
}

static int archive_add_to_playlist(HWND hwnd, PlayerState *state, const char *archive_path,
    int replace_existing, char *out_first_path, size_t out_first_size)
{
    ArchiveMusicPath *items;
    char output_folder[MAX_PATH];
    char marker[MAX_PATH];
    HANDLE marker_file;
    unsigned count = 0;
    unsigned i;
    int ready = 0;

    if (out_first_path != NULL && out_first_size > 0) {
        out_first_path[0] = '\0';
    }
    if (state == NULL || archive_path == NULL || !is_archive_path(archive_path)) {
        return 0;
    }
    player_log("archive open begin path=%s", archive_path);
    if (!archive_prepare_cache(archive_path, output_folder, sizeof(output_folder), &ready)) {
        MessageBoxA(hwnd, "Could not create the archive cache.", "Archive", MB_OK);
        return 0;
    }
    if (!ready) {
        if (!archive_run_extractor(archive_path, output_folder)) {
            player_log("archive extraction failed path=%s", archive_path);
            MessageBoxA(hwnd,
                "Could not extract this archive. It may be unsupported, damaged, or password protected.",
                "Archive", MB_OK);
            return 0;
        }
        snprintf(marker, sizeof(marker), "%s\\.complete", output_folder);
        marker_file = CreateFileA(marker, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
            FILE_ATTRIBUTE_HIDDEN, NULL);
        if (marker_file != INVALID_HANDLE_VALUE) {
            CloseHandle(marker_file);
        }
    }
    items = (ArchiveMusicPath *)calloc(PLAYLIST_MAX_ITEMS, sizeof(*items));
    if (items == NULL) {
        return 0;
    }
    archive_collect_music_files(output_folder, items, &count, PLAYLIST_MAX_ITEMS, 0u);
    if (count == 0) {
        free(items);
        player_log("archive contains no PSF music path=%s", archive_path);
        MessageBoxA(hwnd, "No PSF music files were found in this archive.", "Archive", MB_OK);
        return 0;
    }
    qsort(items, count, sizeof(*items), archive_music_path_compare);
    if (replace_existing) {
        playlist_clear(state);
    }
    if (out_first_path != NULL && out_first_size > 0) {
        snprintf(out_first_path, out_first_size, "%s", items[0].path);
    }
    for (i = 0; i < count; ++i) {
        playlist_add_path(state, items[i].path);
    }
    free(items);
    playlist_refresh_listbox(state);
    player_log("archive open done path=%s tracks=%u cache=%s", archive_path, count, output_folder);
    return 1;
}

static int add_music_source_to_playlist(HWND hwnd, PlayerState *state, const char *path,
    int replace_existing, char *out_first_path, size_t out_first_size)
{
    unsigned first_index;

    if (out_first_path != NULL && out_first_size > 0) {
        out_first_path[0] = '\0';
    }
    if (is_directory_path(path)) {
        first_index = state != NULL ? state->playlist_count : 0u;
        remember_playlist_folder(state, path);
        if (playlist_add_folder(state, path, replace_existing) == 0) {
            return 0;
        }
        if (out_first_path != NULL && out_first_size > 0 &&
            state != NULL && first_index < state->playlist_count) {
            snprintf(out_first_path, out_first_size, "%s", state->playlist_paths[first_index]);
        }
        return 1;
    }
    if (is_archive_path(path)) {
        return archive_add_to_playlist(hwnd, state, path, replace_existing,
            out_first_path, out_first_size);
    }
    if (!is_psf_music_path(path)) {
        return 0;
    }
    playlist_add_path(state, path);
    if (out_first_path != NULL && out_first_size > 0) {
        snprintf(out_first_path, out_first_size, "%s", path);
    }
    return 1;
}

static void read_psf_title_tags(const char *path, char *game, size_t game_size, char *title, size_t title_size)
{
    PsfFileInfo info;
    PsfFileResult result;
    char *tags;
    char *line;

    if (game != NULL && game_size > 0) {
        game[0] = '\0';
    }
    if (title != NULL && title_size > 0) {
        title[0] = '\0';
    }
    if (path == NULL) {
        return;
    }

    result = psf_file_read_info(path, &info);
    if (result != PSF_FILE_OK || info.tags == NULL) {
        return;
    }

    tags = _strdup(info.tags);
    psf_file_free_info(&info);
    if (tags == NULL) {
        return;
    }

    line = strtok(tags, "\r\n");
    while (line != NULL) {
        char *equals = strchr(line, '=');
        if (equals != NULL) {
            *equals = '\0';
            trim_tag_value(equals + 1);
            if (game != NULL && game_size > 0 && game[0] == '\0' && lstrcmpiA(line, "game") == 0) {
                snprintf(game, game_size, "%s", equals + 1);
            } else if (title != NULL && title_size > 0 && title[0] == '\0' && lstrcmpiA(line, "title") == 0) {
                snprintf(title, title_size, "%s", equals + 1);
            }
        }
        line = strtok(NULL, "\r\n");
    }

    free(tags);
}

static uint8_t read_psf_version(const char *path)
{
    PsfFileInfo info;
    PsfFileResult result;
    uint8_t version = 0;

    if (path == NULL) {
        return 0;
    }

    result = psf_file_read_info(path, &info);
    if (result == PSF_FILE_OK) {
        version = info.version;
        psf_file_free_info(&info);
    }
    if (version != 0x01u && version != 0x02u) {
        version = read_psf_header_version(path);
    }
    return version;
}

static int psf1_keyboard_height_for_state(const PlayerState *state, uint8_t psf_version)
{
    return psf_version == 0x01u && state != NULL &&
        normalize_psf1_display_mode(state->psf1_display_mode) != PSF1_DISPLAY_VALUES ?
        PSF1_TRACK_KEYBOARD_HEIGHT : 0;
}

static int psf1_values_visible_for_state(const PlayerState *state, uint8_t psf_version)
{
    return psf_version != 0x01u || state == NULL ||
        normalize_psf1_display_mode(state->psf1_display_mode) !=
            PSF1_DISPLAY_KEYBOARD_ONLY;
}

static int psf1_keyboard_gap_for_state(const PlayerState *state, uint8_t psf_version)
{
    return psf_version == 0x01u && state != NULL &&
        normalize_psf1_display_mode(state->psf1_display_mode) ==
            PSF1_DISPLAY_KEYBOARD_ONLY ? PSF1_TRACK_KEYBOARD_GAP : 0;
}

static int player_height_for_mode(const PlayerState *state, uint8_t psf_version)
{
    return psf_version == 0x01u && state != NULL ?
        psf1_mode_default_height(state->psf1_display_mode) :
        PLAYER_DEFAULT_HEIGHT;
}

static int psf1_values_keyboard_window_height(HWND hwnd, const PlayerState *state)
{
    RECT window_rect;
    RECT client_rect;
    int line_height;
    int client_height;
    int nonclient_height;

    if (hwnd == NULL || state == NULL ||
        normalize_psf1_display_mode(state->psf1_display_mode) !=
            PSF1_DISPLAY_VALUES_KEYBOARD) {
        return PLAYER_PSF1_HEIGHT;
    }

    line_height = state->meter_line_height > 0 ?
        state->meter_line_height : get_player_display_line_height(hwnd);
    client_height = 2 + CONTROLS_HEIGHT + (line_height * 2) +
        (24 * (line_height + PSF1_TRACK_KEYBOARD_HEIGHT)) + 4;
    if (!GetWindowRect(hwnd, &window_rect) ||
        !GetClientRect(hwnd, &client_rect)) {
        return PLAYER_PSF1_HEIGHT;
    }
    nonclient_height = (window_rect.bottom - window_rect.top) -
        (client_rect.bottom - client_rect.top);
    return client_height + nonclient_height;
}

static int player_values_window_height(HWND hwnd, const PlayerState *state)
{
    RECT window_rect;
    RECT client_rect;
    int line_height;
    int client_height;
    int nonclient_height;

    if (hwnd == NULL) {
        return PLAYER_DEFAULT_HEIGHT;
    }
    line_height = state != NULL && state->meter_line_height > 0 ?
        state->meter_line_height : get_player_display_line_height(hwnd);
    client_height = 2 + CONTROLS_HEIGHT + (line_height * 2) +
        (24 * line_height) + 4;
    if (!GetWindowRect(hwnd, &window_rect) ||
        !GetClientRect(hwnd, &client_rect)) {
        return PLAYER_DEFAULT_HEIGHT;
    }
    nonclient_height = (window_rect.bottom - window_rect.top) -
        (client_rect.bottom - client_rect.top);
    return client_height + nonclient_height;
}

static int preview_keyboard_is_active(const PlayerState *state)
{
    HWND focus;

    if (state == NULL || state->preview_hwnd == NULL ||
        !IsWindow(state->preview_hwnd) ||
        !IsWindowVisible(state->preview_hwnd)) {
        return 0;
    }
    if (GetActiveWindow() == state->preview_hwnd ||
        GetForegroundWindow() == state->preview_hwnd) {
        return 1;
    }
    focus = GetFocus();
    return focus != NULL &&
        GetAncestor(focus, GA_ROOT) == state->preview_hwnd;
}

typedef struct PlayerScaleLayout {
    int base_width;
    int base_height;
    int dest_x;
    int dest_y;
    int dest_width;
    int dest_height;
    int scale_milli;
    int scaled;
} PlayerScaleLayout;

static void update_player_base_client_metrics(HWND hwnd)
{
    RECT window_rect;
    RECT client_rect;

    if (hwnd == NULL || IsZoomed(hwnd) || IsIconic(hwnd) ||
        !GetWindowRect(hwnd, &window_rect) ||
        !GetClientRect(hwnd, &client_rect)) {
        return;
    }
    g_window_nonclient_width =
        (window_rect.right - window_rect.left) -
        (client_rect.right - client_rect.left);
    g_window_nonclient_height =
        (window_rect.bottom - window_rect.top) -
        (client_rect.bottom - client_rect.top);
    g_window_base_client_width = g_window_fixed_width - g_window_nonclient_width;
    g_window_base_client_height = g_window_fixed_height - g_window_nonclient_height;
    if (g_window_base_client_width <= 0) {
        g_window_base_client_width = client_rect.right - client_rect.left;
    }
    if (g_window_base_client_height <= 0) {
        g_window_base_client_height = client_rect.bottom - client_rect.top;
    }
}

static void get_player_scale_layout(HWND hwnd, PlayerScaleLayout *layout)
{
    RECT client_rect;
    int client_width;
    int client_height;
    int width_scale;
    int height_scale;

    if (layout == NULL) {
        return;
    }
    ZeroMemory(layout, sizeof(*layout));
    if (hwnd == NULL || !GetClientRect(hwnd, &client_rect)) {
        return;
    }
    client_width = client_rect.right - client_rect.left;
    client_height = client_rect.bottom - client_rect.top;
    layout->base_width = g_window_base_client_width > 0 ?
        g_window_base_client_width : client_width;
    layout->base_height = g_window_base_client_height > 0 ?
        g_window_base_client_height : client_height;
    layout->scale_milli = 1000;
    layout->dest_width = layout->base_width;
    layout->dest_height = layout->base_height;
    if (IsZoomed(hwnd) && layout->base_width > 0 && layout->base_height > 0) {
        width_scale = (client_width * 1000) / layout->base_width;
        height_scale = (client_height * 1000) / layout->base_height;
        layout->scale_milli = width_scale < height_scale ?
            width_scale : height_scale;
        if (layout->scale_milli <= 0) {
            layout->scale_milli = 1;
        }
        layout->dest_width =
            (layout->base_width * layout->scale_milli + 500) / 1000;
        layout->dest_height =
            (layout->base_height * layout->scale_milli + 500) / 1000;
        layout->dest_x = (client_width - layout->dest_width) / 2;
        layout->dest_y = 0;
        layout->scaled = 1;
    }
}

static void get_player_chrome_scale_layout(
    HWND hwnd,
    const PlayerState *state,
    PlayerScaleLayout *layout)
{
    RECT client_rect;
    int client_width;
    int client_height;
    int width_scale;
    int height_scale;

    get_player_scale_layout(hwnd, layout);
    if (hwnd == NULL || state == NULL || layout == NULL ||
        !IsZoomed(hwnd) || !GetClientRect(hwnd, &client_rect)) {
        return;
    }

    client_width = client_rect.right - client_rect.left;
    client_height = client_rect.bottom - client_rect.top;
    layout->base_width = PLAYER_PSF1_WIDTH - g_window_nonclient_width;
    layout->base_height = PLAYER_PSF1_KEYBOARD_ONLY_HEIGHT -
        g_window_nonclient_height;
    if (layout->base_width <= 0 || layout->base_height <= 0) {
        return;
    }

    width_scale = (client_width * 1000) / layout->base_width;
    height_scale = (client_height * 1000) / layout->base_height;
    layout->scale_milli = width_scale < height_scale ?
        width_scale : height_scale;
    layout->scale_milli =
        (layout->scale_milli * PLAYER_FULLSCREEN_CHROME_PERCENT + 50) / 100;
    if (layout->scale_milli <= 0) {
        layout->scale_milli = 1;
    }
    layout->dest_width =
        (layout->base_width * layout->scale_milli + 500) / 1000;
    layout->dest_height =
        (layout->base_height * layout->scale_milli + 500) / 1000;
    layout->dest_x = (client_width - layout->dest_width) / 2;
    layout->dest_y = 0;
    layout->scaled = 1;
}

static void get_fullscreen_reference_layout(
    HWND hwnd,
    const PlayerState *state,
    PlayerScaleLayout *layout)
{
    PlayerScaleLayout chrome_layout;
    RECT client_rect;
    int client_width;
    int client_height;
    int base_width;
    int base_height;
    int chrome_bottom;
    int width_scale;
    int height_scale;

    if (layout == NULL) {
        return;
    }
    ZeroMemory(layout, sizeof(*layout));
    if (hwnd == NULL || state == NULL || !IsZoomed(hwnd) ||
        !GetClientRect(hwnd, &client_rect)) {
        return;
    }
    get_player_chrome_scale_layout(hwnd, state, &chrome_layout);
    if (!chrome_layout.scaled) {
        return;
    }

    client_width = client_rect.right - client_rect.left;
    client_height = client_rect.bottom - client_rect.top;
    base_width = PLAYER_PSF1_WIDTH - g_window_nonclient_width;
    base_height = PLAYER_PSF1_KEYBOARD_ONLY_HEIGHT -
        g_window_nonclient_height;
    chrome_bottom =
        (CONTROLS_HEIGHT * chrome_layout.scale_milli + 500) / 1000;
    if (base_width <= 0 || base_height <= CONTROLS_HEIGHT + 4 ||
        client_height <= chrome_bottom + 4) {
        return;
    }

    width_scale = (client_width * 1000) / base_width;
    height_scale = ((client_height - chrome_bottom - 4) * 1000) /
        (base_height - CONTROLS_HEIGHT - 4);
    layout->base_width = base_width;
    layout->base_height = base_height;
    layout->scale_milli = width_scale < height_scale ?
        width_scale : height_scale;
    if (layout->scale_milli <= 0) {
        layout->scale_milli = 1;
    }
    layout->dest_width =
        (base_width * layout->scale_milli + 500) / 1000;
    layout->dest_height =
        (base_height * layout->scale_milli + 500) / 1000;
    layout->dest_x = (client_width - layout->dest_width) / 2;
    layout->dest_y = chrome_bottom -
        (CONTROLS_HEIGHT * layout->scale_milli + 500) / 1000;
    layout->scaled = 1;
}

static int get_fullscreen_reference_scale_milli(
    HWND hwnd,
    const PlayerState *state)
{
    PlayerScaleLayout layout;

    get_fullscreen_reference_layout(hwnd, state, &layout);
    return layout.scaled ? layout.scale_milli : 1000;
}

static void get_player_content_scale_layout(
    HWND hwnd,
    const PlayerState *state,
    PlayerScaleLayout *layout)
{
    PlayerScaleLayout chrome_layout;
    RECT client_rect;
    int client_width;
    int client_height;
    int chrome_bottom;
    int content_height;
    int line_height;
    int active_header_scale;
    int active_header_height;
    int width_scale;
    int height_scale;

    get_player_scale_layout(hwnd, layout);
    if (hwnd == NULL || state == NULL || layout == NULL ||
        !IsZoomed(hwnd) || !layout->scaled ||
        !GetClientRect(hwnd, &client_rect)) {
        return;
    }

    get_player_chrome_scale_layout(hwnd, state, &chrome_layout);
    if (!chrome_layout.scaled) {
        return;
    }
    client_width = client_rect.right - client_rect.left;
    client_height = client_rect.bottom - client_rect.top;
    chrome_bottom =
        (CONTROLS_HEIGHT * chrome_layout.scale_milli + 500) / 1000;
    line_height = state->meter_line_height > 0 ?
        state->meter_line_height : get_player_display_line_height(hwnd);
    active_header_scale = get_fullscreen_reference_scale_milli(
        hwnd, state);
    active_header_height =
        (line_height * active_header_scale + 500) / 1000;
    if (state->psf_version == 0x01u &&
        normalize_psf1_display_mode(state->psf1_display_mode) ==
        PSF1_DISPLAY_VALUES_KEYBOARD) {
        int rows_height = 2 + line_height +
            16 * (line_height + PSF1_TRACK_KEYBOARD_HEIGHT);
        int rows_scale = ((client_height - chrome_bottom -
            active_header_height - 4) * 1000) / rows_height;

        width_scale = (client_width * 1000) / layout->base_width;
        layout->scale_milli = width_scale < rows_scale ?
            width_scale : rows_scale;
        if (layout->scale_milli <= 0) {
            layout->scale_milli = 1;
        }
        layout->dest_width =
            (layout->base_width * layout->scale_milli + 500) / 1000;
        layout->dest_height =
            (layout->base_height * layout->scale_milli + 500) / 1000;
        layout->dest_x = (client_width - layout->dest_width) / 2;
        layout->dest_y = chrome_bottom -
            (CONTROLS_HEIGHT * layout->scale_milli + 500) / 1000;
        layout->scaled = 1;
        return;
    }
    content_height = layout->base_height - CONTROLS_HEIGHT -
        line_height - 4;
    if (content_height <= 0 ||
        client_height <= chrome_bottom + active_header_height + 4) {
        return;
    }
    width_scale = (client_width * 1000) / layout->base_width;
    height_scale = ((client_height - chrome_bottom -
        active_header_height - 4) * 1000) / content_height;
    if (state->psf_version == 0x02u) {
        int ps2_content_height = layout->base_height - CONTROLS_HEIGHT - 4;
        int ps2_height_scale = ps2_content_height > 0 ?
            ((client_height - chrome_bottom - 4) * 1000) /
                ps2_content_height : height_scale;
        int horizontal_scale;
        int vertical_scale;

        horizontal_scale = width_scale < ps2_height_scale ?
            width_scale : ps2_height_scale;
        horizontal_scale =
            (horizontal_scale * PLAYER_PSF2_FULLSCREEN_CONTENT_PERCENT + 50) /
                100;
        vertical_scale =
            (horizontal_scale * PLAYER_PSF2_FULLSCREEN_VERTICAL_PERCENT + 50) /
                100;
        if (vertical_scale > ps2_height_scale) {
            vertical_scale = ps2_height_scale;
        }
        if (horizontal_scale <= 0) {
            horizontal_scale = 1;
        }
        if (vertical_scale <= 0) {
            vertical_scale = 1;
        }
        layout->scale_milli = vertical_scale;
        layout->dest_width =
            (layout->base_width * horizontal_scale + 500) / 1000;
        layout->dest_height =
            (layout->base_height * vertical_scale + 500) / 1000;
        layout->dest_x = (client_width - layout->dest_width) / 2;
        layout->dest_y = chrome_bottom -
            (CONTROLS_HEIGHT * vertical_scale + 500) / 1000;
        layout->scaled = 1;
        return;
    }
    layout->scale_milli = width_scale < height_scale ?
        width_scale : height_scale;
    if (layout->scale_milli <= 0) {
        layout->scale_milli = 1;
    }
    layout->dest_width =
        (layout->base_width * layout->scale_milli + 500) / 1000;
    layout->dest_height =
        (layout->base_height * layout->scale_milli + 500) / 1000;
    layout->dest_x = (client_width - layout->dest_width) / 2;
    layout->dest_y = chrome_bottom -
        (CONTROLS_HEIGHT * layout->scale_milli + 500) / 1000;
    layout->scaled = 1;
}

static int maximized_active_header_line_height(
    HWND hwnd,
    const PlayerState *state,
    int line_height)
{
    PlayerScaleLayout content_layout;
    int target_scale;
    int logical_height;

    if (hwnd == NULL || state == NULL || line_height <= 0 ||
        !IsZoomed(hwnd)) {
        return line_height;
    }
    if (state->psf_version == 0x02u) {
        return line_height;
    }
    get_player_content_scale_layout(hwnd, state, &content_layout);
    if (!content_layout.scaled || content_layout.scale_milli <= 0) {
        return line_height;
    }
    target_scale = get_fullscreen_reference_scale_milli(hwnd, state);
    logical_height = (line_height * target_scale +
        content_layout.scale_milli / 2) / content_layout.scale_milli;
    return logical_height > 0 ? logical_height : 1;
}

static int scale_player_value(int value, int scale_milli)
{
    return (value * scale_milli + 500) / 1000;
}

static void position_scaled_control(
    HWND control,
    const PlayerScaleLayout *layout,
    int x,
    int y,
    int width,
    int height)
{
    if (control == NULL || layout == NULL) {
        return;
    }
    SetWindowPos(control, NULL,
        layout->dest_x + scale_player_value(x, layout->scale_milli),
        layout->dest_y + scale_player_value(y, layout->scale_milli),
        scale_player_value(width, layout->scale_milli),
        scale_player_value(height, layout->scale_milli),
        SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
}

static void snap_moving_window_to_monitor(PlayerState *state, RECT *window_rect)
{
    MONITORINFO monitor_info;
    HMONITOR monitor;
    POINT cursor;
    LONG left_distance;
    LONG right_distance;
    LONG top_distance;
    LONG bottom_distance;
    LONG offset_x = 0;
    LONG offset_y = 0;

    if (state == NULL || window_rect == NULL) {
        return;
    }
    if (GetCursorPos(&cursor)) {
        if (!state->window_move_drag_active) {
            state->window_move_drag_active = 1;
            state->window_move_cursor_offset_x = cursor.x - window_rect->left;
            state->window_move_cursor_offset_y = cursor.y - window_rect->top;
            state->window_move_width = window_rect->right - window_rect->left;
            state->window_move_height = window_rect->bottom - window_rect->top;
        }
        if (state->window_move_width > 0 && state->window_move_height > 0) {
            window_rect->left = cursor.x - state->window_move_cursor_offset_x;
            window_rect->top = cursor.y - state->window_move_cursor_offset_y;
            window_rect->right = window_rect->left + state->window_move_width;
            window_rect->bottom = window_rect->top + state->window_move_height;
        }
    }
    monitor = MonitorFromRect(window_rect, MONITOR_DEFAULTTONEAREST);
    if (monitor == NULL) {
        return;
    }
    ZeroMemory(&monitor_info, sizeof(monitor_info));
    monitor_info.cbSize = sizeof(monitor_info);
    if (!GetMonitorInfoA(monitor, &monitor_info)) {
        return;
    }

    left_distance = window_rect->left - monitor_info.rcWork.left;
    if (left_distance < 0) {
        left_distance = -left_distance;
    }
    right_distance = window_rect->right - monitor_info.rcWork.right;
    if (right_distance < 0) {
        right_distance = -right_distance;
    }
    top_distance = window_rect->top - monitor_info.rcWork.top;
    if (top_distance < 0) {
        top_distance = -top_distance;
    }
    bottom_distance = window_rect->bottom - monitor_info.rcWork.bottom;
    if (bottom_distance < 0) {
        bottom_distance = -bottom_distance;
    }

    if (left_distance <= PLAYER_WINDOW_SNAP_DISTANCE ||
        right_distance <= PLAYER_WINDOW_SNAP_DISTANCE) {
        if (left_distance <= right_distance) {
            offset_x = monitor_info.rcWork.left - window_rect->left;
        } else {
            offset_x = monitor_info.rcWork.right - window_rect->right;
        }
    }
    if (top_distance <= PLAYER_WINDOW_SNAP_DISTANCE ||
        bottom_distance <= PLAYER_WINDOW_SNAP_DISTANCE) {
        if (top_distance <= bottom_distance) {
            offset_y = monitor_info.rcWork.top - window_rect->top;
        } else {
            offset_y = monitor_info.rcWork.bottom - window_rect->bottom;
        }
    }
    if (offset_x != 0 || offset_y != 0) {
        OffsetRect(window_rect, offset_x, offset_y);
    }
}

static HFONT player_control_font_for_scale(
    PlayerState *state,
    int scale_milli)
{
    HFONT base_font;
    LOGFONTA logfont;
    int scaled_height;

    if (state == NULL) {
        return NULL;
    }
    base_font = state->ui_font != NULL ?
        state->ui_font : (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    if (scale_milli <= 1000) {
        if (state->maximized_ui_font != NULL) {
            DeleteObject(state->maximized_ui_font);
            state->maximized_ui_font = NULL;
            state->maximized_ui_font_scale = 0;
        }
        return base_font;
    }
    if (state->maximized_ui_font != NULL &&
        state->maximized_ui_font_scale == scale_milli) {
        return state->maximized_ui_font;
    }
    if (state->maximized_ui_font != NULL) {
        DeleteObject(state->maximized_ui_font);
        state->maximized_ui_font = NULL;
    }
    ZeroMemory(&logfont, sizeof(logfont));
    if (GetObjectA(base_font, sizeof(logfont), &logfont) == 0) {
        return base_font;
    }
    scaled_height = (logfont.lfHeight * scale_milli) / 1000;
    if (scaled_height == 0) {
        scaled_height = logfont.lfHeight < 0 ? -1 : 1;
    }
    logfont.lfHeight = scaled_height;
    logfont.lfWidth = 0;
    state->maximized_ui_font = CreateFontIndirectA(&logfont);
    state->maximized_ui_font_scale = scale_milli;
    return state->maximized_ui_font != NULL ?
        state->maximized_ui_font : base_font;
}

static int logical_text_width(
    HDC hdc,
    const char *text,
    int scale_milli,
    int fallback)
{
    SIZE size;

    if (hdc == NULL || text == NULL || text[0] == '\0' ||
        !GetTextExtentPoint32A(hdc, text, (int)strlen(text), &size)) {
        return fallback;
    }
    if (scale_milli <= 0) {
        return size.cx;
    }
    return (size.cx * 1000 + scale_milli / 2) / scale_milli;
}

static int control_text_width(
    HDC hdc,
    HWND control,
    const char *fallback_text,
    int scale_milli,
    int fallback_width)
{
    char text[128];

    text[0] = '\0';
    if (control != NULL) {
        GetWindowTextA(control, text, sizeof(text));
    }
    if (text[0] == '\0' && fallback_text != NULL) {
        snprintf(text, sizeof(text), "%s", fallback_text);
    }
    return logical_text_width(
        hdc, text, scale_milli, fallback_width);
}

static void layout_player_controls(HWND hwnd, PlayerState *state)
{
    PlayerScaleLayout layout;
    HFONT control_font;
    int spacing_step;
    int time_width;

    if (hwnd == NULL || state == NULL) {
        return;
    }
    get_player_chrome_scale_layout(hwnd, state, &layout);
    if (layout.base_width <= 0 || layout.base_height <= 0) {
        return;
    }
    control_font = player_control_font_for_scale(state, layout.scale_milli);
    if (control_font != NULL) {
        apply_ui_font_to_window(hwnd, control_font);
    }
    time_width = state->time_label_base_width > 0 ?
        state->time_label_base_width : TIME_LABEL_MIN_WIDTH;
    if (state->ui_font_size <= 12 || state->psf_version == 0x02u) {
        spacing_step = IsZoomed(hwnd) ? 2 : 0;
        position_scaled_control(state->open_button, &layout, 6, 4, 54, 20);
        position_scaled_control(state->play_button, &layout,
            64 + spacing_step, 4, 54, 20);
        position_scaled_control(state->pause_button, &layout,
            122 + spacing_step * 2, 4, 58, 20);
        position_scaled_control(state->stop_button, &layout,
            184 + spacing_step * 3, 4, 54, 20);
        position_scaled_control(state->speed_slider, &layout,
            252 + spacing_step * 4, 1, 128, 26);
        position_scaled_control(state->speed_label, &layout,
            386 + spacing_step * 5, 6, 94, 16);
        position_scaled_control(state->volume_label, &layout,
            484 + spacing_step * 6, 6, 68, 16);
        position_scaled_control(state->main_check, &layout,
            556 + spacing_step * 7, 4, 58, 20);
        position_scaled_control(state->reverb_check, &layout,
            618 + spacing_step * 8, 4, 70, 20);
        state->time_label_base_x = 694 + spacing_step * 9;
        state->time_label_base_y = 6;
        state->time_label_base_height = 16;
        position_scaled_control(state->time_label, &layout,
            state->time_label_base_x, state->time_label_base_y,
            time_width, state->time_label_base_height);
    } else {
        HDC hdc = GetDC(hwnd);
        HFONT old_font = NULL;
        TEXTMETRICA metrics;
        int text_height = 16;
        int open_width;
        int play_width;
        int pause_width;
        int stop_width;
        int speed_width;
        int volume_width;
        int speed_value_width;
        int volume_value_width;
        int main_width;
        int reverb_width;
        int fixed_width;
        int inner_width;
        int gap = 4;
        int slider_width = 128;
        int button_height;
        int label_height;
        int slider_height;
        int button_y;
        int label_y;
        int slider_y;
        int x = 6;
        int transition_step = state->ui_font_size - 12;

        if (transition_step < 1) {
            transition_step = 1;
        } else if (transition_step > 4) {
            transition_step = 4;
        }

        ZeroMemory(&metrics, sizeof(metrics));
        if (hdc != NULL && control_font != NULL) {
            old_font = (HFONT)SelectObject(hdc, control_font);
        }
        if (hdc != NULL && GetTextMetricsA(hdc, &metrics)) {
            text_height = layout.scale_milli > 0 ?
                (metrics.tmHeight * 1000 + layout.scale_milli / 2) /
                    layout.scale_milli : metrics.tmHeight;
        }
        open_width = control_text_width(
            hdc, state->open_button, "Open", layout.scale_milli, 44) + 10;
        play_width = control_text_width(
            hdc, state->play_button, "Play", layout.scale_milli, 44) + 10;
        pause_width = control_text_width(
            hdc, state->pause_button, "Pause", layout.scale_milli, 48) + 10;
        stop_width = control_text_width(
            hdc, state->stop_button, "Stop", layout.scale_milli, 44) + 10;
        speed_width = 94;
        volume_width = 68;
        speed_value_width = logical_text_width(
            hdc, "200%", layout.scale_milli, 38) + 2;
        volume_value_width = speed_value_width;
        main_width = control_text_width(
            hdc, state->main_check, "Main", layout.scale_milli, 38) + 20;
        reverb_width = control_text_width(
            hdc, state->reverb_check, "Reverb", layout.scale_milli, 50) + 20;
        time_width = control_text_width(
            hdc, state->time_label, "00:00.0 / --:--.-",
            layout.scale_milli, TIME_LABEL_MIN_WIDTH) + 2;
        if (old_font != NULL && old_font != HGDI_ERROR) {
            SelectObject(hdc, old_font);
        }
        if (hdc != NULL) {
            ReleaseDC(hwnd, hdc);
        }

        fixed_width = open_width + play_width + pause_width + stop_width +
            speed_width + volume_width + main_width + reverb_width +
            time_width;
        inner_width = layout.base_width - 12;
        gap = (inner_width - fixed_width - slider_width) / 9;
        if (gap > 4) {
            gap = 4;
        } else if (gap < 0) {
            gap = 0;
        }
        if (fixed_width + slider_width + gap * 9 > inner_width) {
            open_width -= 6;
            play_width -= 6;
            pause_width -= 6;
            stop_width -= 6;
            speed_width -= 3;
            volume_width -= 3;
            main_width -= 4;
            reverb_width -= 4;
            fixed_width = open_width + play_width + pause_width + stop_width +
                speed_width + volume_width + main_width + reverb_width +
                time_width;
        }
        if (fixed_width + slider_width > inner_width) {
            int overflow = fixed_width + slider_width - inner_width;
            int reducible = speed_width - speed_value_width;
            int reduction = overflow < reducible ? overflow : reducible;

            speed_width -= reduction;
            overflow -= reduction;
            if (overflow > 0) {
                reducible = volume_width - volume_value_width;
                reduction = overflow < reducible ? overflow : reducible;
                volume_width -= reduction;
            }
            fixed_width = open_width + play_width + pause_width + stop_width +
                speed_width + volume_width + main_width + reverb_width +
                time_width;
        }
        {
            int row_width = fixed_width + slider_width + gap * 9;
            if (row_width < inner_width) {
                int centered_offset = (inner_width - row_width) / 2;
                x += (centered_offset * transition_step + 2) / 4;
            }
        }

        button_height = text_height + 6;
        if (button_height < 20) {
            button_height = 20;
        } else if (button_height > CONTROLS_HEIGHT) {
            button_height = CONTROLS_HEIGHT;
        }
        label_height = text_height + 2;
        if (label_height < 16) {
            label_height = 16;
        } else if (label_height > CONTROLS_HEIGHT) {
            label_height = CONTROLS_HEIGHT;
        }
        slider_height = button_height > 26 ? button_height : 26;
        if (slider_height > CONTROLS_HEIGHT) {
            slider_height = CONTROLS_HEIGHT;
        }
        button_y = (CONTROLS_HEIGHT - button_height) / 2;
        label_y = (CONTROLS_HEIGHT - label_height) / 2;
        slider_y = (CONTROLS_HEIGHT - slider_height) / 2;

#define PLACE_HEADER_CONTROL(control, width, y_pos, height) \
        do { \
            position_scaled_control((control), &layout, x, (y_pos), \
                (width), (height)); \
            x += (width) + gap; \
        } while (0)
        PLACE_HEADER_CONTROL(state->open_button, open_width,
            button_y, button_height);
        PLACE_HEADER_CONTROL(state->play_button, play_width,
            button_y, button_height);
        PLACE_HEADER_CONTROL(state->pause_button, pause_width,
            button_y, button_height);
        PLACE_HEADER_CONTROL(state->stop_button, stop_width,
            button_y, button_height);
        PLACE_HEADER_CONTROL(state->speed_slider, slider_width,
            slider_y, slider_height);
        position_scaled_control(state->speed_label, &layout,
            x, label_y, speed_width, label_height);
        x += speed_width + gap;
        position_scaled_control(state->volume_label, &layout,
            x, label_y, volume_width, label_height);
        x += volume_width + gap;
        PLACE_HEADER_CONTROL(state->main_check, main_width,
            button_y, button_height);
        PLACE_HEADER_CONTROL(state->reverb_check, reverb_width,
            button_y, button_height);
#undef PLACE_HEADER_CONTROL
        state->time_label_base_x = x;
        state->time_label_base_y = label_y;
        state->time_label_base_height = label_height;
        state->time_label_base_width = time_width;
        position_scaled_control(state->time_label, &layout,
            state->time_label_base_x, state->time_label_base_y,
            time_width, state->time_label_base_height);
    }
    RedrawWindow(hwnd, NULL, NULL,
        RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}

static void apply_psf_window_mode(HWND hwnd, const PlayerState *state, uint8_t psf_version)
{
    int width = psf_version == 0x01u ? PLAYER_PSF1_WIDTH : PLAYER_DEFAULT_WIDTH;
    int height = player_height_for_mode(state, psf_version);
    int x = 0;
    int y = 0;
    UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE;

    if (hwnd == NULL) {
        return;
    }

    if (psf_version == 0x01u && state != NULL) {
        int display_mode = normalize_psf1_display_mode(
            state->psf1_display_mode);

        load_psf1_mode_window_size(
            state->psf1_display_mode, &width, &height);
        if (load_psf1_mode_window_position(
                state->psf1_display_mode, &x, &y)) {
            flags &= ~SWP_NOMOVE;
        }
        if (display_mode == PSF1_DISPLAY_VALUES_KEYBOARD) {
            height = psf1_values_keyboard_window_height(hwnd, state);
        } else if (display_mode == PSF1_DISPLAY_VALUES) {
            height = player_values_window_height(hwnd, state);
        }
    } else {
        height = player_values_window_height(hwnd, state);
    }
    if (g_main_restore_position_valid &&
        (IsZoomed(hwnd) || g_main_restore_position_active)) {
        x = g_main_restore_x;
        y = g_main_restore_y;
        flags &= ~SWP_NOMOVE;
    }

    g_window_fixed_width = width;
    g_window_fixed_height = height;
    if (IsZoomed(hwnd) || IsIconic(hwnd)) {
        WINDOWPLACEMENT placement;

        ZeroMemory(&placement, sizeof(placement));
        placement.length = sizeof(placement);
        if (GetWindowPlacement(hwnd, &placement)) {
            if ((flags & SWP_NOMOVE) != 0) {
                x = placement.rcNormalPosition.left;
                y = placement.rcNormalPosition.top;
            }
            placement.rcNormalPosition.left = x;
            placement.rcNormalPosition.top = y;
            placement.rcNormalPosition.right = x + width;
            placement.rcNormalPosition.bottom = y + height;
            SetWindowPlacement(hwnd, &placement);
        }
        g_window_base_client_width = width - g_window_nonclient_width;
        g_window_base_client_height = height - g_window_nonclient_height;
        layout_player_controls(hwnd, (PlayerState *)state);
        InvalidateRect(hwnd, NULL, TRUE);
        return;
    }
    SetWindowPos(hwnd, NULL, x, y, width, height, flags);
    update_player_base_client_metrics(hwnd);
    layout_player_controls(hwnd, (PlayerState *)state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void toggle_player_maximized(HWND hwnd)
{
    if (hwnd == NULL || IsIconic(hwnd)) {
        return;
    }
    ShowWindow(hwnd, IsZoomed(hwnd) ? SW_RESTORE : SW_MAXIMIZE);
}

static void set_psf1_display_mode(
    HWND hwnd,
    PlayerState *state,
    int display_mode)
{
    display_mode = normalize_psf1_display_mode(display_mode);
    if (state == NULL || state->psf_version != 0x01u) {
        return;
    }
    if (display_mode == normalize_psf1_display_mode(
            state->psf1_display_mode)) {
        update_settings_menu_check(hwnd, state);
        return;
    }
    save_psf1_mode_window_bounds(hwnd, state->psf1_display_mode);
    state->psf1_display_mode = display_mode;
    save_psf1_display_mode(state->psf1_display_mode);
    update_settings_menu_check(hwnd, state);
    state->scroll_y = 0;
    apply_psf_window_mode(hwnd, state, state->psf_version);
    update_scrollbar(hwnd, state);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void cycle_psf1_display_mode(HWND hwnd, PlayerState *state)
{
    int display_mode;

    if (state == NULL) {
        return;
    }
    display_mode =
        (normalize_psf1_display_mode(state->psf1_display_mode) + 1) %
        PSF1_DISPLAY_MODE_COUNT;
    set_psf1_display_mode(hwnd, state, display_mode);
}

static void update_time_label(PlayerState *state)
{
    char elapsed[32];
    char total[32];
    char label[80];
    uint64_t sample_pos;
    uint64_t total_samples;
    uint64_t displayed_tenths;

    if (state == NULL || state->time_label == NULL) {
        return;
    }

    lock_state(state);
    if (state->frame_advance && state->frame_live_valid) {
        sample_pos = timeline_sample_at_locked(state, state->frame_live.last_sample_pos);
    } else if (state->audible_display_valid) {
        sample_pos = state->audible_display_snapshot.timeline_sample_pos;
    } else if (state->playing) {
        sample_pos = 0;
    } else {
        sample_pos = timeline_sample_at_locked(state, state->live.last_sample_pos);
    }
    total_samples = state->total_samples;
    displayed_tenths = ((sample_pos * 10u) + (PLAYER_SAMPLE_RATE / 2u)) /
        PLAYER_SAMPLE_RATE;
    if (state->time_label_cache_valid &&
        state->time_label_tenths == displayed_tenths &&
        state->time_label_total_samples == total_samples) {
        unlock_state(state);
        return;
    }
    state->time_label_tenths = displayed_tenths;
    state->time_label_total_samples = total_samples;
    state->time_label_cache_valid = 1;
    unlock_state(state);

    format_elapsed_time(elapsed, sizeof(elapsed), sample_pos, PLAYER_SAMPLE_RATE);
    if (total_samples > 0) {
        format_elapsed_time(total, sizeof(total), total_samples, PLAYER_SAMPLE_RATE);
        snprintf(label, sizeof(label), "%s / %s", elapsed, total);
    } else {
        snprintf(label, sizeof(label), "%s / --:--.-", elapsed);
    }
    if (strcmp(state->time_label_text, label) != 0) {
        HDC hdc;
        HFONT label_font;
        HGDIOBJ previous_font = NULL;
        SIZE text_size;
        int measured_width = TIME_LABEL_MIN_WIDTH;
        int relayout_needed = 0;

        snprintf(state->time_label_text, sizeof(state->time_label_text), "%s", label);
        hdc = GetDC(state->time_label);
        if (hdc != NULL) {
            label_font = (HFONT)SendMessageA(
                state->time_label, WM_GETFONT, 0, 0);
            if (label_font != NULL) {
                previous_font = SelectObject(hdc, label_font);
            }
            if (GetTextExtentPoint32A(hdc, label, (int)strlen(label), &text_size)) {
                PlayerScaleLayout layout;

                get_player_chrome_scale_layout(
                    state->hwnd, state, &layout);
                measured_width = layout.scale_milli > 0 ?
                    ((text_size.cx + 2) * 1000 + layout.scale_milli / 2) /
                        layout.scale_milli :
                    text_size.cx + 2;
                if (measured_width < 1) {
                    measured_width = 1;
                }
                if (measured_width > state->time_label_base_width) {
                    state->time_label_base_width = measured_width;
                    relayout_needed = state->ui_font_size > 12;
                }
                if (!relayout_needed) {
                    position_scaled_control(state->time_label, &layout,
                        state->time_label_base_x > 0 ?
                            state->time_label_base_x :
                            694 + (IsZoomed(state->hwnd) ? 18 : 0),
                        state->time_label_base_height > 0 ?
                            state->time_label_base_y : 6,
                        state->time_label_base_width,
                        state->time_label_base_height > 0 ?
                            state->time_label_base_height : 16);
                }
            }
            if (previous_font != NULL && previous_font != HGDI_ERROR) {
                SelectObject(hdc, previous_font);
            }
            ReleaseDC(state->time_label, hdc);
        }
        SetWindowTextA(state->time_label, label);
        if (relayout_needed) {
            layout_player_controls(state->hwnd, state);
        }
        RedrawWindow(state->time_label, NULL, NULL,
            RDW_INVALIDATE | RDW_ERASE);
    }
}

static void draw_bar(
    HDC hdc,
    int x,
    int y,
    int width,
    int height,
    uint32_t value,
    uint32_t max_value,
    COLORREF fill_color,
    COLORREF background_color,
    int force_black_border)
{
    RECT border;
    RECT fill;
    HBRUSH brush = (HBRUSH)GetStockObject(DC_BRUSH);
    COLORREF previous_pen_color;
    HGDIOBJ previous_brush;
    int fill_width;

    value = clamp_u32(value, max_value);
    fill_width = (int)((value * (uint32_t)width) / max_value);

    border.left = x;
    border.top = y;
    border.right = x + width;
    border.bottom = y + height;

    if (force_black_border) {
        SetDCBrushColor(hdc, background_color);
        FillRect(hdc, &border, brush);
        if (fill_width > 0) {
            fill.left = x;
            fill.top = y;
            fill.right = x + fill_width;
            fill.bottom = y + height;
            SetDCBrushColor(hdc, fill_color);
            FillRect(hdc, &fill, brush);
        }
        previous_pen_color = GetDCPenColor(hdc);
        previous_brush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        SetDCPenColor(hdc, RGB(0, 0, 0));
        Rectangle(hdc, border.left, border.top, border.right, border.bottom);
        SetDCPenColor(hdc, previous_pen_color);
        SelectObject(hdc, previous_brush);
        return;
    }

    SetDCBrushColor(hdc, background_color);
    Rectangle(hdc, border.left, border.top, border.right, border.bottom);

    if (fill_width <= 1) {
        return;
    }

    fill.left = x + 1;
    fill.top = y + 1;
    fill.right = x + fill_width;
    fill.bottom = y + height - 1;

    SetDCBrushColor(hdc, fill_color);
    FillRect(hdc, &fill, brush);
}

static void draw_stereo_bar(
    HDC hdc,
    int x,
    int y,
    int width,
    int height,
    uint32_t left_value,
    uint32_t right_value,
    uint32_t max_value,
    COLORREF left_color,
    COLORREF right_color,
    COLORREF background_color,
    int force_black_border)
{
    RECT border;
    RECT left_fill;
    RECT right_fill;
    HBRUSH brush = (HBRUSH)GetStockObject(DC_BRUSH);
    COLORREF previous_pen_color;
    HGDIOBJ previous_brush;
    int left_width;
    int right_width;
    int mid;

    left_value = clamp_u32(left_value, max_value);
    right_value = clamp_u32(right_value, max_value);
    if (max_value != 0) {
        if (left_value != 0) {
            left_value = clamp_u32(left_value + ((max_value - left_value) / 32u), max_value);
        }
        if (right_value != 0) {
            right_value = clamp_u32(right_value + ((max_value - right_value) / 32u), max_value);
        }
    }
    left_width = (int)((left_value * (uint32_t)width) / max_value);
    right_width = (int)((right_value * (uint32_t)width) / max_value);
    mid = y + (height / 2);

    border.left = x;
    border.top = y;
    border.right = x + width;
    border.bottom = y + height;
    if (force_black_border) {
        SetDCBrushColor(hdc, background_color);
        FillRect(hdc, &border, brush);
        if (left_width > 0) {
            left_fill.left = x;
            left_fill.top = y;
            left_fill.right = x + left_width;
            left_fill.bottom = mid;
            SetDCBrushColor(hdc, left_color);
            FillRect(hdc, &left_fill, brush);
        }
        if (right_width > 0) {
            right_fill.left = x;
            right_fill.top = mid;
            right_fill.right = x + right_width;
            right_fill.bottom = y + height;
            SetDCBrushColor(hdc, right_color);
            FillRect(hdc, &right_fill, brush);
        }
        previous_pen_color = GetDCPenColor(hdc);
        previous_brush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        SetDCPenColor(hdc, RGB(0, 0, 0));
        Rectangle(hdc, border.left, border.top, border.right, border.bottom);
        SetDCPenColor(hdc, previous_pen_color);
        SelectObject(hdc, previous_brush);
        return;
    }

    SetDCBrushColor(hdc, background_color);
    Rectangle(hdc, border.left, border.top, border.right, border.bottom);

    if (left_width > 1) {
        left_fill.left = x + 1;
        left_fill.top = y + 1;
        left_fill.right = x + left_width;
        left_fill.bottom = mid;
        SetDCBrushColor(hdc, left_color);
        FillRect(hdc, &left_fill, brush);
    }

    if (right_width > 1) {
        right_fill.left = x + 1;
        right_fill.top = mid;
        right_fill.right = x + right_width;
        right_fill.bottom = y + height - 1;
        SetDCBrushColor(hdc, right_color);
        FillRect(hdc, &right_fill, brush);
    }
}

static int is_voice_active(const Spu2LogVoiceSnapshot *voice)
{
    return voice->active || voice->flags != 0 || voice->envx != 0;
}

static int should_hold_voice_for_display(const Spu2LogVoiceSnapshot *voice)
{
    if (voice == NULL) {
        return 0;
    }

    return voice->active ||
        voice->envx != 0 ||
        (voice->flags & SPU2LOG_VOICE_KEY_ON) != 0;
}

static int ps1_voice_has_display_state(const Spu2LogVoiceSnapshot *voice)
{
    if (voice == NULL) {
        return 0;
    }

    return voice->active ||
        voice->adsr_phase != SPU2LOG_ADSR_OFF ||
        voice->flags != 0 ||
        voice->vol_l != 0 ||
        voice->vol_r != 0 ||
        voice->pitch != 0 ||
        voice->adsr1 != 0 ||
        voice->adsr2 != 0 ||
        voice->envx != 0 ||
        voice->ssa != 0 ||
        voice->lsa != 0 ||
        voice->nax != 0;
}

static int is_voice_really_sounding(const Spu2LogVoiceSnapshot *voice)
{
    if (voice == NULL) {
        return 0;
    }

    return voice->envx != 0;
}

static int visible_voice_count(const Spu2LogLiveState *state, unsigned core, int hide_inactive)
{
    unsigned voice;
    int count = 0;

    if (!hide_inactive) {
        return 24;
    }

    for (voice = 0; voice < 24; ++voice) {
        if (is_voice_active(&state->voices[core][voice])) {
            ++count;
        }
    }

    return count;
}

static void voice_status_counts(
    const Spu2LogLiveState *state,
    unsigned core,
    int *active,
    int *attack,
    int *decay,
    int *sustain,
    int *release)
{
    unsigned voice;

    *active = 0;
    *attack = 0;
    *decay = 0;
    *sustain = 0;
    *release = 0;

    for (voice = 0; voice < 24; ++voice) {
        const Spu2LogVoiceSnapshot *snapshot = &state->voices[core][voice];

        if (snapshot->envx == 0) {
            continue;
        }
        *active += 1;

        switch (snapshot->adsr_phase) {
        case SPU2LOG_ADSR_ATTACK:
            *attack += 1;
            break;
        case SPU2LOG_ADSR_DECAY:
            *decay += 1;
            break;
        case SPU2LOG_ADSR_SUSTAIN:
            *sustain += 1;
            break;
        case SPU2LOG_ADSR_RELEASE:
            *release += 1;
            break;
        default:
            break;
        }
    }
}

static int active_voice_count(const Spu2LogLiveState *state, unsigned core)
{
    unsigned voice;
    int count = 0;

    for (voice = 0; voice < 24; ++voice) {
        if (state->voices[core][voice].active) {
            ++count;
        }
    }
    return count;
}

static void adsr_phase_counts(
    const Spu2LogLiveState *state,
    unsigned core,
    int *attack,
    int *decay,
    int *sustain,
    int *release)
{
    int active;

    voice_status_counts(state, core, &active, attack, decay, sustain, release);
    (void)active;
}

static int calculate_content_height(HDC hdc, const PlayerState *state)
{
    TEXTMETRICA tm;
    int line_height;
    int core0_count;
    int core1_count;

    GetTextMetricsA(hdc, &tm);
    line_height = tm.tmHeight + 4;
    core0_count = visible_voice_count(&state->live, 0, state->hide_inactive);
    if (state->psf_version == 0x01u) {
        int values_height = psf1_values_visible_for_state(
            state, state->psf_version) ? line_height : 0;
        return CONTROLS_HEIGHT + 6 +
            (line_height * (values_height > 0 ? 2 : 1)) +
            (values_height > 0 ? 0 : psf1_keyboard_gap_for_state(
                state, state->psf_version)) +
            ((values_height + psf1_keyboard_height_for_state(
                state, state->psf_version) + psf1_keyboard_gap_for_state(
                state, state->psf_version)) * core0_count);
    }
    core1_count = visible_voice_count(&state->live, 1, state->hide_inactive);

    return CONTROLS_HEIGHT + 6 + (line_height * 2) +
        (line_height * (core0_count > core1_count ? core0_count : core1_count));
}

static int player_voice_scroll_enabled(HWND hwnd, const PlayerState *state)
{
    return hwnd != NULL && state != NULL && IsZoomed(hwnd) &&
        state->psf_version == 0x01u &&
        normalize_psf1_display_mode(state->psf1_display_mode) ==
            PSF1_DISPLAY_VALUES_KEYBOARD;
}

static int player_voice_scroll_metrics(
    HWND hwnd,
    const PlayerState *state,
    int *total_height,
    int *page_height,
    int *device_top)
{
    PlayerScaleLayout layout;
    RECT client_rect;
    int line_height;
    int active_header_height;
    int rows_top;
    int row_step;
    int rows_bottom;
    int visible_rows;

    if (!player_voice_scroll_enabled(hwnd, state) ||
        !GetClientRect(hwnd, &client_rect)) {
        return 0;
    }
    get_player_content_scale_layout(hwnd, state, &layout);
    if (!layout.scaled || layout.dest_height <= 0 ||
        layout.base_height <= 0) {
        return 0;
    }
    line_height = state->meter_line_height > 0 ?
        state->meter_line_height : get_player_display_line_height(hwnd);
    active_header_height = maximized_active_header_line_height(
        hwnd, state, line_height);
    row_step = line_height + PSF1_TRACK_KEYBOARD_HEIGHT;
    visible_rows = 16;
    rows_top = 2 + CONTROLS_HEIGHT + active_header_height + line_height;
    rows_bottom = rows_top + 24 * row_step;
    if (total_height != NULL) {
        *total_height = rows_bottom - rows_top;
    }
    if (page_height != NULL) {
        *page_height = visible_rows * row_step;
    }
    if (device_top != NULL) {
        *device_top = layout.dest_y + (int)(((int64_t)rows_top *
            layout.dest_height) / layout.base_height);
    }
    return (24 - visible_rows) * row_step;
}

static int player_voice_scroll_max(HWND hwnd, const PlayerState *state)
{
    return player_voice_scroll_metrics(hwnd, state, NULL, NULL, NULL);
}

static void set_player_voice_scroll(
    HWND hwnd,
    PlayerState *state,
    int scroll_y)
{
    int maximum;

    if (state == NULL) {
        return;
    }
    maximum = player_voice_scroll_max(hwnd, state);
    if (scroll_y < 0) {
        scroll_y = 0;
    } else if (scroll_y > maximum) {
        scroll_y = maximum;
    }
    if (state->scroll_y == scroll_y) {
        return;
    }
    state->scroll_y = scroll_y;
    if (state->voice_scrollbar != NULL) {
        SetScrollPos(state->voice_scrollbar, SB_CTL, scroll_y, TRUE);
    }
    InvalidateRect(hwnd, NULL, FALSE);
}

static void adjust_player_voice_scroll(
    HWND hwnd,
    PlayerState *state,
    int delta)
{
    if (state == NULL || !player_voice_scroll_enabled(hwnd, state)) {
        return;
    }
    set_player_voice_scroll(hwnd, state, state->scroll_y + delta);
}

static void update_scrollbar(HWND hwnd, PlayerState *state)
{
    RECT client_rect;

    if (state == NULL) {
        return;
    }
    if (player_voice_scroll_enabled(hwnd, state)) {
        SCROLLINFO info;
        int total_height = 1;
        int page_height = 1;
        int device_top = 0;
        int maximum = player_voice_scroll_metrics(
            hwnd, state, &total_height, &page_height, &device_top);
        int scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);

        set_player_voice_scroll(hwnd, state, state->scroll_y);
        ZeroMemory(&info, sizeof(info));
        info.cbSize = sizeof(info);
        info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        info.nMin = 0;
        info.nMax = total_height > 0 ? total_height - 1 : 0;
        info.nPage = (UINT)(page_height > 0 ? page_height : 1);
        info.nPos = state->scroll_y > maximum ? maximum : state->scroll_y;
        if (state->voice_scrollbar != NULL) {
            SetScrollInfo(state->voice_scrollbar, SB_CTL, &info, TRUE);
            if (GetClientRect(hwnd, &client_rect)) {
                SetWindowPos(state->voice_scrollbar, HWND_TOP,
                    client_rect.right - scrollbar_width,
                    device_top,
                    scrollbar_width,
                    client_rect.bottom - device_top - 4,
                    SWP_NOACTIVATE | SWP_SHOWWINDOW);
            }
        }
    } else {
        state->scroll_y = 0;
        if (state->voice_scrollbar != NULL) {
            ShowWindow(state->voice_scrollbar, SW_HIDE);
        }
    }
}

static Spu2LogResult player_spu2_write16(
    void *user,
    uint64_t sample_pos,
    uint32_t address,
    uint16_t value)
{
    PlayerState *state = (PlayerState *)user;
    uint8_t core;
    uint32_t normalized_address;
    uint32_t core_offset;
    uint32_t mask;
    uint8_t voice;
    int key_on_write = 0;
    int key_off_write = 0;
    int ps1_register_space = 0;

    if (state == NULL) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    normalized_address = address & 0x1fffffffu;
    if (normalized_address >= PS1_SPU_BASE && normalized_address <= PS1_SPU_END) {
        ps1_register_space = 1;
        core = 0;
        core_offset = normalized_address - PS1_SPU_BASE;
    } else {
        core = (uint8_t)((normalized_address / SPU2_CORE_STRIDE) & 1u);
        core_offset = normalized_address % SPU2_CORE_STRIDE;
    }

    if (ps1_register_space) {
        if (core_offset == PS1_REG_KON) {
            mask = (uint32_t)value;
            key_on_write = 1;
        } else if (core_offset == PS1_REG_KON_HI) {
            mask = ((uint32_t)value) << 16;
            key_on_write = 1;
        } else if (core_offset == PS1_REG_KOFF) {
            mask = (uint32_t)value;
            key_off_write = 1;
        } else if (core_offset == PS1_REG_KOFF_HI) {
            mask = ((uint32_t)value) << 16;
            key_off_write = 1;
        } else {
            return SPU2LOG_OK;
        }
    } else {
        if (core_offset == SPU2_REG_KON) {
            mask = (uint32_t)value;
            key_on_write = 1;
        } else if (core_offset == SPU2_REG_KON_HI) {
            mask = ((uint32_t)value) << 16;
            key_on_write = 1;
        } else if (core_offset == SPU2_REG_KOFF) {
            mask = (uint32_t)value;
            key_off_write = 1;
        } else if (core_offset == SPU2_REG_KOFF_HI) {
            mask = ((uint32_t)value) << 16;
            key_off_write = 1;
        } else {
            return SPU2LOG_OK;
        }
    }

    mask &= 0x00ffffffu;
    if (mask == 0) {
        return SPU2LOG_OK;
    }

    lock_state(state);
    if (state->startup_silence_trim) {
        unlock_state(state);
        return SPU2LOG_OK;
    }
    if (state->seek_discarding || state->timbre_prescanning) {
        state->live.last_sample_pos = sample_pos;
        unlock_state(state);
        return SPU2LOG_OK;
    }
    if (key_on_write &&
        (((core_offset == SPU2_REG_KON || core_offset == PS1_REG_KON) && value == 0xffffu) ||
            ((core_offset == SPU2_REG_KON_HI || core_offset == PS1_REG_KON_HI) &&
                (value & 0x00ffu) == 0x00ffu)) &&
        active_voice_count(&state->live, core) == 0) {
        state->live.last_sample_pos = sample_pos;
        unlock_state(state);
        return SPU2LOG_OK;
    }
    for (voice = 0; voice < 24; ++voice) {
        if (key_on_write && (mask & (1u << voice)) != 0) {
            state->key_on_event_buffers[core][voice] = VOICE_EVENT_DISPLAY_BUFFERS;
            state->release_event_latched[core][voice] = 0;
        }
        if (key_off_write &&
            (mask & (1u << voice)) != 0 &&
            state->live.voices[core][voice].active &&
            !state->release_event_latched[core][voice]) {
            state->release_event_buffers[core][voice] = VOICE_EVENT_DISPLAY_BUFFERS;
            state->release_event_latched[core][voice] = 1;
        }
    }
    state->live.last_sample_pos = sample_pos;
    unlock_state(state);
    return SPU2LOG_OK;
}

static Spu2LogResult player_voice_snapshot(
    void *user,
    uint64_t sample_pos,
    const Spu2LogVoiceSnapshot *snapshot)
{
    PlayerState *state = (PlayerState *)user;
    Spu2LogVoiceSnapshot merged;
    int update_mute_masks = 0;
    int update_timbre_list = 0;

    if (state == NULL || snapshot == NULL || snapshot->core >= 2 || snapshot->voice >= 24) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    merged = *snapshot;
    merged.flags = (merged.flags & ~0x3f00u) | ((((snapshot->flags & 0x3f00u) >> 8) & NOISE_CLOCK_MAX) << 8);
    lock_state(state);
    if (state->startup_silence_trim) {
        unlock_state(state);
        return SPU2LOG_OK;
    }
    if (state->timbre_prescanning) {
        if (state->psf_version == 0x02u && !state->ps2_startup_origin_valid &&
            merged.active && merged.ssa != 0 &&
            (merged.adsr1 != 0 || merged.adsr2 != 0)) {
            state->ps2_startup_origin_sample = sample_pos;
            state->ps2_startup_origin_valid = 1;
        }
        update_timbre_list = timbre_list_add_voice_locked(state, &merged);
        unlock_state(state);
        if (update_timbre_list && state->timbre_hwnd != NULL) {
            PostMessageA(state->hwnd, PLAYER_WM_WORKER_UPDATE, 1, 0);
        }
        return SPU2LOG_OK;
    }
    if (state->seek_discarding) {
        state->live.last_sample_pos = sample_pos;
        unlock_state(state);
        return SPU2LOG_OK;
    }

    if (state->psf_version == 0x01u &&
        !merged.active &&
        ps1_voice_has_display_state(&state->live.voices[snapshot->core][snapshot->voice])) {
        Spu2LogVoiceSnapshot previous = state->live.voices[snapshot->core][snapshot->voice];

        /* DVA reuses mutable instrument parameters after a voice goes idle. */
        merged.vol_l = previous.vol_l;
        merged.vol_r = previous.vol_r;
        merged.pitch = previous.pitch;
        merged.adsr1 = previous.adsr1;
        merged.adsr2 = previous.adsr2;
        merged.envx = snapshot->envx;
        merged.ssa = previous.ssa;
        merged.lsa = previous.lsa;
        merged.nax = previous.nax;
        merged.sample_end = previous.sample_end;
        merged.flags &= ~SPU2LOG_VOICE_KEY_ON;
        if (merged.adsr_phase == SPU2LOG_ADSR_OFF) {
            merged.adsr_phase = SPU2LOG_ADSR_RELEASE;
            merged.flags |= SPU2LOG_VOICE_RELEASE;
        }
    }
    if (state->psf_version != 0x01u && merged.envx == 0) {
        uint8_t core = merged.core;
        uint8_t voice = merged.voice;
        memset(&merged, 0, sizeof(merged));
        merged.core = core;
        merged.voice = voice;
        merged.adsr_phase = SPU2LOG_ADSR_OFF;
        memset(&state->voice_display_hold[core][voice], 0,
            sizeof(state->voice_display_hold[core][voice]));
        state->voice_display_hold[core][voice].core = core;
        state->voice_display_hold[core][voice].voice = voice;
        state->voice_display_hold_until[core][voice] = 0;
    }
    if ((state->voice_adsr_force_mask[merged.core] & (1u << merged.voice)) != 0) {
        merged.envx = 0x7fffu;
        merged.active = 1u;
        if (merged.adsr_phase == SPU2LOG_ADSR_OFF) {
            merged.adsr_phase = SPU2LOG_ADSR_SUSTAIN;
        }
    }
    if ((state->voice_pitch_lock_mask[merged.core] & (1u << merged.voice)) != 0) {
        merged.pitch = state->voice_pitch_lock_value[merged.core][merged.voice];
    }
    if ((state->voice_volume_lock_mask[0][merged.core] & (1u << merged.voice)) != 0) {
        merged.vol_l = state->voice_volume_lock_value[0][merged.core][merged.voice];
    }
    if ((state->voice_volume_lock_mask[1][merged.core] & (1u << merged.voice)) != 0) {
        merged.vol_r = state->voice_volume_lock_value[1][merged.core][merged.voice];
    }
    if (should_hold_voice_for_display(&merged)) {
        state->voice_display_hold[merged.core][merged.voice] = merged;
        state->voice_display_hold_until[merged.core][merged.voice] = sample_pos + VOICE_DISPLAY_HOLD_SAMPLES;
    }
    if (!state->default_voice_valid[merged.core][merged.voice] &&
        (merged.adsr1 != 0 || merged.adsr2 != 0 || merged.active || merged.envx != 0)) {
        state->default_adsr1[merged.core][merged.voice] = merged.adsr1;
        state->default_adsr2[merged.core][merged.voice] = merged.adsr2;
        state->default_pitch[merged.core][merged.voice] = merged.pitch;
        state->default_vol_l[merged.core][merged.voice] = merged.vol_l;
        state->default_vol_r[merged.core][merged.voice] = merged.vol_r;
        state->default_voice_valid[merged.core][merged.voice] = 1u;
    }
    psf1_queue_tuning_locked(state, &merged);
    state->live.voices[merged.core][merged.voice] = merged;
    if (is_voice_active(&merged)) {
        state->stopped_display = 0;
    }
    state->live.last_sample_pos = sample_pos;
    if (state->frame_advance && state->frame_live_valid &&
        state->frame_capture_until != 0 &&
        sample_pos <= state->frame_capture_until) {
        state->frame_live.voices[merged.core][merged.voice] = state->live.voices[merged.core][merged.voice];
        state->frame_live.last_sample_pos = sample_pos;
    }
    /* The prescan owns group membership. Learning by address proximity during
       playback can admit a neighbouring preset and also misses its attack. */
    if (state->timbre_scan_enabled && !state->timbre_list_locked) {
        update_timbre_list = timbre_list_add_voice_locked(state, &merged);
    }
    update_mute_masks = state->timbre_solo_enabled;
    state->sequence += 1;
    unlock_state(state);
    if (update_mute_masks) {
        apply_voice_mute_masks(state);
    }
    if (update_timbre_list && state->hwnd != NULL) {
        PostMessageA(state->hwnd, PLAYER_WM_WORKER_UPDATE, 1, 0);
    }
    return SPU2LOG_OK;
}

static Spu2LogResult player_core_snapshot(
    void *user,
    uint64_t sample_pos,
    const Spu2LogCoreSnapshot *snapshot)
{
    PlayerState *state = (PlayerState *)user;
    Spu2LogCoreSnapshot merged;

    if (state == NULL || snapshot == NULL || snapshot->core >= 2) {
        return SPU2LOG_ERROR_INVALID_ARGUMENT;
    }

    merged = *snapshot;
    lock_state(state);
    if (state->startup_silence_trim) {
        unlock_state(state);
        return SPU2LOG_OK;
    }
    if (state->seek_discarding || state->timbre_prescanning) {
        state->live.last_sample_pos = sample_pos;
        unlock_state(state);
        return SPU2LOG_OK;
    }
    if (!state->manual_reverb_value_valid[snapshot->core][0]) {
        state->default_reverb_l[snapshot->core] = snapshot->reverb_l;
    } else {
        merged.reverb_l = state->live.cores[snapshot->core].reverb_l;
    }
    if (!state->manual_reverb_value_valid[snapshot->core][1]) {
        state->default_reverb_r[snapshot->core] = snapshot->reverb_r;
    } else {
        merged.reverb_r = state->live.cores[snapshot->core].reverb_r;
    }
    if (!state->default_core_valid[snapshot->core]) {
        state->default_noise_clock[snapshot->core] = (uint8_t)(((snapshot->flags & 0x3f00u) >> 8) & NOISE_CLOCK_MAX);
        state->default_reverb_l[snapshot->core] = snapshot->reverb_l;
        state->default_reverb_r[snapshot->core] = snapshot->reverb_r;
        state->default_core_valid[snapshot->core] = 1u;
    }
    state->live.cores[snapshot->core] = merged;
    state->live.last_sample_pos = sample_pos;
    if (state->frame_advance && state->frame_live_valid &&
        state->frame_capture_until != 0 &&
        sample_pos <= state->frame_capture_until) {
        state->frame_live.cores[snapshot->core] = state->live.cores[snapshot->core];
        state->frame_live.last_sample_pos = sample_pos;
    }
    state->sequence += 1;
    unlock_state(state);
    return SPU2LOG_OK;
}

static int sync_audible_display_after_sequence(PlayerState *state, uint64_t completed_sequence);

static void CALLBACK player_waveout_callback(
    HWAVEOUT wave,
    UINT message,
    DWORD_PTR instance,
    DWORD_PTR parameter1,
    DWORD_PTR parameter2)
{
    PlayerState *state = (PlayerState *)instance;
    WAVEHDR *header = (WAVEHDR *)parameter1;

    (void)parameter2;
    if (message != WOM_DONE || state == NULL || header == NULL ||
        state->hwnd == NULL || state->wave != wave) {
        return;
    }
    PostMessageA(
        state->hwnd,
        PLAYER_WM_AUDIO_BUFFER_DONE,
        (WPARAM)header->dwUser,
        (LPARAM)(UINT_PTR)wave);
}

static int open_waveout(HWAVEOUT *out_wave, PlayerState *callback_state)
{
    WAVEFORMATEX format;

    ZeroMemory(&format, sizeof(format));
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = PLAYER_SAMPLE_RATE;
    format.wBitsPerSample = 16;
    format.nBlockAlign = (WORD)(format.nChannels * (format.wBitsPerSample / 8u));
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

    return waveOutOpen(
        out_wave,
        WAVE_MAPPER,
        &format,
        callback_state != NULL ? (DWORD_PTR)player_waveout_callback : 0,
        (DWORD_PTR)callback_state,
        callback_state != NULL ? CALLBACK_FUNCTION : CALLBACK_NULL) == MMSYSERR_NOERROR;
}

static void reset_audio_display_tracking(PlayerState *state)
{
    if (state == NULL) {
        return;
    }
    lock_state(state);
    ZeroMemory(state->audio_display_snapshots, sizeof(state->audio_display_snapshots));
    ZeroMemory(&state->audible_display_snapshot, sizeof(state->audible_display_snapshot));
    state->audible_display_valid = 0;
    unlock_state(state);
}

static void capture_audio_display_snapshot(PlayerState *state, unsigned index, uint64_t sequence)
{
    AudioDisplaySnapshot *snapshot;
    Psf1AkaoPlaybackState akao;
    unsigned queued_index;
    unsigned core;
    unsigned voice;

    if (state == NULL || index >= PLAYER_AUDIO_BUFFERS) {
        return;
    }
    ZeroMemory(&akao, sizeof(akao));
    if (state->core != NULL) {
        (void)psf2log_get_imported_ps1_akao_state(state->core, &akao);
    }
    snapshot = &state->audio_display_snapshots[index];
    lock_state(state);
    state->akao = akao;
    if (akao.detected) {
        for (queued_index = 0; queued_index < PLAYER_AUDIO_BUFFERS; ++queued_index) {
            if (state->audio_display_snapshots[queued_index].valid &&
                !state->audio_display_snapshots[queued_index].akao.detected) {
                state->audio_display_snapshots[queued_index].akao = akao;
            }
        }
    }
    snapshot->live = state->live;
    snapshot->akao = akao;
    memcpy(snapshot->key_on_events,
        state->key_on_event_buffers,
        sizeof(snapshot->key_on_events));
    memcpy(snapshot->release_events,
        state->release_event_buffers,
        sizeof(snapshot->release_events));
    for (core = 0; core < 2; ++core) {
        for (voice = 0; voice < 24; ++voice) {
            if (state->key_on_event_buffers[core][voice] > 0) {
                --state->key_on_event_buffers[core][voice];
            }
            if (state->release_event_buffers[core][voice] > 0) {
                --state->release_event_buffers[core][voice];
            }
        }
    }
    memcpy(snapshot->voice_display_hold_until,
        state->voice_display_hold_until,
        sizeof(snapshot->voice_display_hold_until));
    memcpy(snapshot->voice_display_hold,
        state->voice_display_hold,
        sizeof(snapshot->voice_display_hold));
    memcpy(snapshot->psf1_voice_timbre_key,
        state->psf1_voice_timbre_key,
        sizeof(snapshot->psf1_voice_timbre_key));
    snapshot->timeline_sample_pos = timeline_sample_at_locked(
        state, snapshot->live.last_sample_pos);
    snapshot->sequence = sequence;
    snapshot->stopped_display = state->stopped_display;
    snapshot->startup_track_dimmed = state->ps2_startup_track_dimmed;
    snapshot->valid = 1;
    unlock_state(state);
}

static void sync_audible_display_to_queue(PlayerState *state)
{
    unsigned i;
    unsigned oldest = PLAYER_AUDIO_BUFFERS;
    uint64_t oldest_sequence = UINT64_MAX;

    if (state == NULL) {
        return;
    }
    for (i = 0; i < PLAYER_AUDIO_BUFFERS; ++i) {
        if (state->audio_in_use[i] &&
            state->audio_display_snapshots[i].valid &&
            state->audio_display_snapshots[i].sequence < oldest_sequence) {
            oldest = i;
            oldest_sequence = state->audio_display_snapshots[i].sequence;
        }
    }
    if (oldest < PLAYER_AUDIO_BUFFERS) {
        lock_state(state);
        state->audible_display_snapshot = state->audio_display_snapshots[oldest];
        state->audible_display_valid = 1;
        unlock_state(state);
    }
}

static int sync_audible_display_after_sequence(PlayerState *state, uint64_t completed_sequence)
{
    unsigned i;
    unsigned next = PLAYER_AUDIO_BUFFERS;
    uint64_t next_sequence = UINT64_MAX;
    int akao_changed = 0;

    if (state == NULL) {
        return 0;
    }
    lock_state(state);
    for (i = 0; i < PLAYER_AUDIO_BUFFERS; ++i) {
        if (state->audio_in_use[i] &&
            (state->audio_headers[i].dwFlags & WHDR_DONE) == 0 &&
            state->audio_display_snapshots[i].valid &&
            state->audio_display_snapshots[i].sequence > completed_sequence &&
            state->audio_display_snapshots[i].sequence < next_sequence) {
            next = i;
            next_sequence = state->audio_display_snapshots[i].sequence;
        }
    }
    if (next < PLAYER_AUDIO_BUFFERS) {
        const Psf1AkaoPlaybackState *old_state =
            &state->audible_display_snapshot.akao;
        const Psf1AkaoPlaybackState *new_state =
            &state->audio_display_snapshots[next].akao;

        akao_changed = !state->audible_display_valid ||
            old_state->detected != new_state->detected ||
            old_state->early_format != new_state->early_format ||
            old_state->driver_type != new_state->driver_type ||
            old_state->tempo != new_state->tempo ||
            old_state->beats_per_measure != new_state->beats_per_measure ||
            old_state->ticks_per_beat != new_state->ticks_per_beat ||
            old_state->current_beat != new_state->current_beat ||
            old_state->measure != new_state->measure;
        state->audible_display_snapshot = state->audio_display_snapshots[next];
        state->audible_display_valid = 1;
    }
    unlock_state(state);
    return akao_changed;
}

static void cleanup_audio_queue(PlayerState *state)
{
    unsigned i;

    if (state == NULL) {
        return;
    }

    if (state->wave != NULL) {
        waveOutReset(state->wave);
    }

    for (i = 0; i < PLAYER_AUDIO_BUFFERS; ++i) {
        if (state->audio_in_use[i] && state->wave != NULL) {
            waveOutUnprepareHeader(state->wave, &state->audio_headers[i], sizeof(state->audio_headers[i]));
            state->audio_in_use[i] = 0;
        }

        if (state->audio_buffers[i] != NULL) {
            free(state->audio_buffers[i]);
            state->audio_buffers[i] = NULL;
        }

        ZeroMemory(&state->audio_headers[i], sizeof(state->audio_headers[i]));
    }
    state->audio_queued_buffers = 0;
    state->audio_queue_low_water = PLAYER_AUDIO_BUFFERS;
    state->audio_started = 0;
    state->audio_chunks_queued = 0;
    reset_audio_display_tracking(state);
}

static void copy_scaled_pcm_with_declick(PlayerState *state, int16_t *out_pcm, const int16_t *in_pcm, uint32_t frames);
static void reap_audio_queue(PlayerState *state);

static int init_audio_queue(PlayerState *state)
{
    unsigned i;

    if (state == NULL) {
        return 0;
    }

    for (i = 0; i < PLAYER_AUDIO_BUFFERS; ++i) {
        state->audio_buffers[i] =
            (int16_t *)calloc(PLAYER_MAX_OUTPUT_FRAMES * 2u, sizeof(*state->audio_buffers[i]));
        if (state->audio_buffers[i] == NULL) {
            cleanup_audio_queue(state);
            return 0;
        }
    }

    state->audio_queued_buffers = 0;
    state->audio_queue_low_water = PLAYER_AUDIO_BUFFERS;
    state->audio_started = 0;
    state->audio_chunks_queued = 0;
    state->audio_underruns = 0;
    reset_audio_display_tracking(state);
    if (waveOutPause(state->wave) != MMSYSERR_NOERROR) {
        cleanup_audio_queue(state);
        return 0;
    }

    return 1;
}

static int restart_audio_prebuffer(PlayerState *state)
{
    if (state == NULL || state->wave == NULL) {
        return 0;
    }

    waveOutReset(state->wave);
    reap_audio_queue(state);
    state->audio_queued_buffers = 0;
    state->audio_queue_low_water = PLAYER_AUDIO_BUFFERS;
    state->audio_started = 0;
    state->audio_chunks_queued = 0;
    state->has_last_output = 0;
    reset_audio_display_tracking(state);
    return waveOutPause(state->wave) == MMSYSERR_NOERROR;
}

static void reap_audio_queue(PlayerState *state)
{
    unsigned i;
    int queue_changed = 0;

    if (state == NULL || state->wave == NULL) {
        return;
    }

    for (i = 0; i < PLAYER_AUDIO_BUFFERS; ++i) {
        if (state->audio_in_use[i] &&
            (state->audio_headers[i].dwFlags & WHDR_DONE) != 0) {
            waveOutUnprepareHeader(state->wave, &state->audio_headers[i], sizeof(state->audio_headers[i]));
            state->audio_in_use[i] = 0;
            if (state->audio_queued_buffers > 0) {
                state->audio_queued_buffers--;
            }
            state->audio_display_snapshots[i].valid = 0;
            queue_changed = 1;
            ZeroMemory(&state->audio_headers[i], sizeof(state->audio_headers[i]));
        }
    }
    if (queue_changed && state->audio_started) {
        sync_audible_display_to_queue(state);
    }
}

static int queue_waveout_chunk(PlayerState *state, const int16_t *pcm, uint32_t frames)
{
    MMRESULT result;
    unsigned i;

    if (state == NULL || state->wave == NULL || pcm == NULL || frames == 0) {
        return 1;
    }
    if (frames > PLAYER_MAX_OUTPUT_FRAMES) {
        frames = PLAYER_MAX_OUTPUT_FRAMES;
    }

    while (WaitForSingleObject(state->stop_event, 0) == WAIT_TIMEOUT) {
        reap_audio_queue(state);

        for (i = 0; i < PLAYER_AUDIO_BUFFERS; ++i) {
            if (!state->audio_in_use[i]) {
                if (state->audio_started) {
                    if (state->audio_queued_buffers < state->audio_queue_low_water) {
                        state->audio_queue_low_water = state->audio_queued_buffers;
                    }
                    if (state->audio_queued_buffers == 0) {
                        state->audio_underruns++;
                        player_log("audio queue underrun count=%llu sample=%llu",
                            (unsigned long long)state->audio_underruns,
                            (unsigned long long)spu2log_audacious_get_sample_pos());
                    }
                }
                copy_scaled_pcm_with_declick(state, state->audio_buffers[i], pcm, frames);
                capture_audio_display_snapshot(state, i, state->audio_chunks_queued + 1u);
                ZeroMemory(&state->audio_headers[i], sizeof(state->audio_headers[i]));
                state->audio_headers[i].lpData = (LPSTR)state->audio_buffers[i];
                state->audio_headers[i].dwBufferLength = frames * 2u * (uint32_t)sizeof(*pcm);
                state->audio_headers[i].dwUser =
                    (DWORD_PTR)(state->audio_chunks_queued + 1u);

                result = waveOutPrepareHeader(state->wave, &state->audio_headers[i], sizeof(state->audio_headers[i]));
                if (result != MMSYSERR_NOERROR) {
                    state->audio_display_snapshots[i].valid = 0;
                    return 0;
                }

                result = waveOutWrite(state->wave, &state->audio_headers[i], sizeof(state->audio_headers[i]));
                if (result != MMSYSERR_NOERROR) {
                    waveOutUnprepareHeader(state->wave, &state->audio_headers[i], sizeof(state->audio_headers[i]));
                    ZeroMemory(&state->audio_headers[i], sizeof(state->audio_headers[i]));
                    state->audio_display_snapshots[i].valid = 0;
                    return 0;
                }

                state->audio_in_use[i] = 1;
                state->audio_queued_buffers++;
                state->audio_chunks_queued++;
                if (state->audio_started && state->audio_queued_buffers == 1u) {
                    sync_audible_display_to_queue(state);
                }
                if (!state->audio_started &&
                    state->audio_queued_buffers >= PLAYER_AUDIO_PREBUFFER_BUFFERS) {
                    state->audio_started = 1;
                    sync_audible_display_to_queue(state);
                    result = waveOutRestart(state->wave);
                    if (result != MMSYSERR_NOERROR) {
                        state->audio_started = 0;
                        return 0;
                    }
                    if (state->hwnd != NULL) {
                        SendMessageA(
                            state->hwnd,
                            PLAYER_WM_AUDIO_STARTING,
                            0,
                            (LPARAM)(UINT_PTR)state->wave);
                    }
                    player_log("audio prebuffer ready buffers=%u frames=%u",
                        state->audio_queued_buffers,
                        state->audio_queued_buffers * PLAYER_RENDER_FRAMES);
                }
                if ((state->audio_chunks_queued % 600u) == 0u) {
                    player_log("audio queue chunks=%llu queued=%u low=%u underruns=%llu",
                        (unsigned long long)state->audio_chunks_queued,
                        state->audio_queued_buffers,
                        state->audio_queue_low_water,
                        (unsigned long long)state->audio_underruns);
                    state->audio_queue_low_water = state->audio_queued_buffers;
                }
                return 1;
            }
        }

        Sleep(1);
    }

    return 1;
}

static int16_t clamp_i32_to_i16(int value)
{
    if (value > 32767) {
        return 32767;
    }
    if (value < -32768) {
        return -32768;
    }
    return (int16_t)value;
}

static uint32_t first_nonzero_pcm_frame(const int16_t *pcm, uint32_t frames)
{
    uint32_t frame;

    if (pcm == NULL) {
        return frames;
    }
    for (frame = 0; frame < frames; ++frame) {
        if (pcm[frame * 2u] != 0 || pcm[frame * 2u + 1u] != 0) {
            return frame;
        }
    }
    return frames;
}

static int song_voice_activity_present(PlayerState *state)
{
    unsigned core;
    unsigned voice;
    unsigned core_count;
    int active = 0;

    if (state == NULL) {
        return 0;
    }

    lock_state(state);
    core_count = state->psf_version == 0x01u ? 1u : 2u;
    for (core = 0; core < core_count && !active; ++core) {
        for (voice = 0; voice < 24u; ++voice) {
            const Spu2LogVoiceSnapshot *snapshot =
                &state->live.voices[core][voice];

            if (snapshot->active || snapshot->envx != 0) {
                active = 1;
                break;
            }
        }
    }
    unlock_state(state);
    return active;
}

static void start_timeline_at_first_audible_frame(
    PlayerState *state,
    const int16_t *pcm,
    uint32_t frames)
{
    uint64_t block_end_sample;
    uint64_t block_start_sample;
    uint32_t first_audible;
    int waiting;

    if (state == NULL || pcm == NULL || frames == 0) {
        return;
    }

    lock_state(state);
    waiting = !state->timeline_valid;
    unlock_state(state);
    if (!waiting) {
        return;
    }

    first_audible = first_nonzero_pcm_frame(pcm, frames);
    /* Voice and timbre solo are applied in the mixer. The SPU/SPU2 voice
       state still describes the original song, so do not make elapsed time
       wait for the selected voice to produce non-zero output. */
    if (first_audible >= frames && song_voice_activity_present(state)) {
        first_audible = 0;
    }
    if (first_audible >= frames) {
        return;
    }

    block_end_sample = spu2log_audacious_get_sample_pos();
    block_start_sample = block_end_sample >= frames ?
        block_end_sample - frames : 0;

    lock_state(state);
    if (!state->timeline_valid) {
        state->timeline_source_anchor = block_start_sample + first_audible;
        state->timeline_scaled_anchor = 0;
        state->timeline_speed_percent = effective_speed_percent_locked(state);
        state->timeline_valid = 1;
        state->time_label_cache_valid = 0;
        if (state->psf_version == 0x02u) {
            state->ps2_startup_track_dimmed = 0;
        }
        player_log("elapsed timeline started sample=%llu frame=%lu",
            (unsigned long long)state->timeline_source_anchor,
            (unsigned long)first_audible);
    }
    unlock_state(state);
}

static uint32_t first_ps2_song_frame(
    PlayerState *state,
    const int16_t *pcm,
    uint32_t frames,
    uint64_t block_end_sample)
{
    uint64_t origin_sample = 0;
    uint64_t block_start_sample;
    int use_origin = 0;

    if (state != NULL) {
        lock_state(state);
        use_origin = state->ps2_startup_origin_valid;
        origin_sample = state->ps2_startup_origin_sample;
        unlock_state(state);
    }
    if (!use_origin || block_end_sample < frames) {
        return first_nonzero_pcm_frame(pcm, frames);
    }

    block_start_sample = block_end_sample - frames;
    if (origin_sample >= block_end_sample) {
        return frames;
    }
    if (origin_sample <= block_start_sample) {
        return 0;
    }
    return (uint32_t)(origin_sample - block_start_sample);
}

static void remember_ps2_startup_origin(
    PlayerState *state,
    uint64_t block_end_sample,
    uint32_t rendered,
    uint32_t first_song_frame)
{
    if (state == NULL || first_song_frame >= rendered || block_end_sample < rendered) {
        return;
    }
    lock_state(state);
    if (!state->ps2_startup_origin_valid) {
        state->ps2_startup_origin_sample =
            block_end_sample - rendered + first_song_frame;
        state->ps2_startup_origin_valid = 1;
    }
    unlock_state(state);
}

static int advance_ps2_core_to_audible_start(PlayerState *state, uint64_t *out_sample_pos)
{
    uint64_t trimmed_frames = 0;

    if (out_sample_pos != NULL) {
        *out_sample_pos = 0;
    }
    if (state == NULL || state->psf_version != 0x02 || state->provider == NULL ||
        state->core == NULL || state->pcm == NULL) {
        return 1;
    }

    lock_state(state);
    state->startup_silence_trim = 1;
    unlock_state(state);
    for (;;) {
        Psf2CoreBridgeResult result;
        uint32_t rendered = 0;
        uint32_t first_nonzero;
        uint64_t block_end_sample;

        if (state->stop_event != NULL &&
            WaitForSingleObject(state->stop_event, 0) != WAIT_TIMEOUT) {
            break;
        }
        result = state->provider->render(
            state->core,
            state->pcm,
            PLAYER_RENDER_FRAMES,
            &rendered);
        if (result != PSF2_CORE_BRIDGE_OK || rendered == 0) {
            break;
        }
        block_end_sample = spu2log_audacious_get_sample_pos();
        first_nonzero = first_ps2_song_frame(
            state,
            state->pcm,
            rendered,
            block_end_sample);
        if (first_nonzero == rendered) {
            trimmed_frames += rendered;
            continue;
        }

        trimmed_frames += first_nonzero;
        remember_ps2_startup_origin(
            state,
            block_end_sample,
            rendered,
            first_nonzero);
        lock_state(state);
        state->startup_silence_trim = 0;
        unlock_state(state);
        psf2log_rebase_imported_sample_position(state->core, rendered - first_nonzero);
        reset_live_display(state);
        psf2log_emit_imported_snapshot(state->core);
        if (out_sample_pos != NULL) {
            *out_sample_pos = rendered - first_nonzero;
        }
        player_log("PS2 seek origin trimmed frames=%llu seconds=%.3f",
            (unsigned long long)trimmed_frames,
            (double)trimmed_frames / (double)PLAYER_SAMPLE_RATE);
        return 1;
    }

    lock_state(state);
    state->startup_silence_trim = 0;
    unlock_state(state);
    return 0;
}

static uint32_t preview_adsr_rate(int index)
{
    static uint32_t table[160];
    static int initialized = 0;

    if (!initialized) {
        uint32_t rate = 3u;
        uint32_t step = 1u;
        unsigned repeat = 0;
        int i;

        ZeroMemory(table, sizeof(table));
        for (i = 32; i < 160; ++i) {
            if (rate < 0x3fffffffu) {
                rate += step;
                repeat++;
                if (repeat == 5u) {
                    repeat = 1u;
                    step *= 2u;
                }
            }
            if (rate > 0x3fffffffu) {
                rate = 0x3fffffffu;
            }
            table[i] = rate;
        }
        initialized = 1;
    }
    if (index < 0) {
        return 0;
    }
    if (index > 159) {
        index = 159;
    }
    return table[index];
}

static void preview_step_adsr_locked(PlayerState *state, PreviewVoice *voice)
{
    static const int envelope_curve[8] = {0, 4, 6, 8, 9, 10, 11, 12};
    int64_t envelope;
    unsigned attack_rate;
    unsigned decay_rate;
    unsigned sustain_level;
    unsigned sustain_rate;
    unsigned release_rate;
    uint32_t amount;

    if (state == NULL || voice == NULL || !voice->active) {
        return;
    }
    envelope = voice->envelope;
    attack_rate = (state->preview_adsr1 >> 8) & 0x7fu;
    decay_rate = (state->preview_adsr1 >> 4) & 0x0fu;
    sustain_level = state->preview_adsr1 & 0x0fu;
    sustain_rate = (state->preview_adsr2 >> 6) & 0x7fu;
    release_rate = state->preview_adsr2 & 0x1fu;

    if (voice->releasing) {
        if ((state->preview_adsr2 & 0x0020u) != 0) {
            amount = preview_adsr_rate(
                (4 * ((int)release_rate ^ 0x1f)) - 0x18 + 32 +
                envelope_curve[(unsigned)(envelope >> 28) & 7u]);
        } else {
            amount = preview_adsr_rate((4 * ((int)release_rate ^ 0x1f)) - 0x0c + 32);
        }
        envelope -= amount;
        if (envelope <= 0) {
            voice->envelope = 0;
            voice->active = 0;
            return;
        }
        voice->envelope = envelope;
        return;
    }

    if (voice->adsr_state == SPU2LOG_ADSR_ATTACK) {
        int index = ((int)attack_rate ^ 0x7f) - 0x10 + 32;
        if ((state->preview_adsr1 & 0x8000u) != 0 && envelope >= 0x60000000ll) {
            index -= 8;
        }
        envelope += preview_adsr_rate(index);
        if (envelope >= 0x7fffffffll) {
            envelope = 0x7fffffffll;
            voice->adsr_state = SPU2LOG_ADSR_DECAY;
        }
    } else if (voice->adsr_state == SPU2LOG_ADSR_DECAY) {
        amount = preview_adsr_rate(
            (4 * ((int)decay_rate ^ 0x1f)) - 0x18 + 32 +
            envelope_curve[(unsigned)(envelope >> 28) & 7u]);
        envelope -= amount;
        if (envelope < 0) {
            envelope = 0;
        }
        if (((unsigned)(envelope >> 27) & 0x0fu) <= sustain_level) {
            voice->adsr_state = SPU2LOG_ADSR_SUSTAIN;
        }
    } else {
        int sustain_decrease = (state->preview_adsr2 & 0x4000u) != 0;
        int sustain_exp = (state->preview_adsr2 & 0x8000u) != 0;

        voice->adsr_state = SPU2LOG_ADSR_SUSTAIN;
        if (!sustain_decrease) {
            int index = ((int)sustain_rate ^ 0x7f) - 0x10 + 32;
            if (sustain_exp && envelope >= 0x60000000ll) {
                index -= 8;
            }
            envelope += preview_adsr_rate(index);
            if (envelope > 0x7fffffffll) {
                envelope = 0x7fffffffll;
            }
        } else {
            int index;
            if (sustain_exp) {
                index = ((int)sustain_rate ^ 0x7f) - 0x1b + 32 +
                    envelope_curve[(unsigned)(envelope >> 28) & 7u];
            } else {
                index = ((int)sustain_rate ^ 0x7f) - 0x0f + 32;
            }
            envelope -= preview_adsr_rate(index);
            if (envelope < 0) {
                envelope = 0;
            }
        }
    }
    voice->envelope = envelope;
}

static uint16_t preview_pitch_for_note(uint16_t base_pitch, int midi_note)
{
    static const uint32_t semitone_q16[12] = {
        65536u, 69433u, 73561u, 77936u, 82570u, 87482u,
        92682u, 98193u, 104032u, 110218u, 116772u, 123715u
    };
    int semitones = midi_note - 60;
    int octave = 0;
    uint64_t pitch;

    if (base_pitch == 0) {
        base_pitch = 0x1000u;
    }
    while (semitones < 0) {
        semitones += 12;
        octave--;
    }
    while (semitones >= 12) {
        semitones -= 12;
        octave++;
    }
    pitch = ((uint64_t)base_pitch * semitone_q16[semitones]) >> 16;
    while (octave > 0 && pitch < 0x8000u) {
        pitch <<= 1;
        octave--;
    }
    while (octave < 0) {
        pitch >>= 1;
        octave++;
    }
    if (pitch == 0) {
        pitch = 1;
    }
    if (pitch > 0x3fffu) {
        pitch = 0x3fffu;
    }
    return (uint16_t)pitch;
}

static double preview_estimate_pitch_lag(
    const int16_t *pcm,
    uint32_t start_frame,
    uint32_t analysis_frames,
    uint32_t maximum_lag,
    double *out_score)
{
    double differences[PSF1_TUNING_MAX_LAG + 1u];
    double running = 0.0;
    double best_score = 1.0e30;
    uint32_t best_lag = 0;
    uint32_t lag;

    if (out_score != NULL) {
        *out_score = 1.0e30;
    }
    if (pcm == NULL || analysis_frames == 0 || maximum_lag < 24u ||
        maximum_lag > PSF1_TUNING_MAX_LAG) {
        return 0.0;
    }
    differences[0] = 1.0;
    for (lag = 1u; lag <= maximum_lag; ++lag) {
        double difference = 0.0;
        uint32_t frame;

        for (frame = 0; frame < analysis_frames; ++frame) {
            int delta = (int)pcm[start_frame + frame] -
                (int)pcm[start_frame + frame + lag];
            difference += (double)delta * (double)delta;
        }
        differences[lag] = difference;
    }
    for (lag = 1u; lag <= maximum_lag; ++lag) {
        double score;

        running += differences[lag];
        score = running > 0.0 ? (differences[lag] * (double)lag) / running : 1.0;
        differences[lag] = score;
    }
    for (lag = 22u; lag + 1u <= maximum_lag; ++lag) {
        double score = differences[lag];

        if (score < best_score) {
            best_score = score;
            best_lag = lag;
        }
    }
    if (best_lag != 0) {
        double fold_limit = best_score * 1.5 + 0.01;

        if (fold_limit > best_score + 0.04) {
            fold_limit = best_score + 0.04;
        }
        for (lag = 22u; lag < best_lag; ++lag) {
            if (differences[lag] <= fold_limit &&
                differences[lag] <= differences[lag - 1u] &&
                differences[lag] <= differences[lag + 1u]) {
                best_lag = lag;
                best_score = differences[lag];
                break;
            }
        }
    }
    if (best_lag != 0 && best_score < 0.72) {
        double refined_lag = (double)best_lag;

        if (best_lag > 1u && best_lag < maximum_lag) {
            double before = differences[best_lag - 1u];
            double center = differences[best_lag];
            double after = differences[best_lag + 1u];
            double denominator = before - (2.0 * center) + after;

            if (denominator > 0.000001 || denominator < -0.000001) {
                double offset = 0.5 * (before - after) / denominator;
                if (offset > 0.5) {
                    offset = 0.5;
                } else if (offset < -0.5) {
                    offset = -0.5;
                }
                refined_lag += offset;
            }
        }
        if (out_score != NULL) {
            *out_score = best_score;
        }
        return refined_lag;
    }
    return 0.0;
}

static void preview_prepare_sample_tuning_locked(PreviewSample *sample)
{
    double estimates[3];
    double scores[3];
    uint32_t maximum_lag;
    uint32_t analysis_frames;
    uint32_t segment_frames;
    uint32_t region_start;
    uint32_t region_frames;
    unsigned estimate_count = 0;
    unsigned i;

    if (sample == NULL || sample->tuning_attempted) {
        return;
    }
    sample->tuning_attempted = 1;
    if (sample->pcm == NULL || sample->pcm_frames < 384u) {
        return;
    }
    region_start = sample->loop_enabled ? sample->loop_frame : 0u;
    region_frames = sample->pcm_frames - region_start;
    if (region_frames < 384u) {
        region_start = 0;
        region_frames = sample->pcm_frames;
    }
    maximum_lag = region_frames / 3u;
    if (maximum_lag > 1024u) {
        maximum_lag = 1024u;
    }
    if (maximum_lag < 24u) {
        return;
    }
    analysis_frames = region_frames - maximum_lag;
    if (analysis_frames > 2048u) {
        analysis_frames = 2048u;
    }
    segment_frames = analysis_frames + maximum_lag;
    for (i = 0; i < 3u; ++i) {
        uint32_t spare = region_frames - segment_frames;
        uint32_t start_frame = region_start + (uint32_t)(((uint64_t)spare * (i + 1u)) / 4u);
        double lag = preview_estimate_pitch_lag(
            sample->pcm,
            start_frame,
            analysis_frames,
            maximum_lag,
            &scores[estimate_count]);

        if (lag > 0.0) {
            estimates[estimate_count++] = lag;
        }
    }
    if (estimate_count > 0) {
        double chosen_lag;
        uint32_t c4_pitch;

        if (estimate_count == 3u) {
            if (estimates[0] > estimates[1]) {
                double temporary = estimates[0]; estimates[0] = estimates[1]; estimates[1] = temporary;
            }
            if (estimates[1] > estimates[2]) {
                double temporary = estimates[1]; estimates[1] = estimates[2]; estimates[2] = temporary;
            }
            if (estimates[0] > estimates[1]) {
                double temporary = estimates[0]; estimates[0] = estimates[1]; estimates[1] = temporary;
            }
            chosen_lag = estimates[1];
        } else if (estimate_count == 2u) {
            double difference = estimates[0] > estimates[1]
                ? estimates[0] - estimates[1]
                : estimates[1] - estimates[0];
            chosen_lag = difference < (estimates[0] * 0.08)
                ? (estimates[0] + estimates[1]) * 0.5
                : (scores[0] <= scores[1] ? estimates[0] : estimates[1]);
        } else {
            chosen_lag = estimates[0];
        }
        c4_pitch = (uint32_t)(chosen_lag * 24.300 + 0.5);

        if (c4_pitch == 0) {
            c4_pitch = 1;
        }
        if (c4_pitch > 0xffffu) {
            c4_pitch = 0xffffu;
        }
        sample->c4_pitch = (uint16_t)c4_pitch;
    }
}

static double psf1_estimate_pitch_lag_consensus(
    const int16_t *pcm,
    uint32_t region_start,
    uint32_t region_frames,
    uint32_t maximum_lag,
    double *out_score)
{
    double estimates[5];
    double scores[5];
    uint32_t analysis_frames;
    uint32_t segment_frames;
    unsigned count = 0;
    unsigned i;

    if (out_score != NULL) {
        *out_score = 1.0e30;
    }
    if (pcm == NULL || maximum_lag < 24u || region_frames <= maximum_lag) {
        return 0.0;
    }
    analysis_frames = region_frames - maximum_lag;
    if (analysis_frames > 1536u) {
        analysis_frames = 1536u;
    }
    if (analysis_frames < 128u) {
        return 0.0;
    }
    segment_frames = analysis_frames + maximum_lag;
    for (i = 0; i < 5u; ++i) {
        uint32_t spare = region_frames > segment_frames ?
            region_frames - segment_frames : 0u;
        uint32_t start_frame = region_start +
            (uint32_t)(((uint64_t)spare * (i + 1u)) / 6u);
        double score = 0.0;
        double lag = preview_estimate_pitch_lag(
            pcm, start_frame, analysis_frames, maximum_lag, &score);

        if (lag > 0.0) {
            estimates[count] = lag;
            scores[count] = score;
            count++;
        }
    }
    if (count == 0u) {
        return 0.0;
    }
    if (count == 1u) {
        if (out_score != NULL) {
            *out_score = scores[0];
        }
        return estimates[0];
    }
    {
        unsigned best_count = 0u;
        double best_score_sum = 1.0e30;
        double best_total = 0.0;
        unsigned anchor;

        for (anchor = 0u; anchor < count; ++anchor) {
            unsigned cluster_count = 0u;
            double score_sum = 0.0;
            double total = 0.0;

            for (i = 0u; i < count; ++i) {
                double maximum = estimates[anchor] > estimates[i] ?
                    estimates[anchor] : estimates[i];
                double difference = estimates[anchor] > estimates[i] ?
                    estimates[anchor] - estimates[i] : estimates[i] - estimates[anchor];
                double relative = maximum > 0.0 ? difference / maximum : 1.0e30;

                if (relative <= 0.08) {
                    cluster_count++;
                    score_sum += scores[i];
                    total += estimates[i];
                }
            }
            if (cluster_count > best_count ||
                (cluster_count == best_count && score_sum < best_score_sum)) {
                best_count = cluster_count;
                best_score_sum = score_sum;
                best_total = total;
            }
        }
        if (best_count >= 2u) {
            if (out_score != NULL) {
                *out_score = best_score_sum / (double)best_count;
            }
            return best_total / (double)best_count;
        }
    }
    {
        unsigned best = 0u;

        for (i = 1u; i < count; ++i) {
            if (scores[i] < scores[best]) {
                best = i;
            }
        }
        if (out_score != NULL) {
            *out_score = scores[best];
        }
        return estimates[best];
    }
}

static double psf1_period_error(
    const int16_t *pcm,
    uint32_t region_start,
    uint32_t region_frames,
    uint32_t lag)
{
    uint64_t difference_sum = 0u;
    uint64_t energy_sum = 0u;
    uint32_t frames;
    uint32_t spare;
    uint32_t frame;
    unsigned segment_count;
    unsigned segment;

    if (pcm == NULL || lag < 22u || region_frames <= lag) {
        return 1.0e30;
    }
    frames = region_frames - lag;
    if (frames > 1024u) {
        frames = 1024u;
    }
    if (frames < 128u) {
        return 1.0e30;
    }
    spare = region_frames - (frames + lag);
    segment_count = spare >= 64u ? 3u : 1u;
    for (segment = 0u; segment < segment_count; ++segment) {
        uint32_t start = region_start +
            (uint32_t)(((uint64_t)spare * (segment + 1u)) /
                (segment_count + 1u));

        for (frame = 0u; frame < frames; ++frame) {
            int32_t first = pcm[start + frame];
            int32_t second = pcm[start + frame + lag];
            int64_t difference = (int64_t)first - (int64_t)second;

            difference_sum += (uint64_t)(difference * difference);
            energy_sum += (uint64_t)((int64_t)first * first) +
                (uint64_t)((int64_t)second * second);
        }
    }
    if (energy_sum == 0u) {
        return 1.0e30;
    }
    return (double)difference_sum / (double)energy_sum;
}

static double psf1_refine_fundamental_lag(
    const int16_t *pcm,
    uint32_t region_start,
    uint32_t region_frames,
    uint32_t maximum_lag,
    double estimated_lag)
{
    uint32_t base_lag;
    double base_error;
    uint32_t search_radius;
    uint32_t search_begin;
    uint32_t search_end;
    uint32_t candidate;
    unsigned multiple;

    if (estimated_lag <= 0.0) {
        return 0.0;
    }
    base_lag = (uint32_t)(estimated_lag + 0.5);
    search_radius = base_lag / 64u;
    if (search_radius < 2u) {
        search_radius = 2u;
    } else if (search_radius > 16u) {
        search_radius = 16u;
    }
    search_begin = base_lag > search_radius ? base_lag - search_radius : 22u;
    if (search_begin < 22u) {
        search_begin = 22u;
    }
    search_end = base_lag + search_radius;
    if (search_end > maximum_lag) {
        search_end = maximum_lag;
    }
    base_error = psf1_period_error(
        pcm, region_start, region_frames, base_lag);
    for (candidate = search_begin; candidate <= search_end; ++candidate) {
        double candidate_error = psf1_period_error(
            pcm, region_start, region_frames, candidate);

        if (candidate_error < base_error) {
            base_error = candidate_error;
            base_lag = candidate;
        }
    }
    if (base_error > 0.018 && base_error < 1.0e20) {
        uint32_t harmonic_base_lag = base_lag;
        double harmonic_base_error = base_error;

        for (multiple = 2u; multiple <= 4u; ++multiple) {
            uint32_t candidate_lag;
            uint32_t harmonic_radius;
            uint32_t harmonic_begin;
            uint32_t harmonic_end;
            double candidate_error = 1.0e30;
            double required_ratio = multiple == 2u ? 0.68 :
                (multiple == 3u ? 0.56 : 0.48);

            if (harmonic_base_lag > maximum_lag / multiple) {
                continue;
            }
            candidate_lag = harmonic_base_lag * multiple;
            harmonic_radius = candidate_lag / 128u;
            if (harmonic_radius < 2u) {
                harmonic_radius = 2u;
            } else if (harmonic_radius > 12u) {
                harmonic_radius = 12u;
            }
            harmonic_begin = candidate_lag - harmonic_radius;
            harmonic_end = candidate_lag + harmonic_radius;
            if (harmonic_end > maximum_lag) {
                harmonic_end = maximum_lag;
            }
            for (candidate = harmonic_begin; candidate <= harmonic_end; ++candidate) {
                double error = psf1_period_error(
                    pcm, region_start, region_frames, candidate);

                if (error < candidate_error) {
                    candidate_error = error;
                    candidate_lag = candidate;
                }
            }
            if (candidate_error + 0.006 < harmonic_base_error * required_ratio &&
                candidate_error < base_error) {
                base_lag = candidate_lag;
                base_error = candidate_error;
            }
        }
    }
    if (base_lag > 22u && base_lag < maximum_lag) {
        double before = psf1_period_error(
            pcm, region_start, region_frames, base_lag - 1u);
        double after = psf1_period_error(
            pcm, region_start, region_frames, base_lag + 1u);
        double denominator = before - (2.0 * base_error) + after;

        if (denominator > 0.0000001 || denominator < -0.0000001) {
            double offset = 0.5 * (before - after) / denominator;

            if (offset > 0.5) {
                offset = 0.5;
            } else if (offset < -0.5) {
                offset = -0.5;
            }
            return (double)base_lag + offset;
        }
    }
    return (double)base_lag;
}

static double psf1_analyze_tuning_region(
    const int16_t *pcm,
    uint32_t region_start,
    uint32_t region_frames,
    double *out_score)
{
    uint32_t maximum_lag;
    double lag;

    if (out_score != NULL) {
        *out_score = 1.0e30;
    }
    if (pcm == NULL || region_frames < 384u) {
        return 0.0;
    }
    maximum_lag = region_frames / 3u;
    if (maximum_lag > PSF1_TUNING_MAX_LAG) {
        maximum_lag = PSF1_TUNING_MAX_LAG;
    }
    if (maximum_lag < 24u) {
        return 0.0;
    }
    lag = psf1_estimate_pitch_lag_consensus(
        pcm,
        region_start,
        region_frames,
        maximum_lag,
        out_score);
    if (lag > 0.0) {
        lag = psf1_refine_fundamental_lag(
            pcm,
            region_start,
            region_frames,
            maximum_lag,
            lag);
    }
    return lag;
}

static double psf1_analyze_short_loop(
    const int16_t *pcm,
    uint32_t loop_frame,
    uint32_t pcm_frames,
    double *out_score)
{
    int16_t *repeated = NULL;
    uint32_t loop_frames;
    uint32_t maximum_lag;
    uint32_t analysis_frames = 1536u;
    uint32_t repeated_frames;
    uint32_t frame;
    double lag = 0.0;

    if (out_score != NULL) {
        *out_score = 1.0e30;
    }
    if (pcm == NULL || loop_frame >= pcm_frames) {
        return 0.0;
    }
    loop_frames = pcm_frames - loop_frame;
    if (loop_frames < 24u || loop_frames >= 384u) {
        return 0.0;
    }
    maximum_lag = loop_frames;
    if (maximum_lag > PSF1_TUNING_MAX_LAG) {
        maximum_lag = PSF1_TUNING_MAX_LAG;
    }
    repeated_frames = analysis_frames + maximum_lag;
    repeated = (int16_t *)malloc((size_t)repeated_frames * sizeof(*repeated));
    if (repeated == NULL) {
        return 0.0;
    }
    for (frame = 0u; frame < repeated_frames; ++frame) {
        repeated[frame] = pcm[loop_frame + (frame % loop_frames)];
    }
    lag = preview_estimate_pitch_lag(
        repeated,
        0u,
        analysis_frames,
        maximum_lag,
        out_score);
    if (lag > 0.0) {
        lag = psf1_refine_fundamental_lag(
            repeated,
            0u,
            repeated_frames,
            maximum_lag,
            lag);
    }
    free(repeated);
    return lag;
}

static int psf1_is_silent_backward_redirect(
    Psf2CoreBridge *core,
    uint32_t ssa)
{
    uint8_t encoded[PSF1_BACKWARD_REDIRECT_MAX_ADPCM_BYTES];
    uint32_t loop_offset = 0;
    uint32_t end_flags = 0;
    uint32_t length;
    uint32_t copied;
    uint32_t block;

    if (core == NULL || ssa == 0u) {
        return 0;
    }
    length = psf2log_copy_imported_sample(
        core, ssa, ssa, NULL, 0, &loop_offset, &end_flags);
    if (length < 16u || length > sizeof(encoded) ||
        (end_flags & 3u) != 3u) {
        return 0;
    }
    copied = psf2log_copy_imported_sample(
        core, ssa, ssa, encoded, length, &loop_offset, &end_flags);
    if (copied < length) {
        return 0;
    }
    for (block = 0; block + 16u <= length; block += 16u) {
        uint32_t byte_index;

        for (byte_index = block + 2u; byte_index < block + 16u; ++byte_index) {
            if (encoded[byte_index] != 0u) {
                return 0;
            }
        }
    }
    return 1;
}

static void psf1_process_tuning_request(PlayerState *state)
{
    Psf2CoreBridge *core;
    uint8_t *encoded = NULL;
    int16_t *pcm = NULL;
    uint32_t ssa = 0;
    uint32_t lsa = 0;
    uint32_t analysis_ssa = 0;
    uint32_t analysis_lsa = 0;
    uint32_t length;
    uint32_t copy_length;
    uint32_t copied_length;
    uint32_t loop_offset = 0;
    uint32_t end_flags = 0;
    uint32_t pcm_frames = 0;
    uint32_t loop_frame = 0;
    uint16_t c4_pitch = 0;
    uint16_t previous_c4_pitch = 0;
    int refreshing = 0;
    unsigned index = PSF1_TUNING_CACHE_SIZE;
    unsigned i;

    if (state == NULL) {
        return;
    }
    lock_state(state);
    if (state->psf_version == 0x01u && state->core != NULL) {
        for (i = 0; i < state->psf1_tuning_cache_count; ++i) {
            if (state->psf1_tuning_cache[i].status == PSF1_TUNING_PENDING ||
                state->psf1_tuning_cache[i].status == PSF1_TUNING_REFRESH_PENDING) {
                index = i;
                refreshing = state->psf1_tuning_cache[i].status ==
                    PSF1_TUNING_REFRESH_PENDING;
                previous_c4_pitch = state->psf1_tuning_cache[i].c4_pitch;
                state->psf1_tuning_cache[i].status = refreshing ?
                    PSF1_TUNING_REFRESH_PROCESSING : PSF1_TUNING_PROCESSING;
                ssa = state->psf1_tuning_cache[i].ssa;
                lsa = state->psf1_tuning_cache[i].lsa;
                break;
            }
        }
    }
    core = state->core;
    unlock_state(state);
    if (index >= PSF1_TUNING_CACHE_SIZE || core == NULL) {
        return;
    }

    analysis_ssa = ssa;
    analysis_lsa = lsa;
    /* Some PS1 drivers suppress the attack with a tiny silent end/repeat
       block. Other drivers briefly expose a stale backward loop pointer at
       key-on, so only follow it after validating the SSA block itself. */
    if (lsa != 0u && lsa < ssa) {
        if (psf1_is_silent_backward_redirect(core, ssa)) {
            analysis_ssa = lsa;
            analysis_lsa = lsa;
        } else {
            analysis_lsa = ssa;
        }
    }

    length = psf2log_copy_imported_sample(
        core, analysis_ssa, analysis_lsa, NULL, 0, &loop_offset, &end_flags);
    copy_length = length < PSF1_TUNING_MAX_ADPCM_BYTES ?
        length : PSF1_TUNING_MAX_ADPCM_BYTES;
    copy_length &= ~15u;
    if (copy_length >= 16u) {
        encoded = (uint8_t *)malloc(copy_length);
    }
    if (encoded != NULL) {
        copied_length = psf2log_copy_imported_sample(
            core, analysis_ssa, analysis_lsa,
            encoded, copy_length, &loop_offset, &end_flags);
        if (copied_length >= copy_length) {
            pcm = preview_decode_adpcm(
                encoded,
                copy_length,
                loop_offset,
                &pcm_frames,
                &loop_frame);
        }
    }
    free(encoded);
    if (pcm != NULL && pcm_frames >= 24u) {
        double loop_score = 1.0e30;
        double full_score = 1.0e30;
        double lag = 0.0;
        int has_loop = (end_flags & 2u) != 0 && loop_frame < pcm_frames;

        if (has_loop) {
            uint32_t loop_frames = pcm_frames - loop_frame;

            if (loop_frames < 384u) {
                lag = psf1_analyze_short_loop(
                    pcm, loop_frame, pcm_frames, &loop_score);
            } else {
                lag = psf1_analyze_tuning_region(
                    pcm, loop_frame, loop_frames, &loop_score);
            }
        }
        /* Some loop bodies are too short or too irregular to expose their
           fundamental. Analyze the full timbre only when the cheaper loop
           result is absent or uncertain. Each result is cached per sample. */
        if ((!has_loop || lag <= 0.0 || loop_score > 0.24) &&
            pcm_frames >= 384u) {
            double full_lag = psf1_analyze_tuning_region(
                pcm, 0u, pcm_frames, &full_score);

            if (full_lag > 0.0 &&
                (lag <= 0.0 || full_score + 0.015 < loop_score)) {
                lag = full_lag;
            }
        }
        if (lag > 0.0) {
            uint32_t estimated = (uint32_t)(lag * 24.300 + 0.5);

            if (estimated == 0) {
                estimated = 1;
            }
            if (estimated > 0xffffu) {
                estimated = 0xffffu;
            }
            c4_pitch = (uint16_t)estimated;
        }
        player_log(
            "PS1 tuning ssa=%05lX source=%05lX frames=%lu loop_frame=%lu c4=%04X",
            (unsigned long)ssa,
            (unsigned long)analysis_ssa,
            (unsigned long)pcm_frames,
            (unsigned long)loop_frame,
            (unsigned)c4_pitch);
    }
    free(pcm);

    lock_state(state);
    if (index < state->psf1_tuning_cache_count &&
        state->psf1_tuning_cache[index].status ==
            (refreshing ? PSF1_TUNING_REFRESH_PROCESSING : PSF1_TUNING_PROCESSING)) {
        state->psf1_tuning_cache[index].c4_pitch =
            refreshing && c4_pitch == 0 ? previous_c4_pitch : c4_pitch;
        state->psf1_tuning_cache[index].status = PSF1_TUNING_READY;
        state->sequence += 1u;
    }
    unlock_state(state);
}

static uint32_t psf1_pitch_for_note_unclamped(uint16_t c4_pitch, int midi_note)
{
    static const uint32_t semitone_q16[12] = {
        65536u, 69433u, 73561u, 77936u, 82570u, 87482u,
        92682u, 98193u, 104032u, 110218u, 116772u, 123715u
    };
    int semitones = midi_note - 60;
    int octave = 0;
    uint64_t pitch;

    if (c4_pitch == 0) {
        c4_pitch = 0x1000u;
    }
    while (semitones < 0) {
        semitones += 12;
        octave--;
    }
    while (semitones >= 12) {
        semitones -= 12;
        octave++;
    }
    pitch = ((uint64_t)c4_pitch * semitone_q16[semitones]) >> 16;
    while (octave > 0) {
        pitch <<= 1;
        octave--;
    }
    while (octave < 0) {
        pitch >>= 1;
        octave++;
    }
    if (pitch == 0) {
        pitch = 1;
    }
    return pitch > 0xffffffffu ? 0xffffffffu : (uint32_t)pitch;
}

static int psf1_note_for_voice_locked(
    const PlayerState *state,
    uint32_t key,
    uint16_t pitch)
{
    uint16_t c4_pitch = 0;
    uint64_t best_score = UINT64_MAX;
    int best_note = -1;
    int index;
    int note;

    if (state == NULL || key == 0 || pitch == 0) {
        return -1;
    }
    index = psf1_find_tuning_locked(state, key);
    if (index < 0 ||
        (state->psf1_tuning_cache[index].status != PSF1_TUNING_READY &&
         state->psf1_tuning_cache[index].status != PSF1_TUNING_REFRESH_PENDING &&
         state->psf1_tuning_cache[index].status != PSF1_TUNING_REFRESH_PROCESSING)) {
        return -1;
    }
    c4_pitch = state->psf1_tuning_cache[index].c4_pitch;
    if (c4_pitch == 0) {
        c4_pitch = 0x1000u;
    }
    for (note = PSF1_TRACK_KEY_FIRST_NOTE;
         note <= PSF1_TRACK_KEY_LAST_NOTE;
         ++note) {
        uint32_t expected = psf1_pitch_for_note_unclamped(c4_pitch, note);
        uint32_t maximum = expected > pitch ? expected : pitch;
        uint32_t difference = expected > pitch ? expected - pitch : pitch - expected;
        uint64_t score = maximum != 0 ?
            ((uint64_t)difference << 24) / maximum : UINT64_MAX;

        if (score < best_score) {
            best_score = score;
            best_note = note;
        }
    }
    return best_note;
}

static unsigned preview_choose_sample_locked(
    const PlayerState *state,
    int midi_note,
    uint16_t target_pitch)
{
    uint32_t centers[TIMBRE_SOLO_MAX_KEYS];
    uint32_t minimum_center = 0xffffffffu;
    uint32_t maximum_center = 0;
    uint32_t zone_pitch;
    unsigned selected = PREVIEW_MAX_CACHED_SAMPLES;
    unsigned best_distance = 0xffffffffu;
    unsigned distinct_centers = 0;
    unsigned i;
    int mapping_note = midi_note;

    if (state == NULL || state->preview_selected_sample_count == 0) {
        return PREVIEW_MAX_CACHED_SAMPLES;
    }
    if (mapping_note < PREVIEW_FIRST_MIDI_NOTE) {
        mapping_note = PREVIEW_FIRST_MIDI_NOTE;
    } else if (mapping_note > PREVIEW_LAST_MIDI_NOTE) {
        mapping_note = PREVIEW_LAST_MIDI_NOTE;
    }
    if (state->preview_selected_sample_count == 1) {
        return state->preview_selected_samples[0];
    }
    {
        uint32_t desired_root = preview_pitch_for_note(0x1000u, midi_note);
        uint32_t previous_roots[TIMBRE_SOLO_MAX_KEYS];
        unsigned tuned_count = 0;
        unsigned distinct_roots = 0;

        ZeroMemory(previous_roots, sizeof(previous_roots));
        for (i = 0; i < state->preview_selected_sample_count; ++i) {
            unsigned cache_index = state->preview_selected_samples[i];
            const PreviewSample *sample;
            uint32_t sample_root;
            unsigned j;
            int duplicate = 0;

            if (cache_index >= state->preview_sample_count) {
                continue;
            }
            sample = &state->preview_samples[cache_index];
            if (sample->c4_pitch == 0) {
                continue;
            }
            sample_root = (0x1000u * 0x1000u) / sample->c4_pitch;
            previous_roots[i] = sample_root;
            tuned_count++;
            for (j = 0; j < i; ++j) {
                if (previous_roots[j] == sample_root) {
                    duplicate = 1;
                    break;
                }
            }
            if (!duplicate) {
                distinct_roots++;
            }
        }
        if (tuned_count >= 2u && distinct_roots >= 2u) {
            for (i = 0; i < state->preview_selected_sample_count; ++i) {
                unsigned cache_index = state->preview_selected_samples[i];
                unsigned distance;

                if (cache_index >= state->preview_sample_count || previous_roots[i] == 0) {
                    continue;
                }
                distance = previous_roots[i] > desired_root
                    ? previous_roots[i] - desired_root
                    : desired_root - previous_roots[i];
                if (distance < best_distance) {
                    best_distance = distance;
                    selected = cache_index;
                }
            }
            if (selected < state->preview_sample_count) {
                return selected;
            }
        }
    }
    ZeroMemory(centers, sizeof(centers));
    for (i = 0; i < state->preview_selected_sample_count; ++i) {
        unsigned cache_index = state->preview_selected_samples[i];
        const PreviewSample *sample;

        if (cache_index >= state->preview_sample_count) {
            continue;
        }
        sample = &state->preview_samples[cache_index];
        centers[i] = sample->pitch_min != 0 && sample->pitch_max >= sample->pitch_min
            ? ((uint32_t)sample->pitch_min + sample->pitch_max) / 2u
            : target_pitch;
        if (centers[i] < minimum_center) {
            minimum_center = centers[i];
        }
        if (centers[i] > maximum_center) {
            maximum_center = centers[i];
        }
        {
            unsigned j;
            int duplicate = 0;
            for (j = 0; j < i; ++j) {
                if (centers[j] == centers[i]) {
                    duplicate = 1;
                    break;
                }
            }
            if (!duplicate) {
                distinct_centers++;
            }
        }
    }
    /* The scan records the real pitch ranges used by each region. Map the
       full keyboard across those ranges, then select the nearest region.
       Banks whose regions use one identical tuning fall back to bank order. */
    if (maximum_center > minimum_center + 0x40u && distinct_centers >= 2u) {
        uint32_t note_position = (uint32_t)(mapping_note - PREVIEW_FIRST_MIDI_NOTE);
        uint32_t note_span = PREVIEW_LAST_MIDI_NOTE - PREVIEW_FIRST_MIDI_NOTE;
        zone_pitch = minimum_center +
            (uint32_t)(((uint64_t)(maximum_center - minimum_center) * note_position) / note_span);
        for (i = 0; i < state->preview_selected_sample_count; ++i) {
            unsigned cache_index = state->preview_selected_samples[i];
            unsigned distance;

            if (cache_index >= state->preview_sample_count) {
                continue;
            }
            distance = centers[i] > zone_pitch ? centers[i] - zone_pitch : zone_pitch - centers[i];
            if (distance < best_distance) {
                best_distance = distance;
                selected = cache_index;
            }
        }
    } else {
        unsigned zone = (unsigned)(((uint64_t)(mapping_note - PREVIEW_FIRST_MIDI_NOTE) *
            state->preview_selected_sample_count) /
            (PREVIEW_LAST_MIDI_NOTE - PREVIEW_FIRST_MIDI_NOTE + 1u));
        if (zone >= state->preview_selected_sample_count) {
            zone = state->preview_selected_sample_count - 1u;
        }
        selected = state->preview_selected_samples[zone];
    }
    return selected;
}

static uint16_t preview_sample_pitch_for_note_locked(
    const PlayerState *state,
    unsigned sample_index,
    int midi_note)
{
    uint32_t minimum_center = 0xffffffffu;
    uint32_t maximum_center = 0;
    uint32_t sample_center = 0x1000u;
    unsigned selected_position = 0;
    unsigned distinct_centers = 0;
    unsigned i;
    int zone_center_note;
    uint32_t pitch;

    if (state == NULL || sample_index >= state->preview_sample_count ||
        state->preview_selected_sample_count == 0) {
        return preview_pitch_for_note(state != NULL ? state->preview_pitch : 0x1000u, midi_note);
    }
    if (state->preview_samples[sample_index].c4_pitch != 0) {
        pitch = preview_pitch_for_note(state->preview_samples[sample_index].c4_pitch, midi_note);
        pitch = (pitch *
            (state->preview_pitch != 0 ? state->preview_pitch : 0x1000u)) /
            (state->preview_reference_pitch != 0 ? state->preview_reference_pitch : 0x1000u);
        if (pitch == 0) {
            pitch = 1;
        }
        if (pitch > 0x3fffu) {
            pitch = 0x3fffu;
        }
        return (uint16_t)pitch;
    }
    for (i = 0; i < state->preview_selected_sample_count; ++i) {
        unsigned index = state->preview_selected_samples[i];
        const PreviewSample *sample;
        uint32_t center;
        unsigned j;
        int duplicate = 0;

        if (index >= state->preview_sample_count) {
            continue;
        }
        sample = &state->preview_samples[index];
        center = 0x1000u;
        if (index == sample_index) {
            selected_position = i;
            sample_center = center;
        }
        if (center < minimum_center) {
            minimum_center = center;
        }
        if (center > maximum_center) {
            maximum_center = center;
        }
        for (j = 0; j < i; ++j) {
            unsigned previous_index = state->preview_selected_samples[j];
            const PreviewSample *previous;
            uint32_t previous_center;

            if (previous_index >= state->preview_sample_count) {
                continue;
            }
            previous = &state->preview_samples[previous_index];
            previous_center = 0x1000u;
            if (previous_center == center) {
                duplicate = 1;
                break;
            }
        }
        if (!duplicate) {
            distinct_centers++;
        }
    }
    if (state->preview_selected_sample_count == 1u) {
        zone_center_note = 60;
    } else if (maximum_center > minimum_center + 0x40u && distinct_centers >= 2u) {
        zone_center_note = PREVIEW_FIRST_MIDI_NOTE + (int)(
            ((uint64_t)(sample_center - minimum_center) *
                (PREVIEW_LAST_MIDI_NOTE - PREVIEW_FIRST_MIDI_NOTE)) /
            (maximum_center - minimum_center));
    } else {
        zone_center_note = PREVIEW_FIRST_MIDI_NOTE + (int)(
            ((uint64_t)(selected_position * 2u + 1u) *
                (PREVIEW_LAST_MIDI_NOTE - PREVIEW_FIRST_MIDI_NOTE + 1u)) /
            (state->preview_selected_sample_count * 2u));
    }
    pitch = preview_pitch_for_note(sample_center, 60 + midi_note - zone_center_note);
    pitch = (pitch *
        (state->preview_pitch != 0 ? state->preview_pitch : 0x1000u)) /
        (state->preview_reference_pitch != 0 ? state->preview_reference_pitch : 0x1000u);
    if (pitch == 0) {
        pitch = 1;
    }
    if (pitch > 0x3fffu) {
        pitch = 0x3fffu;
    }
    return (uint16_t)pitch;
}

static void preview_note_on_locked(PlayerState *state, int midi_note)
{
    PreviewVoice *voice = NULL;
    uint16_t target_pitch;
    unsigned sample_index;
    unsigned i;

    if (state == NULL || midi_note < 0 || midi_note >= 128) {
        return;
    }
    target_pitch = preview_pitch_for_note(state->preview_pitch, midi_note);
    sample_index = preview_choose_sample_locked(state, midi_note, target_pitch);
    if (sample_index >= state->preview_sample_count) {
        return;
    }
    preview_handoff_released_locked(state, midi_note);
    for (i = 0; i < PREVIEW_MAX_VOICES; ++i) {
        if (voice == NULL && !state->preview_voices[i].active) {
            voice = &state->preview_voices[i];
        }
    }
    if (voice == NULL) {
        voice = &state->preview_voices[0];
    }
    ZeroMemory(voice, sizeof(*voice));
    voice->active = 1;
    voice->midi_note = midi_note;
    voice->sample_index = sample_index;
    target_pitch = preview_sample_pitch_for_note_locked(state, sample_index, midi_note);
    voice->phase_step = ((uint64_t)target_pitch << 32) / 0x1000u;
    if (voice->phase_step == 0) {
        voice->phase_step = 1;
    }
    voice->envelope = 1;
    voice->adsr_state = SPU2LOG_ADSR_ATTACK;
    voice->noise_lfsr = 0x13579bdu ^ (uint32_t)midi_note;
    state->preview_key_down[midi_note] = 1u;
    state->preview_active_note = midi_note;
}

static void preview_note_off_locked(PlayerState *state, int midi_note)
{
    unsigned i;

    if (state == NULL) {
        return;
    }
    for (i = 0; i < PREVIEW_MAX_VOICES; ++i) {
        if (state->preview_voices[i].active && state->preview_voices[i].midi_note == midi_note) {
            state->preview_voices[i].releasing = 1;
            state->preview_voices[i].adsr_state = SPU2LOG_ADSR_RELEASE;
        }
    }
    if (midi_note >= 0 && midi_note < (int)sizeof(state->preview_key_down)) {
        state->preview_key_down[midi_note] = 0u;
    }
    if (state->preview_active_note == midi_note) {
        state->preview_active_note = -1;
    }
}

static int preview_pcm_at(const PreviewSample *sample, int64_t frame)
{
    if (sample == NULL || sample->pcm == NULL || sample->pcm_frames == 0) {
        return 0;
    }
    if (frame < 0) {
        frame = 0;
    }
    if ((uint64_t)frame >= sample->pcm_frames) {
        if (sample->loop_enabled && sample->loop_frame < sample->pcm_frames) {
            uint32_t loop_length = sample->pcm_frames - sample->loop_frame;
            frame = sample->loop_frame +
                (int64_t)(((uint64_t)frame - sample->loop_frame) % loop_length);
        } else {
            frame = sample->pcm_frames - 1u;
        }
    }
    return sample->pcm[(uint32_t)frame];
}

static int preview_cubic_sample(const PreviewSample *sample, uint32_t frame, uint32_t fraction)
{
    int64_t p0 = preview_pcm_at(sample, (int64_t)frame - 1);
    int64_t p1 = preview_pcm_at(sample, frame);
    int64_t p2 = preview_pcm_at(sample, (int64_t)frame + 1);
    int64_t p3 = preview_pcm_at(sample, (int64_t)frame + 2);
    int64_t t = fraction >> 16;
    int64_t t2 = (t * t) >> 16;
    int64_t t3 = (t2 * t) >> 16;
    int64_t a = -p0 + p2;
    int64_t b = (2 * p0) - (5 * p1) + (4 * p2) - p3;
    int64_t c = -p0 + (3 * p1) - (3 * p2) + p3;
    int64_t value = ((2 * p1 * 65536) + (a * t) + (b * t2) + (c * t3)) /
        (2 * 65536);

    if (value > 32767) {
        value = 32767;
    } else if (value < -32768) {
        value = -32768;
    }
    return (int)value;
}

static void preview_render_frame_locked(PlayerState *state, int *out_left, int *out_right)
{
    int64_t mix_l = 0;
    int64_t mix_r = 0;
    unsigned i;

    if (out_left != NULL) {
        *out_left = 0;
    }
    if (out_right != NULL) {
        *out_right = 0;
    }
    if (state == NULL || state->preview_selected_sample_count == 0) {
        return;
    }
    for (i = 0; i < PREVIEW_MAX_VOICES; ++i) {
        PreviewVoice *voice = &state->preview_voices[i];
        const PreviewSample *sample;
        int value;
        int wrapped = 0;
        uint64_t step;

        if (!voice->active || voice->sample_index >= state->preview_sample_count) {
            continue;
        }
        sample = &state->preview_samples[voice->sample_index];
        preview_step_adsr_locked(state, voice);
        if (!voice->active) {
            continue;
        }
        if ((state->preview_flags & SPU2LOG_VOICE_NOISE) != 0) {
            uint32_t feedback = ((voice->noise_lfsr >> 0) ^
                (voice->noise_lfsr >> 2) ^ (voice->noise_lfsr >> 3) ^
                (voice->noise_lfsr >> 5)) & 1u;
            voice->noise_lfsr = (voice->noise_lfsr >> 1) | (feedback << 30);
            value = (voice->noise_lfsr & 1u) ? 24576 : -24576;
        } else {
            uint32_t frame = (uint32_t)(voice->phase >> 32);
            uint32_t fraction = (uint32_t)voice->phase;

            if (frame >= sample->pcm_frames) {
                if (sample->loop_enabled && sample->loop_frame < sample->pcm_frames) {
                    uint32_t loop_length = sample->pcm_frames - sample->loop_frame;
                    frame = sample->loop_frame + ((frame - sample->loop_frame) % loop_length);
                    voice->phase = ((uint64_t)frame << 32) | fraction;
                    wrapped = 1;
                } else {
                    voice->active = 0;
                    continue;
                }
            }
            value = preview_cubic_sample(sample, frame, fraction);
            if (wrapped && voice->have_last_raw_sample) {
                uint32_t loop_length = sample->pcm_frames - sample->loop_frame;
                uint64_t output_frames = voice->phase_step != 0
                    ? ((uint64_t)loop_length << 32) / voice->phase_step
                    : 0;
                uint32_t smooth_frames = (uint32_t)(output_frames / 4u);

                if (smooth_frames > PREVIEW_LOOP_SMOOTH_FRAMES) {
                    smooth_frames = PREVIEW_LOOP_SMOOTH_FRAMES;
                }
                if (smooth_frames >= 2u) {
                    voice->loop_correction = voice->last_raw_sample - value;
                    voice->loop_smooth_frames = smooth_frames;
                    voice->loop_smooth_total = smooth_frames;
                } else {
                    voice->loop_smooth_frames = 0;
                    voice->loop_smooth_total = 0;
                }
            }
            if (voice->loop_smooth_frames > 0 && voice->loop_smooth_total > 0) {
                value += (int)(((int64_t)voice->loop_correction *
                    voice->loop_smooth_frames) / voice->loop_smooth_total);
                voice->loop_smooth_frames--;
            }
            if (!sample->loop_enabled && sample->pcm_frames - frame < PREVIEW_LOOP_SMOOTH_FRAMES) {
                value = (int)(((int64_t)value * (sample->pcm_frames - frame)) /
                    PREVIEW_LOOP_SMOOTH_FRAMES);
            }
            voice->last_raw_sample = value;
            voice->have_last_raw_sample = 1;
        }
        value = (int)(((int64_t)value * voice->envelope) >> 31);
        if (voice->handoff_frames > 0) {
            value = (int)(((int64_t)value * voice->handoff_frames) /
                PREVIEW_HANDOFF_FRAMES);
            voice->handoff_frames--;
            if (voice->handoff_frames == 0) {
                voice->active = 0;
            }
        }
        mix_l += ((int64_t)value * state->preview_vol_l) / 0x3fffu;
        mix_r += ((int64_t)value * state->preview_vol_r) / 0x3fffu;

        step = voice->phase_step;
        voice->phase += step;
    }

    mix_l /= 2;
    mix_r /= 2;
    if ((state->preview_flags & SPU2LOG_VOICE_REVERB) != 0) {
        uint32_t pos = state->preview_reverb_pos;
        int64_t wet_l = state->preview_reverb_l[pos];
        int64_t wet_r = state->preview_reverb_r[pos];

        state->preview_reverb_l[pos] = (int32_t)(mix_l + (wet_l * 3) / 8);
        state->preview_reverb_r[pos] = (int32_t)(mix_r + (wet_r * 3) / 8);
        mix_l += wet_l / 3;
        mix_r += wet_r / 3;
        state->preview_reverb_pos = (pos + 1u) % PREVIEW_REVERB_FRAMES;
    }
    if (out_left != NULL) {
        *out_left = (int)mix_l;
    }
    if (out_right != NULL) {
        *out_right = (int)mix_r;
    }
}

static void copy_scaled_pcm_with_declick(PlayerState *state, int16_t *out_pcm, const int16_t *in_pcm, uint32_t frames)
{
    uint32_t i;
    int volume_percent;
    int16_t previous_l;
    int16_t previous_r;
    int have_previous;
    int frame_transition;

    if (state == NULL || out_pcm == NULL || in_pcm == NULL || frames == 0) {
        return;
    }

    volume_percent = get_volume_percent(state);
    previous_l = state->last_output_l;
    previous_r = state->last_output_r;
    have_previous = state->has_last_output;
    frame_transition = state->frame_audio_transition;

    for (i = 0; i < frames; ++i) {
        int left = ((int)in_pcm[i * 2u] * volume_percent) / 100;
        int right = ((int)in_pcm[i * 2u + 1u] * volume_percent) / 100;

        out_pcm[i * 2u] = clamp_i32_to_i16(left);
        out_pcm[i * 2u + 1u] = clamp_i32_to_i16(right);
    }

    if (!have_previous) {
        uint32_t ramp_frames =
            frames < PLAYER_STARTUP_FADE_FRAMES ? frames : PLAYER_STARTUP_FADE_FRAMES;

        for (i = 0; i < ramp_frames; ++i) {
            int current_l = out_pcm[i * 2u];
            int current_r = out_pcm[i * 2u + 1u];
            out_pcm[i * 2u] =
                clamp_i32_to_i16((current_l * (int)i) / (int)ramp_frames);
            out_pcm[i * 2u + 1u] =
                clamp_i32_to_i16((current_r * (int)i) / (int)ramp_frames);
        }
    } else if (frames >= 8u) {
        int first_l = out_pcm[0];
        int first_r = out_pcm[1];
        int diff_l = first_l - previous_l;
        int diff_r = first_r - previous_r;
        int threshold = frame_transition ? 0 : 12000;

        if (abs(diff_l) > threshold || abs(diff_r) > threshold) {
            uint32_t ramp_limit = frame_transition ? 192u : (state->frame_advance ? 64u : 32u);
            uint32_t ramp_frames = frames < ramp_limit ? frames : ramp_limit;
            for (i = 0; i < ramp_frames; ++i) {
                int current_l = out_pcm[i * 2u];
                int current_r = out_pcm[i * 2u + 1u];
                int weight = (int)(i + 1u);
                int denom = (int)(ramp_frames + 1u);
                out_pcm[i * 2u] = clamp_i32_to_i16(previous_l + ((current_l - previous_l) * weight) / denom);
                out_pcm[i * 2u + 1u] = clamp_i32_to_i16(previous_r + ((current_r - previous_r) * weight) / denom);
            }
        }
    }

    state->last_output_l = out_pcm[(frames - 1u) * 2u];
    state->last_output_r = out_pcm[(frames - 1u) * 2u + 1u];
    state->has_last_output = 1;
    state->frame_audio_transition = 0;
}

static int apply_tag_fade_to_pcm(
    PlayerState *state,
    int16_t *pcm,
    uint32_t frames)
{
    uint64_t source_end;
    uint64_t timeline_end;
    uint64_t timeline_span;
    uint64_t timeline_start;
    uint64_t fade_start;
    uint64_t fade_duration;
    uint64_t fade_end;
    int enabled;
    uint32_t i;

    if (state == NULL || pcm == NULL || frames == 0) {
        return 0;
    }

    source_end = spu2log_audacious_get_sample_pos();
    lock_state(state);
    enabled = state->tag_fade_enabled;
    fade_start = state->total_samples;
    fade_duration = state->fade_samples;
    timeline_end = timeline_sample_at_locked(state, source_end);
    timeline_span = scale_timeline_delta(
        frames, state->timeline_speed_percent);
    unlock_state(state);

    if (!enabled || fade_start == 0 || timeline_end < fade_start) {
        return 0;
    }
    timeline_start = timeline_end > timeline_span ?
        timeline_end - timeline_span : 0;
    fade_end = UINT64_MAX - fade_start < fade_duration ?
        UINT64_MAX : fade_start + fade_duration;

    for (i = 0; i < frames; ++i) {
        uint64_t timeline_pos = timeline_start +
            (timeline_span * (uint64_t)i) / frames;
        uint64_t remaining;

        if (timeline_pos < fade_start) {
            continue;
        }
        if (fade_duration == 0 || timeline_pos >= fade_end) {
            pcm[i * 2u] = 0;
            pcm[i * 2u + 1u] = 0;
            continue;
        }
        remaining = fade_end - timeline_pos;
        pcm[i * 2u] = (int16_t)(
            ((int64_t)pcm[i * 2u] * (int64_t)remaining) /
            (int64_t)fade_duration);
        pcm[i * 2u + 1u] = (int16_t)(
            ((int64_t)pcm[i * 2u + 1u] * (int64_t)remaining) /
            (int64_t)fade_duration);
    }

    return timeline_end >= fade_end;
}

static int drain_audio_queue_after_tag_fade(PlayerState *state)
{
    if (state == NULL || state->stop_event == NULL) {
        return 0;
    }
    if (!state->audio_started && state->wave != NULL &&
        state->audio_queued_buffers > 0) {
        state->audio_started = 1;
        sync_audible_display_to_queue(state);
        if (waveOutRestart(state->wave) != MMSYSERR_NOERROR) {
            state->audio_started = 0;
            return 0;
        }
        if (state->hwnd != NULL) {
            SendMessageA(
                state->hwnd,
                PLAYER_WM_AUDIO_STARTING,
                0,
                (LPARAM)(UINT_PTR)state->wave);
        }
    }
    while (WaitForSingleObject(state->stop_event, 0) == WAIT_TIMEOUT) {
        reap_audio_queue(state);
        if (state->audio_queued_buffers == 0) {
            return 1;
        }
        Sleep(1);
    }
    return 0;
}

static void preview_cleanup_standalone_queue(PlayerState *state)
{
    unsigned i;

    if (state == NULL) {
        return;
    }
    if (state->preview_wave != NULL) {
        waveOutReset(state->preview_wave);
    }
    for (i = 0; i < PREVIEW_AUDIO_BUFFERS; ++i) {
        if (state->preview_audio_in_use[i] && state->preview_wave != NULL) {
            waveOutUnprepareHeader(
                state->preview_wave,
                &state->preview_audio_headers[i],
                sizeof(state->preview_audio_headers[i]));
        }
        state->preview_audio_in_use[i] = 0;
        free(state->preview_audio_buffers[i]);
        state->preview_audio_buffers[i] = NULL;
        ZeroMemory(&state->preview_audio_headers[i], sizeof(state->preview_audio_headers[i]));
    }
    if (state->preview_wave != NULL) {
        waveOutClose(state->preview_wave);
        state->preview_wave = NULL;
    }
}

static void preview_reap_standalone_queue(PlayerState *state)
{
    unsigned i;

    if (state == NULL || state->preview_wave == NULL) {
        return;
    }
    for (i = 0; i < PREVIEW_AUDIO_BUFFERS; ++i) {
        if (state->preview_audio_in_use[i] &&
            (state->preview_audio_headers[i].dwFlags & WHDR_DONE) != 0) {
            waveOutUnprepareHeader(
                state->preview_wave,
                &state->preview_audio_headers[i],
                sizeof(state->preview_audio_headers[i]));
            state->preview_audio_in_use[i] = 0;
            ZeroMemory(&state->preview_audio_headers[i], sizeof(state->preview_audio_headers[i]));
        }
    }
}

static DWORD WINAPI preview_standalone_thread_proc(void *user)
{
    PlayerState *state = (PlayerState *)user;
    unsigned i;

    if (state == NULL || state->preview_audio_stop_event == NULL ||
        !open_waveout(&state->preview_wave, NULL)) {
        return 0;
    }
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    for (i = 0; i < PREVIEW_AUDIO_BUFFERS; ++i) {
        state->preview_audio_buffers[i] =
            (int16_t *)calloc(PREVIEW_AUDIO_FRAMES * 2u, sizeof(*state->preview_audio_buffers[i]));
        if (state->preview_audio_buffers[i] == NULL) {
            preview_cleanup_standalone_queue(state);
            return 0;
        }
    }

    while (WaitForSingleObject(state->preview_audio_stop_event, 0) == WAIT_TIMEOUT) {
        int free_buffer = -1;

        preview_reap_standalone_queue(state);
        for (i = 0; i < PREVIEW_AUDIO_BUFFERS; ++i) {
            if (!state->preview_audio_in_use[i]) {
                free_buffer = (int)i;
                break;
            }
        }
        if (free_buffer < 0) {
            Sleep(1);
            continue;
        }
        {
            int volume_percent = get_volume_percent(state);
            int16_t *buffer = state->preview_audio_buffers[free_buffer];
            WAVEHDR *header = &state->preview_audio_headers[free_buffer];
            uint32_t frame;
            MMRESULT result;

            lock_state(state);
            for (frame = 0; frame < PREVIEW_AUDIO_FRAMES; ++frame) {
                int left = 0;
                int right = 0;
                preview_render_frame_locked(state, &left, &right);
                buffer[frame * 2u] = clamp_i32_to_i16((left * volume_percent) / 100);
                buffer[frame * 2u + 1u] = clamp_i32_to_i16((right * volume_percent) / 100);
            }
            unlock_state(state);

            ZeroMemory(header, sizeof(*header));
            header->lpData = (LPSTR)buffer;
            header->dwBufferLength = PREVIEW_AUDIO_FRAMES * 2u * (uint32_t)sizeof(*buffer);
            result = waveOutPrepareHeader(state->preview_wave, header, sizeof(*header));
            if (result != MMSYSERR_NOERROR) {
                break;
            }
            result = waveOutWrite(state->preview_wave, header, sizeof(*header));
            if (result != MMSYSERR_NOERROR) {
                waveOutUnprepareHeader(state->preview_wave, header, sizeof(*header));
                ZeroMemory(header, sizeof(*header));
                break;
            }
            state->preview_audio_in_use[free_buffer] = 1;
        }
    }
    preview_cleanup_standalone_queue(state);
    return 0;
}

static void preview_start_standalone_audio(PlayerState *state)
{
    if (state == NULL) {
        return;
    }
    if (state->preview_audio_thread != NULL) {
        if (WaitForSingleObject(state->preview_audio_thread, 0) == WAIT_TIMEOUT) {
            return;
        }
        CloseHandle(state->preview_audio_thread);
        state->preview_audio_thread = NULL;
    }
    if (state->preview_audio_stop_event != NULL) {
        CloseHandle(state->preview_audio_stop_event);
        state->preview_audio_stop_event = NULL;
    }
    state->preview_audio_stop_event = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (state->preview_audio_stop_event == NULL) {
        return;
    }
    state->preview_audio_thread = CreateThread(
        NULL,
        0,
        preview_standalone_thread_proc,
        state,
        0,
        NULL);
    if (state->preview_audio_thread == NULL) {
        CloseHandle(state->preview_audio_stop_event);
        state->preview_audio_stop_event = NULL;
    }
}

static void preview_stop_standalone_audio(PlayerState *state)
{
    if (state == NULL) {
        return;
    }
    if (state->preview_audio_stop_event != NULL) {
        SetEvent(state->preview_audio_stop_event);
    }
    if (state->preview_wave != NULL) {
        waveOutReset(state->preview_wave);
    }
    if (state->preview_audio_thread != NULL) {
        WaitForSingleObject(state->preview_audio_thread, 3000);
        CloseHandle(state->preview_audio_thread);
        state->preview_audio_thread = NULL;
    }
    if (state->preview_audio_stop_event != NULL) {
        CloseHandle(state->preview_audio_stop_event);
        state->preview_audio_stop_event = NULL;
    }
}

static void smooth_pcm_splice(int16_t *pcm, uint32_t frames, uint32_t splice_frame)
{
    uint32_t i;
    uint32_t ramp_frames = 64u;
    uint32_t start;
    uint32_t end;
    int16_t left_before;
    int16_t right_before;
    int16_t left_after;
    int16_t right_after;

    if (pcm == NULL || frames < 4u || splice_frame == 0 || splice_frame >= frames) {
        return;
    }

    if (splice_frame < ramp_frames) {
        ramp_frames = splice_frame;
    }
    if (frames - splice_frame < ramp_frames) {
        ramp_frames = frames - splice_frame;
    }
    if (ramp_frames < 4u) {
        return;
    }

    start = splice_frame - ramp_frames;
    end = splice_frame + ramp_frames;
    left_before = pcm[(splice_frame - 1u) * 2u];
    right_before = pcm[(splice_frame - 1u) * 2u + 1u];
    left_after = pcm[splice_frame * 2u];
    right_after = pcm[splice_frame * 2u + 1u];

    for (i = start; i < end; ++i) {
        int weight = (int)(i - start + 1u);
        int denom = (int)((end - start) + 1u);
        int current_l = pcm[i * 2u];
        int current_r = pcm[i * 2u + 1u];
        int bridge_l = (int)left_before + (((int)left_after - (int)left_before) * weight) / denom;
        int bridge_r = (int)right_before + (((int)right_after - (int)right_before) * weight) / denom;
        pcm[i * 2u] = clamp_i32_to_i16((current_l + bridge_l) / 2);
        pcm[i * 2u + 1u] = clamp_i32_to_i16((current_r + bridge_r) / 2);
    }
}

static DWORD WINAPI playback_thread_proc(void *user)
{
    PlayerState *state = (PlayerState *)user;
    uint32_t debug_render_count = 0;
    uint32_t silent_render_count = 0;
    uint64_t startup_trimmed_frames = 0;
    uint32_t playback_generation = 0;
    int natural_tag_end = 0;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    if (state != NULL) {
        lock_state(state);
        playback_generation = state->playback_generation;
        unlock_state(state);
    }
    player_log("playback thread begin");
    while (state != NULL && WaitForSingleObject(state->stop_event, 0) == WAIT_TIMEOUT) {
        Psf2CoreBridgeResult result;
        uint32_t rendered = 0;

        if (InterlockedCompareExchange(&state->graceful_stop_requested, 0, 0) != 0) {
            ZeroMemory(state->pcm, PLAYER_TRACK_SWITCH_FADE_FRAMES * 2u * sizeof(*state->pcm));
            state->frame_audio_transition = 1;
            if (queue_waveout_chunk(state, state->pcm, PLAYER_TRACK_SWITCH_FADE_FRAMES)) {
                if (!state->audio_started && state->wave != NULL &&
                    state->audio_queued_buffers > 0) {
                    state->audio_started = 1;
                    sync_audible_display_to_queue(state);
                    if (waveOutRestart(state->wave) != MMSYSERR_NOERROR) {
                        state->audio_started = 0;
                    } else if (state->hwnd != NULL) {
                        SendMessageA(
                            state->hwnd,
                            PLAYER_WM_AUDIO_STARTING,
                            0,
                            (LPARAM)(UINT_PTR)state->wave);
                    }
                }
                while (WaitForSingleObject(state->stop_event, 0) == WAIT_TIMEOUT) {
                    reap_audio_queue(state);
                    if (state->audio_queued_buffers == 0) {
                        break;
                    }
                    Sleep(1);
                }
            }
            break;
        }

        if (get_paused(state)) {
            Sleep(10);
            continue;
        }

        if (state->provider == NULL || state->core == NULL || state->pcm == NULL) {
            player_log("playback thread missing core/provider/pcm");
            set_status(state, "Stopped: playback core is not open");
            break;
        }

        if (state->frame_advance) {
            uint32_t steps = 0;

            lock_state(state);
            steps = state->frame_step_request;
            if (steps > 0) {
                state->frame_step_request = steps - 1u;
                state->frame_capture_until = state->live.last_sample_pos + (uint64_t)PLAYER_FRAME_ADVANCE_TICK_FRAMES;
            }
            unlock_state(state);

            if (steps > 0) {
                uint32_t step_rendered = 0;

                psf2log_step_imported_frame_advance(PLAYER_FRAME_ADVANCE_TICK_FRAMES);
                result = state->provider->render(state->core, state->pcm, PLAYER_FRAME_ADVANCE_TICK_FRAMES, &step_rendered);
                if (result != PSF2_CORE_BRIDGE_OK || step_rendered == 0) {
                    set_status(state, "Frame advance render failed");
                    break;
                }
                psf2log_emit_imported_snapshot(state->core);

                lock_state(state);
                if (!state->frame_live_valid) {
                    state->frame_live = state->live;
                    state->frame_live_valid = 1;
                }
                state->frame_live_valid = 1;
                state->frame_capture_until = 0;
                state->frame_audio_transition = 1;
                state->sequence += 1;
                unlock_state(state);
                update_time_label(state);
                InvalidateRect(state->hwnd, NULL, TRUE);

                psf2log_set_imported_adsr_freeze(1);
                result = state->provider->render(state->core, state->pcm, PLAYER_RENDER_FRAMES, &rendered);
                if (result != PSF2_CORE_BRIDGE_OK || rendered == 0) {
                    set_status(state, "Frame advance audio failed");
                    break;
                }
                if (!queue_waveout_chunk(state, state->pcm, rendered)) {
                    set_status(state, "Stopped: waveOut write failed");
                    break;
                }
            } else {
                psf2log_set_imported_adsr_freeze(1);
                result = state->provider->render(state->core, state->pcm, PLAYER_RENDER_FRAMES, &rendered);
                if (result != PSF2_CORE_BRIDGE_OK || rendered == 0) {
                    set_status(state, "Frame advance audio failed");
                    break;
                }
                if (!queue_waveout_chunk(state, state->pcm, rendered)) {
                    set_status(state, "Stopped: waveOut write failed");
                    break;
                }
            }
            continue;
        }

        {
            int has_seek_request;
            uint64_t current_sample;
            uint64_t target_sample;

            lock_state(state);
            has_seek_request = state->seek_request;
            current_sample = state->live.last_sample_pos;
            target_sample = state->seek_target_sample;
            if (has_seek_request) {
                state->seek_request = 0;
            }
            unlock_state(state);

            if (has_seek_request) {
                int seek_result;

                player_log("playback thread seek current=%llu target=%llu",
                    (unsigned long long)current_sample,
                    (unsigned long long)target_sample);
                if (state->wave != NULL) {
                    if (!restart_audio_prebuffer(state)) {
                        set_status(state, "Seek audio reset failed");
                        break;
                    }
                }
                do {
                    if (target_sample <= current_sample) {
                        if (!reopen_core_for_seek(state, &current_sample)) {
                            seek_result = 0;
                            set_status(state, "Seek failed");
                            break;
                        }
                    }

                    seek_result = fast_forward_core(state, target_sample - current_sample, current_sample);
                    if (seek_result == 2) {
                        lock_state(state);
                        target_sample = state->seek_target_sample;
                        state->seek_request = 0;
                        current_sample = state->live.last_sample_pos;
                        unlock_state(state);
                        player_log("playback thread seek retarget current=%llu target=%llu",
                            (unsigned long long)current_sample,
                            (unsigned long long)target_sample);
                    }
                } while (seek_result == 2);

                if (seek_result != 1) {
                    set_status(state, "Seek failed");
                    if (seek_result == 0) {
                        break;
                    }
                }
                set_status(state, "Playing direct");
                continue;
            }
        }

        result = state->provider->render(state->core, state->pcm, PLAYER_RENDER_FRAMES, &rendered);
        if (result != PSF2_CORE_BRIDGE_OK || rendered == 0) {
            char message[128];
            player_log("playback thread render stop result=%d rendered=%lu", (int)result, (unsigned long)rendered);
            player_log_audacious_debug_state("render stop");
            snprintf(message, sizeof(message),
                "Stopped: %s",
                psf2_core_bridge_result_string(result));
            set_status(state, message);
            break;
        }

        debug_render_count += 1;
        if (pcm_peak_abs(state->pcm, rendered) == 0) {
            silent_render_count += 1;
        }
        if (debug_render_count <= 8 || (debug_render_count % 64u) == 0u) {
            player_log(
                "playback thread audio chunk=%lu rendered=%lu peak=%d silent_chunks=%lu sample=%llu hook_writes=%llu",
                (unsigned long)debug_render_count,
                (unsigned long)rendered,
                (int)pcm_peak_abs(state->pcm, rendered),
                (unsigned long)silent_render_count,
                (unsigned long long)spu2log_audacious_get_sample_pos(),
                (unsigned long long)spu2log_audacious_get_spu2_write16_count());
        }

        if (!state->startup_silence_trim) {
            start_timeline_at_first_audible_frame(state, state->pcm, rendered);
        }

        if (state->startup_silence_trim) {
            uint64_t block_end_sample = spu2log_audacious_get_sample_pos();
            uint32_t first_nonzero = first_ps2_song_frame(
                state,
                state->pcm,
                rendered,
                block_end_sample);

            if (first_nonzero == rendered) {
                startup_trimmed_frames += rendered;
                continue;
            }
            remember_ps2_startup_origin(
                state,
                block_end_sample,
                rendered,
                first_nonzero);
            state->startup_silence_trim = 0;
            psf2log_rebase_imported_sample_position(
                state->core,
                rendered);
            reset_live_display(state);
            lock_state(state);
            state->timeline_source_anchor = 0;
            state->timeline_scaled_anchor = 0;
            state->timeline_speed_percent =
                effective_speed_percent_locked(state);
            state->timeline_valid = 0;
            state->time_label_cache_valid = 0;
            unlock_state(state);
            psf2log_emit_imported_snapshot(state->core);
            start_timeline_at_first_audible_frame(
                state, state->pcm, rendered);
            player_log("PS2 startup silence trimmed frames=%llu seconds=%.3f",
                (unsigned long long)startup_trimmed_frames,
                (double)startup_trimmed_frames / (double)PLAYER_SAMPLE_RATE);
            {
                int16_t startup_silence[PLAYER_RENDER_FRAMES * 2u];
                int16_t startup_packet[PLAYER_RENDER_FRAMES * 2u];
                uint32_t silence_frames = PLAYER_RENDER_FRAMES - rendered;
                unsigned silence_buffer;
                int startup_queue_failed = 0;

                /* Keep every retained song sample, but avoid handing waveOut
                   a very short first buffer when the onset falls near the end
                   of the render block. */
                ZeroMemory(startup_silence, sizeof(startup_silence));
                ZeroMemory(startup_packet, sizeof(startup_packet));
                memcpy(startup_packet + silence_frames * 2u,
                    state->pcm,
                    rendered * 2u * sizeof(*state->pcm));
                state->has_last_output = 0;
                for (silence_buffer = 0;
                     silence_buffer < PLAYER_STARTUP_SILENCE_BUFFERS;
                     ++silence_buffer) {
                    if (!queue_waveout_chunk(
                            state,
                            startup_silence,
                            PLAYER_RENDER_FRAMES)) {
                        startup_queue_failed = 1;
                        break;
                    }
                }
                if (startup_queue_failed) {
                    player_log("playback thread startup silence queue failed");
                    set_status(state, "Stopped: waveOut write failed");
                    break;
                }
                lock_state(state);
                state->ps2_startup_track_dimmed = 0;
                unlock_state(state);
                state->last_output_l = startup_packet[0];
                state->last_output_r = startup_packet[1];
                state->has_last_output = 1;
                state->frame_audio_transition = 0;
                if (!queue_waveout_chunk(
                        state,
                        startup_packet,
                        PLAYER_RENDER_FRAMES)) {
                    player_log("playback thread startup packet queue failed");
                    set_status(state, "Stopped: waveOut write failed");
                    break;
                }
            }
            continue;
        }

        {
            int tag_fade_complete = apply_tag_fade_to_pcm(
                state, state->pcm, rendered);

            if (!queue_waveout_chunk(state, state->pcm, rendered)) {
                player_log("playback thread waveOut queue failed");
                set_status(state, "Stopped: waveOut write failed");
                break;
            }
            if (tag_fade_complete) {
                natural_tag_end = drain_audio_queue_after_tag_fade(state);
                break;
            }
        }
        psf1_process_tuning_request(state);
    }

    set_playing(state, 0);
    if (state != NULL && state->hwnd != NULL) {
        PostMessageA(state->hwnd, PLAYER_WM_WORKER_UPDATE, 0, 0);
        if (natural_tag_end) {
            PostMessageA(state->hwnd, PLAYER_WM_PLAYBACK_ENDED,
                (WPARAM)playback_generation, 0);
        }
    }
    player_log("playback thread end");
    return 0;
}

static void stop_playback_internal(PlayerState *state, int graceful)
{
    int thread_closed = 0;

    if (state == NULL) {
        return;
    }

    preview_stop_standalone_audio(state);
    player_log("stop playback begin");
    set_playing(state, 0);
    set_paused(state, 0);
    lock_state(state);
    state->frame_advance = 0;
    state->frame_live_valid = 0;
    state->frame_capture_until = 0;
    state->frame_step_request = 0;
    state->frame_audio_transition = 0;
    state->tab_speed_active = 0;
    preview_stop_all_locked(state);
    unlock_state(state);
    psf2log_set_imported_frame_advance_mode(0);
    psf2log_set_imported_adsr_freeze(0);
    apply_effective_speed_percent(state);
    state->seek_request = 0;
    state->seek_target_sample = 0;
    state->startup_silence_trim = 0;
    update_pause_button_label(state);

    if (graceful && state->thread != NULL && state->stop_event != NULL &&
        WaitForSingleObject(state->stop_event, 0) == WAIT_TIMEOUT) {
        if (state->wave != NULL && state->audio_started) {
            waveOutRestart(state->wave);
        }
        InterlockedExchange(&state->graceful_stop_requested, 1);
        if (WaitForSingleObject(state->thread, 1000) == WAIT_OBJECT_0) {
            CloseHandle(state->thread);
            state->thread = NULL;
            thread_closed = 1;
        }
    }

    if (!thread_closed) {
        if (state->stop_event != NULL) {
            SetEvent(state->stop_event);
        }
        psf2log_abort_imported_render();

        if (state->wave != NULL) {
            waveOutReset(state->wave);
        }
    }

    if (state->thread != NULL) {
        DWORD wait_result = WaitForSingleObject(state->thread, 3000);
        if (wait_result == WAIT_OBJECT_0) {
            CloseHandle(state->thread);
            state->thread = NULL;
        } else {
            player_log("stop playback timed out waiting for playback thread");
            player_log_audacious_debug_state("stop timeout before terminate");
            TerminateThread(state->thread, 1);
            CloseHandle(state->thread);
            state->thread = NULL;
        }
    }

    if (state->stop_event != NULL) {
        CloseHandle(state->stop_event);
        state->stop_event = NULL;
    }

    InterlockedExchange(&state->graceful_stop_requested, 0);
    state->has_last_output = 0;

    cleanup_audio_queue(state);

    if (state->wave != NULL) {
        waveOutClose(state->wave);
        state->wave = NULL;
    }

    if (state->core != NULL && state->provider != NULL && state->provider->close != NULL) {
        state->provider->close(state->core);
        state->core = NULL;
    }

    if (state->pcm != NULL) {
        free(state->pcm);
        state->pcm = NULL;
    }

    update_time_label(state);
    player_log("stop playback end");
}

static void stop_playback(PlayerState *state)
{
    stop_playback_internal(state, 0);
}

static void stop_playback_for_track_switch(PlayerState *state)
{
    /* Discard queued audio from the previous song immediately. Draining the
       old queue here can make its final note sound after the new song has
       already been selected. The new stream has its own startup fade-in. */
    stop_playback_internal(state, 0);
}

static void copy_first_command_arg(char *out_path, size_t out_size, const char *cmd_line)
{
    size_t length;

    if (out_size == 0) {
        return;
    }
    out_path[0] = '\0';

    if (cmd_line == NULL) {
        return;
    }

    while (*cmd_line == ' ' || *cmd_line == '\t') {
        ++cmd_line;
    }

    if (*cmd_line == '"') {
        const char *end;
        ++cmd_line;
        end = strchr(cmd_line, '"');
        if (end == NULL) {
            end = cmd_line + strlen(cmd_line);
        }
        length = (size_t)(end - cmd_line);
    } else {
        const char *end = cmd_line;
        while (*end != '\0' && *end != ' ' && *end != '\t') {
            ++end;
        }
        length = (size_t)(end - cmd_line);
    }

    if (length >= out_size) {
        length = out_size - 1;
    }
    memcpy(out_path, cmd_line, length);
    out_path[length] = '\0';
}

static const wchar_t g_music_open_filter[] =
    L"PSF/archive files (*.psf;*.minipsf;*.psf2;*.minipsf2;*.zip;*.7z;*.rar;*.lha;*.lzh)\0"
    L"*.psf;*.minipsf;*.psf2;*.minipsf2;*.zip;*.7z;*.rar;*.lha;*.lzh\0"
    L"PSF files (*.psf;*.minipsf;*.psf2;*.minipsf2)\0*.psf;*.minipsf;*.psf2;*.minipsf2\0"
    L"Archive files (*.zip;*.7z;*.rar;*.lha;*.lzh)\0"
    L"*.zip;*.7z;*.rar;*.lha;*.lzh\0"
    L"All files (*.*)\0*.*\0";

static int open_psf2_file(HWND hwnd, char *out_path, size_t out_size)
{
    OPENFILENAMEW ofn;
    wchar_t path[MAX_PATH];
    PlayerState *state;

    player_log("open dialog begin");
    ZeroMemory(&ofn, sizeof(ofn));
    ZeroMemory(path, sizeof(path));
    state = (PlayerState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
    if (state != NULL && state->playlist_last_file[0] != '\0') {
        runtime_path_to_wide(state->playlist_last_file, path, MAX_PATH);
    } else if (state != NULL && state->input_path[0] != '\0') {
        runtime_path_to_wide(state->input_path, path, MAX_PATH);
    }
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = g_music_open_filter;
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    apply_app_menu_dark_mode(state != NULL && is_dark_theme_active(state));
    if (!GetOpenFileNameW(&ofn)) {
        player_log("open dialog canceled or failed error=%lu", (unsigned long)CommDlgExtendedError());
        return 0;
    }

    if (!wide_path_to_runtime_path(path, out_path, out_size)) {
        player_log("open dialog selected path could not be represented");
        return 0;
    }
    player_log("open dialog selected path=%s", out_path);
    return 1;
}

static int open_psf_files_to_playlist(HWND hwnd, PlayerState *state, char *out_first_path, size_t out_first_size)
{
    OPENFILENAMEW ofn;
    wchar_t *buffer;
    wchar_t initial_dir[MAX_PATH];
    int added = 0;
    int selected = 0;
    int replace_pending = state != NULL && !state->playlist_append_mode && state->playlist_count > 0;

    if (out_first_path != NULL && out_first_size > 0) {
        out_first_path[0] = '\0';
    }
    buffer = (wchar_t *)calloc(65536u, sizeof(wchar_t));
    if (buffer == NULL) {
        return 0;
    }

    ZeroMemory(&ofn, sizeof(ofn));
    ZeroMemory(initial_dir, sizeof(initial_dir));
    if (state != NULL && state->playlist_last_file[0] != '\0') {
        runtime_path_to_wide(state->playlist_last_file, buffer, 65536u);
    } else if (state != NULL && state->input_path[0] != '\0') {
        runtime_path_to_wide(state->input_path, buffer, 65536u);
    }
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = g_music_open_filter;
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = 65536u;
    if (state != NULL && state->playlist_last_folder[0] != '\0') {
        if (runtime_path_to_wide(state->playlist_last_folder, initial_dir, MAX_PATH)) {
            ofn.lpstrInitialDir = initial_dir;
        }
    }
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

    apply_app_menu_dark_mode(state != NULL && is_dark_theme_active(state));
    if (GetOpenFileNameW(&ofn)) {
        wchar_t *name = buffer + wcslen(buffer) + 1u;
        selected = 1;
        if (*name == L'\0') {
            char first[MAX_PATH];
            char path[MAX_PATH];
            if (wide_path_to_runtime_path(buffer, path, sizeof(path))) {
                remember_playlist_file(state, path);
            }
            if (path[0] != '\0' && add_music_source_to_playlist(hwnd, state, path,
                    replace_pending && (is_directory_path(path) || is_archive_path(path)),
                    first, sizeof(first))) {
                added += 1;
                if (out_first_path != NULL && out_first_size > 0) {
                    snprintf(out_first_path, out_first_size, "%s", first);
                }
            }
        } else {
            char folder[MAX_PATH];
            wide_path_to_runtime_path(buffer, folder, sizeof(folder));
            if (folder[0] != '\0') {
                remember_playlist_folder(state, folder);
            }
            while (*name != L'\0') {
                wchar_t path_w[MAX_PATH];
                char path[MAX_PATH];
                char first[MAX_PATH];
                path[0] = '\0';
                if (swprintf(path_w, MAX_PATH, L"%ls\\%ls", buffer, name) >= 0) {
                    wide_path_to_runtime_path(path_w, path, sizeof(path));
                }
                if (path[0] != '\0') {
                    remember_playlist_file(state, path);
                }
                if (path[0] != '\0' && add_music_source_to_playlist(hwnd, state, path,
                        replace_pending && (is_directory_path(path) || is_archive_path(path)),
                        first, sizeof(first))) {
                    added += 1;
                    if (is_directory_path(path) || is_archive_path(path)) {
                        replace_pending = 0;
                    }
                    if (out_first_path != NULL && out_first_size > 0 && out_first_path[0] == '\0') {
                        snprintf(out_first_path, out_first_size, "%s", first);
                    }
                }
                name += wcslen(name) + 1u;
            }
        }
    }

    free(buffer);
    (void)added;
    return selected && out_first_path != NULL && out_first_path[0] != '\0';
}

static int choose_playlist_folder(HWND hwnd, char *out_folder, size_t out_size)
{
    IFileOpenDialog *dialog = NULL;
    IShellItem *initial_folder = NULL;
    IShellItem *selected_item = NULL;
    PWSTR selected_path = NULL;
    FILEOPENDIALOGOPTIONS options = 0;
    PlayerState *state;
    HRESULT init_result;
    HRESULT result;
    int uninitialize_com = 0;
    int ok = 0;

    if (out_folder == NULL || out_size == 0) {
        return 0;
    }
    out_folder[0] = '\0';
    state = (PlayerState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    init_result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(init_result)) {
        uninitialize_com = 1;
    } else if (init_result != RPC_E_CHANGED_MODE) {
        return 0;
    }

    apply_app_menu_dark_mode(state != NULL && is_dark_theme_active(state));
    result = CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
        &IID_IFileOpenDialog, (void **)&dialog);
    if (FAILED(result) || dialog == NULL) {
        goto cleanup;
    }

    if (SUCCEEDED(dialog->lpVtbl->GetOptions(dialog, &options))) {
        options |= FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST;
        dialog->lpVtbl->SetOptions(dialog, options);
    }
    dialog->lpVtbl->SetTitle(dialog, L"Select music folder");

    if (state != NULL && state->playlist_last_folder[0] != '\0') {
        wchar_t initial_path[MAX_PATH];
        wchar_t initial_name[MAX_PATH];
        char parent_folder[MAX_PATH];
        const char *folder_to_open;

        path_parent_folder(state->playlist_last_folder,
            parent_folder, sizeof(parent_folder));
        folder_to_open = parent_folder[0] != '\0' ?
            parent_folder : state->playlist_last_folder;
        if (runtime_path_to_wide(folder_to_open, initial_path, MAX_PATH) &&
            SUCCEEDED(SHCreateItemFromParsingName(initial_path, NULL, &IID_IShellItem,
                (void **)&initial_folder))) {
            dialog->lpVtbl->SetFolder(dialog, initial_folder);
        }
        if (runtime_path_to_wide(path_basename(state->playlist_last_folder),
                initial_name, MAX_PATH)) {
            dialog->lpVtbl->SetFileName(dialog, initial_name);
        }
    }

    result = dialog->lpVtbl->Show(dialog, hwnd);
    if (FAILED(result) || FAILED(dialog->lpVtbl->GetResult(dialog, &selected_item)) ||
        selected_item == NULL) {
        goto cleanup;
    }
    if (FAILED(selected_item->lpVtbl->GetDisplayName(
            selected_item, SIGDN_FILESYSPATH, &selected_path)) || selected_path == NULL) {
        goto cleanup;
    }
    if (wide_path_to_runtime_path(selected_path, out_folder, out_size)) {
        remember_playlist_folder(state, out_folder);
        ok = 1;
    }

cleanup:
    if (selected_path != NULL) {
        CoTaskMemFree(selected_path);
    }
    if (selected_item != NULL) {
        selected_item->lpVtbl->Release(selected_item);
    }
    if (initial_folder != NULL) {
        initial_folder->lpVtbl->Release(initial_folder);
    }
    if (dialog != NULL) {
        dialog->lpVtbl->Release(dialog);
    }
    if (uninitialize_com) {
        CoUninitialize();
    }
    return ok;
}

static void playlist_save_to_file(HWND hwnd, PlayerState *state)
{
    OPENFILENAMEA ofn;
    char path[MAX_PATH];
    FILE *file;
    unsigned i;

    if (state == NULL) {
        return;
    }
    ZeroMemory(&ofn, sizeof(ofn));
    ZeroMemory(path, sizeof(path));
    if (state->playlist_last_folder[0] != '\0') {
        snprintf(path, sizeof(path), "%s\\playlist.m3u", state->playlist_last_folder);
    } else {
        snprintf(path, sizeof(path), "playlist.m3u");
    }
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Playlist (*.m3u)\0*.m3u\0Text files (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = sizeof(path);
    if (state->playlist_last_folder[0] != '\0') {
        ofn.lpstrInitialDir = state->playlist_last_folder;
    }
    ofn.lpstrDefExt = "m3u";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    apply_app_menu_dark_mode(is_dark_theme_active(state));
    if (!GetSaveFileNameA(&ofn)) {
        return;
    }
    remember_playlist_file(state, path);
    file = fopen(path, "wb");
    if (file == NULL) {
        MessageBoxA(hwnd, "Could not save playlist.", "Playlist", MB_OK | MB_ICONWARNING);
        return;
    }
    for (i = 0; i < state->playlist_count; ++i) {
        fprintf(file, "%s\r\n", state->playlist_paths[i]);
    }
    fclose(file);
}

static void playlist_load_from_file(HWND hwnd, PlayerState *state)
{
    OPENFILENAMEA ofn;
    char path[MAX_PATH];
    FILE *file;
    char line[MAX_PATH + 8];

    if (state == NULL) {
        return;
    }
    ZeroMemory(&ofn, sizeof(ofn));
    ZeroMemory(path, sizeof(path));
    if (state->playlist_last_file[0] != '\0') {
        snprintf(path, sizeof(path), "%s", state->playlist_last_file);
    }
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Playlist (*.m3u;*.txt)\0*.m3u;*.txt\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = sizeof(path);
    if (state->playlist_last_folder[0] != '\0') {
        ofn.lpstrInitialDir = state->playlist_last_folder;
    }
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    apply_app_menu_dark_mode(is_dark_theme_active(state));
    if (!GetOpenFileNameA(&ofn)) {
        return;
    }
    remember_playlist_file(state, path);
    file = fopen(path, "rb");
    if (file == NULL) {
        MessageBoxA(hwnd, "Could not open playlist.", "Playlist", MB_OK | MB_ICONWARNING);
        return;
    }
    playlist_clear(state);
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] != '\0' && line[0] != '#') {
            playlist_add_path(state, line);
        }
    }
    fclose(file);
}

static void playlist_play_index(HWND hwnd, PlayerState *state, int index)
{
    HWND target;

    if (state == NULL || index < 0 || (unsigned)index >= state->playlist_count) {
        return;
    }

    state->playlist_pending_play_index = index;
    target = state->hwnd != NULL ? state->hwnd : hwnd;
    cancel_direct_start_retry(target, state);
    player_log("playlist play requested index=%d current=%d busy=%d",
        index,
        state->playlist_current,
        state->playlist_play_in_progress);
    if (target == NULL || !PostMessageA(target, PLAYER_WM_PLAYLIST_PLAY, 0, 0)) {
        player_log("playlist play request post failed error=%lu",
            (unsigned long)GetLastError());
    }
}

static void playlist_request_playback_stop(PlayerState *state)
{
    if (state == NULL) {
        return;
    }

    set_playing(state, 0);
    set_paused(state, 0);
    state->seek_request = 0;
    state->seek_target_sample = 0;
    if (state->stop_event != NULL) {
        SetEvent(state->stop_event);
    }
    psf2log_abort_imported_render();
    if (state->wave != NULL) {
        waveOutReset(state->wave);
    }
}

static void playlist_cancel_pending_play(HWND hwnd, PlayerState *state)
{
    HWND target;

    if (state == NULL) {
        return;
    }
    target = state->hwnd != NULL ? state->hwnd : hwnd;
    if (target != NULL) {
        KillTimer(target, PLAYER_PLAYLIST_SWITCH_TIMER_ID);
    }
    state->playlist_pending_play_index = -1;
    state->playlist_play_in_progress = 0;
}

static void playlist_process_pending_play(HWND hwnd, PlayerState *state)
{
    char path[MAX_PATH];
    int index;

    if (state == NULL) {
        return;
    }

    if (!state->playlist_play_in_progress) {
        index = state->playlist_pending_play_index;
        if (index < 0 || (unsigned)index >= state->playlist_count) {
            state->playlist_pending_play_index = -1;
            return;
        }
        state->playlist_play_in_progress = 1;
        if (state->thread != NULL &&
            WaitForSingleObject(state->thread, 0) == WAIT_TIMEOUT) {
            player_log("playlist asynchronous stop begin index=%d", index);
            playlist_request_playback_stop(state);
            SetTimer(hwnd,
                PLAYER_PLAYLIST_SWITCH_TIMER_ID,
                PLAYER_PLAYLIST_SWITCH_TIMER_MS,
                NULL);
            return;
        }
    } else if (state->thread != NULL &&
        WaitForSingleObject(state->thread, 0) == WAIT_TIMEOUT) {
        return;
    }

    KillTimer(hwnd, PLAYER_PLAYLIST_SWITCH_TIMER_ID);
    stop_playback(state);
    index = state->playlist_pending_play_index;
    state->playlist_pending_play_index = -1;
    if (index < 0 || (unsigned)index >= state->playlist_count) {
        state->playlist_play_in_progress = 0;
        return;
    }

    state->playlist_current = index;
    playlist_refresh_listbox(state);
    snprintf(path, sizeof(path), "%s", state->playlist_paths[index]);
    player_log("playlist play begin index=%d path=%s", index, path);
    snprintf(state->direct_start_retry_path,
        sizeof(state->direct_start_retry_path), "%s", path);
    state->direct_start_retry_remaining = 2;
    state->direct_start_retry_from_playlist = 1;
    state->direct_start_retry_playlist_index = index;
    start_playback(hwnd, state, path);
    state->direct_start_retry_deadline =
        GetTickCount() + PLAYER_START_RETRY_TIMEOUT_MS;
    state->direct_start_retry_armed = 1;
    SetTimer(hwnd, PLAYER_DIRECT_RETRY_TIMER_ID,
        PLAYER_DIRECT_RETRY_TIMER_MS, NULL);
    player_log("playlist play end index=%d pending=%d", index,
        state->playlist_pending_play_index);
    state->playlist_play_in_progress = 0;

    if (state->playlist_pending_play_index >= 0 &&
        (unsigned)state->playlist_pending_play_index < state->playlist_count) {
        PostMessageA(hwnd, PLAYER_WM_PLAYLIST_PLAY, 0, 0);
    }
}

static void playlist_play_next(HWND hwnd, PlayerState *state, int delta)
{
    int index;

    if (state == NULL || state->playlist_count == 0) {
        return;
    }
    index = state->playlist_current;
    if (index < 0) {
        index = 0;
    } else {
        index += delta;
    }
    if (index < 0) {
        index = (int)state->playlist_count - 1;
    }
    if ((unsigned)index >= state->playlist_count) {
        index = 0;
    }
    playlist_play_index(hwnd, state, index);
}

static int playlist_automatic_next_index(PlayerState *state)
{
    int current;
    unsigned count;

    if (state == NULL || state->playlist_count == 0) {
        return -1;
    }
    if (state->playlist_single_track_mode) {
        return -1;
    }
    count = state->playlist_count;
    current = state->playlist_current;
    if (!state->playlist_random_mode) {
        return current < 0 ? 0 : (current + 1) % (int)count;
    }

    if (state->playlist_random_state == 0u) {
        state->playlist_random_state =
            (uint32_t)GetTickCount() ^ (uint32_t)(uintptr_t)state;
    }
    state->playlist_random_state =
        state->playlist_random_state * 1664525u + 1013904223u;
    if (count == 1u || current < 0 || (unsigned)current >= count) {
        return (int)(state->playlist_random_state % count);
    }
    return (current + 1 +
        (int)(state->playlist_random_state % (count - 1u))) % (int)count;
}

static void timbre_refresh_listbox(PlayerState *state)
{
    unsigned i;
    LRESULT top_index;
    LRESULT caret_index;
    LRESULT caret_data = LB_ERR;

    if (state == NULL || state->timbre_listbox == NULL) {
        return;
    }

    lock_state(state);
    top_index = SendMessageA(state->timbre_listbox, LB_GETTOPINDEX, 0, 0);
    caret_index = SendMessageA(state->timbre_listbox, LB_GETCARETINDEX, 0, 0);
    if (caret_index != LB_ERR) {
        caret_data = SendMessageA(state->timbre_listbox, LB_GETITEMDATA, (WPARAM)caret_index, 0);
    }
    SendMessageA(state->timbre_listbox, WM_SETREDRAW, FALSE, 0);
    SendMessageA(state->timbre_listbox, LB_RESETCONTENT, 0, 0);
    for (i = 0; i < state->timbre_list_count; ++i) {
        char line[160];
        LRESULT row;
        int group_solo;
        unsigned selected_samples = 0;
        unsigned k;

        if (state->timbre_solo_enabled) {
            for (k = 0; k < state->timbre_list_key_count[i]; ++k) {
                if (timbre_solo_has_key_locked(state, state->timbre_list_keys[i][k])) {
                    selected_samples++;
                }
            }
        }
        group_solo = state->timbre_list_key_count[i] > 0 &&
            selected_samples == state->timbre_list_key_count[i];
        if (state->timbre_list_key_count[i] == 1) {
            snprintf(line,
                sizeof(line),
                "  Sample %02u  s:%05X l:%05X adsr:%04X/%04X fl:%02X",
                i,
                state->timbre_list_ssa[i],
                state->timbre_list_lsa[i],
                state->timbre_list_adsr1[i],
                state->timbre_list_adsr2[i],
                (unsigned)state->timbre_list_flags[i]);
        } else {
            snprintf(line,
                sizeof(line),
                "%c [%c] Samples:%02u  s:%05X l:%05X adsr:%04X/%04X fl:%02X",
                state->timbre_list_expanded[i] ? '-' : '+',
                selected_samples == 0 ? ' ' : (group_solo ? '*' : '~'),
                state->timbre_list_key_count[i],
                state->timbre_list_ssa[i],
                state->timbre_list_lsa[i],
                state->timbre_list_adsr1[i],
                state->timbre_list_adsr2[i],
                (unsigned)state->timbre_list_flags[i]);
        }
        row = SendMessageA(state->timbre_listbox, LB_ADDSTRING, 0, (LPARAM)line);
        SendMessageA(state->timbre_listbox,
            LB_SETITEMDATA,
            (WPARAM)row,
            (LPARAM)MAKELONG(TIMBRE_LIST_GROUP_ROW, i));
        SendMessageA(state->timbre_listbox, LB_SETSEL, group_solo ? TRUE : FALSE, (LPARAM)row);

        if (state->timbre_list_key_count[i] <= 1 || !state->timbre_list_expanded[i]) {
            continue;
        }
        for (k = 0; k < state->timbre_list_key_count[i]; ++k) {
            int sample_solo = state->timbre_solo_enabled &&
                timbre_solo_has_key_locked(state, state->timbre_list_keys[i][k]);

            snprintf(line,
                sizeof(line),
                "    Sample %02u  s:%05X l:%05X",
                k,
                state->timbre_list_ssa_keys[i][k],
                state->timbre_list_lsa_keys[i][k]);
            row = SendMessageA(state->timbre_listbox, LB_ADDSTRING, 0, (LPARAM)line);
            SendMessageA(state->timbre_listbox,
                LB_SETITEMDATA,
                (WPARAM)row,
                (LPARAM)MAKELONG(k, i));
            SendMessageA(state->timbre_listbox, LB_SETSEL, sample_solo ? TRUE : FALSE, (LPARAM)row);
        }
    }
    if (top_index >= 0) {
        SendMessageA(state->timbre_listbox, LB_SETTOPINDEX, (WPARAM)top_index, 0);
    }
    if (caret_data != LB_ERR) {
        LRESULT row_count = SendMessageA(state->timbre_listbox, LB_GETCOUNT, 0, 0);
        LRESULT row;

        for (row = 0; row < row_count; ++row) {
            if (SendMessageA(state->timbre_listbox, LB_GETITEMDATA, (WPARAM)row, 0) == caret_data) {
                SendMessageA(state->timbre_listbox, LB_SETCARETINDEX, (WPARAM)row, FALSE);
                break;
            }
        }
    }
    SendMessageA(state->timbre_listbox, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(state->timbre_listbox, NULL, TRUE);
    unlock_state(state);
}

static void reset_timbre_selection(HWND hwnd, PlayerState *state)
{
    if (state == NULL) {
        return;
    }
    lock_state(state);
    reset_timbre_solo_locked(state);
    state->voice_mute_mask[0] = 0u;
    state->voice_mute_mask[1] = 0u;
    unlock_state(state);
    apply_voice_mute_masks(state);
    timbre_refresh_listbox(state);
    if (state->hwnd != NULL) {
        InvalidateRect(state->hwnd, NULL, TRUE);
    }
    if (hwnd != NULL) {
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

static int preview_is_black_key(int midi_note)
{
    int note = midi_note % 12;
    return note == 1 || note == 3 || note == 6 || note == 8 || note == 10;
}

static void preview_keyboard_area(HWND hwnd, RECT *out_rect)
{
    RECT rect;

    GetClientRect(hwnd, &rect);
    out_rect->left = 8;
    out_rect->top = 112;
    out_rect->right = rect.right - 8;
    out_rect->bottom = rect.bottom - 8;
    if (out_rect->bottom < out_rect->top + 100) {
        out_rect->bottom = out_rect->top + 100;
    }
}

static int preview_white_key_count(void)
{
    int note;
    int count = 0;

    for (note = PREVIEW_FIRST_MIDI_NOTE; note <= PREVIEW_LAST_MIDI_NOTE; ++note) {
        if (!preview_is_black_key(note)) {
            count++;
        }
    }
    return count;
}

static int preview_hit_test_key(HWND hwnd, int x, int y)
{
    RECT keyboard;
    int white_count = preview_white_key_count();
    int black_width;
    int black_height;
    int white_index = 0;
    int note;

    preview_keyboard_area(hwnd, &keyboard);
    if (x < keyboard.left || x >= keyboard.right || y < keyboard.top || y >= keyboard.bottom) {
        return -1;
    }
    black_width = ((keyboard.right - keyboard.left) / white_count) * 3 / 5;
    if (black_width < 8) {
        black_width = 8;
    }
    black_height = (keyboard.bottom - keyboard.top) * 3 / 5;
    for (note = PREVIEW_FIRST_MIDI_NOTE; note <= PREVIEW_LAST_MIDI_NOTE; ++note) {
        if (preview_is_black_key(note)) {
            int boundary = keyboard.left +
                ((keyboard.right - keyboard.left) * white_index) / white_count;
            RECT key = {
                boundary - black_width / 2,
                keyboard.top,
                boundary + black_width / 2,
                keyboard.top + black_height
            };
            if (PtInRect(&key, (POINT){x, y})) {
                return note;
            }
        } else {
            white_index++;
        }
    }
    white_index = 0;
    for (note = PREVIEW_FIRST_MIDI_NOTE; note <= PREVIEW_LAST_MIDI_NOTE; ++note) {
        if (!preview_is_black_key(note)) {
            int left = keyboard.left +
                ((keyboard.right - keyboard.left) * white_index) / white_count;
            int right = keyboard.left +
                ((keyboard.right - keyboard.left) * (white_index + 1)) / white_count;
            if (x >= left && x < right) {
                return note;
            }
            white_index++;
        }
    }
    return -1;
}

static void preview_paint_keyboard(HWND hwnd, HDC hdc, PlayerState *state)
{
    RECT keyboard;
    int white_count = preview_white_key_count();
    int white_index = 0;
    int black_width;
    int black_height;
    int dark = is_dark_theme_active(state);
    int octave_shift = 0;
    uint8_t key_down[128];
    int note;
    HBRUSH white_brush = CreateSolidBrush(dark ? RGB(42, 42, 42) : RGB(248, 248, 248));
    HBRUSH black_brush = CreateSolidBrush(dark ? RGB(4, 4, 4) : RGB(0, 0, 0));
    HBRUSH active_brush = CreateSolidBrush(RGB(230, 64, 84));
    HPEN border_pen = CreatePen(PS_SOLID, 1, dark ? RGB(160, 160, 160) : RGB(28, 28, 28));
    HPEN active_pen = CreatePen(PS_SOLID, 2, RGB(230, 64, 84));
    HGDIOBJ old_pen;
    HGDIOBJ old_brush;

    ZeroMemory(key_down, sizeof(key_down));
    lock_state(state);
    if (state != NULL) {
        memcpy(key_down, state->preview_key_down, sizeof(key_down));
        octave_shift = state->preview_octave_shift;
    }
    unlock_state(state);
    preview_keyboard_area(hwnd, &keyboard);
    black_width = ((keyboard.right - keyboard.left) / white_count) * 3 / 5;
    if (black_width < 8) {
        black_width = 8;
    }
    black_height = (keyboard.bottom - keyboard.top) * 3 / 5;
    old_pen = SelectObject(hdc, border_pen);
    old_brush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, dark ? RGB(225, 225, 225) : RGB(35, 35, 35));
    for (note = PREVIEW_FIRST_MIDI_NOTE; note <= PREVIEW_LAST_MIDI_NOTE; ++note) {
        if (!preview_is_black_key(note)) {
            RECT key;
            char label[8];

            key.left = keyboard.left +
                ((keyboard.right - keyboard.left) * white_index) / white_count;
            key.right = keyboard.left +
                ((keyboard.right - keyboard.left) * (white_index + 1)) / white_count;
            key.top = keyboard.top;
            key.bottom = keyboard.bottom;
            {
                int played_note = note + octave_shift * 12;
                int active = played_note >= 0 && played_note < 128 && key_down[played_note];
                FillRect(hdc, &key, active ? active_brush : white_brush);
            }
            Rectangle(hdc, key.left, key.top, key.right, key.bottom);
            if ((note % 12) == 0) {
                snprintf(label, sizeof(label), "C%d", ((note + octave_shift * 12) / 12) - 1);
                DrawTextA(hdc, label, -1, &key, DT_CENTER | DT_BOTTOM | DT_SINGLELINE);
            }
            white_index++;
        }
    }
    white_index = 0;
    for (note = PREVIEW_FIRST_MIDI_NOTE; note <= PREVIEW_LAST_MIDI_NOTE; ++note) {
        if (preview_is_black_key(note)) {
            int boundary = keyboard.left +
                ((keyboard.right - keyboard.left) * white_index) / white_count;
            RECT key = {
                boundary - black_width / 2,
                keyboard.top,
                boundary + black_width / 2,
                keyboard.top + black_height
            };
            {
                int played_note = note + octave_shift * 12;
                int active = played_note >= 0 && played_note < 128 && key_down[played_note];
                FillRect(hdc, &key, active ? active_brush : black_brush);
                if (active) {
                    SelectObject(hdc, active_pen);
                    Rectangle(hdc, key.left, key.top, key.right, key.bottom);
                    SelectObject(hdc, border_pen);
                } else {
                    Rectangle(hdc, key.left, key.top, key.right, key.bottom);
                }
            }
        } else {
            white_index++;
        }
    }
    SelectObject(hdc, old_brush);
    SelectObject(hdc, old_pen);
    DeleteObject(active_pen);
    DeleteObject(border_pen);
    DeleteObject(active_brush);
    DeleteObject(black_brush);
    DeleteObject(white_brush);
}

static unsigned preview_parse_hex_control(HWND hwnd, int control_id, unsigned maximum, unsigned fallback)
{
    char text_value[16];
    char *end = NULL;
    unsigned long value;

    GetWindowTextA(GetDlgItem(hwnd, control_id), text_value, sizeof(text_value));
    value = strtoul(text_value, &end, 16);
    if (end == text_value) {
        return fallback;
    }
    if (value > maximum) {
        value = maximum;
    }
    return (unsigned)value;
}

static void preview_update_config_from_controls(HWND hwnd, PlayerState *state)
{
    uint16_t adsr1;
    uint16_t adsr2;
    unsigned ar;
    unsigned dr;
    unsigned sl;
    unsigned sr;
    unsigned rr;
    unsigned pitch;
    unsigned vol_l;
    unsigned vol_r;
    unsigned linked_volume;
    unsigned i;
    int reverb_was_enabled;

    if (state == NULL || state->preview_controls_updating) {
        return;
    }
    lock_state(state);
    reverb_was_enabled = (state->preview_flags & SPU2LOG_VOICE_REVERB) != 0;
    ar = preview_parse_hex_control(hwnd, IDC_PREVIEW_AR, 0x7fu, (state->preview_adsr1 >> 8) & 0x7fu);
    dr = preview_parse_hex_control(hwnd, IDC_PREVIEW_DR, 0x0fu, (state->preview_adsr1 >> 4) & 0x0fu);
    sl = preview_parse_hex_control(hwnd, IDC_PREVIEW_SL, 0x0fu, state->preview_adsr1 & 0x0fu);
    sr = preview_parse_hex_control(hwnd, IDC_PREVIEW_SR, 0x7fu, (state->preview_adsr2 >> 6) & 0x7fu);
    rr = preview_parse_hex_control(hwnd, IDC_PREVIEW_RR, 0x1fu, state->preview_adsr2 & 0x1fu);
    pitch = preview_parse_hex_control(hwnd, IDC_PREVIEW_PITCH, 0x3fffu, state->preview_pitch);
    vol_l = preview_parse_hex_control(hwnd, IDC_PREVIEW_VOL_L, 0x3fffu,
        state->preview_vol_l);
    vol_r = preview_parse_hex_control(hwnd, IDC_PREVIEW_VOL_R, 0x3fffu,
        state->preview_vol_r);
    adsr1 = (uint16_t)((ar << 8) | (dr << 4) | sl);
    adsr2 = (uint16_t)((sr << 6) | rr);
    if (Button_GetCheck(GetDlgItem(hwnd, IDC_PREVIEW_ATTACK_EXP)) == BST_CHECKED) {
        adsr1 |= 0x8000u;
    }
    if (Button_GetCheck(GetDlgItem(hwnd, IDC_PREVIEW_SUSTAIN_EXP)) == BST_CHECKED) {
        adsr2 |= 0x8000u;
    }
    if (Button_GetCheck(GetDlgItem(hwnd, IDC_PREVIEW_SUSTAIN_DEC)) == BST_CHECKED) {
        adsr2 |= 0x4000u;
    }
    if (Button_GetCheck(GetDlgItem(hwnd, IDC_PREVIEW_RELEASE_EXP)) == BST_CHECKED) {
        adsr2 |= 0x0020u;
    }
    state->preview_adsr1 = adsr1;
    state->preview_adsr2 = adsr2;
    state->preview_pitch = (uint16_t)pitch;
    state->preview_vol_l = (uint16_t)vol_l;
    state->preview_vol_r = (uint16_t)vol_r;
    linked_volume = (vol_l + vol_r + 1u) / 2u;
    state->preview_flags &= ~(SPU2LOG_VOICE_NOISE | SPU2LOG_VOICE_PMOD | SPU2LOG_VOICE_REVERB);
    if (Button_GetCheck(GetDlgItem(hwnd, IDC_PREVIEW_NOISE)) == BST_CHECKED) {
        state->preview_flags |= SPU2LOG_VOICE_NOISE;
    }
    if (Button_GetCheck(GetDlgItem(hwnd, IDC_PREVIEW_PMOD)) == BST_CHECKED) {
        state->preview_flags |= SPU2LOG_VOICE_PMOD;
    }
    if (Button_GetCheck(GetDlgItem(hwnd, IDC_PREVIEW_REVERB)) == BST_CHECKED) {
        state->preview_flags |= SPU2LOG_VOICE_REVERB;
    }
    if (reverb_was_enabled && (state->preview_flags & SPU2LOG_VOICE_REVERB) == 0) {
        ZeroMemory(state->preview_reverb_l, sizeof(state->preview_reverb_l));
        ZeroMemory(state->preview_reverb_r, sizeof(state->preview_reverb_r));
        state->preview_reverb_pos = 0;
    }
    for (i = 0; i < PREVIEW_MAX_VOICES; ++i) {
        if (state->preview_voices[i].active) {
            uint16_t note_pitch = preview_sample_pitch_for_note_locked(
                state,
                state->preview_voices[i].sample_index,
                state->preview_voices[i].midi_note);
            state->preview_voices[i].phase_step = ((uint64_t)note_pitch << 32) / 0x1000u;
        }
    }
    unlock_state(state);
    SendDlgItemMessageA(hwnd, IDC_PREVIEW_VOL_SLIDER,
        TBM_SETPOS, TRUE, (LPARAM)linked_volume);
}

static void preview_set_hex_control(HWND hwnd, int control_id, unsigned value, int digits)
{
    char text_value[16];
    snprintf(text_value, sizeof(text_value), digits == 4 ? "%04X" : "%02X", value);
    SetWindowTextA(GetDlgItem(hwnd, control_id), text_value);
}

static void preview_sync_controls(HWND hwnd, PlayerState *state)
{
    uint16_t adsr1;
    uint16_t adsr2;
    uint16_t pitch;
    uint16_t vol_l;
    uint16_t vol_r;
    uint32_t flags;

    if (state == NULL || hwnd == NULL) {
        return;
    }
    lock_state(state);
    state->preview_controls_updating = 1;
    adsr1 = state->preview_adsr1;
    adsr2 = state->preview_adsr2;
    pitch = state->preview_pitch;
    vol_l = state->preview_vol_l;
    vol_r = state->preview_vol_r;
    flags = state->preview_flags;
    unlock_state(state);
    preview_set_hex_control(hwnd, IDC_PREVIEW_AR, (adsr1 >> 8) & 0x7fu, 2);
    preview_set_hex_control(hwnd, IDC_PREVIEW_DR, (adsr1 >> 4) & 0x0fu, 2);
    preview_set_hex_control(hwnd, IDC_PREVIEW_SL, adsr1 & 0x0fu, 2);
    preview_set_hex_control(hwnd, IDC_PREVIEW_SR, (adsr2 >> 6) & 0x7fu, 2);
    preview_set_hex_control(hwnd, IDC_PREVIEW_RR, adsr2 & 0x1fu, 2);
    preview_set_hex_control(hwnd, IDC_PREVIEW_PITCH, pitch, 4);
    preview_set_hex_control(hwnd, IDC_PREVIEW_VOL_L, vol_l, 4);
    preview_set_hex_control(hwnd, IDC_PREVIEW_VOL_R, vol_r, 4);
    SendDlgItemMessageA(hwnd, IDC_PREVIEW_VOL_SLIDER,
        TBM_SETPOS, TRUE, (LPARAM)(((unsigned)vol_l + vol_r + 1u) / 2u));
    Button_SetCheck(GetDlgItem(hwnd, IDC_PREVIEW_ATTACK_EXP), (adsr1 & 0x8000u) ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(GetDlgItem(hwnd, IDC_PREVIEW_SUSTAIN_EXP), (adsr2 & 0x8000u) ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(GetDlgItem(hwnd, IDC_PREVIEW_SUSTAIN_DEC), (adsr2 & 0x4000u) ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(GetDlgItem(hwnd, IDC_PREVIEW_RELEASE_EXP), (adsr2 & 0x0020u) ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(GetDlgItem(hwnd, IDC_PREVIEW_NOISE), (flags & SPU2LOG_VOICE_NOISE) ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(GetDlgItem(hwnd, IDC_PREVIEW_PMOD), (flags & SPU2LOG_VOICE_PMOD) ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(GetDlgItem(hwnd, IDC_PREVIEW_REVERB), (flags & SPU2LOG_VOICE_REVERB) ? BST_CHECKED : BST_UNCHECKED);
    lock_state(state);
    state->preview_controls_updating = 0;
    unlock_state(state);
}

static void preview_update_status(HWND hwnd, PlayerState *state)
{
    char status[112];
    unsigned selected_count;
    int octave_shift;

    if (hwnd == NULL || state == NULL) {
        return;
    }
    lock_state(state);
    selected_count = state->preview_selected_sample_count;
    octave_shift = state->preview_octave_shift;
    unlock_state(state);
    if (selected_count == 0) {
        snprintf(status, sizeof(status), "Sample data unavailable  |  Octave: %+d", octave_shift);
    } else {
        snprintf(status, sizeof(status), "%u sample%s  |  61 keys  C1-C6  |  Octave: %+d",
            selected_count,
            selected_count == 1 ? "" : "s",
            octave_shift);
    }
    SetWindowTextA(GetDlgItem(hwnd, IDC_PREVIEW_STATUS), status);
}

static HWND preview_create_label(HWND hwnd, const char *text_value, int x, int y, int width)
{
    return CreateWindowA("STATIC", text_value, WS_CHILD | WS_VISIBLE,
        x, y, width, 18, hwnd, NULL, NULL, NULL);
}

static HWND preview_create_edit(HWND hwnd, int id, int x, int y, int width, int digits)
{
    HWND edit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_UPPERCASE,
        x, y, width, 22, hwnd, (HMENU)(UINT_PTR)id, NULL, NULL);
    SendMessageA(edit, EM_SETLIMITTEXT, (WPARAM)digits, 0);
    return edit;
}

static void preview_set_linked_volume_from_slider(HWND hwnd, PlayerState *state)
{
    unsigned value;

    if (hwnd == NULL || state == NULL) {
        return;
    }
    value = (unsigned)SendDlgItemMessageA(
        hwnd, IDC_PREVIEW_VOL_SLIDER, TBM_GETPOS, 0, 0);
    if (value > 0x3fffu) {
        value = 0x3fffu;
    }
    lock_state(state);
    state->preview_controls_updating = 1;
    state->preview_vol_l = (uint16_t)value;
    state->preview_vol_r = (uint16_t)value;
    unlock_state(state);
    preview_set_hex_control(hwnd, IDC_PREVIEW_VOL_L, value, 4);
    preview_set_hex_control(hwnd, IDC_PREVIEW_VOL_R, value, 4);
    lock_state(state);
    state->preview_controls_updating = 0;
    unlock_state(state);
}

static int preview_pc_key_to_note(WPARAM key)
{
    switch (key) {
    case 'Z': return 48;
    case 'S': return 49;
    case 'X': return 50;
    case 'D': return 51;
    case 'C': return 52;
    case 'V': return 53;
    case 'G': return 54;
    case 'B': return 55;
    case 'H': return 56;
    case 'N': return 57;
    case 'J': return 58;
    case 'M': return 59;
    case VK_OEM_COMMA: return 60;
    case 'L': return 61;
    case VK_OEM_PERIOD: return 62;
    case VK_OEM_1: return 63;
    case VK_OEM_2: return 64;
    case 'Q': return 60;
    case '2': return 61;
    case 'W': return 62;
    case '3': return 63;
    case 'E': return 64;
    case 'R': return 65;
    case '5': return 66;
    case 'T': return 67;
    case '6': return 68;
    case 'Y': return 69;
    case '7': return 70;
    case 'U': return 71;
    case 'I': return 72;
    case '9': return 73;
    case 'O': return 74;
    case '0': return 75;
    case 'P': return 76;
    default: return -1;
    }
}

static void preview_change_octave(HWND hwnd, PlayerState *state, int delta)
{
    int next_octave;

    if (hwnd == NULL || state == NULL || delta == 0) {
        return;
    }
    lock_state(state);
    next_octave = state->preview_octave_shift + delta;
    if (next_octave < -3) {
        next_octave = -3;
    } else if (next_octave > 3) {
        next_octave = 3;
    }
    if (next_octave != state->preview_octave_shift) {
        preview_handoff_all_locked(state);
        state->preview_octave_shift = next_octave;
    }
    unlock_state(state);
    preview_update_status(hwnd, state);
    InvalidateRect(hwnd, NULL, FALSE);
}

static LRESULT CALLBACK preview_keyboard_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    PlayerState *state = (PlayerState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCTA *create = (CREATESTRUCTA *)lparam;
        const int label_y = 10;
        const int edit_y = 27;

        state = (PlayerState *)create->lpCreateParams;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)state);
        if (state != NULL) {
            state->preview_hwnd = hwnd;
        }
        preview_create_label(hwnd, "AR", 10, label_y, 38);
        preview_create_edit(hwnd, IDC_PREVIEW_AR, 10, edit_y, 42, 2);
        preview_create_label(hwnd, "DR", 58, label_y, 38);
        preview_create_edit(hwnd, IDC_PREVIEW_DR, 58, edit_y, 42, 2);
        preview_create_label(hwnd, "SL", 106, label_y, 38);
        preview_create_edit(hwnd, IDC_PREVIEW_SL, 106, edit_y, 42, 2);
        preview_create_label(hwnd, "SR", 154, label_y, 38);
        preview_create_edit(hwnd, IDC_PREVIEW_SR, 154, edit_y, 42, 2);
        preview_create_label(hwnd, "RR", 202, label_y, 38);
        preview_create_edit(hwnd, IDC_PREVIEW_RR, 202, edit_y, 42, 2);
        preview_create_label(hwnd, "Pitch", 260, label_y, 62);
        preview_create_edit(hwnd, IDC_PREVIEW_PITCH, 260, edit_y, 64, 4);
        preview_create_label(hwnd, "Vol L", 332, label_y, 62);
        preview_create_edit(hwnd, IDC_PREVIEW_VOL_L, 332, edit_y, 64, 4);
        preview_create_label(hwnd, "Vol R", 404, label_y, 62);
        preview_create_edit(hwnd, IDC_PREVIEW_VOL_R, 404, edit_y, 64, 4);
        preview_create_label(hwnd, "Vol L/R", 476, label_y, 72);
        {
            HWND volume_slider = CreateWindowA(
                TRACKBAR_CLASSA,
                "",
                WS_CHILD | WS_VISIBLE | TBS_NOTICKS,
                476,
                edit_y,
                260,
                24,
                hwnd,
                (HMENU)(UINT_PTR)IDC_PREVIEW_VOL_SLIDER,
                NULL,
                NULL);
            if (volume_slider != NULL) {
                SendMessageA(volume_slider, TBM_SETRANGEMIN, FALSE, 0);
                SendMessageA(volume_slider, TBM_SETRANGEMAX, TRUE, 0x3fffu);
                SendMessageA(volume_slider, TBM_SETLINESIZE, 0, 0x10u);
                SendMessageA(volume_slider, TBM_SETPAGESIZE, 0, 0x100u);
            }
        }
        CreateWindowA("BUTTON", "Exp Attack", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, 57, 92, 22, hwnd, (HMENU)(UINT_PTR)IDC_PREVIEW_ATTACK_EXP, NULL, NULL);
        CreateWindowA("BUTTON", "Exp Sustain", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            108, 57, 100, 22, hwnd, (HMENU)(UINT_PTR)IDC_PREVIEW_SUSTAIN_EXP, NULL, NULL);
        CreateWindowA("BUTTON", "Sustain Dec", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            214, 57, 100, 22, hwnd, (HMENU)(UINT_PTR)IDC_PREVIEW_SUSTAIN_DEC, NULL, NULL);
        CreateWindowA("BUTTON", "Exp Release", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            320, 57, 96, 22, hwnd, (HMENU)(UINT_PTR)IDC_PREVIEW_RELEASE_EXP, NULL, NULL);
        CreateWindowA("BUTTON", "Noise", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            430, 57, 70, 22, hwnd, (HMENU)(UINT_PTR)IDC_PREVIEW_NOISE, NULL, NULL);
        CreateWindowA("BUTTON", "FM", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            504, 57, 54, 22, hwnd, (HMENU)(UINT_PTR)IDC_PREVIEW_PMOD, NULL, NULL);
        CreateWindowA("BUTTON", "Reverb", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            562, 57, 78, 22, hwnd, (HMENU)(UINT_PTR)IDC_PREVIEW_REVERB, NULL, NULL);
        CreateWindowA("STATIC", "61 keys  C1-C6",
            WS_CHILD | WS_VISIBLE, 10, 86, 700, 18, hwnd, (HMENU)(UINT_PTR)IDC_PREVIEW_STATUS, NULL, NULL);
        if (state != NULL) {
            apply_window_theme(hwnd, state);
            apply_ui_font_to_window(hwnd, state->ui_font);
            preview_sync_controls(hwnd, state);
            preview_update_status(hwnd, state);
        }
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int note = preview_hit_test_key(hwnd, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
        if (state != NULL && note >= 0) {
            SetFocus(hwnd);
            SetCapture(hwnd);
            lock_state(state);
            note += state->preview_octave_shift * 12;
            if (state->preview_active_note >= 0 && state->preview_active_note != note) {
                preview_note_off_locked(state, state->preview_active_note);
            }
            preview_note_on_locked(state, note);
            unlock_state(state);
            preview_start_standalone_audio(state);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        break;
    }
    case WM_MOUSEMOVE:
        if (state != NULL && GetCapture() == hwnd && (wparam & MK_LBUTTON) != 0) {
            int note = preview_hit_test_key(hwnd, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            lock_state(state);
            if (note >= 0) {
                note += state->preview_octave_shift * 12;
            }
            if (note >= 0 && note != state->preview_active_note) {
                if (state->preview_active_note >= 0) {
                    preview_note_off_locked(state, state->preview_active_note);
                }
                preview_note_on_locked(state, note);
            }
            unlock_state(state);
            preview_start_standalone_audio(state);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        break;
    case WM_LBUTTONUP:
        if (state != NULL && GetCapture() == hwnd) {
            ReleaseCapture();
            lock_state(state);
            if (state->preview_active_note >= 0) {
                preview_note_off_locked(state, state->preview_active_note);
            }
            unlock_state(state);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        break;
    case WM_CAPTURECHANGED:
        if (state != NULL) {
            lock_state(state);
            if (state->preview_active_note >= 0) {
                preview_note_off_locked(state, state->preview_active_note);
            }
            unlock_state(state);
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
    case WM_KEYDOWN: {
        int note = preview_pc_key_to_note(wparam);

        if (state != NULL && (wparam == VK_LEFT || wparam == VK_UP ||
            wparam == VK_RIGHT || wparam == VK_DOWN)) {
            if (state->timbre_listbox != NULL) {
                SendMessageA(state->timbre_listbox,
                    WM_KEYDOWN,
                    wparam,
                    lparam);
            }
            return 0;
        }
        if (state != NULL && (wparam == VK_PRIOR || wparam == VK_OEM_6)) {
            preview_change_octave(hwnd, state, 1);
            return 0;
        }
        if (state != NULL && (wparam == VK_NEXT || wparam == VK_OEM_4)) {
            preview_change_octave(hwnd, state, -1);
            return 0;
        }
        if (state != NULL && note >= 0) {
            if ((lparam & (1l << 30)) == 0) {
                note += state->preview_octave_shift * 12;
                lock_state(state);
                preview_note_on_locked(state, note);
                unlock_state(state);
                preview_start_standalone_audio(state);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        break;
    }
    case WM_KEYUP: {
        int note = preview_pc_key_to_note(wparam);
        if (state != NULL && note >= 0) {
            note += state->preview_octave_shift * 12;
            lock_state(state);
            preview_note_off_locked(state, note);
            unlock_state(state);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        break;
    }
    case WM_COMMAND:
        if (state != NULL && ((LOWORD(wparam) >= IDC_PREVIEW_AR && LOWORD(wparam) <= IDC_PREVIEW_VOL_R &&
                HIWORD(wparam) == EN_CHANGE) ||
            (LOWORD(wparam) >= IDC_PREVIEW_ATTACK_EXP && LOWORD(wparam) <= IDC_PREVIEW_REVERB &&
                HIWORD(wparam) == BN_CLICKED))) {
            preview_update_config_from_controls(hwnd, state);
            return 0;
        }
        break;
    case WM_HSCROLL:
        if (state != NULL && (HWND)lparam ==
            GetDlgItem(hwnd, IDC_PREVIEW_VOL_SLIDER)) {
            preview_set_linked_volume_from_slider(hwnd, state);
            return 0;
        }
        break;
    case WM_PAINT: {
        PAINTSTRUCT paint;
        RECT rect;
        HDC hdc = BeginPaint(hwnd, &paint);
        HDC memory_dc = CreateCompatibleDC(hdc);
        HBITMAP bitmap;
        HGDIOBJ old_bitmap;

        GetClientRect(hwnd, &rect);
        bitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
        old_bitmap = SelectObject(memory_dc, bitmap);
        fill_player_background(memory_dc, &rect, state);
        preview_paint_keyboard(hwnd, memory_dc, state);
        BitBlt(hdc, 0, 0, rect.right, rect.bottom, memory_dc, 0, 0, SRCCOPY);
        SelectObject(memory_dc, old_bitmap);
        DeleteObject(bitmap);
        DeleteDC(memory_dc);
        EndPaint(hwnd, &paint);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_GETMINMAXINFO: {
        MINMAXINFO *minmax = (MINMAXINFO *)lparam;
        minmax->ptMinTrackSize.x = 760;
        minmax->ptMinTrackSize.y = 235;
        return 0;
    }
    case WM_DRAWITEM:
        if (draw_owner_button(lparam, state)) {
            return TRUE;
        }
        break;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORBTN: {
        LRESULT result = themed_dialog_control_color(wparam, state);
        if (result != 0) {
            return result;
        }
        break;
    }
    case WM_CLOSE:
        save_preview_window_position(hwnd);
        if (state != NULL) {
            lock_state(state);
            preview_stop_all_locked(state);
            unlock_state(state);
            preview_stop_standalone_audio(state);
        }
        ShowWindow(hwnd, SW_HIDE);
        return 0;
    case WM_DESTROY:
        save_preview_window_position(hwnd);
        if (state != NULL) {
            lock_state(state);
            preview_stop_all_locked(state);
            unlock_state(state);
            preview_stop_standalone_audio(state);
            state->preview_hwnd = NULL;
        }
        return 0;
    default:
        break;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static void show_preview_keyboard(PlayerState *state, unsigned group, int sample)
{
    unsigned first;
    unsigned limit;
    unsigned k;
    int x;
    int y;
    char title[128];

    if (state == NULL) {
        return;
    }
    lock_state(state);
    if (group >= state->timbre_list_count || state->timbre_list_key_count[group] == 0) {
        unlock_state(state);
        return;
    }
    preview_handoff_all_locked(state);
    ZeroMemory(state->preview_reverb_l, sizeof(state->preview_reverb_l));
    ZeroMemory(state->preview_reverb_r, sizeof(state->preview_reverb_r));
    state->preview_reverb_pos = 0;
    state->preview_selected_sample_count = 0;
    if (sample >= 0 && (unsigned)sample < state->timbre_list_key_count[group]) {
        first = (unsigned)sample;
        limit = first + 1u;
    } else {
        first = 0;
        limit = state->timbre_list_key_count[group];
    }
    for (k = first; k < limit; ++k) {
        int cache_index = preview_find_sample_locked(state, state->timbre_list_keys[group][k]);
        if (cache_index >= 0 && state->preview_selected_sample_count < TIMBRE_SOLO_MAX_KEYS) {
            preview_prepare_sample_tuning_locked(&state->preview_samples[cache_index]);
            state->preview_selected_samples[state->preview_selected_sample_count++] = (unsigned)cache_index;
        }
    }
    state->preview_adsr1 = state->timbre_list_adsr1[group];
    state->preview_adsr2 = state->timbre_list_adsr2[group];
    state->preview_pitch = 0x1000u;
    state->preview_reference_pitch = 0x1000u;
    state->preview_vol_l = PREVIEW_DEFAULT_VOLUME;
    state->preview_vol_r = PREVIEW_DEFAULT_VOLUME;
    state->preview_reverb_value_l = 0;
    state->preview_reverb_value_r = 0;
    state->preview_flags = 0;
    state->preview_noise_clock = 0;
    if (state->preview_selected_sample_count > 0) {
        const PreviewSample *selected = &state->preview_samples[state->preview_selected_samples[0]];
        state->preview_flags = selected->flags;
        state->preview_noise_clock = selected->noise_clock;
        state->preview_reverb_value_l = state->live.cores[selected->core].reverb_l;
        state->preview_reverb_value_r = state->live.cores[selected->core].reverb_r;
        state->preview_reference_pitch = selected->c4_pitch != 0
            ? selected->c4_pitch
            : 0x1000u;
        state->preview_pitch = state->preview_reference_pitch;
    }
    unlock_state(state);

    if (state->preview_hwnd == NULL) {
        load_preview_window_position(&x, &y);
        state->preview_hwnd = CreateWindowExA(
            WS_EX_TOOLWINDOW,
            "PsfSpuPreviewKeyboardWindow",
            "Sample Keyboard",
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
            x,
            y,
            760,
            260,
            state->timbre_hwnd,
            NULL,
            GetModuleHandleA(NULL),
            state);
    }
    if (state->preview_hwnd == NULL) {
        return;
    }
    if (sample >= 0) {
        snprintf(title, sizeof(title), "Sample Keyboard - Sample %02d", sample);
    } else {
        snprintf(title, sizeof(title), "Sample Keyboard - Samples %02u", group);
    }
    SetWindowTextA(state->preview_hwnd, title);
    preview_update_status(state->preview_hwnd, state);
    preview_sync_controls(state->preview_hwnd, state);
    ShowWindow(state->preview_hwnd, SW_SHOWNOACTIVATE);
    InvalidateRect(state->preview_hwnd, NULL, TRUE);
}

static int timbre_find_group_row(HWND listbox, unsigned group)
{
    int row;
    int count;

    if (listbox == NULL) {
        return -1;
    }
    count = (int)SendMessageA(listbox, LB_GETCOUNT, 0, 0);
    for (row = 0; row < count; ++row) {
        LRESULT item_data = SendMessageA(listbox, LB_GETITEMDATA, (WPARAM)row, 0);
        if (item_data != LB_ERR &&
            LOWORD((DWORD_PTR)item_data) == TIMBRE_LIST_GROUP_ROW &&
            HIWORD((DWORD_PTR)item_data) == group) {
            return row;
        }
    }
    return -1;
}

static void timbre_set_visible_group_selection(HWND listbox, unsigned group, int selected)
{
    int row;
    int count;

    if (listbox == NULL) {
        return;
    }
    count = (int)SendMessageA(listbox, LB_GETCOUNT, 0, 0);
    for (row = 0; row < count; ++row) {
        LRESULT item_data = SendMessageA(listbox, LB_GETITEMDATA, (WPARAM)row, 0);

        if (item_data != LB_ERR && HIWORD((DWORD_PTR)item_data) == group) {
            SendMessageA(listbox, LB_SETSEL, selected ? TRUE : FALSE, (LPARAM)row);
        }
    }
}

static int timbre_set_caret_group_expanded(PlayerState *state, int expanded)
{
    HWND listbox;
    int row;
    int group_row;
    int changed = 0;
    LRESULT item_data;
    unsigned group;

    if (state == NULL || state->timbre_listbox == NULL) {
        return 0;
    }
    listbox = state->timbre_listbox;
    row = (int)SendMessageA(listbox, LB_GETCARETINDEX, 0, 0);
    if (row < 0) {
        return 0;
    }
    item_data = SendMessageA(listbox, LB_GETITEMDATA, (WPARAM)row, 0);
    if (item_data == LB_ERR) {
        return 0;
    }
    group = HIWORD((DWORD_PTR)item_data);
    lock_state(state);
    if (group >= state->timbre_list_count || state->timbre_list_key_count[group] <= 1) {
        unlock_state(state);
        return 1;
    }
    expanded = expanded ? 1 : 0;
    if (state->timbre_list_expanded[group] != expanded) {
        state->timbre_list_expanded[group] = (uint8_t)expanded;
        changed = 1;
    }
    unlock_state(state);
    if (changed) {
        timbre_refresh_listbox(state);
    }
    group_row = timbre_find_group_row(listbox, group);
    if (group_row >= 0) {
        SendMessageA(listbox, LB_SETCARETINDEX, (WPARAM)group_row, FALSE);
    }
    return 1;
}

static LRESULT CALLBACK timbre_listbox_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    PlayerState *state = (PlayerState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    if (msg == WM_KEYDOWN && state != NULL &&
        (wparam == VK_LEFT || wparam == VK_RIGHT)) {
        if (timbre_set_caret_group_expanded(state, wparam == VK_RIGHT)) {
            return 0;
        }
    }

    if (msg == WM_LBUTTONDBLCLK && state != NULL) {
        POINT point;
        LRESULT hit;

        point.x = GET_X_LPARAM(lparam);
        point.y = GET_Y_LPARAM(lparam);
        hit = SendMessageA(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(point.x, point.y));
        if (HIWORD(hit) == 0) {
            int row = (int)LOWORD(hit);
            LRESULT item_data = SendMessageA(hwnd, LB_GETITEMDATA, (WPARAM)row, 0);
            unsigned group = HIWORD((DWORD_PTR)item_data);
            unsigned sample = LOWORD((DWORD_PTR)item_data);

            SetFocus(hwnd);
            SendMessageA(hwnd, LB_SETCARETINDEX, (WPARAM)row, FALSE);
            show_preview_keyboard(state, group,
                sample == TIMBRE_LIST_GROUP_ROW ? -1 : (int)sample);
            return 0;
        }
    }
    if (msg == WM_LBUTTONDOWN && state != NULL) {
        POINT point;
        LRESULT hit;

        point.x = GET_X_LPARAM(lparam);
        point.y = GET_Y_LPARAM(lparam);
        hit = SendMessageA(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(point.x, point.y));
        if (HIWORD(hit) == 0) {
            int row = (int)LOWORD(hit);
            LRESULT item_data = SendMessageA(hwnd, LB_GETITEMDATA, (WPARAM)row, 0);
            unsigned group = HIWORD((DWORD_PTR)item_data);
            unsigned sample = LOWORD((DWORD_PTR)item_data);
            SetFocus(hwnd);
            if (sample == TIMBRE_LIST_GROUP_ROW && point.x < 20) {
                int expandable = 0;

                lock_state(state);
                if (group < state->timbre_list_count &&
                    state->timbre_list_key_count[group] > 1) {
                    expandable = 1;
                    state->timbre_list_expanded[group] =
                        state->timbre_list_expanded[group] ? 0 : 1;
                }
                unlock_state(state);
                if (expandable) {
                    timbre_refresh_listbox(state);
                    return 0;
                }
            }
            {
                int additive = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                int selected = SendMessageA(hwnd, LB_GETSEL, (WPARAM)row, 0) > 0;
                int item_count = (int)SendMessageA(hwnd, LB_GETCOUNT, 0, 0);
                int i;

                if (item_count < 0) {
                    item_count = 0;
                }
                if ((unsigned)item_count > TIMBRE_LIST_MAX_ROWS) {
                    item_count = (int)TIMBRE_LIST_MAX_ROWS;
                }
                ZeroMemory(state->timbre_drag_base_selection,
                    sizeof(state->timbre_drag_base_selection));
                if (additive) {
                    for (i = 0; i < item_count; ++i) {
                        state->timbre_drag_base_selection[i] =
                            SendMessageA(hwnd, LB_GETSEL, (WPARAM)i, 0) > 0 ? 1u : 0u;
                    }
                }

                state->timbre_drag_active = 1;
                state->timbre_drag_anchor_row = row;
                state->timbre_drag_last_caret = row;
                state->timbre_drag_row_count = item_count;
                state->timbre_drag_select_value = additive && selected ? 0 : 1;
                if (!additive) {
                    SendMessageA(hwnd, LB_SETSEL, FALSE, (LPARAM)-1);
                }
                SendMessageA(hwnd,
                    LB_SETSEL,
                    state->timbre_drag_select_value ? TRUE : FALSE,
                    (LPARAM)row);
                if (sample == TIMBRE_LIST_GROUP_ROW) {
                    timbre_set_visible_group_selection(
                        hwnd,
                        group,
                        state->timbre_drag_select_value);
                }
                SendMessageA(hwnd, LB_SETCARETINDEX, (WPARAM)row, FALSE);
                SetCapture(hwnd);
                sync_timbre_solo_from_listbox(
                    state->hwnd != NULL ? state->hwnd : hwnd,
                    state);
                if (state->preview_hwnd != NULL && IsWindowVisible(state->preview_hwnd)) {
                    show_preview_keyboard(state,
                        group,
                        sample == TIMBRE_LIST_GROUP_ROW ? -1 : (int)sample);
                }
                return 0;
            }
        }
    }
    if (msg == WM_MOUSEMOVE && state != NULL && state->timbre_drag_active &&
        (wparam & MK_LBUTTON) != 0) {
        POINT point;
        LRESULT hit;
        int row;

        point.x = GET_X_LPARAM(lparam);
        point.y = GET_Y_LPARAM(lparam);
        hit = SendMessageA(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(point.x, point.y));
        row = (int)LOWORD(hit);
        if (row >= state->timbre_drag_row_count) {
            row = state->timbre_drag_row_count - 1;
        }
        if (row >= 0 && row != state->timbre_drag_last_caret) {
            int first = state->timbre_drag_last_caret;
            int last = row;
            int range_first = state->timbre_drag_anchor_row;
            int range_last = row;
            int i;

            if (first < 0) {
                first = row;
            }
            if (first > last) {
                int swap = first;
                first = last;
                last = swap;
            }
            if (range_first > range_last) {
                int swap = range_first;
                range_first = range_last;
                range_last = swap;
            }
            for (i = first; i <= last; ++i) {
                int selected = state->timbre_drag_base_selection[i] != 0;

                if (i >= range_first && i <= range_last) {
                    selected = state->timbre_drag_select_value;
                }
                SendMessageA(hwnd,
                    LB_SETSEL,
                    selected ? TRUE : FALSE,
                    (LPARAM)i);
            }
            state->timbre_drag_last_caret = row;
            SendMessageA(hwnd, LB_SETCARETINDEX, (WPARAM)row, FALSE);
            sync_timbre_solo_from_listbox(
                state->hwnd != NULL ? state->hwnd : hwnd,
                state);
            if (state->preview_hwnd != NULL && IsWindowVisible(state->preview_hwnd)) {
                LRESULT item_data = SendMessageA(hwnd, LB_GETITEMDATA, (WPARAM)row, 0);

                if (item_data != LB_ERR) {
                    unsigned group = HIWORD((DWORD_PTR)item_data);
                    unsigned sample = LOWORD((DWORD_PTR)item_data);

                    show_preview_keyboard(state,
                        group,
                        sample == TIMBRE_LIST_GROUP_ROW ? -1 : (int)sample);
                }
            }
        }
        return 0;
    }
    if (msg == WM_LBUTTONUP && state != NULL && state->timbre_drag_active) {
        sync_timbre_solo_from_listbox(
            state->hwnd != NULL ? state->hwnd : hwnd,
            state);
        state->timbre_drag_active = 0;
        state->timbre_drag_anchor_row = -1;
        state->timbre_drag_last_caret = -1;
        state->timbre_drag_row_count = 0;
        if (GetCapture() == hwnd) {
            ReleaseCapture();
        }
        refresh_timbre_listbox_after_drag(state);
        return 0;
    }
    if (msg == WM_CAPTURECHANGED && state != NULL) {
        state->timbre_drag_active = 0;
        state->timbre_drag_anchor_row = -1;
        state->timbre_drag_last_caret = -1;
        state->timbre_drag_row_count = 0;
    }
    if (msg == WM_RBUTTONDOWN && state != NULL) {
        POINT point;
        LRESULT hit;
        int row = -1;
        LRESULT count;

        point.x = GET_X_LPARAM(lparam);
        point.y = GET_Y_LPARAM(lparam);
        count = SendMessageA(hwnd, LB_GETCOUNT, 0, 0);
        hit = SendMessageA(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(point.x, point.y));
        if (count > 0 && HIWORD(hit) == 0 && LOWORD(hit) < (unsigned)count) {
            RECT item_rect;

            row = (int)LOWORD(hit);
            if (SendMessageA(hwnd, LB_GETITEMRECT, (WPARAM)row, (LPARAM)&item_rect) == LB_ERR ||
                !PtInRect(&item_rect, point)) {
                row = -1;
            }
        }
        if (row < 0) {
            reset_timbre_selection(state->timbre_hwnd, state);
            return 0;
        }
    }
    if (state != NULL && state->timbre_listbox_proc != NULL) {
        return CallWindowProcA(state->timbre_listbox_proc, hwnd, msg, wparam, lparam);
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static void apply_timbre_topmost(HWND hwnd, int topmost)
{
    if (hwnd == NULL) {
        return;
    }
    SetWindowPos(hwnd,
        topmost ? HWND_TOPMOST : HWND_NOTOPMOST,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

static void layout_timbre_window(HWND hwnd, PlayerState *state)
{
    RECT rect;
    int width;
    int height;

    if (state == NULL) {
        return;
    }
    GetClientRect(hwnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    if (state->timbre_listbox != NULL) {
        SetWindowPos(state->timbre_listbox, NULL, 6, 6, width - 12, height - 42, SWP_NOZORDER);
    }
    SetWindowPos(GetDlgItem(hwnd, IDC_TIMBRE_CLEAR), NULL, 6, height - 30, 90, 24, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, IDC_TIMBRE_TOPMOST), NULL, 104, height - 30, 130, 24, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, IDC_TIMBRE_HIDE), NULL, width - 76, height - 30, 70, 24, SWP_NOZORDER);
}

static LRESULT CALLBACK timbre_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    PlayerState *state = (PlayerState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCTA *create = (CREATESTRUCTA *)lparam;

        state = (PlayerState *)create->lpCreateParams;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)state);
        if (state != NULL) {
            state->timbre_hwnd = hwnd;
            state->timbre_listbox = CreateWindowA("LISTBOX", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOTIFY | LBS_EXTENDEDSEL | LBS_NOINTEGRALHEIGHT,
                6, 6, 520, 300, hwnd, (HMENU)(UINT_PTR)IDC_TIMBRE_LIST, NULL, NULL);
            if (state->timbre_listbox != NULL) {
                SetWindowLongPtrA(state->timbre_listbox, GWLP_USERDATA, (LONG_PTR)state);
                state->timbre_listbox_proc = (WNDPROC)SetWindowLongPtrA(
                    state->timbre_listbox,
                    GWLP_WNDPROC,
                    (LONG_PTR)timbre_listbox_wnd_proc);
            }
        }
        CreateWindowA("BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            6, 312, 90, 24, hwnd, (HMENU)(UINT_PTR)IDC_TIMBRE_CLEAR, NULL, NULL);
        CreateWindowA("BUTTON", "Always on top", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            104, 312, 130, 24, hwnd, (HMENU)(UINT_PTR)IDC_TIMBRE_TOPMOST, NULL, NULL);
        CreateWindowA("BUTTON", "Hide", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            478, 312, 70, 24, hwnd, (HMENU)(UINT_PTR)IDC_TIMBRE_HIDE, NULL, NULL);
        if (state != NULL) {
            Button_SetCheck(GetDlgItem(hwnd, IDC_TIMBRE_TOPMOST),
                state->timbre_topmost ? BST_CHECKED : BST_UNCHECKED);
            apply_timbre_topmost(hwnd, state->timbre_topmost);
            apply_window_theme(hwnd, state);
            apply_ui_font_to_window(hwnd, state->ui_font);
        }
        timbre_refresh_listbox(state);
        layout_timbre_window(hwnd, state);
        return 0;
    }
    case WM_SIZE:
        layout_timbre_window(hwnd, state);
        return 0;
    case WM_DRAWITEM:
        if (draw_owner_button(lparam, state)) {
            return TRUE;
        }
        break;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORLISTBOX: {
        LRESULT result = themed_dialog_control_color(wparam, state);
        if (result != 0) {
            return result;
        }
        break;
    }
    case WM_ERASEBKGND:
        if (state != NULL && is_dark_theme_active(state)) {
            RECT rect;
            GetClientRect(hwnd, &rect);
            fill_player_background((HDC)wparam, &rect, state);
            return 1;
        }
        break;
    case WM_COMMAND:
        if (state != NULL && LOWORD(wparam) == IDC_TIMBRE_LIST &&
            HIWORD(wparam) == LBN_SELCHANGE) {
            int row = (int)SendMessageA(state->timbre_listbox, LB_GETCARETINDEX, 0, 0);

            if (row >= 0) {
                LRESULT item_data = SendMessageA(state->timbre_listbox, LB_GETITEMDATA, (WPARAM)row, 0);
                unsigned group = HIWORD((DWORD_PTR)item_data);
                unsigned sample = LOWORD((DWORD_PTR)item_data);
                sync_timbre_solo_from_listbox(
                    state->hwnd != NULL ? state->hwnd : hwnd,
                    state);
                if (state->preview_hwnd != NULL && IsWindowVisible(state->preview_hwnd)) {
                    show_preview_keyboard(state, group,
                        sample == TIMBRE_LIST_GROUP_ROW ? -1 : (int)sample);
                }
            }
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_TIMBRE_CLEAR && HIWORD(wparam) == BN_CLICKED) {
            reset_timbre_selection(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_TIMBRE_TOPMOST && HIWORD(wparam) == BN_CLICKED) {
            state->timbre_topmost =
                Button_GetCheck(GetDlgItem(hwnd, IDC_TIMBRE_TOPMOST)) == BST_CHECKED ? 1 : 0;
            apply_timbre_topmost(hwnd, state->timbre_topmost);
            save_display_settings(state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_TIMBRE_HIDE && HIWORD(wparam) == BN_CLICKED) {
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }
        break;
    case WM_CLOSE:
        save_timbre_window_bounds(hwnd);
        ShowWindow(hwnd, SW_HIDE);
        return 0;
    case WM_DESTROY:
        save_timbre_window_bounds(hwnd);
        if (state != NULL) {
            state->timbre_hwnd = NULL;
            state->timbre_listbox = NULL;
            state->timbre_listbox_proc = NULL;
        }
        return 0;
    default:
        break;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static void show_timbre_window(HWND hwnd, PlayerState *state)
{
    int x;
    int y;
    int width;
    int height;

    if (state == NULL) {
        return;
    }
    if (state->timbre_hwnd == NULL) {
        load_timbre_window_bounds(&x, &y, &width, &height);
        state->timbre_hwnd = CreateWindowExA(
            WS_EX_APPWINDOW,
            "PsfSpuTimbreWindow",
            "Samples",
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
            x,
            y,
            width,
            height,
            NULL,
            NULL,
            GetModuleHandleA(NULL),
            state);
    }
    if (state->timbre_hwnd != NULL) {
        apply_timbre_topmost(state->timbre_hwnd, state->timbre_topmost);
        timbre_refresh_listbox(state);
        ShowWindow(state->timbre_hwnd, SW_SHOW);
        SetForegroundWindow(state->timbre_hwnd);
        apply_timbre_topmost(state->timbre_hwnd, state->timbre_topmost);
    }
}

static void playlist_add_folder_dialog(HWND hwnd, PlayerState *state)
{
    char folder[MAX_PATH];

    if (choose_playlist_folder(hwnd, folder, sizeof(folder))) {
        playlist_add_folder(state, folder,
            state != NULL && !state->playlist_append_mode && state->playlist_count > 0);
        show_playlist_window(state->hwnd != NULL ? state->hwnd : hwnd, state);
    }
}

static void playlist_add_files_dialog(HWND hwnd, PlayerState *state, int play_first)
{
    char first[MAX_PATH];

    if (open_psf_files_to_playlist(hwnd, state, first, sizeof(first))) {
        if (!play_first) {
            show_playlist_window(state->hwnd != NULL ? state->hwnd : hwnd, state);
        }
        if (play_first) {
            HWND main_hwnd = state->hwnd != NULL ? state->hwnd : hwnd;
            playlist_set_current_path(state, first);
            start_direct_playback(main_hwnd, state, first);
            ShowWindow(main_hwnd, SW_SHOW);
            SetForegroundWindow(main_hwnd);
        }
    }
}

static void open_direct_file_dialog(HWND hwnd, PlayerState *state)
{
    char path[MAX_PATH];

    if (state == NULL) {
        return;
    }
    if (open_psf2_file(hwnd, path, sizeof(path))) {
        remember_playlist_file(state, path);
        if (is_archive_path(path)) {
            if (archive_add_to_playlist(hwnd, state, path,
                    !state->playlist_append_mode && state->playlist_count > 0, NULL, 0)) {
                show_playlist_window(state->hwnd != NULL ? state->hwnd : hwnd, state);
            }
        } else {
            start_direct_playback(hwnd, state, path);
        }
    }
}

static void layout_playlist_window(HWND hwnd, PlayerState *state)
{
    RECT rect;
    int width;
    int height;
    int layout_width;
    int y;
    int list_height;
    const int margin = 6;
    const int gap = 6;
    const int button_h = 24;
    const int delete_w = 46;
    const int mode_w = 66;
    const int small_w = 46;
    const int play_w = 52;
    const int stop_w = 52;
    const int min_width = margin * 2 + delete_w + mode_w + mode_w + small_w + small_w +
        play_w + stop_w + gap * 6;
    const int min_height = 150;
    int x;

    if (state == NULL) {
        return;
    }
    GetClientRect(hwnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    layout_width = width < min_width ? min_width : width;
    if (height < min_height) {
        height = min_height;
    }
    y = height - button_h - 8;
    list_height = y - 12;
    if (list_height < 70) {
        list_height = 70;
    }
    if (state->playlist_listbox != NULL) {
        SetWindowPos(state->playlist_listbox, NULL, margin, margin, layout_width - margin * 2, list_height, SWP_NOZORDER);
    }
    x = margin;
    SetWindowPos(GetDlgItem(hwnd, IDC_PLAYLIST_DELETE), NULL, x, y, delete_w, button_h, SWP_NOZORDER);
    x += delete_w + gap;
    SetWindowPos(GetDlgItem(hwnd, IDC_PLAYLIST_REPLACE), NULL, x, y, mode_w, button_h, SWP_NOZORDER);
    x += mode_w + gap;
    SetWindowPos(GetDlgItem(hwnd, IDC_PLAYLIST_APPEND), NULL, x, y, mode_w, button_h, SWP_NOZORDER);
    x += mode_w + gap;
    SetWindowPos(GetDlgItem(hwnd, IDC_PLAYLIST_PREV), NULL, x, y, small_w, button_h, SWP_NOZORDER);
    x += small_w + gap;
    SetWindowPos(GetDlgItem(hwnd, IDC_PLAYLIST_NEXT), NULL, x, y, small_w, button_h, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, IDC_PLAYLIST_PLAY), NULL, layout_width - margin - stop_w - gap - play_w, y, play_w, button_h, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, IDC_PLAYLIST_STOP), NULL, layout_width - margin - stop_w, y, stop_w, button_h, SWP_NOZORDER);
}

static LRESULT CALLBACK playlist_listbox_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    PlayerState *state = (PlayerState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    if (msg == WM_KEYDOWN && state != NULL && wparam == VK_RETURN) {
        playlist_play_index(
            state->playlist_hwnd != NULL ? state->playlist_hwnd : hwnd,
            state,
            playlist_selected_index(state));
        return 0;
    }
    if (state != NULL && state->playlist_listbox_proc != NULL) {
        return CallWindowProcA(state->playlist_listbox_proc, hwnd, msg, wparam, lparam);
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static LRESULT CALLBACK playlist_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    PlayerState *state = (PlayerState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCTA *create = (CREATESTRUCTA *)lparam;
        HMENU menu = CreateMenu();
        HMENU playlist_menu = CreatePopupMenu();
        HMENU settings_menu = CreatePopupMenu();

        state = (PlayerState *)create->lpCreateParams;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)state);
        if (state != NULL) {
            state->playlist_hwnd = hwnd;
        }
        append_darkable_menu_item(playlist_menu, IDM_OPEN, "Add files...");
        append_darkable_menu_item(playlist_menu, IDM_OPEN_FOLDER, "Add folder...");
        append_darkable_menu_separator(playlist_menu);
        append_darkable_menu_item(playlist_menu, IDM_PLAYLIST_LOAD, "Load playlist...");
        append_darkable_menu_item(playlist_menu, IDM_PLAYLIST_SAVE, "Save playlist...");
        append_darkable_menu_bar_popup(menu, playlist_menu, "Playlist");
        append_darkable_menu_item(settings_menu, IDM_TAG_FADE_ENABLED, "Fade at tag time");
        append_darkable_menu_item(settings_menu, IDM_PLAYLIST_TOPMOST, "Always on top");
        append_darkable_menu_separator(settings_menu);
        append_darkable_menu_item(settings_menu, IDM_PLAYLIST_ORDER_SINGLE, "Current track only");
        append_darkable_menu_item(settings_menu, IDM_PLAYLIST_ORDER_NEXT, "Next track");
        append_darkable_menu_item(settings_menu, IDM_PLAYLIST_ORDER_RANDOM, "Random");
        append_darkable_menu_bar_popup(menu, settings_menu, "Settings");
        SetMenu(hwnd, menu);
        apply_window_theme(hwnd, state);
        if (state != NULL) {
            CheckMenuItem(menu, IDM_PLAYLIST_TOPMOST,
                MF_BYCOMMAND | (state->playlist_topmost ? MF_CHECKED : MF_UNCHECKED));
            update_playlist_settings_menu_check(state);
        }

        if (state != NULL) {
            state->playlist_listbox = CreateWindowA("LISTBOX", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL,
                6, 6, 480, 330, hwnd, (HMENU)(UINT_PTR)IDC_PLAYLIST_LIST, NULL, NULL);
            if (state->playlist_listbox != NULL) {
                SetWindowLongPtrA(state->playlist_listbox, GWLP_USERDATA, (LONG_PTR)state);
                state->playlist_listbox_proc = (WNDPROC)SetWindowLongPtrA(
                    state->playlist_listbox,
                    GWLP_WNDPROC,
                    (LONG_PTR)playlist_listbox_wnd_proc);
            }
        }
        CreateWindowA("BUTTON", "Del", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            6, 350, 46, 24, hwnd, (HMENU)(UINT_PTR)IDC_PLAYLIST_DELETE, NULL, NULL);
        CreateWindowA("BUTTON", "Replace", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            58, 350, 66, 24, hwnd, (HMENU)(UINT_PTR)IDC_PLAYLIST_REPLACE, NULL, NULL);
        CreateWindowA("BUTTON", "Append", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            130, 350, 66, 24, hwnd, (HMENU)(UINT_PTR)IDC_PLAYLIST_APPEND, NULL, NULL);
        CreateWindowA("BUTTON", "Prev", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            202, 350, 44, 24, hwnd, (HMENU)(UINT_PTR)IDC_PLAYLIST_PREV, NULL, NULL);
        CreateWindowA("BUTTON", "Next", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            252, 350, 44, 24, hwnd, (HMENU)(UINT_PTR)IDC_PLAYLIST_NEXT, NULL, NULL);
        CreateWindowA("BUTTON", "Play", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_OWNERDRAW,
            430, 350, 52, 24, hwnd, (HMENU)(UINT_PTR)IDC_PLAYLIST_PLAY, NULL, NULL);
        CreateWindowA("BUTTON", "Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            488, 350, 52, 24, hwnd, (HMENU)(UINT_PTR)IDC_PLAYLIST_STOP, NULL, NULL);
        if (state != NULL) {
            apply_ui_font_to_window(hwnd, state->ui_font);
        }
        DragAcceptFiles(hwnd, TRUE);
        playlist_refresh_listbox(state);
        layout_playlist_window(hwnd, state);
        if (state != NULL && state->playlist_topmost) {
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
        return 0;
    }
    case WM_SIZE:
        layout_playlist_window(hwnd, state);
        return 0;
    case WM_GETMINMAXINFO: {
        MINMAXINFO *minmax = (MINMAXINFO *)lparam;
        minmax->ptMinTrackSize.x = 420;
        minmax->ptMinTrackSize.y = 190;
        return 0;
    }
    case WM_DROPFILES: {
        HDROP drop = (HDROP)wparam;
        UINT count = DragQueryFileW(drop, 0xFFFFFFFFu, NULL, 0);
        UINT i;
        int added = 0;
        int replace_pending = state != NULL && !state->playlist_append_mode && state->playlist_count > 0;

        if (state != NULL) {
            for (i = 0; i < count; ++i) {
                wchar_t path_w[MAX_PATH];
                char path[MAX_PATH];
                char first[MAX_PATH];
                path[0] = '\0';
                path_w[0] = L'\0';
                if (DragQueryFileW(drop, i, path_w, MAX_PATH) != 0 &&
                    wide_path_to_runtime_path(path_w, path, sizeof(path))) {
                    remember_playlist_file(state, path);
                    int container = is_directory_path(path) || is_archive_path(path);
                    int added_source = add_music_source_to_playlist(hwnd, state, path,
                        replace_pending && container, first, sizeof(first));
                    added += added_source ? 1 : 0;
                    if (added_source && container) {
                        replace_pending = 0;
                    }
                }
            }
            if (added > 0) {
                playlist_refresh_listbox(state);
            }
        }
        DragFinish(drop);
        return 0;
    }
    case WM_MEASUREITEM:
        if (measure_owner_menu_item(lparam)) {
            return TRUE;
        }
        break;
    case WM_DRAWITEM:
        if (draw_owner_menu_item(lparam, state)) {
            return TRUE;
        }
        if (draw_owner_button(lparam, state)) {
            return TRUE;
        }
        break;
    case WM_NOTIFY:
        if (state != NULL && ((NMHDR *)lparam)->hwndFrom == state->speed_slider) {
            return draw_speed_slider_custom((NMHDR *)lparam, state);
        }
        break;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORLISTBOX: {
        LRESULT result = themed_dialog_control_color(wparam, state);
        if (result != 0) {
            return result;
        }
        break;
    }
    case WM_ERASEBKGND:
        if (state != NULL && is_dark_theme_active(state)) {
            RECT rect;
            GetClientRect(hwnd, &rect);
            fill_player_background((HDC)wparam, &rect, state);
            return 1;
        }
        break;
    case WM_KEYDOWN:
        if (state != NULL && wparam == VK_DELETE) {
            playlist_delete_selection(state);
            return 0;
        }
        if (state != NULL && wparam == VK_RETURN) {
            playlist_play_index(hwnd, state, playlist_selected_index(state));
            return 0;
        }
        break;
    case WM_COMMAND:
        if (state != NULL && LOWORD(wparam) == IDC_PLAYLIST_LIST && HIWORD(wparam) == LBN_DBLCLK) {
            playlist_play_index(hwnd, state, playlist_selected_index(state));
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_OPEN) {
            playlist_add_files_dialog(hwnd, state, 0);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_OPEN_FOLDER) {
            playlist_add_folder_dialog(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_PLAYLIST_PREV) {
            playlist_play_next(hwnd, state, -1);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_PLAYLIST_NEXT) {
            playlist_play_next(hwnd, state, 1);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_PLAYLIST_DELETE) {
            playlist_delete_selection(state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_PLAYLIST_REPLACE) {
            state->playlist_append_mode = 0;
            save_playlist_append_mode(0);
            InvalidateRect(GetDlgItem(hwnd, IDC_PLAYLIST_REPLACE), NULL, TRUE);
            InvalidateRect(GetDlgItem(hwnd, IDC_PLAYLIST_APPEND), NULL, TRUE);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_PLAYLIST_APPEND) {
            state->playlist_append_mode = 1;
            save_playlist_append_mode(1);
            InvalidateRect(GetDlgItem(hwnd, IDC_PLAYLIST_REPLACE), NULL, TRUE);
            InvalidateRect(GetDlgItem(hwnd, IDC_PLAYLIST_APPEND), NULL, TRUE);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_PLAYLIST_PLAY) {
            playlist_play_index(hwnd, state, playlist_selected_index(state));
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_PLAYLIST_STOP) {
            cancel_direct_start_retry(hwnd, state);
            playlist_cancel_pending_play(hwnd, state);
            stop_playback(state);
            reset_stopped_display(state);
            set_status(state, "Stopped");
            if (state->hwnd != NULL) {
                update_settings_menu_check(state->hwnd, state);
                InvalidateRect(state->hwnd, NULL, TRUE);
            }
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_PLAYLIST_SAVE) {
            playlist_save_to_file(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_PLAYLIST_LOAD) {
            playlist_load_from_file(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_PLAYLIST_TOPMOST) {
            HMENU menu = GetMenu(hwnd);
            state->playlist_topmost = !state->playlist_topmost;
            save_playlist_topmost(state->playlist_topmost);
            CheckMenuItem(menu, IDM_PLAYLIST_TOPMOST,
                MF_BYCOMMAND | (state->playlist_topmost ? MF_CHECKED : MF_UNCHECKED));
            SetWindowPos(hwnd,
                state->playlist_topmost ? HWND_TOPMOST : HWND_NOTOPMOST,
                0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_TAG_FADE_ENABLED) {
            set_tag_fade_enabled(hwnd, state, !state->tag_fade_enabled);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_PLAYLIST_ORDER_NEXT) {
            set_playlist_order_mode(state, 0, 0);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_PLAYLIST_ORDER_RANDOM) {
            set_playlist_order_mode(state, 0, 1);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_PLAYLIST_ORDER_SINGLE) {
            set_playlist_order_mode(state, 1, 0);
            return 0;
        }
        break;
    case WM_CLOSE:
        save_playlist_window_bounds(hwnd);
        ShowWindow(hwnd, SW_HIDE);
        return 0;
    case WM_DESTROY:
        save_playlist_window_bounds(hwnd);
        if (state != NULL) {
            state->playlist_hwnd = NULL;
            state->playlist_listbox = NULL;
            state->playlist_listbox_proc = NULL;
        }
        return 0;
    default:
        break;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static void show_playlist_window(HWND hwnd, PlayerState *state)
{
    int x;
    int y;
    int width;
    int height;

    if (state == NULL) {
        return;
    }
    if (state->playlist_hwnd == NULL) {
        load_playlist_window_bounds(&x, &y, &width, &height);
        state->playlist_hwnd = CreateWindowExA(
            0,
            "PsfSpuPlaylistWindow",
            "Playlist",
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
            x,
            y,
            width,
            height,
            NULL,
            NULL,
            GetModuleHandleA(NULL),
            state);
    }
    if (state->playlist_hwnd != NULL) {
        playlist_refresh_listbox(state);
        SetWindowPos(state->playlist_hwnd,
            state->playlist_topmost ? HWND_TOPMOST : HWND_NOTOPMOST,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE);
        ShowWindow(state->playlist_hwnd, SW_SHOW);
        SetForegroundWindow(state->playlist_hwnd);
    }
}

static int fast_forward_core_ex(PlayerState *state, uint64_t frames_to_skip, uint64_t base_sample, int discard_snapshots)
{
    uint64_t rendered_total = 0;
    int16_t *discard_pcm;
    int ok = 1;

    if (state == NULL || frames_to_skip == 0) {
        return 1;
    }

    discard_pcm = (int16_t *)calloc(PLAYER_SEEK_RENDER_FRAMES * 2u, sizeof(*discard_pcm));
    if (discard_pcm == NULL) {
        return 0;
    }

    set_status(state, "Seeking...");
    lock_state(state);
    state->seek_discarding = discard_snapshots ? 1 : 0;
    unlock_state(state);

    while (rendered_total < frames_to_skip) {
        uint32_t frames = PLAYER_SEEK_RENDER_FRAMES;
        uint32_t rendered = 0;
        Psf2CoreBridgeResult result;
        uint64_t pending_target = 0;
        int has_pending_target = 0;

        if (state->stop_event != NULL && WaitForSingleObject(state->stop_event, 0) != WAIT_TIMEOUT) {
            ok = 0;
            break;
        }

        lock_state(state);
        if (state->seek_request) {
            has_pending_target = 1;
            pending_target = state->seek_target_sample;
            state->seek_request = 0;
        }
        unlock_state(state);
        if (has_pending_target) {
            uint64_t current_seek_sample = base_sample + rendered_total;
            if (pending_target < current_seek_sample) {
                lock_state(state);
                state->seek_target_sample = pending_target;
                state->seek_request = 1;
                unlock_state(state);
                ok = 2;
                break;
            }
            frames_to_skip = pending_target - base_sample;
            if (rendered_total >= frames_to_skip) {
                break;
            }
        }

        if (frames_to_skip - rendered_total < frames) {
            frames = (uint32_t)(frames_to_skip - rendered_total);
        }

        result = state->provider->render(state->core, discard_pcm, frames, &rendered);
        if (result != PSF2_CORE_BRIDGE_OK || rendered == 0) {
            player_log("seek fast-forward failed result=%d rendered=%lu skip=%llu done=%llu",
                (int)result,
                (unsigned long)rendered,
                (unsigned long long)frames_to_skip,
                (unsigned long long)rendered_total);
            ok = 0;
            break;
        }
        rendered_total += rendered;
    }

    lock_state(state);
    state->seek_discarding = 0;
    state->live.last_sample_pos = base_sample + rendered_total;
    unlock_state(state);
    free(discard_pcm);
    return ok;
}

static int fast_forward_core(PlayerState *state, uint64_t frames_to_skip, uint64_t base_sample)
{
    return fast_forward_core_ex(state, frames_to_skip, base_sample, 1);
}

static int reopen_core_for_seek(PlayerState *state, uint64_t *out_sample_pos)
{
    Psf2CoreCallbacks callbacks;
    Psf2CoreBridgeResult result;
    char path[MAX_PATH];

    if (state == NULL || state->provider == NULL) {
        return 0;
    }
    if (out_sample_pos != NULL) {
        *out_sample_pos = 0;
    }

    lock_state(state);
    snprintf(path, sizeof(path), "%s", state->input_path);
    unlock_state(state);
    if (path[0] == '\0') {
        return 0;
    }

    if (state->core != NULL && state->provider->close != NULL) {
        state->provider->close(state->core);
        state->core = NULL;
    }

    reset_live_display(state);
    ZeroMemory(&callbacks, sizeof(callbacks));
    callbacks.user = state;
    callbacks.spu2_write16 = player_spu2_write16;
    callbacks.voice_snapshot = player_voice_snapshot;
    callbacks.core_snapshot = player_core_snapshot;

    result = state->provider->open(&state->core, path, PLAYER_SAMPLE_RATE, &callbacks);
    player_log("seek reopen provider result=%d core=%p", (int)result, (void *)state->core);
    if (result != PSF2_CORE_BRIDGE_OK) {
        state->core = NULL;
        return 0;
    }

    apply_voice_mute_masks_immediate(state);
    apply_voice_reverb_masks(state);
    apply_voice_noise_masks(state);
    apply_voice_pmod_masks(state);
    apply_voice_adsr_force_masks(state);
    clear_imported_voice_pitch_locks();
    clear_imported_voice_volume_locks();
    if (!advance_ps2_core_to_audible_start(state, out_sample_pos)) {
        return 0;
    }
    return 1;
}

static void prescan_timbre_list(HWND hwnd, PlayerState *state, const char *path)
{
    Psf2CoreCallbacks callbacks;
    Psf2CoreBridge *scan_core = NULL;
    Psf2CoreBridgeResult result;
    uint64_t scan_frames;
    uint64_t scanned_total = 0;
    uint64_t saved_total_samples;
    uint32_t scanned = 0;
    void *saved_core;
    int16_t *discard_pcm = NULL;

    if (state == NULL || state->provider == NULL || path == NULL || path[0] == '\0') {
        return;
    }

    lock_state(state);
    saved_total_samples = state->total_samples;
    unlock_state(state);
    scan_frames = saved_total_samples;
    if (scan_frames == 0) {
        scan_frames = (uint64_t)PLAYER_SAMPLE_RATE * TIMBRE_PRESCAN_FALLBACK_SECONDS;
    }
    if (scan_frames > (uint64_t)PLAYER_SAMPLE_RATE * TIMBRE_PRESCAN_MAX_SECONDS) {
        scan_frames = (uint64_t)PLAYER_SAMPLE_RATE * TIMBRE_PRESCAN_MAX_SECONDS;
    }

    ZeroMemory(&callbacks, sizeof(callbacks));
    callbacks.user = state;
    callbacks.spu2_write16 = player_spu2_write16;
    callbacks.voice_snapshot = player_voice_snapshot;
    callbacks.core_snapshot = player_core_snapshot;

    psf2log_set_imported_timbre_solo(0, NULL, NULL, NULL, 0);
    psf2log_set_imported_voice_mute_masks(0, 0);
    result = state->provider->open(&scan_core, path, PLAYER_SAMPLE_RATE, &callbacks);
    if (result != PSF2_CORE_BRIDGE_OK || scan_core == NULL) {
        return;
    }

    saved_core = state->core;
    state->core = scan_core;
    lock_state(state);
    state->timbre_prescanning = 1;
    unlock_state(state);
    player_log("timbre fast scan begin frames=%llu", (unsigned long long)scan_frames);

    while (scanned_total < scan_frames) {
        uint32_t batch = TIMBRE_PRESCAN_BATCH_FRAMES;
        MSG msg;

        if (scan_frames - scanned_total < batch) {
            batch = (uint32_t)(scan_frames - scanned_total);
        }
        scanned = 0;
        result = psf2log_scan_imported_timbres(scan_core, batch, &scanned);
        scanned_total += scanned;
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (result != PSF2_CORE_BRIDGE_OK || scanned == 0) {
            break;
        }
    }

    /* Some DVA revisions advance their sequence from hardware interrupt timing.
       The CPU-only path cannot observe that wait, so retry with normal offline
       rendering only when the fast pass found no playable sample at all. */
    if (state->timbre_list_count == 0 && state->provider->render != NULL) {
        if (state->provider->close != NULL) {
            state->provider->close(scan_core);
        }
        scan_core = NULL;
        lock_state(state);
        preview_free_samples_locked(state);
        reset_timbre_list_locked(state);
        unlock_state(state);
        result = state->provider->open(&scan_core, path, PLAYER_SAMPLE_RATE, &callbacks);
        if (result == PSF2_CORE_BRIDGE_OK && scan_core != NULL) {
            discard_pcm = (int16_t *)calloc(
                TIMBRE_PRESCAN_BATCH_FRAMES * 2u,
                sizeof(*discard_pcm));
        }
        if (discard_pcm != NULL) {
            player_log("timbre scan fallback begin frames=%llu", (unsigned long long)scan_frames);
            state->core = scan_core;
            scanned_total = 0;
            while (scanned_total < scan_frames) {
                uint32_t batch = TIMBRE_PRESCAN_BATCH_FRAMES;
                uint32_t rendered = 0;
                MSG msg;

                if (scan_frames - scanned_total < batch) {
                    batch = (uint32_t)(scan_frames - scanned_total);
                }
                result = state->provider->render(scan_core, discard_pcm, batch, &rendered);
                scanned_total += rendered;
                while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessageA(&msg);
                }
                if (result != PSF2_CORE_BRIDGE_OK || rendered == 0) {
                    break;
                }
            }
            player_log("timbre scan fallback end scanned=%llu groups=%u",
                (unsigned long long)scanned_total,
                state->timbre_list_count);
        }
        free(discard_pcm);
    }

    state->core = (Psf2CoreBridge *)saved_core;
    lock_state(state);
    state->timbre_prescanning = 0;
    timbre_list_regroup_locked(state);
    state->timbre_list_locked = 1;
    unlock_state(state);
    if (scan_core != NULL && state->provider->close != NULL) {
        state->provider->close(scan_core);
    }
    player_log("timbre fast scan end scanned=%llu groups=%u",
        (unsigned long long)scanned_total,
        state->timbre_list_count);
    {
        unsigned group;
        for (group = 0; group < state->timbre_list_count; ++group) {
            unsigned sample;
            player_log("timbre group=%u samples=%u adsr=%04X/%04X flags=%02X",
                group + 1u,
                state->timbre_list_key_count[group],
                state->timbre_list_adsr1[group],
                state->timbre_list_adsr2[group],
                (unsigned)state->timbre_list_flags[group]);
            for (sample = 0; sample < state->timbre_list_key_count[group]; ++sample) {
                player_log("timbre group=%u sample=%u ssa=%05X lsa=%05X end=%05X",
                    group + 1u,
                    sample + 1u,
                    state->timbre_list_ssa_keys[group][sample],
                    state->timbre_list_lsa_keys[group][sample],
                    state->timbre_list_end_keys[group][sample]);
            }
        }
    }

    reset_live_display(state);
    lock_state(state);
    state->live.last_sample_pos = 0;
    unlock_state(state);
    if (state->timbre_hwnd != NULL) {
        timbre_refresh_listbox(state);
    }
    InvalidateRect(hwnd, NULL, FALSE);
}

static void start_playback_at(HWND hwnd, PlayerState *state, const char *path, uint64_t start_sample)
{
    Psf2CoreCallbacks callbacks;
    Psf2CoreBridgeResult result;
    char previous_path[MAX_PATH];
    char open_path[MAX_PATH];
    char game[128];
    char title[128];
    char window_title[512];
    uint64_t seek_base_sample = 0;
    uint64_t tag_length_samples = 0;
    uint64_t tag_fade_samples = 0;
    int playback_from_playlist;
    int path_changed;

    if (state == NULL || path == NULL || path[0] == '\0') {
        return;
    }
    snprintf(open_path, sizeof(open_path), "%s", path);
    path = open_path;
    player_log("start playback begin path=%s start_sample=%llu",
        path,
        (unsigned long long)start_sample);
    lock_state(state);
    snprintf(previous_path, sizeof(previous_path), "%s", state->input_path);
    unlock_state(state);
    path_changed = (open_path[0] != '\0' && lstrcmpiA(previous_path, open_path) != 0);
    playback_from_playlist = state != NULL && state->playlist_play_in_progress;
    read_psf_timing_samples(
        path, &tag_length_samples, &tag_fade_samples);

    if (state != NULL && state->psf_version == 0x01u) {
        save_psf1_mode_window_bounds(hwnd, state->psf1_display_mode);
    }
    stop_playback_for_track_switch(state);
    player_log("start playback after stop");
    reset_live_display(state);
    lock_state(state);
    state->voice_reverb_force_on_mask[0] = 0u;
    state->voice_reverb_force_on_mask[1] = 0u;
    state->voice_reverb_force_off_mask[0] = 0u;
    state->voice_reverb_force_off_mask[1] = 0u;
    state->voice_noise_force_on_mask[0] = 0u;
    state->voice_noise_force_on_mask[1] = 0u;
    state->voice_noise_force_off_mask[0] = 0u;
    state->voice_noise_force_off_mask[1] = 0u;
    state->voice_pmod_force_on_mask[0] = 0u;
    state->voice_pmod_force_on_mask[1] = 0u;
    state->voice_pmod_force_off_mask[0] = 0u;
    state->voice_pmod_force_off_mask[1] = 0u;
    state->voice_adsr_force_mask[0] = 0u;
    state->voice_adsr_force_mask[1] = 0u;
    ZeroMemory(state->debug_adsr_touched_mask, sizeof(state->debug_adsr_touched_mask));
    ZeroMemory(state->debug_saved_voice_valid, sizeof(state->debug_saved_voice_valid));
    ZeroMemory(state->debug_saved_core_valid, sizeof(state->debug_saved_core_valid));
    state->voice_pitch_lock_mask[0] = 0u;
    state->voice_pitch_lock_mask[1] = 0u;
    ZeroMemory(state->voice_pitch_lock_value, sizeof(state->voice_pitch_lock_value));
    ZeroMemory(state->voice_volume_lock_mask, sizeof(state->voice_volume_lock_mask));
    ZeroMemory(state->voice_volume_lock_value, sizeof(state->voice_volume_lock_value));
    if (path_changed) {
        state->voice_mute_mask[0] = 0u;
        state->voice_mute_mask[1] = 0u;
        reset_timbre_solo_locked(state);
        preview_free_samples_locked(state);
        reset_timbre_list_locked(state);
        ZeroMemory(state->psf1_voice_timbre_key, sizeof(state->psf1_voice_timbre_key));
        ZeroMemory(state->psf1_tuning_cache, sizeof(state->psf1_tuning_cache));
        state->psf1_tuning_cache_count = 0;
        state->ps2_startup_origin_valid = 0;
        state->ps2_startup_origin_sample = 0;
    }
    state->psf_version = normalize_saved_psf_version(read_psf_version(path));
    state->ps2_startup_track_dimmed = state->psf_version == 0x02u ? 1 : 0;
    state->startup_silence_trim =
        state->psf_version == 0x02u && start_sample == 0 ? 1 : 0;
    state->timeline_source_anchor = 0;
    state->timeline_scaled_anchor = 0;
    state->timeline_speed_percent = effective_speed_percent_locked(state);
    state->timeline_valid = start_sample > 0 ? 1 : 0;
    state->time_label_cache_valid = 0;
    state->total_samples = tag_length_samples;
    state->fade_samples = tag_fade_samples;
    state->playback_from_playlist = playback_from_playlist ? 1 : 0;
    unlock_state(state);
    save_last_psf_version(state->psf_version);
    apply_psf_window_mode(hwnd, state, state->psf_version);
    update_settings_menu_check(hwnd, state);
    read_psf_title_tags(path, game, sizeof(game), title, sizeof(title));
    if (title[0] != '\0') {
        snprintf(window_title, sizeof(window_title), "%s - PSF SPU Player " PSF2_PLAYER_VERSION_DISPLAY, title);
    } else if (game[0] != '\0') {
        snprintf(window_title, sizeof(window_title), "%s - PSF SPU Player " PSF2_PLAYER_VERSION_DISPLAY, game);
    } else {
        snprintf(window_title, sizeof(window_title), "%s - PSF SPU Player " PSF2_PLAYER_VERSION_DISPLAY, path_basename(path));
    }
    SetWindowTextA(hwnd, window_title);
    playlist_set_current_path(state, path);
    update_time_label(state);
    psf2log_set_imported_main_enabled(state->main_enabled);
    psf2log_set_imported_reverb_enabled(state->reverb_enabled);
    psf2log_set_imported_text_log_enabled(state->text_log_enabled);
    apply_voice_mute_masks(state);
    apply_voice_reverb_masks(state);
    apply_voice_noise_masks(state);
    apply_voice_pmod_masks(state);
    apply_voice_adsr_force_masks(state);
    clear_imported_voice_pitch_locks();
    clear_imported_voice_volume_locks();

    state->provider = psf2log_get_imported_provider();
    if (state->provider == NULL) {
        player_log("start playback failed no provider");
        set_status(state, "No PSF2 provider");
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }
    player_log("start playback provider=%s", state->provider->name != NULL ? state->provider->name : "(null)");

    if (start_sample == 0 && state->timbre_scan_enabled &&
        (path_changed || !state->timbre_list_locked)) {
        prescan_timbre_list(hwnd, state, path);
    } else if (!state->timbre_scan_enabled) {
        lock_state(state);
        state->timbre_list_locked = 1;
        unlock_state(state);
    }

    state->pcm = (int16_t *)calloc(PLAYER_RENDER_FRAMES * 2u, sizeof(*state->pcm));
    if (state->pcm == NULL) {
        player_log("start playback failed pcm allocation");
        set_status(state, "Out of memory");
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }
    player_log("start playback pcm allocated");

    if (!open_waveout(&state->wave, state)) {
        player_log("start playback failed waveOut open");
        set_status(state, "waveOut open failed");
        stop_playback(state);
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }
    player_log("start playback waveOut opened");

    if (!init_audio_queue(state)) {
        player_log("start playback failed audio queue allocation");
        set_status(state, "Out of audio buffers");
        stop_playback(state);
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }
    player_log("start playback audio queue allocated");

    ZeroMemory(&callbacks, sizeof(callbacks));
    callbacks.user = state;
    callbacks.spu2_write16 = player_spu2_write16;
    callbacks.voice_snapshot = player_voice_snapshot;
    callbacks.core_snapshot = player_core_snapshot;

    lock_state(state);
    if (start_sample == 0) {
        state->timeline_source_anchor = 0;
        state->timeline_scaled_anchor = 0;
        state->timeline_speed_percent = effective_speed_percent_locked(state);
        state->timeline_valid = 0;
        state->time_label_cache_valid = 0;
    }
    unlock_state(state);

    player_log("start playback provider open begin");
    result = state->provider->open(&state->core, path, PLAYER_SAMPLE_RATE, &callbacks);
    player_log("start playback provider open result=%d core=%p", (int)result, (void *)state->core);
    if (result != PSF2_CORE_BRIDGE_OK) {
        char message[128];
        snprintf(message, sizeof(message), "Open failed: %s", psf2_core_bridge_result_string(result));
        set_status(state, message);
        stop_playback(state);
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }
    apply_voice_mute_masks_immediate(state);
    apply_voice_reverb_masks(state);
    apply_voice_noise_masks(state);
    apply_voice_pmod_masks(state);
    apply_voice_adsr_force_masks(state);

    if (start_sample > 0 && state->psf_version == 0x02 &&
        !advance_ps2_core_to_audible_start(state, &seek_base_sample)) {
        set_status(state, "Seek failed");
        stop_playback(state);
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }
    if (start_sample > 0 && state->psf_version == 0x02) {
        lock_state(state);
        state->ps2_startup_track_dimmed = 0;
        unlock_state(state);
    }
    if (start_sample > seek_base_sample &&
        !fast_forward_core(state, start_sample - seek_base_sample, seek_base_sample)) {
        set_status(state, "Seek failed");
        stop_playback(state);
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }

    state->stop_event = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (state->stop_event == NULL) {
        player_log("start playback failed CreateEvent error=%lu", (unsigned long)GetLastError());
        set_status(state, "CreateEvent failed");
        stop_playback(state);
        InvalidateRect(hwnd, NULL, TRUE);
        return;
    }
    player_log("start playback stop event created");

    lock_state(state);
    snprintf(state->input_path, sizeof(state->input_path), "%s", path);
    snprintf(state->status, sizeof(state->status), "Playing direct");
    state->playing = 1;
    state->paused = 0;
    state->playback_generation += 1u;
    if (state->playback_generation == 0u) {
        state->playback_generation = 1u;
    }
    unlock_state(state);
    update_pause_button_label(state);

    state->thread = CreateThread(NULL, 0, playback_thread_proc, state, 0, NULL);
    if (state->thread == NULL) {
        player_log("start playback failed CreateThread error=%lu", (unsigned long)GetLastError());
        set_status(state, "CreateThread failed");
        stop_playback(state);
        InvalidateRect(hwnd, NULL, TRUE);
        return;
    }
    player_log("start playback thread created");

    update_scrollbar(hwnd, state);
    InvalidateRect(hwnd, NULL, FALSE);
    player_log("start playback done thread-render");
}

static void start_playback(HWND hwnd, PlayerState *state, const char *path)
{
    start_playback_at(hwnd, state, path, 0);
}

static void cancel_direct_start_retry(HWND hwnd, PlayerState *state)
{
    HWND target;

    if (state == NULL) {
        return;
    }
    target = state->hwnd != NULL ? state->hwnd : hwnd;
    if (target != NULL) {
        KillTimer(target, PLAYER_DIRECT_RETRY_TIMER_ID);
    }
    state->direct_start_retry_armed = 0;
    state->direct_start_retry_remaining = 0;
    state->direct_start_retry_from_playlist = 0;
    state->direct_start_retry_playlist_index = -1;
    state->direct_start_retry_deadline = 0;
    state->direct_start_retry_path[0] = '\0';
}

static void start_direct_playback(HWND hwnd, PlayerState *state, const char *path)
{
    HWND target;
    char stable_path[MAX_PATH];

    if (state == NULL || path == NULL || path[0] == '\0') {
        return;
    }
    snprintf(stable_path, sizeof(stable_path), "%s", path);
    target = state->hwnd != NULL ? state->hwnd : hwnd;
    cancel_direct_start_retry(target, state);
    playlist_cancel_pending_play(target, state);
    snprintf(state->direct_start_retry_path,
        sizeof(state->direct_start_retry_path), "%s", stable_path);
    state->direct_start_retry_remaining = 2;
    state->direct_start_retry_from_playlist = 0;
    state->direct_start_retry_playlist_index = -1;
    start_playback(target, state, stable_path);
    state->direct_start_retry_deadline =
        GetTickCount() + PLAYER_START_RETRY_TIMEOUT_MS;
    state->direct_start_retry_armed = 1;
    if (target != NULL) {
        SetTimer(target, PLAYER_DIRECT_RETRY_TIMER_ID,
            PLAYER_DIRECT_RETRY_TIMER_MS, NULL);
    }
}

static void process_direct_start_retry(HWND hwnd, PlayerState *state)
{
    char path[MAX_PATH];
    int from_playlist;
    int playlist_index;
    DWORD now;

    if (state == NULL) {
        return;
    }
    KillTimer(hwnd, PLAYER_DIRECT_RETRY_TIMER_ID);
    if (!state->direct_start_retry_armed) {
        return;
    }
    if (get_playing(state)) {
        if (state->audio_started) {
            cancel_direct_start_retry(hwnd, state);
            return;
        }
        now = GetTickCount();
        if ((LONG)(now - state->direct_start_retry_deadline) < 0) {
            SetTimer(hwnd, PLAYER_DIRECT_RETRY_TIMER_ID,
                PLAYER_DIRECT_RETRY_TIMER_MS, NULL);
            return;
        }
        player_log("%s playback audio start timed out path=%s",
            state->direct_start_retry_from_playlist ? "playlist" : "direct",
            state->direct_start_retry_path);
    }
    if (state->direct_start_retry_remaining <= 0 ||
        state->direct_start_retry_path[0] == '\0') {
        cancel_direct_start_retry(hwnd, state);
        return;
    }
    snprintf(path, sizeof(path), "%s", state->direct_start_retry_path);
    from_playlist = state->direct_start_retry_from_playlist;
    playlist_index = state->direct_start_retry_playlist_index;
    state->direct_start_retry_remaining -= 1;
    state->direct_start_retry_armed = 0;
    player_log("%s playback startup retry remaining=%d path=%s",
        from_playlist ? "playlist" : "direct",
        state->direct_start_retry_remaining,
        path);
    if (from_playlist) {
        state->playlist_play_in_progress = 1;
        if (playlist_index >= 0 &&
            (unsigned)playlist_index < state->playlist_count) {
            state->playlist_current = playlist_index;
            playlist_refresh_listbox(state);
        }
    }
    start_playback(hwnd, state, path);
    if (from_playlist) {
        state->playlist_play_in_progress = 0;
    }
    state->direct_start_retry_deadline =
        GetTickCount() + PLAYER_START_RETRY_TIMEOUT_MS;
    state->direct_start_retry_armed = 1;
    if (state->direct_start_retry_remaining > 0) {
        SetTimer(hwnd, PLAYER_DIRECT_RETRY_TIMER_ID,
            PLAYER_DIRECT_RETRY_TIMER_MS, NULL);
    } else if (!get_playing(state)) {
        cancel_direct_start_retry(hwnd, state);
    }
}

static void seek_absolute_sample(HWND hwnd, PlayerState *state, uint64_t target)
{
    uint64_t total_samples;
    int playing;
    int paused;
    char path[MAX_PATH];

    if (state == NULL || state->input_path[0] == '\0') {
        return;
    }

    lock_state(state);
    total_samples = state->total_samples;
    playing = state->playing;
    paused = state->paused;
    snprintf(path, sizeof(path), "%s", state->input_path);
    unlock_state(state);

    if (total_samples > 0 && target > total_samples) {
        target = total_samples;
    }

    player_log("seek absolute target=%llu", (unsigned long long)target);
    if (state->core != NULL && playing && !paused) {
        lock_state(state);
        state->seek_target_sample = target;
        state->seek_request = 1;
        unlock_state(state);
        set_status(state, "Seeking...");
        InvalidateRect(hwnd, NULL, TRUE);
        return;
    }
    start_playback_at(hwnd, state, path, target);
}

static void input_seek_time(HWND hwnd, PlayerState *state)
{
    uint64_t current;
    double seconds;
    uint64_t target;

    if (state == NULL || state->input_path[0] == '\0') {
        return;
    }
    lock_state(state);
    current = state->live.last_sample_pos;
    unlock_state(state);
    if (!choose_seek_time(hwnd, state, current, &seconds)) {
        return;
    }
    target = (uint64_t)((seconds * (double)PLAYER_SAMPLE_RATE) + 0.5);
    seek_absolute_sample(hwnd, state, target);
}

static void seek_relative_seconds(HWND hwnd, PlayerState *state, int seconds)
{
    int64_t current;
    int64_t target;
    uint64_t total_samples;

    if (state == NULL || state->input_path[0] == '\0') {
        return;
    }

    lock_state(state);
    current = (int64_t)state->live.last_sample_pos;
    total_samples = state->total_samples;
    unlock_state(state);

    target = current + ((int64_t)seconds * (int64_t)PLAYER_SAMPLE_RATE);
    if (target < 0) {
        target = 0;
    }
    if (total_samples > 0 && (uint64_t)target > total_samples) {
        target = (int64_t)total_samples;
    }

    player_log("seek relative seconds=%d current=%lld target=%lld",
        seconds,
        (long long)current,
        (long long)target);
    seek_absolute_sample(hwnd, state, (uint64_t)target);
}

static void frame_advance_tick(HWND hwnd, PlayerState *state, int delta_ticks)
{
    if (state == NULL || !state->frame_advance || delta_ticks <= 0) {
        return;
    }

    lock_state(state);
    if (!state->frame_live_valid) {
        state->frame_live = state->live;
        state->frame_live_valid = 1;
    }
    state->frame_step_request += (uint32_t)delta_ticks;
    state->sequence += 1;
    unlock_state(state);
    set_status(state, "Frame advance");
    InvalidateRect(hwnd, NULL, FALSE);
}

static int player_core_panel_x(
    HWND hwnd,
    const PlayerState *state,
    uint8_t psf_version,
    unsigned core)
{
    RECT client_rect;
    int logical_width;
    int panel_x;

    if (psf_version != 0x01u || core != 0u || hwnd == NULL) {
        return core == 0u ? CORE0_X : CORE1_X;
    }

    if (IsZoomed(hwnd)) {
        PlayerScaleLayout layout;

        get_player_content_scale_layout(hwnd, state, &layout);
        logical_width = layout.base_width;
    } else if (GetClientRect(hwnd, &client_rect)) {
        logical_width = client_rect.right - client_rect.left;
    } else {
        logical_width = CORE_PANEL_WIDTH + (CORE0_X * 2);
    }
    panel_x = (logical_width - CORE_PANEL_WIDTH) / 2;
    return panel_x > 0 ? panel_x : 0;
}

static void render_playback_tick(HWND hwnd, PlayerState *state)
{
    RECT redraw_rect;
    uint64_t render_sequence;
    int render_source;
    int gauge_only;
    int same_render_state;
    int skip_draw;
    int line_height;
    int core0_x;
    uint8_t psf_version;

    if (state == NULL) {
        return;
    }

    update_time_label(state);
    if (state->playback_only || IsIconic(hwnd) || !IsWindowVisible(hwnd)) {
        return;
    }

    lock_state(state);
    if (state->frame_advance && state->frame_live_valid) {
        render_source = 3;
        render_sequence = state->sequence;
    } else if (!state->frame_advance && state->audible_display_valid) {
        render_source = 2;
        render_sequence = state->audible_display_snapshot.sequence;
    } else {
        render_source = 1;
        render_sequence = state->sequence;
    }
    same_render_state = state->display_render_source == render_source &&
        state->display_render_sequence == render_sequence;
    gauge_only = state->playing && !state->frame_advance &&
        state->audible_display_valid && state->paint_frame_initialized &&
        state->gauge_animation_active && same_render_state;
    skip_draw = state->playing && !state->frame_advance &&
        state->audible_display_valid && state->paint_frame_initialized &&
        !state->gauge_animation_active &&
        state->display_render_source == render_source &&
        state->display_render_sequence == render_sequence;
    state->display_render_source = render_source;
    state->display_render_sequence = render_sequence;
    state->gauge_only_paint_pending = gauge_only;
    line_height = state->meter_line_height > 0 ? state->meter_line_height : 16;
    psf_version = state->psf_version;
    unlock_state(state);

    if (skip_draw) {
        return;
    }

    if (IsZoomed(hwnd)) {
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }

    GetClientRect(hwnd, &redraw_rect);
    if (gauge_only) {
        core0_x = player_core_panel_x(hwnd, state, psf_version, 0u);
        redraw_rect.left = core0_x + COL_ENV_BAR_X;
        redraw_rect.top = 2 + CONTROLS_HEIGHT + (line_height * 2);
        redraw_rect.right = psf_version == 0x01u ?
            core0_x + COL_VOL_BAR_X + 100 :
            CORE1_X + COL_VOL_BAR_X + 100;
    } else {
        redraw_rect.top = CONTROLS_HEIGHT;
    }
    InvalidateRect(hwnd, &redraw_rect, FALSE);
}

static int psf1_track_white_key_count(void)
{
    int note;
    int count = 0;

    for (note = PSF1_TRACK_KEY_FIRST_NOTE;
         note <= PSF1_TRACK_KEY_LAST_NOTE;
         ++note) {
        if (!preview_is_black_key(note)) {
            count++;
        }
    }
    return count;
}

static void psf1_track_key_rect(int note, int width, RECT *out_rect)
{
    int white_count = psf1_track_white_key_count();
    int white_index = 0;
    int current;

    ZeroMemory(out_rect, sizeof(*out_rect));
    for (current = PSF1_TRACK_KEY_FIRST_NOTE;
         current <= PSF1_TRACK_KEY_LAST_NOTE;
         ++current) {
        if (current == note) {
            if (preview_is_black_key(current)) {
                int boundary = (width * white_index) / white_count;
                int black_width = (width / white_count) * 3 / 5;
                if (black_width < 5) {
                    black_width = 5;
                }
                out_rect->left = boundary - black_width / 2;
                out_rect->right = boundary + (black_width + 1) / 2;
                out_rect->top = 0;
                out_rect->bottom = (PSF1_TRACK_KEYBOARD_HEIGHT * 3) / 5;
            } else {
                out_rect->left = (width * white_index) / white_count;
                out_rect->right = (width * (white_index + 1)) / white_count;
                out_rect->top = 0;
                out_rect->bottom = PSF1_TRACK_KEYBOARD_HEIGHT;
            }
            return;
        }
        if (!preview_is_black_key(current)) {
            white_index++;
        }
    }
}

static void draw_psf1_track_white_key_boundaries(
    HDC hdc,
    int width,
    int height)
{
    int white_count = psf1_track_white_key_count();
    int white_index = 0;
    int note;

    if (hdc == NULL || width <= 0 || height <= 0 || white_count <= 0) {
        return;
    }
    for (note = PSF1_TRACK_KEY_FIRST_NOTE;
         note <= PSF1_TRACK_KEY_LAST_NOTE;
         ++note) {
        int boundary;
        int top;

        if (preview_is_black_key(note)) {
            continue;
        }
        white_index++;
        if (white_index >= white_count) {
            break;
        }
        boundary = (width * white_index) / white_count;
        top = preview_is_black_key(note + 1) ? (height * 3) / 5 : 0;

        MoveToEx(hdc, boundary, top, NULL);
        LineTo(hdc, boundary, height);
    }
    MoveToEx(hdc, width - 1, 0, NULL);
    LineTo(hdc, width - 1, height);
}

static int ensure_psf1_track_keyboard(
    PlayerState *state,
    HDC reference_dc,
    int dark,
    int width)
{
    HBRUSH white_brush;
    HBRUSH black_brush;
    HPEN border_pen;
    HGDIOBJ old_brush;
    HGDIOBJ old_pen;
    RECT rect = {0, 0, width, PSF1_TRACK_KEYBOARD_HEIGHT};
    int white_count = psf1_track_white_key_count();
    int white_index = 0;
    int note;

    if (state == NULL || reference_dc == NULL || width <= 0) {
        return 0;
    }
    if (state->psf1_keyboard_dc != NULL &&
        state->psf1_keyboard_bitmap != NULL &&
        state->psf1_keyboard_base_dc != NULL &&
        state->psf1_keyboard_base_bitmap != NULL &&
        state->psf1_keyboard_dark == dark &&
        state->psf1_keyboard_width == width) {
        return 1;
    }
    if (state->psf1_keyboard_base_dc != NULL) {
        if (state->psf1_keyboard_base_old_bitmap != NULL) {
            SelectObject(state->psf1_keyboard_base_dc,
                state->psf1_keyboard_base_old_bitmap);
        }
        if (state->psf1_keyboard_base_bitmap != NULL) {
            DeleteObject(state->psf1_keyboard_base_bitmap);
        }
        DeleteDC(state->psf1_keyboard_base_dc);
        state->psf1_keyboard_base_dc = NULL;
        state->psf1_keyboard_base_bitmap = NULL;
        state->psf1_keyboard_base_old_bitmap = NULL;
    }
    if (state->psf1_keyboard_dc != NULL) {
        if (state->psf1_keyboard_old_bitmap != NULL) {
            SelectObject(state->psf1_keyboard_dc, state->psf1_keyboard_old_bitmap);
        }
        if (state->psf1_keyboard_bitmap != NULL) {
            DeleteObject(state->psf1_keyboard_bitmap);
        }
        DeleteDC(state->psf1_keyboard_dc);
        state->psf1_keyboard_dc = NULL;
        state->psf1_keyboard_bitmap = NULL;
        state->psf1_keyboard_old_bitmap = NULL;
        state->psf1_keyboard_width = 0;
    }
    state->psf1_keyboard_dc = CreateCompatibleDC(reference_dc);
    state->psf1_keyboard_bitmap = CreateCompatibleBitmap(
        reference_dc, width, PSF1_TRACK_KEYBOARD_HEIGHT);
    if (state->psf1_keyboard_dc == NULL || state->psf1_keyboard_bitmap == NULL) {
        if (state->psf1_keyboard_bitmap != NULL) {
            DeleteObject(state->psf1_keyboard_bitmap);
        }
        if (state->psf1_keyboard_dc != NULL) {
            DeleteDC(state->psf1_keyboard_dc);
        }
        state->psf1_keyboard_dc = NULL;
        state->psf1_keyboard_bitmap = NULL;
        return 0;
    }
    state->psf1_keyboard_old_bitmap = (HBITMAP)SelectObject(
        state->psf1_keyboard_dc, state->psf1_keyboard_bitmap);
    white_brush = CreateSolidBrush(dark ? RGB(48, 48, 48) : RGB(248, 248, 248));
    black_brush = CreateSolidBrush(dark ? RGB(3, 3, 3) : RGB(0, 0, 0));
    border_pen = CreatePen(PS_SOLID, 1, dark ? RGB(160, 160, 160) : RGB(45, 45, 45));
    FillRect(state->psf1_keyboard_dc, &rect, white_brush);
    old_brush = SelectObject(state->psf1_keyboard_dc, white_brush);
    old_pen = SelectObject(state->psf1_keyboard_dc, border_pen);
    for (note = PSF1_TRACK_KEY_FIRST_NOTE;
         note <= PSF1_TRACK_KEY_LAST_NOTE;
         ++note) {
        if (!preview_is_black_key(note)) {
            int left = (width * white_index) / white_count;
            int right = (width * (white_index + 1)) / white_count;
            Rectangle(state->psf1_keyboard_dc,
                left, 0, right + 1, PSF1_TRACK_KEYBOARD_HEIGHT);
            white_index++;
        }
    }
    white_index = 0;
    SelectObject(state->psf1_keyboard_dc, black_brush);
    for (note = PSF1_TRACK_KEY_FIRST_NOTE;
         note <= PSF1_TRACK_KEY_LAST_NOTE;
         ++note) {
        if (preview_is_black_key(note)) {
            RECT key;
            psf1_track_key_rect(note, width, &key);
            Rectangle(state->psf1_keyboard_dc,
                key.left, key.top, key.right + 1, key.bottom + 1);
        } else {
            white_index++;
        }
    }
    draw_psf1_track_white_key_boundaries(
        state->psf1_keyboard_dc, width, PSF1_TRACK_KEYBOARD_HEIGHT);
    SelectObject(state->psf1_keyboard_dc, old_pen);
    SelectObject(state->psf1_keyboard_dc, old_brush);
    DeleteObject(border_pen);
    DeleteObject(black_brush);
    DeleteObject(white_brush);
    state->psf1_keyboard_base_dc = CreateCompatibleDC(reference_dc);
    state->psf1_keyboard_base_bitmap = CreateCompatibleBitmap(
        reference_dc, width, PSF1_TRACK_KEYBOARD_HEIGHT);
    if (state->psf1_keyboard_base_dc == NULL ||
        state->psf1_keyboard_base_bitmap == NULL) {
        if (state->psf1_keyboard_base_bitmap != NULL) {
            DeleteObject(state->psf1_keyboard_base_bitmap);
        }
        if (state->psf1_keyboard_base_dc != NULL) {
            DeleteDC(state->psf1_keyboard_base_dc);
        }
        state->psf1_keyboard_base_dc = NULL;
        state->psf1_keyboard_base_bitmap = NULL;
        return 0;
    }
    state->psf1_keyboard_base_old_bitmap = (HBITMAP)SelectObject(
        state->psf1_keyboard_base_dc, state->psf1_keyboard_base_bitmap);
    BitBlt(state->psf1_keyboard_base_dc, 0, 0,
        width, PSF1_TRACK_KEYBOARD_HEIGHT,
        state->psf1_keyboard_dc, 0, 0, SRCCOPY);
    state->psf1_keyboard_dark = dark;
    state->psf1_keyboard_width = width;
    return 1;
}

static void paint_psf1_track_keyboard(
    HDC hdc,
    PlayerState *state,
    int x,
    int y,
    int width,
    int active_note,
    int muted)
{
    if (hdc == NULL || state == NULL || state->psf1_keyboard_dc == NULL ||
        state->psf1_keyboard_base_dc == NULL ||
        width <= 0 || state->psf1_keyboard_width != width) {
        return;
    }
    if (active_note >= PSF1_TRACK_KEY_FIRST_NOTE &&
        active_note <= PSF1_TRACK_KEY_LAST_NOTE) {
        RECT key;
        RECT active_rect;
        RECT restore_rect;
        int full_key_highlight = state->hwnd != NULL &&
            IsZoomed(state->hwnd) &&
            psf1_keyboard_height_for_state(state, state->psf_version) > 0;
        HBRUSH active_brush = CreateSolidBrush(
            muted ? RGB(154, 78, 88) : RGB(230, 64, 84));

        psf1_track_key_rect(active_note, width, &key);
        active_rect = key;
        if (full_key_highlight) {
            if (active_rect.right < width) {
                active_rect.right += 1;
            }
            if (preview_is_black_key(active_note) &&
                active_rect.bottom < PSF1_TRACK_KEYBOARD_HEIGHT) {
                active_rect.bottom += 1;
            }
            if (full_key_highlight) {
                active_rect.top += 1;
                if (!preview_is_black_key(active_note)) {
                    active_rect.bottom -= 1;
                }
                if (key.left <= 0) {
                    active_rect.left += 1;
                }
                if (active_rect.right >= width) {
                    active_rect.right -= 1;
                }
            }
        } else {
            active_rect.top += 1;
            if (preview_is_black_key(active_note)) {
                active_rect.left += 1;
            } else {
                active_rect.left += 1;
                active_rect.bottom -= 1;
            }
        }
        if (active_rect.right > active_rect.left &&
            active_rect.bottom > active_rect.top) {
            FillRect(state->psf1_keyboard_dc, &active_rect, active_brush);
            if (!preview_is_black_key(active_note)) {
                int note;

                for (note = PSF1_TRACK_KEY_FIRST_NOTE;
                     note <= PSF1_TRACK_KEY_LAST_NOTE;
                     ++note) {
                    RECT black_key;

                    if (!preview_is_black_key(note)) {
                        continue;
                    }
                    psf1_track_key_rect(note, width, &black_key);
                    if (black_key.right < active_rect.left ||
                        black_key.left >= active_rect.right) {
                        continue;
                    }
                    BitBlt(state->psf1_keyboard_dc,
                        black_key.left, black_key.top,
                        black_key.right - black_key.left + 1,
                        black_key.bottom - black_key.top + 1,
                        state->psf1_keyboard_base_dc,
                        black_key.left, black_key.top, SRCCOPY);
                }
            }
        }
        BitBlt(hdc, x, y, width, PSF1_TRACK_KEYBOARD_HEIGHT,
            state->psf1_keyboard_dc, 0, 0, SRCCOPY);
        restore_rect = active_rect;
        if (restore_rect.left < 0) {
            restore_rect.left = 0;
        }
        if (restore_rect.right > width) {
            restore_rect.right = width;
        }
        if (restore_rect.top < 0) {
            restore_rect.top = 0;
        }
        if (restore_rect.bottom > PSF1_TRACK_KEYBOARD_HEIGHT) {
            restore_rect.bottom = PSF1_TRACK_KEYBOARD_HEIGHT;
        }
        if (restore_rect.right > restore_rect.left &&
            restore_rect.bottom > restore_rect.top) {
            BitBlt(state->psf1_keyboard_dc,
                restore_rect.left, restore_rect.top,
                restore_rect.right - restore_rect.left,
                restore_rect.bottom - restore_rect.top,
                state->psf1_keyboard_base_dc,
                restore_rect.left, restore_rect.top, SRCCOPY);
        }
        DeleteObject(active_brush);
    } else {
        BitBlt(hdc, x, y, width, PSF1_TRACK_KEYBOARD_HEIGHT,
            state->psf1_keyboard_dc, 0, 0, SRCCOPY);
    }
}

static void paint_core_panel(
    HDC hdc,
    PlayerState *player_state,
    const Spu2LogLiveState *live,
    const uint8_t key_on_events[2][24],
    const uint8_t release_events[2][24],
    const uint32_t voice_mute_mask[2],
    const uint32_t voice_reverb_force_on_mask[2],
    const uint32_t voice_reverb_force_off_mask[2],
    const uint32_t voice_noise_force_on_mask[2],
    const uint32_t voice_noise_force_off_mask[2],
    const uint32_t voice_pmod_force_on_mask[2],
    const uint32_t voice_pmod_force_off_mask[2],
    const uint32_t gauge_env[2][24],
    const uint32_t gauge_vol_l[2][24],
    const uint32_t gauge_vol_r[2][24],
    const int psf1_track_notes[24],
    const Psf1AkaoPlaybackState *akao,
    unsigned core,
    int hide_inactive,
    int env_color_index,
    int lr_color_index,
    COLORREF env_custom_color,
    COLORREF lr_custom_color,
    int x,
    int y,
    int line_height,
    int active_header_line_height,
    int voice_scroll_y,
    int show_core_label,
    uint8_t psf_version,
    int stopped_display,
    int force_track_dimmed,
    COLORREF active_text_color,
    COLORREF inactive_text_color,
    COLORREF muted_text_color,
    COLORREF gauge_background_color,
    COLORREF gauge_border_color,
    int voice_rows_target_bottom,
    int gauges_only)
{
    unsigned voice;
    char line[160];
    const char *adsr_header;
    POLYTEXTA header_text[6];
    POLYTEXTA row_text[5];
    const Spu2LogCoreSnapshot *core_state = &live->cores[core];
    int active_count;
    int attack_count;
    int decay_count;
    int sustain_count;
    int release_count;
    unsigned noise_clock;
    unsigned noise_hz;
    COLORREF env_color = env_color_from_index(env_color_index, env_custom_color);
    COLORREF lr_left_color = lr_left_color_from_index(lr_color_index, lr_custom_color);
    COLORREF lr_right_color = lr_right_color_from_index(lr_color_index, lr_custom_color);
    COLORREF current_text_color = active_text_color;
    TEXTMETRICA text_metrics;
    int text_char_width = 8;
    int show_values = psf1_values_visible_for_state(player_state, psf_version);
    int keyboard_height = psf1_keyboard_height_for_state(player_state, psf_version);
    int keyboard_gap = psf1_keyboard_gap_for_state(player_state, psf_version);
    int visible_count;
    int row_content_height;
    int row_extra_total = 0;
    int visible_row_index = 0;
    int rows_clip_saved_dc = 0;
    int voice_hit_clip_top = 0;
    HFONT previous_row_font = NULL;

    if (GetTextMetricsA(hdc, &text_metrics) && text_metrics.tmAveCharWidth > 0) {
        text_char_width = text_metrics.tmAveCharWidth;
    }

    SetDCPenColor(hdc, gauge_border_color);
    if (!gauges_only) {
        SetTextColor(hdc, active_text_color);
        voice_status_counts(
            live,
            core,
            &active_count,
            &attack_count,
            &decay_count,
            &sustain_count,
            &release_count);
        noise_clock = noise_clock_from_core_flags(core_state->flags);
        noise_hz = noise_clock_frequency_hz(noise_clock);
        if (show_core_label) {
            snprintf(line, sizeof(line),
                "Core %u  active:%02d/24  ADSR:%02d/%02d/%02d/%02d  rv:0x%04X/0x%04X  nclk:%5uHz",
                core,
                active_count,
                attack_count,
                decay_count,
                sustain_count,
                release_count,
                core_state->reverb_l,
                core_state->reverb_r,
                noise_hz);
        } else if (psf_version == 0x01u && akao != NULL && akao->detected) {
            unsigned denominator = akao->beat_denominator != 0u ?
                akao->beat_denominator :
                (akao->ticks_per_beat != 0 ? 192u / akao->ticks_per_beat : 0u);
            if (akao->driver_type == PSF1_MUSIC_DRIVER_SONY_SEQ) {
                snprintf(line, sizeof(line),
                    "active:%02d/24  ADSR:%02d/%02d/%02d/%02d  rv:0x%04X/0x%04X nclk:%5uHz tempo:0x%06lX(%7.3f) %u/%u beat:%02u bar:%04u",
                    active_count,
                    attack_count,
                    decay_count,
                    sustain_count,
                    release_count,
                    core_state->reverb_l,
                    core_state->reverb_r,
                    noise_hz,
                    (unsigned long)akao->tempo,
                    psf1_tempo_bpm(akao->tempo, akao->early_format, akao->driver_type),
                    akao->beats_per_measure,
                    denominator,
                    (unsigned)akao->current_beat + 1u,
                    akao->measure);
            } else {
                snprintf(line, sizeof(line),
                    "active:%02d/24  ADSR:%02d/%02d/%02d/%02d  rv:0x%04X/0x%04X nclk:%5uHz tempo:0x%04lX(%7.3f) %u/%u beat:%02u bar:%04u",
                    active_count,
                    attack_count,
                    decay_count,
                    sustain_count,
                    release_count,
                    core_state->reverb_l,
                    core_state->reverb_r,
                    noise_hz,
                    (unsigned long)akao->tempo,
                    psf1_tempo_bpm(akao->tempo, akao->early_format, akao->driver_type),
                    akao->beats_per_measure,
                    denominator,
                    (unsigned)akao->current_beat + 1u,
                    akao->measure);
            }
        } else {
            snprintf(line, sizeof(line),
                "active:%02d/24  ADSR:%02d/%02d/%02d/%02d  rv:0x%04X/0x%04X nclk:%5uHz",
                active_count,
                attack_count,
                decay_count,
                sustain_count,
                release_count,
                core_state->reverb_l,
                core_state->reverb_r,
                noise_hz);
        }
        if (psf_version == 0x01u && player_state != NULL &&
            IsZoomed(player_state->hwnd)) {
            PlayerScaleLayout header_layout;
            SIZE text_size;
            int saved_dc;
            int active_header_x = x;

            get_fullscreen_reference_layout(
                player_state->hwnd, player_state, &header_layout);
            saved_dc = SaveDC(hdc);
            if (saved_dc != 0 && header_layout.scaled) {
                SetMapMode(hdc, MM_ANISOTROPIC);
                SetWindowExtEx(hdc, header_layout.base_width,
                    header_layout.base_height, NULL);
                SetViewportExtEx(hdc, header_layout.dest_width,
                    header_layout.dest_height, NULL);
                SetViewportOrgEx(hdc, header_layout.dest_x,
                    header_layout.dest_y, NULL);
                if (player_state->meter_font != NULL) {
                    SelectObject(hdc, player_state->meter_font);
                }
                if (GetTextExtentPoint32A(
                        hdc, line, (int)strlen(line), &text_size)) {
                    active_header_x = x +
                        (CORE_PANEL_WIDTH - text_size.cx) / 2;
                }
                TextOutA(hdc, active_header_x, y,
                    line, (int)strlen(line));
                RestoreDC(hdc, saved_dc);
            } else {
                if (saved_dc != 0) {
                    RestoreDC(hdc, saved_dc);
                }
                if (GetTextExtentPoint32A(
                        hdc, line, (int)strlen(line), &text_size)) {
                    active_header_x = x +
                        (CORE_PANEL_WIDTH - text_size.cx) / 2;
                }
                TextOutA(hdc, active_header_x, y,
                    line, (int)strlen(line));
            }
        } else {
            SIZE text_size;
            int active_header_x = x;

            if (GetTextExtentPoint32A(
                    hdc, line, (int)strlen(line), &text_size)) {
                active_header_x = x +
                    (CORE_PANEL_WIDTH - text_size.cx) / 2;
            }
            TextOutA(hdc, active_header_x, y,
                line, (int)strlen(line));
        }
        y += active_header_line_height;

        if (show_values) {
            adsr_header = adsr_rate_header_for_psf(psf_version);
            ZeroMemory(header_text, sizeof(header_text));
            header_text[0].x = x + COL_TEXT_X;
            header_text[0].y = y;
            header_text[0].n = 10;
            header_text[0].lpstr = (LPSTR)"Vo On ADSR";
            header_text[1].x = x + COL_TEXT_X + (18 * text_char_width);
            header_text[1].y = y;
            header_text[1].n = 7;
            header_text[1].lpstr = (LPSTR)"Vol L/R";
            header_text[2].x = x + COL_TEXT_X + (29 * text_char_width);
            header_text[2].y = y;
            header_text[2].n = 5;
            header_text[2].lpstr = (LPSTR)"Pitch";
            header_text[3].x = x + COL_ENV_TEXT_X;
            header_text[3].y = y;
            header_text[3].n = 3;
            header_text[3].lpstr = (LPSTR)"Env";
            header_text[4].x = x + COL_VOL_BAR_X;
            header_text[4].y = y;
            header_text[4].n = 7;
            header_text[4].lpstr = (LPSTR)"Vol L/R";
            header_text[5].x = x + COL_FLAGS_X;
            header_text[5].y = y;
            header_text[5].n = (UINT)strlen(adsr_header);
            header_text[5].lpstr = adsr_header;
            PolyTextOutA(hdc, header_text, 6);
            y += line_height;

            ZeroMemory(row_text, sizeof(row_text));
            row_text[0].x = x + COL_TEXT_X;
            row_text[0].n = 17u;
            row_text[1].x = x + COL_TEXT_X + (18 * text_char_width);
            row_text[1].n = 9u;
            row_text[2].x = x + COL_TEXT_X + (29 * text_char_width);
            row_text[2].n = 4u;
            row_text[3].x = x + COL_ENV_TEXT_X;
            row_text[3].n = 4u;
            row_text[4].x = x + COL_FLAGS_X;
            row_text[4].n = 25u;
        }
    } else {
        y += active_header_line_height +
            (show_values ? line_height : 0);
    }

    if (!show_values) {
        y += keyboard_gap;
    }
    if (voice_scroll_y > 0) {
        voice_hit_clip_top = y;
        rows_clip_saved_dc = SaveDC(hdc);
        IntersectClipRect(hdc, x, y,
            x + CORE_PANEL_WIDTH, 0x7fff);
        y -= voice_scroll_y;
    }

    visible_count = visible_voice_count(live, core, hide_inactive);
    row_content_height = (show_values ? line_height : 0) + keyboard_height;
    if (voice_rows_target_bottom > 0 && visible_count > 1) {
        int base_bottom = y + row_content_height * visible_count +
            keyboard_gap * (visible_count - 1);

        if (voice_rows_target_bottom > base_bottom) {
            row_extra_total = voice_rows_target_bottom - base_bottom;
        }
    }

    if (!gauges_only &&
        psf_version == 0x01u &&
        psf1_keyboard_height_for_state(player_state, psf_version) > 0 &&
        player_state->meter_bold_font != NULL) {
        previous_row_font = (HFONT)SelectObject(hdc, player_state->meter_bold_font);
    }

    for (voice = 0; voice < 24; ++voice) {
        const Spu2LogVoiceSnapshot *v = &live->voices[core][voice];
        char modes[8];
        char rates[15];
        char voice_detail[26];
        char env_text[5];
        const char *phase_label;
        size_t phase_length;
        int row_y = y;
        int ps2_env_off = psf_version != 0x01u && v->envx == 0;
        int active = ps2_env_off ? 0 :
            (psf_version == 0x01u ? is_voice_really_sounding(v) : is_voice_active(v));
        int muted = (voice_mute_mask[core] & (1u << voice)) != 0;
        uint32_t flags;
        int key_on_pulse;
        int release_pulse;
        COLORREF row_text_color;

        if (hide_inactive && !active) {
            continue;
        }

        if (player_state != NULL) {
            RECT *voice_rect =
                &player_state->voice_number_hit_rects[core][voice];

            voice_rect->left = show_values ? x + COL_TEXT_X : x + 8;
            voice_rect->right = voice_rect->left +
                (show_values ? text_char_width * 2 : 40);
            voice_rect->top = row_y;
            voice_rect->bottom = row_y +
                (show_values ? line_height : keyboard_height);
            if (voice_hit_clip_top > 0 &&
                voice_rect->top < voice_hit_clip_top) {
                voice_rect->top = voice_hit_clip_top;
            }
            player_state->voice_number_hit_valid[core][voice] =
                voice_rect->bottom > voice_rect->top ? 1u : 0u;
        }

        if (!gauges_only && show_values) {
            flags = ps2_env_off ? 0 : effective_voice_flags(
                v->flags,
                voice_reverb_force_on_mask[core],
                voice_reverb_force_off_mask[core],
                voice_noise_force_on_mask[core],
                voice_noise_force_off_mask[core],
                voice_pmod_force_on_mask[core],
                voice_pmod_force_off_mask[core],
                voice);
            key_on_pulse = !ps2_env_off && key_on_events[core][voice] != 0;
            release_pulse = !ps2_env_off && release_events[core][voice] != 0;
            format_voice_mode_columns(modes, sizeof(modes), flags, key_on_pulse, release_pulse, psf_version);
            format_adsr_rate_columns(rates, sizeof(rates), v->adsr1, v->adsr2);
            phase_label = v->envx == 0 ?
                "off" : adsr_state_label(v->adsr_phase, v->adsr1, v->adsr2);
            phase_length = strlen(phase_label);
            if (phase_length > 11u) {
                phase_length = 11u;
            }
            line[0] = (char)('0' + (voice / 10u));
            line[1] = (char)('0' + (voice % 10u));
            line[2] = ' ';
            line[3] = v->envx >= 1 ? '*' : ' ';
            line[4] = ' ';
            line[5] = ' ';
            memset(line + 6, ' ', 11u);
            memcpy(line + 6, phase_label, phase_length);
            line[17] = ' ';
            write_hex_u16(line + 18, v->vol_l);
            line[22] = '/';
            write_hex_u16(line + 23, v->vol_r);
            line[27] = ' ';
            line[28] = ' ';
            write_hex_u16(line + 29, v->pitch);
            line[33] = '\0';
            format_hex_u16(env_text, v->envx);
            memcpy(voice_detail, modes, 7u);
            voice_detail[7] = ' ';
            voice_detail[8] = (flags & SPU2LOG_VOICE_REVERB) ? '*' : '.';
            voice_detail[9] = ' ';
            voice_detail[10] = ' ';
            memcpy(voice_detail + 11, rates, 14u);
            voice_detail[25] = '\0';
            row_text_color = force_track_dimmed ? inactive_text_color :
                (muted ? muted_text_color :
                    (psf_version == 0x01u || active || stopped_display ?
                        active_text_color : inactive_text_color));
            if (row_text_color != current_text_color) {
                SetTextColor(hdc, row_text_color);
                current_text_color = row_text_color;
            }
            if (psf_version == 0x01u && !active) {
                TextOutA(hdc, x + COL_TEXT_X, y, line, 6);
                TextOutA(hdc, x + COL_TEXT_X + (18 * text_char_width), y,
                    line + 18, 9);
                TextOutA(hdc, x + COL_TEXT_X + (29 * text_char_width), y,
                    line + 29, 4);
                TextOutA(hdc, x + COL_ENV_TEXT_X, y, env_text, 4);
                TextOutA(hdc, x + COL_FLAGS_X, y, voice_detail, 25);
                SetTextColor(hdc,
                    muted ? muted_text_color : inactive_text_color);
                current_text_color =
                    muted ? muted_text_color : inactive_text_color;
                TextOutA(hdc, x + COL_TEXT_X + (6 * text_char_width), y,
                    line + 6, 11);
            } else {
                row_text[0].y = y;
                row_text[0].lpstr = line;
                row_text[1].y = y;
                row_text[1].lpstr = line + 18;
                row_text[2].y = y;
                row_text[2].lpstr = line + 29;
                row_text[3].y = y;
                row_text[3].lpstr = env_text;
                row_text[4].y = y;
                row_text[4].lpstr = voice_detail;
                PolyTextOutA(hdc, row_text, 5);
            }
        }

        if (show_values) {
            uint32_t vol_bar_l;
            uint32_t vol_bar_r;
            COLORREF row_env_color = muted ? muted_gauge_color(env_color, gauge_background_color) : env_color;
            COLORREF row_lr_left_color = muted ? muted_gauge_color(lr_left_color, gauge_background_color) : lr_left_color;
            COLORREF row_lr_right_color = muted ? muted_gauge_color(lr_right_color, gauge_background_color) : lr_right_color;
            COLORREF row_border_color = psf_version != 0x01u && !active ?
                inactive_text_color : gauge_border_color;

            SetDCPenColor(hdc, row_border_color);
            draw_bar(hdc, x + COL_ENV_BAR_X, row_y + 1, 175, line_height - 5, gauge_env[core][voice], 0x7fffu,
                row_env_color, gauge_background_color,
                player_state != NULL && IsZoomed(player_state->hwnd));
            vol_bar_l = gauge_vol_l[core][voice];
            vol_bar_r = gauge_vol_r[core][voice];
            draw_stereo_bar(hdc, x + COL_VOL_BAR_X, row_y + 1, 100, line_height - 5,
                vol_bar_l, vol_bar_r, 0x3fffu, row_lr_left_color, row_lr_right_color,
                gauge_background_color,
                player_state != NULL && IsZoomed(player_state->hwnd));
        }

        if (psf_version == 0x01u &&
            psf1_keyboard_height_for_state(player_state, psf_version) > 0 &&
            !gauges_only) {
            int active_note = v->envx >= 1 && psf1_track_notes != NULL ?
                psf1_track_notes[voice] : -1;
            int keyboard_x = show_values ? x + 8 : x + 48;
            int keyboard_width = show_values ?
                CORE_PANEL_WIDTH - 16 : CORE_PANEL_WIDTH - 56;
            int keyboard_y = row_y + (show_values ? line_height : 0);

            paint_psf1_track_keyboard(
                hdc,
                player_state,
                keyboard_x,
                keyboard_y,
                keyboard_width,
                active_note,
                muted);
            if (!show_values) {
                char voice_label[5];
                COLORREF label_color = muted ? muted_text_color :
                    (force_track_dimmed ? inactive_text_color : active_text_color);

                snprintf(voice_label, sizeof(voice_label), "Vo%02u", voice);
                SetTextColor(hdc, label_color);
                current_text_color = label_color;
                TextOutA(hdc, x + 8, keyboard_y + 2,
                    voice_label, (int)strlen(voice_label));
            }
        }
        y += row_content_height;
        if (visible_row_index + 1 < visible_count) {
            int extra_before = visible_count > 1 ?
                (row_extra_total * visible_row_index) / (visible_count - 1) : 0;
            int extra_after = visible_count > 1 ?
                (row_extra_total * (visible_row_index + 1)) / (visible_count - 1) : 0;

            y += keyboard_gap + extra_after - extra_before;
        }
        ++visible_row_index;
    }
    if (!gauges_only && current_text_color != active_text_color) {
        SetTextColor(hdc, active_text_color);
    }
    if (previous_row_font != NULL) {
        SelectObject(hdc, previous_row_font);
    }
    if (rows_clip_saved_dc != 0) {
        RestoreDC(hdc, rows_clip_saved_dc);
    }
}

static void paint_player(HWND hwnd, HDC hdc, PlayerState *state, int gauges_only)
{
    int line_height;
    int y = 2;
    HFONT font;
    HFONT old_font;
    HBRUSH old_brush;
    HPEN old_pen;
    Spu2LogLiveState live;
    uint8_t key_on_events[2][24];
    uint8_t release_events[2][24];
    uint32_t voice_mute_mask[2];
    uint32_t voice_reverb_force_on_mask[2];
    uint32_t voice_reverb_force_off_mask[2];
    uint32_t voice_noise_force_on_mask[2];
    uint32_t voice_noise_force_off_mask[2];
    uint32_t voice_pmod_force_on_mask[2];
    uint32_t voice_pmod_force_off_mask[2];
    uint32_t gauge_env[2][24];
    uint32_t gauge_vol_l[2][24];
    uint32_t gauge_vol_r[2][24];
    int psf1_track_notes[24];
    int use_audible_display;
    int stopped_display;
    int startup_track_dimmed;
    int hide_inactive;
    int env_color_index;
    int lr_color_index;
    COLORREF env_custom_color;
    COLORREF lr_custom_color;
    COLORREF active_text_color;
    COLORREF inactive_text_color;
    COLORREF muted_text_color;
    COLORREF gauge_background_color;
    COLORREF gauge_border_color;
    uint8_t psf_version;
    Psf1AkaoPlaybackState akao;
    unsigned display_core_count;
    unsigned note_voice;
    int core0_x;
    int voice_rows_target_bottom;
    int active_header_line_height;
    int voice_scroll_y;

    if (state == NULL) {
        return;
    }

    ZeroMemory(state->voice_number_hit_valid,
        sizeof(state->voice_number_hit_valid));

    for (note_voice = 0; note_voice < 24u; ++note_voice) {
        psf1_track_notes[note_voice] = -1;
    }

    lock_state(state);
    ZeroMemory(&akao, sizeof(akao));
    psf_version = state->psf_version;
    display_core_count = psf_version == 0x01u ? 1u : 2u;
    use_audible_display = !state->frame_advance && state->audible_display_valid;
    if (state->frame_advance && state->frame_live_valid) {
        live = state->frame_live;
    } else if (use_audible_display) {
        live = state->audible_display_snapshot.live;
    } else {
        live = state->live;
    }
    akao = use_audible_display ?
        state->audible_display_snapshot.akao : state->akao;
    if (state->playing && !state->frame_advance && !state->audio_started) {
        ZeroMemory(&akao, sizeof(akao));
    }
    stopped_display = use_audible_display ?
        state->audible_display_snapshot.stopped_display :
        state->stopped_display;
    startup_track_dimmed = psf_version == 0x02u && (use_audible_display ?
        state->audible_display_snapshot.startup_track_dimmed :
        state->ps2_startup_track_dimmed);
    if (!stopped_display) {
        unsigned hold_core;
        unsigned hold_voice;

        for (hold_core = 0; hold_core < display_core_count; ++hold_core) {
            for (hold_voice = 0; hold_voice < 24; ++hold_voice) {
                uint64_t hold_until = use_audible_display ?
                    state->audible_display_snapshot.voice_display_hold_until[hold_core][hold_voice] :
                    state->voice_display_hold_until[hold_core][hold_voice];
                const Spu2LogVoiceSnapshot *held_voice = use_audible_display ?
                    &state->audible_display_snapshot.voice_display_hold[hold_core][hold_voice] :
                    &state->voice_display_hold[hold_core][hold_voice];

                if (!is_voice_active(&live.voices[hold_core][hold_voice]) &&
                    hold_until != 0 &&
                    live.last_sample_pos <= hold_until &&
                    should_hold_voice_for_display(held_voice)) {
                    live.voices[hold_core][hold_voice] = *held_voice;
                }
            }
        }
    }
    update_gauge_display_values(state, &live, psf_version);
    if (use_audible_display) {
        memcpy(key_on_events,
            state->audible_display_snapshot.key_on_events,
            sizeof(key_on_events));
        memcpy(release_events,
            state->audible_display_snapshot.release_events,
            sizeof(release_events));
    } else {
        memcpy(key_on_events, state->key_on_event_buffers, sizeof(key_on_events));
        memcpy(release_events, state->release_event_buffers, sizeof(release_events));
        if (state->frame_advance || !state->playing) {
            unsigned event_core;
            unsigned event_voice;

            for (event_core = 0; event_core < display_core_count; ++event_core) {
                for (event_voice = 0; event_voice < 24; ++event_voice) {
                    if (state->key_on_event_buffers[event_core][event_voice] > 0) {
                        --state->key_on_event_buffers[event_core][event_voice];
                    }
                    if (state->release_event_buffers[event_core][event_voice] > 0) {
                        --state->release_event_buffers[event_core][event_voice];
                    }
                }
            }
        }
    }
    effective_voice_mute_masks_locked(state, &live, &voice_mute_mask[0], &voice_mute_mask[1]);
    memcpy(voice_reverb_force_on_mask, state->voice_reverb_force_on_mask, sizeof(voice_reverb_force_on_mask));
    memcpy(voice_reverb_force_off_mask, state->voice_reverb_force_off_mask, sizeof(voice_reverb_force_off_mask));
    memcpy(voice_noise_force_on_mask, state->voice_noise_force_on_mask, sizeof(voice_noise_force_on_mask));
    memcpy(voice_noise_force_off_mask, state->voice_noise_force_off_mask, sizeof(voice_noise_force_off_mask));
    memcpy(voice_pmod_force_on_mask, state->voice_pmod_force_on_mask, sizeof(voice_pmod_force_on_mask));
    memcpy(voice_pmod_force_off_mask, state->voice_pmod_force_off_mask, sizeof(voice_pmod_force_off_mask));
    memcpy(gauge_env, state->gauge_env, sizeof(gauge_env));
    memcpy(gauge_vol_l, state->gauge_vol_l, sizeof(gauge_vol_l));
    memcpy(gauge_vol_r, state->gauge_vol_r, sizeof(gauge_vol_r));
    hide_inactive = state->hide_inactive;
    env_color_index = state->env_color_index;
    lr_color_index = state->lr_color_index;
    env_custom_color = state->env_custom_color;
    lr_custom_color = state->lr_custom_color;
    active_text_color = player_text_color(state);
    inactive_text_color = player_inactive_text_color(state);
    muted_text_color = player_muted_text_color(state);
    gauge_background_color = player_background_color(state);
    gauge_border_color = active_text_color;
    if (psf_version == 0x01u &&
        psf1_keyboard_height_for_state(state, psf_version) > 0) {
        const uint32_t *voice_keys = use_audible_display ?
            state->audible_display_snapshot.psf1_voice_timbre_key :
            state->psf1_voice_timbre_key;

        for (note_voice = 0; note_voice < 24u; ++note_voice) {
            const Spu2LogVoiceSnapshot *voice = &live.voices[0][note_voice];
            if (voice->envx >= 1 &&
                (voice->flags & SPU2LOG_VOICE_NOISE) == 0) {
                psf1_track_notes[note_voice] = psf1_note_for_voice_locked(
                    state, voice_keys[note_voice], voice->pitch);
            }
        }
    }
    unlock_state(state);

    if (state->meter_font == NULL) {
        state->meter_font = CreateFontA(
            -14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
        if (state->meter_font == NULL) {
            state->meter_font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
        }
    }
    if (state->meter_bold_font == NULL) {
        state->meter_bold_font = CreateFontA(
            -14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
        if (state->meter_bold_font == NULL) {
            state->meter_bold_font = state->meter_font;
        }
    }
    font = state->meter_font;
    old_font = (HFONT)SelectObject(hdc, font);
    old_brush = (HBRUSH)SelectObject(hdc, GetStockObject(DC_BRUSH));
    old_pen = (HPEN)SelectObject(hdc, GetStockObject(DC_PEN));
    if (state->meter_line_height <= 0) {
        TEXTMETRICA tm;

        state->meter_line_height = GetTextMetricsA(hdc, &tm) ? tm.tmHeight + 1 : 16;
    }
    line_height = state->meter_line_height;
    active_header_line_height = maximized_active_header_line_height(
        hwnd, state, line_height);
    y += CONTROLS_HEIGHT;
    voice_scroll_y = player_voice_scroll_enabled(hwnd, state) ?
        state->scroll_y : 0;
    voice_rows_target_bottom = voice_scroll_y > 0 ||
        player_voice_scroll_enabled(hwnd, state) ? 0 :
        maximized_voice_rows_target_bottom(hwnd, state);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, active_text_color);
    core0_x = player_core_panel_x(hwnd, state, psf_version, 0u);

    if (psf_version == 0x01u &&
        psf1_keyboard_height_for_state(state, psf_version) > 0) {
        int dark_keyboard =
            ((int)GetRValue(gauge_background_color) +
             (int)GetGValue(gauge_background_color) +
             (int)GetBValue(gauge_background_color)) < 384;
        int keyboard_width = psf1_values_visible_for_state(state, psf_version) ?
            CORE_PANEL_WIDTH - 16 : CORE_PANEL_WIDTH - 56;
        ensure_psf1_track_keyboard(
            state, hdc, dark_keyboard, keyboard_width);
    }

    paint_core_panel(hdc, state, &live, key_on_events, release_events, voice_mute_mask, voice_reverb_force_on_mask, voice_reverb_force_off_mask, voice_noise_force_on_mask, voice_noise_force_off_mask, voice_pmod_force_on_mask, voice_pmod_force_off_mask, gauge_env, gauge_vol_l, gauge_vol_r, psf1_track_notes, &akao, 0, hide_inactive, env_color_index, lr_color_index, env_custom_color, lr_custom_color, core0_x, y, line_height, active_header_line_height, voice_scroll_y, psf_version != 0x01u, psf_version, stopped_display, startup_track_dimmed, active_text_color, inactive_text_color, muted_text_color, gauge_background_color, gauge_border_color, voice_rows_target_bottom, gauges_only);
    if (psf_version != 0x01u) {
        paint_core_panel(hdc, state, &live, key_on_events, release_events, voice_mute_mask, voice_reverb_force_on_mask, voice_reverb_force_off_mask, voice_noise_force_on_mask, voice_noise_force_off_mask, voice_pmod_force_on_mask, voice_pmod_force_off_mask, gauge_env, gauge_vol_l, gauge_vol_r, psf1_track_notes, NULL, 1, hide_inactive, env_color_index, lr_color_index, env_custom_color, lr_custom_color, CORE1_X, y, line_height, active_header_line_height, 0, 1, psf_version, stopped_display, startup_track_dimmed, active_text_color, inactive_text_color, muted_text_color, gauge_background_color, gauge_border_color, voice_rows_target_bottom, gauges_only);
    }

    SelectObject(hdc, old_pen);
    SelectObject(hdc, old_brush);
    SelectObject(hdc, old_font);
    (void)hwnd;
}

static void destroy_player_paint_resources(PlayerState *state)
{
    if (state == NULL) {
        return;
    }
    if (state->paint_dc != NULL && state->paint_old_bitmap != NULL) {
        SelectObject(state->paint_dc, state->paint_old_bitmap);
    }
    if (state->paint_bitmap != NULL) {
        DeleteObject(state->paint_bitmap);
    }
    if (state->paint_dc != NULL) {
        DeleteDC(state->paint_dc);
    }
    if (state->psf1_keyboard_dc != NULL &&
        state->psf1_keyboard_old_bitmap != NULL) {
        SelectObject(state->psf1_keyboard_dc, state->psf1_keyboard_old_bitmap);
    }
    if (state->psf1_keyboard_bitmap != NULL) {
        DeleteObject(state->psf1_keyboard_bitmap);
    }
    if (state->psf1_keyboard_dc != NULL) {
        DeleteDC(state->psf1_keyboard_dc);
    }
    if (state->psf1_keyboard_base_dc != NULL &&
        state->psf1_keyboard_base_old_bitmap != NULL) {
        SelectObject(state->psf1_keyboard_base_dc,
            state->psf1_keyboard_base_old_bitmap);
    }
    if (state->psf1_keyboard_base_bitmap != NULL) {
        DeleteObject(state->psf1_keyboard_base_bitmap);
    }
    if (state->psf1_keyboard_base_dc != NULL) {
        DeleteDC(state->psf1_keyboard_base_dc);
    }
    if (state->meter_bold_font != NULL &&
        state->meter_bold_font != state->meter_font &&
        state->meter_bold_font != (HFONT)GetStockObject(ANSI_FIXED_FONT)) {
        DeleteObject(state->meter_bold_font);
    }
    if (state->meter_font != NULL &&
        state->meter_font != (HFONT)GetStockObject(ANSI_FIXED_FONT)) {
        DeleteObject(state->meter_font);
    }
    state->paint_dc = NULL;
    state->paint_bitmap = NULL;
    state->paint_old_bitmap = NULL;
    state->psf1_keyboard_dc = NULL;
    state->psf1_keyboard_bitmap = NULL;
    state->psf1_keyboard_old_bitmap = NULL;
    state->psf1_keyboard_base_dc = NULL;
    state->psf1_keyboard_base_bitmap = NULL;
    state->psf1_keyboard_base_old_bitmap = NULL;
    state->psf1_keyboard_width = 0;
    state->meter_font = NULL;
    state->meter_bold_font = NULL;
    state->meter_line_height = 0;
    state->paint_width = 0;
    state->paint_height = 0;
    state->gauge_only_paint_pending = 0;
    state->paint_frame_initialized = 0;
}

static int ensure_player_backbuffer(PlayerState *state, HDC reference_dc, int width, int height)
{
    HDC new_dc;
    HBITMAP new_bitmap;
    HBITMAP old_bitmap;

    if (state == NULL || reference_dc == NULL || width <= 0 || height <= 0) {
        return 0;
    }
    if (state->paint_dc != NULL && state->paint_bitmap != NULL &&
        state->paint_width == width && state->paint_height == height) {
        return 1;
    }

    if (state->paint_dc != NULL && state->paint_old_bitmap != NULL) {
        SelectObject(state->paint_dc, state->paint_old_bitmap);
    }
    if (state->paint_bitmap != NULL) {
        DeleteObject(state->paint_bitmap);
    }
    if (state->paint_dc != NULL) {
        DeleteDC(state->paint_dc);
    }
    state->paint_dc = NULL;
    state->paint_bitmap = NULL;
    state->paint_old_bitmap = NULL;
    state->paint_width = 0;
    state->paint_height = 0;
    state->gauge_only_paint_pending = 0;
    state->paint_frame_initialized = 0;

    new_dc = CreateCompatibleDC(reference_dc);
    if (new_dc == NULL) {
        return 0;
    }
    new_bitmap = CreateCompatibleBitmap(reference_dc, width, height);
    if (new_bitmap == NULL) {
        DeleteDC(new_dc);
        return 0;
    }
    old_bitmap = (HBITMAP)SelectObject(new_dc, new_bitmap);
    if (old_bitmap == NULL || old_bitmap == HGDI_ERROR) {
        DeleteObject(new_bitmap);
        DeleteDC(new_dc);
        return 0;
    }

    state->paint_dc = new_dc;
    state->paint_bitmap = new_bitmap;
    state->paint_old_bitmap = old_bitmap;
    state->paint_width = width;
    state->paint_height = height;
    return 1;
}

static int get_player_display_line_height(HWND hwnd)
{
    HDC hdc;
    HFONT font;
    HFONT old_font;
    TEXTMETRICA tm;
    int line_height = 16;

    hdc = GetDC(hwnd);
    if (hdc == NULL) {
        return line_height;
    }

    font = CreateFontA(
        -14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    if (font == NULL) {
        font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
    }
    old_font = (HFONT)SelectObject(hdc, font);
    if (GetTextMetricsA(hdc, &tm)) {
        line_height = tm.tmHeight + 1;
    }
    SelectObject(hdc, old_font);
    if (font != GetStockObject(ANSI_FIXED_FONT)) {
        DeleteObject(font);
    }
    ReleaseDC(hwnd, hdc);
    return line_height;
}

static int maximized_voice_rows_target_bottom(
    HWND hwnd,
    const PlayerState *state)
{
    PlayerScaleLayout layout;
    RECT client_rect;
    int device_bottom;

    if (hwnd == NULL || !IsZoomed(hwnd) ||
        !GetClientRect(hwnd, &client_rect)) {
        return 0;
    }
    get_player_content_scale_layout(hwnd, state, &layout);
    if (!layout.scaled || layout.dest_height <= 0 || layout.base_height <= 0) {
        return 0;
    }
    device_bottom = client_rect.bottom - 4;
    if (device_bottom <= layout.dest_y) {
        return 0;
    }
    return (int)(((int64_t)(device_bottom - layout.dest_y) *
        layout.base_height) / layout.dest_height);
}

static int voice_row_index_from_y(
    HWND hwnd,
    const PlayerState *state,
    const Spu2LogLiveState *live,
    unsigned core,
    uint8_t psf_version,
    int hide_inactive,
    int mouse_y,
    int line_height,
    int hit_height)
{
    int show_values;
    int keyboard_height;
    int keyboard_gap;
    int row_top;
    int row_content_height;
    int visible_count;
    int target_bottom;
    int base_bottom;
    int extra_total = 0;
    int active_header_height;
    int row;

    show_values = psf1_values_visible_for_state(state, psf_version);
    keyboard_height = psf1_keyboard_height_for_state(state, psf_version);
    keyboard_gap = psf1_keyboard_gap_for_state(state, psf_version);
    active_header_height = maximized_active_header_line_height(
        hwnd, state, line_height);
    if (player_voice_scroll_enabled(hwnd, state)) {
        mouse_y += state->scroll_y;
    }
    row_top = 2 + CONTROLS_HEIGHT +
        active_header_height + (show_values ? line_height : 0) +
        (show_values ? 0 : keyboard_gap);
    row_content_height = (show_values ? line_height : 0) + keyboard_height;
    visible_count = visible_voice_count(live, core, hide_inactive);
    if (visible_count <= 0 || row_content_height <= 0) {
        return -1;
    }
    target_bottom = maximized_voice_rows_target_bottom(hwnd, state);
    base_bottom = row_top + (row_content_height * visible_count) +
        (keyboard_gap * (visible_count - 1));
    if (target_bottom > base_bottom && visible_count > 1) {
        extra_total = target_bottom - base_bottom;
    }
    for (row = 0; row < visible_count; ++row) {
        int row_y = row_top + row * (row_content_height + keyboard_gap);

        if (visible_count > 1) {
            row_y += (extra_total * row) / (visible_count - 1);
        }
        if (mouse_y >= row_y && mouse_y < row_y + hit_height) {
            return row;
        }
    }
    return -1;
}

static int get_player_display_char_width(HWND hwnd)
{
    HDC hdc;
    HFONT font;
    HFONT old_font;
    TEXTMETRICA tm;
    int char_width = 8;

    hdc = GetDC(hwnd);
    if (hdc == NULL) {
        return char_width;
    }

    font = CreateFontA(
        -14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    if (font == NULL) {
        font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
    }
    old_font = (HFONT)SelectObject(hdc, font);
    if (GetTextMetricsA(hdc, &tm)) {
        char_width = tm.tmAveCharWidth;
    }
    SelectObject(hdc, old_font);
    if (font != GetStockObject(ANSI_FIXED_FONT)) {
        DeleteObject(font);
    }
    ReleaseDC(hwnd, hdc);
    return char_width;
}

static int hit_test_voice_row(HWND hwnd, PlayerState *state, int mouse_x, int mouse_y, unsigned *out_core, unsigned *out_voice)
{
    unsigned core;
    unsigned voice;

    if (state == NULL || out_core == NULL || out_voice == NULL) {
        return 0;
    }

    (void)hwnd;
    for (core = 0; core < 2u; ++core) {
        for (voice = 0; voice < 24u; ++voice) {
            const RECT *voice_rect =
                &state->voice_number_hit_rects[core][voice];

            if (state->voice_number_hit_valid[core][voice] != 0u &&
                mouse_x >= voice_rect->left && mouse_x < voice_rect->right &&
                mouse_y >= voice_rect->top && mouse_y < voice_rect->bottom) {
                *out_core = core;
                *out_voice = voice;
                return 1;
            }
        }
    }

    return 0;
}

static int hit_test_painted_voice_y(
    const PlayerState *state,
    unsigned core,
    int mouse_y,
    unsigned *out_voice)
{
    unsigned voice;

    if (state == NULL || out_voice == NULL || core >= 2u) {
        return 0;
    }
    for (voice = 0; voice < 24u; ++voice) {
        const RECT *voice_rect =
            &state->voice_number_hit_rects[core][voice];

        if (state->voice_number_hit_valid[core][voice] != 0u &&
            mouse_y >= voice_rect->top && mouse_y < voice_rect->bottom) {
            *out_voice = voice;
            return 1;
        }
    }
    return 0;
}

static int hit_test_voice_text_column(HWND hwnd, PlayerState *state, int mouse_x, int mouse_y, unsigned *out_core, unsigned *out_voice, unsigned *out_column)
{
    uint8_t psf_version;
    int char_width;
    int panel_x;
    int rel_x;
    unsigned core;
    unsigned voice;
    unsigned column = VOICE_TEXT_COLUMN_NONE;

    if (state == NULL || out_core == NULL || out_voice == NULL || out_column == NULL) {
        return 0;
    }

    lock_state(state);
    psf_version = state->psf_version;
    unlock_state(state);

    if (!psf1_values_visible_for_state(state, psf_version)) {
        return 0;
    }

    if (psf_version == 0x01u) {
        core = 0;
        panel_x = player_core_panel_x(hwnd, state, psf_version, core);
    } else if (mouse_x >= CORE1_X) {
        core = 1;
        panel_x = CORE1_X;
    } else {
        core = 0;
        panel_x = CORE0_X;
    }

    if (mouse_x < panel_x || mouse_x >= panel_x + CORE_PANEL_WIDTH) {
        return 0;
    }
    if (mouse_x < panel_x + COL_TEXT_X) {
        return 0;
    }

    char_width = get_player_display_char_width(hwnd);
    rel_x = (mouse_x - (panel_x + COL_TEXT_X)) / char_width;
    if (rel_x >= 6 && rel_x <= 16) {
        column = VOICE_TEXT_COLUMN_ADSR_STATE;
    } else if (rel_x >= 18 && rel_x <= 21) {
        column = VOICE_TEXT_COLUMN_VOL_L;
    } else if (rel_x >= 23 && rel_x <= 26) {
        column = VOICE_TEXT_COLUMN_VOL_R;
    } else if (rel_x >= 29 && rel_x <= 33) {
        column = VOICE_TEXT_COLUMN_PITCH;
    }
    if (column == VOICE_TEXT_COLUMN_NONE) {
        return 0;
    }

    if (!hit_test_painted_voice_y(state, core, mouse_y, &voice)) {
        return 0;
    }
    *out_core = core;
    *out_voice = voice;
    *out_column = column;
    return 1;
}

static int hit_test_voice_env_column(HWND hwnd, PlayerState *state, int mouse_x, int mouse_y, unsigned *out_core, unsigned *out_voice)
{
    uint8_t psf_version;
    int panel_x;
    unsigned core;
    unsigned voice;

    if (state == NULL || out_core == NULL || out_voice == NULL) {
        return 0;
    }

    lock_state(state);
    psf_version = state->psf_version;
    unlock_state(state);

    if (!psf1_values_visible_for_state(state, psf_version)) {
        return 0;
    }

    if (psf_version == 0x01u) {
        core = 0;
        panel_x = player_core_panel_x(hwnd, state, psf_version, core);
    } else if (mouse_x >= CORE1_X) {
        core = 1;
        panel_x = CORE1_X;
    } else {
        core = 0;
        panel_x = CORE0_X;
    }

    if (mouse_x < panel_x + COL_ENV_TEXT_X || mouse_x >= panel_x + COL_ENV_BAR_X + 175) {
        return 0;
    }

    if (!hit_test_painted_voice_y(state, core, mouse_y, &voice)) {
        return 0;
    }
    *out_core = core;
    *out_voice = voice;
    return 1;
}

static int hit_test_voice_flag_column(HWND hwnd, PlayerState *state, int mouse_x, int mouse_y, unsigned *out_core, unsigned *out_voice, unsigned *out_column)
{
    uint8_t psf_version;
    int char_width;
    int panel_x;
    int flag_x;
    int rel_x;
    int column = VOICE_FLAG_COLUMN_NONE;
    unsigned core;
    unsigned voice;

    if (state == NULL || out_core == NULL || out_voice == NULL || out_column == NULL) {
        return 0;
    }

    lock_state(state);
    psf_version = state->psf_version;
    unlock_state(state);

    if (!psf1_values_visible_for_state(state, psf_version)) {
        return 0;
    }

    if (psf_version == 0x01u) {
        core = 0;
        panel_x = player_core_panel_x(hwnd, state, psf_version, core);
    } else if (mouse_x >= CORE1_X) {
        core = 1;
        panel_x = CORE1_X;
    } else {
        core = 0;
        panel_x = CORE0_X;
    }

    if (mouse_x < panel_x || mouse_x >= panel_x + CORE_PANEL_WIDTH) {
        return 0;
    }

    char_width = get_player_display_char_width(hwnd);
    flag_x = panel_x + COL_FLAGS_X;
    rel_x = (mouse_x - flag_x) / char_width;
    if (rel_x == 4) {
        column = VOICE_FLAG_COLUMN_NOISE;
    } else if (rel_x == 6) {
        column = VOICE_FLAG_COLUMN_PMOD;
    } else if (rel_x == 8 || rel_x == 9) {
        column = VOICE_FLAG_COLUMN_REVERB;
    } else if (rel_x >= 11 && rel_x <= 25) {
        int field = (rel_x - 11) / 3;
        int field_col = 11 + (field * 3);
        if (field >= 0 && field <= 4 && rel_x >= field_col && rel_x < field_col + 2) {
            column = VOICE_FLAG_COLUMN_ADSR0 + field;
        }
    }
    if (column == VOICE_FLAG_COLUMN_NONE) {
        return 0;
    }

    if (!hit_test_painted_voice_y(state, core, mouse_y, &voice)) {
        return 0;
    }
    *out_core = core;
    *out_voice = voice;
    *out_column = (unsigned)column;
    return 1;
}

static void format_core_header_line(
    const Spu2LogLiveState *live,
    unsigned core,
    uint8_t psf_version,
    int show_core_label,
    const Psf1AkaoPlaybackState *akao,
    char *line,
    size_t line_size)
{
    const Spu2LogCoreSnapshot *core_state = &live->cores[core];
    int active_count;
    int attack_count;
    int decay_count;
    int sustain_count;
    int release_count;
    unsigned noise_clock = noise_clock_from_core_flags(core_state->flags);
    unsigned noise_hz = noise_clock_frequency_hz(noise_clock);

    voice_status_counts(
        live,
        core,
        &active_count,
        &attack_count,
        &decay_count,
        &sustain_count,
        &release_count);
    if (show_core_label) {
        snprintf(line, line_size,
            "Core %u  active:%02d/24  ADSR:%02d/%02d/%02d/%02d  rv:0x%04X/0x%04X  nclk:%5uHz",
            core,
            active_count,
            attack_count,
            decay_count,
            sustain_count,
            release_count,
            core_state->reverb_l,
            core_state->reverb_r,
            noise_hz);
    } else if (psf_version == 0x01u && akao != NULL && akao->detected) {
        unsigned denominator = akao->beat_denominator != 0u ?
            akao->beat_denominator :
            (akao->ticks_per_beat != 0 ? 192u / akao->ticks_per_beat : 0u);

        if (akao->driver_type == PSF1_MUSIC_DRIVER_SONY_SEQ) {
            snprintf(line, line_size,
                "active:%02d/24  ADSR:%02d/%02d/%02d/%02d  rv:0x%04X/0x%04X nclk:%5uHz tempo:0x%06lX(%7.3f) %u/%u beat:%02u bar:%04u",
                active_count,
                attack_count,
                decay_count,
                sustain_count,
                release_count,
                core_state->reverb_l,
                core_state->reverb_r,
                noise_hz,
                (unsigned long)akao->tempo,
                psf1_tempo_bpm(
                    akao->tempo, akao->early_format, akao->driver_type),
                akao->beats_per_measure,
                denominator,
                (unsigned)akao->current_beat + 1u,
                akao->measure);
        } else {
            snprintf(line, line_size,
                "active:%02d/24  ADSR:%02d/%02d/%02d/%02d  rv:0x%04X/0x%04X nclk:%5uHz tempo:0x%04lX(%7.3f) %u/%u beat:%02u bar:%04u",
                active_count,
                attack_count,
                decay_count,
                sustain_count,
                release_count,
                core_state->reverb_l,
                core_state->reverb_r,
                noise_hz,
                (unsigned long)akao->tempo,
                psf1_tempo_bpm(
                    akao->tempo, akao->early_format, akao->driver_type),
                akao->beats_per_measure,
                denominator,
                (unsigned)akao->current_beat + 1u,
                akao->measure);
        }
    } else {
        snprintf(line, line_size,
            "active:%02d/24  ADSR:%02d/%02d/%02d/%02d  rv:0x%04X/0x%04X nclk:%5uHz",
            active_count,
            attack_count,
            decay_count,
            sustain_count,
            release_count,
            core_state->reverb_l,
            core_state->reverb_r,
            noise_hz);
    }
}

static void active_header_logical_point(
    HWND hwnd,
    const PlayerState *state,
    int *x,
    int *y)
{
    PlayerScaleLayout content_layout;
    PlayerScaleLayout header_layout;
    int device_x;
    int device_y;

    if (hwnd == NULL || state == NULL || x == NULL || y == NULL ||
        !IsZoomed(hwnd) || state->psf_version != 0x01u) {
        return;
    }
    get_player_content_scale_layout(hwnd, state, &content_layout);
    get_fullscreen_reference_layout(hwnd, state, &header_layout);
    if (!content_layout.scaled || !header_layout.scaled ||
        content_layout.base_width <= 0 || content_layout.base_height <= 0 ||
        header_layout.dest_width <= 0 || header_layout.dest_height <= 0) {
        return;
    }

    device_x = content_layout.dest_x + (int)(((int64_t)*x *
        content_layout.dest_width) / content_layout.base_width);
    device_y = content_layout.dest_y + (int)(((int64_t)*y *
        content_layout.dest_height) / content_layout.base_height);
    *x = (int)(((int64_t)(device_x - header_layout.dest_x) *
        header_layout.base_width) / header_layout.dest_width);
    *y = (int)(((int64_t)(device_y - header_layout.dest_y) *
        header_layout.base_height) / header_layout.dest_height);
}

static int hit_test_core_reverb_value(HWND hwnd, PlayerState *state, int mouse_x, int mouse_y, unsigned *out_core, unsigned *out_side)
{
    Spu2LogLiveState live;
    Psf1AkaoPlaybackState akao;
    uint8_t psf_version;
    int char_width;
    int line_height;
    int panel_x;
    int header_x;
    int rel_x;
    unsigned core;
    char line[256];
    const char *marker;
    const char *slash;
    int left_index;
    int right_index;
    int marker_index;

    if (state == NULL || out_core == NULL || out_side == NULL) {
        return 0;
    }

    lock_state(state);
    live = state->live;
    akao = state->akao;
    psf_version = state->psf_version;
    unlock_state(state);

    active_header_logical_point(hwnd, state, &mouse_x, &mouse_y);

    line_height = get_player_display_line_height(hwnd);
    if (mouse_y < 2 + CONTROLS_HEIGHT || mouse_y >= 2 + CONTROLS_HEIGHT + line_height) {
        return 0;
    }

    if (psf_version == 0x01u) {
        core = 0;
        panel_x = player_core_panel_x(hwnd, state, psf_version, core);
    } else if (mouse_x >= CORE1_X) {
        core = 1;
        panel_x = CORE1_X;
    } else {
        core = 0;
        panel_x = CORE0_X;
    }
    if (mouse_x < panel_x || mouse_x >= panel_x + CORE_PANEL_WIDTH) {
        return 0;
    }

    format_core_header_line(
        &live, core, psf_version, psf_version != 0x01u,
        &akao, line, sizeof(line));
    marker = strstr(line, "rv:0x");
    if (marker == NULL) {
        return 0;
    }
    slash = strchr(marker, '/');
    if (slash == NULL) {
        return 0;
    }

    char_width = get_player_display_char_width(hwnd);
    header_x = panel_x;
    header_x += (CORE_PANEL_WIDTH -
        (int)strlen(line) * char_width) / 2;
    rel_x = (mouse_x - header_x) / char_width;
    marker_index = (int)(marker - line);
    left_index = (int)((marker - line) + 5);
    right_index = (int)((slash - line) + 3);
    if (rel_x >= left_index && rel_x < left_index + 4) {
        *out_core = core;
        *out_side = 0u;
        return 1;
    }
    if (rel_x >= right_index && rel_x < right_index + 4) {
        *out_core = core;
        *out_side = 1u;
        return 1;
    }
    if (rel_x >= marker_index && rel_x < right_index + 4) {
        *out_core = core;
        *out_side = 2u;
        return 1;
    }
    return 0;
}

static int hit_test_core_noise_clock_value(HWND hwnd, PlayerState *state, int mouse_x, int mouse_y, unsigned *out_core)
{
    Spu2LogLiveState live;
    Psf1AkaoPlaybackState akao;
    uint8_t psf_version;
    int char_width;
    int line_height;
    int panel_x;
    int header_x;
    int rel_x;
    unsigned core;
    char line[256];
    const char *marker;
    int value_index;

    if (state == NULL || out_core == NULL) {
        return 0;
    }

    lock_state(state);
    live = state->live;
    akao = state->akao;
    psf_version = state->psf_version;
    unlock_state(state);

    active_header_logical_point(hwnd, state, &mouse_x, &mouse_y);

    line_height = get_player_display_line_height(hwnd);
    if (mouse_y < 2 + CONTROLS_HEIGHT || mouse_y >= 2 + CONTROLS_HEIGHT + line_height) {
        return 0;
    }

    if (psf_version == 0x01u) {
        core = 0;
        panel_x = player_core_panel_x(hwnd, state, psf_version, core);
    } else if (mouse_x >= CORE1_X) {
        core = 1;
        panel_x = CORE1_X;
    } else {
        core = 0;
        panel_x = CORE0_X;
    }
    if (mouse_x < panel_x || mouse_x >= panel_x + CORE_PANEL_WIDTH) {
        return 0;
    }

    format_core_header_line(
        &live, core, psf_version, psf_version != 0x01u,
        &akao, line, sizeof(line));
    marker = strstr(line, "nclk:");
    if (marker == NULL) {
        return 0;
    }

    char_width = get_player_display_char_width(hwnd);
    header_x = panel_x;
    header_x += (CORE_PANEL_WIDTH -
        (int)strlen(line) * char_width) / 2;
    rel_x = (mouse_x - header_x) / char_width;
    value_index = (int)(marker - line) + (int)strlen("nclk:");
    if (rel_x >= value_index && rel_x < value_index + 7) {
        *out_core = core;
        return 1;
    }
    return 0;
}

static LPARAM player_logical_mouse_lparam(
    HWND hwnd,
    const PlayerState *state,
    LPARAM lparam)
{
    PlayerScaleLayout layout;
    int x = GET_X_LPARAM(lparam);
    int y = GET_Y_LPARAM(lparam);

    get_player_content_scale_layout(hwnd, state, &layout);
    if (!layout.scaled || layout.dest_width <= 0 || layout.dest_height <= 0 ||
        layout.base_width <= 0 || layout.base_height <= 0) {
        return lparam;
    }
    x = (int)(((int64_t)(x - layout.dest_x) * layout.base_width) /
        layout.dest_width);
    y = (int)(((int64_t)(y - layout.dest_y) * layout.base_height) /
        layout.dest_height);
    return MAKELPARAM((short)x, (short)y);
}

static LRESULT CALLBACK player_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    PlayerState *state = (PlayerState *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    if (state != NULL &&
        (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP ||
         msg == WM_LBUTTONDBLCLK || msg == WM_MOUSEMOVE ||
         msg == WM_RBUTTONDOWN ||
         msg == WM_RBUTTONUP || msg == WM_RBUTTONDBLCLK)) {
        lparam = player_logical_mouse_lparam(hwnd, state, lparam);
    }

    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCTA *create = (CREATESTRUCTA *)lparam;
        HMENU menu = CreateMenu();
        HMENU file_menu = CreatePopupMenu();
        HMENU settings_menu = CreatePopupMenu();
        HMENU env_color_menu = CreatePopupMenu();
        HMENU lr_color_menu = CreatePopupMenu();
        HMENU speed_menu = CreatePopupMenu();
        HMENU volume_menu = CreatePopupMenu();
        HMENU performance_menu = CreatePopupMenu();
        HMENU theme_menu = CreatePopupMenu();
        HMENU keyboard_menu = CreatePopupMenu();
        HMENU help_menu = CreatePopupMenu();
        unsigned speed_index;
        unsigned volume_percent;

        state = (PlayerState *)create->lpCreateParams;
        state->hwnd = hwnd;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)state);

        append_darkable_menu_item(file_menu, IDM_OPEN, "Open...");
        append_darkable_menu_item(file_menu, IDM_OPEN_FOLDER, "Open folder...");
        append_darkable_menu_separator(file_menu);
        append_darkable_menu_item(file_menu, IDM_PLAYLIST_SHOW, "Playlist...");
        append_darkable_menu_separator(file_menu);
        append_darkable_menu_item(file_menu, IDM_EXIT, "Exit");
        append_darkable_menu_bar_popup(menu, file_menu, "File");
        for (speed_index = 0; speed_index < SPEED_PRESET_COUNT; ++speed_index) {
            append_darkable_menu_item(
                speed_menu,
                IDM_SPEED_PRESET_FIRST + speed_index,
                g_speed_menu_labels[speed_index]);
        }
        append_darkable_menu_popup(settings_menu, speed_menu, "Speed");
        for (volume_percent = 0; volume_percent <= 200; volume_percent += 20) {
            AppendMenuA(volume_menu, MF_OWNERDRAW, IDM_VOLUME_0 + (volume_percent / 10), volume_menu_label_for_id(IDM_VOLUME_0 + (volume_percent / 10)));
        }
        append_darkable_menu_popup(settings_menu, volume_menu, "Volume");
        append_darkable_menu_item(settings_menu, IDM_TAG_FADE_ENABLED, "Fade at tag time");
        append_darkable_menu_separator(settings_menu);
        append_darkable_menu_item(settings_menu, IDM_TIMBRE_SCAN, "Samples Scan");
        append_darkable_menu_item(settings_menu, IDM_DEBUG_EDIT_CONTROLS, "Debug");
        append_darkable_menu_item(settings_menu, IDM_FRAME_ADVANCE, "Frame Advance");
        append_darkable_menu_separator(settings_menu);
        append_darkable_menu_item(settings_menu, IDM_PLAYBACK_ONLY, "Playback Only");
        append_darkable_menu_item(
            keyboard_menu, IDM_KEYBOARD_VALUES, "Values");
        append_darkable_menu_item(
            keyboard_menu, IDM_KEYBOARD_VALUES_AND_KEYS,
            "Values + Keyboard");
        append_darkable_menu_item(
            keyboard_menu, IDM_KEYBOARD_KEYS_ONLY, "Keyboard Only");
        state->keyboard_parent_menu = settings_menu;
        state->keyboard_submenu = keyboard_menu;
        state->keyboard_parent_position = (UINT)GetMenuItemCount(settings_menu);
        append_darkable_menu_popup(settings_menu, keyboard_menu, "Keyboard");
        append_darkable_menu_item(settings_menu, IDM_FONT_SELECT, "Font...");
        append_darkable_menu_item(theme_menu, IDM_THEME_SYSTEM, "System");
        append_darkable_menu_item(theme_menu, IDM_THEME_LIGHT, "Light");
        append_darkable_menu_item(theme_menu, IDM_THEME_DARK, "Dark");
        append_darkable_menu_popup(settings_menu, theme_menu, "Theme");
        append_darkable_menu_item(env_color_menu, IDM_ENV_COLOR_GREEN, "Green");
        append_darkable_menu_item(env_color_menu, IDM_ENV_COLOR_BLUE, "Blue");
        append_darkable_menu_item(env_color_menu, IDM_ENV_COLOR_CYAN, "Cyan");
        append_darkable_menu_item(env_color_menu, IDM_ENV_COLOR_AMBER, "Amber");
        append_darkable_menu_item(env_color_menu, IDM_ENV_COLOR_RED, "Red");
        append_darkable_menu_item(env_color_menu, IDM_ENV_COLOR_MAGENTA, "Magenta");
        append_darkable_menu_item(env_color_menu, IDM_ENV_COLOR_VIOLET, "Violet");
        append_darkable_menu_item(env_color_menu, IDM_ENV_COLOR_WHITE, "White");
        append_darkable_menu_separator(env_color_menu);
        append_darkable_menu_item(env_color_menu, IDM_ENV_COLOR_CUSTOM, "Custom...");
        append_darkable_menu_popup(settings_menu, env_color_menu, "Env color");
        append_darkable_menu_item(lr_color_menu, IDM_LR_COLOR_GREEN, "Green");
        append_darkable_menu_item(lr_color_menu, IDM_LR_COLOR_BLUE, "Blue");
        append_darkable_menu_item(lr_color_menu, IDM_LR_COLOR_CYAN, "Cyan");
        append_darkable_menu_item(lr_color_menu, IDM_LR_COLOR_AMBER, "Amber");
        append_darkable_menu_item(lr_color_menu, IDM_LR_COLOR_RED, "Red");
        append_darkable_menu_item(lr_color_menu, IDM_LR_COLOR_MAGENTA, "Magenta");
        append_darkable_menu_item(lr_color_menu, IDM_LR_COLOR_VIOLET, "Violet");
        append_darkable_menu_item(lr_color_menu, IDM_LR_COLOR_WHITE, "White");
        append_darkable_menu_separator(lr_color_menu);
        append_darkable_menu_item(lr_color_menu, IDM_LR_COLOR_CUSTOM, "Custom...");
        append_darkable_menu_popup(settings_menu, lr_color_menu, "L/R color");
        append_darkable_menu_separator(settings_menu);
        AppendMenuA(settings_menu, MF_OWNERDRAW | MF_CHECKED, IDM_MAIN_ENABLED, "Main");
        AppendMenuA(settings_menu, MF_OWNERDRAW | MF_CHECKED, IDM_REVERB_ENABLED, "Reverb");
        append_darkable_menu_item(performance_menu, IDM_PERF_LOW, "Low spec");
        append_darkable_menu_item(performance_menu, IDM_PERF_MIDDLE, "Middle spec");
        append_darkable_menu_item(performance_menu, IDM_PERF_HIGH, "High spec");
        append_darkable_menu_popup(settings_menu, performance_menu, "Performance");
        append_darkable_menu_bar_popup(menu, settings_menu, "Settings");
        append_darkable_menu_item(help_menu, IDM_ABOUT, "About");
        append_darkable_menu_bar_popup(menu, help_menu, "Help");
        SetMenu(hwnd, menu);
        apply_window_theme(hwnd, state);
        update_settings_menu_check(hwnd, state);

        state->open_button = CreateWindowA("BUTTON", "Open", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            6, 4, 54, 20, hwnd, (HMENU)(UINT_PTR)IDC_OPEN, NULL, NULL);
        state->play_button = CreateWindowA("BUTTON", "Play", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            64, 4, 54, 20, hwnd, (HMENU)(UINT_PTR)IDC_PLAY, NULL, NULL);
        state->pause_button = CreateWindowA("BUTTON", "Pause", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            122, 4, 58, 20, hwnd, (HMENU)(UINT_PTR)IDC_PAUSE, NULL, NULL);
        state->stop_button = CreateWindowA("BUTTON", "Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            184, 4, 54, 20, hwnd, (HMENU)(UINT_PTR)IDC_STOP, NULL, NULL);
        state->speed_slider = CreateWindowA(TRACKBAR_CLASSA, "",
            WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
            252, 1, 128, 26, hwnd, (HMENU)(UINT_PTR)IDC_SPEED, NULL, NULL);
        state->speed_label = CreateWindowA("STATIC", "Speed",
            WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_OWNERDRAW,
            386, 6, 94, 16, hwnd, (HMENU)(UINT_PTR)IDC_SPEED_LABEL, NULL, NULL);
        state->volume_label = CreateWindowA("STATIC", "Vol",
            WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_OWNERDRAW,
            484, 6, 68, 16, hwnd, (HMENU)(UINT_PTR)IDC_VOLUME_LABEL, NULL, NULL);
        state->main_check = CreateWindowA("BUTTON", "Main", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            556, 4, 58, 20, hwnd, (HMENU)(UINT_PTR)IDC_MAIN_CHECK, NULL, NULL);
        state->reverb_check = CreateWindowA("BUTTON", "Reverb", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            618, 4, 70, 20, hwnd, (HMENU)(UINT_PTR)IDC_REVERB_CHECK, NULL, NULL);
        state->time_label = CreateWindowA("STATIC", "00:00.0 / --:--.-",
            WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_CENTERIMAGE,
            694, 6, TIME_LABEL_MIN_WIDTH, 16,
            hwnd, (HMENU)(UINT_PTR)IDC_TIME_LABEL, NULL, NULL);
        state->voice_scrollbar = CreateWindowA("SCROLLBAR", "",
            WS_CHILD | SBS_VERT,
            0, 0, GetSystemMetrics(SM_CXVSCROLL), 100,
            hwnd, NULL, NULL, NULL);
        apply_voice_scrollbar_theme(state);
        if (state->speed_slider != NULL) {
            SendMessageA(state->speed_slider, TBM_SETRANGE, TRUE, MAKELPARAM(10, 200));
            SendMessageA(state->speed_slider, TBM_SETTICFREQ, 10, 0);
            SendMessageA(state->speed_slider, TBM_SETLINESIZE, 0, 10);
            SendMessageA(state->speed_slider, TBM_SETPAGESIZE, 0, 10);
            SendMessageA(state->speed_slider, TBM_SETPOS, TRUE, state->speed_percent);
        }
        set_speed_percent(hwnd, state, state->speed_percent);
        set_volume_percent(hwnd, state, state->volume_percent, 0);
        apply_ui_font_to_window(hwnd, state->ui_font);
        update_main_reverb_controls(state);
        update_time_label(state);
        DragAcceptFiles(hwnd, TRUE);

        update_scrollbar(hwnd, state);
        restart_display_timer(hwnd, state);
        return 0;
    }
    case WM_TIMER:
        if (state != NULL && wparam == PLAYER_CLICK_TIMER_ID) {
            update_pending_numeric_click(hwnd, state);
            return 0;
        }
        if (state != NULL && wparam == PLAYER_PLAYLIST_SWITCH_TIMER_ID) {
            playlist_process_pending_play(hwnd, state);
            return 0;
        }
        if (state != NULL && wparam == PLAYER_DIRECT_RETRY_TIMER_ID) {
            process_direct_start_retry(hwnd, state);
            return 0;
        }
        if (state != NULL && wparam == PLAYER_TIMER_ID) {
            if (state->playlist_play_in_progress) {
                playlist_process_pending_play(hwnd, state);
            }
            render_playback_tick(hwnd, state);
        }
        return 0;
    case PLAYER_WM_WORKER_UPDATE:
        if (state != NULL && state->timbre_hwnd != NULL) {
            timbre_refresh_listbox(state);
        }
        if (state != NULL && state->direct_start_retry_armed &&
            !get_playing(state)) {
            SetTimer(hwnd, PLAYER_DIRECT_RETRY_TIMER_ID,
                PLAYER_DIRECT_RETRY_TIMER_MS, NULL);
        }
        return 0;
    case PLAYER_WM_AUDIO_STARTING:
        if (state != NULL && state->playing && state->audio_started &&
            state->wave == (HWAVEOUT)(UINT_PTR)lparam) {
            RECT header_rect;
            int line_height = state->meter_line_height > 0 ?
                state->meter_line_height : 16;

            GetClientRect(hwnd, &header_rect);
            header_rect.top = CONTROLS_HEIGHT;
            header_rect.bottom = 2 + CONTROLS_HEIGHT + line_height * 2;
            InvalidateRect(hwnd, IsZoomed(hwnd) ? NULL : &header_rect, FALSE);
            UpdateWindow(hwnd);
            cancel_direct_start_retry(hwnd, state);
        }
        return 0;
    case PLAYER_WM_AUDIO_BUFFER_DONE:
        if (state != NULL && state->playing && state->audio_started &&
            state->wave == (HWAVEOUT)(UINT_PTR)lparam &&
            sync_audible_display_after_sequence(state, (uint64_t)wparam)) {
            RECT header_rect;
            int line_height = state->meter_line_height > 0 ?
                state->meter_line_height : 16;

            GetClientRect(hwnd, &header_rect);
            header_rect.top = CONTROLS_HEIGHT;
            header_rect.bottom = 2 + CONTROLS_HEIGHT + line_height * 2;
            InvalidateRect(hwnd, IsZoomed(hwnd) ? NULL : &header_rect, FALSE);
            UpdateWindow(hwnd);
        }
        return 0;
    case PLAYER_WM_PLAYBACK_ENDED:
        if (state != NULL &&
            (uint32_t)wparam == state->playback_generation) {
            if (state->playback_from_playlist && state->playlist_count > 0) {
                int next_index = playlist_automatic_next_index(state);
                if (next_index >= 0) {
                    playlist_play_index(hwnd, state, next_index);
                    return 0;
                }
            }
            stop_playback(state);
            reset_stopped_display(state);
            set_status(state, "Stopped: tag fade complete");
            update_settings_menu_check(hwnd, state);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    case PLAYER_WM_PLAYLIST_PLAY:
        playlist_process_pending_play(hwnd, state);
        return 0;
    case WM_MEASUREITEM:
        if (measure_owner_menu_item(lparam)) {
            return TRUE;
        }
        break;
    case WM_DRAWITEM:
        if (draw_owner_menu_item(lparam, state)) {
            return TRUE;
        }
        if (draw_header_percent_label(lparam, state)) {
            return TRUE;
        }
        if (draw_owner_button(lparam, state)) {
            return TRUE;
        }
        break;
    case WM_NOTIFY:
        if (state != NULL && ((NMHDR *)lparam)->hwndFrom == state->speed_slider) {
            return draw_speed_slider_custom((NMHDR *)lparam, state);
        }
        break;
    case WM_DROPFILES: {
        HDROP drop = (HDROP)wparam;
        UINT count = DragQueryFileW(drop, 0xFFFFFFFFu, NULL, 0);
        UINT i;

        if (state != NULL) {
            for (i = 0; i < count; ++i) {
                wchar_t path_w[MAX_PATH];
                char path[MAX_PATH];
                path[0] = '\0';
                path_w[0] = L'\0';
                if (DragQueryFileW(drop, i, path_w, MAX_PATH) != 0 &&
                    wide_path_to_runtime_path(path_w, path, sizeof(path))) {
                    if (is_directory_path(path)) {
                        remember_playlist_folder(state, path);
                        if (add_music_source_to_playlist(hwnd, state, path,
                                !state->playlist_append_mode && state->playlist_count > 0,
                                NULL, 0)) {
                            show_playlist_window(hwnd, state);
                        }
                        break;
                    } else if (is_archive_path(path)) {
                        remember_playlist_file(state, path);
                        if (archive_add_to_playlist(hwnd, state, path,
                                !state->playlist_append_mode && state->playlist_count > 0,
                                NULL, 0)) {
                            show_playlist_window(hwnd, state);
                            break;
                        }
                    } else if (is_psf_music_path(path)) {
                        start_direct_playback(hwnd, state, path);
                        break;
                    }
                }
            }
        }
        DragFinish(drop);
        return 0;
    }
    case WM_COMMAND:
        if (state != NULL && LOWORD(wparam) == IDC_SPEED_LABEL && HIWORD(wparam) == STN_DBLCLK) {
            set_speed_percent(hwnd, state, 100);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_VOLUME_LABEL && HIWORD(wparam) == STN_DBLCLK) {
            set_volume_percent(hwnd, state, 100, 1);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_TIME_LABEL && HIWORD(wparam) == STN_DBLCLK) {
            input_seek_time(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_OPEN) {
            open_direct_file_dialog(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_PLAY) {
            if (state->core != NULL && get_paused(state)) {
                set_paused(state, 0);
                set_status(state, "Playing direct");
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (state->input_path[0] != '\0') {
                start_direct_playback(hwnd, state, state->input_path);
            } else {
                open_direct_file_dialog(hwnd, state);
            }
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_PAUSE) {
            toggle_pause_playback(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_STOP) {
            cancel_direct_start_retry(hwnd, state);
            playlist_cancel_pending_play(hwnd, state);
            stop_playback(state);
            reset_stopped_display(state);
            set_status(state, "Stopped");
            update_settings_menu_check(hwnd, state);
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_MAIN_CHECK) {
            set_main_enabled(hwnd, state, SendMessageA(state->main_check, BM_GETCHECK, 0, 0) == BST_CHECKED);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_REVERB_CHECK) {
            set_reverb_enabled(hwnd, state, SendMessageA(state->reverb_check, BM_GETCHECK, 0, 0) == BST_CHECKED);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_OPEN) {
            open_direct_file_dialog(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_OPEN_FOLDER) {
            show_playlist_window(hwnd, state);
            playlist_add_folder_dialog(
                state->playlist_hwnd != NULL ? state->playlist_hwnd : hwnd,
                state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_PLAYLIST_SHOW) {
            show_playlist_window(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_PLAYLIST_SAVE) {
            playlist_save_to_file(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_PLAYLIST_LOAD) {
            playlist_load_from_file(hwnd, state);
            show_playlist_window(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_PLAYLIST_PREV) {
            playlist_play_next(hwnd, state, -1);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDC_PLAYLIST_NEXT) {
            playlist_play_next(hwnd, state, 1);
            return 0;
        }
        if (LOWORD(wparam) == IDM_EXIT) {
            DestroyWindow(hwnd);
            return 0;
        }
        if (LOWORD(wparam) == IDM_ABOUT) {
            show_about_dialog(hwnd);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_REVERB_ENABLED) {
            toggle_reverb_enabled(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_MAIN_ENABLED) {
            toggle_main_enabled(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_DEBUG_EDIT_CONTROLS) {
            toggle_debug_edit_controls(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_FRAME_ADVANCE) {
            toggle_frame_advance_mode(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_PLAYBACK_ONLY) {
            toggle_playback_only_mode(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_TAG_FADE_ENABLED) {
            set_tag_fade_enabled(hwnd, state, !state->tag_fade_enabled);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_TIMBRE_SCAN) {
            toggle_timbre_scan_mode(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_KEYBOARD_VALUES) {
            set_psf1_display_mode(hwnd, state, PSF1_DISPLAY_VALUES);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) ==
                IDM_KEYBOARD_VALUES_AND_KEYS) {
            set_psf1_display_mode(
                hwnd, state, PSF1_DISPLAY_VALUES_KEYBOARD);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_KEYBOARD_KEYS_ONLY) {
            set_psf1_display_mode(hwnd, state, PSF1_DISPLAY_KEYBOARD_ONLY);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_FONT_SELECT) {
            choose_ui_font(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_THEME_SYSTEM) {
            set_theme_mode(hwnd, state, THEME_SYSTEM);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_THEME_LIGHT) {
            set_theme_mode(hwnd, state, THEME_LIGHT);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_THEME_DARK) {
            set_theme_mode(hwnd, state, THEME_DARK);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) >= IDM_ENV_COLOR_GREEN && LOWORD(wparam) <= IDM_ENV_COLOR_WHITE) {
            set_env_color_index(hwnd, state, (int)(LOWORD(wparam) - IDM_ENV_COLOR_GREEN));
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_ENV_COLOR_CUSTOM) {
            set_env_custom_color(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) >= IDM_LR_COLOR_GREEN && LOWORD(wparam) <= IDM_LR_COLOR_WHITE) {
            set_lr_color_index(hwnd, state, (int)(LOWORD(wparam) - IDM_LR_COLOR_GREEN));
            return 0;
        }
        if (state != NULL && LOWORD(wparam) == IDM_LR_COLOR_CUSTOM) {
            set_lr_custom_color(hwnd, state);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) >= IDM_SPEED_PRESET_FIRST &&
            LOWORD(wparam) <= IDM_SPEED_PRESET_LAST) {
            unsigned speed_index = (unsigned)(LOWORD(wparam) - IDM_SPEED_PRESET_FIRST);
            set_speed_percent(hwnd, state, g_speed_menu_values[speed_index]);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) >= IDM_VOLUME_0 && LOWORD(wparam) <= IDM_VOLUME_200) {
            set_volume_percent(hwnd, state, (int)(LOWORD(wparam) - IDM_VOLUME_0) * 10, 1);
            return 0;
        }
        if (state != NULL && LOWORD(wparam) >= IDM_PERF_LOW && LOWORD(wparam) <= IDM_PERF_HIGH) {
            set_performance_profile(hwnd, state, (int)(LOWORD(wparam) - IDM_PERF_LOW));
            return 0;
        }
        break;
    case WM_KEYDOWN:
        if (state != NULL && preview_keyboard_is_active(state)) {
            return 0;
        }
        if (state != NULL && wparam == VK_TAB) {
            set_tab_speed_active(hwnd, state, 1);
            return 0;
        }
        if (state != NULL && wparam == 'P') {
            toggle_pause_playback(hwnd, state);
            return 0;
        }
        if (state != NULL && wparam == '0') {
            set_all_voice_mutes(hwnd, state, 0);
            return 0;
        }
        if (state != NULL && wparam == '1') {
            set_all_voice_mutes(hwnd, state, 1);
            return 0;
        }
        if (state != NULL &&
            (wparam == VK_UP || wparam == VK_DOWN) &&
            player_voice_scroll_enabled(hwnd, state)) {
            int line_height = state->meter_line_height > 0 ?
                state->meter_line_height : get_player_display_line_height(hwnd);
            int step = line_height + PSF1_TRACK_KEYBOARD_HEIGHT;

            adjust_player_voice_scroll(hwnd, state,
                wparam == VK_UP ? -step : step);
            return 0;
        }
        if (state != NULL && wparam == VK_LEFT) {
            if (state->frame_advance) {
                return 0;
            }
            seek_relative_seconds(hwnd, state, -5);
            return 0;
        }
        if (state != NULL && wparam == VK_RIGHT) {
            if (state->frame_advance) {
                frame_advance_tick(hwnd, state, 1);
                return 0;
            }
            seek_relative_seconds(hwnd, state, 5);
            return 0;
        }
        if (state != NULL && wparam == 'M') {
            toggle_main_enabled(hwnd, state);
            return 0;
        }
        if (state != NULL && wparam == 'R') {
            toggle_reverb_enabled(hwnd, state);
            return 0;
        }
        if (state != NULL && wparam == 'V') {
            toggle_playback_only_mode(hwnd, state);
            return 0;
        }
        if (state != NULL && wparam == 'K') {
            if ((lparam & (1l << 30)) == 0) {
                cycle_psf1_display_mode(hwnd, state);
            }
            return 0;
        }
        if (state != NULL && wparam == 'F') {
            toggle_frame_advance_mode(hwnd, state);
            return 0;
        }
        if (state != NULL && wparam == VK_ADD) {
            adjust_volume_percent(hwnd, state, 10);
            return 0;
        }
        if (state != NULL && wparam == VK_SUBTRACT) {
            adjust_volume_percent(hwnd, state, -10);
            return 0;
        }
        if (state != NULL && (wparam == VK_OEM_PLUS || wparam == 0xBB)) {
            set_speed_percent(hwnd, state, get_speed_percent(state) + 10);
            return 0;
        }
        if (state != NULL && (wparam == VK_OEM_MINUS || wparam == 0xBD)) {
            set_speed_percent(hwnd, state, get_speed_percent(state) - 10);
            return 0;
        }
        return 0;
    case WM_KEYUP:
        if (state != NULL && wparam == VK_TAB) {
            set_tab_speed_active(hwnd, state, 0);
            return 0;
        }
        break;
    case WM_LBUTTONDOWN:
        if (state != NULL) {
            unsigned core;
            unsigned voice;
            unsigned column;
            unsigned side;
            if (hit_test_voice_env_column(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core, &voice)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                toggle_voice_adsr_force(hwnd, state, core, voice);
                return 0;
            }
            if (hit_test_voice_text_column(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core, &voice, &column)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                if (column == VOICE_TEXT_COLUMN_ADSR_STATE) {
                    toggle_voice_adsr_force(hwnd, state, core, voice);
                } else if (column == VOICE_TEXT_COLUMN_PITCH) {
                    toggle_voice_pitch_lock(hwnd, state, core, voice);
                } else if (column == VOICE_TEXT_COLUMN_VOL_L) {
                    lock_voice_volume_current_value(hwnd, state, core, voice, 0u);
                } else if (column == VOICE_TEXT_COLUMN_VOL_R) {
                    lock_voice_volume_current_value(hwnd, state, core, voice, 1u);
                }
                return 0;
            }
            if (hit_test_core_reverb_value(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core, &side)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                start_pending_numeric_click(hwnd, state, core, 0u,
                    side == 2u ? VOICE_FLAG_COLUMN_REVERB_BOTH :
                    (side == 0u ? VOICE_FLAG_COLUMN_REVERB_L : VOICE_FLAG_COLUMN_REVERB_R), 1);
                return 0;
            }
            if (hit_test_core_noise_clock_value(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                start_pending_numeric_click(hwnd, state, core, 0u, VOICE_FLAG_COLUMN_NOISE_CLOCK, 1);
                return 0;
            }
            if (hit_test_voice_flag_column(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core, &voice, &column)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                if (column == VOICE_FLAG_COLUMN_NOISE) {
                    toggle_voice_effect_override(hwnd, state, core, voice, column);
                } else if (column == VOICE_FLAG_COLUMN_PMOD) {
                    toggle_voice_effect_override(hwnd, state, core, voice, column);
                } else if (column == VOICE_FLAG_COLUMN_REVERB) {
                    toggle_voice_effect_override(hwnd, state, core, voice, column);
                } else if (column == VOICE_FLAG_COLUMN_NOISE_CLOCK) {
                    start_pending_numeric_click(hwnd, state, core, voice, column, 1);
                } else if (column >= VOICE_FLAG_COLUMN_ADSR0 && column < VOICE_FLAG_COLUMN_ADSR0 + 5) {
                    start_pending_numeric_click(hwnd, state, core, voice, column, 1);
                }
                return 0;
            }
            if (hit_test_voice_row(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core, &voice)) {
                if ((GetKeyState(VK_SHIFT) & 0x8000) != 0 &&
                    (GetKeyState(VK_CONTROL) & 0x8000) != 0) {
                    toggle_timbre_solo_from_voice(hwnd, state, core, voice);
                } else if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) {
                    toggle_voice_solo(hwnd, state, core, voice);
                } else {
                    toggle_voice_mute(hwnd, state, core, voice);
                }
                return 0;
            }
        }
        break;
    case WM_LBUTTONUP:
        if (state != NULL && state->pending_click_active && state->pending_click_button == 1) {
            if (state->pending_click_repeating) {
                cancel_pending_numeric_click(hwnd, state);
            }
            return 0;
        }
        break;
    case WM_MOUSEWHEEL:
        if (state != NULL && player_voice_scroll_enabled(hwnd, state)) {
            int wheel_steps = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
            int line_height = state->meter_line_height > 0 ?
                state->meter_line_height : get_player_display_line_height(hwnd);
            int step = (line_height + PSF1_TRACK_KEYBOARD_HEIGHT) * 3;

            if (wheel_steps != 0) {
                adjust_player_voice_scroll(hwnd, state,
                    -wheel_steps * step);
            }
            return 0;
        }
        break;
    case WM_LBUTTONDBLCLK:
        if (state != NULL) {
            unsigned core;
            unsigned voice;
            unsigned column;
            unsigned side;
            if (hit_test_voice_text_column(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core, &voice, &column)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                cancel_pending_numeric_click(hwnd, state);
                if (column == VOICE_TEXT_COLUMN_PITCH) {
                    input_voice_pitch_value(hwnd, state, core, voice);
                } else if (column == VOICE_TEXT_COLUMN_VOL_L) {
                    input_voice_volume_value(hwnd, state, core, voice, 0u);
                } else if (column == VOICE_TEXT_COLUMN_VOL_R) {
                    input_voice_volume_value(hwnd, state, core, voice, 1u);
                }
                return 0;
            }
            if (hit_test_core_reverb_value(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core, &side)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                cancel_pending_numeric_click(hwnd, state);
                if (side == 2u) {
                    input_reverb_both_values(hwnd, state, core);
                } else {
                    input_reverb_value(hwnd, state, core, side);
                }
                return 0;
            }
            if (hit_test_core_noise_clock_value(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                cancel_pending_numeric_click(hwnd, state);
                input_noise_clock_value(hwnd, state, core);
                return 0;
            }
            if (hit_test_voice_flag_column(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core, &voice, &column)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                cancel_pending_numeric_click(hwnd, state);
                if (column == VOICE_FLAG_COLUMN_NOISE_CLOCK) {
                    input_noise_clock_value(hwnd, state, core);
                    return 0;
                }
                if (column >= VOICE_FLAG_COLUMN_ADSR0 && column < VOICE_FLAG_COLUMN_ADSR0 + 5) {
                    input_voice_adsr_value(hwnd, state, core, voice, column - VOICE_FLAG_COLUMN_ADSR0);
                    return 0;
                }
            }
        }
        break;
    case WM_RBUTTONDOWN:
        if (state != NULL) {
            unsigned core;
            unsigned voice;
            unsigned column;
            unsigned side;
            if (hit_test_core_reverb_value(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core, &side)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                start_pending_numeric_click(hwnd, state, core, 0u,
                    side == 2u ? VOICE_FLAG_COLUMN_REVERB_BOTH :
                    (side == 0u ? VOICE_FLAG_COLUMN_REVERB_L : VOICE_FLAG_COLUMN_REVERB_R), 2);
                return 0;
            }
            if (hit_test_core_noise_clock_value(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                start_pending_numeric_click(hwnd, state, core, 0u, VOICE_FLAG_COLUMN_NOISE_CLOCK, 2);
                return 0;
            }
            if (hit_test_voice_flag_column(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core, &voice, &column)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                if (column == VOICE_FLAG_COLUMN_NOISE) {
                    set_voice_noise_override(hwnd, state, core, voice, 0);
                } else if (column == VOICE_FLAG_COLUMN_PMOD) {
                    set_voice_pmod_override(hwnd, state, core, voice, 0);
                } else if (column == VOICE_FLAG_COLUMN_REVERB) {
                    set_voice_reverb_override(hwnd, state, core, voice, 0);
                } else if (column == VOICE_FLAG_COLUMN_NOISE_CLOCK) {
                    start_pending_numeric_click(hwnd, state, core, voice, column, 2);
                } else if (column >= VOICE_FLAG_COLUMN_ADSR0 && column < VOICE_FLAG_COLUMN_ADSR0 + 5) {
                    start_pending_numeric_click(hwnd, state, core, voice, column, 2);
                }
                return 0;
            }
            if (hit_test_voice_row(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core, &voice)) {
                set_all_voice_mutes(hwnd, state, 0);
                return 0;
            }
        }
        break;
    case WM_RBUTTONUP:
        if (state != NULL && state->pending_click_active && state->pending_click_button == 2) {
            if (state->pending_click_repeating) {
                cancel_pending_numeric_click(hwnd, state);
            }
            return 0;
        }
        break;
    case WM_RBUTTONDBLCLK:
        if (state != NULL) {
            unsigned core;
            unsigned voice;
            unsigned column;
            unsigned side;
            if (hit_test_voice_text_column(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core, &voice, &column)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                cancel_pending_numeric_click(hwnd, state);
                if (column == VOICE_TEXT_COLUMN_PITCH) {
                    reset_voice_pitch_value_to_default(hwnd, state, core, voice);
                } else if (column == VOICE_TEXT_COLUMN_VOL_L) {
                    unlock_voice_volume_value(hwnd, state, core, voice, 0u);
                } else if (column == VOICE_TEXT_COLUMN_VOL_R) {
                    unlock_voice_volume_value(hwnd, state, core, voice, 1u);
                }
                return 0;
            }
            if (hit_test_core_reverb_value(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core, &side)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                cancel_pending_numeric_click(hwnd, state);
                if (side == 2u) {
                    reset_reverb_both_values_to_default(hwnd, state, core);
                } else {
                    reset_reverb_value_to_default(hwnd, state, core, side);
                }
                return 0;
            }
            if (hit_test_core_noise_clock_value(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                cancel_pending_numeric_click(hwnd, state);
                reset_noise_clock_value_to_default(hwnd, state, core);
                return 0;
            }
            if (hit_test_voice_flag_column(hwnd, state, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &core, &voice, &column)) {
                if (!debug_edit_controls_active(state)) {
                    return 0;
                }
                cancel_pending_numeric_click(hwnd, state);
                if (column == VOICE_FLAG_COLUMN_NOISE_CLOCK) {
                    reset_noise_clock_value_to_default(hwnd, state, core);
                    return 0;
                }
                if (column >= VOICE_FLAG_COLUMN_ADSR0 && column < VOICE_FLAG_COLUMN_ADSR0 + 5) {
                    reset_voice_adsr_value_to_default(hwnd, state, core, voice, column - VOICE_FLAG_COLUMN_ADSR0);
                    return 0;
                }
            }
        }
        break;
    case WM_HSCROLL:
        if (state != NULL && (HWND)lparam == state->speed_slider) {
            int speed = (int)SendMessageA(state->speed_slider, TBM_GETPOS, 0, 0);
            set_speed_percent(hwnd, state, speed);
        }
        return 0;
    case WM_VSCROLL:
        if (state != NULL && (HWND)lparam == state->voice_scrollbar &&
            player_voice_scroll_enabled(hwnd, state)) {
            SCROLLINFO info;
            int line_height = state->meter_line_height > 0 ?
                state->meter_line_height : get_player_display_line_height(hwnd);
            int row_step = line_height + PSF1_TRACK_KEYBOARD_HEIGHT;
            int target = state->scroll_y;

            ZeroMemory(&info, sizeof(info));
            info.cbSize = sizeof(info);
            info.fMask = SIF_ALL;
            GetScrollInfo(state->voice_scrollbar, SB_CTL, &info);
            switch (LOWORD(wparam)) {
            case SB_LINEUP:
                target -= row_step;
                break;
            case SB_LINEDOWN:
                target += row_step;
                break;
            case SB_PAGEUP:
                target -= (int)info.nPage;
                break;
            case SB_PAGEDOWN:
                target += (int)info.nPage;
                break;
            case SB_THUMBPOSITION:
            case SB_THUMBTRACK:
                target = info.nTrackPos;
                break;
            case SB_TOP:
                target = 0;
                break;
            case SB_BOTTOM:
                target = player_voice_scroll_max(hwnd, state);
                break;
            default:
                return 0;
            }
            set_player_voice_scroll(hwnd, state, target);
            return 0;
        }
        break;
    case WM_ENTERSIZEMOVE:
        if (state != NULL && !IsZoomed(hwnd) && !IsIconic(hwnd)) {
            RECT window_rect;
            POINT cursor;

            state->window_move_drag_active = 0;
            if (GetWindowRect(hwnd, &window_rect) && GetCursorPos(&cursor)) {
                state->window_move_drag_active = 1;
                state->window_move_cursor_offset_x = cursor.x - window_rect.left;
                state->window_move_cursor_offset_y = cursor.y - window_rect.top;
                state->window_move_width = window_rect.right - window_rect.left;
                state->window_move_height = window_rect.bottom - window_rect.top;
            }
        }
        break;
    case WM_MOVING:
        if (state != NULL && !IsZoomed(hwnd) && !IsIconic(hwnd)) {
            snap_moving_window_to_monitor(state, (RECT *)lparam);
            return TRUE;
        }
        break;
    case WM_EXITSIZEMOVE:
        if (state != NULL) {
            state->window_move_drag_active = 0;
        }
        break;
    case WM_SIZE:
        if (wparam == SIZE_MAXIMIZED) {
            if (!g_main_window_was_maximized) {
                WINDOWPLACEMENT placement;

                ZeroMemory(&placement, sizeof(placement));
                placement.length = sizeof(placement);
                if (GetWindowPlacement(hwnd, &placement)) {
                    g_main_restore_x = placement.rcNormalPosition.left;
                    g_main_restore_y = placement.rcNormalPosition.top;
                    g_main_restore_position_valid = 1;
                }
            }
            g_main_window_was_maximized = 1;
            layout_player_controls(hwnd, state);
        } else if (wparam == SIZE_RESTORED && g_main_window_was_maximized) {
            g_main_window_was_maximized = 0;
            g_main_restore_position_active = 1;
            apply_psf_window_mode(hwnd, state,
                state != NULL ? state->psf_version : load_last_psf_version());
            g_main_restore_position_active = 0;
            g_main_restore_position_valid = 0;
        } else if (wparam == SIZE_RESTORED) {
            update_player_base_client_metrics(hwnd);
            layout_player_controls(hwnd, state);
        }
        update_scrollbar(hwnd, state);
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    case WM_SETTINGCHANGE:
        if (state != NULL && normalize_theme_mode(state->theme_mode) == THEME_SYSTEM) {
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;
    case WM_GETMINMAXINFO: {
        MINMAXINFO *minmax = (MINMAXINFO *)lparam;
        int fixed_width = g_window_fixed_width > 0 ?
            g_window_fixed_width : PLAYER_DEFAULT_WIDTH;
        int fixed_height = g_window_fixed_height > 0 ?
            g_window_fixed_height : PLAYER_DEFAULT_HEIGHT;
        minmax->ptMinTrackSize.x = fixed_width;
        minmax->ptMinTrackSize.y = fixed_height;
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        RECT rect;
        HDC hdc = BeginPaint(hwnd, &ps);

        if (state != NULL) {
            int gauges_only = 0;
            int line_height = state->meter_line_height > 0 ? state->meter_line_height : 16;
            int voice_top = 2 + CONTROLS_HEIGHT + (line_height * 2);

            GetClientRect(hwnd, &rect);
            if (state->playback_only) {
                state->gauge_only_paint_pending = 0;
                fill_player_background(hdc, &ps.rcPaint, state);
                EndPaint(hwnd, &ps);
                return 0;
            }
            {
                PlayerScaleLayout layout;

                get_player_content_scale_layout(hwnd, state, &layout);
                if (layout.scaled && ensure_player_backbuffer(
                        state, hdc,
                        rect.right - rect.left,
                        rect.bottom - rect.top)) {
                    int saved_screen_dc;
                    int saved_buffer_dc;

                    saved_screen_dc = SaveDC(hdc);
                    saved_buffer_dc = SaveDC(state->paint_dc);
                    SelectClipRgn(hdc, NULL);
                    SelectClipRgn(state->paint_dc, NULL);
                    state->gauge_only_paint_pending = 0;
                    SetMapMode(state->paint_dc, MM_TEXT);
                    SetViewportOrgEx(state->paint_dc, 0, 0, NULL);
                    SetWindowOrgEx(state->paint_dc, 0, 0, NULL);
                    fill_player_background(state->paint_dc, &rect, state);
                    SetMapMode(state->paint_dc, MM_ANISOTROPIC);
                    SetWindowExtEx(state->paint_dc,
                        layout.base_width, layout.base_height, NULL);
                    SetViewportExtEx(state->paint_dc,
                        layout.dest_width, layout.dest_height, NULL);
                    SetViewportOrgEx(state->paint_dc,
                        layout.dest_x, layout.dest_y, NULL);
                    paint_player(hwnd, state->paint_dc, state, 0);
                    state->paint_frame_initialized = 1;
                    if (saved_buffer_dc != 0) {
                        RestoreDC(state->paint_dc, saved_buffer_dc);
                    }
                    BitBlt(hdc,
                        0,
                        0,
                        rect.right - rect.left,
                        rect.bottom - rect.top,
                        state->paint_dc,
                        0,
                        0,
                        SRCCOPY);
                    if (saved_screen_dc != 0) {
                        RestoreDC(hdc, saved_screen_dc);
                    }
                    EndPaint(hwnd, &ps);
                    return 0;
                }
            }
            if (ensure_player_backbuffer(
                    state, hdc, rect.right - rect.left, rect.bottom - rect.top)) {
                int saved_dc = SaveDC(state->paint_dc);

                gauges_only = state->gauge_only_paint_pending &&
                    state->paint_frame_initialized &&
                    ps.rcPaint.left >= player_core_panel_x(
                        hwnd, state, state->psf_version, 0u) + COL_ENV_BAR_X &&
                    ps.rcPaint.top >= voice_top;
                state->gauge_only_paint_pending = 0;
                if (saved_dc != 0) {
                    IntersectClipRect(state->paint_dc,
                        ps.rcPaint.left,
                        ps.rcPaint.top,
                        ps.rcPaint.right,
                        ps.rcPaint.bottom);
                }
                if (!gauges_only) {
                    fill_player_background(state->paint_dc, &ps.rcPaint, state);
                }
                paint_player(hwnd, state->paint_dc, state, gauges_only);
                if (!gauges_only) {
                    state->paint_frame_initialized = 1;
                }
                if (saved_dc != 0) {
                    RestoreDC(state->paint_dc, saved_dc);
                }
                BitBlt(hdc,
                    ps.rcPaint.left,
                    ps.rcPaint.top,
                    ps.rcPaint.right - ps.rcPaint.left,
                    ps.rcPaint.bottom - ps.rcPaint.top,
                    state->paint_dc,
                    ps.rcPaint.left,
                    ps.rcPaint.top,
                    SRCCOPY);
            } else {
                state->gauge_only_paint_pending = 0;
                fill_player_background(hdc, &ps.rcPaint, state);
                paint_player(hwnd, hdc, state, 0);
            }
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
        if (state != NULL && is_dark_theme_active(state)) {
            SetTextColor((HDC)wparam, player_text_color(state));
            SetBkColor((HDC)wparam, player_background_color(state));
            return (LRESULT)GetStockObject(BLACK_BRUSH);
        }
        break;
    case WM_CLOSE:
        if (state != NULL) {
            cancel_direct_start_retry(hwnd, state);
            save_window_bounds(hwnd, state);
            stop_playback(state);
            if (state->playlist_hwnd != NULL) {
                DestroyWindow(state->playlist_hwnd);
                state->playlist_hwnd = NULL;
                state->playlist_listbox = NULL;
            }
            if (state->timbre_hwnd != NULL) {
                DestroyWindow(state->timbre_hwnd);
                state->timbre_hwnd = NULL;
                state->timbre_listbox = NULL;
            }
            if (state->preview_hwnd != NULL) {
                DestroyWindow(state->preview_hwnd);
                state->preview_hwnd = NULL;
            }
        }
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        DragAcceptFiles(hwnd, FALSE);
        KillTimer(hwnd, PLAYER_CLICK_TIMER_ID);
        KillTimer(hwnd, PLAYER_PLAYLIST_SWITCH_TIMER_ID);
        KillTimer(hwnd, PLAYER_DIRECT_RETRY_TIMER_ID);
        if (state != NULL) {
            stop_playback(state);
            destroy_player_paint_resources(state);
            if (state->playlist_hwnd != NULL) {
                DestroyWindow(state->playlist_hwnd);
                state->playlist_hwnd = NULL;
                state->playlist_listbox = NULL;
            }
            if (state->timbre_hwnd != NULL) {
                DestroyWindow(state->timbre_hwnd);
                state->timbre_hwnd = NULL;
                state->timbre_listbox = NULL;
            }
            if (state->preview_hwnd != NULL) {
                DestroyWindow(state->preview_hwnd);
                state->preview_hwnd = NULL;
            }
            state->hwnd = NULL;
        }
        KillTimer(hwnd, PLAYER_TIMER_ID);
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }

    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int show_cmd)
{
    WNDCLASSA wc;
    HWND hwnd;
    MSG msg;
    PlayerState state;
    INITCOMMONCONTROLSEX icc;
    HANDLE instance_mutex;
    MMRESULT timer_period_result;
    int window_x;
    int window_y;
    int window_width;
    int window_height;
    int start_maximized;

    (void)prev_instance;

    instance_mutex = CreateMutexA(NULL, TRUE, "Local\\PsfSpuPlayerSingleInstance");
    if (instance_mutex == NULL) {
        return 1;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(instance_mutex);
        return 0;
    }

    g_text_log_enabled = load_text_log_enabled();
    SetUnhandledExceptionFilter(player_unhandled_exception_filter);
    player_log("WinMain begin version=%s cmd=%s", PSF2_PLAYER_VERSION_DISPLAY, cmd_line != NULL ? cmd_line : "(null)");
    player_log("WinMain module=%p", (void *)GetModuleHandleA(NULL));

    ZeroMemory(&icc, sizeof(icc));
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);
    GdiSetBatchLimit(1024u);

    ZeroMemory(&state, sizeof(state));
    InitializeCriticalSection(&state.lock);
    state.lock_ready = 1;
    state.hide_inactive = 0;
    state.main_enabled = 1;
    state.reverb_enabled = 1;
    state.text_log_enabled = g_text_log_enabled;
    state.debug_edit_controls = 0;
    state.frame_advance = 0;
    state.playback_only = 0;
    state.psf1_display_mode = load_psf1_display_mode();
    state.theme_mode = THEME_SYSTEM;
    state.playlist_topmost = load_playlist_topmost();
    state.playlist_append_mode = load_playlist_append_mode();
    state.psf_version = load_last_psf_version();
    state.speed_percent = 100;
    state.volume_percent = 100;
    state.ui_font_size = 9;
    state.performance_profile = 2;
    state.tag_fade_enabled = 0;
    state.playlist_random_mode = 0;
    state.playlist_single_track_mode = 0;
    state.playlist_random_state = (uint32_t)GetTickCount();
    state.playlist_current = -1;
    state.playlist_pending_play_index = -1;
    state.direct_start_retry_playlist_index = -1;
    state.preview_active_note = -1;
    state.env_color_index = 0;
    state.lr_color_index = 0;
    state.env_custom_color = RGB(80, 180, 95);
    state.lr_custom_color = RGB(80, 180, 95);
    init_custom_colors(&state);
    load_display_settings(&state);
    load_playlist_last_paths(&state);
    load_ui_font_settings(&state);
    rebuild_ui_font(&state);
    save_display_settings(&state);
    snprintf(state.status, sizeof(state.status), "Idle");
    reset_live_display(&state);
    psf2log_set_imported_main_enabled(state.main_enabled);
    psf2log_set_imported_reverb_enabled(state.reverb_enabled);
    psf2log_set_imported_text_log_enabled(state.text_log_enabled);
    apply_voice_reverb_masks(&state);
    apply_voice_noise_masks(&state);
    apply_voice_pmod_masks(&state);

    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = player_wnd_proc;
    wc.hInstance = instance;
    wc.lpszClassName = "Psf2Spu2PlayerWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIconA(instance, MAKEINTRESOURCEA(IDI_APP_ICON));

    if (!RegisterClassA(&wc)) {
        player_log("RegisterClass failed error=%lu", (unsigned long)GetLastError());
        CloseHandle(instance_mutex);
        return 1;
    }

    wc.lpfnWndProc = playlist_wnd_proc;
    wc.lpszClassName = "PsfSpuPlaylistWindow";
    wc.hIcon = LoadIconA(instance, MAKEINTRESOURCEA(IDI_APP_ICON));
    if (!RegisterClassA(&wc)) {
        player_log("RegisterClass playlist failed error=%lu", (unsigned long)GetLastError());
    }

    wc.lpfnWndProc = timbre_wnd_proc;
    wc.lpszClassName = "PsfSpuTimbreWindow";
    wc.hIcon = LoadIconA(instance, MAKEINTRESOURCEA(IDI_APP_ICON));
    if (!RegisterClassA(&wc)) {
        player_log("RegisterClass timbre failed error=%lu", (unsigned long)GetLastError());
    }

    wc.lpfnWndProc = preview_keyboard_wnd_proc;
    wc.lpszClassName = "PsfSpuPreviewKeyboardWindow";
    wc.hIcon = LoadIconA(instance, MAKEINTRESOURCEA(IDI_APP_ICON));
    if (!RegisterClassA(&wc)) {
        player_log("RegisterClass preview keyboard failed error=%lu", (unsigned long)GetLastError());
    }

    load_window_bounds(
        &state, &window_x, &window_y, &window_width, &window_height);
    start_maximized = load_window_maximized();
    g_window_fixed_width = window_width;
    g_window_fixed_height = window_height;
    timer_period_result = timeBeginPeriod(1u);

    hwnd = CreateWindowExA(
        0,
        "Psf2Spu2PlayerWindow",
        "PSF SPU Player " PSF2_PLAYER_VERSION_DISPLAY,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX |
            WS_MAXIMIZEBOX | WS_CLIPCHILDREN,
        window_x,
        window_y,
        window_width,
        window_height,
        NULL,
        NULL,
        instance,
        &state);
    if (hwnd == NULL) {
        player_log("CreateWindow failed error=%lu", (unsigned long)GetLastError());
        if (timer_period_result == TIMERR_NOERROR) {
            timeEndPeriod(1u);
        }
        CloseHandle(instance_mutex);
        return 1;
    }

    state.meter_line_height = get_player_display_line_height(hwnd);
    apply_psf_window_mode(hwnd, &state, state.psf_version);
    ShowWindow(hwnd, start_maximized ? SW_SHOWMAXIMIZED : show_cmd);
    UpdateWindow(hwnd);
    if (state.timbre_scan_enabled) {
        show_timbre_window(hwnd, &state);
    }

    if (cmd_line != NULL && cmd_line[0] != '\0') {
        char path[MAX_PATH];
        copy_first_command_arg(path, sizeof(path), cmd_line);
        if (path[0] != '\0') {
            player_log("WinMain autostart path=%s", path);
            if (is_directory_path(path) || is_archive_path(path)) {
                if (is_directory_path(path)) {
                    remember_playlist_folder(&state, path);
                } else {
                    remember_playlist_file(&state, path);
                }
                if (add_music_source_to_playlist(hwnd, &state, path, 0, NULL, 0)) {
                    show_playlist_window(hwnd, &state);
                }
            } else {
                start_direct_playback(hwnd, &state, path);
            }
        }
    }

    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_SYSKEYDOWN && msg.wParam == VK_RETURN &&
            ((HIWORD(msg.lParam) & KF_ALTDOWN) != 0 ||
             (GetKeyState(VK_MENU) & 0x8000) != 0)) {
            if (!preview_keyboard_is_active(&state)) {
                toggle_player_maximized(hwnd);
            }
            continue;
        }
        if (state.playlist_hwnd != NULL && state.playlist_listbox != NULL &&
            (msg.hwnd == state.playlist_hwnd || IsChild(state.playlist_hwnd, msg.hwnd)) &&
            msg.message == WM_KEYDOWN &&
            (msg.wParam == VK_UP || msg.wParam == VK_DOWN)) {
            SendMessageA(state.playlist_listbox, WM_KEYDOWN, msg.wParam, msg.lParam);
            continue;
        }
        if (state.playlist_hwnd != NULL &&
            (msg.hwnd == state.playlist_hwnd || IsChild(state.playlist_hwnd, msg.hwnd)) &&
            msg.message == WM_KEYDOWN &&
            (msg.wParam == VK_DELETE || msg.wParam == VK_RETURN)) {
            if (msg.wParam == VK_RETURN) {
                playlist_play_index(
                    state.playlist_hwnd,
                    &state,
                    playlist_selected_index(&state));
            } else {
                SendMessageA(state.playlist_hwnd, WM_KEYDOWN, msg.wParam, msg.lParam);
            }
            continue;
        }
        if (state.preview_hwnd != NULL &&
            (msg.hwnd == state.preview_hwnd || IsChild(state.preview_hwnd, msg.hwnd)) &&
            (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP)) {
            char class_name[24];

            class_name[0] = '\0';
            GetClassNameA(msg.hwnd, class_name, sizeof(class_name));
            if (lstrcmpiA(class_name, "Edit") == 0) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
                continue;
            }
            if (preview_pc_key_to_note(msg.wParam) >= 0) {
                SendMessageA(state.preview_hwnd, msg.message, msg.wParam, msg.lParam);
                continue;
            }
            if (msg.wParam == VK_LEFT || msg.wParam == VK_UP ||
                msg.wParam == VK_RIGHT || msg.wParam == VK_DOWN ||
                msg.wParam == VK_PRIOR || msg.wParam == VK_NEXT ||
                msg.wParam == VK_OEM_4 || msg.wParam == VK_OEM_6) {
                SendMessageA(state.preview_hwnd, msg.message, msg.wParam, msg.lParam);
                continue;
            }
        }
        if (state.preview_hwnd != NULL && IsWindowVisible(state.preview_hwnd) &&
            (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) &&
            (msg.wParam == VK_PRIOR || msg.wParam == VK_NEXT ||
             msg.wParam == VK_OEM_4 || msg.wParam == VK_OEM_6)) {
            SendMessageA(state.preview_hwnd, msg.message, msg.wParam, msg.lParam);
            continue;
        }
        if (state.timbre_listbox != NULL && msg.hwnd == state.timbre_listbox &&
            msg.message == WM_KEYDOWN &&
            (msg.wParam == VK_LEFT || msg.wParam == VK_RIGHT)) {
            SendMessageA(state.timbre_listbox,
                WM_KEYDOWN,
                msg.wParam,
                msg.lParam);
            continue;
        }
        if (state.preview_hwnd != NULL && state.timbre_listbox != NULL &&
            msg.hwnd == state.timbre_listbox &&
            (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) &&
            preview_pc_key_to_note(msg.wParam) >= 0) {
            SendMessageA(state.preview_hwnd, msg.message, msg.wParam, msg.lParam);
            continue;
        }
        if (preview_keyboard_is_active(&state) &&
            (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP)) {
            if (msg.message == WM_KEYUP && msg.wParam == VK_TAB) {
                set_tab_speed_active(hwnd, &state, 0);
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
            continue;
        }
        if ((msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) &&
            msg.wParam == VK_TAB) {
            SendMessageA(hwnd, msg.message, msg.wParam, msg.lParam);
            continue;
        }
        if (msg.message == WM_KEYDOWN &&
            (msg.wParam == VK_LEFT || msg.wParam == VK_RIGHT ||
             msg.wParam == VK_UP || msg.wParam == VK_DOWN ||
             msg.wParam == 'M' || msg.wParam == 'R' || msg.wParam == 'P' ||
             msg.wParam == 'V' || msg.wParam == 'K' || msg.wParam == 'F' ||
             msg.wParam == '0' || msg.wParam == '1' ||
             msg.wParam == VK_ADD || msg.wParam == VK_SUBTRACT ||
             msg.wParam == VK_OEM_PLUS || msg.wParam == VK_OEM_MINUS ||
             msg.wParam == 0xBB || msg.wParam == 0xBD)) {
            SendMessageA(hwnd, WM_KEYDOWN, msg.wParam, msg.lParam);
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    if (state.lock_ready) {
        lock_state(&state);
        preview_free_samples_locked(&state);
        unlock_state(&state);
        DeleteCriticalSection(&state.lock);
        state.lock_ready = 0;
    }
    if (state.maximized_ui_font != NULL) {
        DeleteObject(state.maximized_ui_font);
        state.maximized_ui_font = NULL;
    }
    if (state.ui_font != NULL) {
        DeleteObject(state.ui_font);
        state.ui_font = NULL;
        g_ui_font = NULL;
    }
    if (timer_period_result == TIMERR_NOERROR) {
        timeEndPeriod(1u);
    }

    CloseHandle(instance_mutex);
    player_log("WinMain end code=%d", (int)msg.wParam);
    ExitProcess((UINT)msg.wParam);
    return (int)msg.wParam;
}

#else

int main(void)
{
    return 0;
}

#endif

#ifndef SPU2LOG_AUDACIOUS_COMPAT_AUDIO_H
#define SPU2LOG_AUDACIOUS_COMPAT_AUDIO_H

#include <stdint.h>

static inline uint16_t spu2log_compat_bswap16(uint16_t value)
{
    return (uint16_t)((value >> 8) | (value << 8));
}

static inline uint32_t spu2log_compat_bswap32(uint32_t value)
{
    return ((value & 0x000000ffu) << 24) |
        ((value & 0x0000ff00u) << 8) |
        ((value & 0x00ff0000u) >> 8) |
        ((value & 0xff000000u) >> 24);
}

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define FROM_LE16(x) spu2log_compat_bswap16((uint16_t)(x))
#define FROM_LE32(x) spu2log_compat_bswap32((uint32_t)(x))
#else
#define FROM_LE16(x) ((uint16_t)(x))
#define FROM_LE32(x) ((uint32_t)(x))
#endif

#define FMT_S16_NE 0

#endif

#ifndef SPU2LOG_AUDACIOUS_COMPAT_AUDSTRINGS_H
#define SPU2LOG_AUDACIOUS_COMPAT_AUDSTRINGS_H

#include <ctype.h>
#include <string.h>

static inline int strcmp_nocase(const char *left, const char *right)
{
    unsigned char a;
    unsigned char b;

    if (left == 0 && right == 0) {
        return 0;
    }
    if (left == 0) {
        return -1;
    }
    if (right == 0) {
        return 1;
    }

    while (*left != '\0' && *right != '\0') {
        a = (unsigned char)tolower((unsigned char)*left);
        b = (unsigned char)tolower((unsigned char)*right);
        if (a != b) {
            return (int)a - (int)b;
        }
        ++left;
        ++right;
    }

    return (int)(unsigned char)*left - (int)(unsigned char)*right;
}

#endif

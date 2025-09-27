#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef enum {
    STDIN = 0,
    STDOUT,
    STDERR,
} FileDescriptor;

typedef struct ModeInfo {
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
} ModeInfo;

#endif
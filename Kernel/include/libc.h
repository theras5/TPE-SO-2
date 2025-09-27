#ifndef LIBC_H
#define LIBC_H

#include <stdint.h>

void * _memset(void * destination, int32_t character, uint64_t length);
void * _memcpy(void * destination, const void * source, uint64_t length);
void * _memmove(void * destination, const void * source, uint64_t length);

#endif
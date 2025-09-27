#ifndef EXCEPTION_DISPATCHER_H
#define EXCEPTION_DISPATCHER_H

#include "videoDriver.h"
#include "keyboardDriver.h"
#include <stdint.h>

#define ZERO_EXCEPTION_ID 0
#define INVALID_OPCODE 6


extern void _sti();
extern void dumpRegisters(void);

void exceptionDispatcher(uint64_t exception);

#endif // EXCEPTION_DISPATCHER_H
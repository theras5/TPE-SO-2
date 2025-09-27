#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include <stdint.h> 
#include <stdarg.h>
#include "time_display.h"
#include "time.h"
#include "keyboardDriver.h"
#include "interrupts.h"
#include "libasm.h"
#include "types.h"  

typedef unsigned long size_t;

#define SYS_READ                0
#define SYS_WRITE               1
#define SYS_REGS                2
#define SYS_GET_TIME            4
#define SYS_SET_CURSOR          5
#define SYS_CLEAR_SCREEN        6
#define SYS_SET_ZOOM            7
#define SYS_EXIT                8
#define SYS_GET_MODE_INFO       9
#define SYS_IS_KEY_DOWN         10
#define SYS_KEY_READY           12
#define SYS_PUT_PIXEL           15
#define SYS_DRAW_RECT           16
#define SYS_DRAW_CIRCLE         17
#define SYS_RESET_KBD_BUFFER    19
#define SYS_BEEP                20
#define SYS_ERR   ((uint64_t)-1)
#define SYS_GET_ZOOM            21

uint64_t sysCallDispatcher(uint64_t rax, ...);

void sys_write(FileDescriptor fd, const char *buf, size_t count);

uint64_t sys_read(FileDescriptor fd, char *buf, size_t count);

#endif
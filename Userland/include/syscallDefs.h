#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

// standard file descriptors
typedef enum {
    STDIN  = 0,
    STDOUT = 1,
    STDERR = 2
} FDS;

#define SYS_READ            0
#define SYS_WRITE           1
#define SYS_REGS            2
#define SYS_GET_TIME        4
#define SYS_SET_CURSOR      5
#define SYS_CLEAR_SCREEN    6
#define SYS_SET_ZOOM        7
#define SYS_EXIT            8
#define SYS_GET_MODE_INFO   9
#define SYS_IS_KEY_DOWN     10
#define SYS_CHAR_READY      12
#define SYS_READ_KEY        13
#define SYS_PUT_PIXEL       15
#define SYS_DRAW_RECT       16
#define SYS_DRAW_CIRCLE     17
#define SYS_DRAW_BITMAP     18
#define SYS_RESET_KBD_BUFFER   19
#define SYS_BEEP            20
#define SYS_GET_ZOOM        21


#endif // _SYSCALLS_H_

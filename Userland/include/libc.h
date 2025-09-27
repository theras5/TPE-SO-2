#ifndef _LIBC_H_
#define _LIBC_H_

#include <stdint.h>
#include "libasm.h"
#include "syscallDefs.h"
#include <stdint.h>


/* Time structure */
typedef struct {
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} Time;

typedef struct ModeInfo {
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
} ModeInfo;

/* ------------------------------------------------------------------------- */
/* Basic character I/O (built on sys_read/write)                             */
/* ------------------------------------------------------------------------- */
void     putChar(char c);
uint8_t  isCharReady(void);
char     getChar(void);
void     putString(const char *s);

/* ------------------------------------------------------------------------- */
/* String and memory routines                                                */
/* ------------------------------------------------------------------------- */
uint64_t   strlen(const char *s);
int      strcmp(const char *s1, const char *s2);
int      strncmp(const char *s1, const char *s2, uint64_t n);
int     get_zoom(void);
/* ------------------------------------------------------------------------- */
/* Numeric conversions                                                       */
/* ------------------------------------------------------------------------- */
int      itoa(int64_t value, char *buf, int base);
int      atoi(const char *s);

/* ------------------------------------------------------------------------- */
/* Timing and delays                                                         */
/* ------------------------------------------------------------------------- */
void     getTime(void);            // fetch system time 

void    wait_next_tick(void); /* block until next timer IRQ (60 fps target) */

/* ------------------------------------------------------------------------- */
/* Cursor control & screen clearing                                          */
/* ------------------------------------------------------------------------- */
void     set_cursor(uint32_t x, uint32_t y);
void     clear_screen(void);
void     set_zoom(int level);

/* ------------------------------------------------------------------------- */
/* Exception / halt helper                                                    */
/* ------------------------------------------------------------------------- */
void hltUntil_c();

/* ------------------------------------------------------------------------- */
/* CPU register inspection                                                   */
/* ------------------------------------------------------------------------- */

void      showRegisters(void);    /* print regs to stdout */ 

/* ------------------------------------------------------------------------- */
/* Game (Pongis)                                                             */
/* ------------------------------------------------------------------------- */
uint64_t get_mode_info(ModeInfo *mode);


/* ------------------------------------------------------------------------- */
/* Sound                                                                     */
/* ------------------------------------------------------------------------- */
void exception_sound();

void system_start_sound();
void draw_welcome();
void draw_smile(ModeInfo mode);

#endif /* _LIBC_H_ */

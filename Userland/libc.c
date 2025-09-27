#include "libc.h"

/* ------------------------------------------------------------------------- */
/* Basic character I/O (built on sys_read/write)                             */
/* ------------------------------------------------------------------------- */

void putChar(char c) {
    sys_call(SYS_WRITE, STDOUT, (uint64_t)&c, 1, 0, 0);
}

char getChar(void) {
    char c;
    sys_call((uint64_t) SYS_READ, (uint64_t) STDIN,(uint64_t) &c,(uint64_t) 1, 0, 0);
    return c;
}

uint8_t isCharReady(void) {
    return sys_call(SYS_CHAR_READY, 0, 0, 0, 0, 0);
}

void putString(const char *str) {
    sys_call(SYS_WRITE, STDOUT, (uint64_t)str, strlen(str), 0, 0);
}

/* ------------------------------------------------------------------------- */
/* String and memory routines                                                */
/* ------------------------------------------------------------------------- */
uint64_t strlen(const char *s) {
    uint64_t len = 0;
    while (s[len]) len++;
    return len;
}

// Compare two strings byte-wise
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, uint64_t n) {
    while (n-- && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    if (n == (uint64_t)-1) return 0; // If n was 0, they are equal
    return (unsigned char)*s1 - (unsigned char)*s2;
}


/* ------------------------------------------------------------------------- */
/* Numeric conversions                                                       */
/* ------------------------------------------------------------------------- */

// Convert a non‐negative value to a string in buf using the given base (2–36).q2   1
int itoa(int64_t value, char *buf, int base) {
    if (base < 2 || base > 36) {
        *buf = '\0';
        return 0;
    }

    char *p = buf;
    uint64_t u = (uint64_t)value;  // assume value >= 0

    // Generate digits in reverse order
    do {
        int digit = u % base;
        *p++ = (digit < 10 ? '0' + digit : 'a' + (digit - 10));
        u /= base;
    } while (u);

    int len = p - buf;
    *p = '\0';

    // Reverse the buffer in-place
    char *start = buf, *end = p - 1;
    while (start < end) {
        char tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }

    return len;
}

// Convert a string to int (base 10)
int atoi(const char *s) {
    int result = 0;
    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }
    return result;
}


/* ------------------------------------------------------------------------- */
/* Timing and delays                                                         */
/* ------------------------------------------------------------------------- */

void getTime() {
	sys_call(SYS_GET_TIME, 0, 0, 0, 0, 0);
}

void wait_next_tick(void) {
    _hlt(); // Halt the CPU until the next tick
}

/* ------------------------------------------------------------------------- */
/* Cursor control & screen clearing                                          */
/* ------------------------------------------------------------------------- */

void clear_screen(void) {
    sys_call(SYS_CLEAR_SCREEN, 0, 0, 0, 0, 0);
}

void set_cursor(uint32_t col, uint32_t row) {
    sys_call(SYS_SET_CURSOR, (uint64_t)col, (uint64_t)row, 0, 0, 0);
}

void set_zoom(int level) {
    sys_call(SYS_SET_ZOOM, (uint64_t)level, 0, 0, 0, 0);
}
int get_zoom(void) {
    return sys_call(SYS_GET_ZOOM, 0, 0, 0, 0, 0);
}
/* ------------------------------------------------------------------------- */
/* Exception / halt helper                                                    */
/* ------------------------------------------------------------------------- */

void hltUntil_c() {
    while (getChar() != 'c') {
        _hlt();
    }
    clear_screen();
}

/* ------------------------------------------------------------------------- */
/* CPU register inspection                                                   */
/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */
/* Game (Pongis)                                                             */
/* ------------------------------------------------------------------------- */


uint64_t get_mode_info(ModeInfo *mode) {
    return sys_call(SYS_GET_MODE_INFO, (uint64_t)mode, 0, 0, 0, 0);
}


void exception_sound() {
    uint64_t melody[] = { 523, 415};
    uint64_t duration[] = { 150, 300};

    for (int i = 0; i < 2; i++) {
        sys_call(SYS_BEEP, melody[i], duration[i], 0, 0, 0);
    }
}

void system_start_sound() {
    unsigned int melody[] = { 330, 392, 494, 659};
    unsigned int duration[] = { 200, 200, 200, 400};

    for (int i = 0; i < 4; i++) {
        sys_call(SYS_BEEP, melody[i], duration[i], 0, 0, 0);
    }
}




void draw_smile(ModeInfo mode) {
    int width = mode.width;
    int height = mode.height;


    int centerX = width / 2;
    int centerY = height / 2;
    int radius = height / 4; // proporcional a la altura

    // FACE
    sys_call(SYS_DRAW_CIRCLE, centerX, centerY, radius, 0xFF8700, 0); // Orange circle
    sys_call(SYS_DRAW_CIRCLE, centerX, centerY, radius - 15, 0xFFFF00, 0); // Yellow circle

    // EYES
    int eyeHeight = radius / 3;
    int eyeWidth = radius / 10;
    sys_call(SYS_DRAW_RECT, centerX - eyeWidth*3, centerY - eyeHeight, eyeWidth, eyeHeight, 0x000000); // Left eye
    sys_call(SYS_DRAW_RECT, centerX + eyeWidth*2, centerY - eyeHeight, eyeWidth, eyeHeight, 0x000000); // Right eye

    // MOUTH
    int mouthWidth = radius;
    int mouthHeight = radius / 15;
    sys_call(SYS_DRAW_RECT, centerX - mouthWidth/2, centerY + mouthHeight*5, mouthWidth, mouthHeight, 0x000000); // Mouth bar

    // Mouth corners
    sys_call(SYS_DRAW_RECT, centerX - mouthWidth/2 - mouthHeight*2, centerY + mouthHeight*5-mouthHeight*2, mouthHeight, mouthHeight, 0x000000); // Left corner 1
    sys_call(SYS_DRAW_RECT, centerX - mouthWidth/2 - mouthHeight, centerY + mouthHeight*5-mouthHeight, mouthHeight, mouthHeight, 0x000000); // Left corner 2
    sys_call(SYS_DRAW_RECT, centerX + mouthWidth/2 + mouthHeight, centerY + mouthHeight*5-mouthHeight*2, mouthHeight, mouthHeight, 0x000000); // Right corner 1
    sys_call(SYS_DRAW_RECT, centerX + mouthWidth/2, centerY + mouthHeight*5-mouthHeight, mouthHeight, mouthHeight, 0x000000); // Right corner 2

}

void draw_welcome(){
    ModeInfo mode;
    sys_call(SYS_GET_MODE_INFO, (uint64_t)&mode, 0, 0, 0, 0);

    int width = mode.width;
    int height = mode.height;

    int border_thick = 10;
    int border_height = 50;

    // BORDES PRINCIPALES
    sys_call(SYS_DRAW_RECT, 0, 0, width, border_height, 0x9300ff); // Top purple
    sys_call(SYS_DRAW_RECT, 0, height - border_height, width, border_height, 0x9300ff); // Bottom purple
    sys_call(SYS_DRAW_RECT, 0, border_height, border_thick, height - 2 * border_height, 0x9300ff); // Left purple
    sys_call(SYS_DRAW_RECT, width - border_thick, border_height, border_thick, height - 2 * border_height, 0x9300ff); // Right purple

    // BORDES AZULES
    sys_call(SYS_DRAW_RECT, border_thick, border_height, border_thick, height - 2 * border_height, 0x0020ff); // Left blue
    sys_call(SYS_DRAW_RECT, width - 2 * border_thick, border_height, border_thick, height - 2 * border_height, 0x0020ff); // Right blue
    sys_call(SYS_DRAW_RECT, border_thick, border_height - border_thick, width - 2 * border_thick, border_thick, 0x0020ff); // Top blue
    sys_call(SYS_DRAW_RECT, border_thick, height - border_height, width - 2 * border_thick, border_thick, 0x0020ff); // Bottom blue

    // DIBUJITOS DEL BORDE
    for(int i = border_thick; i < width - border_thick; i += 10){
        if(i % 20 == 0){
            sys_call(SYS_DRAW_RECT, i, border_height - border_thick, 10, 10, 0x0a00c7);//color azul
            sys_call(SYS_DRAW_RECT, i, height - border_height, 10, 10, 0x0a00c7);
        }
    }

    for(int i = 0; i < width; i += 64){
        if(i % 128 == 0){
            sys_call(SYS_DRAW_RECT, i, 0, 64, border_height - border_thick, 0xff00ff);//color
            sys_call(SYS_DRAW_RECT, i, height - border_height + border_thick, 64, border_height - border_thick, 0xff00ff);
        }
    }

    draw_smile(mode); // Draw smiley face in the center
    //message
    // TEXTOS
    set_zoom(3);
    set_cursor(width / 12, height / 14);
    putString("ARQUITECTURA DE COMPUTADORAS - ITBA");

    set_cursor(width / 6, height / 8);
    putString("Trabajo Practico Especial");

    set_zoom(2);
    set_cursor(width / 20, height * 0.82);
    putString("Creado por: Pablo Gorostiaga, Tiago Heras y Amador Vallejo\n");

    set_cursor(width / 20, height * 0.89);
    putString("Presione 'c' para continuar...");
}

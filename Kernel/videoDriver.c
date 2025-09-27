
#include <videoDriver.h>
#include "sysCallDispatcher.h"

struct vbe_mode_info_structure {
    uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
    uint8_t window_a;			// deprecated
    uint8_t window_b;			// deprecated
    uint16_t granularity;		// deprecated; used while calculating bank numbers
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
    uint16_t pitch;			// number of bytes per horizontal line
    uint16_t width;			// width in pixels
    uint16_t height;		// height in pixels
    uint8_t w_char;			// unused...
    uint8_t y_char;			// ...
    uint8_t planes;
    uint8_t bpp;			// bits per pixel in this mode
    uint8_t banks;			// deprecated; total number of banks in this mode
    uint8_t memory_model;
    uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
    uint8_t image_pages;
    uint8_t reserved0;

    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;

    uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
    uint8_t reserved1[206];
} __attribute__ ((packed));

typedef struct vbe_mode_info_structure * VBEInfoPtr;

VBEInfoPtr VBE_mode_info = (VBEInfoPtr) 0x0000000000005C00;

static int cursorX, cursorY;     // (Pixels)
static uint32_t text_color = COLOR_WHITE;
static uint32_t background_color = COLOR_BLACK;
static int zoom = 2;   

static uint8_t font_bitmap[256 * CHAR_HEIGHT];

void vd_put_pixel(uint32_t hexColor, uint64_t x, uint64_t y) {
    uint8_t * framebuffer = (uint8_t *)(uintptr_t) VBE_mode_info->framebuffer;
    uint64_t offset = (x * ((VBE_mode_info->bpp)/8)) + (y * VBE_mode_info->pitch);
    framebuffer[offset]     =  (hexColor) & 0xFF;
    framebuffer[offset+1]   =  (hexColor >> 8) & 0xFF;
    framebuffer[offset+2]   =  (hexColor >> 16) & 0xFF;
}

void vd_put_multpixel(uint32_t hexColor, uint64_t x, uint64_t y, int mult) {
    for (int i = 0; i < mult; i++) {
        for (int j = 0; j < mult; j++) {
            vd_put_pixel(hexColor, x+i, y+j);
        }
    }
}

void draw_char(char c) {
    int mask[8]={0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01}; //mascara para cada bit
    unsigned char * glyph=font_bitmap+(int)c*16; // se posiciona en la letra correspondiente

    for (int cy = 0; cy < CHAR_HEIGHT*zoom; cy+=zoom) {
        for (int cx = 0; cx < CHAR_WIDTH*zoom; cx+=zoom) {
            vd_put_multpixel(glyph[cy/zoom] & mask[cx/zoom] ? text_color : background_color, (cursorX + cx), (cursorY + cy), zoom);
        }
    }
}

static void scroll_screen(void) {
    uint8_t *fb    = (uint8_t*)(uintptr_t)VBE_mode_info->framebuffer;
    uint32_t pitch = VBE_mode_info->pitch;
    uint32_t h     = VBE_mode_info->height;
    uint32_t lines = CHAR_HEIGHT * zoom;
    uint64_t move_bytes = (uint64_t)(h - lines) * pitch;

    // slide every scanline up by “lines”
    _memcpy(fb, fb + (uint64_t)lines * pitch, move_bytes);

    // clear the new bottom “lines” of pixels
    _memset(fb + move_bytes, 0, (uint64_t)lines * pitch);
}

int vd_get_zoom(void) {
    return zoom;
}

void vd_put_char(unsigned char c, FileDescriptor fd) {
    text_color = (fd == STDOUT) ? COLOR_WHITE : COLOR_RED;

    if (c == '\n') { // new line
        cursorX = 0;
        cursorY += CHAR_HEIGHT * zoom;

    } else if (c == '\b') { // delete 
        if (cursorX >= CHAR_WIDTH * zoom) {
            cursorX -= CHAR_WIDTH * zoom;
            draw_char(' ');
        } else if (cursorX == 0 && cursorY >= CHAR_HEIGHT * zoom) {
            cursorY -= CHAR_HEIGHT * zoom;
            cursorX = (VBE_mode_info->width / (CHAR_WIDTH * zoom)) * CHAR_WIDTH * zoom; // move to the end of the previous line
            draw_char(' ');
        } 

    } else {
        draw_char(c);
        cursorX += CHAR_WIDTH * zoom;
    }

    // check if cursorX exceeds screen width
    if (cursorX + CHAR_WIDTH * zoom > VBE_mode_info->width) {
        cursorX = 0;
        cursorY += CHAR_HEIGHT * zoom;
    }

    // check if cursorY exceeds screen height
    if (cursorY + CHAR_HEIGHT * zoom > VBE_mode_info->height) { 
        scroll_screen();
        cursorY -= CHAR_HEIGHT * zoom;
    }
}

void vd_put_string(const char *str, FileDescriptor fd) {
    while (*str) {
        vd_put_char(*str++, fd);
    }
}

// Convert a non‐negative value to a string in buf using the given base (2–36).q2   1
int itoa(int64_t value, char *buf, int base) {
    if (base < 2 || base > 36) {
        *buf = '\0';
        return 0;
    }

    char *p = buf;
    uint64_t u = (uint64_t)value;  // assume value >= 0
    int min_len = 0;

    if (base == 16)
        min_len = 16;

    // Generar dígitos en orden inverso
    int digit_count = 0;
    do {
        int digit = u % base;
        *p++ = (digit < 10 ? '0' + digit : 'A' + (digit - 10));
        u /= base;
        digit_count++;
    } while (u);

    // Rellenar con ceros a la izquierda si es necesario
    while (digit_count < min_len) {
        *p++ = '0';
        digit_count++;
    }

    int len = p - buf;
    *p = '\0';

    // Invertir el buffer in-place
    char *start = buf, *end = p - 1;
    while (start < end) {
        char tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }

    return len;
}

void vd_init(void) {
    cursorX = cursorY = 0;
    vd_clear_screen();
}

void vd_set_zoom(int new_zoom) {
    if (new_zoom >= ZOOM_MIN && new_zoom <= ZOOM_MAX) {
        vd_put_char('\n', STDOUT); // Print a space to trigger the screen update
        zoom = new_zoom;
    } else {
        vd_put_string("\nInvalid zoom level. Usage: zoom <number (Between 1 and 10)>\n", STDOUT);
    }
}

void vd_set_cursor(int col, int row) {
    cursorX = col ;
    cursorY = row;
}

void vd_clear_screen(void) {
    uint8_t *framebuffer = (uint8_t *)(uintptr_t)VBE_mode_info->framebuffer;
    uint32_t total_bytes = VBE_mode_info->width * VBE_mode_info->height * (VBE_mode_info->bpp / 8);

    if (background_color == COLOR_BLACK) {
        _memset(framebuffer, 0, total_bytes); // Clear to black
    } else {
        vd_draw_rectangle(0, 0, VBE_mode_info->width, VBE_mode_info->height, background_color);
    }
    
    cursorX = 0;
    cursorY = 0;
}

void vd_draw_rectangle(int x, int y, int width, int height, uint32_t color) {
    // Boundary checks to prevent drawing outside screen
    if (x < 0 || y < 0 || x >= VBE_mode_info->width || y >= VBE_mode_info->height) {
        return;
    }
    
    // Clamp width and height to stay within screen bounds
    if (x + width > VBE_mode_info->width) {
        width = VBE_mode_info->width - x;
    }
    if (y + height > VBE_mode_info->height) {
        height = VBE_mode_info->height - y;
    }
    
    // Draw the rectangle row by row for better cache performance
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            vd_put_pixel(color, x + col, y + row);
        }
    }
}

void vd_draw_circle(int center_x, int center_y, int radius, uint32_t color) {
    if (center_x + radius < 0 || center_x - radius >= VBE_mode_info->width ||
        center_y + radius < 0 || center_y - radius >= VBE_mode_info->height) {
        return;
    }
    
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            // Check if point is inside circle using distance formula
            if (x * x + y * y <= radius * radius) {
                int px = center_x + x;
                int py = center_y + y;
                
                // Bounds checking
                if (px >= 0 && px < VBE_mode_info->width && 
                    py >= 0 && py < VBE_mode_info->height) {
                    vd_put_pixel(color, px, py);
                }
            }
        }
    }
}

void vd_get_mode_info(ModeInfo *mode) {
    mode->width = VBE_mode_info->width;
    mode->height = VBE_mode_info->height;
    mode->bpp = VBE_mode_info->bpp;
}

static uint8_t font_bitmap[256 * CHAR_HEIGHT] = {
        // Relleno para las primeras letras (Espacio, símbolos, etc.)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,


        // Font para la letra ' '
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        // Font para la letra '!'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        // Font para la letra ' " '
        0b00000000,
        0b01100110,
        0b01100110,
        0b01000100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        // Font para la letra '#'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001010,
        0b00010100,
        0b01111110,
        0b00010100,
        0b01111110,
        0b00100100,
        0b01001000,
        0b00000000,
        0b00000000,
        0b00000000,
        // Font para la letra '$'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00111110,
        0b01011000,
        0b01011000,
        0b00111000,
        0b00011100,
        0b00011010,
        0b00011010,
        0b00011100,
        0b01111000,
        0b00011000,
        0b00000000,
        0b00000000,

        // Font para la letra '%'
        0b00000000,
        0b00000000,
        0b00110000,
        0b01001000,
        0b00110010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01001100,
        0b00010010,
        0b00001100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para la letra '&'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00100100,
        0b00100000,
        0b00100000,
        0b00010000,
        0b00110000,
        0b01001010,
        0b01000110,
        0b00111010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para la letra '''
        0b00000000,
        0b00011000,
        0b00011000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para la letra '('
        0b00000000,
        0b00000000,
        0b00001100,
        0b00010000,
        0b00010000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00010000,
        0b00010000,
        0b00001100,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para la letra ')'
        0b00000000,
        0b00000000,
        0b00110000,
        0b00001000,
        0b00001000,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00001000,
        0b00001000,
        0b00110000,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para la letra '*'
        0b00000000,
        0b00000000,
        0b00010000,
        0b01010010,
        0b00110100,
        0b00011000,
        0b00110100,
        0b01010010,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para la letra '+'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00010000,
        0b00010000,
        0b01111100,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para la letra ','
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00010000,
        0b00100000,

        // Font para la letra '-'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para la letra '.'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,

        // Font para la letra '/'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000100,
        0b00000100,
        0b00001000,
        0b00001000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00100000,
        0b00100000,
        0b01000000,
        0b01000000,
        0b00000000,
        0b00000000,

        // Font para el número '0' estirado
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000110,
        0b01001010,
        0b01001010,
        0b01001010,
        0b01010010,
        0b01010010,
        0b01100010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,

        // Font para el número '1' estirado
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001000,
        0b00011000,
        0b00101000,
        0b01001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b01111110,
        0b00000000,
        0b00000000,

        // Font para el número '2' estirado
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b01111110,
        0b00000000,
        0b00000000,

        // Font para el número '3' estirado
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b00000010,
        0b00000010,
        0b00000100,
        0b00111000,
        0b00000100,
        0b00000010,
        0b00000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,

        // Font para el número '4' estirado
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000100,
        0b00001100,
        0b00010100,
        0b00100100,
        0b01000100,
        0b01111110,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000000,
        0b00000000,

        // Font para el número '5' estirado
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111100,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b01111100,
        0b00000000,
        0b00000000,

        // Font para el número '6' estirado
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000000,
        0b01000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,

        // Font para el número '7' estirado
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000010,
        0b00000010,
        0b00000100,
        0b00000100,
        0b00001000,
        0b00001000,
        0b00010000,
        0b00010000,
        0b00100000,
        0b00100000,
        0b00000000,
        0b00000000,

        // Font para el número '8' estirado
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,

        // Font para el número '9' estirado
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000010,
        0b00000010,
        0b00000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,

        // Font para la letra ':'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para la letra ';'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00011000,
        0b00010000,
        0b00100000,
        0b00000000,
        0b00000000,

        // Font para la letra '<'
        0b00000000,
        0b00000000,
        0b00000010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b00100000,
        0b00010000,
        0b00001000,
        0b00000100,
        0b00000010,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para la letra '='
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para la letra '>'
        0b00000000,
        0b00000000,
        0b01000000,
        0b00100000,
        0b00010000,
        0b00001000,
        0b00000100,
        0b00000010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para la letra '?'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b00000100,
        0b00001000,
        0b00001000,
        0b00000000,
        0b00001000,
        0b00001000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para la letra '@'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01011110,
        0b01010110,
        0b01010110,
        0b01011110,
        0b01000000,
        0b00111110,
        0b00000000,
        0b00000000,
        0b00000000,


        // Font para la letra 'A'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        // Font para la letra 'B'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b00000000,
        0b00000000,
        // Font para la letra 'C'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        // Font para la letra 'D'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b00000000,
        0b00000000,
        // Font para la letra 'E'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111110,
        0b00000000,
        0b00000000,
        // Font para la letra 'F'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00000000,
        0b00000000,
        // Font para la letra 'G'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01001110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        // Font para la letra 'H'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        // Font para la letra 'I'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111110,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00111110,
        0b00000000,
        0b00000000,
        // Font para la letra 'J'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111110,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        // Font para la letra 'K'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000100,
        0b01001000,
        0b01110000,
        0b01001100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        // Font para la letra 'L'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111110,
        0b00000000,
        0b00000000,
        // Font para la letra 'M'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01100110,
        0b01100110,
        0b01011010,
        0b01011010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        // Font para la letra 'N'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01100010,
        0b01100010,
        0b01010010,
        0b01010010,
        0b01011010,
        0b01001010,
        0b01000110,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        // Font para la letra 'O'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        // Font para la letra 'P'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00000000,
        0b00000000,
        // Font para la letra 'Q'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01001010,
        0b01000100,
        0b00111010,
        0b00000000,
        0b00000000,
        // Font para la letra 'P'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        // Font para la letra 'S'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00111100,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        // Font para la letra 'T'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111110,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00000000,
        0b00000000,
        // Font para la letra 'U'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,
        // Font para la letra 'V'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00100100,
        0b00100100,
        0b00100100,
        0b00011000,
        0b00011000,
        0b00000000,
        0b00000000,
        // Font para la letra 'W'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01011010,
        0b01011010,
        0b01100110,
        0b01100110,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        // Font para la letra 'X'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b00100100,
        0b00100100,
        0b00011000,
        0b00011000,
        0b00100100,
        0b00100100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,
        // Font para la letra 'Y'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00100010,
        0b00100010,
        0b00010100,
        0b00010100,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00000000,
        0b00000000,
        // Font para la letra 'Z' [REVISAR]
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b00000100,
        0b00000100,
        0b00001000,
        0b00001000,
        0b00010000,
        0b00010000,
        0b00100000,
        0b00100000,
        0b01000000,
        0b01111110,
        0b00000000,
        0b00000000,
        // Font para el caracter '['
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00111100,
        0b00000000,
        0b00000000,

        // Font para el caracter '\'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000000,
        0b01000000,
        0b00100000,
        0b00100000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00001000,
        0b00001000,
        0b00000100,
        0b00000100,
        0b00000000,
        0b00000000,

        // Font para el caracter ']'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00111100,
        0b00000000,
        0b00000000,

        // Font para el caracter '^'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001000,
        0b00010100,
        0b00100010,
        0b01000001,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para el caracter '_'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111111,
        0b00000000,

        // Font para el caracter '`'
        0b00000000,
        0b00010000,
        0b00001000,
        0b00000100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,

        // Font para la letra 'a'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b00000010,
        0b00000010,
        0b00111110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000000,
        0b00000000,

        // Font para la letra 'b'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000000,
        0b01000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b00000000,
        0b00000000,

        // Font para la letra 'c'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,

        // Font para la letra 'd'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000010,
        0b00000010,
        0b00111110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000000,
        0b00000000,

        // Font para la letra 'e'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111110,
        0b01000000,
        0b01000000,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,

        // Font para la letra 'f'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001110,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00111100,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,

        // Font para la letra 'g'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000010,
        0b00000010,
        0b01111100,
        0b00000000,
        0b00000000,

        // Font para la letra 'h'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,

        // Font para la letra 'i'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00011100,
        0b00000000,
        0b00000000,

        // Font para la letra 'j'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001000,
        0b00000000,
        0b00000000,
        0b00011000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b01001000,
        0b00110000,
        0b00000000,
        0b00000000,

        // Font para la letra 'k'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000000,
        0b01000000,
        0b01000010,
        0b01000100,
        0b01000100,
        0b01001000,
        0b01110000,
        0b01001000,
        0b01000100,
        0b01000100,
        0b01000010,
        0b00000000,
        0b00000000,

        // Font para la letra 'l'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00110000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00111000,
        0b00000000,
        0b00000000,

        // Font para la letra 'm'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111110,
        0b01001001,
        0b01001001,
        0b01001001,
        0b01001001,
        0b01001001,
        0b01001001,
        0b01001001,
        0b01001001,
        0b00000000,
        0b00000000,

        // Font para la letra 'n'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000,
        0b00000000,

        // Font para la letra 'o'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000,
        0b00000000,

        // Font para la letra 'p'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01111100,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00000000,

        // Font para la letra 'q'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000000,

        // Font para la letra 'r'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000000,
        0b01001100,
        0b01010010,
        0b01100000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00000000,
        0b00000000,

        // Font para la letra 's'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111110,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00111100,
        0b00000010,
        0b00000010,
        0b00000010,
        0b01111100,
        0b00000000,
        0b00000000,

        // Font para la letra 't'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00100000,
        0b00100000,
        0b01111000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100100,
        0b00011000,
        0b00000000,
        0b00000000,

        // Font para la letra 'u'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000000,
        0b00000000,

        // Font para la letra 'v'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00100010,
        0b00100100,
        0b00100100,
        0b00100100,
        0b00010100,
        0b00001000,
        0b00000000,
        0b00000000,

        // Font para la letra 'w'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01000010,
        0b01001010,
        0b01001010,
        0b01011010,
        0b01010110,
        0b00100100,
        0b00000000,
        0b00000000,

        // Font para la letra 'x'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00100010,
        0b00100010,
        0b00010100,
        0b00001000,
        0b00001000,
        0b00010100,
        0b00100010,
        0b00100010,
        0b00100010,
        0b00000000,
        0b00000000,

        // Font para la letra 'y'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01000100,
        0b01000100,
        0b01000100,
        0b01000100,
        0b01000100,
        0b00111100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00111000,
        0b00000000,

        // Font para la letra 'z'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01111100,
        0b00000100,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b01000000,
        0b01111100,
        0b00000000,
        0b00000000,

        // Font para el caracter '{'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00001110,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00110000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00001110,
        0b00000000,
        0b00000000,

        // Font para el caracter '|'
        0b00000000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00000000,

        // Font para el caracter '}'
        0b00000000,
        0b00000000,
        0b00000000,
        0b01110000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001100,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b01110000,
        0b00000000,
        0b00000000,

        // Font para el caracter '~'
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00110010,
        0b01001100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,

};
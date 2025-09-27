#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <stdint.h>
#include "types.h"  // Include the shared types
#include "libc.h"

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

#define COLOR_WHITE 0xFFFFFF
#define COLOR_BLACK 0x000000
#define COLOR_RED 0xFF0000

#define ZOOM_MAX 10  // Maximum zoom level
#define ZOOM_MIN 1   // Minimum zoom level

/** Character cell dimensions (in pixels) */ 
#define CHAR_WIDTH   8
#define CHAR_HEIGHT 16

#define NUMBER_OF_REGISTERS 18 // Number of CPU registers we want to inspect

void vd_put_pixel( uint32_t color, uint64_t x, uint64_t y);

void vd_put_multpixel(uint32_t hexColor, uint64_t x, uint64_t y, int mult);

void draw_char(char c);

/**
 * @brief Initialize the graphics‐mode console.
 *        Must be called once after switching into a VBE linear‐framebuffer mode.
 */
void vd_init(void);

/**
 * @brief Draw one character at the current cursor, advance, wrap & scroll.
 * @args c:  Character to draw
 *       fd: FileDescriptor (STDOUT or STDERR) to select text color
 */
void vd_put_char(unsigned char c, FileDescriptor fd);

/**
 * @brief Draw a string at the current cursor, advance, wrap & scroll.
 * @args c:  String to draw
 *       fd: FileDescriptor (STDOUT or STDERR) to select text color
 */
void vd_put_string(const char *str, FileDescriptor fd);

int vd_get_zoom(void);
/**
 * @brief Clear the entire screen and reset the cursor to the top‐left.
 */
void vd_clear_screen(void);

/**
 * @brief Move the console cursor to the given character cell.
 * @param col: Column index (0..)
 * @param row: Row index (0..)
 */
void vd_set_cursor(int col, int row);
/**
 * @brief Show the CPU registers in a human‐readable format.
 *        This function will print the register names and their values to the console.
 */
void vd_show_registers(FileDescriptor fd);
/**
 * @brief Set the zoom level for the text.
 *        The zoom level affects the size of characters drawn on the screen.
 * @param new_zoom: New zoom level (1 to 10)
 */
void vd_set_zoom(int new_zoom);

/**
 * @brief Draws a filled rectangle on the screen
 * @param x: X coordinate of top-left corner
 * @param y: Y coordinate of top-left corner  
 * @param width: Width of the rectangle in pixels
 * @param height: Height of the rectangle in pixels
 * @param color: Color in RGB format (0xRRGGBB)
 */
void vd_draw_rectangle(int x, int y, int width, int height, uint32_t color);

/**
 * @brief Draws a filled circle
 * @param center_x: X coordinate of circle center
 * @param center_y: Y coordinate of circle center
 * @param radius: Radius of the circle in pixels
 * @param color: Color in RGB format (0xRRGGBB)
 */
void vd_draw_circle(int x, int y, int radius, uint32_t color);

//void vd_draw_bitmap(int x, int y, int width, int height, const uint32_t *pixels);

void vd_get_mode_info(ModeInfo *mode);

#endif 
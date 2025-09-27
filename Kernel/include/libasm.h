#ifndef LIBASM_H
#define LIBASM_H

#include <stdint.h>

extern void _hlt(void);

extern void _sti(void);
extern void _cli(void);
/**
 * @brief Retrieves the CPU vendor string.
 *
 * @param vendor A buffer to store the CPU vendor string.
 */
extern void cpuVendor(char *vendor);

// Send a byte to an I/O port
extern void outb(uint16_t port, uint8_t val);

// Send a word (2 bytes) to an I/O port
extern void outw(uint16_t port, uint16_t val);

// Receive a byte from an I/O port
extern uint8_t inb(uint16_t port);



#endif
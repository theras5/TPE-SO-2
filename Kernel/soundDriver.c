#include "soundDriver.h"

void play_sound(unsigned int frequency) {
    // Input validation
    if (frequency < MIN_FREQUENCY || frequency > MAX_FREQUENCY) {
        return; // Invalid frequency, do nothing
    }
    
    unsigned int divisor;
    unsigned char tmp;
    
    // Calculate divisor for the PIT to generate the desired frequency
    divisor = PIT_FREQUENCY / frequency;
    
    // Prevent division by zero or invalid divisor
    if (divisor == 0 || divisor > 0xFFFF) {
        return;
    }
    
    // Configure PIT channel 2 in square wave mode, 16-bit (0xB6 = 10110110b)
    outb(PIT_COMMAND_PORT, 0xB6);
    
    // Send divisor to PIT channel 2 
    outb(PIT_CHANNEL2_PORT, (unsigned char)(divisor & 0xFF));        // Low byte
    outb(PIT_CHANNEL2_PORT, (unsigned char)((divisor >> 8) & 0xFF)); // High byte
    
    // Enable PC speaker by setting bits 0 and 1 in speaker control port
    tmp = inb(SPEAKER_CONTROL_PORT);
    outb(SPEAKER_CONTROL_PORT, tmp | 0x03); // Set bits 0 and 1
}

void beep(unsigned int frequency, unsigned int duration_ms) {
    if (frequency < MIN_FREQUENCY || frequency > MAX_FREQUENCY) return;

    unsigned int divisor = PIT_FREQUENCY / frequency;

    // Configurar PIT canal 2 en modo square wave (10110110b)
    outb(PIT_COMMAND_PORT, 0xB6);
    outb(PIT_CHANNEL2_PORT, (uint8_t) (divisor & 0xFF));        // Low byte
    outb(PIT_CHANNEL2_PORT, (uint8_t) ((divisor >> 8) & 0xFF)); // High byte

    // Activar speaker (bits 0 y 1)
    uint8_t tmp = inb(SPEAKER_CONTROL_PORT);
    outb(SPEAKER_CONTROL_PORT, tmp | 0x03);

    // Esperar el tiempo indicado (QEMU necesita delay real)
    int ticks_to_wait = (duration_ms * 18) / 1000; // Convertir ms a ticks (18.2 ticks/segundo)
    sleep(ticks_to_wait);

    // Desactivar solo bit 0 (mantener 1 para dejar en modo "activo")
    tmp = inb(SPEAKER_CONTROL_PORT);
    outb(SPEAKER_CONTROL_PORT, tmp & ~0x01);
}

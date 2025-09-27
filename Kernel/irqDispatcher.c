#include <time.h>
#include <stdint.h>
#include "keyboardDriver.h"

static void int_20(void);
static void int_21(void);

static void (*interrupt_arr[])(void) = {int_20, int_21};

void irqDispatcher(uint64_t irq) {
    interrupt_arr[irq]();
    return;
}

void int_20(void) {
    timer_handler();
}

void int_21(void) {
    kbd_get_key();
}
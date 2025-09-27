#include "time.h"
#include "libasm.h"

static unsigned long ticks = 0;


void timer_handler() {
	ticks++;
}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / 18;
}

unsigned long get_ticks(){
    return ticks;
}

void sleep(int ticksToWait) {
    unsigned long start = ticks;
    while (ticks - start < ticksToWait){
        _hlt();
    };
}
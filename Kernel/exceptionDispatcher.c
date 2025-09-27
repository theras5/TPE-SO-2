#include "exceptionDispatcher.h"

static void zero_division();
static void invalid_opcode();

void exceptionDispatcher(uint64_t exception) {
	if (exception == ZERO_EXCEPTION_ID)
		zero_division();
    if (exception == INVALID_OPCODE)
        invalid_opcode();
}

static void zero_division() {
    _sti();
    vd_put_string("\nZero Division Exception\n", STDOUT);
    vd_put_string("This exception occurs when a division by zero is attempted.\n", STDOUT);
    char buff[1000];
    copy_regs(buff);
    vd_put_string(buff, STDOUT);
}


static void invalid_opcode() {
    _sti();
    vd_put_string("\nInvalid Opcode Exception\n", STDOUT);
    vd_put_string("This exception occurs when an invalid or undefined instruction is executed.\n", STDOUT);
    char buff[1000];
    copy_regs(buff);
    vd_put_string(buff, STDOUT);
}